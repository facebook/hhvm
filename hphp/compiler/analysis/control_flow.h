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

#ifndef incl_HPHP_CONTROL_FLOW_H_
#define incl_HPHP_CONTROL_FLOW_H_

#include <boost/graph/properties.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/adjacency_list.hpp>

#include "hphp/compiler/hphp.h"

#include "hphp/compiler/analysis/ast_walker.h"
#include "hphp/compiler/analysis/bit_set_vec.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ControlBlock;
class ControlFlowGraph;
class ControlEdge;
class AdjacencyIterator;
class InvAdjdacencyIterator;

///////////////////////////////////////////////////////////////////////////////
}

namespace boost {
///////////////////////////////////////////////////////////////////////////////

template <>
struct graph_traits<HPHP::ControlFlowGraph> {
  typedef HPHP::ControlBlock *vertex_descriptor;
  typedef HPHP::ControlEdge *edge_descriptor;

  typedef std::list<HPHP::ControlEdge*>::const_iterator in_edge_iterator;
  typedef std::list<HPHP::ControlEdge*>::const_iterator out_edge_iterator;
  typedef std::list<HPHP::ControlBlock*>::const_iterator vertex_iterator;
  typedef std::list<HPHP::ControlEdge*>::const_iterator edge_iterator;
  typedef adjacency_iterator_generator<
    HPHP::ControlFlowGraph,
    vertex_descriptor,
    out_edge_iterator>::type adjacency_iterator;
  typedef inv_adjacency_iterator_generator<
    HPHP::ControlFlowGraph,
    vertex_descriptor, in_edge_iterator>::type inv_adjacency_iterator;

  typedef directed_tag directed_category;
  typedef disallow_parallel_edge_tag edge_parallel_category;
  typedef bidirectional_graph_tag traversal_category;

  typedef int vertices_size_type;
  typedef int edges_size_type;
  typedef std::list<HPHP::ControlEdge*>::size_type degree_size_type;

  static vertex_descriptor null_vertex() { return nullptr; }
};

template<>
struct property_traits<vertex_color_t> {
  typedef default_color_type value_type;
  typedef HPHP::ControlBlock *key_type;
  typedef default_color_type &reference;
  typedef lvalue_property_map_tag category;
};

///////////////////////////////////////////////////////////////////////////////
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(Construct);
DECLARE_BOOST_TYPES(Expression);
DECLARE_BOOST_TYPES(Statement);
DECLARE_BOOST_TYPES(MethodStatement);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(AnalysisResult);

class ControlFlowGraph {
public:
  typedef boost::graph_traits<ControlFlowGraph> graph_traits;
  typedef graph_traits::vertex_descriptor vertex_descriptor;
  typedef graph_traits::adjacency_iterator adjacency_iterator;
  typedef graph_traits::vertex_iterator vertex_iterator;
  typedef graph_traits::edge_iterator edge_iterator;
  typedef graph_traits::out_edge_iterator out_edge_iterator;
  typedef graph_traits::in_edge_iterator in_edge_iterator;

  static ControlFlowGraph *buildControlFlow(MethodStatementPtr m);
  ~ControlFlowGraph();

  void                  dump(AnalysisResultConstPtr ar);
  void                  allocateDataFlow(size_t width, int rows, int *rowIds);
  ControlBlock          *getDfBlock(int dfn) const {
    return m_depthFirstBlocks.at(dfn - 1);
  }
  BitOps::Bits          *getTempBits(int id) const {
    return m_bitSetVec.getTempBits(id);
  }
  bool                  rowExists(int row) const {
    return m_bitSetVec.rowExists(row);
  }
  size_t                bitWidth() const { return m_bitSetVec.width(); }
  int                   getNumBlocks() const { return m_nextDfn - 1; }
  void                  dfnAdd(ControlBlock *cb);
  vertex_iterator       vbegin() const { return m_blocks.begin(); }
  vertex_iterator       vend() const { return m_blocks.end(); }
  edge_iterator         ebegin() const { return m_edges.begin(); }
  edge_iterator         eend() const { return m_edges.end(); }
  ControlBlock          *add_vertex(AstWalkerStateVec &s);
  ControlEdge           *add_edge(ControlBlock *a, ControlBlock *b);
  MethodStatementPtr    getMethod() { return m_stmt; }
private:
  MethodStatementPtr         m_stmt;
  BitSetVec                  m_bitSetVec;
  int                        m_nextDfn;
  std::list<ControlEdge*>    m_edges;
  std::list<ControlBlock*>   m_blocks;
  std::vector<ControlBlock*> m_depthFirstBlocks;
};

class ControlEdge : public std::pair<ControlBlock*,ControlBlock*> {
public:
  ControlEdge(ControlBlock *from, ControlBlock *to) :
      std::pair<ControlBlock*,ControlBlock*>(from, to) {}
};

class ControlBlock {
  typedef ControlFlowGraph::graph_traits graph_traits;
  typedef boost::default_color_type color_type;
public:
  ControlBlock(const AstWalkerStateVec &s, ControlBlock *prev);

