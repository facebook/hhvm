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
#if defined(__powerpc64__)
  ActRec* m_sfp;         // Previous hardware frame pointer/ActRec.
  uint32_t m_savedCR;    // PPC64's sign flags (CR)
  uint32_t m_reserved;   // Reserved word as on ABI
  uint64_t m_savedRip;   // In-TC address to return to.
  uint64_t m_savedToc;   // TOC save doubleword
#else // X64 style
  // This pair of uint64_t's must be the first two elements in the structure
  // so that the pointer to the ActRec can also be used for RBP chaining.
  // Note that ActRecs are also native frames, so this is an implicit machine
  // dependency.
  ActRec* m_sfp;       // Previous hardware frame pointer/ActRec
  uint64_t m_savedRip; // native (in-TC) return address
#endif
  const Func* m_func;  // Function.
  uint32_t m_soff;     // offset of caller from its bytecode start.
  uint32_t m_numArgsAndFlags; // arg_count:28, flags:4
  union {
    ObjectData* m_thisUnsafe; // This.
    Class* m_clsUnsafe;       // Late bound class.
  };
  union {
    VarEnv* m_varEnv;       // Variable environment when live
    ExtraArgs* m_extraArgs; // Lightweight extra args, when live
    StringData* m_invName;  // Invoked name, used for __call(), when pre-live
  };

  TYPE_SCAN_CUSTOM_FIELD(m_thisUnsafe) {
    // skip if "this" is a class
    if (checkThisOrNull(m_thisUnsafe)) scanner.scan(m_thisUnsafe);
  }
  TYPE_SCAN_CUSTOM_FIELD(m_varEnv) {
    // All three union members could be heap pointers, but we don't care
    // which kind; PtrMap will resolve things.
    scanner.scan(m_varEnv);
  }

  /////////////////////////////////////////////////////////////////////////////

  enum Flags : uint32_t {
    None          = 0,

    // Set if the function was called using FCall instruction with more
    // than one return values and must return a value via RetM.
    MultiReturn = (1u << 26),

    // Set if this corresponds to a dynamic call
    DynamicCall = (1u << 27),

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

  static constexpr int kNumArgsBits = 26;
  static constexpr int kNumArgsMask = (1 << kNumArgsBits) - 1;
  static constexpr int kFlagsMask = ~kNumArgsMask;
  static constexpr int kExecutionModeMask =
    ~(LocalsDecRefd | DynamicCall | MultiReturn);

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
  bool localsDecRefd() const;
  bool resumed() const;
  bool isFCallAwait() const;
  bool mayNeedStaticWaitHandle() const;
  bool magicDispatch() const;
  bool isDynamicCall() const;
  bool isFCallM() const;

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
  void setLocalsDecRefd();
  void setResumed();
  void setFCallAwait();
  void setDynamicCall();
  void setFCallM();

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
   *            !m_func->isStaticInPrologue()
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
  void resetExtraArgs();

  /*
   * Get the extra argument with index `ind', from either the VarEnv or the
   * ExtraArgs, whichever is set.
   *
   * Returns nullptr if there are no extra arguments.
   */
  TypedValue* getExtraArg(unsigned ind) const;

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
constexpr auto kNativeFrameSize = offsetof(ActRec, m_func);

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
bool isDebuggerReturnHelper(void* address);

/* Offset of the m_func and m_thisUnsafe fields in cells */

static_assert(offsetof(ActRec, m_func) % sizeof(Cell) == 0, "");
static_assert(offsetof(ActRec, m_thisUnsafe) % sizeof(Cell) == 0, "");

constexpr auto kActRecFuncCellOff = offsetof(ActRec, m_func) /
                                    sizeof(Cell);
constexpr auto kActRecCtxCellOff  = offsetof(ActRec, m_thisUnsafe) /
                                    sizeof(Cell);

///////////////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/vm/act-rec-inl.h"

#endif
