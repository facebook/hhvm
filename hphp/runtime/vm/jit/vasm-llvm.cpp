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

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/reserved-stack.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

#ifdef USE_LLVM

#include <llvm/Analysis/Passes.h>
#include <llvm/CodeGen/MachineFunctionAnalysis.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/ObjectCache.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
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
#include <llvm/Target/TargetMachine.h>
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
 * Map of global symbols used by LLVM.
 */
static jit::hash_map<std::string, uint64_t> globalSymbols;

void registerGlobalSymbol(const std::string& name, uint64_t address) {
  auto it = globalSymbols.emplace(name, address);
  always_assert((it.second == true || it.first->second == address) &&
                "symbol already registered with a different value");
}

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
    // TODO(#5398968): find a better way to disable alignment.
    Alignment = 1;
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

  virtual void registerEHFrames(uint8_t* Addr, uint64_t LoadAddr,
                                size_t Size) override {
    // Do nothing; the TC has one huge eh frame.
  }

  virtual void deregisterEHFrames(uint8_t* Addr, uint64_t LoadAddr,
                                  size_t Size) override {
    // Do nothing; the TC has one huge eh frame.
  }

  virtual bool finalizeMemory(std::string* ErrMsg = nullptr) override {
    return false;
  }

  virtual uint64_t getSymbolAddress(const std::string& name) override {
    FTRACE(1, "getSymbolAddress({})\n", name);
    auto element = globalSymbols.find(name);
    return element == globalSymbols.end() ? 0 : element->second;
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
      llvm::FunctionType::get(
          llvm::Type::getVoidTy(m_context),
          std::vector<llvm::Type*>({
              llvm::IntegerType::get(m_context, 64),
              llvm::IntegerType::get(m_context, 64),
              llvm::IntegerType::get(m_context, 64)}),
          false),
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
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    m_function->setCallingConv(llvm::CallingConv::X86_64_HHVM_TC);
    m_function->setAlignment(1);

    // TODO(#5398968): find a better way to disable 16-byte alignment.
    m_function->addFnAttr(llvm::Attribute::OptimizeForSize);

    m_blocks[unit.entry] = m_irb.GetInsertBlock();

    // Register all unit's constants.
    for (auto const& pair : unit.cpool) {
      defineValue(pair.second, cns(pair.first));
    }

    auto args = m_function->arg_begin();
    m_valueInfo[Vreg(x64::rVmSp)].llval = args++;
    m_rVmTl = m_valueInfo[Vreg(x64::rVmTl)].llval = args++;
    m_rVmTl->setName("rVmTl");
    m_rVmFp = m_valueInfo[Vreg(x64::rVmFp)].llval = args++;
    m_rVmFp->setName("rVmFp");

    // Commonly used types and values.
    m_int8  = m_irb.getInt8Ty();
    m_int16 = m_irb.getInt16Ty();
    m_int32 = m_irb.getInt32Ty();
    m_int64 = m_irb.getInt64Ty();

    m_int8Ptr  = llvm::Type::getInt8PtrTy(m_context);
    m_int16Ptr = llvm::Type::getInt16PtrTy(m_context);
    m_int32Ptr = llvm::Type::getInt32PtrTy(m_context);
    m_int64Ptr = llvm::Type::getInt64PtrTy(m_context);

    m_int8FSPtr  = llvm::Type::getInt8PtrTy(m_context,  257);
    m_int16FSPtr = llvm::Type::getInt16PtrTy(m_context, 257);
    m_int32FSPtr = llvm::Type::getInt32PtrTy(m_context, 257);
    m_int64FSPtr = llvm::Type::getInt64PtrTy(m_context, 257);

    m_int8Zero  = m_irb.getInt8(0);
    m_int8One   = m_irb.getInt8(1);
    m_int16Zero = m_irb.getInt16(0);
    m_int16One  = m_irb.getInt16(1);
    m_int32Zero = m_irb.getInt32(0);
    m_int32One  = m_irb.getInt32(1);
    m_int64Zero = m_irb.getInt64(0);
    m_int64One  = m_irb.getInt64(1);

    m_int64Undef  = llvm::UndefValue::get(m_int64);

    auto m_personalityFTy = llvm::FunctionType::get(m_int32, false);
    m_personalityFunc =
      llvm::Function::Create(m_personalityFTy,
                             llvm::GlobalValue::ExternalLinkage,
                             "personality0",
                             m_module.get());
    m_personalityFunc->setCallingConv(llvm::CallingConv::C);
    registerGlobalSymbol("personality0", 0xbadbadbad);

    m_retFuncPtrPtrType = llvm::PointerType::get(llvm::PointerType::get(
          llvm::FunctionType::get(
              m_irb.getVoidTy(),
              std::vector<llvm::Type*>({m_int64, m_int64, m_int64}),
              false),
          0), 0);

    m_typedValueType = llvm::StructType::get(
        m_context,
        packed_tv
          ? std::vector<llvm::Type*>({m_int8,   // padding
                                      m_int8,   // m_type
                                      m_int64}) // m_data
          : std::vector<llvm::Type*>({m_int64,  // m_data
                                      m_int8}), // m_type
        /*isPacked*/ false);
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


    // TODO(#5406596): teach LLVM our alignment rules. For the
    // moment override 16-byte ABI default.
    llvm::TargetOptions targetOptions;
    targetOptions.StackAlignmentOverride = 8;

    std::string errStr;
    std::unique_ptr<llvm::ExecutionEngine> ee(
      llvm::EngineBuilder(m_module.get())
      .setErrorStr(&errStr)
      .setUseMCJIT(true)
      .setMCJITMemoryManager(new TCMemoryManager(m_areas))
      .setOptLevel(llvm::CodeGenOpt::Aggressive)
      .setRelocationModel(llvm::Reloc::Static)
      .setCodeModel(llvm::CodeModel::Small)
      .setVerifyModules(true)
      .setTargetOptions(targetOptions)
      .create());
    always_assert_flog(ee, "ExecutionEngine creation failed: {}\n", errStr);

    assert(m_module != nullptr);

    llvm::LLVMTargetMachine* targetMachine =
      static_cast<llvm::LLVMTargetMachine*>(ee->getTargetMachine());

    auto fpm = folly::make_unique<llvm::FunctionPassManager>(m_module.get());
    fpm->add(new llvm::DataLayoutPass(m_module.get()));
    targetMachine->addAnalysisPasses(*fpm);

    fpm->add(llvm::createBasicAliasAnalysisPass());
    fpm->add(llvm::createVerifierPass(true));
    fpm->add(llvm::createDebugInfoVerifierPass(false));
    fpm->add(llvm::createLoopSimplifyPass());
    fpm->add(llvm::createGCLoweringPass());
    fpm->add(llvm::createUnreachableBlockEliminationPass());
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

    m_module.release(); // ee took ownership of the module.

    ee->setProcessAllSections(true);
    ee->finalizeObject();
  }

  /*
   * Register the fact that llvmVal represents the Vasm value in tmp.
   */
  void defineValue(Vreg tmp, llvm::Value* llvmVal) {
    always_assert(tmp.isPhys() || m_valueInfo.at(tmp).llval == nullptr);

    if (tmp.isVirt()) {
      llvmVal->setName(folly::to<std::string>('t', size_t(tmp)));
    }
    m_valueInfo[tmp].llval = llvmVal;
  }

  /*
   * Look up the llvm::Value representing tmp.
   */
  llvm::Value* value(Vreg tmp) const {
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
    return m_irb.CreateBitCast(val, intNType(bits));
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
VASM_OPCODES
#undef O

  void emitTrap();
  void emitCall(const Vinstr& instr);
  //
  /// Emit code for the pointer. Return value is of type <i{bits} *>.
  llvm::Value* emitPtr(const Vptr s, size_t bits = 64);

  llvm::Type* intNType(size_t bits) const;
  llvm::Type* ptrIntNType(size_t bits, bool inFS) const;

  llvm::Value* getReturnAddress();
  UNUSED llvm::Value* getFrameAddress();
  UNUSED llvm::Value* getStackPointer();

  UNUSED void emitAsm(const std::string& asmStatement,
                      const std::string& asmConstraints,
                      bool hasSideEffects);


  llvm::LLVMContext& m_context;
  std::unique_ptr<llvm::Module> m_module;
  llvm::Function* m_function;
  llvm::IRBuilder<> m_irb;

  // Function type used for tail call return.
  llvm::Type* m_retFuncPtrPtrType{nullptr};

  // Mimic HHVM's TypedValue.
  llvm::StructType* m_typedValueType{nullptr};

  // Address to return if any.
  llvm::Value* m_addressToReturn{nullptr};

  // Saved LLVM intrinsics.
  llvm::Function* m_llvmFrameAddress{nullptr};
  llvm::Function* m_llvmReadRegister{nullptr};
  llvm::Function* m_llvmReturnAddress{nullptr};

  // Vreg -> RegInfo map
  jit::vector<RegInfo> m_valueInfo;

  // Vlabel -> llvm::BasicBlock map
  jit::vector<llvm::BasicBlock*> m_blocks;

  const Vunit& m_unit;
  Vasm::AreaList& m_areas;

  // Special regs.
  llvm::Value* m_rVmTl{nullptr};
  llvm::Value* m_rVmFp{nullptr};

  // Faux personality for emitting landingpad.
  llvm::Function* m_personalityFunc;

  // Commonly used types. Some LLVM APIs require non-consts.
  llvm::IntegerType* m_int8;
  llvm::IntegerType* m_int16;
  llvm::IntegerType* m_int32;
  llvm::IntegerType* m_int64;

  llvm::PointerType* m_int8Ptr;
  llvm::PointerType* m_int16Ptr;
  llvm::PointerType* m_int32Ptr;
  llvm::PointerType* m_int64Ptr;

  llvm::PointerType* m_int8FSPtr;
  llvm::PointerType* m_int16FSPtr;
  llvm::PointerType* m_int32FSPtr;
  llvm::PointerType* m_int64FSPtr;

  // Commonly used constants. No const either.
  llvm::ConstantInt* m_int8Zero;
  llvm::ConstantInt* m_int8One;
  llvm::ConstantInt* m_int16Zero;
  llvm::ConstantInt* m_int16One;
  llvm::ConstantInt* m_int32Zero;
  llvm::ConstantInt* m_int32One;
  llvm::ConstantInt* m_int64Zero;
  llvm::ConstantInt* m_int64One;

  llvm::UndefValue*  m_int64Undef;
};

