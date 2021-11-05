//
//  Copyright (c) 2012 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <common/unicode/encoding_utf.h>

#include <string>

namespace nowide {
///
/// \brief Template function that converts a buffer of UTF sequence in range
/// [source_begin,source_end) to the output \a buffer of size \a buffer_size.
///
/// In case of success a NUL terminated string returned (buffer), otherwise 0 is
/// returned.
///
/// If there is not enough room in the buffer or the source sequence contains
/// invalid UTF 0 is returned, and the contend of the buffer is undefined.
///
template <typename CharOut, typename CharIn>
auto basic_convert(CharOut *buffer, size_t buffer_size,
                   CharIn const *source_begin, CharIn const *source_end)
    -> CharOut * {
  CharOut *rv = buffer;
  if (buffer_size == 0) {
    return nullptr;
  }
  buffer_size--;
  while (source_begin != source_end) {
    using nowide::utf::code_point;
    using nowide::utf::illegal;
    using nowide::utf::incomplete;
    using nowide::utf::utf_traits;
    code_point c = utf_traits<CharIn>::template decode<CharIn const *>(
        source_begin, source_end);
    if (c == illegal || c == incomplete) {
      rv = nullptr;
      break;
    }
    size_t width = utf_traits<CharOut>::width(c);
    if (buffer_size < width) {
      rv = nullptr;
      break;
    }
    buffer = utf_traits<CharOut>::template encode<CharOut *>(c, buffer);
    buffer_size -= width;
  }
  *buffer = 0;
  return rv;
}

/// \cond INTERNAL
namespace details {
//
// wcslen defined only in C99... So we will not use it
//
template <typename Char>
auto basic_strend(Char const *s) -> Char const * {
  while (*s) {
    s++;
  }
  return s;
}
}  // namespace details
/// \endcond

///
/// Convert NUL terminated UTF source string to NUL terminated \a output string
/// of size at most output_size (including NUL)
///
/// In case of surcess output is returned, if the input sequence is illegal,
/// or there is not enough room NULL is returned
///
inline auto narrow(char *output, size_t output_size, wchar_t const *source)
    -> char * {
  return basic_convert(output, output_size, source,
                       details::basic_strend(source));
}
///
/// Convert UTF text in range [begin,end) to NUL terminated \a output string of
/// size at most output_size (including NUL)
///
/// In case of surcess output is returned, if the input sequence is illegal,
/// or there is not enough room NULL is returned
///
inline auto narrow(char *output, size_t output_size, wchar_t const *begin,
                   wchar_t const *end) -> char * {
  return basic_convert(output, output_size, begin, end);
}
///
/// Convert NUL terminated UTF source string to NUL terminated \a output string
/// of size at most output_size (including NUL)
///
/// In case of surcess output is returned, if the input sequence is illegal,
/// or there is not enough room NULL is returned
///
inline auto widen(wchar_t *output, size_t output_size, char const *source)
    -> wchar_t * {
  return basic_convert(output, output_size, source,
                       details::basic_strend(source));
}
///
/// Convert UTF text in range [begin,end) to NUL terminated \a output string of
/// size at most output_size (including NUL)
///
/// In case of surcess output is returned, if the input sequence is illegal,
/// or there is not enough room NULL is returned
///
inline auto widen(wchar_t *output, size_t output_size, char const *begin,
                  char const *end) -> wchar_t * {
  return basic_convert(output, output_size, begin, end);
}

///
/// Convert between Wide - UTF-16/32 string and UTF-8 string.
///
/// nowide::conv::conversion_error is thrown in a case of a error
///
template <typename Allocator = std::allocator<char>>
inline auto narrow(wchar_t const *s, const Allocator &alloc = Allocator())
    -> std::string {
  return nowide::conv::utf_to_utf<char>(s, alloc);
}
///
/// Convert between UTF-8 and UTF-16 string, implemented only on Windows
/// platform
///
/// nowide::conv::conversion_error is thrown in a case of a error
///
template <typename Allocator = std::allocator<wchar_t>>
inline auto widen(char const *s, const Allocator &alloc = Allocator())
    -> std::wstring {
  return nowide::conv::utf_to_utf<wchar_t>(s, alloc);
}
///
/// Convert between Wide - UTF-16/32 string and UTF-8 string
///
/// nowide::conv::conversion_error is thrown in a case of a error
///
template <typename Allocator = std::allocator<char>>
inline auto narrow(std::wstring const &s, const Allocator &alloc = Allocator())
    -> std::string {
  return nowide::conv::utf_to_utf<char>(s, alloc);
}
///
/// Convert between UTF-8 and UTF-16 string, implemented only on Windows
/// platform
///
/// nowide::conv::conversion_error is thrown in a case of a error
///
template <typename Allocator = std::allocator<wchar_t>>
inline auto widen(std::string const &s, const Allocator &alloc = Allocator())
    -> std::wstring {
  return nowide::conv::utf_to_utf<wchar_t>(s, alloc);
}

///
/// Convert between Wide - UTF-16/32 string and UTF-8 string
///
/// nowide::conv::conversion_error is thrown in a case of a error
///
template <typename Allocator = std::allocator<char>>
inline auto narrow(wchar_t const *begin, wchar_t const *end,
                   const Allocator &alloc = Allocator()) -> std::string {
  return nowide::conv::utf_to_utf<char>(begin, end, alloc);
}

}  // namespace nowide
