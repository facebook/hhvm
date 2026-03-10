// Copyright 2019, VIXL authors
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of ARM Limited nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// Test infrastructure.
//
// Tests are functions which accept no parameters and have no return values.
// The testing code should not perform an explicit return once completed. For
// example to test the mov immediate instruction a very simple test would be:
//
//   TEST(mov_x0_one) {
//     SETUP();
//
//     START();
//     __ mov(x0, 1);
//     END();
//
//     if (CAN_RUN()) {
//       RUN();
//
//       ASSERT_EQUAL_64(1, x0);
//     }
//   }
//
// Within a START ... END block all registers but sp can be modified. sp has to
// be explicitly saved/restored. The END() macro replaces the function return
// so it may appear multiple times in a test if the test has multiple exit
// points.
//
// Tests requiring specific CPU features should specify exactly what they
// require using SETUP_WITH_FEATURES(...) instead of SETUP().
//
// Once the test has been run all integer and floating point registers as well
// as flags are accessible through a RegisterDump instance, see
// utils-aarch64.cc for more info on RegisterDump.
//
// We provide some helper assert to handle common cases:
//
//   ASSERT_EQUAL_32(int32_t, int_32t)
//   ASSERT_EQUAL_FP32(float, float)
//   ASSERT_EQUAL_32(int32_t, W register)
//   ASSERT_EQUAL_FP32(float, S register)
//   ASSERT_EQUAL_64(int64_t, int_64t)
//   ASSERT_EQUAL_FP64(double, double)
//   ASSERT_EQUAL_64(int64_t, X register)
//   ASSERT_EQUAL_64(X register, X register)
//   ASSERT_EQUAL_FP64(double, D register)
//
// e.g. ASSERT_EQUAL_64(0.5, d30);
//
// If more advanced computation is required before the assert then access the
// RegisterDump named core directly:
//
//   ASSERT_EQUAL_64(0x1234, core->reg_x0() & 0xffff);

