/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/vasm-llvm.h"

#include "hphp/util/assertions.h"
#include "hphp/util/disasm.h"

#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

#ifdef USE_LLVM

#include <llvm/Analysis/Passes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/ObjectCache.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/TypeBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/PassManager.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>

TRACE_SET_MOD(llvm);

namespace HPHP { namespace jit {

namespace {

void reportLLVMError(void* data, const std::string& err, bool gen_crash_diag) {
  always_assert_flog(false, "LLVM fatal error: {}", err);
}

struct LLVMErrorInit {
  LLVMErrorInit() {
    llvm::install_fatal_error_handler(reportLLVMError);
  }

  ~LLVMErrorInit() {
    llvm::remove_fatal_error_handler();
  }
};
static LLVMErrorInit s_llvmErrorInit;

/*
 * TCMemoryManager allows llvm to emit code into the appropriate places in the
 * TC. Currently all code goes into the Main code block.
 */
struct TCMemoryManager : public llvm::RTDyldMemoryManager {
  explicit TCMemoryManager(Vasm::AreaList& areas)
    : m_areas(areas)
  {
  }

  uint8_t* allocateCodeSection(
    uintptr_t Size, unsigned Alignment, unsigned SectionID,
    llvm::StringRef SectionName
  ) override {
    auto& code = m_areas[static_cast<size_t>(AreaIndex::Main)].code;
    uint8_t* ret = code.alloc<uint8_t>(Alignment, Size);

    FTRACE(1, "Allocate code section \"{}\" id={} at addr={}, size={},"
           " alignment={}\n",
           SectionName.str(), SectionID, ret, Size, Alignment);
    return ret;
  }

  uint8_t* allocateDataSection(
    uintptr_t Size, unsigned Alignment, unsigned SectionID,
    llvm::StringRef SectionName, bool IsReadOnly
  ) override {
    uint8_t* ret = mcg->globalData().alloc<uint8_t>(Alignment, Size);
    FTRACE(1, "Allocate {} data section \"{}\" id={} at addr={}, size={},"
           " alignment={}\n",
           IsReadOnly ? "read-only" : "read-write",
           SectionName.str(), SectionID, ret, Size, Alignment);
    return ret;
  }

  virtual void reserveAllocationSpace(uintptr_t CodeSize,
                                      uintptr_t DataSizeRO,
                                      uintptr_t DataSizeRW) override {
    FTRACE(1, "reserve CodeSize={}, DataSizeRO={}, DataSizeRW={}\n", CodeSize,
           DataSizeRO, DataSizeRW);
  }

  virtual bool needsToReserveAllocationSpace() override {
    return true;
  }

  virtual void registerEHFrames(uint8_t *Addr, uint64_t LoadAddr,
                                size_t Size) override {
    // Do nothing; the TC has one huge eh frame.
  }

  virtual void deregisterEHFrames(uint8_t *Addr, uint64_t LoadAddr,
                                  size_t Size) override {
    // Do nothing; the TC has one huge eh frame.
  }

  virtual bool finalizeMemory(std::string *ErrMsg = nullptr) override {
    return false;
  }

  virtual uint64_t getSymbolAddress(const std::string& name) override {
    FTRACE(1, "getSymbolAddress({})\n", name);
    // This is currently only called with '__unnamed_2', and the address we
    // return doesn't appear anywhere in the generated code. This may change,
    // and when it does we should figure out what llvm wants and return a
    // meaningful address.
    return 0xbadbadbad;
  }

private:
  Vasm::AreaList& m_areas;
};

/*
 * LLVMEmitter is responsible for transforming a Vunit into LLVM IR, then
 * optimizing that and emitting machine code from the result.
 */
struct LLVMEmitter {
  explicit LLVMEmitter(const Vunit& unit, Vasm::AreaList& areas)
    : m_context(llvm::getGlobalContext())
    , m_module(new llvm::Module("", m_context))
    , m_function(llvm::Function::Create(
      llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false),
      llvm::Function::ExternalLinkage, "", m_module.get()))
    , m_irb(llvm::BasicBlock::Create(m_context,
                                     folly::to<std::string>('B',
                                                            size_t(unit.entry)),
                                     m_function))
    , m_valueInfo(unit.next_vr)
    , m_blocks(unit.blocks.size())
    , m_unit(unit)
    , m_areas(areas)
  {
    m_function->setCallingConv(llvm::CallingConv::X86_64_HHVM);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    m_blocks[unit.entry] = m_irb.GetInsertBlock();

    for (auto& pair : unit.cpool) {
      defineValue(pair.second, cns(pair.first));
    }
  }

