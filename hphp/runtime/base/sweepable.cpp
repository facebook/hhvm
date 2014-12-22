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

#include "hphp/runtime/base/sweepable.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/util/logger.h"
#include <unordered_map>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using SweepMap = std::unordered_map<ObjectData*,Sweeper>;

static __thread Sweepable::Node t_sweep;
static thread_local SweepMap t_objects;

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

// Called once per thread initialization from ThreadInfo::init
void Sweepable::InitList() {
  t_sweep.init();
  t_objects.clear();
}

// Called when a thread goes idle; free any book-keeping overhead.
void Sweepable::FlushList() {
  assert(t_sweep.next == &t_sweep && t_objects.empty());
  t_objects = SweepMap();
}

unsigned Sweepable::SweepAll() {
  unsigned count = 0;
  // iterate until both sweep lists are empty. Entries can be added or removed
  // from either list during sweeping, so we drain the lists without holding
  // iterators across any sweep() callbacks.
  do {
    while (t_sweep.next != &t_sweep) {
      count++;
      auto n = t_sweep.next;
      n->delist();
      n->init();
      auto s = reinterpret_cast<Sweepable*>(
        uintptr_t(n) - offsetof(Sweepable, m_sweepNode)
      );
      s->sweep();
    }
    while (!t_objects.empty()) {
      // drain each bucket before moving to the next, avoiding O(n) begin()
      for (unsigned i = 0; i < t_objects.bucket_count(); i++) {
        for (auto it = t_objects.begin(i); it != t_objects.end(i);
             it = t_objects.begin(i)) {
          count++;
          auto obj = it->first;
          auto sweeper = it->second;
          t_objects.erase(obj);
          sweeper(obj);
        }
      }
    }
  } while (t_sweep.next != &t_sweep);
  return count;
}

Sweepable::Sweepable() {
  m_sweepNode.enlist(t_sweep);
}

Sweepable::~Sweepable() {
  m_sweepNode.delist();
}

void Sweepable::unregister() {
  m_sweepNode.delist();
  m_sweepNode.init(); // in case destructor runs later.
}

void registerSweepableObj(ObjectData* obj, Sweeper f) {
  t_objects[obj] = f;
}

void unregisterSweepableObj(ObjectData* obj) {
  t_objects.erase(obj);
}

///////////////////////////////////////////////////////////////////////////////
}

