//
//  Copyright (c) 2015 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/nowide/utf8_codecvt.hpp>
#include <boost/nowide/convert.hpp>
#include <boost/filesystem/operations.hpp>
#include <locale>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "test.hpp"
#include "test_sets.hpp"

namespace detail
{

static char const * utf8_name = "\xf0\x9d\x92\x9e-\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82-\xE3\x82\x84\xE3\x81\x82.txt";
static wchar_t const * wide_name = L"\U0001D49E-\u043F\u0440\u0438\u0432\u0435\u0442-\u3084\u3042.txt";

static std::string prefix = boost::filesystem::unique_path( "nowide-%%%%-%%%%-" ).string();

static std::string utf8_name_str = prefix + utf8_name;
static std::wstring wide_name_str = boost::nowide::widen( prefix ) + wide_name;

} // detail

static char const * utf8_name = detail::utf8_name_str.c_str();
static wchar_t const * wide_name = detail::wide_name_str.c_str();

char const *res(std::codecvt_base::result r)
{
    switch(r){
    case std::codecvt_base::ok: return "ok";
    case std::codecvt_base::partial: return "partial";
    case std::codecvt_base::error: return "error";
    case std::codecvt_base::noconv: return "noconv";
    default:
        return "error";
    }
}

typedef std::codecvt<wchar_t,char,std::mbstate_t> cvt_type;

void test_codecvt_in_n_m(cvt_type const &cvt,int n,int m)
{
    wchar_t const *wptr = wide_name;
    int wlen = std::wcslen(wide_name);
    int u8len = std::strlen(utf8_name);
    char const *from = utf8_name;
    char const *end = from;
    char const *real_end = utf8_name + u8len;
    char const *from_next = from;
    std::mbstate_t mb=std::mbstate_t();
    while(from_next < real_end) {
        if(from == end) {
            end = from + n;
            if(end > real_end)
                end = real_end;
        }

        wchar_t buf[128];
        wchar_t *to = buf;
        wchar_t *to_end = to + m;
        wchar_t *to_next = to;


        std::mbstate_t mb2 = mb;
        std::codecvt_base::result r = cvt.in(mb,from,end,from_next,to,to_end,to_next);
        //std::cout << "In from_size=" << (end-from) << " from move=" <<  (from_next - from) << " to move= " << to_next - to << " state = " << res(r) << std::endl;

        int count = cvt.length(mb2,from,end,to_end - to);
        #ifndef BOOST_NOWIDE_DO_LENGTH_MBSTATE_CONST
        TEST(std::memcmp(&mb,&mb2,sizeof(mb))==0);
        if(count != from_next - from) {
            std::cout << count << " " << from_next - from << std::endl;
        }
        TEST(count == from_next - from);
        #else
        TEST(count == to_next - to);
        #endif


        if(r == cvt_type::partial) {
            end+=n;
            if(end > real_end)
                end = real_end;
        }
        else
            TEST(r == cvt_type::ok);
        while(to!=to_next) {
            TEST(*wptr == *to);
            wptr++;
            to++;
        }
        to=to_next;
        from = from_next;
    }
    TEST(wptr == wide_name + wlen);
    TEST(from == real_end);

}

void test_codecvt_out_n_m(cvt_type const &cvt,int n,int m)
{
    char const *nptr = utf8_name;
    int wlen = std::wcslen(wide_name);
    int u8len = std::strlen(utf8_name);

    std::mbstate_t mb=std::mbstate_t();

    wchar_t const *from_next = wide_name;
    wchar_t const *real_from_end = wide_name + wlen;

    char buf[256];
    char *to = buf;
    char *to_next = to;
    char *to_end = to + n;
    char *real_to_end = buf + sizeof(buf);

    while(from_next < real_from_end) {
        wchar_t const *from = from_next;
        wchar_t const *from_end = from + m;
        if(from_end > real_from_end)
            from_end = real_from_end;
        if(to_end == to) {
            to_end = to+n;
        }

        std::codecvt_base::result r = cvt.out(mb,from,from_end,from_next,to,to_end,to_next);
        //std::cout << "In from_size=" << (end-from) << " from move=" <<  (from_next - from) << " to move= " << to_next - to << " state = " << res(r) << std::endl;
        if(r == cvt_type::partial) {
            TEST(to_end - to_next < cvt.max_length());
            to_end += n;
            if(to_end > real_to_end)
                to_end = real_to_end;
        }
        else {
            TEST(r == cvt_type::ok);
        }

        while(to!=to_next) {
            TEST(*nptr == *to);
            nptr++;
            to++;
        }
        from = from_next;
    }
    TEST(nptr == utf8_name + u8len);
    TEST(from_next == real_from_end);
    TEST(cvt.unshift(mb,to,to+n,to_next)==cvt_type::ok);
    TEST(to_next == to);

}