  ~LLVMEmitter() {
  }

  std::string showModule() const {
    std::string s;
    llvm::raw_string_ostream stream(s);
    m_module->print(stream, nullptr);
    return stream.str();
  }

  void verifyModule() const {
    std::string err;
    llvm::raw_string_ostream stream(err);
    always_assert_flog(!llvm::verifyModule(*m_module, &stream),
                       "LLVM verifier failed:\n{}\n{:-^80}\n{}\n{:-^80}\n{}",
                       stream.str(), " vasm unit ", show(m_unit),
                       " llvm module ", showModule());
  }

  /*
   * Finalize the code generation process by optimizing and generating code for
   * m_module.
   */
  void finalize() {
    FTRACE(1, "{:-^80}\n{}\n", " LLVM IR before optimizing ", showModule());
    verifyModule();

    assert(m_module != nullptr);

    auto fpm = folly::make_unique<llvm::FunctionPassManager>(m_module.get());
    fpm->add(new llvm::DataLayoutPass(m_module.get()));
    fpm->add(llvm::createBasicAliasAnalysisPass());
    fpm->add(llvm::createPromoteMemoryToRegisterPass());
    fpm->add(llvm::createInstructionCombiningPass());
    fpm->add(llvm::createReassociatePass());
    fpm->add(llvm::createGVNPass());
    fpm->add(llvm::createCFGSimplificationPass());
    fpm->doInitialization();

    for (auto it = m_module->begin(); it != m_module->end(); ++it) {
      fpm->run(*it);
    }
    FTRACE(2, "{:-^80}\n{}\n", " LLVM IR after optimizing ", showModule());

    std::string errStr;
    std::unique_ptr<llvm::ExecutionEngine> ee(
      llvm::EngineBuilder(m_module.get())
      .setErrorStr(&errStr)
      .setUseMCJIT(true)
      .setMCJITMemoryManager(new TCMemoryManager(m_areas))
      .setVerifyModules(true)
      .create());
    always_assert_flog(ee,
                       "ExecutionEngine creation failed: {}\n", errStr);
    m_module.release(); // ee took ownership of the module.

    ee->setProcessAllSections(true);
    ee->finalizeObject();
  }

  /*
   * Register the fact that llvmVal represents the Vasm value in tmp.
   */
  void defineValue(Vreg tmp, llvm::Value* llvmVal) {
    if (!tmp.isVirt()) return;

    always_assert(m_valueInfo.at(tmp).llval == nullptr);
    llvmVal->setName(folly::to<std::string>('t', size_t(tmp)));
    m_valueInfo[tmp].llval = llvmVal;
  }

  /*
   * Look up the llvm::Value representing tmp.
   */
  llvm::Value* value(Vreg tmp) const {
    if (!tmp.isVirt()) return cns(int64_t{0});

    auto& info = m_valueInfo.at(tmp);
    always_assert(info.llval);
    return info.llval;
  }

  /*
   * Register the fact that tmp is defined by inst, since vasm units don't have
   * a natural way of following use-def chains.
   */
  void defineValue(Vreg tmp, const Vinstr& inst) {
    always_assert(m_valueInfo.at(tmp).inst.op == Vinstr::ud2 || !tmp.isVirt());
    m_valueInfo[tmp].inst = inst;
  }

  /*
   * Look up the Vinstr that defined tmp.
   */
  const Vinstr& defInst(Vreg tmp) const {
    return m_valueInfo.at(tmp).inst;
  }

  /*
   * Certain vasm instructions compute or load intermediate values that aren't
   * destinations of the instruction, but are used to produce a status flags
   * register that is. One example of this is the incwm instruction: it loads
   * an int16 from memory, increments it, produces a status flags register
   * based on the incremented value, and finally stores the incremented value
   * back to memory. When emitting code for an instruction that consumes this
   * status flag register, we need access to the intermediate value due to the
   * way llvm handles conditional jumps. We call this value a "flag temporary",
   * and is stored in a side table keyed on the status flags Vreg it
   * corresponds to.
   */
  void defineFlagTmp(Vreg vr, llvm::Value* tmp) {
    always_assert(m_valueInfo.at(vr).flagTmp == nullptr);
    m_valueInfo[vr].flagTmp = tmp;
  }

