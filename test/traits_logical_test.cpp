//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <catch2/catch.hpp>

#include <common/traits/logical.h>
#include <asap/asap-features.h>

using asap::conjunction;
using asap::disjunction;
using asap::negation;

#if ASAP_COMPILER_IS_GNU
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#elif ASAP_COMPILER_IS_Clang
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-local-typedefs"
#endif

TEST_CASE("Conjunction_typedef", "[common][traits][logical]") {
  // Check for required typedefs
  typedef conjunction<std::true_type, std::true_type> test_type;
  typedef test_type::value_type value_type;
  typedef test_type::type type;
  typedef test_type::type::value_type type_value_type;
  typedef test_type::type::type type_type;
}

TEST_CASE("Disjunction_typedef", "[common][traits][logical]") {
  // Check for required typedefs
  typedef disjunction<std::false_type, std::true_type> test_type;
  typedef test_type::value_type value_type;
  typedef test_type::type type;
  typedef test_type::type::value_type type_value_type;
  typedef test_type::type::type type_type;
}

TEST_CASE("Negation_typedef", "[common][traits][logical]") {
  // Check for required typedefs
  typedef negation<std::false_type> test_type;
  typedef test_type::value_type value_type;
  typedef test_type::type type;
  typedef test_type::type::value_type type_value_type;
  typedef test_type::type::type type_type;
}

TEST_CASE("Value", "[common][traits][logical]") {
  REQUIRE(negation<std::false_type>{});
  REQUIRE(!negation<std::true_type>{});
  REQUIRE(conjunction<>{});
  REQUIRE(!disjunction<>{});
  REQUIRE(conjunction<std::true_type>{});
  REQUIRE(!conjunction<std::false_type>{});
  REQUIRE(disjunction<std::true_type>{});
  REQUIRE(!disjunction<std::false_type>{});
  REQUIRE(conjunction<std::true_type, std::true_type>{});
  REQUIRE(!conjunction<std::true_type, std::false_type>{});
  REQUIRE(disjunction<std::false_type, std::true_type>{});
  REQUIRE(!disjunction<std::false_type, std::false_type>{});
  REQUIRE(conjunction<std::true_type, std::true_type, std::true_type>{});
  REQUIRE(!conjunction<std::true_type, std::true_type, std::false_type>{});
  REQUIRE(disjunction<std::false_type, std::false_type, std::true_type>{});
  REQUIRE(!disjunction<std::false_type, std::false_type, std::false_type>{});
}

#if ASAP_COMPILER_IS_GNU
#pragma GCC diagnostic pop
#elif ASAP_COMPILER_IS_Clang
#pragma clang diagnostic pop
#endif
