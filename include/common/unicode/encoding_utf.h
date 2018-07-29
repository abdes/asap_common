//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <iterator>
#include <string>

#include <common/unicode/encoding_errors.h>
#include <common/unicode/utf.h>

//#ifdef NOWIDE_MSVC
//#  pragma warning(push)
//#  pragma warning(disable : 4275 4251 4231 4660)
//#endif

namespace nowide {
namespace conv {
///
/// Convert a Unicode text in range [begin,end) to other Unicode encoding
///
template <typename CharOut, typename CharIn,
          typename Traits = std::char_traits<CharOut>,
          class Allocator = std::allocator<CharOut>>
std::basic_string<CharOut, Traits, Allocator> utf_to_utf(
    CharIn const *begin, CharIn const *end,
    const Allocator &alloc = Allocator()) {
  std::basic_string<CharOut, Traits, Allocator> result(alloc);
  result.reserve(end - begin);
  typedef std::back_insert_iterator<
      std::basic_string<CharOut, Traits, Allocator>>
      inserter_type;
  inserter_type inserter(result);
  utf::code_point c;
  while (begin != end) {
    c = utf::utf_traits<CharIn>::template decode<CharIn const *>(begin, end);
    if (c == utf::illegal || c == utf::incomplete) {
      throw conversion_error();
    } else {
      utf::utf_traits<CharOut>::template encode<inserter_type>(c, inserter);
    }
  }
  return result;
}

///
/// Convert a Unicode NUL terminated string \a str other Unicode encoding
///
template <typename CharOut, typename CharIn,
          typename Traits = std::char_traits<CharOut>,
          class Allocator = std::allocator<CharOut>>
std::basic_string<CharOut, Traits, Allocator> utf_to_utf(
    CharIn const *str, const Allocator &alloc = Allocator()) {
  CharIn const *end = str;
  while (*end) end++;
  return utf_to_utf<CharOut, CharIn, Traits, Allocator>(str, end, alloc);
}

///
/// Convert a Unicode string \a str other Unicode encoding
///
template <typename CharOut, typename CharIn,
          typename Traits = std::char_traits<CharOut>,
          class Allocator = std::allocator<CharOut>>
std::basic_string<CharOut, Traits, Allocator> utf_to_utf(
    std::basic_string<CharIn> const &str,
    const Allocator &alloc = Allocator()) {
  return utf_to_utf<CharOut, CharIn, Traits, Allocator>(
      str.c_str(), str.c_str() + str.size(), alloc);
}

}  // namespace conv
}  // namespace nowide

//#ifdef NOWIDE_MSVC
//#pragma warning(pop)
//#endif