  /*
   * Get the flag temp for the given vr.
   */
  llvm::Value* flagTmp(Vreg vr) const {
    auto& info = m_valueInfo.at(vr);
    always_assert(info.flagTmp);
    return info.flagTmp;
  }

  /*
   * Look up or create the llvm block corresponding to the given vasm block.
   */
  llvm::BasicBlock* block(Vlabel l) {
    if (m_blocks[l] == nullptr) {
      return m_blocks[l] =
        llvm::BasicBlock::Create(m_context,
                                 folly::to<std::string>('B', size_t(l)),
                                 m_function);
    }

    return m_blocks[l];
  }

  /*
   * Generate an llvm::Value representing the given integral constant, with an
   * approprate bit width.
   */
  template<typename Int>
  typename std::enable_if<std::is_integral<Int>::value, llvm::Value*>::type
  cns(Int val) const {
    return llvm::ConstantInt::get(
      m_context,
      llvm::APInt(sizeof(val) * CHAR_BIT, val, std::is_signed<Int>::value)
    );
  }

  /*
   * Return val, bitcast to an approriately-sized integer type.
   */
  llvm::Value* asInt(llvm::Value* val, size_t bits = 64) {
    return m_irb.CreateBitCast(val, m_irb.getIntNTy(bits));
  }

  /*
   * emit LLVM IR for the given list of vasm blocks.
   */
  void emit(const jit::vector<Vlabel>& labels);

 private:
  /*
   * RegInfo is used to track information about Vregs, including their
   * corresponding llvm::Value and the Vinstr that defined them.
   */
  struct RegInfo {
    llvm::Value* llval;
    llvm::Value* flagTmp;
    Vinstr inst;
  };

#define O(name, ...) void emit(const name&);
X64_OPCODES
#undef O

  void emitTrap();
  llvm::Value* emitPtrMath(Vptr p);
  llvm::Value* emitPtr(Vptr p, size_t bitWidth = 64);

  llvm::LLVMContext& m_context;
  std::unique_ptr<llvm::Module> m_module;
  llvm::Function* m_function;
  llvm::IRBuilder<> m_irb;

  // Vreg -> RegInfo map
  jit::vector<RegInfo> m_valueInfo;

  // Vlabel -> llvm::BasicBlock map
  jit::vector<llvm::BasicBlock*> m_blocks;

  const Vunit& m_unit;
  Vasm::AreaList& m_areas;
};

void LLVMEmitter::emit(const jit::vector<Vlabel>& labels) {
  for (auto label : labels) {
    auto& b = m_unit.blocks[label];
    m_irb.SetInsertPoint(block(label));
    for (auto& inst : b.code) {
      switch (inst.op) {
// This list will eventually go away; for now only a very small subset of
// operations are supported.
#define SUPPORTED_OPS \
O(addqi) \
O(vcall) \
O(vinvoke) \
O(end) \
O(cmpq) \
O(cmpqim) \
O(copy) \
O(decqm) \
O(incstat) \
O(incwm) \
O(jcc) \
O(jmp) \
O(ldimm) \
O(lea) \
O(load) \
O(loadq) \
O(movq) \
O(pushm) \
O(resume) \
O(ret) \
O(retransopt) \
O(storebim) \
O(storeqim) \
O(syncpoint) \
O(testlim) \
O(ud2) \
O(landingpad)
#define O(name) case Vinstr::name: emit(inst.name##_); break;
        SUPPORTED_OPS
#undef O
#undef SUPPORTED_OPS

      case Vinstr::call:
      case Vinstr::unwind:
        always_assert_flog(false,
                           "Banned opcode in B{}: {}",
                           size_t(label), show(m_unit, inst));

      default:
        throw FailedLLVMCodeGen(
          folly::format(
            "Unsupported opcode in B{}: {}",
            size_t(label), show(m_unit, inst)).str());
      }

      visitDefs(m_unit, inst, [&](Vreg def) {
        defineValue(def, inst);
      });
    }
  }

  finalize();
}

void LLVMEmitter::emit(const addqi& inst) {
  auto result = m_irb.CreateAdd(value(inst.s1), cns(inst.s0.q()));
  defineValue(inst.d, result);
}

void LLVMEmitter::emit(const end& inst) {
  // no-op
}

void LLVMEmitter::emit(const vcall& inst) {
  auto fnType = llvm::PointerType::get(
    llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false),
    0
  );
  auto fn = m_irb.CreateIntToPtr(cns(uintptr_t(inst.call.address())), fnType);
  m_irb.CreateCall(fn);
}

void LLVMEmitter::emit(const vinvoke& inst) {
  auto next = block(inst.targets[0]);
  auto taken = block(inst.targets[1]);

  auto fnType = llvm::PointerType::get(
    llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false),
    0
  );
  auto fn = m_irb.CreateIntToPtr(cns(uintptr_t(inst.call.address())), fnType);

