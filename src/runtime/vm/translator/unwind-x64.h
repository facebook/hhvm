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

#ifndef incl_VM_TRANSLATOR_UNWIND_X64_H_
#define incl_VM_TRANSLATOR_UNWIND_X64_H_

#include <cstdlib>
#include <sstream>
#include <string>
#include <boost/shared_ptr.hpp>

#include "util/assertions.h"
#include "runtime/base/types.h"
#include "runtime/vm/translator/types.h"
#include "runtime/vm/tread_hash_map.h"
#include "runtime/vm/translator/types.h"
#include "util/asm-x64.h"
#include "runtime/vm/translator/runtime-type.h"

namespace HPHP { namespace VM { namespace Transl {

//////////////////////////////////////////////////////////////////////

/*
 * Information about callee-saved registers that was dirty at a
 * callsite.  This should be registered during translation time so the
 * unwinder can see it.
 */
struct UnwindRegInfo {
  static const int kMaxCalleeSaved = 3;

  struct Data {
    bool     dirty     : 1;
    bool     exStack   : 1;  // false means a local
    signed   dataType  : 8;  // DataType values
    unsigned reg       : 4;  // Register id
    int16_t  locOffset;

    std::string pretty() const;
  };
  static_assert(sizeof(Data) == sizeof(uint32_t),
                "UnwindRegInfo::Data was too big");

  explicit UnwindRegInfo();

  void add(RegNumber reg, DataType type, Location loc);
  void clear();
  bool empty() const;

  Data m_regs[kMaxCalleeSaved];
};

static_assert(sizeof(UnwindRegInfo) <= sizeof(uint64_t) * 2,
              "Unexpected size for UnwindRegInfo");

typedef TreadHashMap<CTCA,UnwindRegInfo,ctca_identity_hash> UnwindRegMap;

//////////////////////////////////////////////////////////////////////

/*
 * Called whenever we create a new translation cache for the whole
 * region of code.
 */
typedef boost::shared_ptr<void> UnwindInfoHandle;
UnwindInfoHandle register_unwind_region(unsigned char* address, size_t size);

//////////////////////////////////////////////////////////////////////

}}}

#endif