  void                  setDfn(int n) { m_dfn = n; }
  int                   getDfn() const { return m_dfn; }
  BitOps::Bits          *getRow(int row) { return m_bitBlock.getRow(row); }

  void                  setBit(int row, int id, bool flag = true) {
    m_bitBlock.setBit(row, id, flag);
  }
  bool                  getBit(int row, int id) const {
    return m_bitBlock.getBit(row, id);
  }
  void                  setBlock(const BitSetBlock &b) { m_bitBlock = b; }
  void                  setEndBefore(ConstructRawPtr e) { m_endBefore = e; }
  void                  setEndAfter(ConstructRawPtr e) { m_endAfter = e; }
  ConstructRawPtr       getStartNode() const { return m_start.back().cp; }
  const AstWalkerStateVec &getStartState() const { return m_start; }
  ConstructRawPtr       getEndBefore() const { return m_endBefore; }
  ConstructRawPtr       getEndAfter() const { return m_endAfter; }
  void                  dump(int spc, AnalysisResultConstPtr ar,
                             const ControlFlowGraph *graph);

  graph_traits::in_edge_iterator ibegin() const { return m_preds.begin(); }
  graph_traits::in_edge_iterator iend() const { return m_preds.end(); }
  graph_traits::out_edge_iterator obegin() const { return m_succs.begin(); }
  graph_traits::out_edge_iterator oend() const { return m_succs.end(); }
  graph_traits::adjacency_iterator abegin(const ControlFlowGraph &g) const {
    return graph_traits::adjacency_iterator(m_succs.begin(), &g);
  }
  graph_traits::adjacency_iterator aend(const ControlFlowGraph &g) const {
    return graph_traits::adjacency_iterator(m_succs.end(), &g);
  }
  graph_traits::inv_adjacency_iterator iabegin(const ControlFlowGraph &g) {
    return graph_traits::inv_adjacency_iterator(m_preds.begin(), &g);
  }
  graph_traits::inv_adjacency_iterator iaend(const ControlFlowGraph &g) {
    return graph_traits::inv_adjacency_iterator(m_preds.end(), &g);
  }
  ControlEdge             *find_to(ControlBlock *to);
  ControlEdge             *find_from(ControlBlock *from);
  static void             add_edge(ControlEdge *e);
  int in_size() const { return m_preds.size(); }
  int out_size() const { return m_succs.size(); }
  color_type get_color() { return m_color; }
  void set_color(color_type c) { m_color = c; }
  ControlBlock            *next() const { return m_next; }
private:
  int                     m_dfn;
  AstWalkerStateVec       m_start;
  ConstructRawPtr         m_endBefore;
  ConstructRawPtr         m_endAfter;
  BitSetBlock             m_bitBlock;
  std::list<ControlEdge*> m_preds;
  std::list<ControlEdge*> m_succs;
  color_type              m_color;
  ControlBlock            *m_next;
};

///////////////////////////////////////////////////////////////////////////////

inline boost::graph_traits<ControlFlowGraph>::vertex_descriptor
source(boost::graph_traits<ControlFlowGraph>::edge_descriptor e,
       const ControlFlowGraph& g) {
  return e->first;
}

inline boost::graph_traits<ControlFlowGraph>::vertex_descriptor
target(boost::graph_traits<ControlFlowGraph>::edge_descriptor e,
       const ControlFlowGraph& g) {
  return e->second;
}

inline std::pair<
  boost::graph_traits<ControlFlowGraph>::vertex_iterator,
  boost::graph_traits<ControlFlowGraph>::vertex_iterator>
vertices(const ControlFlowGraph &g) {
  return std::make_pair(g.vbegin(), g.vend());
}

inline std::pair<
  boost::graph_traits<ControlFlowGraph>::edge_iterator,
  boost::graph_traits<ControlFlowGraph>::edge_iterator>
edges(const ControlFlowGraph &g) {
  return std::make_pair(g.ebegin(), g.eend());
}

inline std::pair<
  boost::graph_traits<ControlFlowGraph>::out_edge_iterator,
  boost::graph_traits<ControlFlowGraph>::out_edge_iterator>
in_edges(
  boost::graph_traits<ControlFlowGraph>::vertex_descriptor u,
  const ControlFlowGraph &g) {
  return std::make_pair(u->ibegin(), u->iend());
}

inline std::pair<
  boost::graph_traits<ControlFlowGraph>::out_edge_iterator,
  boost::graph_traits<ControlFlowGraph>::out_edge_iterator>
out_edges(
  boost::graph_traits<ControlFlowGraph>::vertex_descriptor u,
  const ControlFlowGraph &g) {
  return std::make_pair(u->obegin(), u->oend());
}

inline std::pair<
  boost::graph_traits<ControlFlowGraph>::adjacency_iterator,
  boost::graph_traits<ControlFlowGraph>::adjacency_iterator>
adjacent_vertices(
  boost::graph_traits<ControlFlowGraph>::vertex_descriptor u,
  const ControlFlowGraph &g) {
  return std::make_pair(u->abegin(g), u->aend(g));
}

inline std::pair<
  boost::graph_traits<ControlFlowGraph>::inv_adjacency_iterator,
  boost::graph_traits<ControlFlowGraph>::inv_adjacency_iterator>
inv_adjacent_vertices(
  boost::graph_traits<ControlFlowGraph>::vertex_descriptor u,
  const ControlFlowGraph &g) {
  return std::make_pair(u->iabegin(g), u->iaend(g));
}

inline std::pair<
  boost::graph_traits<ControlFlowGraph>::edge_descriptor, bool>
edge(boost::graph_traits<ControlFlowGraph>::vertex_descriptor a,
     boost::graph_traits<ControlFlowGraph>::vertex_descriptor b,
     const ControlFlowGraph &g) {
  boost::graph_traits<ControlFlowGraph>::edge_descriptor e = a->find_to(b);
  return std::make_pair(e, e != 0);
}

inline void add_edge(boost::graph_traits<ControlFlowGraph>::vertex_descriptor a,
                     boost::graph_traits<ControlFlowGraph>::vertex_descriptor b,
                     ControlFlowGraph &g) {
  g.add_edge(a, b);
}

inline int in_degree(boost::graph_traits<ControlFlowGraph>::vertex_descriptor a,
                     const ControlFlowGraph &g) {
  return a->in_size();
}

inline int out_degree(boost::graph_traits<ControlFlowGraph>::vertex_descriptor a,
                      const ControlFlowGraph &g) {
  return a->out_size();
}

template <typename P>
inline P get(P p, ControlFlowGraph &g) {
  return p;
}

inline boost::default_color_type get(
  boost::vertex_color_t c,
  boost::graph_traits<ControlFlowGraph>::vertex_descriptor a) {
  return a->get_color();
}

inline void put(boost::vertex_color_t c,
                boost::graph_traits<ControlFlowGraph>::vertex_descriptor a,
                boost::default_color_type value) {
  a->set_color(value);
}

///////////////////////////////////////////////////////////////////////////////

class ControlFlowGraphWalker : public FunctionWalker {
public:
  explicit ControlFlowGraphWalker(ControlFlowGraph *g)
    : m_block(0)
    , m_graph(*g)
  {}
  template <class T>
  void walk(T &t) {
    std::pair<ControlFlowGraph::vertex_iterator,
      ControlFlowGraph::vertex_iterator> v(vertices(m_graph));
    while (v.first != v.second) {
      ControlBlock *b = *v.first;
      m_block = b;
      AstWalkerStateVec s = b->getStartState();
      beforeBlock(b);
      AstWalker::walk(t, s, b->getEndBefore(), b->getEndAfter());
      afterBlock(b);
      ++v.first;
    }
  }
  virtual void beforeBlock(ControlBlock *b) {}
  virtual void afterBlock(ControlBlock *b) {}
protected:
  ControlBlock *m_block;
  ControlFlowGraph &m_graph;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_CONTROL_FLOW_H_
