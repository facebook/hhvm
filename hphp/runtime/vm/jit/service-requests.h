/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP {

struct ActRec;

namespace jit {

struct CGMeta;

///////////////////////////////////////////////////////////////////////////////

namespace svcreq {

///////////////////////////////////////////////////////////////////////////////

/*
 * A service request stub is a tiny routine used to invoke JIT translation
 * services, used in place of the actual translation. Incoming branches to
 * service request stubs are smashable and tracked by SrcDB.
 *
 * Stubs, first, set up enough context to reconstruct the SrcKey and stack
 * positions, then, jump to the corresponding unique stub to perform the
 * requested operation.
 */
enum class StubType : uint8_t {
  // A request to translate the code at the given current location.
  // See svcreq::handleTranslate() for more details.
  Translate,

  // A request to retranslate the code at the given current location.
  // See svcreq::handleRetranslate() for more details.
  Retranslate,
};

/*
 * Get an existing stub or emit a new one to perform request of the given type
 * at the given code and stack position. The same stub may be compatible with
 * multiple different positions.
 *
 * Returns nullptr on failure, e.g. when there is no more TC space available.
 */
TCA getOrEmitStub(StubType type, SrcKey sk, SBInvOffset spOff);

///////////////////////////////////////////////////////////////////////////////
// Emitters.

/*
 * Emit a stub which syncs vmsp and vmpc and then calls
 * resumeHelperNoTranslate. Call are smashed to this when we know we
 * can no longer create new translations. The address of the stub is
 * returned if successful, nullptr otherwise (the creation can fail if
 * there's no space in the TC for it).
 */
TCA emit_interp_no_translate_stub(SBInvOffset spOff, SrcKey sk);

///////////////////////////////////////////////////////////////////////////////

}}}
