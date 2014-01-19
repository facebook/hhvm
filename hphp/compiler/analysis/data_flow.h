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

#ifndef incl_HPHP_DATA_FLOW_H_
#define incl_HPHP_DATA_FLOW_H_

#include "hphp/compiler/analysis/bit_set_vec.h"
#include "hphp/compiler/analysis/control_flow.h"

namespace HPHP {

DECLARE_BOOST_TYPES(ListAssignment);

///////////////////////////////////////////////////////////////////////////////

#define DECLARE_DATA_FLOW(x)                    \
  x(Anticipated,0),                             \
    x(Altered,0),                               \
    x(Available,0),                             \
    x(Referenced,0),                            \
    x(Inited,0),                                \
    x(Killed,0),                                \
    x(Object,0),                                \
    x(NotObject,0),                             \
    x(Dying,0),                                 \
    x(Used,0),                                  \
    x(AvailIn,0),                               \
    x(AvailOut,1),                              \
    x(AntIn,1),                                 \
    x(AntOut,0),                                \
    x(PAvailIn,0),                              \
    x(PAvailOut,0),                             \
    x(PRefIn,0),                                \
    x(PRefOut,0),                               \
    x(PInitIn,0),                               \
    x(PInitOut,0),                              \
    x(PObjIn,0),                                \
    x(PObjOut,0),                               \
    x(PAntIn,0),                                \
    x(PAntOut,0),                               \
    x(PDieIn,0),                                \
    x(PDieOut,0)

class DataFlow {
public:
  enum {
#define DECLARE_DF_ENUM(x,v) x
    DECLARE_DATA_FLOW(DECLARE_DF_ENUM),

    NumBVs
  };

  static const char *GetName(int i);
  static int GetInit(int i);
  typedef ControlFlowGraph::vertex_descriptor vertex_descriptor;
  typedef ControlFlowGraph::out_edge_iterator out_edge_iterator;
  typedef ControlFlowGraph::in_edge_iterator in_edge_iterator;

  static void ComputeAvailable(const ControlFlowGraph &g);
  static void ComputeAnticipated(const ControlFlowGraph &g);
  static void ComputePartialAvailable(const ControlFlowGraph &g);
  static void ComputePartialAnticipated(const ControlFlowGraph &g);
  static void ComputePartialReferenced(const ControlFlowGraph &g);
  static void ComputePartialNeeded(const ControlFlowGraph &g);
  static void ComputeUsed(const ControlFlowGraph &g);
  static void ComputePartialDying(const ControlFlowGraph &g);
  static void ComputePartialInited(const ControlFlowGraph &g);
private:
  template <typename T>
  static void ComputeForwards(T func, const ControlFlowGraph &g,
                              int lAttr, int altAttr,
                              int inAttr, int outAttr);
  template <typename T>
  static void ComputeBackwards(T func, const ControlFlowGraph &g,
                               int lAttr, int altAttr,
                               int inAttr, int outAttr);
};

class DataFlowWalker : public ControlFlowGraphWalker {
public:
  explicit DataFlowWalker(ControlFlowGraph *g) : ControlFlowGraphWalker(g) {}

  template<class T>
  void walk(T &t) { ControlFlowGraphWalker::walk(t); }

  int after(ConstructRawPtr cp);
  int afterEach(ConstructRawPtr cur, int i, ConstructRawPtr kid);
  void processAccessChain(ExpressionPtr e);
  void processAccessChainLA(ListAssignmentPtr la);
  void process(ExpressionPtr e, bool doAccessChains = false);

  virtual void processAccess(ExpressionPtr e) = 0;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // incl_HPHP_DATA_FLOW_H_
