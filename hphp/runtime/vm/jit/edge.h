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

#ifndef incl_HPHP_VM_EDGE_H_
#define incl_HPHP_VM_EDGE_H_

#include <boost/intrusive/list.hpp>

#include "hphp/runtime/vm/jit/containers.h"

namespace HPHP { namespace jit {

struct Block;
struct IRInstruction;

/*
 * An Edge represents a control-flow edge as an encapsulated pointer to a
 * successor block that maintains a list of predecessors of each block.
 * The predecessor list is updated by calling setTo().
 */
struct Edge {
  Edge() {}
  Edge(const Edge&) = delete;
  explicit Edge(IRInstruction* inst, Block* to) : m_inst(inst) { setTo(to); }

  // The instruction that owns this edge
  IRInstruction* inst() const { return m_inst; }
  void setInst(IRInstruction* inst) { m_inst = inst; };

  // The block this edge takes us to.  Changing this property updates
  // the affected Block's preds property.
  Block* to() const { return m_to; }
  void setTo(Block* to);

  // The block this edge comes from.
  Block* from() const;

  // set the to field but don't update any predecessor lists. Only used
  // for transient instructions.
  void setTransientTo(Block* to) { m_to = to; }

  // Use to/from accessors to access or mutate pointers
  Edge& operator=(const Edge&) = delete;

 private:
  Block* m_to{nullptr};
  IRInstruction* m_inst{nullptr};
 public:
  boost::intrusive::list_member_hook<> m_node; // for Block::m_preds
};

using EdgeHookOption = boost::intrusive::member_hook<
  Edge,
  boost::intrusive::list_member_hook<>,
  &Edge::m_node
>;

using EdgeList = boost::intrusive::list<Edge, EdgeHookOption>;
using EdgeSet = jit::flat_set<Edge*>;

}}

#endif
