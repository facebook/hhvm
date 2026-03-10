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

#define TEST(name) TEST_(AARCH64_DISASM_##name)

#define SETUP_COMMON()                                \
  MacroAssembler masm;                                \
  masm.GetCPUFeatures()->Combine(CPUFeatures::All()); \
  Decoder decoder;                                    \
  Disassembler disasm;                                \
  decoder.AppendVisitor(&disasm)

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64
// Run tests with the simulator.
#define SETUP()   \
  SETUP_COMMON(); \
  masm.SetGenerateSimulatorCode(true)

#else  // ifdef VIXL_INCLUDE_SIMULATOR_AARCH64.
#define SETUP()   \
  SETUP_COMMON(); \
  masm.SetGenerateSimulatorCode(false)

#endif  // ifdef VIXL_INCLUDE_SIMULATOR_AARCH64.

// A conservative limit for the size of the code that we generate in these
// tests.
#define MAX_SIZE_GENERATED 1024

#define DISASSEMBLE()                                                       \
  do {                                                                      \
    printf("----\n");                                                       \
    PrintDisassembler print_disasm(stdout);                                 \
    Instruction* dis_start =                                                \
        masm.GetBuffer()->GetStartAddress<Instruction*>();                  \
    Instruction* dis_end = masm.GetBuffer()->GetEndAddress<Instruction*>(); \
    print_disasm.DisassembleBuffer(dis_start, dis_end);                     \
  } while (0)

#define COMPARE(ASM, EXP)                                                \
  do {                                                                   \
    masm.Reset();                                                        \
    {                                                                    \
      ExactAssemblyScope guard(&masm,                                    \
                               MAX_SIZE_GENERATED,                       \
                               ExactAssemblyScope::kMaximumSize);        \
      masm.ASM;                                                          \
    }                                                                    \
    masm.FinalizeCode();                                                 \
    decoder.Decode(masm.GetBuffer()->GetStartAddress<Instruction*>());   \
    uint32_t encoding = *masm.GetBuffer()->GetStartAddress<uint32_t*>(); \
    if (strcmp(disasm.GetOutput(), EXP) != 0) {                          \
      printf("\nEncoding: %08" PRIx32 "\nExpected: %s\nFound:    %s\n",  \
             encoding,                                                   \
             EXP,                                                        \
             disasm.GetOutput());                                        \
      abort();                                                           \
    }                                                                    \
    if (Test::disassemble()) DISASSEMBLE();                              \
  } while (0)

#define COMPARE_PREFIX(ASM, EXP)                                         \
  do {                                                                   \
    masm.Reset();                                                        \
    {                                                                    \
      ExactAssemblyScope guard(&masm,                                    \
                               MAX_SIZE_GENERATED,                       \
                               ExactAssemblyScope::kMaximumSize);        \
      masm.ASM;                                                          \
    }                                                                    \
    masm.FinalizeCode();                                                 \
    decoder.Decode(masm.GetBuffer()->GetStartAddress<Instruction*>());   \
    uint32_t encoding = *masm.GetBuffer()->GetStartAddress<uint32_t*>(); \
    if (strncmp(disasm.GetOutput(), EXP, strlen(EXP)) != 0) {            \
      printf("\nEncoding: %08" PRIx32 "\nExpected: %s\nFound:    %s\n",  \
             encoding,                                                   \
             EXP,                                                        \
             disasm.GetOutput());                                        \
      abort();                                                           \
    }                                                                    \
    if (Test::disassemble()) DISASSEMBLE();                              \
  } while (0)

#define COMPARE_MACRO_BASE(ASM, EXP)                        \
  masm.Reset();                                             \
  masm.ASM;                                                 \
  masm.FinalizeCode();                                      \
  std::string res;                                          \
                                                            \
  Instruction* instruction =                                \
      masm.GetBuffer()->GetStartAddress<Instruction*>();    \
  Instruction* end = masm.GetCursorAddress<Instruction*>(); \
  while (instruction != end) {                              \
    decoder.Decode(instruction);                            \
    res.append(disasm.GetOutput());                         \
    instruction = instruction->GetNextInstruction();        \
    if (instruction != end) {                               \
      res.append("\n");                                     \
    }                                                       \
  }

#define COMPARE_MACRO(ASM, EXP)                                 \
  do {                                                          \
    COMPARE_MACRO_BASE(ASM, EXP)                                \
    if (strcmp(res.c_str(), EXP) != 0) {                        \
      printf("Expected: %s\nFound:    %s\n", EXP, res.c_str()); \
      abort();                                                  \
    }                                                           \
    if (Test::disassemble()) DISASSEMBLE();                     \
  } while (0)

#define COMPARE_MACRO_PREFIX(ASM, EXP)                                   \
  do {                                                                   \
    COMPARE_MACRO_BASE(ASM, EXP)                                         \
    if (strncmp(res.c_str(), EXP, strlen(EXP)) != 0) {                   \
      printf("Expected (prefix): %s\nFound:    %s\n", EXP, res.c_str()); \
      abort();                                                           \
    }                                                                    \
    if (Test::disassemble()) DISASSEMBLE();                              \
  } while (0)

#define CLEANUP()
