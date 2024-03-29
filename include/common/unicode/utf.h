//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <hedley/hedley.h>

#include <cstddef>  // for std::size_t
#include <cstdint>  // for int types

// spdlog causes a bunch of compiler warnings we can't do anything about except
// temporarily disabling them
HEDLEY_DIAGNOSTIC_PUSH
#if defined(HEDLEY_GCC_VERSION)
HEDLEY_PRAGMA(GCC diagnostic ignored "-Wsign-conversion")
#endif

/// Namespace holding the API for unicode operations relying entirely on UTF-8
/// as the internal storage format for text (described in detail at
/// https://utf8everywhere.org).
namespace nowide {

/// Namespace that holds basic operations on UTF encoded sequences.
namespace utf {

/// \cond INTERNAL
#if defined(HEDLEY_GCC_VERSION)
#define NOWIDE_LIKELY(x) __builtin_expect((x), 1)
#define NOWIDE_UNLIKELY(x) __builtin_expect((x), 0)
#else
#define NOWIDE_LIKELY(x) (x)
#define NOWIDE_UNLIKELY(x) (x)
#endif
/// \endcond

/// The integral type that can hold a Unicode code point.
using code_point = std::uint32_t;

/// Special constant that defines illegal code point.
static const code_point illegal = 0xFFFFFFFFU;

/// Special constant that defines incomplete code point.
static const code_point incomplete = 0xFFFFFFFEU;

/// Checks if \a v is a valid code point.
inline auto is_valid_codepoint(code_point v) -> bool {
  if (v > 0x10FFFF) {
    return false;
  }
  return (v < 0xD800 || v > 0xDFFF);  // surrogates
}

#ifdef DOXYGEN_DOCUMENTATION_BUILD
///
/// \brief UTF Traits class - functions to convert UTF sequences to and from
/// Unicode code points
///
template <typename CharType, int size = sizeof(CharType)>
struct utf_traits {
  ///
  /// The type of the character
  ///
  typedef CharType char_type;
  ///
  /// Read one code point from the range [p,e) and return it.
  ///
  /// - If the sequence that was read is incomplete sequence returns
  /// incomplete,
  /// - If illegal sequence detected returns illegal
  ///
  /// Requirements
  ///
  /// - Iterator is valid input iterator
  ///
  /// Postconditions
  ///
  /// - p points to the last consumed character
  ///
  template <typename Iterator>
  static code_point decode(Iterator &p, Iterator e);

  ///
  /// Maximal width of valid sequence in the code units:
  ///
  /// - UTF-8  - 4
  /// - UTF-16 - 2
  /// - UTF-32 - 1
  ///
  static const size_t max_width;
  ///
  /// The width of specific code point in the code units.
  ///
  /// Requirement: value is a valid Unicode code point
  /// Returns value in range [1..max_width]
  ///
  static size_t width(code_point value);

  ///
  /// Get the size of the trail part of variable length encoded sequence.
  ///
  /// Returns -1 if C is not valid lead character
  ///
  static int trail_length(char_type c);
  ///
  /// Returns true if c is trail code unit, always false for UTF-32
  ///
  static bool is_trail(char_type c);
  ///
  /// Returns true if c is lead code unit, always true of UTF-32
  ///
  static bool is_lead(char_type c);

  ///
  /// Convert valid Unicode code point \a value to the UTF sequence.
  ///
  /// Requirements:
  ///
  /// - \a value is valid code point
  /// - \a out is an output iterator should be able to accept at least
  /// width(value) units
  ///
  /// Returns the iterator past the last written code unit.
  ///
  template <typename Iterator>
  static Iterator encode(code_point value, Iterator out);
  ///
  /// Decodes valid UTF sequence that is pointed by p into code point.
  ///
  /// If the sequence is invalid or points to end the behavior is undefined
  ///
  template <typename Iterator>
  static code_point decode_valid(Iterator &p);
};

#else  // DOXYGEN_DOCUMENTATION_BUILD

template <typename CharType, int size = sizeof(CharType)>
struct utf_traits;

template <typename CharType>
struct utf_traits<CharType, 1> {
  using char_type = CharType;

  static int trail_length(char_type ci) {
    auto c = static_cast<unsigned char>(ci);
    if (c < 128) {
      return 0;
    }
    if (NOWIDE_UNLIKELY(c < 194)) return -1;
    if (c < 224) {
      return 1;
    }
    if (c < 240) {
      return 2;
    }
    if (NOWIDE_LIKELY(c <= 244)) {
      return 3;
    }
    return -1;
  }

