// Copyright 2013 ARM Limited
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
// THIS SOFTWARE IS PROVIDED BY ARM LIMITED AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL ARM LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "hphp/vixl/a64/debugger-a64.h"

#include "folly/Format.h"

namespace vixl {

// List of commands supported by the debugger.
#define DEBUG_COMMAND_LIST(C)  \
C(HelpCommand)                 \
C(ContinueCommand)             \
C(StepCommand)                 \
C(DisasmCommand)               \
C(PrintCommand)                \
C(MemCommand)

// Debugger command lines are broken up in token of different type to make
// processing easier later on.
class Token {
 public:
  virtual ~Token() {}

  // Token type.
  virtual bool IsRegister() const { return false; }
  virtual bool IsFPRegister() const { return false; }
  virtual bool IsIdentifier() const { return false; }
  virtual bool IsAddress() const { return false; }
  virtual bool IsInteger() const { return false; }
  virtual bool IsFormat() const { return false; }
  virtual bool IsUnknown() const { return false; }
  // Token properties.
  virtual bool CanAddressMemory() const { return false; }
  virtual uint8_t* ToAddress(Debugger* debugger) const;
  virtual void Print(FILE* out = stdout) const = 0;

  static Token* Tokenize(const char* arg);
};

// Tokens often hold one value.
template<typename T> class ValueToken : public Token {
 public:
  explicit ValueToken(T value) : value_(value) {}
  ValueToken() {}

  T value() const { return value_; }

 protected:
  T value_;
};

// Integer registers (X or W) and their aliases.
// Format: wn or xn with 0 <= n < 32 or a name in the aliases list.
class RegisterToken : public ValueToken<const Register> {
 public:
  explicit RegisterToken(const Register reg)
      : ValueToken<const Register>(reg) {}

  virtual bool IsRegister() const { return true; }
  virtual bool CanAddressMemory() const { return value().Is64Bits(); }
  virtual uint8_t* ToAddress(Debugger* debugger) const;
  virtual void Print(FILE* out = stdout) const ;
  const char* Name() const;

  static Token* Tokenize(const char* arg);
  static RegisterToken* Cast(Token* tok) {
    assert(tok->IsRegister());
    return reinterpret_cast<RegisterToken*>(tok);
  }

 private:
  static const int kMaxAliasNumber = 4;
  static const char* kXAliases[kNumberOfRegisters][kMaxAliasNumber];
  static const char* kWAliases[kNumberOfRegisters][kMaxAliasNumber];
};

// Floating point registers (D or S).
// Format: sn or dn with 0 <= n < 32.
class FPRegisterToken : public ValueToken<const FPRegister> {
 public:
  explicit FPRegisterToken(const FPRegister fpreg)
      : ValueToken<const FPRegister>(fpreg) {}

  virtual bool IsFPRegister() const { return true; }
  virtual void Print(FILE* out = stdout) const ;

  static Token* Tokenize(const char* arg);
  static FPRegisterToken* Cast(Token* tok) {
    assert(tok->IsFPRegister());
    return reinterpret_cast<FPRegisterToken*>(tok);
  }
};


// Non-register identifiers.
// Format: Alphanumeric string starting with a letter.
class IdentifierToken : public ValueToken<char*> {
 public:
  explicit IdentifierToken(const char* name) {
    int size = strlen(name) + 1;
    value_ = new char[size];
    strncpy(value_, name, size);
  }
  virtual ~IdentifierToken() { delete[] value_; }

  virtual bool IsIdentifier() const { return true; }
  virtual bool CanAddressMemory() const { return strcmp(value(), "pc") == 0; }
  virtual uint8_t* ToAddress(Debugger* debugger) const;
  virtual void Print(FILE* out = stdout) const;

  static Token* Tokenize(const char* arg);
  static IdentifierToken* Cast(Token* tok) {
    assert(tok->IsIdentifier());
    return reinterpret_cast<IdentifierToken*>(tok);
  }
};

// 64-bit address literal.
// Format: 0x... with up to 16 hexadecimal digits.
class AddressToken : public ValueToken<uint8_t*> {
 public:
  explicit AddressToken(uint8_t* address) : ValueToken<uint8_t*>(address) {}

  virtual bool IsAddress() const { return true; }
  virtual bool CanAddressMemory() const { return true; }
  virtual uint8_t* ToAddress(Debugger* debugger) const;
  virtual void Print(FILE* out = stdout) const ;

  static Token* Tokenize(const char* arg);
  static AddressToken* Cast(Token* tok) {
    assert(tok->IsAddress());
    return reinterpret_cast<AddressToken*>(tok);
  }
};


// 64-bit decimal integer literal.
// Format: n.
class IntegerToken : public ValueToken<int64_t> {
 public:
  explicit IntegerToken(int value) : ValueToken<int64_t>(value) {}

  virtual bool IsInteger() const { return true; }
  virtual void Print(FILE* out = stdout) const;

