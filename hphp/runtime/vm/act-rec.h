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

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/coeffects.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/util/compact-tagged-ptrs.h"

/*
 * These header dependencies need to stay as minimal as possible.
 */

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ActRec;
struct Class;
struct Func;
struct ObjectData;
struct StringData;
struct Unit;

///////////////////////////////////////////////////////////////////////////////

/*
 * An "ActRec" is a call activation record.  The ordering of the fields assumes
 * that stacks grow toward lower addresses.
 *
 * For most purposes, an ActRec can be considered to be in one of three
 * possible states:
 *   Pre-live:
 *     In the middle of the FCall instruction and function prologues, when the
 *     ActRec is materialized on the stack, but the rest of the frame (such as
 *     locals) is not yet initialized.
 *   Live:
 *     After the corresponding FCall instruction but before the ActRec fields
 *     and locals/iters have been decref'd (either by return or unwinding).
 *   Post-live:
 *     After the ActRec fields and locals/iters have been decref'd.
 *
 * Note that when a function is invoked by the runtime via invokeFunc(), the
 * "pre-live" state is skipped and the ActRec is materialized in the "live"
 * state.
 */
struct ActRec {
  // This pair of uint64_t's must be the first two elements in the structure
  // so that the pointer to the ActRec can also be used for RBP chaining.
  // Note that ActRecs are also native frames, so this is an implicit machine
  // dependency.
  ActRec* m_sfp;       // Previous hardware frame pointer/ActRec
  uint64_t m_savedRip; // native (in-TC) return address
  FuncId m_funcId;
  // bit 0: LocalsDecRefd
  // bit 1: AsyncEagerRet
  // bits 2-31: bc offset of call opcode from caller func entry
  uint32_t m_callOffAndFlags;
  union {
    ObjectData* m_thisUnsafe; // This.
    Class* m_clsUnsafe;       // Late bound class.
  };

  TYPE_SCAN_CUSTOM_FIELD(m_thisUnsafe) {
    if (func()->implCls()) scanner.scan(m_thisUnsafe);
  }

  /////////////////////////////////////////////////////////////////////////////

  enum Flags : uint32_t {
    // This bit can be independently set on ActRecs with any other flag state.
    // It's used by the unwinder to know that an ActRec has been partially torn
    // down (locals freed).
    LocalsDecRefd,

    // Async eager return was requested.
    AsyncEagerRet,

    // The first bit of the call offset.
    CallOffsetStart,
  };

  /*
   * To conserve space, we use unions for pairs of mutually exclusive fields
   * (fields that are not used at the same time).
   *
   * The least significant bit is used as a marker for each pair of fields so
   * that we can distinguish at runtime which field is valid.  We define
   * accessors (below) to encapsulate this logic.
   */

  static constexpr uintptr_t kTrashedThisSlot = 0xfeeefeeef00fe00e;
  static constexpr uintptr_t kTrashedFuncSlot = 0xfeeefeeef00fe00d;

  /////////////////////////////////////////////////////////////////////////////

  /*
   * The next outermost VM frame, or nullptr if this is a reentry frame.
   */
  ActRec* sfp() const;

  /*
   * The Func and Unit for this frame.
   */
  const Func* func() const;
  const Unit* unit() const;

  void setFunc(const Func*);

  /*
   * Set up frame linkage with the caller ActRec.
   */
  void setReturn(ActRec* fp, PC callPC, void* retAddr, bool asyncEagerReturn);
  void setJitReturn(void* retAddr);

  /*
   * Hijack the frame such that a PHP return will cause us to leave the current
   * VM nesting layer.
   */
  void setReturnVMExit();

  /*
   * Whether this frame should be skipped when searching for context.
   *
   * @returns: func() && func()->isSkipFrame().
   */
  bool skipFrame() const;

  /*
   * Whether this frame is inlined.
   */
  bool isInlined() const;

