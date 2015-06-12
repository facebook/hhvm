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

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/thread-init-fini.h"

#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/code-gen-x64.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/llvm-locrecs.h"
#include "hphp/runtime/vm/jit/llvm-stack-maps.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/unwind-x64.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#ifdef USE_LLVM

#include <llvm/ADT/Triple.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/CodeGen/MachineFunctionAnalysis.h>
#include <llvm/CodeGen/Passes.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/ObjectCache.h>
#include <llvm/ExecutionEngine/RuntimeDyld.h>
#include <llvm/IR/AssemblyAnnotationWriter.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/ConstantFolder.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/TypeBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/InitializePasses.h>
#include <llvm/PassManager.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetLibraryInfo.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
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
  TCA start;
  TCA end;
  TCA landingPad;
};

/*
 * Parse a .gcc_except_table section as generated by LLVM, extracting regions
 * with nonzero landingpads. This function was also adapted from the
 * ExceptionDemo.cpp example in llvm.
 */
jit::vector<EHInfo> parse_gcc_except_table(const uint8_t* ptr,
                                           uint8_t* funcStart,
                                           uint8_t* coldStart) {
  jit::vector<EHInfo> ret;

  FTRACE(2, "Parsing exception table at {}\n", ptr);
  uint8_t lpStartEncoding = *ptr++;

  uint8_t* lpStart = funcStart;
  if (lpStartEncoding != DW_EH_PE_omit) {
    lpStart =
      reinterpret_cast<uint8_t*>(readEncodedPointer(ptr, lpStartEncoding));
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
    // 0 or 127 indicate a cleanup entry, the only kinds we generate.
    always_assert(actionEntry == 0 || actionEntry == 127);

    // With a hot/cold splitting we can get a real landing pad at offset 0.
    if (landingPad == UINT32_MAX) continue;
    if (!RuntimeOption::EvalJitLLVMSplitHotCold && landingPad == 0) continue;

    auto rangeBase = funcStart;
    if (actionEntry) {
      rangeBase = coldStart;
      // Skip over nop's at the start of the section since all offsets are
      // recorded from the beginning of "real" code.
      // TODO(#6836929): this has to be removed once we fix the underlying
      // issue with smashable instructions at the start of a section.
      while (42) {
        DecodedInstruction di(rangeBase);
        if (!di.isNop()) break;
        rangeBase += di.size();
      }
    }
    FTRACE(2, "Adding entry: [{},{}): landingPad {}\n",
           rangeBase + start, rangeBase + start + length, lpStart + landingPad);
    ret.emplace_back(EHInfo{rangeBase + start,
                            rangeBase + start + length,
                            lpStart + landingPad});
  }

  return ret;
}

void reportLLVMError(void* data, const std::string& err, bool gen_crash_diag) {
  always_assert_flog(false, "LLVM fatal error: {}", err);
}

InitFiniNode llvmInit(
  []() { llvm::install_fatal_error_handler(reportLLVMError); },
  InitFiniNode::When::ProcessInit);

InitFiniNode llvmExit(llvm::remove_fatal_error_handler,
                      InitFiniNode::When::ProcessExit);

std::string showNewCode(const Vasm::AreaList& areas) DEBUG_ONLY;
std::string showNewCode(const Vasm::AreaList& areas) {
  std::ostringstream str;
  Disasm disasm(Disasm::Options().indent(2));

  for (unsigned i = 0, n = areas.size(); i < n; ++i) {
    auto& area = areas[i];
    auto const start = area.start;
    auto const end = area.code.frontier();

    if (i > 0 && start == areas[i-1].start) continue;

    if (start != end) {
      str << folly::format("emitted {} bytes of code into area {}:\n",
                           end - start, i);
      disasm.disasm(str, start, end);
      str << '\n';
    }
  }

  return str.str();
}

/*
 * TCMemoryManager allows llvm to emit code into the appropriate places in the
 * TC. Currently all code goes into the Main code block.
 */
struct TCMemoryManager : public llvm::RTDyldMemoryManager {
  struct LowMallocDeleter {
    void operator()(uint8_t *ptr) {
      FTRACE(1, "Free memory at addr={}\n", ptr);
      low_free(ptr);
    }
  };

  struct SectionInfo {
    std::unique_ptr<uint8_t[], LowMallocDeleter> data;
    size_t size;
  };

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

    auto areaIndex = SectionName.endswith(".cold") ? AreaIndex::Cold
                                                   : AreaIndex::Main;
    auto& code = m_areas[static_cast<size_t>(areaIndex)].code;
    auto codeSkew = m_codeSkews[static_cast<size_t>(areaIndex)];

    // We override/ignore the alignment and use skew value to compensate.
    uint8_t* ret = code.alloc<uint8_t>(1, Size);
    assertx(Alignment < x64::kCacheLineSize &&
           "alignment exceeds cache line size");
    always_assert(
      codeSkew == (reinterpret_cast<size_t>(ret) & (x64::kCacheLineSize - 1)) &&
      "drift in code skew detected");

    FTRACE(1, "Allocate code section \"{}\" id={} at addr={}, size={},"
           " alignment={}, skew={}\n",
           SectionName.str(), SectionID, ret, Size, Alignment, codeSkew);
    return ret;
  }

  uint8_t* allocateDataSection(
    uintptr_t Size, unsigned Alignment, unsigned SectionID,
    llvm::StringRef SectionName, bool IsReadOnly
  ) override {
    if (SectionName.startswith(".rodata")) {
      // This needs to be allocated somewhere persistent since code might
      // reference it.
      auto ret = mcg->globalData().alloc<uint8_t>(Alignment, Size);
      FTRACE(1, "Allocate rodata section '{}' id={} at addr={}, size={},"
             " alignment={}\n",
             SectionName.str(), SectionID, ret, Size, Alignment);
      return ret;
    }

    assert_not_implemented(Alignment <= 8);
    // Some of the data sections will use 32-bit relocations to reference
    // code, thus we have to make sure they are allocated close to code.
    std::unique_ptr<uint8_t[], LowMallocDeleter>
      data{new (low_malloc(Size)) uint8_t[Size], LowMallocDeleter()};

    FTRACE(1, "Allocate {} data section \"{}\" id={} at addr={}, size={},"
           " alignment={}\n",
           IsReadOnly ? "read-only" : "read-write",
           SectionName.str(), SectionID, data.get(), Size, Alignment);
    auto it = m_dataSections.emplace(SectionName.str(),
                                     SectionInfo({std::move(data), Size}));
    return it.first->second.data.get();
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
    auto element = m_symbols.find(name);
    return element == m_symbols.end() ? 0 : element->second;
  }

  /*
   * Register a symbol's name for later lookup by llvm.
   */
  void registerSymbolAddress(const std::string& name, uint64_t address) {
    auto it = m_symbols.emplace(name, address);
    always_assert((it.second == true || it.first->second == address) &&
                  "symbol already registered with a different value");
  }

  /*
   * Append an arbitrary id to the end of the given prefix to make it unique.
   */
  std::string getUniqueSymbolName(const std::string& prefix) {
    auto name = prefix;
    while (m_symbols.count(name)) {
      name = folly::to<std::string>(prefix, '_', m_nextSymbolId++);
    }
    return name;
  }

  const SectionInfo* getDataSection(const std::string& name) const {
    auto it = m_dataSections.find(name);
    return (it == m_dataSections.end()) ? nullptr : &it->second;
  }

  bool hasDataSection(const std::string& name) const {
    return m_dataSections.count(name);
  }

  uint32_t computeCodeSkew(unsigned alignment, AreaIndex area) {
    auto& code = m_areas[static_cast<size_t>(area)].code;
    return m_codeSkews[static_cast<size_t>(area)] =
      reinterpret_cast<uint64_t>(code.frontier()) & (alignment - 1);
  }

  /*
   * Since all of our callees will be within 2GB reach, we can safely tell
   * LLVM to not reserve space for stubs. Otherwise it will create gaps
   * betweeen tracelets.
   */
  virtual bool allowStubAllocation() const override {
    return false;
  }

private:
  Vasm::AreaList& m_areas;

  std::unordered_map<std::string, SectionInfo> m_dataSections;

  jit::vector<uint32_t> m_codeSkews{0,0,0};

  jit::hash_map<std::string, uint64_t> m_symbols;
  uint32_t m_nextSymbolId{0};
};

template<typename T>
std::string llshow(T* val) {
  std::string str;
  {
    llvm::raw_string_ostream os(str);
    val->print(os);
  }
  return str;
}

struct VasmAnnotationWriter : llvm::AssemblyAnnotationWriter {
  explicit VasmAnnotationWriter(const std::vector<std::string>& strs)
    : m_strs(strs)
  {}

  void emitBasicBlockStartAnnot(const llvm::BasicBlock* b,
                                llvm::formatted_raw_ostream& os) override {
    m_curId = -1;
    m_prefix = "";
  }

  void emitInstructionAnnot(const llvm::Instruction* i,
                            llvm::formatted_raw_ostream& os) override {
    SCOPE_EXIT { m_prefix = "\n"; };

    auto dbg = i->getDebugLoc();
    if (dbg.isUnknown() || m_curId == dbg.getLine()) return;

    m_curId = dbg.getLine();
    os << m_prefix << "; " << m_strs[m_curId] << "\n";
  }

