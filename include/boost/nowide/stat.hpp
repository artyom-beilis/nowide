//
//  Copyright (c) 2020 Alexander Grund
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_NOWIDE_STAT_HPP_INCLUDED
#define BOOST_NOWIDE_STAT_HPP_INCLUDED

#include <boost/nowide/config.hpp>
#include <sys/types.h>
// Include after sys/types.h
#include <sys/stat.h>

namespace boost {
namespace nowide {
#if !defined(BOOST_WINDOWS) && !defined(BOOST_NOWIDE_DOXYGEN)
    using stat_t = struct stat;
    using posix_stat_t = struct stat;

    using ::stat;
#else
    /// \brief Typedef for the file info structure.
    /// Able to hold 64 bit filesize and timestamps on Windows and usually also on other 64 Bit systems
    /// This allows to write portable code with option LFS support
    using stat_t = struct ::__stat64;
    /// \brief Typedef for the file info structure used in the POSIX stat call
    /// Resolves to `struct _stat` on Windows and `struct stat` otherwise
    /// This allows to write portable code using the default stat function
    using posix_stat_t = struct ::_stat;

    /// \cond INTERNAL
    namespace detail {
        BOOST_NOWIDE_DECL int stat(const char* path, posix_stat_t* buffer, size_t buffer_size);
    }
    /// \endcond

    ///
    /// \brief UTF-8 aware stat function, returns 0 on success
    ///
    /// Return information about a file from an UTF-8 encoded path
    ///
    BOOST_NOWIDE_DECL int stat(const char* path, stat_t* buffer);
    ///
    /// \brief UTF-8 aware stat function, returns 0 on success
    ///
    /// Return information about a file from an UTF-8 encoded path
    ///
    inline int stat(const char* path, posix_stat_t* buffer)
    {
        return detail::stat(path, buffer, sizeof(*buffer));
    }
#endif
} // namespace nowide
} // namespace boost

#endif
