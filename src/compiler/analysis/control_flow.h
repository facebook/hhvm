/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __CONTROL_FLOW_H__
#define __CONTROL_FLOW_H__

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

#include <compiler/hphp.h>

#include <compiler/analysis/ast_walker.h>

enum vertex_control_block_t { control_block };

namespace boost {
  BOOST_INSTALL_PROPERTY(vertex, control_block);
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Construct);
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(MethodStatement);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ControlBlock);
DECLARE_BOOST_TYPES(AnalysisResult);

typedef boost::adjacency_list<
  boost::listS, boost::listS, boost::bidirectionalS,
  boost::property<vertex_control_block_t, ControlBlockPtr,
                  boost::property<boost::vertex_color_t,
                                  boost::default_color_type> >
  > ControlFlowGraphBase;

class ControlFlowGraph : public ControlFlowGraphBase {
public:
  typedef boost::graph_traits<ControlFlowGraphBase> graph_traits;
  typedef graph_traits::vertex_descriptor vertex_descriptor;
  typedef graph_traits::adjacency_iterator adjacency_iterator;
  typedef graph_traits::vertex_iterator vertex_iterator;
  typedef boost::property_map<ControlFlowGraphBase,
                              vertex_control_block_t>::type control_block_map_t;

  static ControlFlowGraph *buildControlFlow(MethodStatementPtr m);
  void                  dump(AnalysisResultPtr ar);
private:
  FunctionScopePtr      funcScope;
};

class ControlBlock {
public:
  typedef ControlFlowGraph::vertex_descriptor   vertex_descriptor;
  ControlBlock(const AstWalkerStateVec &s);

  void                  setDfn(int n) { m_dfn = n; }
  int                   getDfn() const { return m_dfn; }
  void                  setEndBefore(ConstructPtr e) { m_endBefore = e; }
  void                  setEndAfter(ConstructPtr e) { m_endAfter = e; }
  void                  setVertex(vertex_descriptor v) { m_vertex = v; }
  vertex_descriptor     getVertex() const { return m_vertex; }
  ConstructPtr          getStartNode() const { return m_start.back().cp; }
  int                   getStartIndex() const { return m_start.back().index; }
  AstWalkerState        getStartState() const { return m_start.back(); }
  void                  dump(int spc, AnalysisResultPtr ar,
                             const ControlFlowGraph *graph);
private:
  int                   m_dfn;
  vertex_descriptor     m_vertex;
  AstWalkerStateVec     m_start;
  ConstructPtr          m_endBefore;
  ConstructPtr          m_endAfter;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CONTROL_FLOW_H__
