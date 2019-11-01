//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#pragma once

namespace asap {

template <typename T>
inline void FlagSet(T &mask, T flag) {
  mask |= flag;
}

template <typename T>
inline void FlagClear(T &mask, T flag) {
  mask &= ~flag;
}

template <typename T>
inline void FlagFlip(T &mask, T flag) {
  mask ^= flag;
}

template <typename T>
inline bool FlagTest(T &mask, T flag) {
  return (mask & flag) == flag;
}

}  // namespace asap