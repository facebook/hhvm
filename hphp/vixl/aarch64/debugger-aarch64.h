// Copyright 2023, VIXL authors
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

#ifndef VIXL_AARCH64_DEBUGGER_AARCH64_H_
#define VIXL_AARCH64_DEBUGGER_AARCH64_H_

#include <optional>
#include <unordered_set>
#include <vector>

#include "../cpu-features.h"
#include "../globals-vixl.h"
#include "../utils-vixl.h"

#include "abi-aarch64.h"
#include "cpu-features-auditor-aarch64.h"
#include "disasm-aarch64.h"
#include "instructions-aarch64.h"
#include "simulator-aarch64.h"
#include "simulator-constants-aarch64.h"

#ifdef VIXL_INCLUDE_SIMULATOR_AARCH64

namespace vixl {
namespace aarch64 {

class Simulator;

enum DebugReturn { DebugContinue, DebugExit };


// A debugger command that performs some action when used by the simulator
// debugger.
class DebuggerCmd {
 public:
  DebuggerCmd(Simulator* sim,
              std::string cmd_word,
              std::string cmd_alias,
              std::string usage,
              std::string description);
  virtual ~DebuggerCmd() {}

  // Perform some action based on the arguments passed in. Returns true if the
  // debugger should exit after the action, false otherwise.
  virtual DebugReturn Action(const std::vector<std::string>& args) = 0;

  // Return the command word.
  std::string_view GetCommandWord() { return command_word_; }
  // Return the alias for this command. Returns an empty string if this command
  // has no alias.
  std::string_view GetCommandAlias() { return command_alias_; }
  // Return this commands usage.
  std::string_view GetArgsString() { return args_str_; }
  // Return this commands description.
  std::string_view GetDescription() { return description_; }

 protected:
  // Simulator which this command will be performed on.
  Simulator* sim_;
  // Stream to output the result of the command to.
  FILE* ostream_;
  // Command word that, when given to the interactive debugger, calls Action.
  std::string command_word_;
  // Optional alias for the command_word.
  std::string command_alias_;
  // Optional string showing the arguments that can be passed to the command.
  std::string args_str_;
  // Optional description of the command.
  std::string description_;
};


//
// Base debugger command handlers:
//


class HelpCmd : public DebuggerCmd {
 public:
  HelpCmd(Simulator* sim)
      : DebuggerCmd(sim, "help", "h", "", "Display this help message.") {}

  DebugReturn Action(const std::vector<std::string>& args) override;
};


class BreakCmd : public DebuggerCmd {
 public:
  BreakCmd(Simulator* sim)
      : DebuggerCmd(sim,
                    "break",
                    "b",
                    "<address>",
                    "Set or remove a breakpoint.") {}

  DebugReturn Action(const std::vector<std::string>& args) override;
};


class StepCmd : public DebuggerCmd {
 public:
  StepCmd(Simulator* sim)
      : DebuggerCmd(sim,
                    "step",
                    "s",
                    "[<n>]",
                    "Step n instructions, default step 1 instruction.") {}

  DebugReturn Action(const std::vector<std::string>& args) override;
};


class ContinueCmd : public DebuggerCmd {
 public:
  ContinueCmd(Simulator* sim)
      : DebuggerCmd(sim,
                    "continue",
                    "c",
                    "",
                    "Exit the debugger and continue executing instructions.") {}

  DebugReturn Action(const std::vector<std::string>& args) override;
};


class PrintCmd : public DebuggerCmd {
 public:
  PrintCmd(Simulator* sim)
      : DebuggerCmd(sim,
                    "print",
                    "p",
                    "<register|all|system>",
                    "Print the contents of a register, all registers or all"
                    " system registers.") {}

  DebugReturn Action(const std::vector<std::string>& args) override;
};


class TraceCmd : public DebuggerCmd {
 public:
  TraceCmd(Simulator* sim)
      : DebuggerCmd(sim,
                    "trace",
                    "t",
                    "",
                    "Start/stop memory and register tracing.") {}

  DebugReturn Action(const std::vector<std::string>& args) override;
};


class GdbCmd : public DebuggerCmd {
 public:
  GdbCmd(Simulator* sim)
      : DebuggerCmd(sim,
                    "gdb",
                    "g",
                    "",
                    "Enter an already running instance of gdb.") {}

  DebugReturn Action(const std::vector<std::string>& args) override;
};


// A debugger for the Simulator which takes input from the user in order to
// control the running of the Simulator.
class Debugger {
 public:
  // A pair consisting of a register character (e.g: W, X, V) and a register
  // code (e.g: 0, 1 ...31) which represents a single parsed register.
  //
  // Note: the register character is guaranteed to be upper case.
  using RegisterParsedFormat = std::pair<char, unsigned>;

  Debugger(Simulator* sim);

  // Set the input stream, from which commands are read, to a custom stream.
  void SetInputStream(std::istream* stream) { input_stream_ = stream; }

  // Register a new command for the debugger.
  template <class T>
  void RegisterCmd();

  // Set a breakpoint at the given address.
  void RegisterBreakpoint(uint64_t addr) { breakpoints_.insert(addr); }
  // Remove a breakpoint at the given address.
  void RemoveBreakpoint(uint64_t addr) { breakpoints_.erase(addr); }
  // Return true if the address is the location of a breakpoint.
  bool IsBreakpoint(uint64_t addr) const {
    return (breakpoints_.find(addr) != breakpoints_.end());
  }
  // Return true if the simulator pc is a breakpoint.
  bool IsAtBreakpoint() const;

  // Main loop for the debugger. Keep prompting for user inputted debugger
  // commands and try to execute them until a command is given that exits the
  // interactive debugger.
  void Debug();

  // Get an unsigned integer value from a string and return it in 'value'.
  // Base is used to determine the numeric base of the number to be read,
  // i.e: 8 for octal, 10 for decimal, 16 for hexadecimal and 0 for
  // auto-detect. Return true if an integer value was found, false otherwise.
  static std::optional<uint64_t> ParseUint64String(std::string_view uint64_str,
                                                   int base = 0);

  // Get a register from a string and return it in 'reg'. Return true if a
  // valid register character and code (e.g: W0, X29, V31) was found, false
  // otherwise.
  static std::optional<RegisterParsedFormat> ParseRegString(
      std::string_view reg_str);

  // Print the usage of each debugger command.
  void PrintUsage();

 private:
  // Split a string based on the separator given (a single space character by
  // default) and return as a std::vector of strings.
  static std::vector<std::string> Tokenize(std::string_view input_line,
                                           char separator = ' ');

  // Try to execute a single debugger command.
  DebugReturn ExecDebugCommand(const std::vector<std::string>& tokenized_cmd);

  // Return true if the string is zero, i.e: all characters in the string
  // (other than prefixes) are zero.
  static bool IsZeroUint64String(std::string_view uint64_str, int base);

  // The simulator that this debugger acts on.
  Simulator* sim_;

  // A vector of all commands recognised by the debugger.
  std::vector<std::unique_ptr<DebuggerCmd>> debugger_cmds_;

  // Input stream from which commands are read. Default is std::cin.
  std::istream* input_stream_;

  // Output stream from the simulator.
  FILE* ostream_;

  // A list of all instruction addresses that, when executed by the
  // simulator, will start the interactive debugger if it hasn't already.
  std::unordered_set<uint64_t> breakpoints_;
};


}  // namespace aarch64
}  // namespace vixl

#endif  // VIXL_INCLUDE_SIMULATOR_AARCH64

#endif  // VIXL_AARCH64_DEBUGGER_AARCH64_H_
