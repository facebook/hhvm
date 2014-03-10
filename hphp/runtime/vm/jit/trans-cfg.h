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

#ifndef incl_HPHP_TRANS_CFG_H_
#define incl_HPHP_TRANS_CFG_H_

#include <vector>

#include "hphp/runtime/vm/jit/srcdb.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

namespace HPHP {
namespace JIT {

/**
 * A dynamic control-flow graph of single-block translations.
 */
class TransCFG {
 public:
  class Arc {
   public:
    static const int64_t kUnknownWeight = -1;

    Arc(TransID src, TransID dst, int64_t w)
        : m_src(src)
        , m_dst(dst)
        , m_weight(w)
        , m_guessed(false)
      {}
    TransID src()          const { return m_src;     }
    TransID dst()          const { return m_dst;     }
    int64_t weight()       const { return m_weight;  }
    bool    guessed()      const { return m_guessed; }
    void    setWeight(int64_t w) { m_weight = w;     }
    void    setGuessed()         { m_guessed = true; }
   private:
    TransID m_src;
    TransID m_dst;
    int64_t m_weight;
    bool    m_guessed; // whether or not m_weight was guessed
  };

  typedef std::vector<Arc*>                      ArcPtrVec;
  typedef hphp_hash_set<Arc*, pointer_hash<Arc>> ArcPtrSet;

  class Node {
   public:
    Node(TransID id, int64_t w)
        : m_id(id)
        , m_weight(w)
      {}
    ~Node();

    TransID          transId()     const { return m_id;              }
    int64_t          weight()      const { return m_weight;          }
    const ArcPtrVec& inArcs()      const { return m_inArcs;          }
    const ArcPtrVec& outArcs()     const { return m_outArcs;         }
    void             addInArc (Arc* arc) { m_inArcs.push_back(arc);  }
    void             addOutArc(Arc* arc) { m_outArcs.push_back(arc); }
   private:
    TransID   m_id;
    int64_t   m_weight;
    ArcPtrVec m_inArcs;
    ArcPtrVec m_outArcs;
  };

  TransCFG() {}
  TransCFG(FuncId funcId,
           const ProfData* profData,
           const SrcDB& srcDB,
           const TcaTransIDMap& jmpToTransID);

  const std::vector<TransID>& nodes() const { return m_transIds; }
  ArcPtrVec              arcs()  const;
  int64_t                weight(TransID id) const;
  void                   setNodeWeight(TransID id, int64_t weight);
  const ArcPtrVec&       inArcs(TransID id) const;
  const ArcPtrVec&       outArcs(TransID id) const;
  void                   addNode(TransID id, int64_t weight);
  bool                   hasNode(TransID id) const;
  void                   addArc(TransID srcId, TransID dstId, int64_t weight=0);
  void                   print(std::string fileName,
                               FuncId funcId,
                               const ProfData* profData,
                               const TransIDSet* selected = nullptr) const;

 private:
  std::vector<TransID>                m_transIds;  // vector of TransIDs in the graph
  std::vector<Node>                   m_nodeInfo;  // info about each node
  hphp_hash_map<TransID, size_t> m_idToIdx;   // map from TransIDs to indices
                                              // in m_nodeInfo
};

} }

#endif
