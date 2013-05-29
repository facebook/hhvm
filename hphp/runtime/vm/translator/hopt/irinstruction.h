/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "hphp/runtime/vm/translator/hopt/ir.h"
#include "hphp/runtime/vm/translator/hopt/edge.h"
#include "hphp/runtime/vm/translator/hopt/extradata.h"

namespace HPHP { namespace JIT {

/*
 * IRInstructions must be arena-allocatable.
 * (Destructors are not called when they come from IRFactory.)
 */
struct IRInstruction {
  enum Id { kTransient = 0xffffffff };

  /*
   * Create an IRInstruction for the opcode `op'.
   *
   * IRInstruction creation is usually done through IRFactory or
   * TraceBuilder rather than directly.
   */
  explicit IRInstruction(Opcode op,
                         uint32_t numSrcs = 0,
                         SSATmp** srcs = nullptr)
    : m_op(op)
    , m_typeParam(Type::None)
    , m_numSrcs(numSrcs)
    , m_numDsts(0)
    , m_id(kTransient)
    , m_srcs(srcs)
    , m_dst(nullptr)
    , m_extra(nullptr)
  {}

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
    auto opcode = op();
    if (debug) assert_opcode_extra<T>(opcode);
    return static_cast<const T*>(m_extra);
  }

  /*
   * Returns whether or not this opcode has an associated extra data
   * struct.
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
    * IRFactory::gen to avoid having dangling IRExtraData*'s into stack
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
   * need to keep, using factory for any needed memory.
   *
   * Pre: other->isTransient() || numDsts() == other->numDsts()
   */
  void become(IRFactory*, IRInstruction* other);

  /*
   * Deep-copy an IRInstruction, using factory to allocate memory for
   * the IRInstruction itself, and its srcs/dests.
   */
  IRInstruction* clone(IRFactory* factory) const;

  Opcode     op()   const       { return m_op; }
  void       setOpcode(Opcode newOpc)  { m_op = newOpc; }
  Type       typeParam() const         { return m_typeParam; }
  void       setTypeParam(Type t)      { m_typeParam = t; }
  uint32_t   numSrcs()  const          { return m_numSrcs; }
  void       setNumSrcs(uint32_t i)    {
    assert(i <= m_numSrcs);
    m_numSrcs = i;
  }
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
  Trace*     trace() const;

  /*
   * block() and setBlock() keep track of the block that contains this
   * instruction, as well as where the taken edge is coming from, if there
   * is a taken edge.
   */
  Block*     block() const { return m_taken.from(); }
  void       setBlock(Block* b) { m_taken.setFrom(b); }

  /*
   * Optional control flow edge.  If present, this instruction must
   * be the last one in the block.
   */
  Block*     taken() const { return m_taken.to(); }
  void       setTaken(Block* b) {
    if (isTransient()) m_taken.setTransientTo(b);
    else m_taken.setTo(b);
  }

  bool isControlFlowInstruction() const { return bool(m_taken.to()); }
  bool isBlockEnd() const { return m_taken.to() || isTerminal(); }
  bool isLoad() const;
  bool stores(uint32_t srcIdx) const;

  /*
   * Comparison and hashing for the purposes of CSE-equality.
   *
   * Pre: canCSE()
   */
  bool cseEquals(IRInstruction* inst) const;
  size_t cseHash() const;

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
  bool hasMemEffects() const;
  bool isRematerializable() const;
  bool isNative() const;
  bool consumesReferences() const;
  bool consumesReference(int srcNo) const;
  bool producesReference() const;
  bool mayModifyRefs() const;
  bool mayRaiseError() const;
  bool isEssential() const;
  bool isTerminal() const;
  bool isPassthrough() const;
  SSATmp* getPassthroughValue() const;
  bool killsSources() const;
  bool killsSource(int srcNo) const;

  bool modifiesStack() const;
  SSATmp* modifiedStkPtr() const;
  // hasMainDst provides raw access to the HasDest flag, for instructions with
  // ModifiesStack set.
  bool hasMainDst() const;

  friend const Edge* takenEdge(IRInstruction*); // only for validation

private:
  bool mayReenterHelper() const;

private:
  Opcode            m_op;
  Type              m_typeParam;
  uint16_t          m_numSrcs;
  uint16_t          m_numDsts;
  const Id          m_id;
  SSATmp**          m_srcs;
  SSATmp*           m_dst;     // if HasDest or NaryDest
  Edge              m_taken;   // for branches, guards, and jmp
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
