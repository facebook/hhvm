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

#ifndef incl_HPHP_VM_IRINSTRUCTION_H_
#define incl_HPHP_VM_IRINSTRUCTION_H_

#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/edge.h"
#include "hphp/runtime/vm/jit/extra-data.h"

namespace HPHP { namespace JIT {

/*
 * BCMarker holds the location of a specific bytecode instruction, along with
 * the offset from vmfp to vmsp at the beginning of the instruction. Every
 * IRInstruction has one to keep track of which bytecode instruction it came
 * from.
 */
struct BCMarker {
  const Func* func;
  Offset      bcOff;
  int32_t     spOff;

  /*
   * This is for use by test code that needs to provide BCMarkers but is not
   * deriving them from an actual bytecode region. It is always valid().
   */
  static BCMarker Dummy() {
    return BCMarker(reinterpret_cast<const Func*>(1), 0, 0);
  }

  BCMarker()
    : func(nullptr)
    , bcOff(0)
    , spOff(0)
  {}

  BCMarker(const Func* f, Offset o, int32_t sp)
    : func(f)
    , bcOff(o)
    , spOff(sp)
  {
    assert(valid());
  }

  bool operator==(BCMarker b) const {
    return b.func == func &&
      b.bcOff == bcOff &&
      b.spOff == spOff;
  }
  bool operator!=(BCMarker b) const { return !operator==(b); }

  std::string show() const;
  bool valid() const;

  SrcKey sk() const {
    assert(valid());
    return SrcKey { func, bcOff };
  }
};

/*
 * IRInstructions must be arena-allocatable.
 * (Destructors are not called when they come from IRUnit.)
 */
struct IRInstruction {
  enum Id { kTransient = 0xffffffff };

  /*
   * Create an IRInstruction for the opcode `op'.
   *
   * IRInstruction creation is usually done through IRUnit or
   * IRBuilder rather than directly.
   */
  explicit IRInstruction(Opcode op,
                         BCMarker marker,
                         Edge* edges = nullptr,
                         uint32_t numSrcs = 0,
                         SSATmp** srcs = nullptr)
    : m_op(op)
    , m_typeParam(folly::none)
    , m_numSrcs(numSrcs)
    , m_numDsts(0)
    , m_id(kTransient)
    , m_srcs(srcs)
    , m_dst(nullptr)
    , m_block(nullptr)
    , m_edges(edges)
    , m_marker(marker)
    , m_extra(nullptr)
  {
    if (op != DefConst) {
      // DefConst is the only opcode that's allowed to not have a marker, since
      // it's not part of the instruction stream.
      assert(m_marker.valid());
    }
  }

  IRInstruction(const IRInstruction&) = delete;
  IRInstruction& operator=(const IRInstruction&) = delete;

  /*
   * Construct an IRInstruction as a deep copy of `inst', using
   * arena to allocate memory for its srcs/dests.
   */
  explicit IRInstruction(Arena& arena, const IRInstruction* inst, Id id);

  /*
   * Initialize the source list for this IRInstruction.  We must not
   * have already had our sources initialized before this function is
   * called.
   *
   * Memory for `srcs' is owned outside of this class and must outlive
   * it.
   */
  void initializeSrcs(uint32_t numSrcs, SSATmp** srcs) {
    assert(!m_srcs && !m_numSrcs);
    m_numSrcs = numSrcs;
    m_srcs = srcs;
  }

  /*
   * Return access to extra-data on this instruction, for the
   * specified opcode type.
   *
   * Pre: op() == opc
   */
  template<Opcode opc>
  const typename IRExtraDataType<opc>::type* extra() const {
    assert(opc == op() && "ExtraData type error");
    assert(m_extra != nullptr);
    return static_cast<typename IRExtraDataType<opc>::type*>(m_extra);
  }

  template<Opcode opc>
  typename IRExtraDataType<opc>::type* extra() {
    assert(opc == op() && "ExtraData type error");
    return static_cast<typename IRExtraDataType<opc>::type*>(m_extra);
  }