 private:
  const std::vector<std::string>& m_strs;
  size_t m_curId;
  const char* m_prefix{nullptr};
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
          std::vector<llvm::Type*>(3, llvm::IntegerType::get(m_context, 64)),
          false),
      llvm::Function::ExternalLinkage, "tracelet", m_module.get()))
    , m_irb(m_context,
            llvm::ConstantFolder(),
            IRBuilderVasmInserter(*this))
    , m_tcMM(new TCMemoryManager(areas))
    , m_valueInfo(unit.next_vr)
    , m_blocks(unit.blocks.size())
    , m_incomingVmFp(unit.blocks.size())
    , m_incomingVmSp(unit.blocks.size())
    , m_unit(unit)
    , m_areas(areas)
  {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    m_function->setCallingConv(llvm::CallingConv::X86_64_HHVM_TC);
    m_function->setAlignment(1);

    if (RuntimeOption::EvalJitLLVMOptSize) {
      m_function->addFnAttr(llvm::Attribute::OptimizeForSize);
    }

    if (RuntimeOption::EvalJitLLVMMinSize) {
      m_function->addFnAttr(llvm::Attribute::MinSize);
    }

    m_irb.SetInsertPoint(
      llvm::BasicBlock::Create(m_context,
                               folly::to<std::string>('B', size_t(unit.entry)),
                               m_function));
    m_blocks[unit.entry] = m_irb.GetInsertBlock();

    auto args = m_function->arg_begin();
    args->setName("rVmSp");
    defineValue(x64::rVmSp, args++);
    args->setName("rVmTl");
    defineValue(x64::rVmTl, args++);
    args->setName("rVmFp");
    defineValue(x64::rVmFp, args++);

    // Commonly used types and values.
    m_void = m_irb.getVoidTy();

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

    m_int8Zero    = m_irb.getInt8(0);
    m_int8NegOne  = m_irb.getInt8(-1);
    m_int8One     = m_irb.getInt8(1);
    m_int16Zero   = m_irb.getInt16(0);
    m_int16NegOne = m_irb.getInt16(-1);
    m_int16One    = m_irb.getInt16(1);
    m_int32Zero   = m_irb.getInt32(0);
    m_int32NegOne = m_irb.getInt32(-1);
    m_int32One    = m_irb.getInt32(1);
    m_int64Zero   = m_irb.getInt64(0);
    m_int64NegOne = m_irb.getInt64(-1);
    m_int64One    = m_irb.getInt64(1);

    m_int64Undef  = llvm::UndefValue::get(m_int64);

    // Register all unit's constants.
    for (auto const& pair : unit.constToReg) {
      switch (pair.first.kind) {
        case Vconst::Quad:
          if (pair.first.isUndef) {
            defineValue(pair.second, m_int64Undef);
          } else {
            defineValue(pair.second, cns(pair.first.val));
          }
          break;
        case Vconst::Long:
          if (pair.first.isUndef) {
            defineValue(pair.second, llvm::UndefValue::get(m_int32));
          } else {
            defineValue(pair.second, cns(int32_t(pair.first.val)));
          }
          break;
        case Vconst::Byte:
          if (pair.first.isUndef) {
            defineValue(pair.second, llvm::UndefValue::get(m_int8));
          } else {
            defineValue(pair.second, cns(uint8_t(pair.first.val)));
          }
          break;
        case Vconst::Double:
          if (pair.first.isUndef) {
            defineValue(pair.second,
                        llvm::UndefValue::get(m_irb.getDoubleTy()));
          } else {
            defineValue(pair.second, cnsDbl(pair.first.doubleVal));
          }
          break;
        case Vconst::ThreadLocal:
          always_assert(false);
          break;
      }
    }

    auto m_personalityFTy = llvm::FunctionType::get(m_int32, false);
    m_personalityFunc =
      llvm::Function::Create(m_personalityFTy,
                             llvm::GlobalValue::ExternalLinkage,
                             "personality0",
                             m_module.get());
    m_personalityFunc->setCallingConv(llvm::CallingConv::C);
    // It's a fake symbol but we still want it in the first 4GB of space to
    // avoid hitting assertions from LLVM relocations.
    m_tcMM->registerSymbolAddress("personality0", 0xbadbad);

    m_int64Pair = llvm::StructType::get(
      m_context, std::vector<llvm::Type*>(2, m_int64), false /* isPacked */
    );
    m_traceletFnTy = llvm::FunctionType::get(
      m_void, std::vector<llvm::Type*>(3, m_int64), false /* isVarArg */
    );
    m_bindcallFnTy = llvm::FunctionType::get(
      m_int64Pair, std::vector<llvm::Type*>(3, m_int64), false /* isVarArg */
    );

    static_assert(offsetof(TypedValue, m_data) == 0, "");
    static_assert(offsetof(TypedValue, m_type) == 8, "");
    m_tvStructType = llvm::StructType::get(
      m_context, std::vector<llvm::Type*>{m_int64, m_int8},
      /*isPacked*/ false
    );
    m_tvVectorType = llvm::VectorType::get(m_int64, 2);

    // The hacky way of tuning LLVM command-line parameters. This has to live
    // till a better debug option control library is implemented.
    static bool isCLSet = false;
    if (!isCLSet) {
      std::vector<std::string> pseudoCL = {
        "hhvm",
        "-disable-block-placement"
      };

      if (RuntimeOption::EvalJitLLVMCondTail) {
        pseudoCL.push_back("-cond-tail-dup");
      }

      if (RuntimeOption::EvalJitLLVMPrintAfterAll ) {
        pseudoCL.push_back("-print-after-all");
      }

      auto const numArgs = pseudoCL.size();
      std::unique_ptr<const char*[]> pseudoCLArray(new const char*[numArgs]);
      for(size_t i = 0; i < pseudoCL.size(); ++i) {
        pseudoCLArray[i] = pseudoCL[i].c_str();
      }

      llvm::cl::ParseCommandLineOptions(numArgs, pseudoCLArray.get(), "");
      isCLSet = true;
    }
  }

  ~LLVMEmitter() {
  }

  std::string showModule(llvm::Module* module) const {
    std::string s;
    llvm::raw_string_ostream stream(s);
    VasmAnnotationWriter vw(m_instStrs);
    module->print(stream,
                  HPHP::Trace::moduleEnabled(Trace::llvm, 5) ? &vw : nullptr);
    return stream.str();
  }

  void verifyModule() const {
    std::string err;
    llvm::raw_string_ostream stream(err);
    always_assert_flog(!llvm::verifyModule(*m_module, &stream),
                       "LLVM verifier failed:\n{}\n", stream.str());
  }

  /*
   * Finalize the code generation process by optimizing and generating code for
   * m_module.
   */
  void finalize() {
    FTRACE(1, "{:-^80}\n{}\n", " LLVM IR before optimizing ",
           showModule(m_module.get()));
    verifyModule();

    llvm::TargetOptions targetOptions;
    targetOptions.EnableFastISel = RuntimeOption::EvalJitLLVMFastISel;
    targetOptions.MCOptions.SplitHotCold =
      RuntimeOption::EvalJitLLVMSplitHotCold;

    auto module = m_module.get();
    auto tcMM = m_tcMM.release();
    auto cpu = RuntimeOption::EvalJitCPU;
    std::vector<std::string> attrs;
    folly::split(" ", RuntimeOption::EvalJitLLVMAttrs, attrs, true);
    if (cpu == "native") cpu = llvm::sys::getHostCPUName();
    FTRACE(1, "Creating ExecutionEngine with CPU '{}'\n", cpu);
    std::string errStr;
    std::unique_ptr<llvm::ExecutionEngine> ee(
      llvm::EngineBuilder(module)
      .setMCPU(cpu)
      .setMAttrs(attrs)
      .setErrorStr(&errStr)
      .setUseMCJIT(true)
      .setMCJITMemoryManager(tcMM)
      .setOptLevel(llvm::CodeGenOpt::Aggressive)
      .setRelocationModel(llvm::Reloc::Static)
      .setCodeModel(llvm::CodeModel::Small)
      .setVerifyModules(true)
      .setTargetOptions(targetOptions)
      .create());
    always_assert_flog(ee, "ExecutionEngine creation failed: {}\n", errStr);

    llvm::PassManager pm;
    if (!RuntimeOption::EvalJitLLVMBasicOpt) {
      auto TLI =
        new llvm::TargetLibraryInfo(llvm::Triple(module->getTargetTriple()));
      pm.add(TLI);
      auto& Registry = *llvm::PassRegistry::getPassRegistry();
      llvm::initializeCore(Registry);
      llvm::initializeScalarOpts(Registry);
      llvm::initializeVectorization(Registry);
      llvm::initializeIPO(Registry);
      llvm::initializeAnalysis(Registry);
      llvm::initializeIPA(Registry);
      llvm::initializeTransformUtils(Registry);
      llvm::initializeInstCombine(Registry);
      llvm::initializeTarget(Registry);
      llvm::initializeCodeGenPreparePass(Registry);
      llvm::initializeAtomicExpandLoadLinkedPass(Registry);
    }

    llvm::LLVMTargetMachine* targetMachine =
      static_cast<llvm::LLVMTargetMachine*>(ee->getTargetMachine());

    module->addModuleFlag(llvm::Module::Error, "code_skew",
                          tcMM->computeCodeSkew(x64::kCacheLineSize,
                                                  AreaIndex::Main));
    module->addModuleFlag(llvm::Module::Error, "cold_code_skew",
                          tcMM->computeCodeSkew(x64::kCacheLineSize,
                                                AreaIndex::Cold));

    // Record start of cold block. Alas there's no better way to find where
    // LLVM-generated cold code goes.
    auto coldStart =
      m_areas[static_cast<size_t>(AreaIndex::Cold)].code.frontier();

    auto fpm = folly::make_unique<llvm::FunctionPassManager>(module);
    fpm->add(new llvm::DataLayoutPass(module));
    if (RuntimeOption::EvalJitLLVMBasicOpt) {
      targetMachine->addAnalysisPasses(*fpm);
      fpm->add(llvm::createBasicAliasAnalysisPass());
      fpm->add(llvm::createUnreachableBlockEliminationPass());
      fpm->add(llvm::createPromoteMemoryToRegisterPass());
      fpm->add(llvm::createInstructionCombiningPass());
      fpm->add(llvm::createReassociatePass());
      fpm->add(llvm::createGVNPass());
      fpm->add(llvm::createCFGSimplificationPass());
    } else {
      llvm::PassManagerBuilder PM;
      PM.OptLevel = RuntimeOption::EvalJitLLVMOptLevel;
      PM.SizeLevel = RuntimeOption::EvalJitLLVMSizeLevel;
      PM.Inliner = llvm::createFunctionInliningPass(PM.OptLevel, PM.SizeLevel);
      PM.DisableUnitAtATime = false;
      PM.SLPVectorize = RuntimeOption::EvalJitLLVMSLPVectorize;
      PM.BBVectorize = RuntimeOption::EvalJitLLVMBBVectorize;
      PM.LoadCombine = RuntimeOption::EvalJitLLVMLoadCombine;
      PM.populateFunctionPassManager(*fpm);
      PM.populateModulePassManager(pm);
    }

    {
      Timer _t(Timer::llvm_optimize);
      fpm->doInitialization();
      for (auto it = module->begin(); it != module->end(); ++it) {
        fpm->run(*it);
      }
      fpm->doFinalization();
      pm.run(*module);
    }

    FTRACE(2, "{:-^80}\n{}\n", " LLVM IR after optimizing ",
           showModule(module));

    {
      Timer _t(Timer::llvm_codegen);
      ee->setProcessAllSections(true);
      ee->finalizeObject();
    }

    SCOPE_EXIT { m_module.release(); };

    if (!RuntimeOption::EvalJitLLVMCompare.empty()) {
      jit::vector<std::string> code;
      Disasm disasm;
      for (auto const& area : m_areas) {
        std::ostringstream str;
        disasm.disasm(str, area.start, area.code.frontier());
        code.emplace_back(str.str());
      }
      throw CompareLLVMCodeGen(std::move(code),
                               showModule(m_module.get()),
                               m_areas[0].code.frontier() - m_areas[0].start);
    }

    if (RuntimeOption::EvalJitLLVMDiscard) return;

    // Now that codegen is done, we need to parse location records and
    // gcc_except_table sections and update our own metadata.
    uint8_t* funcStart =
      static_cast<uint8_t*>(ee->getPointerToFunction(m_function));
    FTRACE(2, "LLVM function address: {}\n", funcStart);
    FTRACE(1, "\n{:-^80}\n{}\n",
           " x64 after LLVM codegen ", showNewCode(m_areas));

    if (auto secLocRecs = tcMM->getDataSection(".llvm_locrecs")) {
      auto const recs = parseLocRecs(secLocRecs->data.get(),
                                     secLocRecs->size);
      FTRACE(2, "LLVM experimental locrecs:\n{}", show(recs));
      processFixups(recs);
      processSvcReqs(recs);
    }

    if (auto secGEH = tcMM->getDataSection(".gcc_except_table")) {
      auto const ehInfos = parse_gcc_except_table(secGEH->data.get(), funcStart,
                                                  coldStart);
      processEHInfos(ehInfos);
    }
  }

  /*
   * For each entry in m_fixups, find its corresponding locrec entry, find
   * the actual call instruction, and register the fixup.
   *
   * NB: LLVM may optimize away call instructions.
   */
  void processFixups(const LocRecs& locRecs) {
    for (auto& fix : m_fixups) {
      auto it = locRecs.records.find(fix.id);
      if (it == locRecs.records.end()) continue;
      for (auto const& record : it->second) {
        uint8_t* ip = record.address;
        DecodedInstruction di(ip);
        if (di.isCall()) {
          auto afterCall = ip + di.size();
          FTRACE(2, "From afterCall for fixup = {}\n", afterCall);
          mcg->recordSyncPoint(afterCall, fix.fixup);
        }
      }
    }
  }

  template<typename L>
  void visitJmps(const jit::vector<LocRecs::LocationRecord>& records,
                 L body) {
    for (auto& record : records) {
      uint8_t* ip = record.address;
      DecodedInstruction di(ip);
      if (di.isBranch()) {
        body(ip, di.isJmp() ? CC_None : di.jccCondCode());
      }
    }
  }

  void processSvcReqs(const LocRecs& locRecs) {
    for (auto& req : m_bindjmps) {
      bool found = false;
      auto doBindJmp = [&] (uint8_t* jmpIp, ConditionCode cc) {
        FTRACE(2, "Processing bindjmp at {}, stub {}\n", jmpIp, req.stub);

        auto jmpLen = (cc == CC_None) ? x64::kJmpLen : x64::kJmpccLen;
        mcg->cgFixups().m_alignFixups.emplace(jmpIp, std::make_pair(jmpLen, 0));
        mcg->setJmpTransID(jmpIp);

        if (found) {
          // If LLVM duplicated the tail call into more than one jmp
          // instruction, we have to create new stubs for the duplicates to
          // avoid reusing the ephemeral stubs before they're done.
          auto& frozen = m_areas[size_t(AreaIndex::Frozen)].code;
          auto optSPOff = folly::Optional<FPInvOffset>{};
          if (!req.target.resumed()) optSPOff = req.spOff;
          auto newStub = emitEphemeralServiceReq(
            frozen,
            mcg->getFreeStub(frozen, &mcg->cgFixups()),
            optSPOff,
            REQ_BIND_JMP,
            RipRelative(jmpIp),
            req.target.toAtomicInt(),
            req.trflags.packed
          );
          FTRACE(2, "Patching tail call at {} to point to {}\n",
                 jmpIp, newStub);

          // Patch the jmp to point to the new stub.
          auto afterJmp = jmpIp + jmpLen;
          auto delta = safe_cast<int32_t>(newStub - afterJmp);
          memcpy(afterJmp - sizeof(delta), &delta, sizeof(delta));
        } else {
          found = true;
          adjustBindJmpPatchableJmpAddress(
            req.stub,
            req.target.resumed(),
            jmpIp
          );
        }
      };

      FTRACE(2, "Processing bindjmp {}, stub {}\n", req.id, req.stub);
      auto it = locRecs.records.find(req.id);
      if (it != locRecs.records.end()) {
        visitJmps(it->second, doBindJmp);
      }

      // The jump was optimized out. Free the stub, but only if it was a reused
      // stub.
      if (!found && req.stubReused) {
        FTRACE(2, "  no corresponding code found. Freeing stub.\n");
        mcg->freeRequestStub(req.stub);
      }
    }

    for (auto& req : m_fallbacks) {
      auto doFallback = [&] (uint8_t* jmpIp, ConditionCode cc) {
        auto destSR = mcg->tx().getSrcRec(req.dest);
        destSR->registerFallbackJump(jmpIp, cc);
      };
      auto it = locRecs.records.find(req.id);
      if (it != locRecs.records.end()) {
        visitJmps(it->second, doFallback);
      }
    }
  }

  /*
   * For each entry in infos, find all call instructions in the region and
   * register the landing pad as a catch block for each one.
   */
  void processEHInfos(const jit::vector<EHInfo>& infos) {
    for (auto& info : infos) {
      FTRACE(2, "Looking for calls for landingPad {}, in EH region [{},{})\n",
             info.landingPad, info.start, info.end);
      auto found = false;
      for(auto ip = info.start; ip < info.end; ) {
        DecodedInstruction di(ip);
        ip += di.size();
        if (di.isCall()) {
          FTRACE(2, "  afterCall: {}\n", ip);
          mcg->registerCatchBlock(ip, info.landingPad);
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
    always_assert_flog(info.llval, "No llvm value exists for {}", show(tmp));
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
      auto& b = m_unit.blocks[l];
      auto name = folly::to<std::string>('B', size_t(l));
      if (b.area == AreaIndex::Cold || b.area == AreaIndex::Frozen) {
        name += ".cold";
      };

      return m_blocks[l] =
        llvm::BasicBlock::Create(m_context, name, m_function);
    }

    return m_blocks[l];
  }

  /*
   * Create a new llvm block using the current block's name with `suffix'
   * appended to it. The block will be inserted at the end of the current
   * function.
   */
  llvm::BasicBlock* makeBlock(folly::StringPiece suffix) {
    auto const curName = m_irb.GetInsertBlock()->getName().str();
    return llvm::BasicBlock::Create(
      m_context, folly::to<std::string>(curName, suffix), m_function
    );
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

  llvm::Value* cnsDbl(double d) {
    return llvm::ConstantFP::get(m_irb.getDoubleTy(), d);
  }

  /*
   * Truncate val to i<bits>
   */
  llvm::Value* narrow(llvm::Value* val, size_t bits) {
    assertx(val->getType()->isIntegerTy());
    return m_irb.CreateTrunc(val, intNType(bits));
  }

  /*
   * Zero-extend val to i<bits>
   */
  llvm::Value* zext(llvm::Value* val, size_t bits) {
    assertx(val->getType()->isIntegerTy());
    return m_irb.CreateZExt(val, intNType(bits));
  }

  /*
   * Bitcast val to Double.
   */
  llvm::Value* asDbl(llvm::Value* val) {
    return m_irb.CreateBitCast(val, m_irb.getDoubleTy());
  }

  /*
   * emit LLVM IR for the given list of vasm blocks.
   */
  void emit(const jit::vector<Vlabel>& labels);

  /*
   * Propagate special-cased physregs along edges to b's successors, creating
   * phi nodes when needed.
   */
  void handleOutgoingPhysRegs(const Vblock& b,
                              const boost::dynamic_bitset<>& backedge_targets);

private:

  /*
   * Custom LLVM IR inserter that can emit inline metadata for tracking
   * vasm origins of IR instructions in debug dumps.
   */
  struct IRBuilderVasmInserter {
    explicit IRBuilderVasmInserter(LLVMEmitter& e)
      : m_emitter(e)
      , m_mdNode(llvm::MDNode::get(m_emitter.m_context,
                                   std::vector<llvm::Value*>{}))
    {}

    void setVinstId(size_t id) { m_instId = id; }

   protected:
    void InsertHelper(llvm::Instruction* I, const llvm::Twine& Name,
                      llvm::BasicBlock* BB,
                      llvm::BasicBlock::iterator InsertPt) const {
      if (BB) BB->getInstList().insert(InsertPt, I);
      I->setName(Name);

      ONTRACE(5, I->setDebugLoc(llvm::DebugLoc::get(m_instId, 0, m_mdNode)));
    }

   private:
    LLVMEmitter& m_emitter;
    llvm::MDNode* m_mdNode;
    size_t m_instId{0};
  };

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

  struct LLVMBindJmp {
    uint32_t id;
    TCA stub;
    bool stubReused;
    SrcKey target;
    FPInvOffset spOff;
    TransFlags trflags;
  };

  struct LLVMFallback {
    uint32_t id;
    SrcKey dest;
  };

  /*
   * PhiInfo is responsible for the bookkeeping needed to transform vasm's
   * phijmp/phidef instructions into LLVM phi nodes.
   */
  struct PhiInfo {
    void phij(LLVMEmitter& e, llvm::BasicBlock* fromLabel,
              const VregList& uses) {
      UseInfo useInfo{fromLabel, uses};

      // Update LLVM phi instructions if they've already been emitted; otherwise
      // enqueue useInfo so that phidef() can emit the uses.
      if (m_phis.size() != 0) {
        assertx(m_phis.size() == useInfo.uses.size());
        for (auto phiInd = 0; phiInd < m_phis.size(); ++phiInd) {
          addIncoming(e, useInfo, phiInd);
        }
      } else {
        m_pendingPreds.emplace_back(std::move(useInfo));
      }
    }

    void phidef(LLVMEmitter& e, llvm::BasicBlock* toLabel,
                const VregList& defs) {
      assertx(m_phis.size() == 0);
      assertx(m_pendingPreds.size() > 0);

      m_toLabel = toLabel;
      m_defs = defs;

      for (auto phiInd = 0; phiInd < m_defs.size(); ++phiInd) {
        llvm::Type* type = e.value(m_pendingPreds[0].uses[phiInd])->getType();
        // doubles through phis are treated as i64. See comment in
        // addIncoming().
        if (type->isDoubleTy()) type = e.m_int64;
        llvm::PHINode* phi = e.m_irb.CreatePHI(type, 1);
        m_phis.push_back(phi);

        // Emit uses for the phi* instructions which preceded this phidef.
        for (auto& useInfo : m_pendingPreds) {
          addIncoming(e, useInfo, phiInd);
        }
        e.defineValue(m_defs[phiInd], phi);
      }
    }

   private:
    struct UseInfo {
      llvm::BasicBlock* fromLabel;
      VregList uses;
    };

    void addIncoming(LLVMEmitter& e, UseInfo& useInfo, unsigned phiInd) {
      // Due to the lack of a proper type system in vasm, we're tolerant of
      // doubles being passed around as i64, and we insert bitcast instructions
      // as needed to get proper double values. Until we get stronger types in
      // the incoming Vunit, we bitcast all double values to i64 when sending
      // them through a phi node, to avoid having to look ahead and figure out
      // where we might be joining an i64 with a double.
      auto value = e.value(useInfo.uses[phiInd]);
      if (value->getType()->isDoubleTy()) {
        value = new llvm::BitCastInst(value, e.m_int64, "",
                                      useInfo.fromLabel->getTerminator());
      }
      m_phis[phiInd]->addIncoming(value, useInfo.fromLabel);
      auto typeStr = llshow(e.value(useInfo.uses[phiInd])->getType());
      FTRACE(2,
             "phidef --> phiInd:{}, type:{}, incoming:{}, use:%{}, "
             "block:{}, def:%{}\n", phiInd, typeStr,
             useInfo.fromLabel->getName().str(),
             size_t{useInfo.uses[phiInd]},
             m_toLabel->getName().str(), size_t{m_defs[phiInd]});
    }

    jit::vector<UseInfo> m_pendingPreds;
    jit::vector<llvm::PHINode*> m_phis;
    VregList m_defs;
    llvm::BasicBlock* m_toLabel;
  };

  static constexpr unsigned kFSAddressSpace = 257;

#define O(name, ...) void emit(const name&);
VASM_OPCODES
#undef O

  llvm::Value* getGlobal(const std::string& name, int64_t value,
                         llvm::Type* type);
  void emitTrap();
  void emitCall(const Vinstr& instr);
  llvm::Value* emitCmpForCC(Vreg sf, ConditionCode cc);

  llvm::Value* emitFuncPtr(const std::string& name,
                           llvm::FunctionType* type,
                           uint64_t address);
  llvm::CallInst* emitTraceletTailCall(llvm::Value* target, RegSet argRegs);
  std::vector<llvm::Value*> makePhysRegArgs(
    RegSet argRegs, std::initializer_list<PhysReg> order);

  // Emit code for the pointer. Return value is of the given type, or
  // <i{bits} *> for the second overload.
  llvm::Value* emitPtr(const Vptr s, llvm::Type* ptrTy);
  llvm::Value* emitPtr(const Vptr s, size_t bits = 64);

  template<typename Taken>
  void emitJcc(Vreg sf, ConditionCode cc, const char* takenSuffix,
               Taken taken);

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
  llvm::IRBuilder<true, llvm::ConstantFolder, IRBuilderVasmInserter> m_irb;
  std::unique_ptr<TCMemoryManager> m_tcMM;

  // Function types used for call to tracelets/PHP functions.
  llvm::StructType* m_int64Pair{nullptr};
  llvm::FunctionType* m_traceletFnTy{nullptr};
  llvm::FunctionType* m_bindcallFnTy{nullptr};

  // Mimic HHVM's TypedValue, as a struct type and a Vector for full
  // loads/stores.
  llvm::StructType* m_tvStructType{nullptr};
  llvm::VectorType* m_tvVectorType{nullptr};

  // Saved LLVM intrinsics.
  llvm::Function* m_llvmFrameAddress{nullptr};
  llvm::Function* m_llvmReadRegister{nullptr};
  llvm::Function* m_llvmReturnAddress{nullptr};

  // Vreg -> RegInfo map
  jit::vector<RegInfo> m_valueInfo;

  // Vlabel -> llvm::BasicBlock map
  jit::vector<llvm::BasicBlock*> m_blocks;

  // Vlabel -> Value for rVmFp/rVmSp entering a vasm block
  jit::vector<llvm::Value*> m_incomingVmFp;
  jit::vector<llvm::Value*> m_incomingVmSp;

  jit::hash_map<llvm::BasicBlock*, PhiInfo> m_phiInfos;

  // Pending Fixups that must be processed after codegen
  jit::vector<LLVMFixup> m_fixups;

  // Pending service requests that must be processed after codegen
  jit::vector<LLVMBindJmp> m_bindjmps;
  jit::vector<LLVMFallback> m_fallbacks;

  // Vector of vasm instruction strings, used for printing in llvm IR dumps.
  jit::vector<std::string> m_instStrs;

  // The next id to use for LLVM location record. These IDs only have to
  // be unique within the function.
  uint32_t m_nextLocRec{1};

  const Vunit& m_unit;
  Vasm::AreaList& m_areas;

  // Faux personality for emitting landingpad.
  llvm::Function* m_personalityFunc;

  llvm::Function* m_fabs{nullptr};

  // Commonly used types. Some LLVM APIs require non-consts.
  llvm::Type* m_void;

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
  llvm::ConstantInt* m_int8NegOne;
  llvm::ConstantInt* m_int8One;
  llvm::ConstantInt* m_int16Zero;
  llvm::ConstantInt* m_int16NegOne;
  llvm::ConstantInt* m_int16One;
  llvm::ConstantInt* m_int32Zero;
  llvm::ConstantInt* m_int32NegOne;
  llvm::ConstantInt* m_int32One;
  llvm::ConstantInt* m_int64Zero;
  llvm::ConstantInt* m_int64NegOne;
  llvm::ConstantInt* m_int64One;

  llvm::UndefValue*  m_int64Undef;
};

void LLVMEmitter::emit(const jit::vector<Vlabel>& labels) {
  Timer timer(Timer::llvm_irGeneration);
  SCOPE_ASSERT_DETAIL("LLVM Module") { return showModule(m_module.get()); };

  auto const backedge_targets = backedgeTargets(m_unit, labels);

  // Make sure all the llvm blocks are emitted in the order given by
  // layoutBlocks, regardless of which ones we need to use as jump targets
  // first.
  for (auto label : layoutBlocks(m_unit)) {
    block(label);
  }

  auto const traceInstrs = Trace::moduleEnabled(Trace::llvm, 5);

  for (auto label : labels) {
    auto& b = m_unit.blocks[label];
    m_irb.SetInsertPoint(block(label));

    // If this isn't the first block of the unit, load the incoming value of
    // rVmFp/rVmSp from our pred(s).
    if (label != labels.front()) {
      if (auto inFp = m_incomingVmFp[label]) {
        defineValue(x64::rVmFp, inFp);
      } else {
        // Only catch blocks are allowed to not have an incoming vmfp.
        always_assert(b.code.front().op == Vinstr::landingpad);
      }

      // Only some instructions propagate a value for rVmSp to their
      // successor(s).
      auto inSp = m_incomingVmSp[label];
      if (inSp) defineValue(x64::rVmSp, inSp);
    }

    for (auto& inst : b.code) {
      if (traceInstrs) {
        m_irb.setVinstId(m_instStrs.size());
        m_instStrs.emplace_back(show(m_unit, inst).c_str());
      }

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
O(bindcall) \
O(vcallstub) \
O(bindjcc1st) \
O(bindjcc) \
O(bindjmp) \
O(bindaddr) \
O(debugtrap) \
O(defvmsp) \
O(cloadq) \
O(cmovq) \
O(cmpb) \
O(cmpbi) \
O(cmpbim) \
O(cmpwim) \
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
O(fallback) \
O(fallbackcc) \
O(incwm) \
O(incl) \
O(inclm) \
O(incq) \
O(incqm) \
O(incqmlock) \
O(jcc) \
O(jmp) \
O(jmpr) \
O(jmpm) \
O(jmpi) \
O(ldimmb) \
O(ldimml) \
O(ldimmq) \
O(lea) \
O(loadups) \
O(load) \
O(loadtqb) \
O(loadl) \
O(loadsd) \
O(loadzbl) \
O(loadzbq) \
O(loadzlq) \
O(loadqp) \
O(leap) \
O(movb) \
O(movl) \
O(movzbl) \
O(movzbq) \
O(movtqb) \
O(movtql) \
O(mulsd) \
O(mul) \
O(neg) \
O(nop) \
O(not) \
O(notb) \
O(orwim) \
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
O(storeups) \
O(storel) \
O(storeli) \
O(storeqi) \
O(storesd) \
O(storew) \
O(storewi) \
O(subbi) \
O(subl) \
O(subli) \
O(subq) \
O(subqi) \
O(subsd) \
O(svcreq) \
O(syncvmsp) \
O(testb) \
O(testbi) \
O(testbim) \
O(testwim) \
O(testl) \
O(testli) \
O(testlim) \
O(testq) \
O(testqm) \
O(testqim) \
O(ud2) \
O(xorb) \
O(xorbi) \
O(xorl) \
O(xorq) \
O(xorqi) \
O(landingpad) \
O(leavetc) \
O(vretm) \
O(vret) \
O(absdbl) \
O(phijmp) \
O(phijcc) \
O(phidef) \
O(countbytecode) \
O(unpcklpd)
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
      case Vinstr::callstub:
      case Vinstr::unwind:
      case Vinstr::nothrow:
      case Vinstr::syncpoint:
      case Vinstr::cqo:
      case Vinstr::idiv:
      case Vinstr::sarq:
      case Vinstr::shlq:
      case Vinstr::ret:
      case Vinstr::push:
      case Vinstr::pop:
      case Vinstr::psllq:
      case Vinstr::psrlq:
      case Vinstr::fallthru:
      case Vinstr::popm:  // currently used in cgEnterFrame
      case Vinstr::callfaststub:
        always_assert_flog(false,
                           "Banned opcode in B{}: {}",
                           size_t(label), show(m_unit, inst));

      // Not yet implemented opcodes:
      case Vinstr::jcci:
      case Vinstr::contenter:
      case Vinstr::mccall:
      case Vinstr::mcprep:
      case Vinstr::cmpsd:
      case Vinstr::ucomisd:
      case Vinstr::ldimmqs:
      case Vinstr::cmpqims:
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
        throw FailedLLVMCodeGen("Unsupported opcode in B{}: {}",
                                size_t(label), show(m_unit, inst));
      }

      visitDefs(m_unit, inst, [&](Vreg def) {
        defineValue(def, inst);
      });
    }

    handleOutgoingPhysRegs(b, backedge_targets);
  }

  timer.stop();
  finalize();

  if (RuntimeOption::EvalJitLLVMDiscard) {
    throw FailedLLVMCodeGen("Discarding all hard work");
  }
}

void LLVMEmitter::handleOutgoingPhysRegs(
  const Vblock& b,
  const boost::dynamic_bitset<>& backedge_targets
) {
  // rVmFp isn't SSA in vasm code, so we manually propagate its value across
  // control flow edges, creating phi nodes when appropriate. We conservatively
  // create phis at all back-edge targets and rely on LLVM to clean up the
  // unnecessary ones.
  for (auto succ : succs(b)) {
    // Catch traces from PHP calls have a weird ABI and define their own value
    // for rVmFp from the landingpad instruction, so we skip those here.
    auto& front = m_unit.blocks[succ].code.front();
    if (front.op == Vinstr::landingpad && front.landingpad_.fromPHPCall) {
      continue;
    }

    auto& inFp = m_incomingVmFp[succ];
    auto const outFp = value(x64::rVmFp);
    auto const target = block(succ);

    // When inFp already comes from a phi node in the successor, add this new
    // incoming value.
    if (inFp != nullptr &&
        llvm::isa<llvm::PHINode>(inFp) &&
        llvm::cast<llvm::Instruction>(inFp)->getParent() == target) {
      auto const phi = llvm::cast<llvm::PHINode>(inFp);
      phi->addIncoming(outFp, m_irb.GetInsertBlock());
      FTRACE(2, "Add incoming phi value {} to {} for {}\n",
             llshow(outFp), llshow(phi), succ);
      continue;
    }

    // Either this is our first time seeing a backedge target or we're
    // providing a value that differs from existing predecessors. Either way,
    // create a phi node at the beginning of succ.
    if ((inFp == nullptr && backedge_targets.test(succ)) ||
        (inFp != nullptr && inFp != outFp)) {
      FTRACE(2, "Creating phi node for {}\n", succ);
      auto const phi = llvm::PHINode::Create(m_int64, 2, "rVmFp");
      target->getInstList().insert(target->begin(), phi);
      phi->addIncoming(outFp, m_irb.GetInsertBlock());
      if (inFp != nullptr) {
        // Also add the old incoming value to the phi node. We know that the
        // successor has one or more predecessors other than the current block
        // that all provide the same value, so register inFp from all of them.
        for (auto it = llvm::pred_begin(target), end = llvm::pred_end(target);
             it != end; ++it) {
          if (*it == m_irb.GetInsertBlock()) continue;
          phi->addIncoming(inFp, *it);
        }
      }

      inFp = phi;
      continue;
    }

    if (inFp == nullptr) {
      // First time seeing this successor, and it's not a backedge target.
      inFp = outFp;
    } else {
      always_assert(inFp == outFp);
    }
  }

  // Make sure we don't let any values from this block go to the next RPO
  // block. PhysReg values are explicitly passed along edges as needed (see
  // above).
  defineValue(x64::rVmSp, nullptr);
  defineValue(x64::rVmFp, nullptr);
  defineValue(x64::rAsm, nullptr);
}

llvm::Value* LLVMEmitter::getGlobal(const std::string& name,
                                    int64_t address,
                                    llvm::Type* type) {
  m_tcMM->registerSymbolAddress(name, address);
  return m_module->getOrInsertGlobal(name, type);
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
  defineValue(inst.d, m_irb.CreateFAdd(asDbl(value(inst.s0)),
                                       asDbl(value(inst.s1))));
}

void LLVMEmitter::emit(const andb& inst) {
  defineValue(inst.d, m_irb.CreateAnd(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const andbi& inst) {
  defineValue(inst.d, m_irb.CreateAnd(cns(inst.s0.b()),
                                      narrow(value(inst.s1), 8)));
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
  defineValue(inst.d, m_irb.CreateAnd(cns(inst.s0.l()),
                                      value(inst.s1)));
}

void LLVMEmitter::emit(const andq& inst) {
  defineValue(inst.d, m_irb.CreateAnd(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const andqi& inst) {
  defineValue(inst.d, m_irb.CreateAnd(cns(inst.s0.q()), value(inst.s1)));
}

template<typename Taken>
void LLVMEmitter::emitJcc(Vreg sf, ConditionCode cc, const char* takenName,
                          Taken taken) {
  auto blockName = m_irb.GetInsertBlock()->getName().str();
  auto nextBlock = makeBlock("_");
  nextBlock->moveAfter(m_irb.GetInsertBlock());
  auto takenBlock = makeBlock(folly::to<std::string>("_", takenName));
  takenBlock->moveAfter(nextBlock);

  auto cond = emitCmpForCC(sf, cc);
  m_irb.CreateCondBr(cond, takenBlock, nextBlock);

  m_irb.SetInsertPoint(takenBlock);
  taken();

  m_irb.SetInsertPoint(nextBlock);
}

void LLVMEmitter::emit(const bindjmp& inst) {
  // bindjmp is a smashable tail call to a service request stub. The stub needs
  // to reference the address of the tail call, but we don't know the address
  // of the tail call until after codegen. This means we either need to emit
  // the stub here and patch the rip-relative lea in the stub after codegen, or
  // emit a tail call to a dummy address that we'll patch after codegen when we
  // can emit the stub with the right address. When given the choice between
  // lying to our stub emitter or llvm it's generally better to lie to the stub
  // emitter, so that's what we do here.

  auto& frozen = m_areas[size_t(AreaIndex::Frozen)].code;
  bool reused;
  auto optSPOff = folly::Optional<FPInvOffset>{};
  if (!inst.target.resumed()) optSPOff = inst.spOff;
  auto reqIp = emitEphemeralServiceReq(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups(), &reused),
    optSPOff,
    REQ_BIND_JMP,
    RipRelative(mcg->code.base()),
    inst.target.toAtomicInt(),
    inst.trflags.packed
  );
  auto stubName = folly::sformat("bindjmpStub_{}", reqIp);
  auto stubFunc = emitFuncPtr(stubName, m_traceletFnTy, uint64_t(reqIp));
  auto call = emitTraceletTailCall(stubFunc, inst.args);
  call->setSmashable();

  auto id = m_nextLocRec++;
  FTRACE(2, "Adding bindjmp locrec {} for {}\n", id, llshow(call));
  call->setMetadata(llvm::LLVMContext::MD_locrec,
                    llvm::MDNode::get(m_context, cns(id)));
  m_bindjmps.emplace_back(
    LLVMBindJmp{id, reqIp, reused, inst.target, inst.spOff, inst.trflags}
  );
}

void LLVMEmitter::emit(const bindcall& inst) {
  // All bindcall instructions use the same stub so we have to provide a unique
  // name to ensure LLVM doesn't collapse the calls together.
  auto funcName = m_tcMM->getUniqueSymbolName("bindcall_");
  auto func = emitFuncPtr(funcName, m_bindcallFnTy, uint64_t(inst.stub));
  auto args = makePhysRegArgs(
    inst.args, {x64::rVmSp, x64::rVmTl, x64::rVmFp});
  auto next = makeBlock("_next");
  next->moveAfter(m_irb.GetInsertBlock());
  auto unwind = block(inst.targets[1]);
  auto call = m_irb.CreateInvoke(func, next, unwind, args);
  call->setCallingConv(llvm::CallingConv::X86_64_HHVM_PHP);
  call->setSmashable();
  m_irb.SetInsertPoint(next);

  // Register the new value of rVmFp.
  defineValue(x64::rVmFp, m_irb.CreateExtractValue(call, 1, "rVmFp"));

  // bindcall is followed by a defvmsp to copy rVmSp into a Vreg, but it's not
  // in this block. Pass the value to our next block. This is necessary because
  // rVmSp's value isn't automatically propagated between blocks like rVmFp is.
  m_incomingVmSp[inst.targets[0]] = m_irb.CreateExtractValue(call, 0, "rVmSp");
  m_irb.CreateBr(block(inst.targets[0]));
}

void LLVMEmitter::emit(const vcallstub& inst) {
  // vcallstub is like bindcall but it's not smashable and it can take extra
  // arguments.
  auto funcName = folly::sformat("vcallstub_{}", inst.target);
  auto args = makePhysRegArgs(
    inst.args, {x64::rVmSp, x64::rVmTl, x64::rVmFp, reg::r15});
  for (auto arg : m_unit.tuples[inst.extraArgs]) args.emplace_back(value(arg));
  std::vector<llvm::Type*> argTypes(args.size(), m_int64);
  auto funcTy = llvm::FunctionType::get(m_int64Pair, argTypes, false);
  auto func = emitFuncPtr(funcName, funcTy, uint64_t(inst.target));
  auto next = makeBlock("_next");
  next->moveAfter(m_irb.GetInsertBlock());
  auto unwind = block(inst.targets[1]);
  auto call = m_irb.CreateInvoke(func, next, unwind, args);
  call->setCallingConv(llvm::CallingConv::X86_64_HHVM_PHP);
  m_irb.SetInsertPoint(next);

  // Register new rVmSp/rVmFp, just like bindcall.
  defineValue(x64::rVmFp, m_irb.CreateExtractValue(call, 1, "rVmFp"));
  m_incomingVmSp[inst.targets[0]] = m_irb.CreateExtractValue(call, 0, "rVmSp");
  m_irb.CreateBr(block(inst.targets[0]));
}

// Emitting a real REQ_BIND_(JUMPCC_FIRST|JCC) only makes sense if we can
// guarantee that llvm will emit a smashable jcc. Until then, we jcc to a
// REQ_BIND_JMP.
void LLVMEmitter::emit(const bindjcc1st& inst) {
  emitJcc(
    inst.sf, inst.cc, "jcc1",
    [&] {
      emit(bindjmp{inst.targets[1], inst.spOff, TransFlags{}, inst.args});
    }
  );

  emit(bindjmp{inst.targets[0], inst.spOff, TransFlags{}, inst.args});
}

void LLVMEmitter::emit(const bindjcc& inst) {
  emitJcc(
    inst.sf, inst.cc, "jcc",
    [&] {
      emit(bindjmp{inst.target, inst.spOff, inst.trflags, inst.args});
    }
  );
}

void LLVMEmitter::emit(const bindaddr& inst) {
  // inst.dest is a pointer to memory allocated in globalData, so we can just
  // do what vasm does here.

  auto& frozen = m_areas[size_t(AreaIndex::Frozen)].code;
  mcg->setJmpTransID((TCA)inst.dest);

  auto optSPOff = folly::Optional<FPInvOffset>{};
  if (!inst.sk.resumed()) optSPOff = inst.spOff;

  *inst.dest = emitEphemeralServiceReq(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups()),
    optSPOff,
    REQ_BIND_ADDR,
    inst.dest,
    inst.sk.toAtomicInt(),
    TransFlags{}.packed
  );
  mcg->cgFixups().m_codePointers.insert(inst.dest);
  mcg->setJmpTransID(TCA(inst.dest));
}

void LLVMEmitter::emit(const defvmsp& inst) {
  defineValue(inst.d, value(x64::rVmSp));
}

llvm::Value* LLVMEmitter::emitFuncPtr(const std::string& name,
                                      llvm::FunctionType* type,
                                      uint64_t address) {
  auto funcPtr = m_module->getFunction(name);
  if (!funcPtr) {
    m_tcMM->registerSymbolAddress(name, address);
    funcPtr = llvm::Function::Create(type,
                                     llvm::GlobalValue::ExternalLinkage,
                                     name,
                                     m_module.get());
  }
  return funcPtr;
}

/*
 * Return a vector of values for the PhysRegs in `order', using undef for any
 * PhysRegs that aren't in argRegs.
 */
std::vector<llvm::Value*> LLVMEmitter::makePhysRegArgs(
  RegSet argRegs,
  std::initializer_list<PhysReg> order
) {
  RegSet passed;
  std::vector<llvm::Value*> ret;
  ret.reserve(order.size());

  for (auto reg : order) {
    if (argRegs.contains(reg)) {
      ret.emplace_back(value(reg));
      passed.add(reg);
    } else {
      ret.emplace_back(m_int64Undef);
    }
  }

  always_assert_flog(passed == argRegs,
                     "argRegs = {}, but only passed {}",
                     show(argRegs), show(passed));
  return ret;
}

llvm::CallInst* LLVMEmitter::emitTraceletTailCall(llvm::Value* target,
                                                  RegSet argRegs) {
  auto args = makePhysRegArgs(argRegs, {x64::rVmSp, x64::rVmTl, x64::rVmFp});
  auto call = m_irb.CreateCall(target, args);
  call->setCallingConv(llvm::CallingConv::X86_64_HHVM_TC);
  call->setTailCallKind(llvm::CallInst::TCK_MustTail);
  m_irb.CreateRetVoid();
  return call;
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
    returnType = m_void;
    break;
  case DestType::SSA:
    returnType = m_int64;
    break;
  case DestType::Byte:
    returnType = m_int8;
    break;
  case DestType::Dbl:
    returnType = m_irb.getDoubleTy();
    break;
  case DestType::SIMD:
  case DestType::TV:
    returnType = m_tvStructType;
    break;
  }

  std::vector<llvm::Type*> argTypes = { m_int64 };
  std::vector<llvm::Value*> args = { value(x64::rVmFp) };
  auto doArgs = [&] (const VregList& srcs, bool isDbl = false) {
    for(int i = 0; i < srcs.size(); ++i) {
      auto arg = value(srcs[i]);
      if (isDbl) {
        // When the helper expects a double, make sure it's properly typed so
        // it ends up in an xmm register.
        arg = asDbl(arg);
      } else if (arg->getType()->isDoubleTy()) {
        // This case can happen when we're passing m_data in a KindOfDouble
        // TypedValue: the llvm value is a double but it needs to be passed in
        // a GP register.
        arg = m_irb.CreateBitCast(arg, intNType(64));
      }
      args.push_back(arg);
      argTypes.push_back(arg->getType());
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
  doArgs(vargs.simdArgs, true);
  doArgs(vargs.stkArgs);

  auto const funcType = llvm::FunctionType::get(returnType, argTypes, false);

  llvm::Value* funcPtr = nullptr;
  switch (call.kind()) {
    case CppCall::Kind::Direct:
      funcPtr = emitFuncPtr(getNativeFunctionName(call.address()),
                            funcType,
                            uint64_t(call.address()));
      break;

    case CppCall::Kind::Virtual:
    case CppCall::Kind::ArrayVirt:
      throw FailedLLVMCodeGen("Unsupported call type: {}",
                              (int)call.kind());

    case CppCall::Kind::Destructor: {
      assertx(vargs.args.size() == 1);
      llvm::Value* type = value(call.reg());
      type = m_irb.CreateLShr(type, kShiftDataTypeToDestrIndex, "typeIdx");

      auto destructors = getGlobal("g_destructors", uint64_t(g_destructors),
                                   llvm::VectorType::get(ptrType(funcType),
                                                         kDestrTableSize));
      funcPtr = m_irb.CreateExtractElement(m_irb.CreateLoad(destructors), type);
      break;
    }
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

  // Record location of the call/invoke instruction.
  if (fixup.isValid()) {
    auto id = m_nextLocRec++;
    m_fixups.emplace_back(LLVMFixup{id, fixup});
    FTRACE(2, "Adding fixup locrec {} for {}\n", id, llshow(callInst));
    callInst->setMetadata(llvm::LLVMContext::MD_locrec,
                          llvm::MDNode::get(m_context, cns(id)));
  }

  // Extract value(s) from the call.
  switch (destType) {
  case DestType::None:
    // nothing to do
    assertx(dests.size() == 0);
    break;
  case DestType::SSA:
  case DestType::Byte:
  case DestType::Dbl:
    assertx(dests.size() == 1);
    defineValue(dests[0], callInst);
    break;
  case DestType::SIMD:
  case DestType::TV: {
    static_assert(offsetof(TypedValue, m_data) == 0, "");
    static_assert(offsetof(TypedValue, m_type) == 8, "");
    assertx(dests.size() <= 2 && dests.size() >= 1);
    auto const data = m_irb.CreateExtractValue(callInst, 0);
    auto const type = zext(m_irb.CreateExtractValue(callInst, 1), 64);

    if (destType == DestType::TV) {
      defineValue(dests[0], data);
      if (dests.size() == 2) defineValue(dests[1], type);
    } else {
      assertx(dests.size() == 1);
      llvm::Value* packed = llvm::UndefValue::get(m_tvVectorType);
      packed = m_irb.CreateInsertElement(packed, data, cns(0));
      packed = m_irb.CreateInsertElement(packed, type, cns(1));
      defineValue(dests[0], packed);
    }
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

void LLVMEmitter::emit(const cmpwim& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateLoad(emitPtr(inst.s1, 16)));
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
  assertx(srcs.size() == dsts.size());
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
  defineValue(inst.d, m_irb.CreateAdd(value(inst.s), m_int32NegOne, "",
                                      /* NUW = */ false, /* NSW = */ true));
}

void LLVMEmitter::emit(const declm& inst) {
  auto ptr = emitPtr(inst.m, 32);
  auto load = m_irb.CreateLoad(ptr, RuntimeOption::EvalJitLLVMVolatileIncDec);
  auto sub = m_irb.CreateAdd(load, m_int32NegOne, "",
                             /* NUW = */ false, /* NSW = */ true);
  defineFlagTmp(inst.sf, sub);
  m_irb.CreateStore(sub, ptr);
}

void LLVMEmitter::emit(const decq& inst) {
  defineValue(inst.d, m_irb.CreateAdd(value(inst.s), m_int64NegOne, "",
                                      /* NUW = */ false, /* NSW = */ true));
}

void LLVMEmitter::emit(const decqm& inst) {
  auto ptr = emitPtr(inst.m, 64);
  auto oldVal = m_irb.CreateLoad(ptr);
  auto newVal = m_irb.CreateAdd(oldVal, m_int64NegOne, "",
                                /* NUW = */ false, /* NSW = */ true);
  defineFlagTmp(inst.sf, newVal);
  m_irb.CreateStore(newVal, ptr);
}

void LLVMEmitter::emit(const divsd& inst) {
  defineValue(inst.d, m_irb.CreateFDiv(asDbl(value(inst.s1)),
                                       asDbl(value(inst.s0))));
}

void LLVMEmitter::emit(const imul& inst) {
  defineValue(inst.d, m_irb.CreateMul(value(inst.s0), value(inst.s1)));
}

void LLVMEmitter::emit(const fallback& inst) {
  TCA stub;
  auto const sr = mcg->tx().getSrcRec(inst.dest);
  if (inst.trflags.packed == 0) {
    stub = sr->getFallbackTranslation();
  } else {
    // Emit a custom REQ_RETRANSLATE
    auto& frozen = m_areas[size_t(AreaIndex::Frozen)].code;
    auto const args = packServiceReqArgs(inst.dest.offset(),
                                         inst.trflags.packed);
    auto optSPOff = folly::Optional<FPInvOffset>{};
    if (!inst.dest.resumed()) optSPOff = sr->nonResumedSPOff();
    stub = mcg->backEnd().emitServiceReqWork(
      frozen,
      frozen.frontier(),
      SRFlags::Persist,
      optSPOff,
      REQ_RETRANSLATE,
      args
    );
  }

  auto func = emitFuncPtr(folly::sformat("reqRetranslate_{}", stub),
                          m_traceletFnTy,
                          uint64_t(stub));
  auto call = emitTraceletTailCall(func, inst.args);
  call->setSmashable();

  LLVMFallback req{m_nextLocRec++, inst.dest};
  FTRACE(2, "Adding fallback locrec {} for {}\n", req.id, llshow(call));
  call->setMetadata(llvm::LLVMContext::MD_locrec,
                    llvm::MDNode::get(m_context, cns(req.id)));
  m_fallbacks.emplace_back(req);
}

void LLVMEmitter::emit(const fallbackcc& inst) {
  assert_not_implemented(inst.trflags.packed == 0);

  emitJcc(
    inst.sf, inst.cc, "guard",
    [&] {
      emit(fallback{inst.dest, inst.trflags, inst.args});
    }
  );
}

void LLVMEmitter::emit(const incwm& inst) {
  auto ptr = emitPtr(inst.m, 16);
  auto oldVal = m_irb.CreateLoad(ptr, RuntimeOption::EvalJitLLVMVolatileIncDec);
  auto newVal = m_irb.CreateAdd(oldVal, m_int16One, "",
                                /* NUW = */ false, /* NSW = */ true);
  defineFlagTmp(inst.sf, newVal);
  m_irb.CreateStore(newVal, ptr);
}

void LLVMEmitter::emit(const incl& inst) {
  defineValue(inst.d, m_irb.CreateAdd(value(inst.s), m_int32One, "",
                                      /* NUW = */ false, /* NSW = */ true));
}

void LLVMEmitter::emit(const inclm& inst) {
  auto ptr = emitPtr(inst.m, 32);
  auto load = m_irb.CreateLoad(ptr, RuntimeOption::EvalJitLLVMVolatileIncDec);
  auto add = m_irb.CreateAdd(load, m_int32One, "",
                             /* NUW = */ false, /* NSW = */ true);
  defineFlagTmp(inst.sf, add);
  m_irb.CreateStore(add, ptr);
}

void LLVMEmitter::emit(const incq& inst) {
  defineValue(inst.d, m_irb.CreateAdd(value(inst.s), m_int64One, "",
                                      /* NUW = */ false, /* NSW = */ true));
}

void LLVMEmitter::emit(const incqm& inst) {
  auto ptr = emitPtr(inst.m, 64);
  auto load = m_irb.CreateLoad(ptr);
  auto add = m_irb.CreateAdd(load, m_int64One, "",
                             /* NUW = */ false, /* NSW = */ true);
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
    default:    throw FailedLLVMCodeGen("Unsupported CC {}", cc_names[cc]);
  }
}

llvm::Value* LLVMEmitter::emitCmpForCC(Vreg sf, ConditionCode cc) {
  auto& cmp = defInst(sf);
  llvm::Value* lhs = nullptr;
  llvm::Value* rhs = nullptr;

  if (cmp.op == Vinstr::addq) {
    lhs = value(cmp.addq_.d);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::addqi) {
    lhs = value(cmp.addqi_.d);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::addqim) {
    lhs = flagTmp(sf);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::cmpb) {
    lhs = narrow(value(cmp.cmpb_.s1), 8);
    rhs = narrow(value(cmp.cmpb_.s0), 8);
  } else if (cmp.op == Vinstr::cmpbi) {
    lhs = narrow(value(cmp.cmpbi_.s1), 8);
    rhs = cns(cmp.cmpbi_.s0.b());
  } else if (cmp.op == Vinstr::cmpbim) {
    lhs = flagTmp(sf);
    rhs = cns(cmp.cmpbim_.s0.b());
  } else if (cmp.op == Vinstr::cmpwim) {
    lhs = flagTmp(sf);
    rhs = cns(cmp.cmpwim_.s0.w());
  } else if (cmp.op == Vinstr::cmpl) {
    lhs = value(cmp.cmpl_.s1);
    rhs = value(cmp.cmpl_.s0);
  } else if (cmp.op == Vinstr::cmpli) {
    lhs = value(cmp.cmpli_.s1);
    rhs = cns(cmp.cmpli_.s0.l());
  } else if (cmp.op == Vinstr::cmplim) {
    lhs = flagTmp(sf);
    rhs = cns(cmp.cmplim_.s0.l());
  } else if (cmp.op == Vinstr::cmplm) {
    lhs = flagTmp(sf);
    rhs = value(cmp.cmplm_.s0);
  } else if (cmp.op == Vinstr::cmpq) {
    lhs = value(cmp.cmpq_.s1);
    rhs = value(cmp.cmpq_.s0);
  } else if (cmp.op == Vinstr::cmpqi) {
    lhs = value(cmp.cmpqi_.s1);
    rhs = cns(cmp.cmpqi_.s0.q());
  } else if (cmp.op == Vinstr::cmpqim) {
    lhs = flagTmp(sf);
    rhs = cns(cmp.cmpqim_.s0.q());
  } else if (cmp.op == Vinstr::cmpqm) {
    lhs = flagTmp(sf);
    rhs = value(cmp.cmpqm_.s0);
  } else if (cmp.op == Vinstr::decl) {
    lhs = value(cmp.decl_.d);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::declm) {
    lhs = flagTmp(sf);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::decq) {
    lhs = value(cmp.decq_.d);
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
  } else if (cmp.op == Vinstr::subbi) {
    lhs = narrow(value(cmp.subbi_.d), 8);
    rhs = m_int8Zero;
  } else if (cmp.op == Vinstr::subl) {
    lhs = value(cmp.subl_.d);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::subli) {
    lhs = value(cmp.subli_.d);
    rhs = m_int32Zero;
  } else if (cmp.op == Vinstr::subq) {
    lhs = value(cmp.subq_.d);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::subqi) {
    lhs = value(cmp.subqi_.d);
    rhs = m_int64Zero;
  } else if (cmp.op == Vinstr::testb ||
             cmp.op == Vinstr::testbi ||
             cmp.op == Vinstr::testbim) {
    lhs = flagTmp(sf);
    rhs = m_int8Zero;
  } else if (cmp.op == Vinstr::testwim) {
    lhs = flagTmp(sf);
    rhs = m_int16Zero;
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
    throw FailedLLVMCodeGen("Unsupported flags src: {}",
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

void LLVMEmitter::emit(const jmpr& inst) {
  auto func = m_irb.CreateIntToPtr(value(inst.target), ptrType(m_traceletFnTy));
  emitTraceletTailCall(func, inst.args);
}

void LLVMEmitter::emit(const jmpm& inst) {
  auto func = m_irb.CreateLoad(emitPtr(inst.target,
                                       ptrType(ptrType(m_traceletFnTy))));
  emitTraceletTailCall(func, inst.args);
}

void LLVMEmitter::emit(const jmpi& inst) {
  auto args = makePhysRegArgs(
    inst.args, {x64::rVmSp, x64::rVmTl, x64::rVmFp, x64::rAsm});
  std::vector<llvm::Type*> argTypes(args.size(), m_int64);

  auto func = emitFuncPtr(
    folly::sformat("jmpi_{}", inst.target),
    llvm::FunctionType::get(m_void, argTypes, false),
    reinterpret_cast<uint64_t>(inst.target)
  );
  auto call = m_irb.CreateCall(func, args);
  call->setCallingConv(llvm::CallingConv::X86_64_HHVM_SR);
  call->setTailCallKind(llvm::CallInst::TCK_Tail);
  m_irb.CreateRetVoid();
}

void LLVMEmitter::emit(const ldimmb& inst) {
  defineValue(inst.d, cns(inst.s.b()));
}

void LLVMEmitter::emit(const ldimml& inst) {
  defineValue(inst.d, cns(inst.s.l()));
}

void LLVMEmitter::emit(const ldimmq& inst) {
  defineValue(inst.d, cns(inst.s.q()));
}

void LLVMEmitter::emit(const lea& inst) {
  auto value = m_irb.CreatePtrToInt(emitPtr(inst.s, 8), m_int64, "conv");
  defineValue(inst.d, value);
}

void LLVMEmitter::emit(const loadups& inst) {
  // This will need to change if we ever use loadups with values that aren't
  // TypedValues. Ideally, we'd leave this kind of decision to llvm anyway.
  auto vec = m_irb.CreateLoad(emitPtr(inst.s, ptrType(m_tvVectorType)));
  vec->setAlignment(sizeof(intptr_t));
  defineValue(inst.d, vec);
}

void LLVMEmitter::emit(const load& inst) {
  defineValue(inst.d, m_irb.CreateLoad(emitPtr(inst.s, 64)));
}

void LLVMEmitter::emit(const loadtqb& inst) {
  auto quad = m_irb.CreateLoad(emitPtr(inst.s, 64));
  defineValue(inst.d, narrow(quad, 8));
}

void LLVMEmitter::emit(const loadl& inst) {
  defineValue(inst.d, m_irb.CreateLoad(emitPtr(inst.s, 32)));
}

void LLVMEmitter::emit(const loadsd& inst) {
  defineValue(inst.d,
              m_irb.CreateLoad(emitPtr(inst.s, ptrType(m_irb.getDoubleTy()))));
}

void LLVMEmitter::emit(const loadzbl& inst) {
  auto byteVal = m_irb.CreateLoad(emitPtr(inst.s, 8));
  defineValue(inst.d, zext(byteVal, 32));
}

void LLVMEmitter::emit(const loadzbq& inst) {
  auto byteVal = m_irb.CreateLoad(emitPtr(inst.s, 8));
  defineValue(inst.d, zext(byteVal, 64));
}

void LLVMEmitter::emit(const loadzlq& inst) {
  auto val = m_irb.CreateLoad(emitPtr(inst.s, 32));
  defineValue(inst.d, zext(val, 64));
}

// loadqp/leap are intended to be rip-relative instructions, but that's not
// necessary for correctness. Depending on the target of the load, it may be
// needed to work with code relocation - see t5662452 for details.
void LLVMEmitter::emit(const loadqp& inst) {
  auto addr = m_irb.CreateIntToPtr(cns(inst.s.r.disp), m_int64Ptr);
  defineValue(inst.d, m_irb.CreateLoad(addr));
}

void LLVMEmitter::emit(const leap& inst) {
  defineValue(inst.d, cns(inst.s.r.disp));
}

void LLVMEmitter::emit(const movb& inst) {
  defineValue(inst.d, value(inst.s));
}

void LLVMEmitter::emit(const movl& inst) {
  defineValue(inst.d, value(inst.s));
}

void LLVMEmitter::emit(const movzbl& inst) {
  defineValue(inst.d, zext(value(inst.s), 32));
}

void LLVMEmitter::emit(const movzbq& inst) {
  defineValue(inst.d, zext(value(inst.s), 64));
}

void LLVMEmitter::emit(const movtqb& inst) {
  defineValue(inst.d, narrow(value(inst.s), 8));
}

void LLVMEmitter::emit(const movtql& inst) {
  defineValue(inst.d, narrow(value(inst.s), 32));
}

void LLVMEmitter::emit(const mulsd& inst) {
  defineValue(inst.d, m_irb.CreateFMul(asDbl(value(inst.s0)),
                                       asDbl(value(inst.s1))));
}

void LLVMEmitter::emit(const mul& inst) {
  defineValue(inst.d, m_irb.CreateFMul(asDbl(value(inst.s0)),
                                       asDbl(value(inst.s1))));
}

void LLVMEmitter::emit(const neg& inst) {
  defineValue(inst.d, m_irb.CreateSub(m_int64Zero, value(inst.s), "",
                                      /* NUW = */ false, /* NSW = */ true));
}

void LLVMEmitter::emit(const nop& inst) {
}

void LLVMEmitter::emit(const not& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s), cns(int64_t{-1})));
}

void LLVMEmitter::emit(const notb& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s), cns(int8_t{-1})));
}

void LLVMEmitter::emit(const orwim& inst) {
  auto ptr = emitPtr(inst.m, 16);
  auto value = m_irb.CreateOr(cns(inst.s0.w()), m_irb.CreateLoad(ptr));
  defineFlagTmp(inst.sf, value);
  m_irb.CreateStore(value, ptr);
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

void LLVMEmitter::emit(const phijmp& inst) {
  m_irb.CreateBr(block(inst.target));
  m_phiInfos[block(inst.target)].phij(*this, m_irb.GetInsertBlock(),
                                      m_unit.tuples[inst.uses]);
}

void LLVMEmitter::emit(const phijcc& inst) {
  auto curBlock = m_irb.GetInsertBlock();
  auto next  = block(inst.targets[0]);
  auto taken = block(inst.targets[1]);
  auto& uses = m_unit.tuples[inst.uses];
  auto cond = emitCmpForCC(inst.sf, inst.cc);
  m_irb.CreateCondBr(cond, taken, next);

  m_phiInfos[next].phij(*this, curBlock, uses);
  m_phiInfos[taken].phij(*this, curBlock, uses);
}

void LLVMEmitter::emit(const phidef& inst) {
  const VregList& defs = m_unit.tuples[inst.defs];
  auto block = m_irb.GetInsertBlock();
  m_phiInfos.at(block).phidef(*this, block, defs);
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
    llvm::FunctionType::get(m_void, false);
  auto const iasm = llvm::InlineAsm::get(funcType, asmStatement, asmConstraints,
                                         hasSideEffects);
  auto call = m_irb.CreateCall(iasm, "");
  call->setCallingConv(llvm::CallingConv::C);
}

void LLVMEmitter::emit(const vretm& inst) {
  // We emit volatile loads for return addresses to prevent LLVM from
  // generating move from memory to register via another register.
  auto const retPtr = emitPtr(inst.retAddr, ptrType(ptrType(m_traceletFnTy)));
  auto const retAddr = m_irb.CreateLoad(retPtr, true);
  auto const prevFp = m_irb.CreateLoad(emitPtr(inst.prevFp, 64), true);
  defineValue(inst.d, prevFp);

  // "Return" with a tail call to the loaded address
  auto call = emitTraceletTailCall(retAddr, inst.args);
  if (RuntimeOption::EvalJitLLVMRetOpt) {
    call->setCallingConv(llvm::CallingConv::X86_64_HHVM_TCR);
    call->setTailCallKind(llvm::CallInst::TCK_Tail);
  }
}

void LLVMEmitter::emit(const vret& inst) {
  auto const retAddr = m_irb.CreateIntToPtr(value(inst.retAddr),
                                            ptrType(m_traceletFnTy));
  auto call = emitTraceletTailCall(retAddr, inst.args);
  if (RuntimeOption::EvalJitLLVMRetOpt) {
    call->setCallingConv(llvm::CallingConv::X86_64_HHVM_TCR);
    call->setTailCallKind(llvm::CallInst::TCK_Tail);
  }
}

void LLVMEmitter::emit(const leavetc& inst) {
  auto const exit = reinterpret_cast<intptr_t>(
    mcg->tx().uniqueStubs.callToExit
  );
  auto const exit_ptr = m_irb.CreateIntToPtr(
    cns(exit),
    ptrType(m_traceletFnTy)
  );
  auto call = emitTraceletTailCall(exit_ptr, inst.args);
  if (RuntimeOption::EvalJitLLVMRetOpt) {
    call->setCallingConv(llvm::CallingConv::X86_64_HHVM_TCR);
    call->setTailCallKind(llvm::CallInst::TCK_Tail);
  }
}

void LLVMEmitter::emit(const absdbl& inst) {
  if (!m_fabs) {
    m_fabs = llvm::Intrinsic::getDeclaration(
      m_module.get(),
      llvm::Intrinsic::fabs,
      std::vector<llvm::Type*>{m_irb.getDoubleTy()}
    );
  }
  defineValue(inst.d, m_irb.CreateCall(m_fabs, asDbl(value(inst.s))));
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

  auto func = llvm::Intrinsic::getDeclaration(
    m_module.get(),
    roundID,
    std::vector<llvm::Type*>{m_irb.getDoubleTy()}
  );
  defineValue(inst.d, m_irb.CreateCall(func, asDbl(value(inst.s))));
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
  defineValue(inst.d, zext(emitCmpForCC(inst.sf, inst.cc), 8));
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
  // The square root of a negative number in vasm is NaN, but the llvm sqrt
  // intrinsic is undefined for negative inputs. On x86, we can use an sse2
  // intrinsic to get the right behavior with a single instruction, but on
  // other platforms we emit a runtime check. We also skip the x64-specific
  // path when the input is a constant, since llvm doesn't know how to constant
  // fold the sse2 intrinsic.

  auto dblVal = asDbl(value(inst.s));
  if (arch() == Arch::X64 && !llvm::isa<llvm::Constant>(dblVal)) {
    auto sqrtFunc = llvm::Intrinsic::getDeclaration(
      m_module.get(),
      llvm::Intrinsic::x86_sse2_sqrt_sd
    );

    llvm::Value* vec = llvm::UndefValue::get(
      llvm::VectorType::get(m_irb.getDoubleTy(), 2));
    vec = m_irb.CreateInsertElement(vec, dblVal, cns(0));
    vec = m_irb.CreateCall(sqrtFunc, vec);
    defineValue(inst.d, m_irb.CreateExtractElement(vec, cns(0)));
  } else {
    auto sqrtFunc = llvm::Intrinsic::getDeclaration(
      m_module.get(),
      llvm::Intrinsic::sqrt,
      std::vector<llvm::Type*>{m_irb.getDoubleTy()}
    );

    auto lt = m_irb.CreateFCmpOLT(dblVal, cnsDbl(0.0));
    auto NaN = llvm::ConstantFP::get(
      m_context, llvm::APFloat::getNaN(llvm::APFloat::IEEEdouble));
    defineValue(inst.d,
                m_irb.CreateSelect(lt, NaN,
                                   m_irb.CreateCall(sqrtFunc, dblVal)));
  }
}

void LLVMEmitter::emit(const store& inst) {
  auto val = value(inst.s);
  assertx(val->getType()->getPrimitiveSizeInBits() == 64);
  m_irb.CreateStore(val, emitPtr(inst.d, ptrType(val->getType())));
}

void LLVMEmitter::emit(const storeb& inst) {
  m_irb.CreateStore(narrow(value(inst.s), 8), emitPtr(inst.m, 8));
}

void LLVMEmitter::emit(const storebi& inst) {
  m_irb.CreateStore(cns(inst.s.b()), emitPtr(inst.m, 8));
}

void LLVMEmitter::emit(const storeups& inst) {
  // Like loadups, this will need to change if we ever use storeups with values
  // that aren't TypedValues.
  auto vecPtr = emitPtr(inst.m, ptrType(m_tvVectorType));
  m_irb.CreateStore(value(inst.s), vecPtr)->setAlignment(sizeof(intptr_t));
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

void LLVMEmitter::emit(const subbi& inst) {
  defineValue(inst.d, m_irb.CreateSub(narrow(value(inst.s1), 8),
                                      cns(inst.s0.b())));
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
  defineValue(inst.d, m_irb.CreateFSub(asDbl(value(inst.s1)),
                                       asDbl(value(inst.s0))));
}

void LLVMEmitter::emit(const svcreq& inst) {
  auto args = makePhysRegArgs(inst.args, {x64::rVmSp, x64::rVmTl, x64::rVmFp});
  args.emplace_back(cns(reinterpret_cast<uintptr_t>(inst.stub_block)));
  args.emplace_back(cns(uint64_t{inst.req}));
  for (auto arg : m_unit.tuples[inst.extraArgs]) {
    args.push_back(value(arg));
  }
  std::vector<llvm::Type*> argTypes(args.size(), m_int64);
  auto funcType = llvm::FunctionType::get(m_void, argTypes, false);
  auto func = emitFuncPtr(folly::to<std::string>("handleSRHelper_",
                                                 argTypes.size()),
                          funcType,
                          uint64_t(handleSRHelper));
  auto call = m_irb.CreateCall(func, args);
  call->setCallingConv(llvm::CallingConv::X86_64_HHVM_SR);
  call->setTailCallKind(llvm::CallInst::TCK_Tail);
  m_irb.CreateRetVoid();
}

void LLVMEmitter::emit(const syncvmsp& inst) {
  defineValue(x64::rVmSp, value(inst.s));
}

void LLVMEmitter::emit(const testb& inst) {
  auto result = m_irb.CreateAnd(narrow(value(inst.s1), 8),
                                narrow(value(inst.s0), 8));
  defineFlagTmp(inst.sf, result);
}

void LLVMEmitter::emit(const testbi& inst) {
  auto result = m_irb.CreateAnd(narrow(value(inst.s1), 8), inst.s0.b());
  defineFlagTmp(inst.sf, result);
}

void LLVMEmitter::emit(const testbim& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 8));
  defineFlagTmp(inst.sf, m_irb.CreateAnd(lhs, inst.s0.b()));
}

void LLVMEmitter::emit(const testwim& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 16));
  defineFlagTmp(inst.sf, m_irb.CreateAnd(lhs, inst.s0.w()));
}

void LLVMEmitter::emit(const testl& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateAnd(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const testli& inst) {
  defineFlagTmp(inst.sf, m_irb.CreateAnd(value(inst.s1), inst.s0.l()));
}

void LLVMEmitter::emit(const testlim& inst) {
  auto lhs = m_irb.CreateLoad(emitPtr(inst.s1, 32));
  defineFlagTmp(inst.sf, m_irb.CreateAnd(lhs, inst.s0.l()));
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

void LLVMEmitter::emit(const xorl& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const xorq& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s1), value(inst.s0)));
}

