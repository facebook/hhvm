// Copyright 2013, ARM Limited
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

#include "hphp/vixl/a64/instrument-a64.h"

namespace vixl {

Counter::Counter(const char* name, CounterType type)
    : count_(0), enabled_(false), type_(type) {
  assert(name != nullptr);
  strncpy(name_, name, kCounterNameMaxLength);
}


void Counter::Enable() {
  enabled_ = true;
}


void Counter::Disable() {
  enabled_ = false;
}


bool Counter::IsEnabled() {
  return enabled_;
}


void Counter::Increment() {
  if (enabled_) {
    count_++;
  }
}


uint64_t Counter::count() {
  uint64_t result = count_;
  if (type_ == Gauge) {
    // If the counter is a Gauge, reset the count after reading.
    count_ = 0;
  }
  return result;
}


const char* Counter::name() {
  return name_;
}


CounterType Counter::type() {
  return type_;
}


typedef struct {
  const char* name;
  CounterType type;
} CounterDescriptor;


static const CounterDescriptor kCounterList[] = {
  {"Instruction", Cumulative},

  {"Move Immediate", Gauge},
  {"Add/Sub DP", Gauge},
  {"Logical DP", Gauge},
  {"Other Int DP", Gauge},
  {"FP DP", Gauge},

  {"Conditional Select", Gauge},
  {"Conditional Compare", Gauge},

  {"Unconditional Branch", Gauge},
  {"Compare and Branch", Gauge},
  {"Test and Branch", Gauge},
  {"Conditional Branch", Gauge},

  {"Load Integer", Gauge},
  {"Load FP", Gauge},
  {"Load Pair", Gauge},
  {"Load Literal", Gauge},

  {"Store Integer", Gauge},
  {"Store FP", Gauge},
  {"Store Pair", Gauge},

  {"PC Addressing", Gauge},
  {"Other", Gauge},
};


Instrument::Instrument(const char* datafile, uint64_t sample_period)
    : output_stream_(stdout), sample_period_(sample_period) {

  // Set up the output stream. If datafile is non-nullptr, use that file. If it
  // can't be opened, or datafile is nullptr, use stdout.
  if (datafile != nullptr) {
    output_stream_ = fopen(datafile, "w");
    if (output_stream_ == nullptr) {
      printf("Can't open output file %s. Using stdout.\n", datafile);
      output_stream_ = stdout;
    }
  }

  static const int num_counters =
    sizeof(kCounterList) / sizeof(CounterDescriptor);

  // Dump an instrumentation description comment at the top of the file.
  fprintf(output_stream_, "# counters=%d\n", num_counters);
  fprintf(output_stream_, "# sample_period=%" PRIu64 "\n", sample_period_);

  // Construct Counter objects from counter description array.
  for (int i = 0; i < num_counters; i++) {
    Counter* counter = new Counter(kCounterList[i].name, kCounterList[i].type);
    counters_.push_back(counter);
  }

  DumpCounterNames();
}


Instrument::~Instrument() {
  // Dump any remaining instruction data to the output file.
  DumpCounters();

  // Free all the counter objects.
  std::list<Counter*>::iterator it;
  for (it = counters_.begin(); it != counters_.end(); it++) {
    delete *it;
  }

  if (output_stream_ != stdout) {
    fclose(output_stream_);
  }
}


void Instrument::Update() {
  // Increment the instruction counter, and dump all counters if a sample period
  // has elapsed.
  static Counter* counter = GetCounter("Instruction");
  assert(counter->type() == Cumulative);
  counter->Increment();

  if (counter->IsEnabled() && (counter->count() % sample_period_) == 0) {
    DumpCounters();
  }
}


void Instrument::DumpCounters() {
  // Iterate through the counter objects, dumping their values to the output
  // stream.
  std::list<Counter*>::const_iterator it;
  for (it = counters_.begin(); it != counters_.end(); it++) {
    fprintf(output_stream_, "%" PRIu64 ",", (*it)->count());
  }
  fprintf(output_stream_, "\n");
  fflush(output_stream_);
}


void Instrument::DumpCounterNames() {
  // Iterate through the counter objects, dumping the counter names to the
  // output stream.
  std::list<Counter*>::const_iterator it;
  for (it = counters_.begin(); it != counters_.end(); it++) {
    fprintf(output_stream_, "%s,", (*it)->name());
  }
  fprintf(output_stream_, "\n");
  fflush(output_stream_);
}


void Instrument::HandleInstrumentationEvent(unsigned event) {
  switch (event) {
    case InstrumentStateEnable: Enable(); break;
    case InstrumentStateDisable: Disable(); break;
    default: DumpEventMarker(event);
  }
}


void Instrument::DumpEventMarker(unsigned marker) {
  // Dumpan event marker to the output stream as a specially formatted comment
  // line.
  static Counter* counter = GetCounter("Instruction");

  fprintf(output_stream_, "# %c%c @ %" PRId64 "\n", marker & 0xff,
          (marker >> 8) & 0xff, counter->count());
}


Counter* Instrument::GetCounter(const char* name) {
  // Get a Counter object by name from the counter list.
  std::list<Counter*>::const_iterator it;
  for (it = counters_.begin(); it != counters_.end(); it++) {
    if (strcmp((*it)->name(), name) == 0) {
      return *it;
    }
  }

  // A Counter by that name does not exist: print an error message to stderr
  // and the output file, and exit.
  static const char* error_message =
    "# Error: Unknown counter \"%s\". Exiting.\n";
  fprintf(stderr, error_message, name);
  fprintf(output_stream_, error_message, name);
  exit(1);
}


void Instrument::Enable() {
  std::list<Counter*>::iterator it;
  for (it = counters_.begin(); it != counters_.end(); it++) {
    (*it)->Enable();
  }
}


void Instrument::Disable() {
  std::list<Counter*>::iterator it;
  for (it = counters_.begin(); it != counters_.end(); it++) {
    (*it)->Disable();
  }
}


void Instrument::VisitPCRelAddressing(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("PC Addressing");
  counter->Increment();
}


void Instrument::VisitAddSubImmediate(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Add/Sub DP");
  counter->Increment();
}


void Instrument::VisitLogicalImmediate(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Logical DP");
  counter->Increment();
}


void Instrument::VisitMoveWideImmediate(Instruction* instr) {
  Update();
  static Counter* counter = GetCounter("Move Immediate");

  if (instr->IsMovn() && (instr->Rd() == kZeroRegCode)) {
    unsigned imm = instr->ImmMoveWide();
    HandleInstrumentationEvent(imm);
  } else {
    counter->Increment();
  }
}


void Instrument::VisitBitfield(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other Int DP");
  counter->Increment();
}


void Instrument::VisitExtract(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other Int DP");
  counter->Increment();
}


void Instrument::VisitUnconditionalBranch(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Unconditional Branch");
  counter->Increment();
}


void Instrument::VisitUnconditionalBranchToRegister(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Unconditional Branch");
  counter->Increment();
}


void Instrument::VisitCompareBranch(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Compare and Branch");
  counter->Increment();
}


void Instrument::VisitTestBranch(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Test and Branch");
  counter->Increment();
}


void Instrument::VisitConditionalBranch(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Conditional Branch");
  counter->Increment();
}


void Instrument::VisitSystem(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other");
  counter->Increment();
}


void Instrument::VisitException(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other");
  counter->Increment();
}


void Instrument::InstrumentLoadStorePair(Instruction* instr) {
  static Counter* load_pair_counter = GetCounter("Load Pair");
  static Counter* store_pair_counter = GetCounter("Store Pair");

  if (instr->Mask(LoadStorePairLBit) != 0) {
    load_pair_counter->Increment();
  } else {
    store_pair_counter->Increment();
  }
}


void Instrument::VisitLoadStorePairPostIndex(Instruction* instr) {
  USE(instr);
  Update();
  InstrumentLoadStorePair(instr);
}


void Instrument::VisitLoadStorePairOffset(Instruction* instr) {
  USE(instr);
  Update();
  InstrumentLoadStorePair(instr);
}


void Instrument::VisitLoadStorePairPreIndex(Instruction* instr) {
  USE(instr);
  Update();
  InstrumentLoadStorePair(instr);
}


void Instrument::VisitLoadStorePairNonTemporal(Instruction* instr) {
  USE(instr);
  Update();
  InstrumentLoadStorePair(instr);
}


void Instrument::VisitLoadLiteral(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Load Literal");
  counter->Increment();
}


void Instrument::InstrumentLoadStore(Instruction* instr) {
  static Counter* load_int_counter = GetCounter("Load Integer");
  static Counter* store_int_counter = GetCounter("Store Integer");
  static Counter* load_fp_counter = GetCounter("Load FP");
  static Counter* store_fp_counter = GetCounter("Store FP");

  switch (instr->Mask(LoadStoreOpMask)) {
    case STRB_w:    // Fall through.
    case STRH_w:    // Fall through.
    case STR_w:     // Fall through.
    case STR_x:     store_int_counter->Increment(); break;
    case STR_s:     // Fall through.
    case STR_d:     store_fp_counter->Increment(); break;
    case LDRB_w:    // Fall through.
    case LDRH_w:    // Fall through.
    case LDR_w:     // Fall through.
    case LDR_x:     // Fall through.
    case LDRSB_x:   // Fall through.
    case LDRSH_x:   // Fall through.
    case LDRSW_x:   // Fall through.
    case LDRSB_w:   // Fall through.
    case LDRSH_w:   load_int_counter->Increment(); break;
    case LDR_s:     // Fall through.
    case LDR_d:     load_fp_counter->Increment(); break;
  }
}


void Instrument::VisitLoadStoreUnscaledOffset(Instruction* instr) {
  Update();
  InstrumentLoadStore(instr);
}


void Instrument::VisitLoadStorePostIndex(Instruction* instr) {
  USE(instr);
  Update();
  InstrumentLoadStore(instr);
}


void Instrument::VisitLoadStorePreIndex(Instruction* instr) {
  Update();
  InstrumentLoadStore(instr);
}


void Instrument::VisitLoadStoreRegisterOffset(Instruction* instr) {
  Update();
  InstrumentLoadStore(instr);
}


void Instrument::VisitLoadStoreUnsignedOffset(Instruction* instr) {
  Update();
  InstrumentLoadStore(instr);
}


void Instrument::VisitLogicalShifted(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Logical DP");
  counter->Increment();
}


void Instrument::VisitAddSubShifted(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Add/Sub DP");
  counter->Increment();
}


void Instrument::VisitAddSubExtended(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Add/Sub DP");
  counter->Increment();
}


void Instrument::VisitAddSubWithCarry(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Add/Sub DP");
  counter->Increment();
}


void Instrument::VisitConditionalCompareRegister(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Conditional Compare");
  counter->Increment();
}


void Instrument::VisitConditionalCompareImmediate(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Conditional Compare");
  counter->Increment();
}


void Instrument::VisitConditionalSelect(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Conditional Select");
  counter->Increment();
}


void Instrument::VisitDataProcessing1Source(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other Int DP");
  counter->Increment();
}


void Instrument::VisitDataProcessing2Source(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other Int DP");
  counter->Increment();
}


void Instrument::VisitDataProcessing3Source(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other Int DP");
  counter->Increment();
}


void Instrument::VisitFPCompare(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("FP DP");
  counter->Increment();
}


void Instrument::VisitFPConditionalCompare(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Conditional Compare");
  counter->Increment();
}


void Instrument::VisitFPConditionalSelect(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Conditional Select");
  counter->Increment();
}


void Instrument::VisitFPImmediate(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("FP DP");
  counter->Increment();
}


void Instrument::VisitFPDataProcessing1Source(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("FP DP");
  counter->Increment();
}


void Instrument::VisitFPDataProcessing2Source(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("FP DP");
  counter->Increment();
}


void Instrument::VisitFPDataProcessing3Source(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("FP DP");
  counter->Increment();
}


void Instrument::VisitFPIntegerConvert(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("FP DP");
  counter->Increment();
}


void Instrument::VisitFPFixedPointConvert(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("FP DP");
  counter->Increment();
}


void Instrument::VisitUnallocated(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other");
  counter->Increment();
}


void Instrument::VisitUnimplemented(Instruction* instr) {
  USE(instr);
  Update();
  static Counter* counter = GetCounter("Other");
  counter->Increment();
}


}  // namespace v8::internal
