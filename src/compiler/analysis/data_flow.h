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

#ifndef __DATA_FLOW_H__
#define __DATA_FLOW_H__

#include <compiler/analysis/bit_set_vec.h>
#include <compiler/analysis/control_flow.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define DECLARE_DATA_FLOW(x)                    \
  x(Anticipated,0),                             \
    x(Altered,0),                               \
    x(Available,0),                             \
    x(AvailIn,1),                               \
    x(AvailOut,1),                              \
    x(AntIn,1),                                 \
    x(AntOut,1)

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
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __DATA_FLOW_H__