void LLVMEmitter::emit(const jit::vector<Vlabel>& labels) {
  for (auto label : labels) {
    auto& b = m_unit.blocks[label];
    m_irb.SetInsertPoint(block(label));

    // TODO(#5376594): before rVmFp is SSA-ified we are using the hack below.
    m_valueInfo[Vreg(x64::rVmFp)].llval = m_rVmFp;

    for (auto& inst : b.code) {
      switch (inst.op) {
// This list will eventually go away; for now only a very small subset of
// operations are supported.
#define SUPPORTED_OPS \
O(addq) \
O(addqi) \
O(bindjmp) \
O(defvmsp) \
O(end) \
O(cmpbi) \
O(cmpbim) \
O(cmplim) \
O(cmpq) \
O(cmpqim) \
O(copy) \
O(declm) \
O(decqm) \
O(fallbackcc) \
O(inclm) \
O(incwm) \
O(jcc) \
O(jmp) \
O(ldimm) \
O(lea) \
O(load) \
O(nothrow) \
O(pushm) \
O(ret) \
O(store) \
O(storeb) \
O(storebi) \
O(storel) \
O(storeli) \
O(storeqi) \
O(svcreq) \
O(syncpoint) \
O(syncvmfp) \
O(syncvmsp) \
O(testbi) \
O(testlim) \
O(testq) \
O(ud2) \
O(landingpad)
#define O(name) case Vinstr::name: emit(inst.name##_); break;
  SUPPORTED_OPS
#undef O
#undef SUPPORTED_OPS

      case Vinstr::vcall:
      case Vinstr::vinvoke:
        emitCall(inst);
        break;
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

void LLVMEmitter::emit(const addq& inst) {
  auto result = m_irb.CreateAdd(value(inst.s0), value(inst.s1));
  defineValue(inst.d, result);
}

void LLVMEmitter::emit(const addqi& inst) {
  // TODO(#5134526): ignore %rsp adjustments for now. They are typically
  // emitted in unwind handler.
  if (inst.s1 == reg::rsp) return;
  auto result = m_irb.CreateAdd(value(inst.s1), cns(inst.s0.q()));
  defineValue(inst.d, result);
}

void LLVMEmitter::emit(const bindjmp& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const defvmsp& inst) {
  defineValue(inst.d, value(x64::rVmSp));
}

void LLVMEmitter::emit(const end& inst) {
  // no-op
}

void LLVMEmitter::emitCall(const Vinstr& inst) {
  auto const is_vcall = inst.op == Vinstr::vcall;
  auto const vcall = inst.vcall_;
  auto const vinvoke = inst.vinvoke_;

  // Extract all the relevant information from the appropriate instruction.
  auto const call = is_vcall ? vcall.call : vinvoke.call;
  auto const& vargs = m_unit.vcallArgs[is_vcall ? vcall.args : vinvoke.args];
  auto const dests = m_unit.tuples[is_vcall ? vcall.d : vinvoke.d];
  auto const destType = is_vcall ? vcall.destType : vinvoke.destType;

  // Generate the right function signature to be used by call/invoke.
  // Perhaps caching it can improve the performance.
  llvm::Type* returnType = nullptr;
  switch (destType) {
  case DestType::None:
    returnType = m_irb.getVoidTy();
    break;
  case DestType::SSA:
    returnType = m_int64;
    break;
  case DestType::Dbl:
    returnType = m_irb.getDoubleTy();
    break;
  case DestType::SIMD:
  case DestType::TV:
    returnType = m_typedValueType;
    break;
  }

  std::vector<llvm::Type*> argTypes;
  std::vector<llvm::Value*> args;
  auto doArgs = [&] (const VregList& srcs) {
    for(int i = 0; i < srcs.size(); ++i) {
      args.push_back(value(srcs[i]));
      argTypes.push_back(value(srcs[i])->getType());
    }
  };
  doArgs(vargs.args);
  // Handle special case of TypedValue being put on the stack while not
  // all arg regs have being used. Since TypedValue is already split into
  // regs, we need to manually insert the padding arg.
  if (vargs.stkArgs.size() &&
      vargs.stkArgs[0].isGP() &&
      vargs.args.size() == x64::kNumRegisterArgs - 1) {
    args.push_back(m_int64Undef);
    argTypes.push_back(m_int64);
  }
  doArgs(vargs.simdArgs);
  doArgs(vargs.stkArgs);

  auto const funcType = llvm::FunctionType::get(returnType, argTypes, false);

  llvm::Value* funcPtr = nullptr;
  switch (call.kind()) {
  default:
    throw FailedLLVMCodeGen(
        folly::format("Unsupported call type: {}",
          (int)call.kind()).str());
  case CppCall::Kind::Destructor: {
    assert(vargs.args.size() >= 2);
    llvm::Value* reg = value(vargs.args[1]);
    reg = m_irb.CreateZExt(reg, m_int64, "zext");
    reg = m_irb.CreateLShr(value(vargs.args[1]),
                                 kShiftDataTypeToDestrIndex,
                                 "lshr");
    llvm::Value* destructors =
      m_irb.CreateIntToPtr(cns(size_t(g_destructors)), m_int64Ptr, "conv");
    funcPtr = m_irb.CreateGEP(destructors, reg, "getelem");
    funcPtr = m_irb.CreateBitCast(funcPtr,
                                  llvm::PointerType::get(funcType,0),
                                  "destructor");
    break;
  }
  case CppCall::Kind::Direct:
    std::string funcName = getNativeFunctionName(call.address());
    funcPtr = m_module->getFunction(funcName);
    if (!funcPtr) {
      registerGlobalSymbol(funcName, (uint64_t)call.address());
      funcPtr = llvm::Function::Create(funcType,
                                       llvm::GlobalValue::ExternalLinkage,
                                       funcName,
                                       m_module.get());
    }
  }

  llvm::Instruction* callInst = nullptr;
  if (is_vcall) {
    auto call = m_irb.CreateCall(funcPtr, args);
    call->setCallingConv(llvm::CallingConv::C);
    callInst = call;
  } else {
    auto normal = block(vinvoke.targets[0]);
    auto unwind = block(vinvoke.targets[1]);
    auto invoke = m_irb.CreateInvoke(funcPtr, normal, unwind, args);
    invoke->setCallingConv(llvm::CallingConv::C);
    callInst = invoke;
    // The result can only be used on normal path. The unwind branch cannot
    // access return values.
    m_irb.SetInsertPoint(normal);
  }

  // Extract value(s) from the call.
  switch (destType) {
  case DestType::None:
    // nothing to do
    assert(dests.size() == 0);
    break;
  case DestType::SSA:
  case DestType::Dbl:
    assert(dests.size() == 1);
    defineValue(dests[0], callInst);
    break;
  case DestType::TV: {
    assert(dests.size() == 2);
    if (packed_tv) {
      defineValue(dests[0], m_irb.CreateExtractValue(callInst, 2)); // m_data
      defineValue(dests[1], m_irb.CreateExtractValue(callInst, 1)); // m_type
    } else {
      defineValue(dests[0], m_irb.CreateExtractValue(callInst, 0)); // m_data
      defineValue(dests[1], m_irb.CreateExtractValue(callInst, 1)); // m_type
    }
    break;
  }
  case DestType::SIMD: {
    assert(dests.size() == 1);
    // Do we want to pack it manually into a <2 x i64>? Or bitcast to X86_MMX?
    // Leave it as TypedValue for now and see what LLVM optimizer does.
    defineValue(dests[0], callInst);
    break;
  }
  }
}

void LLVMEmitter::emit(const cmpbi& inst) {
}

void LLVMEmitter::emit(const cmpbim& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 8)));
}

void LLVMEmitter::emit(const cmplim& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 32)));
}

