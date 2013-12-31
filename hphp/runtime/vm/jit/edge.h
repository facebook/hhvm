/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include <boost/noncopyable.hpp>

namespace HPHP { namespace JIT {

struct Block;
struct IRInstruction;

/*
 * An Edge represents a control-flow edge as an encapsulated pointer to a
 * successor block that maintains a list of predecessors of each block.
 * The predecessor list is updated by calling setTo().
 */
struct Edge : private boost::noncopyable {
  Edge() : m_to(nullptr), m_inst(nullptr) {}
  Edge(const Edge& other) : m_to(nullptr), m_inst(nullptr) {
    setTo(other.m_to);
  }
  explicit Edge(IRInstruction* inst, Block* to) : m_to(nullptr), m_inst(inst) {
    setTo(to);
  }

  // The instruction that owns this edge
  IRInstruction* inst() const { return m_inst; }
  void setInst(IRInstruction* inst) { m_inst = inst; };

  // The block this edge takes us to.  Changing this property updates
  // the affected Block's preds property.
  Block* to() const { return m_to; }
  void setTo(Block* to);

  // set the to field but don't update any predecessor lists. Only used
  // for transient instructions.
  void setTransientTo(Block* to) {
    m_to = to;
  }

  // Use to/from accessors to access or mutate pointers
  Edge& operator=(const Edge& other) = delete;

 private:
  Block* m_to;
  IRInstruction* m_inst;
 public:
  boost::intrusive::list_member_hook<> m_node; // for Block::m_preds
};

typedef boost::intrusive::member_hook<Edge,
                                      boost::intrusive::list_member_hook<>,
                                      &Edge::m_node>
        EdgeHookOption;
typedef boost::intrusive::list<Edge, EdgeHookOption> EdgeList;

}}

#endif