  static Token* Tokenize(const char* arg);
  static IntegerToken* Cast(Token* tok) {
    assert(tok->IsInteger());
    return reinterpret_cast<IntegerToken*>(tok);
  }
};

// Literal describing how to print a chunk of data (up to 64 bits).
// Format: %qt
// where q (qualifier) is one of
//  * s: signed integer
//  * u: unsigned integer
//  * a: hexadecimal floating point
// and t (type) is one of
//  * x: 64-bit integer
//  * w: 32-bit integer
//  * h: 16-bit integer
//  * b: 8-bit integer
//  * c: character
//  * d: double
//  * s: float
// When no qualifier is given for integers, they are printed in hexadecinal.
class FormatToken : public Token {
 public:
  FormatToken() {}

  virtual bool IsFormat() const { return true; }
  virtual int SizeOf() const = 0;
  virtual void PrintData(void* data, FILE* out = stdout) const = 0;
  virtual void Print(FILE* out = stdout) const = 0;

  static Token* Tokenize(const char* arg);
  static FormatToken* Cast(Token* tok) {
    assert(tok->IsFormat());
    return reinterpret_cast<FormatToken*>(tok);
  }
};


template<typename T> class Format : public FormatToken {
 public:
  explicit Format(const char* fmt) : fmt_(fmt) {}

  virtual int SizeOf() const { return sizeof(T); }
  virtual void PrintData(void* data, FILE* out = stdout) const {
    T value;
    memcpy(&value, data, sizeof(value));
    fprintf(out, fmt_, value);
  }
  virtual void Print(FILE* out = stdout) const;

 private:
  const char* fmt_;
};

// Tokens which don't fit any of the above.
class UnknownToken : public Token {
 public:
  explicit UnknownToken(const char* arg) {
    int size = strlen(arg) + 1;
    unknown_ = new char[size];
    strncpy(unknown_, arg, size);
  }
  virtual ~UnknownToken() { delete[] unknown_; }

  virtual bool IsUnknown() const { return true; }
  virtual void Print(FILE* out = stdout) const;

 private:
  char* unknown_;
};


// All debugger commands must subclass DebugCommand and implement Run, Print
// and Build. Commands must also define kHelp and kAliases.
class DebugCommand {
 public:
  explicit DebugCommand(Token* name) : name_(IdentifierToken::Cast(name)) {}
  DebugCommand() : name_(nullptr) {}
  virtual ~DebugCommand() { delete name_; }

  const char* name() { return name_->value(); }
  // Run the command on the given debugger. The command returns true if
  // execution should move to the next instruction.
  virtual bool Run(Debugger * debugger) = 0;
  virtual void Print(FILE* out = stdout);

  static bool Match(const char* name, const char** aliases);
  static DebugCommand* Parse(char* line);
  static void PrintHelp(const char** aliases,
                        const char* args,
                        const char* help);

 private:
  IdentifierToken* name_;
};

// For all commands below see their respective kHelp and kAliases in
// debugger-a64.cc
class HelpCommand : public DebugCommand {
 public:
  explicit HelpCommand(Token* name) : DebugCommand(name) {}

  virtual bool Run(Debugger* debugger);

  static DebugCommand* Build(std::vector<Token*> args);

  static const char* kHelp;
  static const char* kAliases[];
  static const char* kArguments;
};


class ContinueCommand : public DebugCommand {
 public:
  explicit ContinueCommand(Token* name) : DebugCommand(name) {}

  virtual bool Run(Debugger* debugger);

  static DebugCommand* Build(std::vector<Token*> args);

  static const char* kHelp;
  static const char* kAliases[];
  static const char* kArguments;
};


class StepCommand : public DebugCommand {
 public:
  StepCommand(Token* name, IntegerToken* count)
      : DebugCommand(name), count_(count) {}
  virtual ~StepCommand() { delete count_; }

  int64_t count() { return count_->value(); }
  virtual bool Run(Debugger* debugger);
  virtual void Print(FILE* out = stdout);

  static DebugCommand* Build(std::vector<Token*> args);

  static const char* kHelp;
  static const char* kAliases[];
  static const char* kArguments;

 private:
  IntegerToken* count_;
};

class DisasmCommand : public DebugCommand {
 public:
  DisasmCommand(Token* name, Token* target, IntegerToken* count)
      : DebugCommand(name), target_(target), count_(count) {}
  virtual ~DisasmCommand() {
    delete target_;
    delete count_;
  }

  Token* target() { return target_; }
  int64_t count() { return count_->value(); }
  virtual bool Run(Debugger* debugger);
  virtual void Print(FILE* out = stdout);

  static DebugCommand* Build(std::vector<Token*> args);

  static const char* kHelp;
  static const char* kAliases[];
  static const char* kArguments;

 private:
  Token* target_;
  IntegerToken* count_;
};


class PrintCommand : public DebugCommand {
 public:
  PrintCommand(Token* name, Token* target)
      : DebugCommand(name), target_(target) {}
  virtual ~PrintCommand() { delete target_; }

  Token* target() { return target_; }
  virtual bool Run(Debugger* debugger);
  virtual void Print(FILE* out = stdout);

  static DebugCommand* Build(std::vector<Token*> args);

  static const char* kHelp;
  static const char* kAliases[];
  static const char* kArguments;

 private:
  Token* target_;
};

class MemCommand : public DebugCommand {
 public:
  MemCommand(Token* name,
             Token* target,
             IntegerToken* count,
             FormatToken* format)
      : DebugCommand(name), target_(target), count_(count), format_(format) {}
  virtual ~MemCommand() {
    delete target_;
    delete count_;
    delete format_;
  }

