// Copyright 2014, VIXL authors
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

#include "test-runner.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// Initialize the list as empty.
vixl::Test* vixl::Test::first_ = NULL;
vixl::Test* vixl::Test::last_ = NULL;

bool vixl::Test::verbose_ = false;

// No tracing to start with.
bool vixl::Test::trace_sim_ = false;
bool vixl::Test::trace_reg_ = false;
bool vixl::Test::trace_write_ = false;
bool vixl::Test::trace_branch_ = false;

// Do not disassemble by default.
bool vixl::Test::disassemble_ = false;
bool vixl::Test::disassemble_infrastructure_ = false;

// No colour highlight by default.
bool vixl::Test::coloured_trace_ = false;

// Don't generate traces by default.
bool vixl::Test::generate_test_trace_ = false;

// Look for 'search' in the arguments.
static bool IsInArgs(const char* search, int argc, char* argv[]) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(search, argv[i]) == 0) {
      return true;
    }
  }
  return false;
}


static bool IsOption(const char* arg) {
  // Any argument like "--option" is an option.
  return ((arg[0] == '-') && (arg[1] == '-'));
}


static void NormalizeOption(char* arg) {
  // Squash all '_' characters in options. This allows --trace_sim and
  // --trace-sim to be handled in the same way, for example.
  VIXL_ASSERT(IsOption(arg));
  for (char* c = arg; *c != '\0'; c++) {
    if (*c == '_') {
      *c = '-';
    }
  }
}


static void PrintHelpMessage() {
  printf(
      "Usage:  ./test [options] [test names]\n"
      "Run all tests specified on the command line.\n"
      "--help                   Print this help message.\n"
      "--list                   List all available tests.\n"
      "--run_all                Run all available tests.\n"
      "--verbose                Print verbose output when available.\n"
      "--trace_all              "
      "Enable all trace options, plus --coloured_trace.\n"
      "--trace_sim              "
      "Generate a trace of simulated instructions, as\n"
      "                         well as disassembly from the DISASM tests.\n"
      "--trace_reg              Generate a trace of simulated registers.\n"
      "--trace_write            Generate a trace of memory writes.\n"
      "--trace_branch           Generate a trace of branches taken.\n"
      "--disassemble            Disassemble and print generated instructions.\n"
      "--disassemble-test-code  "
      "As above, but don't disassemble infrastructure code.\n"
      "--coloured_trace         Generate coloured trace.\n"
      "--generate_test_trace    "
      "Print result traces for SIM_* and TRACE_* tests.\n");
}

int main(int argc, char* argv[]) {
  // Parse the arguments. Option flags must appear first, followed by an
  // optional list of tests to run.

  int test_specifiers = 0;
  for (int i = 1; i < argc; i++) {
    if (IsOption(argv[i])) {
      NormalizeOption(argv[i]);
    } else {
      // Anything that isn't an option is a test specifier.
      test_specifiers++;
    }
  }

  // Options controlling test conditions and debug output.

  if (IsInArgs("--trace-all", argc, argv)) {
    vixl::Test::set_trace_reg(true);
    vixl::Test::set_trace_write(true);
    vixl::Test::set_trace_branch(true);
    vixl::Test::set_trace_sim(true);
    vixl::Test::set_coloured_trace(true);
  }

  if (IsInArgs("--coloured-trace", argc, argv)) {
    vixl::Test::set_coloured_trace(true);
  }

  if (IsInArgs("--verbose", argc, argv)) {
    vixl::Test::set_verbose(true);
  }

  if (IsInArgs("--trace-write", argc, argv)) {
    vixl::Test::set_trace_write(true);
  }

  if (IsInArgs("--trace-branch", argc, argv)) {
    vixl::Test::set_trace_branch(true);
  }

  if (IsInArgs("--trace-reg", argc, argv)) {
    vixl::Test::set_trace_reg(true);
  }

  if (IsInArgs("--trace-sim", argc, argv)) {
    vixl::Test::set_trace_sim(true);
  }

  if (IsInArgs("--disassemble", argc, argv)) {
    vixl::Test::set_disassemble(true);
    vixl::Test::set_disassemble_infrastructure(true);
  } else if (IsInArgs("--disassemble-test-code", argc, argv)) {
    vixl::Test::set_disassemble(true);
    vixl::Test::set_disassemble_infrastructure(false);
  }

  if (IsInArgs("--generate-test-trace", argc, argv)) {
    vixl::Test::set_generate_test_trace(true);
  }

  // Basic (mutually-exclusive) operations.

  if (IsInArgs("--help", argc, argv)) {
    PrintHelpMessage();

  } else if (IsInArgs("--list", argc, argv)) {
    // List all registered tests, then exit.
    for (vixl::Test* c = vixl::Test::first(); c != NULL; c = c->next()) {
      printf("%s\n", c->name());
    }

  } else if (IsInArgs("--run-all", argc, argv)) {
    // Run all registered tests.
    for (vixl::Test* c = vixl::Test::first(); c != NULL; c = c->next()) {
      printf("Running %s\n", c->name());
      c->run();
    }

  } else {
    // Run the specified tests.
    if (test_specifiers == 0) {
      printf("No tests specified.\n");
      PrintHelpMessage();
      return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++) {
      if (!IsOption(argv[i])) {
        vixl::Test* c;
        for (c = vixl::Test::first(); c != NULL; c = c->next()) {
          if (strcmp(c->name(), argv[i]) == 0) {
            c->run();
            break;
          }
        }
        // Fail if we have not found a matching test to run.
        if (c == NULL) {
          printf("Test '%s' does not exist. Aborting...\n", argv[i]);
          abort();
        }
      }
    }
  }

  return EXIT_SUCCESS;
}

void vixl::Test::set_callback(TestFunction* callback) {
  callback_ = callback;
  callback_with_config_ = NULL;
}

void vixl::Test::set_callback(TestFunctionWithConfig* callback) {
  callback_ = NULL;
  callback_with_config_ = callback;
}

void vixl::Test::run() {
  if (callback_ == NULL) {
    VIXL_ASSERT(callback_with_config_ != NULL);
    callback_with_config_(this);
  } else {
    VIXL_ASSERT(callback_with_config_ == NULL);
    callback_();
  }
}
