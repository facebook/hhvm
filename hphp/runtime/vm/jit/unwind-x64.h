/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_VM_TRANSLATOR_UNWIND_X64_H_
#define incl_HPHP_VM_TRANSLATOR_UNWIND_X64_H_

#include <cstdlib>
#include <sstream>
#include <string>
#include <unwind.h>
#include <boost/shared_ptr.hpp>

#include "hphp/util/assertions.h"
#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/tread_hash_map.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/util/asm-x64.h"
#include "hphp/runtime/vm/jit/runtime-type.h"

namespace HPHP { namespace Transl {

//////////////////////////////////////////////////////////////////////

typedef TreadHashMap<CTCA, TCA, ctca_identity_hash> CatchTraceMap;

//////////////////////////////////////////////////////////////////////

inline const std::type_info& typeInfoFromUnwindException(
  _Unwind_Exception* exceptionObj)
{
  constexpr size_t kTypeInfoOff = 112;
  return **reinterpret_cast<std::type_info**>(
    reinterpret_cast<char*>(exceptionObj + 1) - kTypeInfoOff);
}

inline std::exception* exceptionFromUnwindException(
  _Unwind_Exception* exceptionObj)
{
  return reinterpret_cast<std::exception*>(exceptionObj + 1);
}

/*
 * Called whenever we create a new translation cache for the whole
 * region of code.
 */
typedef boost::shared_ptr<void> UnwindInfoHandle;
UnwindInfoHandle register_unwind_region(unsigned char* address, size_t size);

//////////////////////////////////////////////////////////////////////

}}

#endif