void LLVMEmitter::emit(const xorqi& inst) {
  defineValue(inst.d, m_irb.CreateXor(value(inst.s1), inst.s0.q()));
}

void LLVMEmitter::emit(const landingpad& inst) {
  // m_personalityFunc is a dummy value because we just extract the information
  // we need from the .gcc_except_table section rather than using it directly.
  auto pad = m_irb.CreateLandingPad(m_int64Pair, m_personalityFunc, 0);
  pad->setCleanup(true);

  if (inst.fromPHPCall) {
    // bindcall destroys all registers, so tc_unwind_resume gives us the
    // correct value of rVmFp in the second unwinder scratch register.
    auto newFp = m_irb.CreateExtractValue(pad, 1, "rVmFp");
    defineValue(x64::rVmFp, newFp);
  }
}

void LLVMEmitter::emit(const countbytecode& inst) {
  auto ptr = emitPtr(inst.base[g_bytecodesLLVM.handle()], 64);
  auto load = m_irb.CreateLoad(ptr);
  m_irb.CreateStore(m_irb.CreateAdd(load, m_int64One), ptr);
}

void LLVMEmitter::emit(const unpcklpd& inst) {
  llvm::Value* dest = llvm::UndefValue::get(m_tvVectorType);
  // Flipped operand order is due to ATT style in vasm.
  dest = m_irb.CreateInsertElement(dest, value(inst.s1), cns(0));
  dest = m_irb.CreateInsertElement(dest, value(inst.s0), cns(1));
  defineValue(inst.d, dest);
}