  static const std::size_t max_width = 4;

  static auto width(code_point value) -> std::size_t {
    if (value <= 0x7F) {
      return 1;
    }
    if (value <= 0x7FF) {
      return 2;
    }
    if (NOWIDE_LIKELY(value <= 0xFFFF)) {
      return 3;
    }
    return 4;
  }

  static auto is_trail(char_type ci) -> bool {
    auto c = static_cast<unsigned char>(ci);
    return (c & 0xC0) == 0x80;
  }

  static auto is_lead(char_type ci) -> bool { return !is_trail(ci); }

  template <typename Iterator>
  static auto decode(Iterator &p, Iterator e) -> code_point {
    if (NOWIDE_UNLIKELY(p == e)) {
      return incomplete;
    }

    auto lead = *p++;

    // First byte is fully validated here
    int trail_size = trail_length(lead);

    if (NOWIDE_UNLIKELY(trail_size < 0)) {
      return illegal;
    }

    //
    // Ok as only ASCII may be of size = 0
    // also optimize for ASCII text
    //
    if (trail_size == 0) {
      return static_cast<code_point>(lead);
    }

    code_point c = lead & ((1 << (6 - trail_size)) - 1);

    // Read the rest
    char tmp = 0;
    switch (trail_size) {
      case 3:
        if (NOWIDE_UNLIKELY(p == e)) {
          return incomplete;
        }
        tmp = *p++;
        if (!is_trail(tmp)) return illegal;
        c = (c << 6) | (tmp & 0x3F);
#if defined(__clang__)
        [[clang::fallthrough]];
#endif  // __clang__
        /* FALLTHRU */
      case 2:
        if (NOWIDE_UNLIKELY(p == e)) {
          return incomplete;
        }
        tmp = *p++;
        if (!is_trail(tmp)) {
          return illegal;
        }
        c = (c << 6) | (tmp & 0x3F);
#if defined(__clang__)
        [[clang::fallthrough]];
#endif  // __clang__
        /* FALLTHRU */
      case 1:
        if (NOWIDE_UNLIKELY(p == e)) {
          return incomplete;
        }
        tmp = *p++;
        if (!is_trail(tmp)) {
          return illegal;
        }
        c = (c << 6) | (tmp & 0x3F);
        break;
      default:
        HEDLEY_UNREACHABLE();
    }

    // Check code point validity: no surrogates and
    // valid range
    if (NOWIDE_UNLIKELY(!is_valid_codepoint(c))) {
      return illegal;
    }

    // make sure it is the most compact representation
    if (NOWIDE_UNLIKELY(width(c) != static_cast<std::size_t>(trail_size + 1))) {
      return illegal;
    }

    return c;
  }

  template <typename Iterator>
  static auto decode_valid(Iterator &p) -> code_point {
    unsigned char lead = *p++;
    if (lead < 192) {
      return lead;
    }

    int trail_size = 0;

    if (lead < 224) {
      trail_size = 1;
    } else if (NOWIDE_LIKELY(lead < 240)) {  // non-BMP rare
      trail_size = 2;
    } else {
      trail_size = 3;
    }

    code_point c = lead & ((1 << (6 - trail_size)) - 1);

    switch (trail_size) {  // NOLINT(hicpp-multiway-paths-covered)
      case 3:              // NOLINT(bugprone-branch-clone)
        c = (c << 6) | (static_cast<unsigned char>(*p++) & 0x3FU);
      case 2:
        c = (c << 6) | (static_cast<unsigned char>(*p++) & 0x3FU);
      case 1:
        c = (c << 6) | (static_cast<unsigned char>(*p++) & 0x3FU);
    }

    return c;
  }

  template <typename Iterator>
  static auto encode(code_point value, Iterator out) -> Iterator {
    if (value <= 0x7F) {
      *out++ = static_cast<char_type>(value);
    } else if (value <= 0x7FF) {
      *out++ = static_cast<char_type>((value >> 6) | 0xC0);
      *out++ = static_cast<char_type>((value & 0x3F) | 0x80);
    } else if (NOWIDE_LIKELY(value <= 0xFFFF)) {
      *out++ = static_cast<char_type>((value >> 12) | 0xE0);
      *out++ = static_cast<char_type>(((value >> 6) & 0x3F) | 0x80);
      *out++ = static_cast<char_type>((value & 0x3F) | 0x80);
    } else {
      *out++ = static_cast<char_type>((value >> 18) | 0xF0);
      *out++ = static_cast<char_type>(((value >> 12) & 0x3F) | 0x80);
      *out++ = static_cast<char_type>(((value >> 6) & 0x3F) | 0x80);
      *out++ = static_cast<char_type>((value & 0x3F) | 0x80);
    }
    return out;
  }
};  // utf8

template <typename CharType>
struct utf_traits<CharType, 2> {
  typedef CharType char_type;

