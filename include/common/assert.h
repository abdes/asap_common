//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

/*!
 * @file assert.h
 *
 * @brief Macros to use assertions in the implementation code.
 */

#pragma once

#include <hedley/hedley.h>

#include <common/asap_common_api.h>
#include <common/config.h>

/*!
 * @def ASAP_UNREACHABLE()
 * @hideinitializer
 *
 * @brief Macro that can be used to indicate that the control flow will never
 * reach that point.
 *
 * This macro can be very useful to eliminate warnings by some compilers when
 * a switch statements covers all cases but does not provide a default case.
 * Consider for example the following switch statement that handles all the
 * possible values of the parameter \em col and would generate a warning from
 * some compilers because it has no default case.
 *
 * @code
 * enum class ColorChannel { RED, GREEN, BLUE };
 * void doSomething(ColorChannel col) {
 *   switch(col) {
 *     case ColorChannel::RED:
 *       // Do stuff for RED
 *       break;
 *     case ColorChannel::GREEN:
 *       // Do stuff for GREEN
 *       break;
 *     case ColorChannel::BLUE:
 *       // Do stuff for BLUE
 *       break;
 *     // There is no default case and we explicitly omit the default case so
 *     // that good compilers will warn about missing values from the switch
 *     // statement if we ever miss a possible value from the enum.
 *   }
 *   // The next statement will avoid some compilers to emit useless and
 *   // irrelevant warnings.
 *   ASAP_UNRACHABLE();
 * }
 * @endcode
 */
#define ASAP_UNREACHABLE() HEDLEY_UNREACHABLE()

/// @cond (INTERNAL_DETAIL)
// clang-format off
#if HEDLEY_GCC_VERSION
# define ASAP_FUNCTION __PRETTY_FUNCTION__
#else
# define ASAP_FUNCTION __FUNCTION__
#endif
// clang-format on
/// @endcond (INTERNAL_DETAIL)

namespace asap {

/*!
 * @brief Fail execution with an assertion described using the provided
 * information.
 *
 * This function is used internally in the assertion macros. It is not intended
 * to be used as-is. Use the macros instead.
 *
 * @param expr the expression that evaluated to false.
 * @param line the source code line number.
 * @param file the source code file name.
 * @param function the function name within which the assertion failed.
 * @param val a value to print for debug (nothing printed out if val is null).
 * @param kind if 1, indicates a precondition that failed, otherwise it's a
 * general assertion.
 */
void ASAP_COMMON_API assert_fail(const char *expr, int line, char const *file,
                                 char const *function, char const *val,
                                 int kind = 0);

}  // namespace asap

#if ASAP_USE_ASSERTS || defined(DOXYGEN_DOCUMENTATION_BUILD)

#if ASAP_USE_SYSTEM_ASSERTS && !defined(DOXYGEN_DOCUMENTATION_BUILD)

#include <cassert>
#define ASAP_ASSERT(a) assert(a)
#define ASAP_ASSERT_PRECOND(a) assert(a)
#define ASAP_ASSERT_VAL(a, x) assert(a)
#define ASAP_ASSERT_FAIL() assert(false)
#define ASAP_ASSERT_FAIL_VAL(x) assert(false)

#else  // !ASAP_USE_SYSTEM_ASSERTS
#include <sstream>

/// @cond (INTERNAL_DETAIL)
// This is to disable the warning of conditional expressions being constant
// in msvc.
// clang-format off
#if HEDLEY_MSVC_VERSION
#  define ASAP_WHILE_0  \
       __pragma( warning(push) ) \
       __pragma( warning(disable:4127) ) \
       while (false) \
       __pragma( warning(pop) )
#else
#  define ASAP_WHILE_0 while (false)
#endif
// clang-format on
/// @endcond (INTERNAL_DETAIL)

/*!
 * @hideinitializer
 * @brief Check the expression \em a and if it evaluates to 0 print an
 * assertion diagnostic message and abort the program.
 */
#define ASAP_ASSERT(a)                                                      \
  do {                                                                      \
    if (a) {                                                                \
    } else                                                                  \
      asap::assert_fail(#a, __LINE__, __FILE__, ASAP_FUNCTION, nullptr, 0); \
  }                                                                         \
  ASAP_WHILE_0

/*!
 * @hideinitializer
 * @brief Check the expression \em a and if it evaluates to 0 print an
 * assertion diagnostic message and abort the program.
 *
 * This assertion macro explicitly states that the expression must be true as
 * a precondition and its failure is most likely due to a logic or design
 * error in the program.
 */
#define ASAP_ASSERT_PRECOND(a)                                              \
  do {                                                                      \
    if (a) {                                                                \
    } else                                                                  \
      asap::assert_fail(#a, __LINE__, __FILE__, ASAP_FUNCTION, nullptr, 1); \
  }                                                                         \
  ASAP_WHILE_0

/*!
 * @hideinitializer
 * @brief Check the expression \em a and if it evaluates to 0 print an
 * assertion diagnostic message including the value \em x and abort the
 * program.
 */
#define ASAP_ASSERT_VAL(a, x)                                  \
  do {                                                         \
    if (a) {                                                   \
    } else {                                                   \
      std::stringstream __s__;                                 \
      __s__ << #x ": " << x;                                   \
      asap::assert_fail(#a, __LINE__, __FILE__, ASAP_FUNCTION, \
                        __s__.str().c_str(), 0);               \
    }                                                          \
  }                                                            \
  ASAP_WHILE_0

/*!
 * @hideinitializer
 * @brief Unconditionally fail, printing an assertion diagnostic message and
 * abort the program.
 */
#define ASAP_ASSERT_FAIL()                                                \
  asap::assert_fail("<unconditional>", __LINE__, __FILE__, ASAP_FUNCTION, \
                    nullptr, 0)

/*!
 * @hideinitializer
 * @brief Unconditionally fail, printing an assertion diagnostic message
 * including the value \em x and abort the program.
 */
#define ASAP_ASSERT_FAIL_VAL(x)                                             \
  do {                                                                      \
    std::stringstream __s__;                                                \
    __s__ << #x ": " << x;                                                  \
    asap::assert_fail("<unconditional>", __LINE__, __FILE__, ASAP_FUNCTION, \
                      __s__.str().c_str(), 0);                              \
  }                                                                         \
  ASAP_WHILE_0

#endif  // !ASAP_USE_SYSTEM_ASSERTS

#else  // !ASAP_USE_ASSERTS

#define ASAP_ASSERT(a) \
  do {                 \
  }                    \
  ASAP_WHILE_0
#define ASAP_ASSERT_PRECOND(a) \
  do {                         \
  }                            \
  ASAP_WHILE_0
#define ASAP_ASSERT_VAL(a, b) \
  do {                        \
  }                           \
  ASAP_WHILE_0
#define ASAP_ASSERT_FAIL() \
  do {                     \
  }                        \
  ASAP_WHILE_0
#define ASAP_ASSERT_FAIL_VAL(a) \
  do {                          \
  }                             \
  ASAP_WHILE_0

#endif  // ASAP_USE_ASSERTS
