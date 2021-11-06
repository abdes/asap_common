//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <common/non_copiable.h>

namespace asap {

// We can't optimize this out to use the default generated constructor by the
// compiler due to DLL export symbols. Classes extending NonCopiable in other
// libraries will need to see this symbol.
//
// NOLINTNEXTLINE
NonCopiable::NonCopiable() {}

// We can't optimize this out to use the default generated constructor by the
// compiler due to DLL export symbols. Classes extending NonCopiable in other
// libraries will need to see this symbol.
//
// NOLINTNEXTLINE
NonCopiable::~NonCopiable() {}

}  // namespace asap
