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
#include "hphp/runtime/vm/jit/llvm-stack-maps.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/reserved-stack.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
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

/*
 * Read an unsigned LEB128 value from data, advancing it past the value.
 */
uintptr_t readULEB128(const uint8_t*& data) {
  uintptr_t result = 0;
  uintptr_t shift = 0;
  unsigned char byte;

  do {
    byte = *data++;
    result |= (byte & 0x7f) << shift;
    shift += 7;
  } while (byte & 0x80);

  return result;
}

/*
 * Read a signed LEB128 value from data, advancing it past the value.
 */
uintptr_t readSLEB128(const uint8_t*& data) {
  uintptr_t result = 0;
  uintptr_t shift = 0;
  unsigned char byte;

  do {
    byte = *data++;
    result |= (byte & 0x7f) << shift;
    shift += 7;
  } while (byte & 0x80);

  if ((byte & 0x40) && (shift < (sizeof(result) << 3))) {
    result |= (~0 << shift);
  }

  return result;
}

/*
 * Read and return a T from data, advancing it past the read item.
 */
template<typename T>
T readValue(const uint8_t*& data) {
  T val;
  memcpy(&val, data, sizeof(T));
  data += sizeof(T);
  return val;
}

/*
 * Read an encoded DWARF value from data, advancing it past any data read. This
 * function was adapted from the ExceptionDemo.cpp example in llvm.
 */
uintptr_t readEncodedPointer(const uint8_t*& data, uint8_t encoding) {
  uintptr_t result = 0;
  auto const start = data;

  if (encoding == DW_EH_PE_omit) return result;

  // first get value
  switch (encoding & 0x0F) {
    case DW_EH_PE_absptr:
      result = readValue<uintptr_t>(data);
      break;
    case DW_EH_PE_uleb128:
      result = readULEB128(data);
      break;
    case DW_EH_PE_sleb128:
      result = readSLEB128(data);
      break;
    case DW_EH_PE_udata2:
      result = readValue<uint16_t>(data);
      break;
    case DW_EH_PE_udata4:
      result = readValue<uint32_t>(data);
      break;
    case DW_EH_PE_udata8:
      result = readValue<uint64_t>(data);
      break;
    case DW_EH_PE_sdata2:
      result = readValue<int16_t>(data);
      break;
    case DW_EH_PE_sdata4:
      result = readValue<int32_t>(data);
      break;
    case DW_EH_PE_sdata8:
      result = readValue<int64_t>(data);
      break;
    default:
      not_implemented();
  }

  // then add relative offset
  switch (encoding & 0x70) {
    case DW_EH_PE_absptr:
      // do nothing
      break;
    case DW_EH_PE_pcrel:
      result += reinterpret_cast<uintptr_t>(start);
      break;
    case DW_EH_PE_textrel:
    case DW_EH_PE_datarel:
    case DW_EH_PE_funcrel:
    case DW_EH_PE_aligned:
    default:
      not_implemented();
  }

  // then apply indirection
  if (encoding & 0x80 /*DW_EH_PE_indirect*/) {
    result = *((uintptr_t*)result);
  }

  return result;
}

/*
 * Information parsed out of the .gcc_except_table section. start and
 * landingPad are offsets from the beginning of the function.
 */
struct EHInfo {
  uintptr_t start;
  uintptr_t length;
  uintptr_t landingPad;
};

/*
 * Parse a .gcc_except_table section as generated by LLVM, extracting regions
 * with nonzero landingpads. This function was also adapted from the
 * ExceptionDemo.cpp example in llvm.
 */
