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

#ifndef __HPHP_TAINT_TRACE_PTR_H__
#define __HPHP_TAINT_TRACE_PTR_H__

#ifdef TAINTED

#include <runtime/base/util/countable.h>
#include <runtime/base/util/smart_ptr.h>
#include <runtime/base/memory/smart_allocator.h>

namespace HPHP {

class TaintTraceData;

/*
 * SmartPtr for TaintTraceData.
 */
class TaintTraceDataPtr : public SmartPtr<TaintTraceData> {
public:
  TaintTraceDataPtr();
  ~TaintTraceDataPtr();

  TaintTraceDataPtr(TaintTraceData* ttd);
  TaintTraceDataPtr(const TaintTraceDataPtr& ttd);
};

class TaintTraceNode;

/*
 * A SmartPtr for TaintTraceNode which does automatic refcounting.
 */
class TaintTraceNodePtr : public SmartPtr<TaintTraceNode> {
public:
  TaintTraceNodePtr() { }
  ~TaintTraceNodePtr() { }

  TaintTraceNodePtr(TaintTraceNode* ttn)
    : SmartPtr<TaintTraceNode>(ttn) { }
  TaintTraceNodePtr(const TaintTraceNodePtr& ttn)
    : SmartPtr<TaintTraceNode>(ttn) { }

  TaintTraceNodePtr &operator=(const TaintTraceDataPtr& data);
};

/*
 * A node in a tree representing taint trace information. While the structure
 * is ultimately just a binary tree, we distinguish sibling and child to
 * better capture the semantics that TaintObserver enforces for passing trace.
 *
 * Within the scope of a given TaintObserver, the traced callsites attached
 * to all HTML strings are globbed together so that they will be passed to any
 * newly created string. In order to avoid duplication, we create new nodes
 * whose children are these aggregated trace subtrees. Note that by this
 * mechanism, once a node is created, it should not and must not be altered,
 * nor should any of its children (although their refcounts may increment).
 *
 * TaintTraceNodes are fixed-sized, SmartAllocated objects with no out-of-
 * line pointers. This allows us to take advantage of SmartAllocation's
 * preservation of internal pointers through backups and restores, and hence
 * allows us to keep trace data on any strings that persist across requests.
 */
class TaintTraceNode : public Countable {
public:
  TaintTraceNode() { }

  TaintTraceNode(TaintTraceNodePtr sibling, TaintTraceNodePtr child)
    : m_sibling(sibling), m_child(child) { }
  TaintTraceNode(TaintTraceNodePtr sibling, TaintTraceDataPtr data)
    : m_sibling(sibling), m_leaf(data) { }

  const TaintTraceNodePtr& getNext() const { return m_sibling; }
  const TaintTraceNodePtr& getChild() const { return m_child; }
  const TaintTraceDataPtr& getLeaf() const { return m_leaf; }

  void setRefCount(int n) { _count = n; }

  // SmartAllocator methods
  DECLARE_SMART_ALLOCATION_NOCALLBACKS(TaintTraceNode);
  void dump() const { }

private:
  const TaintTraceNodePtr m_sibling;
  const TaintTraceNodePtr m_child;
  const TaintTraceDataPtr m_leaf;
};

}

#endif // TAINTED

#endif // __HPHP_TAINT_TRACE_PTR_H__