  // See RFC 2781
  static bool is_first_surrogate(std::uint16_t x) {
    return 0xD800 <= x && x <= 0xDBFF;
  }
  static bool is_second_surrogate(std::uint16_t x) {
    return 0xDC00 <= x && x <= 0xDFFF;
  }
  static auto combine_surrogate(std::uint16_t w1, std::uint16_t w2)
      -> code_point {
    return ((code_point(w1 & 0x3FF) << 10) | (w2 & 0x3FF)) + 0x10000;
  }
  static int trail_length(char_type c) {
    if (is_first_surrogate(c)) {
      return 1;
    }
    if (is_second_surrogate(c)) {
      return -1;
    }
    return 0;
  }
  ///
  /// Returns true if c is trail code unit, always false for UTF-32
  ///
  static auto is_trail(char_type c) -> bool { return is_second_surrogate(c); }
  ///
  /// Returns true if c is lead code unit, always true of UTF-32
  ///
  static auto is_lead(char_type c) -> bool { return !is_second_surrogate(c); }

  template <typename It>
  static auto decode(It &current, It last) -> code_point {
    if (NOWIDE_UNLIKELY(current == last)) {
      return incomplete;
    }
    std::uint16_t w1 = *current++;
    if (NOWIDE_LIKELY(w1 < 0xD800 || 0xDFFF < w1)) {
      return w1;
    }
    if (w1 > 0xDBFF) {
      return illegal;
    }
    if (current == last) {
      return incomplete;
    }
    std::uint16_t w2 = *current++;
    if (w2 < 0xDC00 || 0xDFFF < w2) {
      return illegal;
    }
    return combine_surrogate(w1, w2);
  }
  template <typename It>
  static auto decode_valid(It &current) -> code_point {
    std::uint16_t w1 = *current++;
    if (NOWIDE_LIKELY(w1 < 0xD800 || 0xDFFF < w1)) {
      return w1;
    }
    std::uint16_t w2 = *current++;
    return combine_surrogate(w1, w2);
  }

  static const std::size_t max_width = 2;
  static auto width(code_point u) -> std::size_t {
    return u >= 0x10000 ? 2 : 1;
  }
  template <typename It>
  static auto encode(code_point u, It out) -> It {
    if (NOWIDE_LIKELY(u <= 0xFFFF)) {
      *out++ = static_cast<char_type>(u);
    } else {
      u -= 0x10000;
      *out++ = static_cast<char_type>(0xD800 | (u >> 10));
      *out++ = static_cast<char_type>(0xDC00 | (u & 0x3FF));
    }
    return out;
  }
};  // utf16;

template <typename CharType>
struct utf_traits<CharType, 4> {
  using char_type = CharType;
  static auto trail_length(char_type c) -> int {
    if (is_valid_codepoint(c)) {
      return 0;
    }
    return -1;
  }
  static auto is_trail(char_type /*c*/) -> bool { return false; }
  static auto is_lead(char_type /*c*/) -> bool { return true; }

  template <typename It>
  static auto decode_valid(It &current) -> code_point {
    return *current++;
  }

  template <typename It>
  static auto decode(It &current, It last) -> code_point {
    if (NOWIDE_UNLIKELY(current == last)) {
      return nowide::utf::incomplete;
    }
    auto c = static_cast<code_point>(*current++);
    if (NOWIDE_UNLIKELY(!is_valid_codepoint(c))) {
      return nowide::utf::illegal;
    }
    return c;
  }
  static const std::size_t max_width = 1;
  static auto width(code_point /*u*/) -> std::size_t { return 1; }
  template <typename It>
  static auto encode(code_point u, It out) -> It {
    *out++ = static_cast<char_type>(u);
    return out;
  }

};  // utf32

#endif  // DOXYGEN_DOCUMENTATION_BUILD

}  // namespace utf

}  // namespace nowide

HEDLEY_DIAGNOSTIC_POP