  /*
   * Return access to extra-data of type T.  Requires that
   * IRExtraDataType<opc>::type is T for this instruction's opcode.
   *
   * It's normally preferable to use the version of this function that
   * takes the opcode instead of this one.  This is for writing code
   * that is supposed to be able to handle multiple opcode types that
   * share the same kind of extra data.
   */
  template<class T> const T* extra() const {
    if (debug) assert_opcode_extra<T>(op());
    return static_cast<const T*>(m_extra);
  }

  /*
   * Returns whether or not this instruction has an extra data struct.
   */
  bool hasExtra() const;

  /*
   * Set the extra-data for this IRInstruction to the given pointer.
   * Lifetime is must outlast this IRInstruction (and any of its
   * clones).
   */
  void setExtra(IRExtraData* data) { assert(!m_extra); m_extra = data; }

  /*
   * Return the raw extradata pointer, for pretty-printing.
   */
  const IRExtraData* rawExtra() const { return m_extra; }

   /*
    * Clear the extra data pointer in a IRInstruction.  Used during
    * IRUnit::gen to avoid having dangling IRExtraData*'s into stack
    * memory.
    */
  void clearExtra() { m_extra = nullptr; }

  /*
   * Replace an instruction in place with a Nop.  This sometimes may
   * be a result of a simplification pass.
   */
  void convertToNop();

  /*
   * Replace a branch with a Jmp; used when we have proven the branch
   * is always taken.
   */
  void convertToJmp();
  void convertToJmp(Block* target);

  /*
   * Replace an instruction in place with a Mov. Used when we have
   * proven that the instruction's side effects are not needed.
   *
   * TODO: replace with become
   */
  void convertToMov();

  /*
   * Turns this instruction into the target instruction, without
   * changing stable fields (id, current block, list fields).  The
   * existing destination SSATmp(s) will continue to think they came
   * from this instruction.
   *
   * The target instruction may be transient---we'll clone anything we
   * need to keep, using IRUnit for any needed memory.
   *
   * Pre: other->isTransient() || numDsts() == other->numDsts()
   */
  void become(IRUnit&, IRInstruction* other);

  /*
   * Add an additional src SSATmp and dst Operand to this Shuffle.
   */
  void addCopy(IRUnit&, SSATmp* src, const PhysLoc& dest);

  bool       is() const { return false; }
  template<typename... Args>
  bool       is(Opcode op, Args&&... args) const {
    return m_op == op || is(std::forward<Args>(args)...);
  }

  Opcode     op()   const       { return m_op; }
  void       setOpcode(Opcode newOpc);
  bool       hasTypeParam() const      { return m_typeParam.hasValue(); }
  Type       typeParam() const         { return m_typeParam.value(); }
  folly::Optional<Type> maybeTypeParam() const { return m_typeParam; }
  void       setTypeParam(Type t) {
    m_typeParam.assign(t);
  }
  uint32_t   numSrcs()  const          { return m_numSrcs; }
  SSATmp*    src(uint32_t i) const;
  void       setSrc(uint32_t i, SSATmp* newSrc);
  SrcRange   srcs() const {
    return SrcRange(m_srcs, m_numSrcs);
  }
  unsigned   numDsts() const { return m_numDsts; }
  SSATmp*    dst() const {
    assert(!naryDst());
    return m_dst;
  }
  void       setDst(SSATmp* newDst) {
    assert(hasDst());
    m_dst = newDst;
    m_numDsts = newDst ? 1 : 0;
  }

  /*
   * Returns the ith dest of this instruction. i == 0 is treated specially: if
   * the instruction has no dests, dst(0) will return nullptr, and if the
   * instruction is not naryDest, dst(0) will return the single dest.
   */
  SSATmp*    dst(unsigned i) const;
  DstRange   dsts();
  Range<const SSATmp*> dsts() const;
  void       setDsts(unsigned numDsts, SSATmp* newDsts) {
    assert(naryDst());
    m_numDsts = numDsts;
    m_dst = newDsts;
  }

  /*
   * Instruction id is stable and useful as an array index.
   */
  uint32_t id() const {
    assert(m_id != kTransient);
    return m_id;
  }

