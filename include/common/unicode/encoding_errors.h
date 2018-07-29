//
//  Copyright (c) 2009-2011 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <stdexcept>

namespace nowide {
namespace conv {
///
/// \addtogroup codepage
///
/// @{

///
/// \brief The excepton that is thrown in case of conversion error
///
class conversion_error : public std::runtime_error {
 public:
  conversion_error() : std::runtime_error("Conversion failed") {}
};

/// @}

}  // namespace conv
}  // namespace nowide