void LLVMEmitter::emit(const cmpq& inst) {
  // no-op. The real work happens in jcc
}

void LLVMEmitter::emit(const cmpqim& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 64)));
}

void LLVMEmitter::emit(const copy& inst) {
  defineValue(inst.d, value(inst.s));
}

void LLVMEmitter::emit(const declm& inst) {
  auto ptr = emitPtr(inst.m, 32);
  auto load = m_irb.CreateLoad(ptr);
  auto sub = m_irb.CreateSub(load, m_int32One);
  defineFlagTmp(inst.sf, sub);
  m_irb.CreateStore(sub, ptr);
}

void LLVMEmitter::emit(const decqm& inst) {
  auto ptr = emitPtr(inst.m, 64);
  auto oldVal = m_irb.CreateLoad(ptr);
  auto newVal = m_irb.CreateSub(oldVal, m_int64One);
  defineFlagTmp(inst.sf, newVal);
  m_irb.CreateStore(newVal, ptr);
}

void LLVMEmitter::emit(const fallbackcc& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const inclm& inst) {
  auto ptr = emitPtr(inst.m, 32);
  auto load = m_irb.CreateLoad(ptr);
  auto add = m_irb.CreateAdd(load, m_int32One);
  defineFlagTmp(inst.sf, add);
  m_irb.CreateStore(add, ptr);
}