jit::vector<EHInfo> parse_gcc_except_table(const uint8_t* ptr) {
  jit::vector<EHInfo> ret;

  FTRACE(2, "Parsing exception table at {}\n", ptr);
  uint8_t lpStartEncoding = *ptr++;

  if (lpStartEncoding != DW_EH_PE_omit) {
    readEncodedPointer(ptr, lpStartEncoding);
  }

  uint8_t ttypeEncoding = *ptr++;

  if (ttypeEncoding != DW_EH_PE_omit) {
    readULEB128(ptr);
  }

  uint8_t         callSiteEncoding = *ptr++;
  uint32_t        callSiteTableLength = readULEB128(ptr);
  const uint8_t*  callSiteTableStart = ptr;
  const uint8_t*  callSiteTableEnd = callSiteTableStart + callSiteTableLength;
  const uint8_t*  callSitePtr = callSiteTableStart;

  while (callSitePtr < callSiteTableEnd) {
    uintptr_t start = readEncodedPointer(callSitePtr, callSiteEncoding);
    uintptr_t length = readEncodedPointer(callSitePtr, callSiteEncoding);
    uintptr_t landingPad = readEncodedPointer(callSitePtr, callSiteEncoding);

    uintptr_t actionEntry = readULEB128(callSitePtr);
    // 0 indicates a cleanup entry, the only kind we generate
    always_assert(actionEntry == 0);
    if (landingPad == 0) continue;

    FTRACE(2, "Adding entry: [{},{}): landingPad {}\n",
           start, start + length, landingPad);
    ret.emplace_back(EHInfo{start, length, landingPad});
  }

  return ret;
}

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

  ~TCMemoryManager() {
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
    assert_not_implemented(Alignment <= 8);
    std::unique_ptr<uint8_t[]> data{new uint8_t[Size]};

    FTRACE(1, "Allocate {} data section \"{}\" id={} at addr={}, size={},"
           " alignment={}\n",
           IsReadOnly ? "read-only" : "read-write",
           SectionName.str(), SectionID, data.get(), Size, Alignment);
    return m_data.emplace(SectionName.str(),
                          std::move(data)).first->second.get();
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

  const uint8_t* getDataSection(const std::string& name) const {
    return m_data.at(name).get();
  }

private:
  Vasm::AreaList& m_areas;

  std::unordered_map<std::string, std::unique_ptr<uint8_t[]>> m_data;
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

    m_int8FSPtr  = llvm::Type::getInt8PtrTy(m_context,  kFSAddressSpace);
    m_int16FSPtr = llvm::Type::getInt16PtrTy(m_context, kFSAddressSpace);
    m_int32FSPtr = llvm::Type::getInt32PtrTy(m_context, kFSAddressSpace);
    m_int64FSPtr = llvm::Type::getInt64PtrTy(m_context, kFSAddressSpace);

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

    TCMemoryManager* tcMM;
    std::string errStr;
    std::unique_ptr<llvm::ExecutionEngine> ee(
      llvm::EngineBuilder(m_module.get())
      .setErrorStr(&errStr)
      .setUseMCJIT(true)
      .setMCJITMemoryManager(tcMM = new TCMemoryManager(m_areas))
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

    // Now that codegen is done, we need to parse the stack map and
    // gcc_except_table sections and update our own metadata.
    auto const maps = parseStackMaps(tcMM->getDataSection(".llvm_stackmaps"));
    FTRACE(2, "LLVM stackmaps:\n{}", show(maps));
    always_assert(maps.stkSizeRecords.size() == 1);
    auto const funcStart = maps.stkSizeRecords.front().funcAddr;

    processFixups(maps, funcStart);

    auto const ehInfos =
      parse_gcc_except_table(tcMM->getDataSection(".gcc_except_table"));
    processEHInfos(ehInfos, funcStart);
  }

  /*
   * For each entry in m_fixups, find its corresponding stack map entry, find
   * the actual call instruction, and register the fixup.
   */
  void processFixups(const StackMaps& maps, uint8_t* funcStart) {
    for (auto& fix : m_fixups) {
      auto const mapStart = funcStart + maps.records.at(fix.id).offset;
      auto ip = mapStart;
      while (true) {
        DecodedInstruction di(ip);
        ip += di.size();
        if (di.isCall()) break;
      }
      FTRACE(2, "From stackmap at {}, afterCall for fixup = {}\n",
             mapStart, ip);
      mcg->recordSyncPoint(ip, fix.fixup.pcOffset, fix.fixup.spOffset);
    }
  }

  /*
   * For each entry in infos, find all call instructions in the region and
   * register the landing pad as a catch block for each one.
   */
  void processEHInfos(const jit::vector<EHInfo>& infos, uint8_t* funcStart) {
    for (auto& info : infos) {
      auto ip = funcStart + info.start;
      auto const end = ip + info.length;
      auto const landingPad = funcStart + info.landingPad;

      FTRACE(2, "Looking for calls for landingPad {}, in EH region [{},{})\n",
             landingPad, ip, end);
      auto found = false;
      while (ip < end) {
        DecodedInstruction di(ip);
        ip += di.size();
        if (di.isCall()) {
          FTRACE(2, "  afterCall: {}\n", ip);
          mcg->registerCatchBlock(ip, landingPad);
          found = true;
        }
      }

      always_assert(found && "EH region with no calls");
    }
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
   * Assuming val is already an integer type, zero-extend or truncate it to the
   * given size integer type.
   */
  llvm::Value* asInt(llvm::Value* val, size_t bits) {
    assert(val->getType()->isIntegerTy());
    return m_irb.CreateZExtOrTrunc(val, intNType(bits));
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

  struct LLVMFixup {
    uint64_t id;
    Fixup fixup;
  };

  static constexpr unsigned kFSAddressSpace = 257;

#define O(name, ...) void emit(const name&);
VASM_OPCODES
#undef O

  void emitTrap();
  void emitCall(const Vinstr& instr);
  llvm::Value* emitCmpForCC(Vreg sf, ConditionCode cc);

  llvm::Value* emitFuncPtr(const std::string& name,
                           llvm::FunctionType* type,
                           uintptr_t address);

  // Emit code for the pointer. Return value is of the given type, or
  // <i{bits} *> for the second overload.
  llvm::Value* emitPtr(const Vptr s, llvm::Type* ptrTy);
  llvm::Value* emitPtr(const Vptr s, size_t bits = 64);

  llvm::Type* ptrType(llvm::Type* ty, unsigned addressSpace = 0) const;
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

  // Saved LLVM intrinsics.
  llvm::Function* m_llvmFrameAddress{nullptr};
  llvm::Function* m_llvmReadRegister{nullptr};
  llvm::Function* m_llvmReturnAddress{nullptr};
  llvm::Function* m_llvmStackmap{nullptr};

  // Vreg -> RegInfo map
  jit::vector<RegInfo> m_valueInfo;

  // Vlabel -> llvm::BasicBlock map
  jit::vector<llvm::BasicBlock*> m_blocks;

  // Pending Fixups that must be processed after codegen
  jit::vector<LLVMFixup> m_fixups;

  // The next id to use for a stackmap or patchpoint instruction. These ids
  // only need to be unique within this translation unit.
  uint64_t m_nextStackmap{0};

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
O(addli) \
O(addlm) \
O(addq) \
O(addqi) \
O(addqim) \
O(addsd) \
O(andb) \
O(andbi) \
O(andbim) \
O(andl) \
O(andli) \
O(andq) \
O(andqi) \
O(bindjmp) \
O(bindaddr) \
O(debugtrap) \
O(defvmsp) \
O(fallthru) \
O(cloadq) \
O(cmovq) \
O(cmpb) \
O(cmpbi) \
O(cmpbim) \
O(cmpl) \
O(cmpli) \
O(cmplim) \
O(cmplm) \
O(cmpq) \
O(cmpqi) \
O(cmpqim) \
O(cmpqm) \
O(cvttsd2siq) \
O(cvtsi2sd) \
O(cvtsi2sdm) \
O(copy) \
O(copy2) \
O(copyargs) \
O(decl) \
O(declm) \
O(decq) \
O(decqm) \
O(divsd) \
O(imul) \
O(fallbackcc) \
O(incwm) \
O(incl) \
O(inclm) \
O(incq) \
O(incqm) \
O(incqmlock) \
O(jcc) \
O(jmp) \
O(ldimm) \
O(lea) \
O(loaddqu) \
O(load) \
O(loadl) \
O(loadsd) \
O(loadzbl) \
O(movb) \
O(movl) \
O(movzbl) \
O(mulsd) \
O(mul) \
O(neg) \
O(nop) \
O(not) \
O(orq) \
O(orqi) \
O(orqim) \
O(roundsd) \
O(srem) \
O(sar) \
O(sarqi) \
O(setcc) \
O(shlli) \
O(shl) \
O(shlqi) \
O(shrli) \
O(shrqi) \
O(sqrtsd) \
O(store) \
O(storeb) \
O(storebi) \
O(storedqu) \
O(storel) \
O(storeli) \
O(storeqi) \
O(storesd) \
O(storew) \
O(storewi) \
O(subl) \
O(subli) \
O(subq) \
O(subqi) \
O(subsd) \
O(svcreq) \
O(syncvmfp) \
O(syncvmsp) \
O(testb) \
O(testbi) \
O(testbim) \
O(testl) \
O(testli) \
O(testlim) \
O(testq) \
O(testqm) \
O(testqim) \
O(ud2) \
O(xorb) \
O(xorbi) \
O(xorq) \
O(xorqi) \
O(landingpad) \
O(ldretaddr) \
O(retctrl)
#define O(name) case Vinstr::name: emit(inst.name##_); break;
  SUPPORTED_OPS
#undef O
#undef SUPPORTED_OPS

      case Vinstr::vcall:
      case Vinstr::vinvoke:
        emitCall(inst);
        break;

      // These instructions are intentionally unsupported for a variety of
      // reasons, and if code-gen-x64.cpp emits one it's a bug:
      case Vinstr::call:
      case Vinstr::callm:
      case Vinstr::callr:
      case Vinstr::unwind:
      case Vinstr::nothrow:
      case Vinstr::syncpoint:
      case Vinstr::cqo:
      case Vinstr::idiv:
      case Vinstr::sarq:
      case Vinstr::shlq:
      case Vinstr::ret:
      case Vinstr::push:
      case Vinstr::pushm:
      case Vinstr::pop:
      case Vinstr::popm:
        always_assert_flog(false,
                           "Banned opcode in B{}: {}",
                           size_t(label), show(m_unit, inst));

      // ARM opcodes:
      case Vinstr::asrv:
      case Vinstr::brk:
      case Vinstr::cbcc:
      case Vinstr::hcsync:
      case Vinstr::hcnocatch:
      case Vinstr::hcunwind:
      case Vinstr::hostcall:
      case Vinstr::lslv:
      case Vinstr::tbcc:
      // Fallthrough. Eventually we won't have a default case.
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

void LLVMEmitter::emit(const addli& inst) {
  defineValue(inst.d, m_irb.CreateAdd(cns(inst.s0.l()), value(inst.s1)));
}

void LLVMEmitter::emit(const addlm& inst) {
  auto ptr = emitPtr(inst.m, 32);
  auto result = m_irb.CreateAdd(value(inst.s0), m_irb.CreateLoad(ptr));
  defineFlagTmp(inst.sf, result);
  m_irb.CreateStore(result, ptr);
}

void LLVMEmitter::emit(const addq& inst) {
  defineValue(inst.d, m_irb.CreateAdd(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const addqi& inst) {
  // TODO(#5134526): ignore %rsp adjustments for now. They are typically
  // emitted in unwind handler.
  if (inst.s1 == reg::rsp) return;
  defineValue(inst.d, m_irb.CreateAdd(value(inst.s1), cns(inst.s0.q())));
}

void LLVMEmitter::emit(const addqim& inst) {
  auto ptr = emitPtr(inst.m, 64);
  auto result = m_irb.CreateAdd(cns(inst.s0.q()), m_irb.CreateLoad(ptr));
  defineFlagTmp(inst.sf, result);
  m_irb.CreateStore(result, ptr);
}

void LLVMEmitter::emit(const addsd& inst) {
  defineValue(inst.d, m_irb.CreateFAdd(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const andb& inst) {
  defineValue(inst.d, m_irb.CreateAnd(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const andbi& inst) {
  defineValue(inst.d, m_irb.CreateAnd(cns(inst.s0.b()), value(inst.s1)));
}

void LLVMEmitter::emit(const andbim& inst) {
  auto ptr = emitPtr(inst.m, 8);
  auto result = m_irb.CreateAnd(cns(inst.s.b()), m_irb.CreateLoad(ptr));
  defineFlagTmp(inst.sf, result);
  m_irb.CreateStore(result, ptr);
}

void LLVMEmitter::emit(const andl& inst) {
  defineValue(inst.d, m_irb.CreateAnd(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const andli& inst) {
  defineValue(inst.d, m_irb.CreateAnd(cns(inst.s0.l()), value(inst.s1)));
}

void LLVMEmitter::emit(const andq& inst) {
  defineValue(inst.d, m_irb.CreateAnd(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const andqi& inst) {
  defineValue(inst.d, m_irb.CreateAnd(cns(inst.s0.q()), value(inst.s1)));
}


void LLVMEmitter::emit(const bindjmp& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const bindaddr& inst) {
  // inst.dest is a pointer to memory allocated in globalData, so we can just
  // do what vasm does here.

  mcg->setJmpTransID((TCA)inst.dest);
  *inst.dest = emitEphemeralServiceReq(
    mcg->code.frozen(),
    mcg->getFreeStub(mcg->code.frozen(), &mcg->cgFixups()),
    REQ_BIND_ADDR,
    inst.dest,
    inst.sk.toAtomicInt(),
    TransFlags{}.packed
  );
  mcg->cgFixups().m_codePointers.insert(inst.dest);
}

void LLVMEmitter::emit(const defvmsp& inst) {
  defineValue(inst.d, value(x64::rVmSp));
}

void LLVMEmitter::emit(const fallthru& inst) {
  // no-op
}

llvm::Value* LLVMEmitter::emitFuncPtr(const std::string& name,
                                      llvm::FunctionType* type,
                                      uintptr_t address) {
  auto funcPtr = m_module->getFunction(name);
  if (!funcPtr) {
    registerGlobalSymbol(name, address);
    funcPtr = llvm::Function::Create(type,
                                     llvm::GlobalValue::ExternalLinkage,
                                     name,
                                     m_module.get());
  }
  return funcPtr;
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
  auto const fixup = is_vcall ? vcall.fixup : vinvoke.fixup;

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

  std::vector<llvm::Type*> argTypes = { m_int64 };
  std::vector<llvm::Value*> args = { value(x64::rVmFp) };
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
    funcPtr = emitFuncPtr(getNativeFunctionName(call.address()),
                          funcType,
                          uintptr_t(call.address()));
  }

  if (fixup.isValid()) {
    if (!m_llvmStackmap) {
      m_llvmStackmap = llvm::Function::Create(
        llvm::FunctionType::get(
          m_irb.getVoidTy(),
          std::vector<llvm::Type*>({m_int64, m_int32}),
          true
        ),
        llvm::Function::ExternalLinkage,
        "llvm.experimental.stackmap",
        m_module.get()
      );
    }

    auto id = m_nextStackmap++;
    m_fixups.emplace_back(LLVMFixup{id, fixup});
    m_irb.CreateCall2(m_llvmStackmap, cns(id), m_int32Zero);
  }

  llvm::Instruction* callInst = nullptr;
  if (is_vcall) {
    auto call = m_irb.CreateCall(funcPtr, args);
    call->setCallingConv(llvm::CallingConv::X86_64_HHVM_C);
    callInst = call;
  } else {
    auto normal = block(vinvoke.targets[0]);
    auto unwind = block(vinvoke.targets[1]);
    auto invoke = m_irb.CreateInvoke(funcPtr, normal, unwind, args);
    invoke->setCallingConv(llvm::CallingConv::X86_64_HHVM_C);
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

void LLVMEmitter::emit(const cloadq& inst) {
  auto trueVal = m_irb.CreateLoad(emitPtr(inst.t, 64));
  auto falseVal = value(inst.f);
  auto cond = emitCmpForCC(inst.sf, inst.cc);
  defineValue(inst.d, m_irb.CreateSelect(cond, trueVal, falseVal));
}

void LLVMEmitter::emit(const cmovq& inst) {
  auto cond = emitCmpForCC(inst.sf, inst.cc);
  defineValue(inst.d, m_irb.CreateSelect(cond, value(inst.t), value(inst.f)));
}

void LLVMEmitter::emit(const cmpb& inst) {
  // no-op. The real work for this and other non-memory cmps happens in
  // emitCmpForCC.
}

void LLVMEmitter::emit(const cmpbi& inst) {
  // no-op.
}

void LLVMEmitter::emit(const cmpbim& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 8)));
}

void LLVMEmitter::emit(const cmpl& inst) {
  // no-op.
}

void LLVMEmitter::emit(const cmpli& inst) {
  // no-op.
}

void LLVMEmitter::emit(const cmplim& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 32)));
}

void LLVMEmitter::emit(const cmplm& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 32)));
}

void LLVMEmitter::emit(const cmpq& inst) {
  // no-op.
}

void LLVMEmitter::emit(const cmpqi& inst) {
  // no-op.
}

void LLVMEmitter::emit(const cmpqim& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 64)));
}

void LLVMEmitter::emit(const cmpqm& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 64)));
}