namespace vixl {
namespace aarch64 {

#define __ masm.
#define TEST(name) TEST_(AARCH64_ASM_##name)

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
// Run tests with the simulator.

#define SETUP()        \
  MacroAssembler masm; \
  SETUP_COMMON();      \
  SETUP_COMMON_SIM()

#define SETUP_WITH_FEATURES(...)                 \
  MacroAssembler masm;                           \
  SETUP_COMMON();                                \
  SETUP_COMMON_SIM();                            \
  masm.SetCPUFeatures(CPUFeatures(__VA_ARGS__)); \
  simulator.SetCPUFeatures(CPUFeatures(__VA_ARGS__))

#define SETUP_CUSTOM(size, pic)                                  \
  MacroAssembler masm(size + CodeBuffer::kDefaultCapacity, pic); \
  SETUP_COMMON();                                                \
  SETUP_COMMON_SIM()

#define SETUP_CUSTOM_SIM(...)                                   \
  MacroAssembler masm;                                          \
  SETUP_COMMON();                                               \
  Simulator simulator(&simulator_decoder, stdout, __VA_ARGS__); \
  simulator.SetColouredTrace(Test::coloured_trace());           \
  simulator.SetCPUFeatures(CPUFeatures::None())

#define SETUP_COMMON()                                                   \
  bool queried_can_run = false;                                          \
  bool printed_sve_lane_warning = false;                                 \
  /* Avoid unused-variable warnings in case a test never calls RUN(). */ \
  USE(queried_can_run);                                                  \
  USE(printed_sve_lane_warning);                                         \
  masm.SetCPUFeatures(CPUFeatures::None());                              \
  masm.SetGenerateSimulatorCode(true);                                   \
  Decoder simulator_decoder;                                             \
  RegisterDump core;                                                     \
  ptrdiff_t offset_after_infrastructure_start;                           \
  ptrdiff_t offset_before_infrastructure_end

#define SETUP_COMMON_SIM()                            \
  Simulator simulator(&simulator_decoder);            \
  simulator.SetColouredTrace(Test::coloured_trace()); \
  simulator.SetCPUFeatures(CPUFeatures::None())

#define START()                                                               \
  masm.Reset();                                                               \
  simulator.ResetState();                                                     \
  {                                                                           \
    SimulationCPUFeaturesScope cpu(&masm, kInfrastructureCPUFeatures);        \
    __ PushCalleeSavedRegisters();                                            \
  }                                                                           \
  /* The infrastructure code hasn't been covered at the moment, e.g. */       \
  /* prologue/epilogue. Suppress tagging mis-match exception before  */       \
  /* this point. */                                                           \
  if (masm.GetCPUFeatures()->Has(CPUFeatures::kMTE)) {                        \
    __ Hlt(DebugHltOpcode::kMTEActive);                                       \
  }                                                                           \
  {                                                                           \
    int trace_parameters = 0;                                                 \
    if (Test::trace_reg()) trace_parameters |= LOG_STATE;                     \
    if (Test::trace_write()) trace_parameters |= LOG_WRITE;                   \
    if (Test::trace_sim()) trace_parameters |= LOG_DISASM;                    \
    if (Test::trace_branch()) trace_parameters |= LOG_BRANCH;                 \
    if (trace_parameters != 0) {                                              \
      __ Trace(static_cast<TraceParameters>(trace_parameters), TRACE_ENABLE); \
    }                                                                         \
  }                                                                           \
  offset_after_infrastructure_start = masm.GetCursorOffset();                 \
  /* Avoid unused-variable warnings in case a test never calls RUN(). */      \
  USE(offset_after_infrastructure_start)

#define END()                                                            \
  offset_before_infrastructure_end = masm.GetCursorOffset();             \
  /* Avoid unused-variable warnings in case a test never calls RUN(). */ \
  USE(offset_before_infrastructure_end);                                 \
  __ Trace(LOG_ALL, TRACE_DISABLE);                                      \
  if (masm.GetCPUFeatures()->Has(CPUFeatures::kMTE)) {                   \
    __ Hlt(DebugHltOpcode::kMTEInactive);                                \
  }                                                                      \
  {                                                                      \
    SimulationCPUFeaturesScope cpu(&masm, kInfrastructureCPUFeatures);   \
    core.Dump(&masm);                                                    \
    __ PopCalleeSavedRegisters();                                        \
  }                                                                      \
  __ Ret();                                                              \
  masm.FinalizeCode()

#define RUN()                                                                  \
  RUN_WITHOUT_SEEN_FEATURE_CHECK();                                            \
  {                                                                            \
    /* We expect the test to use all of the features it requested, plus the */ \
    /* features that the instruction code requires.                         */ \
    CPUFeatures const& expected_features =                                     \
        simulator.GetCPUFeatures()->With(CPUFeatures::kNEON);                  \
    CPUFeatures const& seen = simulator.GetSeenFeatures();                     \
    /* This gives three broad categories of features that we care about:    */ \
    /*  1. Things both expected and seen.                                   */ \
    /*  2. Things seen, but not expected. The simulator catches these.      */ \
    /*  3. Things expected, but not seen. We check these here.              */ \
    /* In a valid, passing test, categories 2 and 3 should be empty.        */ \
    if (seen != expected_features) {                                           \
      /* The Simulator should have caught anything in category 2 already.   */ \
      VIXL_ASSERT(expected_features.Has(seen));                                \
      /* Anything left is category 3: things expected, but not seen. This   */ \
      /* is not necessarily a bug in VIXL itself, but indicates that the    */ \
      /* test is less strict than it could be.                              */ \
      CPUFeatures missing = expected_features.Without(seen);                   \
      VIXL_ASSERT(missing.Count() > 0);                                        \
      std::cout << "Error: expected to see CPUFeatures { " << missing          \
                << " }\n";                                                     \
      VIXL_ABORT();                                                            \
    }                                                                          \
  }

#define RUN_WITHOUT_SEEN_FEATURE_CHECK() \
  DISASSEMBLE();                         \
  VIXL_ASSERT(QUERIED_CAN_RUN());        \
  VIXL_ASSERT(CAN_RUN());                \
  simulator.RunFrom(masm.GetBuffer()->GetStartAddress<Instruction*>())

#else  // ifdef VIXL_INCLUDE_SIMULATOR_AARCH64.
#define SETUP()        \
  MacroAssembler masm; \
  SETUP_COMMON()

#define SETUP_WITH_FEATURES(...) \
  MacroAssembler masm;           \
  SETUP_COMMON();                \
  masm.SetCPUFeatures(CPUFeatures(__VA_ARGS__))

#define SETUP_CUSTOM(size, pic)                             \
  size_t buffer_size = size + CodeBuffer::kDefaultCapacity; \
  MacroAssembler masm(buffer_size, pic);                    \
  SETUP_COMMON()

#define SETUP_COMMON()                                                   \
  bool queried_can_run = false;                                          \
  bool printed_sve_lane_warning = false;                                 \
  /* Avoid unused-variable warnings in case a test never calls RUN(). */ \
  USE(queried_can_run);                                                  \
  USE(printed_sve_lane_warning);                                         \
  masm.SetCPUFeatures(CPUFeatures::None());                              \
  masm.SetGenerateSimulatorCode(false);                                  \
  RegisterDump core;                                                     \
  CPU::SetUp();                                                          \
  ptrdiff_t offset_after_infrastructure_start;                           \
  ptrdiff_t offset_before_infrastructure_end

#define START()                                                          \
  masm.Reset();                                                          \
  {                                                                      \
    CPUFeaturesScope cpu(&masm, kInfrastructureCPUFeatures);             \
    __ PushCalleeSavedRegisters();                                       \
  }                                                                      \
  offset_after_infrastructure_start = masm.GetCursorOffset();            \
  /* Avoid unused-variable warnings in case a test never calls RUN(). */ \
  USE(offset_after_infrastructure_start)

#define END()                                                            \
  offset_before_infrastructure_end = masm.GetCursorOffset();             \
  /* Avoid unused-variable warnings in case a test never calls RUN(). */ \
  USE(offset_before_infrastructure_end);                                 \
  {                                                                      \
    CPUFeaturesScope cpu(&masm, kInfrastructureCPUFeatures);             \
    core.Dump(&masm);                                                    \
    __ PopCalleeSavedRegisters();                                        \
  }                                                                      \
  __ Ret();                                                              \
  masm.FinalizeCode()

// Execute the generated code from the memory area.
#define RUN()                                               \
  DISASSEMBLE();                                            \
  VIXL_ASSERT(QUERIED_CAN_RUN());                           \
  VIXL_ASSERT(CAN_RUN());                                   \
  masm.GetBuffer()->SetExecutable();                        \
  ExecuteMemory(masm.GetBuffer()->GetStartAddress<byte*>(), \
                masm.GetSizeOfCodeGenerated());             \
  masm.GetBuffer()->SetWritable()

// This just provides compatibility with VIXL_INCLUDE_SIMULATOR_AARCH64 builds.
// We cannot run seen-feature checks when running natively.
#define RUN_WITHOUT_SEEN_FEATURE_CHECK() RUN()

#endif  // ifdef VIXL_INCLUDE_SIMULATOR_AARCH64.

#define CAN_RUN() CanRun(*masm.GetCPUFeatures(), &queried_can_run)
#define QUERIED_CAN_RUN() (queried_can_run)

#define DISASSEMBLE()                                                     \
  if (Test::disassemble()) {                                              \
    PrintDisassembler disasm(stdout);                                     \
    CodeBuffer* buffer = masm.GetBuffer();                                \
    Instruction* test_start = buffer->GetOffsetAddress<Instruction*>(     \
        offset_after_infrastructure_start);                               \
    Instruction* test_end = buffer->GetOffsetAddress<Instruction*>(       \
        offset_before_infrastructure_end);                                \
                                                                          \
    if (Test::disassemble_infrastructure()) {                             \
      Instruction* infra_start = buffer->GetStartAddress<Instruction*>(); \
      printf("# Infrastructure code (prologue)\n");                       \
      disasm.DisassembleBuffer(infra_start, test_start);                  \
      printf("# Test code\n");                                            \
    } else {                                                              \
      printf(                                                             \
          "# Warning: Omitting infrastructure code. "                     \
          "Use --disassemble to see it.\n");                              \
    }                                                                     \
                                                                          \
    disasm.DisassembleBuffer(test_start, test_end);                       \
                                                                          \
    if (Test::disassemble_infrastructure()) {                             \
      printf("# Infrastructure code (epilogue)\n");                       \
      Instruction* infra_end = buffer->GetEndAddress<Instruction*>();     \
      disasm.DisassembleBuffer(test_end, infra_end);                      \
    }                                                                     \
  }

#define ASSERT_EQUAL_NZCV(expected) \
  VIXL_CHECK(EqualNzcv(expected, core.flags_nzcv()))

#define ASSERT_EQUAL_REGISTERS(expected) \
  VIXL_CHECK(EqualRegisters(&expected, &core))

#define ASSERT_EQUAL_FP16(expected, result) \
  VIXL_CHECK(EqualFP16(expected, &core, result))

#define ASSERT_EQUAL_32(expected, result) \
  VIXL_CHECK(Equal32(static_cast<uint32_t>(expected), &core, result))

#define ASSERT_EQUAL_FP32(expected, result) \
  VIXL_CHECK(EqualFP32(expected, &core, result))

#define ASSERT_EQUAL_64(expected, result) \
  VIXL_CHECK(Equal64(expected, &core, result))

#define ASSERT_NOT_EQUAL_64(expected, result) \
  VIXL_CHECK(NotEqual64(expected, &core, result))

#define ASSERT_EQUAL_FP64(expected, result) \
  VIXL_CHECK(EqualFP64(expected, &core, result))

#define ASSERT_EQUAL_128(expected_h, expected_l, result) \
  VIXL_CHECK(Equal128(expected_h, expected_l, &core, result))

#define ASSERT_LITERAL_POOL_SIZE(expected) \
  VIXL_CHECK((expected + kInstructionSize) == (masm.GetLiteralPoolSize()))

#define ASSERT_EQUAL_SVE_LANE(expected, result, lane) \
  VIXL_CHECK(EqualSVELane(expected, &core, result, lane));

// If `expected` is scalar, check that every lane of `result` matches it.
// If `expected` is an array of N expected values, check that the first N
// lanes on `result` match. The rightmost (highest-indexed) array element maps
// to the lowest-numbered lane.
#define ASSERT_EQUAL_SVE(expected, result) \
  VIXL_CHECK(EqualSVE(expected, &core, result, &printed_sve_lane_warning))

#define ASSERT_EQUAL_MEMORY(expected, result, ...)          \
  VIXL_CHECK(EqualMemory(reinterpret_cast<void*>(expected), \
                         reinterpret_cast<void*>(result),   \
                         __VA_ARGS__))

#define MUST_FAIL_WITH_MESSAGE(code, message)                     \
  {                                                               \
    bool aborted = false;                                         \
    try {                                                         \
      code;                                                       \
    } catch (const std::runtime_error& e) {                       \
      const char* expected_error = message;                       \
      size_t error_length = strlen(expected_error);               \
      if (strncmp(expected_error, e.what(), error_length) == 0) { \
        aborted = true;                                           \
      } else {                                                    \
        printf("Mismatch in error message.\n");                   \
        printf("Expected: %s\n", expected_error);                 \
        printf("Found:    %s\n", e.what());                       \
      }                                                           \
    }                                                             \
    VIXL_CHECK(aborted);                                          \
  }

}  // namespace aarch64
}  // namespace vixl
