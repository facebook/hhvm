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

#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

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
#include <llvm/IRReader/IRReader.h>
#include <llvm/PassManager.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>

TRACE_SET_MOD(llvm);

namespace HPHP { namespace jit {

// TODO: Remove this (t5048575)
using namespace ::HPHP::jit::x64;

namespace {

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
    , m_irb(llvm::BasicBlock::Create(m_context, "", m_function))
    , m_valueInfo(unit.next_vr)
    , m_unit(unit)
    , m_areas(areas)
  {
    m_function->setCallingConv(llvm::CallingConv::X86_64_HHVM);
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
  }

  ~LLVMEmitter() {
  }

  std::string showModule() const {
    std::string s;
    llvm::raw_string_ostream stream(s);
    m_module->print(stream, nullptr);
    return s;
  }

  /*
   * Finalize the code generation process by optimizing and generating code for
   * m_module.
   */
  void finalize() {
    // Add a 'ret void' to the generated function so that LLVM codegen doesn't
    // crash.  The correct solution is to have all branches call a service
    // request handler (within a patch point), which then allows LLVM to apply
    // tail call elimination and convert the generated code to a useful form.
    m_irb.CreateRetVoid();
    FTRACE(1, "{:-^80}\n{}\n", " LLVM IR before optimizing ", showModule());

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
  void registerValue(Vreg tmp, llvm::Value* llvmVal) {
    always_assert(m_valueInfo.at(tmp) == nullptr);

    m_valueInfo[tmp] = llvmVal;
  }

  /*
   * Look up the llvm::Value representing tmp.
   */
  llvm::Value* value(Vreg tmp) const {
    auto value = m_valueInfo.at(tmp);
    always_assert(value);
    return value;
  }

  /*
   * Generate an llvm::Value representing the given integral constant, with an
   * approprate bit width.
   */
  template<typename Int>
  typename std::enable_if<std::is_integral<Int>::value, llvm::Value*>::type
  cns(Int val) {
    return llvm::ConstantInt::get(
      m_context,
      llvm::APInt(sizeof(val) * CHAR_BIT, val, std::is_signed<Int>::value)
    );
  }

  void emit(const jit::vector<Vlabel>& labels);

private:
#define O(name, ...) void emit(const name&);
X64_OPCODES
#undef O

  llvm::LLVMContext& m_context;
  std::unique_ptr<llvm::Module> m_module;
  llvm::Function* m_function;
  llvm::IRBuilder<> m_irb;

  // Map of Vasm registers to LLVM IR Values.
  jit::vector<llvm::Value*> m_valueInfo;

  const Vunit& m_unit;
  Vasm::AreaList& m_areas;
};

void LLVMEmitter::emit(const jit::vector<Vlabel>& labels) {
  for (auto label : labels) {
    auto& block = m_unit.blocks[label];
    for (auto& inst : block.code) {
      switch (inst.op) {
// This list will eventually go away; for now only a very small subset of
// operations are supported.
#define SUPPORTED_OPS \
O(addqi) \
O(call) \
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
O(nocatch) \
O(pushm) \
O(resume) \
O(ret) \
O(retransopt) \
O(storebim) \
O(syncpoint) \
O(testlim) \
O(ud2) \
O(unwind)
#define O(name) case Vinstr::name: emit(inst.name##_); break;
  SUPPORTED_OPS
#undef O
#undef SUPPORTED_OPS

      default:
        throw FailedLLVMCodeGen(
          folly::format(
            "Unsupported opcode in B{}: {}",
            size_t(label), show(m_unit, inst)).str());
      }
    }
  }

  finalize();
}

void LLVMEmitter::emit(const addqi& inst) {
}

void LLVMEmitter::emit(const end& inst) {
  // no-op
}

void LLVMEmitter::emit(const call& inst) {
  // Only calls with no arguments and no return value are currently supported.
  auto fnType = llvm::PointerType::get(
    llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false),
    0
  );
  auto fn = m_irb.CreateIntToPtr(cns(uintptr_t(inst.target)), fnType);
  m_irb.CreateCall(fn);
}

void LLVMEmitter::emit(const cmpq& inst) {
}

void LLVMEmitter::emit(const cmpqim& inst) {
}

void LLVMEmitter::emit(const copy& inst) {
}

void LLVMEmitter::emit(const decqm& inst) {
}

void LLVMEmitter::emit(const incstat& inst) {
}

void LLVMEmitter::emit(const incwm& inst) {
}

void LLVMEmitter::emit(const jcc& inst) {
}

void LLVMEmitter::emit(const jmp& inst) {
}

void LLVMEmitter::emit(const ldimm& inst) {
}

void LLVMEmitter::emit(const lea& inst) {
}

void LLVMEmitter::emit(const load& inst) {
  auto disp = cns(int64_t(inst.s.disp));
  if (inst.s.index.isValid()) {
    auto scaledIdx = m_irb.CreateMul(value(inst.s.index),
                                     cns(int64_t(inst.s.scale)));
    disp = m_irb.CreateAdd(disp, scaledIdx);
  }

  auto bytePtrType =
    llvm::PointerType::get(llvm::IntegerType::get(m_context, 8), 0);
  auto ptr = inst.s.base.isValid() ? value(inst.s.base) : cns(0);
  ptr = m_irb.CreateBitCast(ptr, bytePtrType);
  ptr = m_irb.CreateGEP(ptr, disp);
  auto ptrType = llvm::PointerType::get(llvm::IntegerType::get(m_context, 64),
                                        0);
  ptr = m_irb.CreateBitCast(ptr, ptrType);

  if (inst.s.seg == Vptr::FS) {
    auto fsPtrType =
      llvm::PointerType::get(llvm::IntegerType::get(m_context, 64), 257);
    ptr = m_irb.CreateAddrSpaceCast(ptr, fsPtrType);
  }

  auto value = m_irb.CreateLoad(ptr);
  registerValue(inst.d, value);
}

void LLVMEmitter::emit(const loadq& inst) {
}

void LLVMEmitter::emit(const movq& inst) {
}

void LLVMEmitter::emit(const nocatch& inst) {
}

void LLVMEmitter::emit(const pushm& inst) {
}

void LLVMEmitter::emit(const resume& inst) {
}

void LLVMEmitter::emit(const ret& inst) {
}

void LLVMEmitter::emit(const retransopt& inst) {
}

void LLVMEmitter::emit(const storebim& inst) {
}

void LLVMEmitter::emit(const syncpoint& inst) {
}

void LLVMEmitter::emit(const testlim& inst) {
}

void LLVMEmitter::emit(const ud2& inst) {
}

void LLVMEmitter::emit(const unwind& inst) {
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

using namespace x64;
void genCodeLLVM(const Vunit& unit, Vasm::AreaList& areas,
                 const jit::vector<Vlabel>& labels) {
  throw FailedLLVMCodeGen("This build does not support the LLVM backend");
}

} }

#endif // #ifdef USE_LLVM
