//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#if defined(__clang__)
#pragma clang diagnostic push
// Catch2 uses a lot of macro names that will make clang go crazy
#if (__clang_major__ >= 13) && !defined(__APPLE__)
#pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
// Big mess created because of the way spdlog is organizing its source code
// based on header only builds vs library builds. The issue is that spdlog
// places the template definitions in a separate file and explicitly
// instantiates them, so we have no problem at link, but we do have a problem
// with clang (rightfully) complaining that the template definitions are not
// available when the template needs to be instantiated here.
#pragma clang diagnostic ignored "-Wundefined-func-template"
#endif // __clang__

#include <common/unicode/convert.h>

#include <catch2/catch.hpp>

TEST_CASE("Unicode / nowide / widen", "[common][unicode][nowide]") {
  const std::string hello = "\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d";
  const std::wstring whello = L"\u05e9\u05dc\u05d5\u05dd";

  char const *b = hello.c_str();
  char const *e = b + 8;
  wchar_t buf[6] = {0, 0, 0, 0, 0, 1};

  REQUIRE(nowide::widen(buf, 5, b, e) == buf);

  REQUIRE(buf == whello);
  REQUIRE(buf[5] == 1);
  REQUIRE(nowide::widen(buf, 4, b, e) == nullptr);
  REQUIRE(nowide::widen(buf, 5, b, e - 1) == nullptr);
  REQUIRE(nowide::widen(buf, 5, b, e - 2) == buf);

  REQUIRE(nowide::widen(buf, 5, b, b) == buf);
  REQUIRE(buf[0] == 0);

  REQUIRE(nowide::widen(buf, 5, b, b + 2) == buf);
  REQUIRE(buf[1] == 0);
  REQUIRE(buf[0] == whello[0]);

  b = "\xFF\xFF";
  e = b + 2;
  REQUIRE(nowide::widen(buf, 5, b, e) == nullptr);

  b = "\xd7\xa9\xFF";
  e = b + 3;
  REQUIRE(nowide::widen(buf, 5, b, e) == nullptr);
  REQUIRE(nowide::widen(buf, 5, b, b + 1) == nullptr);
}

TEST_CASE("Unicode / nowide / narrow", "[common][unicode][nowide]") {
  const std::string hello = "\xd7\xa9\xd7\x9c\xd7\x95\xd7\x9d";
  const std::wstring whello = L"\u05e9\u05dc\u05d5\u05dd";

  wchar_t const *b = whello.c_str();
  wchar_t const *e = b + 4;
  char buf[10] = {0};
  buf[9] = 1;
  REQUIRE(nowide::narrow(buf, 9, b, e) == buf);
  REQUIRE(buf == hello);
  REQUIRE(buf[9] == 1);
  REQUIRE(nowide::narrow(buf, 8, b, e) == nullptr);
  REQUIRE(nowide::narrow(buf, 7, b, e - 1) == buf);
  REQUIRE(buf == hello.substr(0, 6));
}

TEST_CASE("Unicode / nowide / strings", "[common][unicode][nowide]") {
  char buf[3];
  wchar_t wbuf[3];
  REQUIRE(nowide::narrow(buf, 3, L"xy") == std::string("xy"));
  REQUIRE(nowide::widen(wbuf, 3, "xy") == std::wstring(L"xy"));
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif // __clang__
