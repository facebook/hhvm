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

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/func.h"

#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <folly/Range.h>

namespace HPHP {

namespace jit { struct Type; }

//////////////////////////////////////////////////////////////////////////////

struct MysteryBoxConstraint;
struct MysteryBoxProvenance;

struct RenderException : std::runtime_error {
  using std::runtime_error::runtime_error;
};

struct ROMRenderer {
  virtual void checkConstraint(const MysteryBoxProvenance&,
                               const MysteryBoxConstraint&) = 0;
  virtual void allocAndCopyROM(const uint8_t* source, size_t numBytes) = 0;
  virtual void fixupInterior(ptrdiff_t fixupAddr, ptrdiff_t targetAddr) = 0;
  virtual void breakMystery(const MysteryBoxProvenance&, tv_val_offset off) = 0;
  virtual void incref(const MysteryBoxProvenance&) = 0;
  virtual void outputMystery(const MysteryBoxProvenance&, uint32_t root) = 0;
  virtual void outputHeapPtr(DataType dt, ptrdiff_t off, uint32_t root) = 0;
  virtual void outputConstant(TypedValue tv, uint32_t root) = 0;
};

/*
 * A ROM is segment of heap that can be memcpy'ed into place and converted into
 * live objects.  This rendering can be converted into a JITed routine or
 * stamped out one off.
 */
struct ROMData;

// A smart poitner handle to the ROM.
struct ROMHandle {
  ROMHandle();
  explicit ROMHandle(std::unique_ptr<ROMData>&& data);
  ROMHandle(ROMHandle&& o) noexcept;
  ROMHandle& operator=(ROMHandle&& o) noexcept;
  ~ROMHandle();

  const ROMData& get() const {
    return *data.get();
  }
  std::unique_ptr<ROMData> data;
};

// A wrapper for the smart pointer ROM handle for handling by php.
struct ROMResourceHandle : ROMHandle, SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(ROMResourceHandle);
  using ROMHandle::ROMHandle;
};

std::vector<TypedValue> renderROMOnce(const ROMData& rom, TypedValue ctx,
                                      std::vector<TypedValue>&& args,
                                      bool partial);
ROMHandle optimizeROM(ROMHandle&& rom, bool& unwrapEager, TypedValue ctx,
                      uint32_t nargs, const TypedValue* args);
Optional<ROMHandle> computeROM(const std::vector<TypedValue>& roots,
                               const ROMData* baseROM);

void renderROM(ROMRenderer& renderer, const ROMData& rom);
std::string show(const ROMData& rom);
std::string showShort(const ROMData& rom);

/*
 * MysteryBox is a placeholder used to represent unknown inputs to a function
 * for which we're doing an inline-interp. (A MysteryBox may also be able to
 * represent certain other unknown values; for example, accessing a property
 * of a MysteryBox may return another MysteryBox that tracks its provenance.)
 */
struct MysteryBoxData;

enum class MysteryBoxType { Unknown, Context, Local };

struct MysteryBoxConstraint {
  const TypeConstraint tc;
  const Class* ctx;
  const Class* propDecl;
};

struct MysteryBoxProvenance {
  const MysteryBoxType type;
  const uint32_t local{(uint32_t)-1};
};

struct MysteryBox : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(MysteryBox);

  static req::ptr<MysteryBox> Make();
  static req::ptr<MysteryBox> Make(const jit::Type& type);
  static req::ptr<MysteryBox> Make(folly::StringPiece type);

  static bool IsMysteryBox(TypedValue tv);

  // Try to apply the given TypeConstraint to a MysteryBox. Doing so may fail,
  // or may require making a new box for the coerced value. Returns success.
  static bool TryConstrain(tv_lval lval, const TypeConstraint& constraint,
                           const Class* ctx, const Class* propDecl);

  MysteryBox(MysteryBoxData* data, bool unowned = false);
  ~MysteryBox();

  const String& o_getClassNameHook() const override;

  MysteryBoxData* data;
  bool unowned;
};

/*
 * Per-bytecode inline interp flags:
 *
 *  - NONE means no special handling: interp the bytecode as normal.
 *
 *  - SKIP means that we executed the bytecode via an alternate means (e.g.
 *    by predicting some global's value) and the interpreter should skip it.
 *
 *  - STOP means that we can't inline further and should stop interp-ing.
 */
enum class InlineInterpHookResult { NONE, SKIP, STOP };

InlineInterpHookResult callInlineInterpHook();

Optional<ROMHandle> runInlineInterp(const Func* func, TypedValue context,
                                    uint32_t nargs, const TypedValue* args,
                                    RuntimeCoeffects coeffects,
                                    uint64_t* numOps = nullptr);

//////////////////////////////////////////////////////////////////////////////

}