void LLVMEmitter::emit(const cvttsd2siq& inst) {
  defineValue(inst.d, m_irb.CreateFPToSI(value(inst.s), m_int64));
}

void LLVMEmitter::emit(const cvtsi2sd& inst) {
  defineValue(inst.d, m_irb.CreateSIToFP(value(inst.s), m_irb.getDoubleTy()));
}

void LLVMEmitter::emit(const cvtsi2sdm& inst) {
  auto intVal = m_irb.CreateLoad(emitPtr(inst.s, 64));
  defineValue(inst.d, m_irb.CreateSIToFP(intVal, m_irb.getDoubleTy()));
}

void LLVMEmitter::emit(const copy& inst) {
  defineValue(inst.d, value(inst.s));
}

void LLVMEmitter::emit(const copy2& inst) {
  defineValue(inst.d0, value(inst.s0));
  defineValue(inst.d1, value(inst.s1));
}

void LLVMEmitter::emit(const copyargs& inst) {
  auto& srcs = m_unit.tuples[inst.s];
  auto& dsts = m_unit.tuples[inst.d];
  assert(srcs.size() == dsts.size());
  for (unsigned i = 0, n = srcs.size(); i < n; ++i) {
    defineValue(dsts[i], value(srcs[i]));
  }
}

void LLVMEmitter::emit(const debugtrap& inst) {
  auto trap = llvm::Intrinsic::getDeclaration(m_module.get(),
                                              llvm::Intrinsic::debugtrap);
  m_irb.CreateCall(trap);
  m_irb.CreateUnreachable();
}

