//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <common/unicode/encoding_errors.h>
#include <common/unicode/utf.h>

#include <iterator>
#include <string>

namespace nowide {

/// Namespace holding conversion functions between different unicode encodings.
namespace conv {

/// Convert a Unicode text in range [begin,end) to other Unicode encoding
template <typename CharOut, typename CharIn,
          typename Traits = std::char_traits<CharOut>,
          class Allocator = std::allocator<CharOut>>
auto utf_to_utf(CharIn const *begin, CharIn const *end,
                const Allocator &alloc = Allocator())
    -> std::basic_string<CharOut, Traits, Allocator> {
  std::basic_string<CharOut, Traits, Allocator> result(alloc);
  auto range_size = end - begin;
  if (range_size > 0) {
    result.reserve(
        static_cast<
            typename std::basic_string<CharOut, Traits, Allocator>::size_type>(
            range_size));
  }
  using inserter_type =
      std::back_insert_iterator<std::basic_string<CharOut, Traits, Allocator>>;
  inserter_type inserter(result);
  utf::code_point c = 0;
  while (begin != end) {
    c = utf::utf_traits<CharIn>::template decode<CharIn const *>(begin, end);
    if (c == utf::illegal || c == utf::incomplete) {
      throw conversion_error();
    }
    utf::utf_traits<CharOut>::template encode<inserter_type>(c, inserter);
  }
  return result;
}

/// Convert a Unicode NUL terminated string \a str other Unicode encoding
template <typename CharOut, typename CharIn,
          typename Traits = std::char_traits<CharOut>,
          class Allocator = std::allocator<CharOut>>
auto utf_to_utf(CharIn const *str, const Allocator &alloc = Allocator())
    -> std::basic_string<CharOut, Traits, Allocator> {
  CharIn const *end = str;
  while (*end) {
    end++;
  }
  return utf_to_utf<CharOut, CharIn, Traits, Allocator>(str, end, alloc);
}

/// Convert a Unicode string \a str other Unicode encoding
template <typename CharOut, typename CharIn,
          typename Traits = std::char_traits<CharOut>,
          class Allocator = std::allocator<CharOut>>
auto utf_to_utf(std::basic_string<CharIn> const &str,
                const Allocator &alloc = Allocator())
    -> std::basic_string<CharOut, Traits, Allocator> {
  return utf_to_utf<CharOut, CharIn, Traits, Allocator>(
      str.c_str(), str.c_str() + str.size(), alloc);
}

}  // namespace conv
}  // namespace nowide
