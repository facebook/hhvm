/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_IR_INSTRUCTION_H_
#define incl_HPHP_IR_INSTRUCTION_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"

#include <boost/intrusive/list.hpp>
#include <folly/Range.h>

#include <cstdint>
#include <string>

namespace HPHP { namespace jit {
//////////////////////////////////////////////////////////////////////

struct Block;
struct Edge;
struct IRUnit;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * An HHIR instruction.
 *
 * IRInstructions must be arena-allocatable.  (Destructors are not called when
 * they come from IRUnit.)
 */
struct IRInstruction {
  enum Id { kTransient = 0xffffffff };

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Create an IRInstruction for the opcode `op'.
   *
   * IRInstruction creation is usually done through IRUnit or IRBuilder rather
   * than directly via the constructor.
   */
  explicit IRInstruction(Opcode op,
                         BCMarker marker,
                         Edge* edges = nullptr,
                         uint32_t numSrcs = 0,
                         SSATmp** srcs = nullptr);

  /*
   * Construct an IRInstruction as a deep copy of `inst', using `arena' to
   * allocate memory for its srcs/dsts.
   */
  explicit IRInstruction(Arena& arena, const IRInstruction* inst, Id id);

  /*
   * Stringify the instruction.
   */
  std::string toString() const;

  IRInstruction(const IRInstruction&) = delete;
  IRInstruction& operator=(const IRInstruction&) = delete;


  /////////////////////////////////////////////////////////////////////////////
  // Replacement.

  /*
   * Turn this instruction into the target instruction, without changing
   * stable fields (id, current block, list fields).
   *
   * The existing destination SSATmp(s) will continue to think they came from
   * this instruction, and the instruction's marker will not change.
   *
   * The target instruction may be transient---we'll clone anything we need to
   * keep, using IRUnit for any needed memory.
   *
   * Pre: other->isTransient() || numDsts() == other->numDsts()
   */
  void become(IRUnit&, const IRInstruction* other);

  /*
   * Replace the instruction in-place with a Nop.
   *
   * This is less general than become(), but it is fairly common, and doesn't
   * require access to an IRUnit.
   */
  void convertToNop();


  /////////////////////////////////////////////////////////////////////////////
  // OpcodeFlag helpers.                                                [const]

  /*
   * Helper accessors for the OpcodeFlag bits for this instruction.
   */
  bool hasDst() const;
  bool naryDst() const;
  bool consumesReferences() const;
  bool killsSources() const;
  bool mayRaiseError() const;
  bool isTerminal() const;
  bool hasEdges() const;
  bool isPassthrough() const;

  /*
   * Whether the src numbered srcNo consumes a reference, or the dest produces
   * a reference.
   */
  bool consumesReference(int srcNo) const;
  bool producesReference() const;

  /*
   * Get the src that is passed through.
   *
   * Currently, this is always src(0).
   */
  SSATmp* getPassthroughValue() const;


  /////////////////////////////////////////////////////////////////////////////
  // Opcode, marker, and type.

  /*
   * The ID assigned by the owning IRUnit.
   *
   * The instruction ID is stable and useful as an array index.  If the
   * Instruction is not arena-allocated by an IRUnit, the `m_id' field will be
   * kTransient (in which case this function should not be called).
   */
  uint32_t id() const;

  /*
   * Return true if the instruction is in a transient state.
   *
   * "Transient" means that the instruction is allocated on the stack and we
   * haven't yet committed to including it in the IRUnit's CFG.
   */
  bool isTransient() const;

  /*
   * Whether the instruction has one among a variadic list of opcodes.
   */
  template<typename... Args>
  bool is(Opcode op, Args&&... args) const;
  bool is() const;

  /*
   * Get or set the opcode of the instruction.
   */
  Opcode op() const;
  void setOpcode(Opcode);

  /*
   * Get or set the BCMarker of the instruction.
   */
  const BCMarker& marker() const;
  BCMarker& marker();

  /*
   * Check for, get, or set the instruction's optional type parameter.
   */
  bool hasTypeParam() const;
  Type typeParam() const;
  void setTypeParam(Type);


  /////////////////////////////////////////////////////////////////////////////
  // Srcs and dsts.

  /*
   * Initialize the source list for this IRInstruction.  We must not have
   * already had our sources initialized before this function is called.
   *
   * Memory for `srcs' is owned outside of this class and must outlive it.
   */
  void initializeSrcs(uint32_t numSrcs, SSATmp** srcs);

  /*
   * Number of srcs/dsts.
   */
  uint32_t numSrcs() const;
  uint32_t numDsts() const;

  /*
   * Return the ith src of this instruction.
   */
  SSATmp* src(uint32_t i) const;

  /*
   * Return the singular dst for this instruction.
   *
   * @requires: !naryDst()
   */
  SSATmp* dst() const;

