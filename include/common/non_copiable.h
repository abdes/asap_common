//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#pragma once

#include <common/asap_common_api.h>

namespace asap {

/*!
 * NonCopiable is intended to be used as a \b private base class. It has deleted
 * (C++11) copy constructor and copy assignment operator and can't be copied
 * or assigned; a class that derives from it inherits these properties.
 */
class ASAP_COMMON_API NonCopiable {
 public:
  /// Copy constructor (deleted)
  NonCopiable(const NonCopiable &) = delete;
  /// Copy assignment operator (deleted)
  NonCopiable &operator=(const NonCopiable &) = delete;

 protected:
  /*
   * Class NonCopiable has protected constructor and destructor members to
   * emphasize that it is to be used only as a base class.
   * With C++2011, using an optimized and trivial constructor and similar
   * destructor can be enforced by declaring both and marking them default.
   */
  /// Default constructor (default)
  NonCopiable() = default;
  /// Destructor (default)
  virtual ~NonCopiable() = default;
};

}  // namespace asap