void LLVMEmitter::emit(const incwm& inst) {
  auto ptr = emitPtr(inst.m, 16);
  auto oldVal = m_irb.CreateLoad(ptr);
  auto newVal = m_irb.CreateAdd(oldVal, m_int16One);
  defineFlagTmp(inst.sf, newVal);
  m_irb.CreateStore(newVal, ptr);
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

  if (cmp.op == Vinstr::addq) {
    lhs = asInt(value(cmp.addq_.d));
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::addqi) {
    lhs = asInt(value(cmp.addqi_.d));
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::cmpq) {
    lhs = asInt(value(cmp.cmpq_.s1));
    rhs = asInt(value(cmp.cmpq_.s0));
  } else if (cmp.op == Vinstr::cmpbi) {
    lhs = asInt(value(cmp.cmpbi_.s1), 8);
    rhs = cns(cmp.cmpbi_.s0.b());
  } else if (cmp.op == Vinstr::cmpbim) {
    lhs = flagTmp(inst.sf);
    rhs = cns(cmp.cmpbim_.s0.b());
  } else if (cmp.op == Vinstr::cmplim) {
    lhs = flagTmp(inst.sf);
    rhs = cns(cmp.cmplim_.s0.l());
  } else if (cmp.op == Vinstr::cmpqim) {
    lhs = flagTmp(inst.sf);
    rhs = cns(cmp.cmpqim_.s0.q());
  } else if (cmp.op == Vinstr::declm) {
    lhs = flagTmp(inst.sf);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::decqm) {
    lhs = flagTmp(inst.sf);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::inclm) {
    lhs = flagTmp(inst.sf);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::incwm) {
    lhs = flagTmp(inst.sf);
    rhs = m_int16Zero;
  } else if (cmp.op == Vinstr::testbi) {
    lhs = flagTmp(inst.sf);
    rhs = m_int8Zero;
  } else if (cmp.op == Vinstr::testlim) {
    lhs = flagTmp(inst.sf);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::testq) {
    lhs = flagTmp(inst.sf);
    rhs = m_int64Zero;
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
  assert(inst.d.isVirt());
  defineValue(inst.d, cns(inst.s.q()));
}

void LLVMEmitter::emit(const lea& inst) {
  auto value = m_irb.CreatePtrToInt(emitPtr(inst.s, 8), m_int64, "conv");
  defineValue(inst.d, value);
}

void LLVMEmitter::emit(const load& inst) {
  llvm::Value* value = nullptr;
  // Special: load      [%rsp] => dest
  if (inst.s.base == reg::rsp && inst.s.disp == 0 && !inst.s.index.isValid()) {
    value = getReturnAddress();
  } else {
    value = m_irb.CreateLoad(emitPtr(inst.s, 64));
  }
  defineValue(inst.d, value);
}

void LLVMEmitter::emit(const nothrow& inst) {
}

llvm::Value* LLVMEmitter::getReturnAddress() {
  if (!m_llvmReturnAddress) {
    m_llvmReturnAddress =
      llvm::Intrinsic::getDeclaration(m_module.get(),
                                      llvm::Intrinsic::returnaddress);
  }
  auto rac = m_irb.CreateCall(m_llvmReturnAddress, m_int32Zero);
  rac->setCallingConv(llvm::CallingConv::C);
  return m_irb.CreatePtrToInt(rac, m_int64, "retaddr");
}

UNUSED llvm::Value* LLVMEmitter::getFrameAddress() {
  if (!m_llvmFrameAddress) {
    m_llvmFrameAddress =
      llvm::Intrinsic::getDeclaration(m_module.get(),
                                      llvm::Intrinsic::frameaddress);
  }
  auto call = m_irb.CreateCall(m_llvmFrameAddress, m_int32Zero, "framep");
  call->setCallingConv(llvm::CallingConv::C);
  return m_irb.CreatePtrToInt(call, m_int64, "frameaddress");
}

UNUSED llvm::Value* LLVMEmitter::getStackPointer() {
  if (!m_llvmReadRegister) {
    m_llvmReadRegister =
      llvm::Intrinsic::getDeclaration(m_module.get(),
                                      llvm::Intrinsic::read_register,
                                      m_int64);
  }
  auto metadata =
    llvm::MDNode::get(m_context, llvm::MDString::get(m_context, "rsp"));
  auto call = m_irb.CreateCall(m_llvmReadRegister, metadata, "rspcall");
  call->setCallingConv(llvm::CallingConv::C);
  return call;
}

UNUSED void LLVMEmitter::emitAsm(const std::string& asmStatement,
                                 const std::string& asmConstraints,
                                 bool hasSideEffects) {
  auto const funcType =
    llvm::FunctionType::get(m_irb.getVoidTy(), false);
  auto const iasm = llvm::InlineAsm::get(funcType, asmStatement, asmConstraints,
                                         hasSideEffects);
  auto call = m_irb.CreateCall(iasm, "");
  call->setCallingConv(llvm::CallingConv::C);
}

void LLVMEmitter::emit(const pushm& inst) {
  // Expect 'push 0x8(rVmFp)' i.e. the return address.
  always_assert_flog(inst.s.base == Vreg64(x64::rVmFp) && inst.s.disp == 8 &&
      !inst.s.index.isValid() && inst.s.seg == Vptr::DS, "unexpected push");

  // If we use inline asm statement to emit push of the return address,
  // then it will conflict with LLVM's frame whenever it's non-empty.
  auto const ptr = m_irb.CreateBitCast(emitPtr(inst.s, 8),
                                       m_retFuncPtrPtrType,
                                       "bcast");
  m_addressToReturn = m_irb.CreateLoad(ptr);
}

void LLVMEmitter::emit(const ret& inst) {
  if (m_addressToReturn) {
    // Tail call to faux pushed value.
    // (*value)(rVmSp, rVmTl, rVmFp)
    std::vector<llvm::Value*> args =
      { value(x64::rVmSp), value(x64::rVmTl), value(x64::rVmFp) };
    auto callInst = m_irb.CreateCall(m_addressToReturn, args);
    callInst->setCallingConv(llvm::CallingConv::X86_64_HHVM_TC);
    callInst->setTailCall(true);
    m_addressToReturn = nullptr;
  }
  m_irb.CreateRetVoid();
}

void LLVMEmitter::emit(const store& inst) {
  m_irb.CreateStore(value(inst.s), emitPtr(inst.d, 64));
}

void LLVMEmitter::emit(const storeb& inst) {
  m_irb.CreateStore(value(inst.s), emitPtr(inst.m, 8));
}

void LLVMEmitter::emit(const storebi& inst) {
  m_irb.CreateStore(cns(inst.s.b()), emitPtr(inst.m, 8));
}

void LLVMEmitter::emit(const storel& inst) {
  m_irb.CreateStore(value(inst.s), emitPtr(inst.m, 32));
}

void LLVMEmitter::emit(const storeli& inst) {
  m_irb.CreateStore(cns(inst.s.l()), emitPtr(inst.m, 32));
}

void LLVMEmitter::emit(const storeqi& inst) {
  m_irb.CreateStore(cns(inst.s.q()), emitPtr(inst.m, 64));
}

void LLVMEmitter::emit(const svcreq& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const syncpoint& inst) {
}

void LLVMEmitter::emit(const syncvmfp& inst) {
  // Nothing to do really.
}

void LLVMEmitter::emit(const syncvmsp& inst) {
  defineValue(x64::rVmSp, value(inst.s));
}

void LLVMEmitter::emit(const testbi& inst) {
  auto result = m_irb.CreateAnd(m_irb.CreateTruncOrBitCast(value(inst.s1),
                                                           m_int8),
                                inst.s0.b());
  defineFlagTmp(inst.sf, result);
}

void LLVMEmitter::emit(const testlim& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 32));
  auto result = m_irb.CreateAnd(lhs, inst.s0.w());
  defineFlagTmp(inst.sf, result);
}

