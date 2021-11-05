//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#if defined(__clang__)
#pragma clang diagnostic push
// Catch2 uses a lot of macro names that will make clang go crazy
#pragma clang diagnostic ignored "-Wreserved-identifier"
// Big mess created because of the way spdlog is organizing its source code
// based on header only builds vs library builds. The issue is that spdlog
// places the template definitions in a separate file and explicitly
// instantiates them, so we have no problem at link, but we do have a problem
// with clang (rightfully) complaining that the template definitions are not
// available when the template needs to be instantiated here.
#pragma clang diagnostic ignored "-Wundefined-func-template"
#endif  // __clang__

#include <common/assert.h>

#include <catch2/catch.hpp>

// Test cases for assertions are mainly to check that the include header
// compiles properly. Catch2 is not able to do death tests yet.

TEST_CASE("TestAssertion", "[common][assert]") {
  ASAP_ASSERT(true);
  ASAP_ASSERT_PRECOND(true);
  ASAP_ASSERT_VAL(true, 1);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif  // __clang__
