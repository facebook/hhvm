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

#ifndef incl_HPHP_IR_UNIT_H_
#define incl_HPHP_IR_UNIT_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/check.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-instr-table.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/region-selection.h"
#include "hphp/runtime/vm/jit/translator.h"

#include "hphp/util/arena.h"

#include <string>
#include <type_traits>

namespace HPHP {
//////////////////////////////////////////////////////////////////////

struct SrcKey;

namespace jit {

//////////////////////////////////////////////////////////////////////

struct Block;
struct IRInstruction;
struct SSATmp;

//////////////////////////////////////////////////////////////////////

/*
 * Create an IRInstruction on the stack using `args', and call `func' with a
 * pointer to it, returning the result.
 *
 * Normally IRInstruction creation should go through either IRUnit::gen or
 * IRBuilder::gen.  This utility is used to implement those.
 *
 * The IRInstruction* must not escape `func'---the aforementioned gen() methods
 * use IRUnit::clone() to duplicate the instruction in arena memory.
 */
template<class Func, class... Args>
typename std::result_of<Func(IRInstruction*)>::type
makeInstruction(Func func, Args&&... args);

//////////////////////////////////////////////////////////////////////

/*
 * Map from DefLabel instructions to produced references.
 *
 * See comment in IRBuilder::cond for more details.
 */
using LabelRefs = jit::hash_map<const IRInstruction*, jit::vector<uint32_t>>;

//////////////////////////////////////////////////////////////////////

/*
 * IRUnit is the compilation unit for the JIT.  It owns an Arena used for
 * allocating and controlling the lifetime of Block, IRInstruction, ExtraData,
 * and SSATmp objects, as well as a constant table containing all DefConst
 * instructions, which don't live in Blocks.
 *
 * IRUnit also assigns unique ids to each block, instruction, and tmp, which
 * are useful for StateVector or sparse maps of pass-specific information.
 */
struct IRUnit {
  /*
   * Construct an IRUnit with a single, empty entry Block.
   */
  explicit IRUnit(TransContext context);


  /////////////////////////////////////////////////////////////////////////////
  // Instruction creation.

  /*
   * Create an IRInstruction with lifetime equivalent to this IRUnit.
   *
   * Arguments are passed in the following format:
   *
   *   gen(Opcode op,
   *       BCMarker marker,
   *       const IRExtraData&   // optional
   *       Type typeParam,      // optional
   *       Block* target,       // optional
   *       SSATmp* srcs...
   *   );
   *
   * Optional arguments can be ordered arbitrarily.  The `srcs' must come last,
   * and can be specified either as a varargs list of SSATmp*'s, or as a pair
   * of (number of tmps, SSATmp**).  Zero `srcs' is allowed.
   */
  template<class... Args>
  IRInstruction* gen(Args&&... args);

  /*
   * Create an IRInstruction with a pre-allocated destination operand.
   *
   * The `dst' argument will be retyped to match the newly generated
   * instruction.
   *
   * This function takes arguments in the same format as gen().
   */
  template<class... Args>
  IRInstruction* gen(SSATmp* dst, Args&&... args);

  /*
   * Replace an existing IRInstruction with a new one.
   *
   * This may involve making more allocations in the arena, but the actual
   * IRInstruction itself (i.e., its address, its ID, its BCMarker, etc.) will
   * stay unchanged.
   *
   * This function takes arguments in the same format as gen(), except that the
   * BCMarker is omitted.
   */
  template<class... Args>
  void replace(IRInstruction* old, Opcode op, Args... args);

  /*
   * Deep-copy an IRInstruction and its srcs/dsts into arena-allocated memory.
   *
   * If provided, `dst' will be used as the clone's `dst' instead of a newly-
   * allocated SSATmp.
   */
  IRInstruction* clone(const IRInstruction* inst, SSATmp* dst = nullptr);


  /////////////////////////////////////////////////////////////////////////////
  // Accessors.

  /*
   * Basic getters.
   */
  Arena&              arena();
  const TransContext& context() const;
  Block*              entry() const;
  // TODO(#3538578): The above should return `const Block*'.

  /*
   * Starting positions, from the TransContext.
   */
  uint32_t bcOff() const;
  SrcKey initSrcKey() const;

  /*
   * Counts of the number of SSATmps, Blocks, and IRInstructions allocated in
   * this IRUnit's arena.
   *
   * Note that these are /not/ the counts of these objects in the Unit's CFG;
   * they merely track the number of objects we have assigned IDs to.
   */
  uint32_t numTmps() const;
  uint32_t numBlocks() const;
  uint32_t numInsts() const;

  /*
   * Overloads useful for StateVector and IdSet.
   */
  uint32_t numIds(const SSATmp*) const;
  uint32_t numIds(const Block*) const;
  uint32_t numIds(const IRInstruction*) const;

  /*
   * Find an SSATmp* from an id.
   */
  SSATmp* findSSATmp(uint32_t id) const;

  /*
   * Return the main FramePtr for the unit.  This is the result of the DefFP
   * instruction on the entry block.
   */
  SSATmp* mainFP() const;

  /////////////////////////////////////////////////////////////////////////////

  /*
   * Add a block to the IRUnit's arena.
   */
  Block* defBlock(Block::Hint hint = Block::Hint::Neither);

  /*
   * Add a DefConst instruction to the const table.
   */
  template<typename T>
  SSATmp* cns(T val);
  SSATmp* cns(Type type);

  /*
   * Create a DefLabel instruction.
   */
  IRInstruction* defLabel(unsigned numDst, BCMarker marker);

  /*
   * Add some extra destinations to a defLabel.
   */
  void expandLabel(IRInstruction* label, unsigned extraDst);

  /*
   * Add an extra SSATmp to jmp.
   */
  void expandJmp(IRInstruction* jmp, SSATmp* value);

private:
  template<class... Args> SSATmp* newSSATmp(Args&&...);

private:
  // Contains Block, IRInstruction, and SSATmp objects.
  Arena m_arena;

  // DefConsts for each unique constant in this IR.
  IRInstrTable m_constTable;

  // Translation context for which this IRUnit was created.
  TransContext const m_context;

  // Counters for m_arena allocations.
  uint32_t m_nextBlockId{0};
  uint32_t m_nextInstId{0};

  // Entry point.
  Block* m_entry;

  // Map from SSATmp ids to SSATmp*.
  jit::vector<SSATmp*> m_ssaTmps;
};

//////////////////////////////////////////////////////////////////////

/*
 * Create a debug string for an IRUnit.
 */
std::string show(const IRUnit&);

//////////////////////////////////////////////////////////////////////

Block* findMainExitBlock(const IRUnit& unit, SrcKey lastSk);

//////////////////////////////////////////////////////////////////////

}}

#include "hphp/runtime/vm/jit/ir-unit-inl.h"

#endif
