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

#ifndef incl_HPHP_VM_TRACE_H_
#define incl_HPHP_VM_TRACE_H_

#include "hphp/runtime/vm/translator/hopt/block.h"
#include "hphp/runtime/vm/translator/hopt/ir.h"

namespace HPHP { namespace JIT {

/*
 * A Trace is a single-entry, multi-exit, sequence of blocks.  Typically
 * each block falls through to the next block but this is not guaranteed;
 * traces may contain internal forward-only control flow.
 */
class Trace : boost::noncopyable {
public:
  explicit Trace(Block* first, uint32_t bcOff)
    : m_bcOff(bcOff)
    , m_main(nullptr)
  {
    push_back(first);
  }

  ~Trace() {
    std::for_each(m_exitTraces.begin(), m_exitTraces.end(),
                  boost::checked_deleter<Trace>());
  }

  std::list<Block*>& getBlocks() { return m_blocks; }
  const std::list<Block*>& getBlocks() const { return m_blocks; }
  Block* front() { return *m_blocks.begin(); }
  Block* back() { auto it = m_blocks.end(); return *(--it); }
  const Block* front() const { return *m_blocks.begin(); }
  const Block* back()  const { auto it = m_blocks.end(); return *(--it); }

  Block* push_back(Block* b) {
    b->setTrace(this);
    m_blocks.push_back(b);
    return b;
  }

  // temporary data field for use by individual passes
  //
  // Used by LinearScan as a "fake" instruction id, that comes
  // between the id of the last instruction that branches to
  // this exit trace, and the next instruction on the main trace.
  uint32_t getData() const { return m_data; }
  void setData(uint32_t d) { m_data = d; }

  uint32_t getBcOff() const { return m_bcOff; }
  Trace* addExitTrace(Trace* exit) {
    m_exitTraces.push_back(exit);
    exit->setMain(this);
    return exit;
  }
  bool isMain() const { return m_main == nullptr; }
  void setMain(Trace* t) {
    assert(m_main == nullptr);
    m_main = t;
  }
  Trace* getMain() {
    return m_main;
  }

  typedef std::list<Trace*> ExitList;
  typedef std::list<Trace*>::iterator ExitIterator;

  ExitList& getExitTraces() { return m_exitTraces; }
  const ExitList& getExitTraces() const { return m_exitTraces; }
  std::string toString() const;

private:
  // offset of the first bytecode in this trace; 0 if this trace doesn't
  // represent a bytecode boundary.
  uint32_t m_bcOff;
  uint32_t m_data;
  std::list<Block*> m_blocks; // Blocks in main trace starting with entry block
  ExitList m_exitTraces;      // traces to which this trace exits
  Trace* m_main;              // ptr to parent trace if this is an exit trace
};

}}

#endif