void LLVMEmitter::emit(const testq& inst) {
  auto result = m_irb.CreateAnd(value(inst.s1), value(inst.s0));
  defineFlagTmp(inst.sf, result);
}

void LLVMEmitter::emit(const ud2& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const landingpad& inst) {
  // This is far from correct, but it's enough to keep the llvm verifier happy
  // for now.
  auto pad = m_irb.CreateLandingPad(m_typedValueType, m_personalityFunc, 0);
  pad->setCleanup(true);
}

void LLVMEmitter::emitTrap() {
  auto trap = llvm::Intrinsic::getDeclaration(m_module.get(),
                                              llvm::Intrinsic::trap);
  m_irb.CreateCall(trap);
  m_irb.CreateUnreachable();
}

llvm::Value* LLVMEmitter::emitPtr(const Vptr s, size_t bits) {
  bool inFS = s.seg == Vptr::FS;
  llvm::Value* ptr = nullptr;
  always_assert(s.base != reg::rsp);
  ptr = s.base.isValid() ? value(s.base) : cns(0);
  auto disp = cns(int64_t{s.disp});
  if (s.index.isValid()) {
    auto scaledIdx = m_irb.CreateMul(value(s.index),
                                     cns(int64_t{s.scale}),
                                     "mul");
    disp = m_irb.CreateAdd(disp, scaledIdx, "add");
  }
  ptr = m_irb.CreateIntToPtr(ptr, inFS ? m_int8FSPtr : m_int8Ptr, "conv");
  ptr = m_irb.CreateGEP(ptr, disp, "getelem");

  if (bits != 8) {
    ptr = m_irb.CreateBitCast(ptr, ptrIntNType(bits, inFS));
  }

  return ptr;
}