void test_codecvt_conv()
{
    std::cout << "Conversions " << std::endl;
    std::locale l(std::locale::classic(),new boost::nowide::utf8_codecvt<wchar_t>());

    cvt_type const &cvt = std::use_facet<cvt_type>(l);

    for(int i=1;i<=(int)std::strlen(utf8_name)+1;i++) {
        for(int j=1;j<=(int)std::wcslen(wide_name)+1;j++) {
            try {
                test_codecvt_in_n_m(cvt,i,j);
                test_codecvt_out_n_m(cvt,i,j);
            }
            catch(...) {
                std::cerr << "Wlen=" <<j << " Nlen=" << i << std::endl;
                throw;
            }
        }
    }
}

void test_codecvt_err()
{
    std::cout << "Errors " << std::endl;
    std::locale l(std::locale::classic(),new boost::nowide::utf8_codecvt<wchar_t>());

    cvt_type const &cvt = std::use_facet<cvt_type>(l);

    std::cout << "- UTF-8" << std::endl;
    {

        wchar_t buf[4];
        wchar_t *to=buf;
        wchar_t *to_end = buf+4;
        wchar_t *to_next = to;
        char const *err_utf="1\xFF\xFF\xd7\xa9";
        {
            std::mbstate_t mb=std::mbstate_t();
            char const *from=err_utf;
            char const *from_end = from + std::strlen(from);
            char const *from_next = from;
            to_next = to;
            TEST(cvt.in(mb,from,from_end,from_next,to,to_end,to_next)==cvt_type::ok);
            TEST(from_next == from+5);
            TEST(to_next == to + 4);
            TEST(to[0] == L'1');
            TEST(to[1] == L'\uFFFD');
            TEST(to[2] == L'\uFFFD');
            TEST(to[3] == L'\u05e9');
        }
    }    
    
    std::cout << "- UTF-16/32" << std::endl;
    {
        
        char buf[32];
        char *to=buf;
        char *to_end = buf+32;
        char *to_next = to;
        wchar_t err_buf[3] = { '1' , 0xDC9E }; // second surrogate not works both for UTF-16 and 32
        wchar_t const *err_utf = err_buf;
        {
            std::mbstate_t mb=std::mbstate_t();
            wchar_t const *from=err_utf;
            wchar_t const *from_end = from + std::wcslen(from);
            wchar_t const *from_next = from;
            TEST(cvt.out(mb,from,from_end,from_next,to,to_end,to_next)==cvt_type::ok);
            TEST(from_next == from+2);
            TEST(to_next == to + 4);
            TEST(std::memcmp(to,"1\xEF\xBF\xBD",4)==0);
        }
    }    
    
}

std::wstring codecvt_to_wide(std::string const &s)
{
    std::locale l(std::locale::classic(),new boost::nowide::utf8_codecvt<wchar_t>());
 
    cvt_type const &cvt = std::use_facet<cvt_type>(l);
    std::vector<wchar_t> output(s.size()+1);
    
    std::mbstate_t mb=std::mbstate_t();
    char const *from=s.c_str();
    char const *from_end = from + s.size();
    char const *from_next = from;

    std::vector<wchar_t> buf(s.size()+1);
    wchar_t *to=&buf[0];
    wchar_t *to_end = to + buf.size();
    wchar_t *to_next = to;
    
    TEST(cvt.in(mb,from,from_end,from_next,to,to_end,to_next)==cvt_type::ok);

    std::wstring res(to,to_next);
    return res;

}


std::string codecvt_to_narrow(std::wstring const &s)
{
    std::locale l(std::locale::classic(),new boost::nowide::utf8_codecvt<wchar_t>());
 
    cvt_type const &cvt = std::use_facet<cvt_type>(l);
    std::vector<wchar_t> output(s.size()+1);
    
    std::mbstate_t mb=std::mbstate_t();
    wchar_t const *from=s.c_str();
    wchar_t const *from_end = from + s.size();
    wchar_t const *from_next = from;

    std::vector<char> buf(s.size() * 4 + 1);
    char *to=&buf[0];
    char *to_end = to + buf.size();
    char *to_next = to;
    
    TEST(cvt.out(mb,from,from_end,from_next,to,to_end,to_next)==cvt_type::ok);

    std::string res(to,to_next);
    return res;

}


void test_codecvt_subst()
{
    std::cout << "Substitutions " << std::endl;
    run_all(codecvt_to_wide,codecvt_to_narrow);
}

int main()
{   
    try {
        test_codecvt_conv();
        test_codecvt_err();
        test_codecvt_subst();
        
    }
    catch(std::exception const &e) {
        std::cerr << "Failed : " << e.what() << std::endl;
        BOOST_NOWIDE_TEST_RETURN_FAILURE;
    }

    return boost::report_errors();
}
///
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