  /*
   * Returns the ith dst of this instruction.
   *
   * If the instruction has no dsts, dst(0) will return nullptr, and if the
   * instruction is not NaryDst, dst(0) will return the single dst.  Otherwise,
   * it returns the first variadic dst.
   */
  SSATmp* dst(uint32_t i) const;

  /*
   * Get the srcs/dsts as a folly::Range.
   */
  folly::Range<SSATmp**> srcs() const;
  folly::Range<SSATmp**> dsts();

  /*
   * Set the ith src.
   */
  void setSrc(uint32_t i, SSATmp* newSrc);

  /*
   * Set the dsts, either as a single dst, or as `numDsts' dsts (if the
   * instruction has NaryDst).
   */
  void setDst(SSATmp* newDst);
  void setDsts(uint32_t numDsts, SSATmp** newDsts);


  /////////////////////////////////////////////////////////////////////////////
  // ExtraData.

  /*
   * Whether or not this instruction has an ExtraData.
   */
  bool hasExtra() const;

  /*
   * Return access to the ExtraData on this instruction, for the specified
   * opcode type.
   *
   * Pre: op() == opc
   */
  template<Opcode opc> const typename IRExtraDataType<opc>::type* extra() const;
  template<Opcode opc>       typename IRExtraDataType<opc>::type* extra();

  /*
   * Return access to ExtraData of type T.  Requires that
   * IRExtraDataType<opc>::type is T for this instruction's opcode.
   *
   * It's normally preferable to use the version of this function that takes
   * the opcode instead of this one.  This is for writing code that is supposed
   * to be able to handle multiple opcode types that share the same kind of
   * extra data.
   */
  template<class T> const T* extra() const;

  /*
   * Return the raw ExtraData pointer, for pretty-printing.
   */
  const IRExtraData* rawExtra() const;

  /*
   * Set the ExtraData for this instruction.
   *
   * The lifetime of the ExtraData must outlast this IRInstruction (and any of
   * its clones).
   */
  void setExtra(IRExtraData*);

  /*
   * Clear the extra data pointer in a IRInstruction.
   *
   * Used during IRUnit::gen to avoid having dangling IRExtraData*'s into stack
   * memory.
   */
  void clearExtra();


  /////////////////////////////////////////////////////////////////////////////
  // Blocks and edges.

  /*
   * Get/set the block containing this instruction.
   *
   * The block is also where the taken edge is coming from, if this instruction
   * has a taken edge.
   */
  Block* block() const;
  void setBlock(Block* b);

  /*
   * Optional fall-through edge.
   *
   * If present, this instruction must also have a taken edge.
   */
  Block* next() const;
  Edge* nextEdge();
  void setNext(Block* b);

  /*
   * Optional control flow edge.
   *
   * If present, this instruction must be the last one in the block.
   */
  Block* taken() const;
  Edge* takenEdge();
  void setTaken(Block* b);

  /*
   * Whether this instruction is a control flow instruction, or ends a block.
   */
  bool isControlFlow() const;
  bool isBlockEnd() const;
  bool isRawLoad() const;

private:
  /*
   * Block/edge implementations.
   */
  Block* succ(int i) const;
  Edge* succEdge(int i);
  void setSucc(int i, Block* b);
  void clearEdges();


  /////////////////////////////////////////////////////////////////////////////
  // Data members.

private:
  Type m_typeParam;  // garbage unless m_hasTypeParam is true
  Opcode m_op;
  uint16_t m_numSrcs;
  uint16_t m_numDsts : 15;
  bool m_hasTypeParam : 1;
  // <2 byte hole>
  BCMarker m_marker;
  const Id m_id;
  SSATmp** m_srcs;
  union {
    SSATmp* m_dest; // if HasDest
    SSATmp** m_dsts;// if NaryDest
  };
  Block* m_block;   // what block owns this instruction
  Edge* m_edges;    // outgoing edges, if this is a block-end
  IRExtraData* m_extra;

public:
  boost::intrusive::list_member_hook<> m_listNode; // for InstructionList
};

///////////////////////////////////////////////////////////////////////////////

using IRInstructionHookOption = boost::intrusive::member_hook<
  IRInstruction,
  boost::intrusive::list_member_hook<>,
  &IRInstruction::m_listNode
>;

using InstructionList = boost::intrusive::list<IRInstruction,
                                               IRInstructionHookOption>;

///////////////////////////////////////////////////////////////////////////////

/*
 * Return the output type from a given IRInstruction.
 *
 * The destination type is always predictable from the types of the inputs, any
 * type parameters to the instruction, and the id of the dst.
 */
Type outputType(const IRInstruction*, int dstId = 0);

///////////////////////////////////////////////////////////////////////////////
}}

#include "hphp/runtime/vm/jit/ir-instruction-inl.h"

#endif
