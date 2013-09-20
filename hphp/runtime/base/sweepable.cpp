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

#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static __thread Sweepable::Node t_sweep;

inline void Sweepable::Node::init() {
  next = prev = this;
}

inline void Sweepable::Node::enlist(Node& head) {
  Node* n = next = head.next;
  prev = &head;
  head.next = n->prev = this;
}

void Sweepable::Node::delist() {
  Node *n = next, *p = prev;
  n->prev = p;
  p->next = n;
}

void Sweepable::InitSweepableList() {
  t_sweep.init();
}

void Sweepable::SweepAll() {
  Node persist;
  persist.init();
  while (t_sweep.next != &t_sweep) {
    Node* n = t_sweep.next;
    n->delist();
    n->init();
    auto s = reinterpret_cast<Sweepable*>(uintptr_t(n)
                                          - offsetof(Sweepable, m_sweepNode));
    if (s->m_persistentCount == 0) {
      s->sweep();
    } else {
      n->enlist(persist);
    }
  }
  // copy persist list to t_sweep
  assert(t_sweep.next == &t_sweep && t_sweep.prev == &t_sweep);
  t_sweep.enlist(persist); // stick t_sweep in persist list
  persist.delist(); // remove persist; now t_sweep is "head"
}

Sweepable::Sweepable() : m_persistentCount(0) {
  m_sweepNode.enlist(t_sweep);
}

Sweepable::~Sweepable() {
  m_sweepNode.delist();
}

void Sweepable::unregister() {
  m_sweepNode.delist();
  m_sweepNode.init(); // in case destructor runs later.
}

///////////////////////////////////////////////////////////////////////////////
}

