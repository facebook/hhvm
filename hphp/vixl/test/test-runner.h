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

#ifndef TEST_TEST_H_
#define TEST_TEST_H_

#include "utils-vixl.h"

#include "aarch64/instructions-aarch64.h"

namespace vixl {

// Each test is represented by a Test instance.
// Tests are appended to a static linked list upon creation.
class Test {
  typedef void(TestFunction)();
  typedef void(TestFunctionWithConfig)(Test* config);

 public:
  // Most tests require no per-test configuration, and so take no arguments. A
  // few tests require dynamic configuration, and are passed a `Test` object.
  template <typename Fn>
  Test(const char* name, Fn* callback)
      : name_(name), sve_vl_(aarch64::kZRegMinSize), next_(NULL) {
    set_callback(callback);
    // Append this test to the linked list.
    if (first_ == NULL) {
      VIXL_ASSERT(last_ == NULL);
      first_ = this;
    } else {
      last_->next_ = this;
    }
    last_ = this;
  }

  static Test* MakeSVETest(int vl,
                           const char* name,
                           TestFunctionWithConfig* fn) {
    // We never free this memory, but we need it to live for as long as the
    // static
    // linked list of tests, and this is the easiest way to do it.
    Test* test = new Test(name, fn);
    test->set_sve_vl_in_bits(vl);
    return test;
  }

  const char* name() { return name_; }
  void run();

  // The SVE vector length can be configured by each test, based on either
  // hardware feature detection (in the test itself) or Simulator configuration.
  int sve_vl_in_bits() const { return sve_vl_; }
  void set_sve_vl_in_bits(unsigned sve_vl) {
    VIXL_ASSERT(sve_vl >= aarch64::kZRegMinSize);
    VIXL_ASSERT(sve_vl <= aarch64::kZRegMaxSize);
    VIXL_ASSERT((sve_vl % aarch64::kZRegMinSize) == 0);
    sve_vl_ = sve_vl;
  }

  int sve_vl_in_bytes() const {
    VIXL_ASSERT((sve_vl_ % kBitsPerByte) == 0);
    return sve_vl_ / kBitsPerByte;
  }

  static Test* first() { return first_; }
  static Test* last() { return last_; }
  Test* next() { return next_; }
  static bool verbose() { return verbose_; }
  static void set_verbose(bool value) { verbose_ = value; }
  static bool trace_sim() { return trace_sim_; }
  static void set_trace_sim(bool value) { trace_sim_ = value; }
  static bool trace_reg() { return trace_reg_; }
  static void set_trace_reg(bool value) { trace_reg_ = value; }
  static bool trace_write() { return trace_write_; }
  static void set_trace_write(bool value) { trace_write_ = value; }
  static bool trace_branch() { return trace_branch_; }
  static void set_trace_branch(bool value) { trace_branch_ = value; }
  static bool disassemble() { return disassemble_; }
  static void set_disassemble(bool value) { disassemble_ = value; }
  static bool disassemble_infrastructure() {
    return disassemble_infrastructure_;
  }
  static void set_disassemble_infrastructure(bool value) {
    disassemble_infrastructure_ = value;
  }
  static bool coloured_trace() { return coloured_trace_; }
  static void set_coloured_trace(bool value) { coloured_trace_ = value; }
  static bool generate_test_trace() { return generate_test_trace_; }
  static void set_generate_test_trace(bool value) {
    generate_test_trace_ = value;
  }

 private:
  const char* name_;

  TestFunction* callback_;
  TestFunctionWithConfig* callback_with_config_;

  void set_callback(TestFunction* callback);
  void set_callback(TestFunctionWithConfig* callback);

  int sve_vl_;

  static Test* first_;
  static Test* last_;
  Test* next_;
  static bool verbose_;
  static bool trace_sim_;
  static bool trace_reg_;
  static bool trace_write_;
  static bool trace_branch_;
  static bool disassemble_;
  static bool disassemble_infrastructure_;
  static bool coloured_trace_;
  static bool generate_test_trace_;
};

// Define helper macros for test files.

// Macro to register a test. It instantiates a Test and registers its
// callback function.
#define TEST_(Name)                     \
  void Test##Name();                    \
  Test test_##Name(#Name, &Test##Name); \
  void Test##Name()
}  // namespace vixl

#endif  // TEST_TEST_H_
