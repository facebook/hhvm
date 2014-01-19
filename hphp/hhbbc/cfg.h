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
#ifndef incl_HHBBC_CFG_H_
#define incl_HHBBC_CFG_H_

#include <vector>

#include <boost/variant/static_visitor.hpp>

#include "hphp/hhbbc/misc.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/bc.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

namespace detail {

template<class Fun>
struct TargetVisitor : boost::static_visitor<void> {
  explicit TargetVisitor(Fun f) : f(f) {}

  template<class T>
  typename std::enable_if<!has_target<T>::value,void>::type
  operator()(T const& t) const {}

  template<class T>
  typename std::enable_if<has_target<T>::value,void>::type
  operator()(T const& t) const { f(*t.target); }

  void operator()(const bc::Switch& b) const {
    for (auto& t : b.targets) f(*t);
  }

  void operator()(const bc::SSwitch& b) const {
    for (auto& kv : b.targets) f(*kv.second);
  }

private:
  Fun f;
};

template<class Fun>
void visitExnLeaves(const php::ExnNode& n, Fun f) {
  for (auto& c : n.children) visitExnLeaves(*c, f);
  f(n);
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Returns whether this block ends with an Unwind instruction.
 * I.e. it is terminal for a fault funclet.
 */
inline bool ends_with_unwind(const php::Block& b) {
  return b.hhbcs.back().op == Op::Unwind;
}

/*
 * Returns whether a block consists of a single Nop instruction.
 */
inline bool is_single_nop(const php::Block& b) {
  return b.hhbcs.size() == 1 && b.hhbcs.back().op == Op::Nop;
}

/*
 * Call a function for every jump target of a given bytecode.  If the
 * bytecode has no targets, the function is not called.
 */
template<class Fun>
void forEachTakenEdge(const Bytecode& b, Fun f) {
  visit(b, detail::TargetVisitor<Fun>(f));
}

/*
 * Opcode version of the above, for use in other visitors.
 */
template<class Fun, class T>
void forEachTakenEdge(const T& op, Fun f) {
  detail::TargetVisitor<Fun> v(f);
  v(op);
}

/*
 * Call a function for every successor of `block'.
 *
 * Order unspecified, and the types of successor edges are not
 * distinguished.
 *
 * Factored exit edges are traversed only if the block consists of
 * more than a single Nop instruction.  The order_blocks routine in
 * emit.cpp relies on this for correctness: if the only block for a
 * protected fault region is empty, we need to not include the fault
 * funclet blocks as reachable, or we can end up with fault funclet
 * handlers without an EHEnt pointing at them.  In cases other than
 * emit.cpp, this is not required for correctness, but is slightly
 * better than always traversing the factored exit edges.
 */
template<class Fun>
void forEachSuccessor(const php::Block& block, Fun f) {
  if (!is_single_nop(block)) {
    forEachTakenEdge(block.hhbcs.back(), f);
    for (auto& ex : block.factoredExits) f(*ex);
  }
  if (block.fallthrough) f(*block.fallthrough);
}

/*
 * Call a function for every successor of `block' that is reachable
 * through a non-factored edge.
 */
template<class Fun>
void forEachNormalSuccessor(const php::Block& block, Fun f) {
  forEachTakenEdge(block.hhbcs.back(), f);
  if (block.fallthrough) f(*block.fallthrough);
}

/*
 * Obtain the blocks for a function in a reverse post order, starting
 * with the main entry point.  The exact order is not specified.
 *
 * DV initializer blocks will not appear in this list.
 */
std::vector<borrowed_ptr<php::Block>> rpoSortFromMain(const php::Func&);

/*
 * Obtain the blocks for a function in a reverse post order, taking
 * into account all entry points.
 *
 * This can be thought of as an RPO on the CFG of Func starting from a
 * virtual empty "entry" block, with edges to each DV entry point and
 * an edge to the main entry point.
 */
std::vector<borrowed_ptr<php::Block>> rpoSortAddDVs(const php::Func&);

/*
 * Visit each leaf in the ExnNode tree.
 */
template<class Fun>
void visitExnLeaves(const php::Func& func, Fun f) {
  for (auto& n : func.exnNodes) {
    detail::visitExnLeaves(*n, f);
  }
}

//////////////////////////////////////////////////////////////////////

}}

#endif