  m_irb.CreateInvoke(fn, next, taken);
}

void LLVMEmitter::emit(const cmpq& inst) {
  // no-op. The real work happens in jcc
}

void LLVMEmitter::emit(const cmpqim& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1)));
}

void LLVMEmitter::emit(const copy& inst) {
  defineValue(inst.d, value(inst.s));
}

void LLVMEmitter::emit(const decqm& inst) {
  auto ptr = emitPtr(inst.m);
  auto oldVal = m_irb.CreateLoad(ptr);
  auto newVal = m_irb.CreateSub(oldVal, cns(int64_t{1}));
  defineFlagTmp(inst.sf, newVal);
  m_irb.CreateStore(ptr, newVal);
}

void LLVMEmitter::emit(const incstat& inst) {
}

void LLVMEmitter::emit(const incwm& inst) {
  auto ptr = emitPtr(inst.m, 16);
  auto oldVal = m_irb.CreateLoad(ptr);
  auto newVal = m_irb.CreateAdd(oldVal, cns(int16_t{1}));
  defineFlagTmp(inst.sf, newVal);
  m_irb.CreateStore(ptr, newVal);
}

static llvm::CmpInst::Predicate ccToPred(ConditionCode cc) {
  using Cmp = llvm::CmpInst;
  switch (cc) {
    case CC_E:  return Cmp::ICMP_EQ;
    case CC_NE: return Cmp::ICMP_NE;
    case CC_L:  return Cmp::ICMP_SLT;
    case CC_LE: return Cmp::ICMP_SLE;
    case CC_G:  return Cmp::ICMP_SGT;
    case CC_GE: return Cmp::ICMP_SGE;
    case CC_B:  return Cmp::ICMP_ULT;
    case CC_BE: return Cmp::ICMP_ULE;
    case CC_A:  return Cmp::ICMP_UGT;
    case CC_AE: return Cmp::ICMP_UGE;
    default:    not_implemented();
  }
}

void LLVMEmitter::emit(const jcc& inst) {
  auto next  = block(inst.targets[0]);
  auto taken = block(inst.targets[1]);
  auto& cmp = defInst(inst.sf);
  llvm::Value* lhs = nullptr;
  llvm::Value* rhs = nullptr;

  if (cmp.op == Vinstr::addqi) {
    lhs = asInt(value(cmp.addqi_.d));
    rhs = cns(uint64_t{0});
  } else if (cmp.op == Vinstr::cmpq) {
    lhs = asInt(value(cmp.cmpq_.s1));
    rhs = asInt(value(cmp.cmpq_.s0));
  } else if (cmp.op == Vinstr::cmpqim) {
    lhs = flagTmp(inst.sf);
    rhs = cns(cmp.cmpqim_.s0.q());
  } else if (cmp.op == Vinstr::decqm) {
    lhs = flagTmp(inst.sf);
    rhs = cns(uint64_t{0});
  } else if (cmp.op == Vinstr::incwm) {
    lhs = flagTmp(inst.sf);
    rhs = cns(uint16_t{0});
  } else if (cmp.op == Vinstr::testlim) {
    lhs = flagTmp(inst.sf);
    rhs = cns(uint32_t{0});
  } else {
    always_assert_flog(false, "Unsupported flags src: {}",
                       show(m_unit, inst));
  }

  auto cond = m_irb.CreateICmp(ccToPred(inst.cc), lhs, rhs);
  m_irb.CreateCondBr(cond, taken, next);
}

void LLVMEmitter::emit(const jmp& inst) {
  m_irb.CreateBr(block(inst.target));
}

void LLVMEmitter::emit(const ldimm& inst) {
  defineValue(inst.d, cns(inst.s.q()));
}

void LLVMEmitter::emit(const lea& inst) {
  assert_not_implemented(inst.s.seg == Vptr::DS);
  defineValue(inst.d, emitPtrMath(inst.s));
}

void LLVMEmitter::emit(const load& inst) {
  defineValue(inst.d, m_irb.CreateLoad(emitPtr(inst.s)));
}

