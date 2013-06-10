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

#ifndef incl_HPHP_TREADMILL_H_
#define incl_HPHP_TREADMILL_H_

#include "hphp/runtime/vm/unit.h"

namespace HPHP {

class Class;
class Typedef;

namespace Treadmill {

/*
 * The Treadmill allows us to defer work until all currently-outstanding
 * requests have finished. We hook request start and finish. To defer
 * work, inherit from WorkItem and call Treadmill::WorkItem::enqueue.
 *
 * The work item will be called from base rank.
 */
void startRequest(int threadId);
void finishRequest(int threadId);

/*
 * Ask for memory to be freed (as in free, not delete) by the next
 * appropriate treadmill round.
 */
void deferredFree(void*);

typedef uint64_t GenCount;

class WorkItem {
 protected:
  GenCount m_gen;
  friend void finishRequest(int threadId);

 public:
  WorkItem();
  virtual ~WorkItem() { }
  virtual void operator()() = 0; // doesn't throw.
  static void enqueue(WorkItem* gt);
};

class FreeMemoryTrigger : public WorkItem {
  void* m_ptr;
 public:
  explicit FreeMemoryTrigger(void* ptr);
  virtual void operator()();
};

}}

#endif
