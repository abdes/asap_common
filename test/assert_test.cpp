//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <common/assert.h>

#include <catch2/catch.hpp>

// Test cases for assertions are mainly to check that the include header
// compiles properly. Catch2 is not able to do death tests yet.

TEST_CASE("TestAssertion", "[common][assert]") {
  ASAP_ASSERT(true);
  ASAP_ASSERT_PRECOND(true);
  ASAP_ASSERT_VAL(true, 1);
}