llvm::Type* LLVMEmitter::intNType(size_t bits) const {
  switch (bits) {
  default: always_assert(0 && "unsupported bit width");
  case 8:  return m_int8;
  case 16: return m_int16;
  case 32: return m_int32;
  case 64: return m_int64;
  }
}

llvm::Type* LLVMEmitter::ptrIntNType(size_t bits, bool inFS) const {
  switch (bits) {
  default: always_assert(0 && "unsupported bit width");
  case 8:  return inFS ? m_int8FSPtr  : m_int8Ptr;
  case 16: return inFS ? m_int16FSPtr : m_int16Ptr;
  case 32: return inFS ? m_int32FSPtr : m_int32Ptr;
  case 64: return inFS ? m_int64FSPtr : m_int64Ptr;
  }
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

  jit::vector<UndoMarker> undoAll = {UndoMarker(mcg->globalData())};
  for(auto const& area : areas) {
    undoAll.emplace_back(area.code);
  }

  try {
    LLVMEmitter(unit, areas).emit(labels);
    FTRACE(3, "\n{:-^80}\n{}\n",
           " x64 after LLVM codegen ", showNewCode(areas));
  } catch (const FailedLLVMCodeGen& e) {
    FTRACE(1, "LLVM codegen failed: {}\n", e.what());

    // Undo any code/data we may have allocated.
    for(auto& marker : undoAll) {
      marker.undo();
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
