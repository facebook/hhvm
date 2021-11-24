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
#include "hphp/runtime/vm/jit/irgen-sib.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

#include "hphp/runtime/vm/super-inlining-bros.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {
//////////////////////////////////////////////////////////////////////

struct RenderJIT : ROMRenderer {
  RenderJIT(IRGS& env, SSATmp* ctx,
            const std::vector<SSATmp*>& args, Block* fail)
    : env(env), ctx(ctx), args(args), fail(fail) {}
  void checkConstraint(const MysteryBoxProvenance& provenance,
                       const MysteryBoxConstraint& constraint) override {
    auto const src = selectMysteryBox(provenance);
    verifyMysteryBoxConstraint(env, constraint, src, fail);
  }
  void allocAndCopyROM(const uint8_t* source, size_t numBytes) override {
    if (numBytes > kMaxSmallSize) {
      FTRACE_MOD(Trace::sib, 1, "ROM alloc Jmps to fail mallocing big size\n");
      gen(env, Jmp, fail);
      return;
    }
    AllocInitROMData extra;
    extra.rom = source;
    extra.size = numBytes;
    base = gen(env, AllocInitROM, extra);
  }
  void fixupInterior(ptrdiff_t fixupOff, ptrdiff_t targetOff) override {
    auto const fixupAddr = gen(env, AddOffset, base, cns(env, fixupOff));
    auto const targetAddr = gen(env, AddOffset, base, cns(env, targetOff));
    gen(env, StPtrAt, fixupAddr, targetAddr);
  }
  void breakMystery(
      const MysteryBoxProvenance& provenance, tv_val_offset tv_off) override {
    auto const src = selectMysteryBox(provenance);
    auto const typeAddr = gen(env, AddOffset, base, cns(env, tv_off.typeOffset()));
    gen(env, StTypeAt, typeAddr, src);
    auto const valAddr = gen(env, AddOffset, base, cns(env, tv_off.dataOffset()));
    gen(env, StPtrAt, valAddr, src);
  }
  void incref(const MysteryBoxProvenance& provenance) override {
    auto const src = selectMysteryBox(provenance);
    gen(env, IncRef, src);
  }
  void outputMystery(
      const MysteryBoxProvenance& provenance, uint32_t root) override {
    auto const src = selectMysteryBox(provenance);
    roots.resize(root + 1);
    roots[root] = src;
  }
  void outputHeapPtr(DataType dt, ptrdiff_t off, uint32_t root) override {
    roots.resize(root + 1);
    auto const val = gen(env, AddOffset, base, cns(env, off));
    roots[root] = gen(env, VoidPtrAsDataType, Type(dt), val);
  }
  void outputConstant(TypedValue tv, uint32_t root) override {
    roots.resize(root + 1);
    roots[root] = cns(env, tv);
  }
  std::vector<SSATmp*> roots;

private:
  SSATmp* selectMysteryBox(const MysteryBoxProvenance& provenance) {
    switch (provenance.type) {
      case MysteryBoxType::Unknown:
        always_assert(false);
      case MysteryBoxType::Context:
        return ctx;
      case MysteryBoxType::Local: {
        auto const local = provenance.local;
        assert_flog(local < args.size(),
                    "MysteryBox local {} out of range (nargs = {})",
                    local, args.size());
        return args[local];
      }
    }
    always_assert(false);
  }

  IRGS& env;
  SSATmp* base;
  SSATmp* ctx;
  const std::vector<SSATmp*>& args;
  Block* fail;
};
//////////////////////////////////////////////////////////////////////
}

std::vector<SSATmp*> irgenROM(IRGS& env, const ROMData& rom, SSATmp* ctx,
                              const std::vector<SSATmp*>& args, Block* fail) {
  RenderJIT renderer(env, ctx, args, fail);
  renderROM(renderer, rom);
  return renderer.roots;
}

//////////////////////////////////////////////////////////////////////

}}}