  /////////////////////////////////////////////////////////////////////////////
  // Flags, call offset, number of args.

  /*
   * Raw flags accessors.
   */
  bool localsDecRefd() const;
  bool isAsyncEagerReturn() const;

  /*
   * BC offset of call opcode from caller func entry.
   */
  Offset callOffset() const;

  /*
   * Initialize call offset. Assumes flags were not written yet.
   */
  void initCallOffset(Offset offset);

  /*
   * Combine offset with flags.
   */
  static uint32_t encodeCallOffsetAndFlags(Offset offset, uint32_t flags);

  /*
   * Flags setters.
   */
  void setLocalsDecRefd();

  /////////////////////////////////////////////////////////////////////////////
  // This / Class.

  /*
   * Set m_this/m_cls to the pre-encoded `objOrCls'.
   *
   * One of these asserts if `objOrCls' is null, but it is a mystery which one.
   */
  void setThisOrClass(void* objOrCls);
  void setThisOrClassAllowNull(void* objOrCls);

  /*
   * Whether the m_this/m_cls union is discriminated in the desired way in the
   * function body.
   *
   * @requires: m_func->implCls() != nullptr
   */
  bool hasThis() const;
  bool hasClass() const;

  /*
   * Whether the m_this/m_cls union is discriminated in the desired way in the
   * function prologue.
   *
   * @requires: m_func->implCls() != nullptr
   */
  bool hasThisInPrologue() const;
  bool hasClassInPrologue() const;

  /*
   * Get the (encoded) value of the m_this/m_cls union.
   *
   * @requires: hasThis() || hasClass()
   */
  void* getThisOrClass() const;

  /*
   * Get m_thisUnsafe. Caller takes responsibility for its meaning.
   */
  ObjectData* getThisUnsafe() const;

  /*
   * Get and decode the value of m_this/m_cls in the function body.
   *
   * @requires: hasThis() or hasClass(), respectively
   */
  ObjectData* getThis() const;
  Class* getClass() const;

  /*
   * Get and decode the value of m_this/m_cls in the prologue.
   *
   * @requires: hasThis() or hasClass(), respectively
   */
  ObjectData* getThisInPrologue() const;
  Class* getClassInPrologue() const;

  /*
   * Encode and set `val' to m_this/m_cls
   *
   * @requires: m_func->implClass() and
   *            !m_func->isStaticInPrologue()
   */
  void setThis(ObjectData* val);
  /*
   * Encode and set `val' to m_this/m_cls
   *
   * @requires: m_func->implClass() and m_func->isStaticInPrologue()
   */
  void setClass(Class* val);

  /*
   * Write garbage to the m_this/m_cls union (in debug mode only).
   */
  void trashThis();

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Returns the ambient and required coeffects of the function respectively
   */
  RuntimeCoeffects coeffects() const;
  RuntimeCoeffects requiredCoeffects() const;
  RuntimeCoeffects providedCoeffectsForCall(bool isCtor) const;

  /*
   * address to teleport the return value after destroying this actrec.
   */
  TypedValue* retSlot() {
    return reinterpret_cast<TypedValue*>(this + 1) - 1;
  }
};

static_assert(offsetof(ActRec, m_sfp) == 0,
              "m_sfp should be at offset 0 of ActRec");

/*
 * Size in bytes of the target architecture's call frame.
 */
constexpr auto kNativeFrameSize = offsetof(ActRec, m_funcId);
static_assert(kNativeFrameSize % sizeof(TypedValue) == 0, "");

/*
 * offset from frame ptr to return value slot after teardown
 */
constexpr auto kArRetOff = sizeof(ActRec) - sizeof(TypedValue);
static_assert(kArRetOff % sizeof(TypedValue) == 0, "");

/*
 * Whether `address' is a helper stub that we're permitted to set
 * ActRec::m_savedRip to.
 */
bool isReturnHelper(void* address);

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/vm/act-rec-inl.h"

