/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ACT_REC_H_
#define incl_HPHP_ACT_REC_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/typed-value.h"
/*
 * These header dependencies need to stay as minimal as possible.
 */

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct ActRec;
struct Class;
struct ExtraArgs;
struct Func;
struct ObjectData;
struct StringData;
struct Unit;
struct VarEnv;

///////////////////////////////////////////////////////////////////////////////

/*
 * An "ActRec" is a call activation record.  The ordering of the fields assumes
 * that stacks grow toward lower addresses.
 *
 * For most purposes, an ActRec can be considered to be in one of three
 * possible states:
 *   Pre-live:
 *     After the FPush* instruction which materialized the ActRec on the stack
 *     but before the corresponding FCall instruction.
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
  union {
    // This pair of uint64_t's must be the first two elements in the structure
    // so that the pointer to the ActRec can also be used for RBP chaining.
    // Note that ActRecs are also native frames, so this is an implicit machine
    // dependency.
    TypedValue _dummyA;
    struct {
#if defined(__powerpc64__)
      ActRec* m_sfp;         // Previous hardware frame pointer/ActRec.
      uint32_t m_savedCR;    // PPC64's sign flags (CR)
      uint32_t m_reserved;   // Reserved word as on ABI
      uint64_t m_savedRip;   // In-TC address to return to.
      uint64_t m_savedToc;   // TOC save doubleword
#else // X64 style
      // Previous ActRec (or hardware frame pointer, for reentry frames).
      ActRec* m_sfp;
      // In-TC address to return to.
      uint64_t m_savedRip;
#endif
    };
  };
  union {
    TypedValue _dummyB;
    struct {
      // Function.
      const Func* m_func;
      // Saved offset of caller from beginning of the caller Func's bytecode.
      uint32_t m_soff;
      // Bits 0-27 are the number of function args.  Bits 28-31 are values from
      // the Flags enum.
      uint32_t m_numArgsAndFlags;
    };
  };
  union {
    // Return value is teleported here when the ActRec is post-live.
    TypedValue m_r;
    struct {
      union {
        ObjectData* m_thisUnsafe; // This.
        Class* m_clsUnsafe;       // Late bound class.
      };
      union {
        // Variable environment; only used when the ActRec is live.
        VarEnv* m_varEnv;
        // Lightweight extra args; only used when the ActRec is live.
        ExtraArgs* m_extraArgs;
        // Invoked function name, used for __call().  Only used when the ActRec
        // is pre-live.
        StringData* m_invName;
      };
    };
  };

  /////////////////////////////////////////////////////////////////////////////

  enum Flags : uint32_t {
    None          = 0,

    // In non-HH files the caller can specify whether param type-checking
    // should be strict or weak.
    UseWeakTypes = (1u << 28),

    // This bit can be independently set on ActRecs with any other flag state.
    // It's used by the unwinder to know that an ActRec has been partially torn
    // down (locals freed).
    LocalsDecRefd = (1u << 29),

    // Four mutually exclusive execution mode states in these 2 bits.
    InResumed     = (1u << 30),
    IsFCallAwait  = (1u << 31),
    MagicDispatch = InResumed|IsFCallAwait,
    // MayNeedStaticWaitHandle, if neither bit is set.
  };

  static constexpr int kNumArgsBits = 28;
  static constexpr int kNumArgsMask = (1 << kNumArgsBits) - 1;
  static constexpr int kFlagsMask = ~kNumArgsMask;
  static constexpr int kExecutionModeMask = ~(LocalsDecRefd | UseWeakTypes);

  /*
   * To conserve space, we use unions for pairs of mutually exclusive fields
   * (fields that are not used at the same time).
   *
   * The least significant bit is used as a marker for each pair of fields so
   * that we can distinguish at runtime which field is valid.  We define
   * accessors (below) to encapsulate this logic.
   */
  static auto constexpr      kHasClassBit  = 0x1;  // unset for m_this
  static auto constexpr      kExtraArgsBit = 0x1;  // unset for m_varEnv

  static constexpr uintptr_t kTrashedVarEnvSlot = 0xfeeefeee000f000f;
  static constexpr uintptr_t kTrashedThisSlot = 0xfeeefeeef00fe00e;

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

  /*
   * Set up frame linkage with the caller ActRec.
   */
  void setReturn(ActRec* fp, PC pc, void* retAddr);
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

  /////////////////////////////////////////////////////////////////////////////
  // NumArgs / Flags.

  /*
   * Number of arguments passed for this invocation.
   */
  int32_t numArgs() const;

  /*
   * Raw flags accessors.
   */
  Flags flags() const;
  bool useWeakTypes() const;
  bool localsDecRefd() const;
  bool resumed() const;
  bool isFCallAwait() const;
  bool mayNeedStaticWaitHandle() const;
  bool magicDispatch() const;

  /*
   * Pack `numArgs' and `flags' into the format expected by m_numArgsAndFlags.
   */
  static uint32_t encodeNumArgsAndFlags(uint32_t numArgs, Flags flags);

  /*
   * Set the numArgs component of m_numArgsAndFlags to `numArgs'.
   *
   * The init* flavor zeroes the flags component, whereas the set* flavor
   * preserves flags.
   */
  void initNumArgs(uint32_t numArgs);
  void setNumArgs(uint32_t numArgs);

  /*
   * Flags setters.
   */
  void setUseWeakTypes();
  void setLocalsDecRefd();
  void setResumed();
  void setFCallAwait();

  /*
   * Set or clear both m_invName and the MagicDispatch flag.
   */
  void setMagicDispatch(StringData* invName);
  StringData* clearMagicDispatch();

  /////////////////////////////////////////////////////////////////////////////
  // This / Class.

  /*
   * Encode `obj' or `cls' for the m_this/m_cls union.
   */
  static void* encodeThis(ObjectData* obj);
  static void* encodeClass(const Class* cls);

  /*
   * Determine whether p is a Class* or an ObjectData* based
   * on kHasClassBit.
   *
   * @requires: p != nullptr
   */
  static bool checkThis(void* p);

  /*
   * Determine whether p is a Class* based on kHasClassBit.
   *
   * @requires: p is a Cctx, an ObjectData* or a nullptr
   */
  static bool checkThisOrNull(void* p);

  /*
   * Decode `p', encoded in the format of m_this/m_cls.
   *
   * If `p' has the other encoding (or is nullptr), return nullptr.
   */
  static ObjectData* decodeThis(void* p);
  static Class* decodeClass(void* p);

  /*
   * Set m_this/m_cls to the pre-encoded `objOrCls'.
   *
   * One of these asserts if `objOrCls' is null, but it is a mystery which one.
   */
  void setThisOrClass(void* objOrCls);
  void setThisOrClassAllowNull(void* objOrCls);

  /*
   * Whether the m_this/m_cls union is discriminated in the desired way.
   *
   * @requires: m_func->implCls() != nullptr
   */
  bool hasThis() const;
  bool hasClass() const;

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
   * Get and decode the value of m_this/m_cls.
   *
   * @requires: hasThis() or hasClass(), respectively
   */
  ObjectData* getThis() const;
  Class* getClass() const;

  /*
   * Encode and set `val' to m_this/m_cls
   *
   * @requires: m_func->implClass() and
   *            !m_func->isStaticInProlog()
   */
  void setThis(ObjectData* val);
  /*
   * Encode and set `val' to m_this/m_cls
   *
   * @requires: m_func->implClass() and
   *            !(m_func->attrs() & AttrRequiresThis)
   */
  void setClass(Class* val);

  /*
   * Write garbage to the m_this/m_cls union (in debug mode only).
   */
  void trashThis();

  /////////////////////////////////////////////////////////////////////////////
  // VarEnv / ExtraArgs.

  /*
   * Write garbage to the m_varEnv/m_extraArgs union (in debug mode only).
   */
  void trashVarEnv();

  /*
   * Check that the m_varEnv/m_extraArgs union is not the special garbage
   * value.
   */
  bool checkVarEnv() const;

  /*
   * Whether the m_varEnv/m_extraArgs union is discriminated in the desired
   * way.
   */
  bool hasVarEnv() const;
  bool hasExtraArgs() const;

  /*
   * Get and decode the VarEnv.
   *
   * @requires: hasVarEnv()
   */
  VarEnv* getVarEnv() const;

  /*
   * Get and decode the ExtraArgs.
   *
   * If !hasExtraArgs(), returns nullptr.
   */
  ExtraArgs* getExtraArgs() const;

  /*
   * Get and decode the magic invocation name.
   *
   * @requires: magicDispatch()
   */
  StringData* getInvName() const;

  /*
   * Encode and set `val' to the m_varEnv/m_extraArgs union.
   */
  void setVarEnv(VarEnv* val);
  void setExtraArgs(ExtraArgs* val);

  /*
   * Get the extra argument with index `ind', from either the VarEnv or the
   * ExtraArgs, whichever is set.
   *
   * Returns nullptr if there are no extra arguments.
   */
  TypedValue* getExtraArg(unsigned ind) const;
};

static_assert(offsetof(ActRec, m_sfp) == 0,
              "m_sfp should be at offset 0 of ActRec");

/*
 * Size in bytes of the target architecture's call frame.
 */
constexpr auto kNativeFrameSize = offsetof(ActRec, _dummyB);

/*
 * Whether `address' is a helper stub that we're permitted to set
 * ActRec::m_savedRip to.
 */
bool isReturnHelper(void* address);
bool isDebuggerReturnHelper(void* address);

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/vm/act-rec-inl.h"

#endif