  /*
   * Returns true if the instruction is in a transient state.  That
   * is, it's allocated on the stack and we haven't yet committed to
   * inserting it in any blocks.
   */
  bool       isTransient() const       { return m_id == kTransient; }

  /*
   * block() and setBlock() keep track of the block that contains this
   * instruction, as well as where the taken edge is coming from, if there
   * is a taken edge.
   */
  Block* block() const { return m_block; }
  void setBlock(Block* b) { m_block = b; }

  /*
   * Optional control flow edge.  If present, this instruction must
   * be the last one in the block.
   */
  Block* taken() const { return succ(1); }
  Edge* takenEdge() { return succEdge(1); }
  void setTaken(Block* b) { return setSucc(1, b); }

  /*
   * Optional fall-through edge.  If present, this instruction must
   * also have a taken edge.
   */
  Block* next() const { return succ(0); }
  Edge* nextEdge() { return succEdge(0); }
  void setNext(Block* b) { return setSucc(0, b); }

  bool isControlFlow() const { return bool(taken()); }
  bool isBlockEnd() const { return taken() || isTerminal(); }
  bool isRawLoad() const;

  /*
   * Comparison and hashing for the purposes of CSE-equality.
   *
   * Pre: canCSE()
   */
  bool cseEquals(IRInstruction* inst) const;
  size_t cseHash() const;

  void setMarker(BCMarker marker) {
    assert(marker.valid());
    m_marker = marker;
  }
  const BCMarker& marker() const { return m_marker; }
  BCMarker& marker()             { return m_marker; }

  std::string toString() const;

  /*
   * Helper accessors for the OpcodeFlag bits for this instruction.
   *
   * Note that these wrappers have additional logic beyond just
   * checking the corresponding flags bit---you should generally use
   * these when you have an actual IRInstruction instead of just an
   * Opcode enum value.
   */
  bool canCSE() const;
  bool hasDst() const;
  bool naryDst() const;
  bool isNative() const;
  bool consumesReferences() const;
  bool consumesReference(int srcNo) const;
  bool producesReference(int dstNo) const;
  bool mayRaiseError() const;
  bool isEssential() const;
  bool isTerminal() const;
  bool hasEdges() const { return JIT::hasEdges(op()); }
  bool isPassthrough() const;
  SSATmp* getPassthroughValue() const;
  bool killsSources() const;
  bool killsSource(int srcNo) const;

  bool modifiesStack() const;
  SSATmp* modifiedStkPtr() const;
  SSATmp* previousStkPtr() const;
  // hasMainDst provides raw access to the HasDest flag, for instructions with
  // ModifiesStack set.
  bool hasMainDst() const;

private:
  Block* succ(int i) const {
    assert(!m_edges || hasEdges());
    return m_edges ? m_edges[i].to() : nullptr;
  }
  Edge* succEdge(int i) {
    assert(!m_edges || hasEdges());
    return m_edges && m_edges[i].to() ? &m_edges[i] : nullptr;
  }
  void setSucc(int i, Block* b) {
    if (hasEdges()) {
      if (isTransient()) m_edges[i].setTransientTo(b);
      else m_edges[i].setTo(b);
    } else {
      assert(!b && !m_edges);
    }
  }
  void clearEdges() {
    setSucc(0, nullptr);
    setSucc(1, nullptr);
    m_edges = nullptr;
  }

private:
  Opcode            m_op;
  folly::Optional<Type> m_typeParam;
  uint16_t          m_numSrcs;
  uint16_t          m_numDsts;
  const Id          m_id;
  SSATmp**          m_srcs;
  SSATmp*           m_dst;     // if HasDest or NaryDest
  Block*            m_block;   // what block owns this instruction
  Edge*             m_edges;   // outgoing edges, if this is a block-end.
  BCMarker          m_marker;
  IRExtraData*      m_extra;
public:
  boost::intrusive::list_member_hook<> m_listNode; // for InstructionList
};

typedef boost::intrusive::member_hook<IRInstruction,
                                      boost::intrusive::list_member_hook<>,
                                      &IRInstruction::m_listNode>
        IRInstructionHookOption;
typedef boost::intrusive::list<IRInstruction, IRInstructionHookOption>
        InstructionList;

}}

#endif