void LLVMEmitter::emitTrap() {
  auto trap = llvm::Intrinsic::getDeclaration(m_module.get(),
                                              llvm::Intrinsic::trap);
  m_irb.CreateCall(trap);
  m_irb.CreateUnreachable();
}

llvm::Value* LLVMEmitter::emitPtr(const Vptr s, llvm::Type* ptrTy) {
  bool inFS =
    llvm::cast<llvm::PointerType>(ptrTy)->getAddressSpace() == kFSAddressSpace;
  always_assert(s.base != reg::rsp);

  auto ptr = s.base.isValid() ? zext(value(s.base), 64) : cns(int64_t{0});
  auto disp = cns(int64_t{s.disp});
  if (s.index.isValid()) {
    auto scaledIdx = m_irb.CreateMul(zext(value(s.index), 64),
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

} // unnamed namespace

void genCodeLLVM(const Vunit& unit, Vasm::AreaList& areas) {
  Timer timer(Timer::llvm);
  FTRACE(2, "\nTrying to emit LLVM IR for Vunit:\n{}\n", show(unit));

  try {
    auto const labels = sortBlocks(unit);
    LLVMEmitter(unit, areas).emit(labels);
  } catch (const FailedLLVMCodeGen& e) {
    throw;
  } catch (const std::exception& e) {
    always_assert_flog(false,
                       "Unexpected exception during LLVM codegen: {}\n",
                       e.what());
  }
}

} }

#else // #ifdef USE_LLVM

namespace HPHP { namespace jit {

void genCodeLLVM(const Vunit& unit, Vasm::AreaList& areas) {
  throw FailedLLVMCodeGen("This build does not support the LLVM backend");
}

} }

#endif // #ifdef USE_LLVM
