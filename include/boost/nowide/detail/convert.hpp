//
//  Copyright (c) 2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_NOWIDE_DETAIL_CONVERT_HPP_INCLUDED
#define BOOST_NOWIDE_DETAIL_CONVERT_HPP_INCLUDED

#include <boost/nowide/detail/utf.hpp>
#include <boost/nowide/replacement.hpp>
#include <iterator>
#include <string>

namespace boost {
namespace nowide {
    /// \cond INTERNAL
    namespace detail {
        ///
        /// Convert a buffer of UTF sequences in the range [source_begin, source_end)
        /// from \tparam CharIn to \tparam CharOut to the output \a buffer of size \a buffer_size.
        ///
        /// \return original buffer containing the NULL terminated string or NULL
        ///
        /// If there is not enough room in the buffer NULL is returned, and the content of the buffer is undefined.
        /// Any illegal sequences are replaced with the replacement character, see #BOOST_NOWIDE_REPLACEMENT_CHARACTER
        ///
        template<typename CharOut, typename CharIn>
        CharOut*
        convert_buffer(CharOut* buffer, size_t buffer_size, const CharIn* source_begin, const CharIn* source_end)
        {
            CharOut* rv = buffer;
            if(buffer_size == 0)
                return 0;
            buffer_size--;
            while(source_begin != source_end)
            {
                using namespace detail::utf;
                code_point c = utf_traits<CharIn>::template decode<const CharIn*>(source_begin, source_end);
                if(c == illegal || c == incomplete)
                {
                    c = BOOST_NOWIDE_REPLACEMENT_CHARACTER;
                }
                size_t width = utf_traits<CharOut>::width(c);
                if(buffer_size < width)
                {
                    rv = NULL;
                    break;
                }
                buffer = utf_traits<CharOut>::template encode<CharOut*>(c, buffer);
                buffer_size -= width;
            }
            *buffer++ = 0;
            return rv;
        }

        ///
        /// Convert the UTF sequences in range [begin, end) from \tparam CharIn to \tparam CharOut
        /// and return it as a string
        ///
        /// Any illegal sequences are replaced with the replacement character, see #BOOST_NOWIDE_REPLACEMENT_CHARACTER
        ///
        template<typename CharOut, typename CharIn>
        std::basic_string<CharOut> convert_string(const CharIn* begin, const CharIn* end)
        {
            std::basic_string<CharOut> result;
            result.reserve(end - begin);
            typedef std::back_insert_iterator<std::basic_string<CharOut> > inserter_type;
            inserter_type inserter(result);
            using namespace detail::utf;
            code_point c;
            while(begin != end)
            {
                c = utf_traits<CharIn>::template decode<const CharIn*>(begin, end);
                if(c == illegal || c == incomplete)
                {
                    c = BOOST_NOWIDE_REPLACEMENT_CHARACTER;
                }
                utf_traits<CharOut>::template encode<inserter_type>(c, inserter);
            }
            return result;
        }

        /// Return the pointer to the first NULL value encountered by increasing s
        /// Equivalent to `return s + strlen(s)` but more generic
        template<typename Char>
        const Char* strend(const Char* s)
        {
            while(*s)
                s++;
            return s;
        }

    } // namespace detail
      /// \endcond
} // namespace nowide
} // namespace boost

#endif