void LLVMEmitter::emit(const decl& inst) {
  defineValue(inst.d, m_irb.CreateSub(value(inst.s), m_int32One));
}

void LLVMEmitter::emit(const declm& inst) {
  auto ptr = emitPtr(inst.m, 32);
  auto load = m_irb.CreateLoad(ptr);
  auto sub = m_irb.CreateSub(load, m_int32One);
  defineFlagTmp(inst.sf, sub);
  m_irb.CreateStore(sub, ptr);
}

void LLVMEmitter::emit(const decq& inst) {
  defineValue(inst.d, m_irb.CreateSub(value(inst.s), m_int64One));
}

void LLVMEmitter::emit(const decqm& inst) {
  auto ptr = emitPtr(inst.m, 64);
  auto oldVal = m_irb.CreateLoad(ptr);
  auto newVal = m_irb.CreateSub(oldVal, m_int64One);
  defineFlagTmp(inst.sf, newVal);
  m_irb.CreateStore(newVal, ptr);
}

void LLVMEmitter::emit(const divsd& inst) {
  defineValue(inst.d, m_irb.CreateFDiv(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const imul& inst) {
  defineValue(inst.d, m_irb.CreateMul(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const fallbackcc& inst) {
  auto blockName = m_irb.GetInsertBlock()->getName().str();
  auto taken =
    llvm::BasicBlock::Create(m_context,
                             folly::to<std::string>(blockName, "_guard"),
                             m_function);
  auto next =
    llvm::BasicBlock::Create(m_context,
                             folly::to<std::string>(blockName, '_'),
                             m_function);
  auto cond = emitCmpForCC(inst.sf, inst.cc);
  m_irb.CreateCondBr(cond, taken, next);

  m_irb.SetInsertPoint(taken);
  // We don't yet support smashing jumps, so trap if the guard fails.
  emitTrap();

  m_irb.SetInsertPoint(next);
}

void LLVMEmitter::emit(const incwm& inst) {
  auto ptr = emitPtr(inst.m, 16);
  auto oldVal = m_irb.CreateLoad(ptr);
  auto newVal = m_irb.CreateAdd(oldVal, m_int16One);
  defineFlagTmp(inst.sf, newVal);
  m_irb.CreateStore(newVal, ptr);
}

void LLVMEmitter::emit(const incl& inst) {
  defineValue(inst.d, m_irb.CreateAdd(value(inst.s), m_int32One));
}

void LLVMEmitter::emit(const inclm& inst) {
  auto ptr = emitPtr(inst.m, 32);
  auto load = m_irb.CreateLoad(ptr);
  auto add = m_irb.CreateAdd(load, m_int32One);
  defineFlagTmp(inst.sf, add);
  m_irb.CreateStore(add, ptr);
}

void LLVMEmitter::emit(const incq& inst) {
  defineValue(inst.d, m_irb.CreateAdd(value(inst.s), m_int64One));
}

void LLVMEmitter::emit(const incqm& inst) {
  auto ptr = emitPtr(inst.m, 64);
  auto load = m_irb.CreateLoad(ptr);
  auto add = m_irb.CreateAdd(load, m_int64One);
  defineFlagTmp(inst.sf, add);
  m_irb.CreateStore(add, ptr);
}

void LLVMEmitter::emit(const incqmlock& inst) {
  auto ptr = emitPtr(inst.m, 64);
  m_irb.CreateAtomicRMW(llvm::AtomicRMWInst::Add, ptr,
                        m_int64One, llvm::SequentiallyConsistent);
  // Unlike the other inc*m instruction, we don't define a flagTmp here. The
  // value returned by llvm's atomicrmw is the old value, while the x64 incq
  // instruction this is based on sets flags based on the new value. Nothing
  // currently consumes the sf from an incqmlock instruction; if this changes
  // we'll deal with it then.
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

llvm::Value* LLVMEmitter::emitCmpForCC(Vreg sf, ConditionCode cc) {
  auto& cmp = defInst(sf);
  llvm::Value* lhs = nullptr;
  llvm::Value* rhs = nullptr;

  if (cmp.op == Vinstr::addq) {
    lhs = asInt(value(cmp.addq_.d), 64);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::addqi) {
    lhs = asInt(value(cmp.addqi_.d), 64);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::addqim) {
    lhs = flagTmp(sf);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::cmpb) {
    lhs = asInt(value(cmp.cmpb_.s1), 8);
    rhs = asInt(value(cmp.cmpb_.s0), 8);
  } else if (cmp.op == Vinstr::cmpbi) {
    lhs = asInt(value(cmp.cmpbi_.s1), 8);
    rhs = cns(cmp.cmpbi_.s0.b());
  } else if (cmp.op == Vinstr::cmpbim) {
    lhs = flagTmp(sf);
    rhs = cns(cmp.cmpbim_.s0.b());
  } else if (cmp.op == Vinstr::cmpl) {
    lhs = asInt(value(cmp.cmpl_.s1), 32);
    rhs = asInt(value(cmp.cmpl_.s0), 32);
  } else if (cmp.op == Vinstr::cmpli) {
    lhs = asInt(value(cmp.cmpli_.s1), 32);
    rhs = cns(cmp.cmpli_.s0.l());
  } else if (cmp.op == Vinstr::cmplim) {
    lhs = flagTmp(sf);
    rhs = cns(cmp.cmplim_.s0.l());
  } else if (cmp.op == Vinstr::cmplm) {
    lhs = flagTmp(sf);
    rhs = asInt(value(cmp.cmplm_.s0), 32);
  } else if (cmp.op == Vinstr::cmpq) {
    lhs = asInt(value(cmp.cmpq_.s1), 64);
    rhs = asInt(value(cmp.cmpq_.s0), 64);
  } else if (cmp.op == Vinstr::cmpqi) {
    lhs = asInt(value(cmp.cmpqi_.s1), 64);
    rhs = cns(cmp.cmpqi_.s0.q());
  } else if (cmp.op == Vinstr::cmpqim) {
    lhs = flagTmp(sf);
    rhs = cns(cmp.cmpqim_.s0.q());
  } else if (cmp.op == Vinstr::cmpqm) {
    lhs = flagTmp(sf);
    rhs = asInt(value(cmp.cmpqm_.s0), 64);
  } else if (cmp.op == Vinstr::decl) {
    lhs = asInt(value(cmp.decl_.d), 32);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::declm) {
    lhs = flagTmp(sf);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::decq) {
    lhs = asInt(value(cmp.decq_.d), 64);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::decqm) {
    lhs = flagTmp(sf);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::inclm) {
    lhs = flagTmp(sf);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::incwm) {
    lhs = flagTmp(sf);
    rhs = m_int16Zero;
  } else if (cmp.op == Vinstr::subl) {
    lhs = asInt(value(cmp.subl_.d), 32);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::subli) {
    lhs = asInt(value(cmp.subli_.d), 32);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::subq) {
    lhs = asInt(value(cmp.subq_.d), 64);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::subqi) {
    lhs = asInt(value(cmp.subqi_.d), 64);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::testb ||
             cmp.op == Vinstr::testbi ||
             cmp.op == Vinstr::testbim) {
    lhs = flagTmp(sf);
    rhs = m_int8Zero;
  } else if (cmp.op == Vinstr::testl ||
             cmp.op == Vinstr::testli ||
             cmp.op == Vinstr::testlim) {
    lhs = flagTmp(sf);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::testq ||
             cmp.op == Vinstr::testqm ||
             cmp.op == Vinstr::testqim) {
    lhs = flagTmp(sf);
    rhs = m_int64Zero;
  } else {
    always_assert_flog(false, "Unsupported flags src: {}",
                       show(m_unit, cmp));
  }

  return m_irb.CreateICmp(ccToPred(cc), lhs, rhs);
}

void LLVMEmitter::emit(const jcc& inst) {
  auto cond = emitCmpForCC(inst.sf, inst.cc);
  auto next  = block(inst.targets[0]);
  auto taken = block(inst.targets[1]);

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

void LLVMEmitter::emit(const loaddqu& inst) {
  // This will need to change if we ever use loaddqu with values that aren't
  // TypedValues. Ideally, we'd leave this kind of decision to llvm anyway.
  auto value = m_irb.CreateLoad(emitPtr(inst.s, ptrType(m_typedValueType)));
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

void LLVMEmitter::emit(const loadl& inst) {
  defineValue(inst.d, m_irb.CreateLoad(emitPtr(inst.s, 32)));
}

void LLVMEmitter::emit(const loadsd& inst) {
  defineValue(inst.d,
              m_irb.CreateLoad(emitPtr(inst.s, ptrType(m_irb.getDoubleTy()))));
}

void LLVMEmitter::emit(const loadzbl& inst) {
  // loadzbl writes all 64 bits of its destination, despite the name
  auto byteVal = m_irb.CreateLoad(emitPtr(inst.s, 8));
  defineValue(inst.d, m_irb.CreateZExt(byteVal, m_int64));
}

void LLVMEmitter::emit(const movb& inst) {
  defineValue(inst.d, value(inst.s));
}

void LLVMEmitter::emit(const movl& inst) {
  defineValue(inst.d, value(inst.s));
}

void LLVMEmitter::emit(const movzbl& inst) {
  // movzbl writes all 64 bits of its destination, despite the name
  defineValue(inst.d, m_irb.CreateZExt(value(inst.s), m_int64));
}

void LLVMEmitter::emit(const mulsd& inst) {
  defineValue(inst.d, m_irb.CreateFMul(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const mul& inst) {
  defineValue(inst.d, m_irb.CreateFMul(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const neg& inst) {
  defineValue(inst.d, m_irb.CreateSub(m_int64Zero, value(inst.s)));
}

void LLVMEmitter::emit(const nop& inst) {
}

void LLVMEmitter::emit(const not& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s), cns(int64_t{-1})));
}

void LLVMEmitter::emit(const orq& inst) {
  defineValue(inst.d, m_irb.CreateOr(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const orqi& inst) {
  defineValue(inst.d, m_irb.CreateOr(cns(inst.s0.q()), value(inst.s1)));
}

void LLVMEmitter::emit(const orqim& inst) {
  auto ptr = emitPtr(inst.m, 64);
  auto value = m_irb.CreateOr(cns(inst.s0.q()), m_irb.CreateLoad(ptr));
  defineFlagTmp(inst.sf, value);
  m_irb.CreateStore(value, ptr);
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

void LLVMEmitter::emit(const ldretaddr& inst) {
  auto const ptr = m_irb.CreateBitCast(emitPtr(inst.s, 8),
                                       m_retFuncPtrPtrType,
                                       "bcast");
  defineValue(inst.d, m_irb.CreateLoad(ptr));
}

void LLVMEmitter::emit(const retctrl& inst) {
  // "Return" with a tail call to the loaded address
  // (*value)(rVmSp, rVmTl, rVmFp)
  std::vector<llvm::Value*> args =
    { value(x64::rVmSp), value(x64::rVmTl), value(x64::rVmFp) };
  auto callInst = m_irb.CreateCall(value(inst.s), args);
  callInst->setCallingConv(llvm::CallingConv::X86_64_HHVM_TC);
  callInst->setTailCallKind(llvm::CallInst::TCK_MustTail);
  m_irb.CreateRetVoid();
}

void LLVMEmitter::emit(const roundsd& inst) {
  auto roundID = [&]{
    switch (inst.dir) {
      case RoundDirection::nearest:
        return llvm::Intrinsic::round;

      case RoundDirection::floor:
        return llvm::Intrinsic::floor;

      case RoundDirection::ceil:
        return llvm::Intrinsic::ceil;

      case RoundDirection::truncate:
        return llvm::Intrinsic::trunc;
    }
    not_reached();
  }();

  auto func = llvm::Intrinsic::getDeclaration(m_module.get(), roundID);
  defineValue(inst.d, m_irb.CreateCall(func, value(inst.s)));
}

void LLVMEmitter::emit(const srem& inst) {
  defineValue(inst.d, m_irb.CreateSRem(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const sar& inst) {
  defineValue(inst.d, m_irb.CreateAShr(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const sarqi& inst) {
  defineValue(inst.d, m_irb.CreateAShr(value(inst.s1), inst.s0.q()));
}

void LLVMEmitter::emit(const setcc& inst) {
  defineValue(inst.d, emitCmpForCC(inst.sf, inst.cc));
}

void LLVMEmitter::emit(const shlli& inst) {
  defineValue(inst.d, m_irb.CreateShl(value(inst.s1), inst.s0.q()));
}

void LLVMEmitter::emit(const shl& inst) {
  defineValue(inst.d, m_irb.CreateShl(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const shlqi& inst) {
  defineValue(inst.d, m_irb.CreateShl(value(inst.s1), inst.s0.q()));
}

void LLVMEmitter::emit(const shrli& inst) {
  defineValue(inst.d, m_irb.CreateLShr(value(inst.s1), inst.s0.q()));
}

void LLVMEmitter::emit(const shrqi& inst) {
  defineValue(inst.d, m_irb.CreateLShr(value(inst.s1), inst.s0.q()));
}

void LLVMEmitter::emit(const sqrtsd& inst) {
  auto sqrtFunc = llvm::Intrinsic::getDeclaration(m_module.get(),
                                                  llvm::Intrinsic::sqrt);
  defineValue(inst.d, m_irb.CreateCall(sqrtFunc, value(inst.s)));
}

void LLVMEmitter::emit(const store& inst) {
  m_irb.CreateStore(value(inst.s), emitPtr(inst.d, 64));
}

void LLVMEmitter::emit(const storeb& inst) {
  m_irb.CreateStore(m_irb.CreateZExtOrTrunc(value(inst.s), m_int8),
                    emitPtr(inst.m, 8));
}

void LLVMEmitter::emit(const storebi& inst) {
  m_irb.CreateStore(cns(inst.s.b()), emitPtr(inst.m, 8));
}

void LLVMEmitter::emit(const storedqu& inst) {
  // Like loaddqu, this will need to change if we ever use storedqu with values
  // that aren't TypedValues.
  m_irb.CreateStore(value(inst.s), emitPtr(inst.m, ptrType(m_typedValueType)));
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

void LLVMEmitter::emit(const storesd& inst) {
  m_irb.CreateStore(value(inst.s),
                    emitPtr(inst.m, ptrType(m_irb.getDoubleTy())));
}

void LLVMEmitter::emit(const storew& inst) {
  m_irb.CreateStore(value(inst.s), emitPtr(inst.m, 16));
}

void LLVMEmitter::emit(const storewi& inst) {
  m_irb.CreateStore(cns(inst.s.w()), emitPtr(inst.m, 16));
}

void LLVMEmitter::emit(const subl& inst) {
  defineValue(inst.d, m_irb.CreateSub(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const subli& inst) {
  defineValue(inst.d, m_irb.CreateSub(value(inst.s1), cns(inst.s0.l())));
}

void LLVMEmitter::emit(const subq& inst) {
  defineValue(inst.d, m_irb.CreateSub(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const subqi& inst) {
  defineValue(inst.d, m_irb.CreateSub(value(inst.s1), cns(inst.s0.q())));
}

void LLVMEmitter::emit(const subsd& inst) {
  defineValue(inst.d, m_irb.CreateFSub(value(inst.s1), value(inst.s0)));
}

/*
 * To leave the TC and perform a service request, translated code is supposed
 * to execute a ret instruction after populating the right registers. There are
 * a number of different ways to do this, but the most straightforward for now
 * is to do a tail call with the right calling convention to a stub with a
 * single ret instruction. Rather than emitting a dedicated stub for this, we
 * just reuse the ret at the end of enterTCHelper().
 */
extern "C" void enterTCReturn();

void LLVMEmitter::emit(const svcreq& inst) {
  std::vector<llvm::Value*> args{
    value(x64::rVmSp),
    value(x64::rVmTl),
    value(x64::rVmFp),
    cns(reinterpret_cast<uintptr_t>(inst.stub_block)),
    cns(uint64_t{inst.req})
  };
  for (auto arg : m_unit.tuples[inst.args]) {
    args.push_back(value(arg));
  }

  std::vector<llvm::Type*> argTypes(args.size(), m_int64);
  auto funcType = llvm::FunctionType::get(m_irb.getVoidTy(), argTypes, false);
  auto func = emitFuncPtr(folly::to<std::string>("enterTCServiceReqLLVM_",
                                                 argTypes.size()),
                          funcType,
                          uintptr_t(enterTCReturn));
  auto call = m_irb.CreateCall(func, args);
  call->setCallingConv(llvm::CallingConv::X86_64_HHVM_SR);
  call->setTailCallKind(llvm::CallInst::TCK_Tail);
  m_irb.CreateRetVoid();
}

void LLVMEmitter::emit(const syncvmfp& inst) {
  // Nothing to do really.
}

void LLVMEmitter::emit(const syncvmsp& inst) {
  defineValue(x64::rVmSp, value(inst.s));
}

void LLVMEmitter::emit(const testb& inst) {
  auto result = m_irb.CreateAnd(asInt(value(inst.s1), 8),
                                asInt(value(inst.s0), 8));
  defineFlagTmp(inst.sf, result);
}

void LLVMEmitter::emit(const testbi& inst) {
  auto result = m_irb.CreateAnd(asInt(value(inst.s1), 8), inst.s0.b());
  defineFlagTmp(inst.sf, result);
}

void LLVMEmitter::emit(const testbim& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 8));
  defineFlagTmp(inst.sf, m_irb.CreateAnd(lhs, inst.s0.b()));
}

void LLVMEmitter::emit(const testl& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateAnd(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const testli& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateAnd(value(inst.s1), inst.s0.l()));
}

void LLVMEmitter::emit(const testlim& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 32));
  defineFlagTmp(inst.sf, m_irb.CreateAnd(lhs, inst.s0.w()));
}

void LLVMEmitter::emit(const testq& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateAnd(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const testqm& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 64));
  defineFlagTmp(inst.sf, m_irb.CreateAnd(lhs, value(inst.s0)));
}

void LLVMEmitter::emit(const testqim& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 64));
  defineFlagTmp(inst.sf, m_irb.CreateAnd(lhs, inst.s0.q()));
}

void LLVMEmitter::emit(const ud2& inst) {
  emitTrap();
}

void LLVMEmitter::emit(const xorb& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const xorbi& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s1), inst.s0.b()));
}

void LLVMEmitter::emit(const xorq& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const xorqi& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s1), inst.s0.q()));
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

llvm::Value* LLVMEmitter::emitPtr(const Vptr s, llvm::Type* ptrTy) {
  bool inFS = llvm::cast<llvm::PointerType>(ptrTy)->getAddressSpace() ==
    kFSAddressSpace;
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

  if (ptrTy != m_int8) {
    ptr = m_irb.CreateBitCast(ptr, ptrTy);
  }

  return ptr;
}

llvm::Value* LLVMEmitter::emitPtr(const Vptr s, size_t bits) {
  return emitPtr(s, ptrIntNType(bits, s.seg == Vptr::FS));
}

llvm::Type* LLVMEmitter::ptrType(llvm::Type* ty, unsigned addressSpace) const {
  return llvm::PointerType::get(ty, addressSpace);
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
    always_assert_flog(
      RuntimeOption::EvalJitLLVM < 3,
      "Mandatory LLVM codegen failed with reason `{}' on unit:\n{}",
      e.what(), show(unit)
    );
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
