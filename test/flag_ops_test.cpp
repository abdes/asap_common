//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <catch2/catch.hpp>

#include <common/flag_ops.h>

namespace asap {

TEST_CASE("Flag / Set", "[common][flag]") {
  unsigned long mask = 0x100010;
  unsigned long flag = 0x1000;

  FlagSet(mask, flag);
  // bit corresponding to flag is set
  REQUIRE((mask & flag) != 0);
  // other bits not touched
  REQUIRE(mask == 0x101010);

  // multi-bit flag
  mask = 0x100010;
  flag = 0x1001;
  FlagSet(mask, flag);
  // bit corresponding to flag is set
  REQUIRE(mask == 0x101011);
}

TEST_CASE("Flag / Clear", "[common][flag]") {
  unsigned long mask = 0x100010;
  unsigned long flag = 0x10;

  FlagClear(mask, flag);
  // bit corresponding to flag is cleared
  REQUIRE((mask & flag) == 0);
  // other bits not touched
  REQUIRE(mask == 0x100000);

  // multi-bit flag
  mask = 0x10101010;
  flag = 0x101000;
  FlagClear(mask, flag);
  REQUIRE((mask & flag) == 0);
  REQUIRE(mask == 0x10000010);
}

TEST_CASE("Flag / Flip", "[common][flag]") {
  unsigned long mask = 0x100010;
  unsigned long flag = 0x10;

  FlagFlip(mask, flag);
  // bit corresponding to flag is cleared
  REQUIRE((mask & flag) == 0);
  // other bits not touched
  REQUIRE(mask == 0x100000);
  FlagFlip(mask, flag);
  // bit corresponding to flag is cleared
  REQUIRE((mask & flag) != 0);
  // other bits not touched
  REQUIRE(mask == 0x100010);

  // multi-bit flag
  mask = 0x10101010;
  flag = 0x11111111;
  FlagFlip(mask, flag);
  REQUIRE(mask == 0x01010101);
  FlagFlip(mask, flag);
  REQUIRE(mask == 0x10101010);
}

TEST_CASE("Flag / Test", "[common][flag]") {
  unsigned long mask = 0x100010;
  unsigned long flag = 0x10;

  REQUIRE(FlagTest(mask, flag));
  REQUIRE(FlagTest(mask, mask));
}

}  // namespace asap