  Token* target() { return target_; }
  int64_t count() { return count_->value(); }
  FormatToken* format() { return format_; }
  virtual bool Run(Debugger* debugger);
  virtual void Print(FILE* out = stdout);

  static DebugCommand* Build(std::vector<Token*> args);

  static const char* kHelp;
  static const char* kAliases[];
  static const char* kArguments;

 private:
  Token* target_;
  IntegerToken* count_;
  FormatToken* format_;
};

// Commands which name does not match any of the known commnand.
class UnknownCommand : public DebugCommand {
 public:
  explicit UnknownCommand(std::vector<Token*> args) : args_(args) {}
  virtual ~UnknownCommand();

  virtual bool Run(Debugger* debugger);

 private:
  std::vector<Token*> args_;
};

// Commands which name match a known command but the syntax is invalid.
class InvalidCommand : public DebugCommand {
 public:
  InvalidCommand(std::vector<Token*> args, int index, const char* cause)
      : args_(args), index_(index), cause_(cause) {}
  virtual ~InvalidCommand();

  virtual bool Run(Debugger* debugger);

 private:
  std::vector<Token*> args_;
  int index_;
  const char* cause_;
};

const char* HelpCommand::kAliases[] = { "help", nullptr };
const char* HelpCommand::kArguments = nullptr;
const char* HelpCommand::kHelp = "  print this help";

const char* ContinueCommand::kAliases[] = { "continue", "c", nullptr };
const char* ContinueCommand::kArguments = nullptr;
const char* ContinueCommand::kHelp = "  resume execution";

const char* StepCommand::kAliases[] = { "stepi", "si", nullptr };
const char* StepCommand::kArguments = "[n = 1]";
const char* StepCommand::kHelp = "  execute n next instruction(s)";

const char* DisasmCommand::kAliases[] = { "dis", "d", nullptr };
const char* DisasmCommand::kArguments = "[addr = pc] [n = 1]";
const char* DisasmCommand::kHelp =
  "  disassemble n instruction(s) at address addr.\n"
  "  addr can be an immediate address, a register or the pc."
;

const char* PrintCommand::kAliases[] = { "print", "p", nullptr };
const char* PrintCommand::kArguments =  "<entity>";
const char* PrintCommand::kHelp =
  "  print the given entity\n"
  "  entity can be 'regs' for W and X registers, 'fpregs' for S and D\n"
  "  registers, 'sysregs' for system registers (including NZCV) or 'pc'."
;

const char* MemCommand::kAliases[] = { "mem", "m", nullptr };
const char* MemCommand::kArguments = "<addr> [n = 1] [format = %x]";
const char* MemCommand::kHelp =
  "  print n memory item(s) at address addr according to the given format.\n"
  "  addr can be an immediate address, a register or the pc.\n"
  "  format is made of a qualifer: 's', 'u', 'a' (signed, unsigned, hexa)\n"
  "  and a type 'x', 'w', 'h', 'b' (64- to 8-bit integer), 'c' (character),\n"
  "  's' (float) or 'd' (double). E.g 'mem sp %w' will print a 32-bit word\n"
  "  from the stack as an hexadecimal number."
;

const char* RegisterToken::kXAliases[kNumberOfRegisters][kMaxAliasNumber] = {
  { "x0", nullptr },
  { "x1", nullptr },
  { "x2", nullptr },
  { "x3", nullptr },
  { "x4", nullptr },
  { "x5", nullptr },
  { "x6", nullptr },
  { "x7", nullptr },
  { "x8", nullptr },
  { "x9", nullptr },
  { "x10", nullptr },
  { "x11", nullptr },
  { "x12", nullptr },
  { "x13", nullptr },
  { "x14", nullptr },
  { "x15", nullptr },
  { "ip0", "x16", nullptr },
  { "ip1", "x17", nullptr },
  { "x18", "pr", nullptr },
  { "x19", nullptr },
  { "x20", nullptr },
  { "x21", nullptr },
  { "x22", nullptr },
  { "x23", nullptr },
  { "x24", nullptr },
  { "x25", nullptr },
  { "x26", nullptr },
  { "x27", nullptr },
  { "x28", nullptr },
  { "fp", "x29", nullptr },
  { "lr", "x30", nullptr },
  { "sp", nullptr}
};

const char* RegisterToken::kWAliases[kNumberOfRegisters][kMaxAliasNumber] = {
  { "w0", nullptr },
  { "w1", nullptr },
  { "w2", nullptr },
  { "w3", nullptr },
  { "w4", nullptr },
  { "w5", nullptr },
  { "w6", nullptr },
  { "w7", nullptr },
  { "w8", nullptr },
  { "w9", nullptr },
  { "w10", nullptr },
  { "w11", nullptr },
  { "w12", nullptr },
  { "w13", nullptr },
  { "w14", nullptr },
  { "w15", nullptr },
  { "w16", nullptr },
  { "w17", nullptr },
  { "w18", nullptr },
  { "w19", nullptr },
  { "w20", nullptr },
  { "w21", nullptr },
  { "w22", nullptr },
  { "w23", nullptr },
  { "w24", nullptr },
  { "w25", nullptr },
  { "w26", nullptr },
  { "w27", nullptr },
  { "w28", nullptr },
  { "w29", nullptr },
  { "w30", nullptr },
  { "wsp", nullptr }
};


Debugger::Debugger(Decoder* decoder, std::ostream& stream)
    : Simulator(decoder, stream),
      log_parameters_(0),
      debug_parameters_(0),
      pending_request_(false),
      steps_(0),
      last_command_(nullptr) {
  disasm_ = new PrintDisassembler(std::cout);
  printer_ = new Decoder();
  printer_->AppendVisitor(disasm_);
}


void Debugger::Run() {
  while (pc_ != kEndOfSimAddress) {
    if (pending_request()) {
      LogProcessorState();
      RunDebuggerShell();
    }

    ExecuteInstruction();
  }
}


void Debugger::PrintInstructions(void* address, int64_t count) {
  if (count == 0) {
    return;
  }

  Instruction* from = Instruction::Cast(address);
  if (count < 0) {
    count = -count;
    from -= (count - 1) * kInstructionSize;
  }
  Instruction* to = from + count * kInstructionSize;

  for (Instruction* current = from;
       current < to;
       current = current->NextInstruction()) {
    printer_->Decode(current);
  }
}


void Debugger::PrintMemory(const uint8_t* address,
                           int64_t count,
                           const FormatToken* format) {
  if (count == 0) {
    return;
  }

  const uint8_t* from = address;
  int size = format->SizeOf();
  if (count < 0) {
    count = -count;
    from -= (count - 1) * size;
  }
  const uint8_t* to = from + count * size;

  for (const uint8_t* current = from; current < to; current += size) {
    if (((current - from) % 16) == 0) {
      printf("\n%p: ", current);
    }

    uint64_t data = MemoryRead(current, size);
    format->PrintData(&data);
    printf(" ");
  }
  printf("\n\n");
}


void Debugger::VisitException(Instruction* instr) {
  switch (instr->Mask(ExceptionMask)) {
    case BRK:
      DoBreakpoint(instr);
      return;
    case HLT:
      switch (instr->ImmException()) {
        case kUnreachableOpcode:
          DoUnreachable(instr);
          return;
        case kTraceOpcode:
          DoTrace(instr);
          return;
        case kLogOpcode:
          DoLog(instr);
          return;
      }
      // Fall through
    default: Simulator::VisitException(instr);
  }
}


void Debugger::LogSystemRegisters() {
  if (log_parameters_ & LOG_SYS_REGS) PrintSystemRegisters();
}


void Debugger::LogRegisters() {
  if (log_parameters_ & LOG_REGS) PrintRegisters();
}


void Debugger::LogFPRegisters() {
  if (log_parameters_ & LOG_FP_REGS) PrintFPRegisters();
}


void Debugger::LogProcessorState() {
  LogSystemRegisters();
  LogRegisters();
  LogFPRegisters();
}


// Read a command. A command will be at most kMaxDebugShellLine char long and
// ends with '\n\0'.
// TODO: Should this be a utility function?
char* Debugger::ReadCommandLine(const char* prompt, char* buffer, int length) {
  int fgets_calls = 0;
  char* end = nullptr;

  printf("%s", prompt);
  fflush(stdout);

  do {
    if (fgets(buffer, length, stdin) == nullptr) {
      printf(" ** Error while reading command. **\n");
      return nullptr;
    }

    fgets_calls++;
    end = strchr(buffer, '\n');
  } while (end == nullptr);

  if (fgets_calls != 1) {
    printf(" ** Command too long. **\n");
    return nullptr;
  }

  // Remove the newline from the end of the command.
  assert(end[1] == '\0');
  assert((end - buffer) < (length - 1));
  end[0] = '\0';

  return buffer;
}


void Debugger::RunDebuggerShell() {
  if (IsDebuggerRunning()) {
    if (steps_ > 0) {
      // Finish stepping first.
      --steps_;
      return;
    }

    printf("Next: ");
    PrintInstructions(pc());
    bool done = false;
    while (!done) {
      char buffer[kMaxDebugShellLine];
      char* line = ReadCommandLine("vixl> ", buffer, kMaxDebugShellLine);

      if (line == nullptr) continue;  // An error occurred.

      DebugCommand* command = DebugCommand::Parse(line);
      if (command != nullptr) {
        last_command_ = command;
      }

      if (last_command_ != nullptr) {
        done = last_command_->Run(this);
      } else {
        printf("No previous command to run!\n");
      }
    }

    if ((debug_parameters_ & DBG_BREAK) != 0) {
      // The break request has now been handled, move to next instruction.
      debug_parameters_ &= ~DBG_BREAK;
      increment_pc();
    }
  }
}


void Debugger::DoBreakpoint(Instruction* instr) {
  assert(instr->Mask(ExceptionMask) == BRK);

  printf("Hit breakpoint at pc=%p.\n", reinterpret_cast<void*>(instr));
  set_debug_parameters(debug_parameters() | DBG_BREAK | DBG_ACTIVE);
  // Make the shell point to the brk instruction.
  set_pc(instr);
}


void Debugger::DoUnreachable(Instruction* instr) {
  assert((instr->Mask(ExceptionMask) == HLT) &&
         (instr->ImmException() == kUnreachableOpcode));

  stream_ << folly::format("Hit not_reached marker at pc={:#x}.\n",
                           reinterpret_cast<void*>(instr));
  abort();
}


void Debugger::DoTrace(Instruction* instr) {
  assert((instr->Mask(ExceptionMask) == HLT) &&
         (instr->ImmException() == kTraceOpcode));

  // Read the arguments encoded inline in the instruction stream.
  uint32_t parameters;
  uint32_t command;

  assert(sizeof(*instr) == 1);
  memcpy(&parameters, instr + kTraceParamsOffset, sizeof(parameters));
  memcpy(&command, instr + kTraceCommandOffset, sizeof(command));

  switch (command) {
    case TRACE_ENABLE:
      set_log_parameters(log_parameters() | parameters);
      break;
    case TRACE_DISABLE:
      set_log_parameters(log_parameters() & ~parameters);
      break;
    default:
      not_reached();
  }

  set_pc(instr->InstructionAtOffset(kTraceLength));
}


void Debugger::DoLog(Instruction* instr) {
  assert((instr->Mask(ExceptionMask) == HLT) &&
         (instr->ImmException() == kLogOpcode));

  // Read the arguments encoded inline in the instruction stream.
  uint32_t parameters;

  assert(sizeof(*instr) == 1);
  memcpy(&parameters, instr + kTraceParamsOffset, sizeof(parameters));

  // We don't support a one-shot LOG_DISASM.
  assert((parameters & LOG_DISASM) == 0);
  // Print the requested information.
  if (parameters & LOG_SYS_REGS) PrintSystemRegisters(true);
  if (parameters & LOG_REGS) PrintRegisters(true);
  if (parameters & LOG_FP_REGS) PrintFPRegisters(true);

  set_pc(instr->InstructionAtOffset(kLogLength));
}


static bool StringToUInt64(uint64_t* value, const char* line, int base = 10) {
  char* endptr = nullptr;
  errno = 0;  // Reset errors.
  uint64_t parsed = strtoul(line, &endptr, base);

  if (errno == ERANGE) {
    // Overflow.
    return false;
  }

  if (endptr == line) {
    // No digits were parsed.
    return false;
  }

  if (*endptr != '\0') {
    // Non-digit characters present at the end.
    return false;
  }

  *value = parsed;
  return true;
}


static bool StringToInt64(int64_t* value, const char* line, int base = 10) {
  char* endptr = nullptr;
  errno = 0;  // Reset errors.
  int64_t parsed = strtol(line, &endptr, base);

  if (errno == ERANGE) {
    // Overflow, undeflow.
    return false;
  }

  if (endptr == line) {
    // No digits were parsed.
    return false;
  }

  if (*endptr != '\0') {
    // Non-digit characters present at the end.
    return false;
  }

  *value = parsed;
  return true;
}


uint8_t* Token::ToAddress(Debugger* debugger) const {
  USE(debugger);
  not_reached();
  return nullptr;
}


Token* Token::Tokenize(const char* arg) {
  if ((arg == nullptr) || (*arg == '\0')) {
    return nullptr;
  }

  // The order is important. For example Identifier::Tokenize would consider
  // any register to be a valid identifier.

  Token* token = RegisterToken::Tokenize(arg);
  if (token != nullptr) {
    return token;
  }

  token = FPRegisterToken::Tokenize(arg);
  if (token != nullptr) {
    return token;
  }

  token = IdentifierToken::Tokenize(arg);
  if (token != nullptr) {
    return token;
  }

  token = AddressToken::Tokenize(arg);
  if (token != nullptr) {
    return token;
  }

  token = IntegerToken::Tokenize(arg);
  if (token != nullptr) {
    return token;
  }

  token = FormatToken::Tokenize(arg);
  if (token != nullptr) {
    return token;
  }

  return new UnknownToken(arg);
}


uint8_t* RegisterToken::ToAddress(Debugger* debugger) const {
  assert(CanAddressMemory());
  uint64_t reg_value = debugger->xreg(value().code(), Reg31IsStackPointer);
  uint8_t* address = nullptr;
  memcpy(&address, &reg_value, sizeof(address));
  return address;
}


void RegisterToken::Print(FILE* out) const {
  assert(value().IsValid());
  fprintf(out, "[Register %s]", Name());
}


const char* RegisterToken::Name() const {
  if (value().Is32Bits()) {
    return kWAliases[value().code()][0];
  } else {
    return kXAliases[value().code()][0];
  }
}


Token* RegisterToken::Tokenize(const char* arg) {
  for (unsigned i = 0; i < kNumberOfRegisters; i++) {
    // Is it a X register or alias?
    for (const char** current = kXAliases[i]; *current != nullptr; current++) {
      if (strcmp(arg, *current) == 0) {
        return new RegisterToken(Register::XRegFromCode(i));
      }
    }

    // Is it a W register or alias?
    for (const char** current = kWAliases[i]; *current != nullptr; current++) {
      if (strcmp(arg, *current) == 0) {
        return new RegisterToken(Register::WRegFromCode(i));
      }
    }
  }

  return nullptr;
}


void FPRegisterToken::Print(FILE* out) const {
  assert(value().IsValid());
  char prefix = value().Is32Bits() ? 's' : 'd';
  fprintf(out, "[FPRegister %c%" PRIu32 "]", prefix, value().code());
}


Token* FPRegisterToken::Tokenize(const char* arg) {
  if (strlen(arg) < 2) {
    return nullptr;
  }

  switch (*arg) {
    case 's':
    case 'd':
      const char* cursor = arg + 1;
      uint64_t code = 0;
      if (!StringToUInt64(&code, cursor)) {
        return nullptr;
      }

      if (code > kNumberOfFPRegisters) {
        return nullptr;
      }

      FPRegister fpreg = NoFPReg;
      switch (*arg) {
        case 's': fpreg = FPRegister::SRegFromCode(code); break;
        case 'd': fpreg = FPRegister::DRegFromCode(code); break;
        default: not_reached();
      }

      return new FPRegisterToken(fpreg);
  }

  return nullptr;
}


uint8_t* IdentifierToken::ToAddress(Debugger* debugger) const {
  assert(CanAddressMemory());
  Instruction* pc_value = debugger->pc();
  uint8_t* address = nullptr;
  memcpy(&address, &pc_value, sizeof(address));
  return address;
}

void IdentifierToken::Print(FILE* out) const {
  fprintf(out, "[Identifier %s]", value());
}


Token* IdentifierToken::Tokenize(const char* arg) {
  if (!isalpha(arg[0])) {
    return nullptr;
  }

  const char* cursor = arg + 1;
  while ((*cursor != '\0') && isalnum(*cursor)) {
    ++cursor;
  }

  if (*cursor == '\0') {
    return new IdentifierToken(arg);
  }

  return nullptr;
}


uint8_t* AddressToken::ToAddress(Debugger* debugger) const {
  USE(debugger);
  return value();
}


void AddressToken::Print(FILE* out) const {
  fprintf(out, "[Address %p]", value());
}


Token* AddressToken::Tokenize(const char* arg) {
  if ((strlen(arg) < 3) || (arg[0] != '0') || (arg[1] != 'x')) {
    return nullptr;
  }

  uint64_t ptr = 0;
  if (!StringToUInt64(&ptr, arg, 16)) {
    return nullptr;
  }

  uint8_t* address = reinterpret_cast<uint8_t*>(ptr);
  return new AddressToken(address);
}


void IntegerToken::Print(FILE* out) const {
  fprintf(out, "[Integer %" PRId64 "]", value());
}


Token* IntegerToken::Tokenize(const char* arg) {
  int64_t value = 0;
  if (!StringToInt64(&value, arg)) {
    return nullptr;
  }

  return new IntegerToken(value);
}


Token* FormatToken::Tokenize(const char* arg) {
  if (arg[0] != '%') {
    return nullptr;
  }

  int length = strlen(arg);
  if ((length < 2) || (length > 3)) {
    return nullptr;
  }

  char type = arg[length - 1];
  if (length == 2) {
    switch (type) {
      case 'x': return new Format<uint64_t>("%016" PRIx64);
      case 'w': return new Format<uint32_t>("%08" PRIx32);
      case 'h': return new Format<uint16_t>("%04" PRIx16);
      case 'b': return new Format<uint8_t>("%02" PRIx8);
      case 'c': return new Format<char>("%c");
      case 'd': return new Format<double>("%g");
      case 's': return new Format<float>("%g");
      default: return nullptr;
    }
  }

  assert(length == 3);
  switch (arg[1]) {
    case 's':
      switch (type) {
        case 'x': return new Format<int64_t>("%+20" PRId64);
        case 'w': return new Format<int32_t>("%+11" PRId32);
        case 'h': return new Format<int16_t>("%+6" PRId16);
        case 'b': return new Format<int8_t>("%+4" PRId8);
        default: return nullptr;
      }
    case 'u':
      switch (type) {
        case 'x': return new Format<uint64_t>("%20" PRIu64);
        case 'w': return new Format<uint32_t>("%10" PRIu32);
        case 'h': return new Format<uint16_t>("%5" PRIu16);
        case 'b': return new Format<uint8_t>("%3" PRIu8);
        default: return nullptr;
      }
    case 'a':
      switch (type) {
        case 'd': return new Format<double>("%a");
        case 's': return new Format<float>("%a");
        default: return nullptr;
      }
    default: return nullptr;
  }
}


template<typename T>
void Format<T>::Print(FILE* out) const {
  fprintf(out, "[Format %s - %lu byte(s)]", fmt_, sizeof(T));
}


void UnknownToken::Print(FILE* out) const {
  fprintf(out, "[Unknown %s]", unknown_);
}


void DebugCommand::Print(FILE* out) {
  fprintf(out, "%s", name());
}


bool DebugCommand::Match(const char* name, const char** aliases) {
  for (const char** current = aliases; *current != nullptr; current++) {
    if (strcmp(name, *current) == 0) {
       return true;
    }
  }

  return false;
}


DebugCommand* DebugCommand::Parse(char* line) {
  std::vector<Token*> args;

  char* saveptr = nullptr;
  for (char* chunk = strtok_r(line, " ", &saveptr);
       chunk != nullptr;
       chunk = strtok_r(nullptr, " ", &saveptr)) {
    args.push_back(Token::Tokenize(chunk));
  }

  if (args.size() == 0) {
    return nullptr;
  }

  if (!args[0]->IsIdentifier()) {
    return new InvalidCommand(args, 0, "command name is not an identifier");
  }

  const char* name = IdentifierToken::Cast(args[0])->value();
  #define RETURN_IF_MATCH(Command)       \
  if (Match(name, Command::kAliases)) {  \
    return Command::Build(args);         \
  }
  DEBUG_COMMAND_LIST(RETURN_IF_MATCH);
  #undef RETURN_IF_MATCH

  return new UnknownCommand(args);
}


void DebugCommand::PrintHelp(const char** aliases,
                             const char* args,
                             const char* help) {
  assert(aliases[0] != nullptr);
  assert(help != nullptr);

  printf("\n----\n\n");
  for (const char** current = aliases; *current != nullptr; current++) {
    if (args != nullptr) {
      printf("%s %s\n", *current, args);
    } else {
      printf("%s\n", *current);
    }
  }
  printf("\n%s\n", help);
}


bool HelpCommand::Run(Debugger* debugger) {
  assert(debugger->IsDebuggerRunning());
  USE(debugger);

  #define PRINT_HELP(Command)                     \
    DebugCommand::PrintHelp(Command::kAliases,    \
                            Command::kArguments,  \
                            Command::kHelp);
  DEBUG_COMMAND_LIST(PRINT_HELP);
  #undef PRINT_HELP
  printf("\n----\n\n");

  return false;
}


DebugCommand* HelpCommand::Build(std::vector<Token*> args) {
  if (args.size() != 1) {
    return new InvalidCommand(args, -1, "too many arguments");
  }

  return new HelpCommand(args[0]);
}


bool ContinueCommand::Run(Debugger* debugger) {
  assert(debugger->IsDebuggerRunning());

  debugger->set_debug_parameters(debugger->debug_parameters() & ~DBG_ACTIVE);
  return true;
}


DebugCommand* ContinueCommand::Build(std::vector<Token*> args) {
  if (args.size() != 1) {
    return new InvalidCommand(args, -1, "too many arguments");
  }

  return new ContinueCommand(args[0]);
}


bool StepCommand::Run(Debugger* debugger) {
  assert(debugger->IsDebuggerRunning());

  int64_t steps = count();
  if (steps < 0) {
    printf(" ** invalid value for steps: %" PRId64 " (<0) **\n", steps);
  } else if (steps > 1) {
    debugger->set_steps(steps - 1);
  }

  return true;
}


void StepCommand::Print(FILE* out) {
  fprintf(out, "%s %" PRId64 "", name(), count());
}


DebugCommand* StepCommand::Build(std::vector<Token*> args) {
  IntegerToken* count = nullptr;
  switch (args.size()) {
    case 1: {  // step [1]
      count = new IntegerToken(1);
      break;
    }
    case 2: {  // step n
      Token* first = args[1];
      if (!first->IsInteger()) {
        return new InvalidCommand(args, 1, "expects int");
      }
      count = IntegerToken::Cast(first);
      break;
    }
    default:
      return new InvalidCommand(args, -1, "too many arguments");
  }

  return new StepCommand(args[0], count);
}


bool DisasmCommand::Run(Debugger* debugger) {
  assert(debugger->IsDebuggerRunning());

  uint8_t* from = target()->ToAddress(debugger);
  debugger->PrintInstructions(from, count());

  return false;
}


void DisasmCommand::Print(FILE* out) {
  fprintf(out, "%s ", name());
  target()->Print(out);
  fprintf(out, " %" PRId64 "", count());
}


DebugCommand* DisasmCommand::Build(std::vector<Token*> args) {
  Token* address = nullptr;
  IntegerToken* count = nullptr;
  switch (args.size()) {
    case 1: {  // disasm [pc] [1]
      address = new IdentifierToken("pc");
      count = new IntegerToken(1);
      break;
    }
    case 2: {  // disasm [pc] n or disasm address [1]
      Token* first = args[1];
      if (first->IsInteger()) {
        address = new IdentifierToken("pc");
        count = IntegerToken::Cast(first);
      } else if (first->CanAddressMemory()) {
        address = first;
        count = new IntegerToken(1);
      } else {
        return new InvalidCommand(args, 1, "expects int or addr");
      }
      break;
    }
    case 3: {  // disasm address count
      Token* first = args[1];
      Token* second = args[2];
      if (!first->CanAddressMemory() || !second->IsInteger()) {
        return new InvalidCommand(args, -1, "disasm addr int");
      }
      address = first;
      count = IntegerToken::Cast(second);
      break;
    }
    default:
      return new InvalidCommand(args, -1, "wrong arguments number");
  }

  return new DisasmCommand(args[0], address, count);
}


void PrintCommand::Print(FILE* out) {
  fprintf(out, "%s ", name());
  target()->Print(out);
}


bool PrintCommand::Run(Debugger* debugger) {
  assert(debugger->IsDebuggerRunning());

  Token* tok = target();
  if (tok->IsIdentifier()) {
    char* identifier = IdentifierToken::Cast(tok)->value();
    if (strcmp(identifier, "regs") == 0) {
      debugger->PrintRegisters(true);
    } else if (strcmp(identifier, "fpregs") == 0) {
      debugger->PrintFPRegisters(true);
    } else if (strcmp(identifier, "sysregs") == 0) {
      debugger->PrintSystemRegisters(true);
    } else if (strcmp(identifier, "pc") == 0) {
      printf("pc = %16p\n", reinterpret_cast<void*>(debugger->pc()));
    } else {
      printf(" ** Unknown identifier to print: %s **\n", identifier);
    }

    return false;
  }

  if (tok->IsRegister()) {
    RegisterToken* reg_tok = RegisterToken::Cast(tok);
    Register reg = reg_tok->value();
    if (reg.Is32Bits()) {
      printf("%s = %" PRId32 "\n",
             reg_tok->Name(),
             debugger->wreg(reg.code(), Reg31IsStackPointer));
    } else {
      printf("%s = %" PRId64 "\n",
             reg_tok->Name(),
             debugger->xreg(reg.code(), Reg31IsStackPointer));
    }

    return false;
  }

  if (tok->IsFPRegister()) {
    FPRegister fpreg = FPRegisterToken::Cast(tok)->value();
    if (fpreg.Is32Bits()) {
      printf("s%u = %g\n", fpreg.code(), debugger->sreg(fpreg.code()));
    } else {
      printf("d%u = %g\n", fpreg.code(), debugger->dreg(fpreg.code()));
    }

    return false;
  }

  not_reached();
  return false;
}


DebugCommand* PrintCommand::Build(std::vector<Token*> args) {
  Token* target = nullptr;
  switch (args.size()) {
    case 2: {
      target = args[1];
      if (!target->IsRegister()
          && !target->IsFPRegister()
          && !target->IsIdentifier()) {
        return new InvalidCommand(args, 1, "expects reg or identifier");
      }
      break;
    }
    default:
      return new InvalidCommand(args, -1, "too many arguments");
  }

  return new PrintCommand(args[0], target);
}


bool MemCommand::Run(Debugger* debugger) {
  assert(debugger->IsDebuggerRunning());

  uint8_t* address = target()->ToAddress(debugger);
  debugger->PrintMemory(address, count(), format());

  return false;
}


void MemCommand::Print(FILE* out) {
  fprintf(out, "%s ", name());
  target()->Print(out);
  fprintf(out, " %" PRId64 " ", count());
  format()->Print(out);
}


DebugCommand* MemCommand::Build(std::vector<Token*> args) {
  if (args.size() < 2) {
    return new InvalidCommand(args, -1, "too few arguments");
  }

  Token* target = args[1];
  IntegerToken* count = nullptr;
  FormatToken* format = nullptr;

  if (!target->CanAddressMemory()) {
    return new InvalidCommand(args, 1, "expects address");
  }

  switch (args.size()) {
    case 2: {  // mem addressable [1] [%x]
      count = new IntegerToken(1);
      format = new Format<uint64_t>("%016x");
      break;
    }
    case 3: {  // mem addr n [%x] or mem addr [n] %f
      Token* second = args[2];
      if (second->IsInteger()) {
        count = IntegerToken::Cast(second);
        format = new Format<uint64_t>("%016x");
      } else if (second->IsFormat()) {
        count = new IntegerToken(1);
        format = FormatToken::Cast(second);
      } else {
        return new InvalidCommand(args, 2, "expects int or format");
      }
      break;
    }
    case 4: {  // mem addr n %f
      Token* second = args[2];
      Token* third = args[3];
      if (!second->IsInteger() || !third->IsFormat()) {
        return new InvalidCommand(args, -1, "mem addr >>int<< %F");
      }

      count = IntegerToken::Cast(second);
      format = FormatToken::Cast(third);
      break;
    }
    default:
      return new InvalidCommand(args, -1, "too many arguments");
  }

  return new MemCommand(args[0], target, count, format);
}


UnknownCommand::~UnknownCommand() {
  const int size = args_.size();
  for (int i = 0; i < size; ++i) {
    delete args_[i];
  }
}


bool UnknownCommand::Run(Debugger* debugger) {
  assert(debugger->IsDebuggerRunning());
  USE(debugger);

  printf(" ** Unknown Command:");
  const int size = args_.size();
  for (int i = 0; i < size; ++i) {
    printf(" ");
    args_[i]->Print(stdout);
  }
  printf(" **\n");

  return false;
}


InvalidCommand::~InvalidCommand() {
  const int size = args_.size();
  for (int i = 0; i < size; ++i) {
    delete args_[i];
  }
}


bool InvalidCommand::Run(Debugger* debugger) {
  assert(debugger->IsDebuggerRunning());
  USE(debugger);

  printf(" ** Invalid Command:");
  const int size = args_.size();
  for (int i = 0; i < size; ++i) {
    printf(" ");
    if (i == index_) {
      printf(">>");
      args_[i]->Print(stdout);
      printf("<<");
    } else {
      args_[i]->Print(stdout);
    }
  }
  printf(" **\n");
  printf(" ** %s\n", cause_);

  return false;
}

}  // namespace vixl