void LLVMEmitter::emit(const loadq& inst) {
  defineValue(inst.d, m_irb.CreateLoad(emitPtr(inst.s)));
}

void LLVMEmitter::emit(const movq& inst) {
  defineValue(inst.d, value(inst.s));
}

void LLVMEmitter::emit(const pushm& inst) {
}

void LLVMEmitter::emit(const resume& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const ret& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const retransopt& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const storebim& inst) {
}

void LLVMEmitter::emit(const storeqim& inst) {
}

void LLVMEmitter::emit(const syncpoint& inst) {
}

void LLVMEmitter::emit(const testlim& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 32));
  auto result = m_irb.CreateAnd(lhs, inst.s0.w());
  defineFlagTmp(inst.sf, result);
}

void LLVMEmitter::emit(const ud2& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const landingpad& inst) {
  // This is far from correct, but it's enough to keep the llvm verifier happy
  // for now.
  auto fnType = llvm::PointerType::get(
    llvm::FunctionType::get(m_irb.getVoidTy(), false), 0);
  auto fn = m_irb.CreateIntToPtr(cns(uintptr_t(tc_unwind_personality)), fnType);
  auto pad = m_irb.CreateLandingPad(m_irb.getInt8Ty(), fn, 0);
  pad->setCleanup(true);
}

void LLVMEmitter::emitTrap() {
  auto trap = llvm::Intrinsic::getDeclaration(m_module.get(),
                                              llvm::Intrinsic::trap);
  m_irb.CreateCall(trap);
  m_irb.CreateUnreachable();
}

llvm::Value* LLVMEmitter::emitPtrMath(Vptr src) {
  auto disp = cns(int64_t(src.disp));
  if (src.index.isValid()) {
    auto scaledIdx = m_irb.CreateMul(value(src.index),
                                     cns(int64_t(src.scale)));
    disp = m_irb.CreateAdd(disp, scaledIdx);
  }

  auto base = src.base.isValid() ? value(src.base) : cns(0);
  return m_irb.CreateAdd(base, disp);
}

llvm::Value* LLVMEmitter::emitPtr(Vptr src, size_t bits) {
  auto ptr = emitPtrMath(src);
  auto ptrType = llvm::PointerType::get(m_irb.getIntNTy(bits), 0);
  ptr = m_irb.CreateIntToPtr(ptr, ptrType);

  if (src.seg == Vptr::FS) {
    auto fsPtrType = llvm::PointerType::get(m_irb.getIntNTy(bits), 257);
    ptr = m_irb.CreateAddrSpaceCast(ptr, fsPtrType);
  }

  return ptr;
}

std::string showNewCode(const Vasm::AreaList& areas) DEBUG_ONLY;
std::string showNewCode(const Vasm::AreaList& areas) {
  std::ostringstream str;
  Disasm disasm(Disasm::Options().indent(2));

  for (unsigned i = 0, n = areas.size(); i < n; ++i) {
    auto& area = areas[i];
    auto const start = area.start;
    auto const end = area.code.frontier();

    if (start != end) {
      str << folly::format("emitted {} bytes of code into area {}:\n",
                           end - start, i);
      disasm.disasm(str, start, end);
      str << '\n';
    }
  }

  return str.str();
}

}

void genCodeLLVM(const Vunit& unit, Vasm::AreaList& areas,
                 const jit::vector<Vlabel>& labels) {
  FTRACE(2, "\nTrying to emit LLVM IR for Vunit:\n{}\n", show(unit));

  UndoMarker dataUndo(mcg->globalData());

  try {
    LLVMEmitter(unit, areas).emit(labels);
    FTRACE(3, "\n{:-^80}\n{}\n",
           " x64 after LLVM codegen ", showNewCode(areas));

    throw FailedLLVMCodeGen("LLVM codegen doesn't work yet");
  } catch (const FailedLLVMCodeGen& e) {
    FTRACE(1, "LLVM codegen failed: {}\n", e.what());

    // Undo any code/data we may have allocated.
    dataUndo.undo();
    for (auto& area : areas) {
      area.code.setFrontier(area.start);
    }
    throw e;
  }
}

} }

#else // #ifdef USE_LLVM

namespace HPHP { namespace jit {

void genCodeLLVM(const Vunit& unit, Vasm::AreaList& areas,
                 const jit::vector<Vlabel>& labels) {
  throw FailedLLVMCodeGen("This build does not support the LLVM backend");
}

} }

#endif // #ifdef USE_LLVM
