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

#include "decoder-aarch64.h"

#include <string>

#include "../globals-vixl.h"
#include "../utils-vixl.h"

#include "decoder-constants-aarch64.h"

namespace vixl {
namespace aarch64 {

void Decoder::Decode(const Instruction* instr) {
  std::list<DecoderVisitor*>::iterator it;
  for (it = visitors_.begin(); it != visitors_.end(); it++) {
    VIXL_ASSERT((*it)->IsConstVisitor());
  }
  VIXL_ASSERT(compiled_decoder_root_ != NULL);
  compiled_decoder_root_->Decode(instr);
}

void Decoder::Decode(Instruction* instr) {
  compiled_decoder_root_->Decode(const_cast<const Instruction*>(instr));
}

void Decoder::AddDecodeNode(const DecodeNode& node) {
  if (decode_nodes_.count(node.GetName()) == 0) {
    decode_nodes_.insert(std::make_pair(node.GetName(), node));
  }
}

DecodeNode* Decoder::GetDecodeNode(std::string name) {
  if (decode_nodes_.count(name) != 1) {
    std::string msg = "Can't find decode node " + name + ".\n";
    VIXL_ABORT_WITH_MSG(msg.c_str());
  }
  return &decode_nodes_[name];
}

void Decoder::ConstructDecodeGraph() {
  // Add all of the decoding nodes to the Decoder.
  for (unsigned i = 0; i < ArrayLength(kDecodeMapping); i++) {
    AddDecodeNode(DecodeNode(kDecodeMapping[i], this));

    // Add a node for each instruction form named, identified by having no '_'
    // prefix on the node name.
    const DecodeMapping& map = kDecodeMapping[i];
    for (unsigned j = 0; j < map.mapping.size(); j++) {
      if ((map.mapping[j].handler != NULL) &&
          (map.mapping[j].handler[0] != '_')) {
        AddDecodeNode(DecodeNode(map.mapping[j].handler, this));
      }
    }
  }

  // Add an "unallocated" node, used when an instruction encoding is not
  // recognised by the decoding graph.
  AddDecodeNode(DecodeNode("unallocated", this));

  // Compile the graph from the root.
  compiled_decoder_root_ = GetDecodeNode("Root")->Compile(this);
}

void Decoder::AppendVisitor(DecoderVisitor* new_visitor) {
  visitors_.push_back(new_visitor);
}


void Decoder::PrependVisitor(DecoderVisitor* new_visitor) {
  visitors_.push_front(new_visitor);
}


void Decoder::InsertVisitorBefore(DecoderVisitor* new_visitor,
                                  DecoderVisitor* registered_visitor) {
  std::list<DecoderVisitor*>::iterator it;
  for (it = visitors_.begin(); it != visitors_.end(); it++) {
    if (*it == registered_visitor) {
      visitors_.insert(it, new_visitor);
      return;
    }
  }
  // We reached the end of the list. The last element must be
  // registered_visitor.
  VIXL_ASSERT(*it == registered_visitor);
  visitors_.insert(it, new_visitor);
}


void Decoder::InsertVisitorAfter(DecoderVisitor* new_visitor,
                                 DecoderVisitor* registered_visitor) {
  std::list<DecoderVisitor*>::iterator it;
  for (it = visitors_.begin(); it != visitors_.end(); it++) {
    if (*it == registered_visitor) {
      it++;
      visitors_.insert(it, new_visitor);
      return;
    }
  }
  // We reached the end of the list. The last element must be
  // registered_visitor.
  VIXL_ASSERT(*it == registered_visitor);
  visitors_.push_back(new_visitor);
}


void Decoder::RemoveVisitor(DecoderVisitor* visitor) {
  visitors_.remove(visitor);
}

void Decoder::VisitNamedInstruction(const Instruction* instr,
                                    const std::string& name) {
  std::list<DecoderVisitor*>::iterator it;
  Metadata m = {{"form", name}};
  for (it = visitors_.begin(); it != visitors_.end(); it++) {
    (*it)->Visit(&m, instr);
  }
}

// Initialise empty vectors for sampled bits and pattern table.
const std::vector<uint8_t> DecodeNode::kEmptySampledBits;
const std::vector<DecodePattern> DecodeNode::kEmptyPatternTable;

void DecodeNode::CompileNodeForBits(Decoder* decoder,
                                    std::string name,
                                    uint32_t bits) {
  DecodeNode* n = decoder->GetDecodeNode(name);
  VIXL_ASSERT(n != NULL);
  if (!n->IsCompiled()) {
    n->Compile(decoder);
  }
  VIXL_ASSERT(n->IsCompiled());
  compiled_node_->SetNodeForBits(bits, n->GetCompiledNode());
}


#define INSTANTIATE_TEMPLATE_M(M)                      \
  case 0x##M:                                          \
    bit_extract_fn = &Instruction::ExtractBits<0x##M>; \
    break;
#define INSTANTIATE_TEMPLATE_MV(M, V)                           \
  case 0x##M##V:                                                \
    bit_extract_fn = &Instruction::IsMaskedValue<0x##M, 0x##V>; \
    break;

BitExtractFn DecodeNode::GetBitExtractFunctionHelper(uint32_t x, uint32_t y) {
  // Instantiate a templated bit extraction function for every pattern we
  // might encounter. If the assertion in the default clause is reached, add a
  // new instantiation below using the information in the failure message.
  BitExtractFn bit_extract_fn = NULL;

  // The arguments x and y represent the mask and value. If y is 0, x is the
  // mask. Otherwise, y is the mask, and x is the value to compare against a
  // masked result.
  uint64_t signature = (static_cast<uint64_t>(y) << 32) | x;
  switch (signature) {
    INSTANTIATE_TEMPLATE_M(00000002);
    INSTANTIATE_TEMPLATE_M(00000010);
    INSTANTIATE_TEMPLATE_M(00000060);
    INSTANTIATE_TEMPLATE_M(000000df);
    INSTANTIATE_TEMPLATE_M(00000100);
    INSTANTIATE_TEMPLATE_M(00000200);
    INSTANTIATE_TEMPLATE_M(00000400);
    INSTANTIATE_TEMPLATE_M(00000800);
    INSTANTIATE_TEMPLATE_M(00000c00);
    INSTANTIATE_TEMPLATE_M(00000c10);
    INSTANTIATE_TEMPLATE_M(00000fc0);
    INSTANTIATE_TEMPLATE_M(00001000);
    INSTANTIATE_TEMPLATE_M(00001400);
    INSTANTIATE_TEMPLATE_M(00001800);
    INSTANTIATE_TEMPLATE_M(00001c00);
    INSTANTIATE_TEMPLATE_M(00002000);
    INSTANTIATE_TEMPLATE_M(00002010);
    INSTANTIATE_TEMPLATE_M(00002400);
    INSTANTIATE_TEMPLATE_M(00003000);
    INSTANTIATE_TEMPLATE_M(00003020);
    INSTANTIATE_TEMPLATE_M(00003400);
    INSTANTIATE_TEMPLATE_M(00003800);
    INSTANTIATE_TEMPLATE_M(00003c00);
    INSTANTIATE_TEMPLATE_M(00013000);
    INSTANTIATE_TEMPLATE_M(000203e0);
    INSTANTIATE_TEMPLATE_M(000303e0);
    INSTANTIATE_TEMPLATE_M(00040000);
    INSTANTIATE_TEMPLATE_M(00040010);
    INSTANTIATE_TEMPLATE_M(00060000);
    INSTANTIATE_TEMPLATE_M(00061000);
    INSTANTIATE_TEMPLATE_M(00070000);
    INSTANTIATE_TEMPLATE_M(000703c0);
    INSTANTIATE_TEMPLATE_M(00080000);
    INSTANTIATE_TEMPLATE_M(00090000);
    INSTANTIATE_TEMPLATE_M(000f0000);
    INSTANTIATE_TEMPLATE_M(000f0010);
    INSTANTIATE_TEMPLATE_M(00100000);
    INSTANTIATE_TEMPLATE_M(00180000);
    INSTANTIATE_TEMPLATE_M(001b1c00);
    INSTANTIATE_TEMPLATE_M(001f0000);
    INSTANTIATE_TEMPLATE_M(001f0018);
    INSTANTIATE_TEMPLATE_M(001f2000);
    INSTANTIATE_TEMPLATE_M(001f3000);
    INSTANTIATE_TEMPLATE_M(00400000);
    INSTANTIATE_TEMPLATE_M(00400018);
    INSTANTIATE_TEMPLATE_M(00400800);
    INSTANTIATE_TEMPLATE_M(00403000);
    INSTANTIATE_TEMPLATE_M(00500000);
    INSTANTIATE_TEMPLATE_M(00500800);
    INSTANTIATE_TEMPLATE_M(00583000);
    INSTANTIATE_TEMPLATE_M(005f0000);
    INSTANTIATE_TEMPLATE_M(00800000);
    INSTANTIATE_TEMPLATE_M(00800400);
    INSTANTIATE_TEMPLATE_M(00800c1d);
    INSTANTIATE_TEMPLATE_M(0080101f);
    INSTANTIATE_TEMPLATE_M(00801c00);
    INSTANTIATE_TEMPLATE_M(00803000);
    INSTANTIATE_TEMPLATE_M(00803c00);
    INSTANTIATE_TEMPLATE_M(009f0000);
    INSTANTIATE_TEMPLATE_M(009f2000);
    INSTANTIATE_TEMPLATE_M(00c00000);
    INSTANTIATE_TEMPLATE_M(00c00010);
    INSTANTIATE_TEMPLATE_M(00c0001f);
    INSTANTIATE_TEMPLATE_M(00c00200);
    INSTANTIATE_TEMPLATE_M(00c00400);
    INSTANTIATE_TEMPLATE_M(00c00c00);
    INSTANTIATE_TEMPLATE_M(00c00c19);
    INSTANTIATE_TEMPLATE_M(00c01000);
    INSTANTIATE_TEMPLATE_M(00c01400);
    INSTANTIATE_TEMPLATE_M(00c01c00);
    INSTANTIATE_TEMPLATE_M(00c02000);
    INSTANTIATE_TEMPLATE_M(00c03000);
    INSTANTIATE_TEMPLATE_M(00c03c00);
    INSTANTIATE_TEMPLATE_M(00c70000);
    INSTANTIATE_TEMPLATE_M(00c83000);
    INSTANTIATE_TEMPLATE_M(00d00200);
    INSTANTIATE_TEMPLATE_M(00d80800);
    INSTANTIATE_TEMPLATE_M(00d81800);
    INSTANTIATE_TEMPLATE_M(00d81c00);
    INSTANTIATE_TEMPLATE_M(00d82800);
    INSTANTIATE_TEMPLATE_M(00d82c00);
    INSTANTIATE_TEMPLATE_M(00d92400);
    INSTANTIATE_TEMPLATE_M(00d93000);
    INSTANTIATE_TEMPLATE_M(00db0000);
    INSTANTIATE_TEMPLATE_M(00db2000);
    INSTANTIATE_TEMPLATE_M(00dc0000);
    INSTANTIATE_TEMPLATE_M(00dc2000);
    INSTANTIATE_TEMPLATE_M(00df0000);
    INSTANTIATE_TEMPLATE_M(40000000);
    INSTANTIATE_TEMPLATE_M(40000010);
    INSTANTIATE_TEMPLATE_M(40000c00);
    INSTANTIATE_TEMPLATE_M(40002000);
    INSTANTIATE_TEMPLATE_M(40002010);
    INSTANTIATE_TEMPLATE_M(40003000);
    INSTANTIATE_TEMPLATE_M(40003c00);
    INSTANTIATE_TEMPLATE_M(401f2000);
    INSTANTIATE_TEMPLATE_M(40400800);
    INSTANTIATE_TEMPLATE_M(40400c00);
    INSTANTIATE_TEMPLATE_M(40403c00);
    INSTANTIATE_TEMPLATE_M(405f0000);
    INSTANTIATE_TEMPLATE_M(40800000);
    INSTANTIATE_TEMPLATE_M(40800c00);
    INSTANTIATE_TEMPLATE_M(40802000);
    INSTANTIATE_TEMPLATE_M(40802010);
    INSTANTIATE_TEMPLATE_M(40803400);
    INSTANTIATE_TEMPLATE_M(40803c00);
    INSTANTIATE_TEMPLATE_M(40c00000);
    INSTANTIATE_TEMPLATE_M(40c00400);
    INSTANTIATE_TEMPLATE_M(40c00800);
    INSTANTIATE_TEMPLATE_M(40c00c00);
    INSTANTIATE_TEMPLATE_M(40c00c10);
    INSTANTIATE_TEMPLATE_M(40c02000);
    INSTANTIATE_TEMPLATE_M(40c02010);
    INSTANTIATE_TEMPLATE_M(40c02c00);
    INSTANTIATE_TEMPLATE_M(40c03c00);
    INSTANTIATE_TEMPLATE_M(40c80000);
    INSTANTIATE_TEMPLATE_M(40c90000);
    INSTANTIATE_TEMPLATE_M(40cf0000);
    INSTANTIATE_TEMPLATE_M(40d02000);
    INSTANTIATE_TEMPLATE_M(40d02010);
    INSTANTIATE_TEMPLATE_M(40d80000);
    INSTANTIATE_TEMPLATE_M(40d81800);
    INSTANTIATE_TEMPLATE_M(40dc0000);
    INSTANTIATE_TEMPLATE_M(bf20c000);
    INSTANTIATE_TEMPLATE_MV(00000006, 00000000);
    INSTANTIATE_TEMPLATE_MV(00000006, 00000006);
    INSTANTIATE_TEMPLATE_MV(00000007, 00000000);
    INSTANTIATE_TEMPLATE_MV(0000001f, 0000001f);
    INSTANTIATE_TEMPLATE_MV(00000210, 00000000);
    INSTANTIATE_TEMPLATE_MV(000003e0, 00000000);
    INSTANTIATE_TEMPLATE_MV(000003e0, 000003e0);
    INSTANTIATE_TEMPLATE_MV(000003e2, 000003e0);
    INSTANTIATE_TEMPLATE_MV(000003e6, 000003e0);
    INSTANTIATE_TEMPLATE_MV(000003e6, 000003e6);
    INSTANTIATE_TEMPLATE_MV(00000c00, 00000000);
    INSTANTIATE_TEMPLATE_MV(00000fc0, 00000000);
    INSTANTIATE_TEMPLATE_MV(000013e0, 00001000);
    INSTANTIATE_TEMPLATE_MV(00001c00, 00000000);
    INSTANTIATE_TEMPLATE_MV(00002400, 00000000);
    INSTANTIATE_TEMPLATE_MV(00003000, 00000000);
    INSTANTIATE_TEMPLATE_MV(00003000, 00001000);
    INSTANTIATE_TEMPLATE_MV(00003000, 00002000);
    INSTANTIATE_TEMPLATE_MV(00003000, 00003000);
    INSTANTIATE_TEMPLATE_MV(00003010, 00000000);
    INSTANTIATE_TEMPLATE_MV(00003c00, 00003c00);
    INSTANTIATE_TEMPLATE_MV(00040010, 00000000);
    INSTANTIATE_TEMPLATE_MV(00060000, 00000000);
    INSTANTIATE_TEMPLATE_MV(00061000, 00000000);
    INSTANTIATE_TEMPLATE_MV(00070000, 00030000);
    INSTANTIATE_TEMPLATE_MV(00073ee0, 00033060);
    INSTANTIATE_TEMPLATE_MV(00073f9f, 0000001f);
    INSTANTIATE_TEMPLATE_MV(000f0000, 00000000);
    INSTANTIATE_TEMPLATE_MV(000f0010, 00000000);
    INSTANTIATE_TEMPLATE_MV(00100200, 00000000);
    INSTANTIATE_TEMPLATE_MV(00100210, 00000000);
    INSTANTIATE_TEMPLATE_MV(00160000, 00000000);
    INSTANTIATE_TEMPLATE_MV(00170000, 00000000);
    INSTANTIATE_TEMPLATE_MV(001c0000, 00000000);
    INSTANTIATE_TEMPLATE_MV(001d0000, 00000000);
    INSTANTIATE_TEMPLATE_MV(001e0000, 00000000);
    INSTANTIATE_TEMPLATE_MV(001f0000, 00000000);
    INSTANTIATE_TEMPLATE_MV(001f0000, 00010000);
    INSTANTIATE_TEMPLATE_MV(001f0000, 00100000);
    INSTANTIATE_TEMPLATE_MV(001f0000, 001f0000);
    INSTANTIATE_TEMPLATE_MV(001f3000, 00000000);
    INSTANTIATE_TEMPLATE_MV(001f3000, 00001000);
    INSTANTIATE_TEMPLATE_MV(001f3000, 001f0000);
    INSTANTIATE_TEMPLATE_MV(001f300f, 0000000d);
    INSTANTIATE_TEMPLATE_MV(001f301f, 0000000d);
    INSTANTIATE_TEMPLATE_MV(001f33e0, 000103e0);
    INSTANTIATE_TEMPLATE_MV(001f3800, 00000000);
    INSTANTIATE_TEMPLATE_MV(00401000, 00400000);
    INSTANTIATE_TEMPLATE_MV(005f3000, 001f0000);
    INSTANTIATE_TEMPLATE_MV(005f3000, 001f1000);
    INSTANTIATE_TEMPLATE_MV(00800010, 00000000);
    INSTANTIATE_TEMPLATE_MV(00800400, 00000000);
    INSTANTIATE_TEMPLATE_MV(00800410, 00000000);
    INSTANTIATE_TEMPLATE_MV(00803000, 00002000);
    INSTANTIATE_TEMPLATE_MV(00870000, 00000000);
    INSTANTIATE_TEMPLATE_MV(009f0000, 00010000);
    INSTANTIATE_TEMPLATE_MV(00c00000, 00000000);
    INSTANTIATE_TEMPLATE_MV(00c00000, 00400000);
    INSTANTIATE_TEMPLATE_MV(00c0001f, 00000000);
    INSTANTIATE_TEMPLATE_MV(00c001ff, 00000000);
    INSTANTIATE_TEMPLATE_MV(00c00200, 00400000);
    INSTANTIATE_TEMPLATE_MV(00c0020f, 00400000);
    INSTANTIATE_TEMPLATE_MV(00c003e0, 00000000);
    INSTANTIATE_TEMPLATE_MV(00c00800, 00000000);
    INSTANTIATE_TEMPLATE_MV(00d80800, 00000000);
    INSTANTIATE_TEMPLATE_MV(00df0000, 00000000);
    INSTANTIATE_TEMPLATE_MV(00df3800, 001f0800);
    INSTANTIATE_TEMPLATE_MV(40002000, 40000000);
    INSTANTIATE_TEMPLATE_MV(40003c00, 00000000);
    INSTANTIATE_TEMPLATE_MV(40040000, 00000000);
    INSTANTIATE_TEMPLATE_MV(401f2000, 401f0000);
    INSTANTIATE_TEMPLATE_MV(40800c00, 40000400);
    INSTANTIATE_TEMPLATE_MV(40c00000, 00000000);
    INSTANTIATE_TEMPLATE_MV(40c00000, 00400000);
    INSTANTIATE_TEMPLATE_MV(40c00000, 40000000);
    INSTANTIATE_TEMPLATE_MV(40c00000, 40800000);
    INSTANTIATE_TEMPLATE_MV(40df0000, 00000000);
    default: {
      static bool printed_preamble = false;
      if (!printed_preamble) {
        printf("One or more missing template instantiations.\n");
        printf(
            "Add the following to either GetBitExtractFunction() "
            "implementations\n");
        printf("in %s near line %d:\n", __FILE__, __LINE__);
        printed_preamble = true;
      }

      if (y == 0) {
        printf("  INSTANTIATE_TEMPLATE_M(%08x);\n", x);
        bit_extract_fn = &Instruction::ExtractBitsAbsent;
      } else {
        printf("  INSTANTIATE_TEMPLATE_MV(%08x, %08x);\n", y, x);
        bit_extract_fn = &Instruction::IsMaskedValueAbsent;
      }
    }
  }
  return bit_extract_fn;
}

#undef INSTANTIATE_TEMPLATE_M
#undef INSTANTIATE_TEMPLATE_MV

bool DecodeNode::TryCompileOptimisedDecodeTable(Decoder* decoder) {
  // EitherOr optimisation: if there are only one or two patterns in the table,
  // try to optimise the node to exploit that.
  size_t table_size = pattern_table_.size();
  if ((table_size <= 2) && (GetSampledBitsCount() > 1)) {
    // TODO: support 'x' in this optimisation by dropping the sampled bit
    // positions before making the mask/value.
    if (!PatternContainsSymbol(pattern_table_[0].pattern,
                               PatternSymbol::kSymbolX) &&
        (table_size == 1)) {
      // A pattern table consisting of a fixed pattern with no x's, and an
      // "otherwise" or absent case. Optimise this into an instruction mask and
      // value test.
      uint32_t single_decode_mask = 0;
      uint32_t single_decode_value = 0;
      const std::vector<uint8_t>& bits = GetSampledBits();

      // Construct the instruction mask and value from the pattern.
      VIXL_ASSERT(bits.size() == GetPatternLength(pattern_table_[0].pattern));
      for (size_t i = 0; i < bits.size(); i++) {
        single_decode_mask |= 1U << bits[i];
        if (GetSymbolAt(pattern_table_[0].pattern, i) ==
            PatternSymbol::kSymbol1) {
          single_decode_value |= 1U << bits[i];
        }
      }
      BitExtractFn bit_extract_fn =
          GetBitExtractFunction(single_decode_mask, single_decode_value);

      // Create a compiled node that contains a two entry table for the
      // either/or cases.
      CreateCompiledNode(bit_extract_fn, 2);

      // Set DecodeNode for when the instruction after masking doesn't match the
      // value.
      CompileNodeForBits(decoder, "unallocated", 0);

      // Set DecodeNode for when it does match.
      CompileNodeForBits(decoder, pattern_table_[0].handler, 1);

      return true;
    }
  }
  return false;
}

CompiledDecodeNode* DecodeNode::Compile(Decoder* decoder) {
  if (IsLeafNode()) {
    // A leaf node is a simple wrapper around a visitor function, with no
    // instruction decoding to do.
    CreateVisitorNode();
  } else if (!TryCompileOptimisedDecodeTable(decoder)) {
    // The "otherwise" node is the default next node if no pattern matches.
    std::string otherwise = "unallocated";

    // For each pattern in pattern_table_, create an entry in matches that
    // has a corresponding mask and value for the pattern.
    std::vector<MaskValuePair> matches;
    for (size_t i = 0; i < pattern_table_.size(); i++) {
      matches.push_back(GenerateMaskValuePair(
          GenerateOrderedPattern(pattern_table_[i].pattern)));
    }

    BitExtractFn bit_extract_fn =
        GetBitExtractFunction(GenerateSampledBitsMask());

    // Create a compiled node that contains a table with an entry for every bit
    // pattern.
    CreateCompiledNode(bit_extract_fn,
                       static_cast<size_t>(1) << GetSampledBitsCount());
    VIXL_ASSERT(compiled_node_ != NULL);

    // When we find a pattern matches the representation, set the node's decode
    // function for that representation to the corresponding function.
    for (uint32_t bits = 0; bits < (1U << GetSampledBitsCount()); bits++) {
      for (size_t i = 0; i < matches.size(); i++) {
        if ((bits & matches[i].first) == matches[i].second) {
          // Only one instruction class should match for each value of bits, so
          // if we get here, the node pointed to should still be unallocated.
          VIXL_ASSERT(compiled_node_->GetNodeForBits(bits) == NULL);
          CompileNodeForBits(decoder, pattern_table_[i].handler, bits);
          break;
        }
      }

      // If the decode_table_ entry for these bits is still NULL, the
      // instruction must be handled by the "otherwise" case, which by default
      // is the Unallocated visitor.
      if (compiled_node_->GetNodeForBits(bits) == NULL) {
        CompileNodeForBits(decoder, otherwise, bits);
      }
    }
  }

  VIXL_ASSERT(compiled_node_ != NULL);
  return compiled_node_;
}

void CompiledDecodeNode::Decode(const Instruction* instr) const {
  if (IsLeafNode()) {
    // If this node is a leaf, call the registered visitor function.
    VIXL_ASSERT(decoder_ != NULL);
    decoder_->VisitNamedInstruction(instr, instruction_name_);
  } else {
    // Otherwise, using the sampled bit extractor for this node, look up the
    // next node in the decode tree, and call its Decode method.
    VIXL_ASSERT(bit_extract_fn_ != NULL);
    VIXL_ASSERT((instr->*bit_extract_fn_)() < decode_table_size_);
    VIXL_ASSERT(decode_table_[(instr->*bit_extract_fn_)()] != NULL);
    decode_table_[(instr->*bit_extract_fn_)()]->Decode(instr);
  }
}

DecodeNode::MaskValuePair DecodeNode::GenerateMaskValuePair(
    uint32_t pattern) const {
  uint32_t mask = 0, value = 0;
  for (size_t i = 0; i < GetPatternLength(pattern); i++) {
    PatternSymbol sym = GetSymbolAt(pattern, i);
    mask = (mask << 1) | ((sym == PatternSymbol::kSymbolX) ? 0 : 1);
    value = (value << 1) | (static_cast<uint32_t>(sym) & 1);
  }
  return std::make_pair(mask, value);
}

uint32_t DecodeNode::GenerateOrderedPattern(uint32_t pattern) const {
  const std::vector<uint8_t>& sampled_bits = GetSampledBits();
  uint64_t temp = 0xffffffffffffffff;

  // Place symbols into the field of set bits. Symbols are two bits wide and
  // take values 0, 1 or 2, so 3 will represent "no symbol".
  for (size_t i = 0; i < sampled_bits.size(); i++) {
    int shift = sampled_bits[i] * 2;
    temp ^= static_cast<uint64_t>(kEndOfPattern) << shift;
    temp |= static_cast<uint64_t>(GetSymbolAt(pattern, i)) << shift;
  }

  // Iterate over temp and extract new pattern ordered by sample position.
  uint32_t result = kEndOfPattern;  // End of pattern marker.

  // Iterate over the pattern one symbol (two bits) at a time.
  for (int i = 62; i >= 0; i -= 2) {
    uint32_t sym = (temp >> i) & kPatternSymbolMask;

    // If this is a valid symbol, shift into the result.
    if (sym != kEndOfPattern) {
      result = (result << 2) | sym;
    }
  }

  // The length of the ordered pattern must be the same as the input pattern,
  // and the number of sampled bits.
  VIXL_ASSERT(GetPatternLength(result) == GetPatternLength(pattern));
  VIXL_ASSERT(GetPatternLength(result) == sampled_bits.size());

  return result;
}

uint32_t DecodeNode::GenerateSampledBitsMask() const {
  uint32_t mask = 0;
  for (int bit : GetSampledBits()) {
    mask |= 1 << bit;
  }
  return mask;
}

}  // namespace aarch64
}  // namespace vixl
