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
#ifndef _SRCDB_H_
#define _SRCDB_H_

#include <boost/noncopyable.hpp>

#include "util/asm-x64.h"
#include "util/trace.h"
#include "util/mutex.h"
#include "translator.h"
#include "runtime/vm/tread_hash_map.h"

namespace HPHP {
namespace VM {
namespace Transl {

struct IncomingBranch {
  enum BranchType {
    JMP,
    JCC,
    ADDR,
  };
  IncomingBranch(BranchType t, TCA src)
  : m_type(t), m_src(src) { }
  IncomingBranch(TCA src)
  : m_type(JMP), m_src(src) { }
  IncomingBranch(TCA* addr)
  : m_type(ADDR), m_addr(addr) { }

  BranchType m_type;
  union {
    TCA m_src;
    TCA* m_addr;
  };
};

/*
 * SrcRec: record of translator output for a given source location.
 */
struct SrcRec {
  typedef X64Assembler Asm;
  static const unsigned int kMaxTranslations = 12;

  SrcRec()
    : m_topTranslation(NULL)
    , m_anchorTranslation(0)
    , m_dbgBranchGuardSrc(NULL)
  {}

  /*
   * The top translation is our first target, a translation whose type
   * checks properly chain through all other translations. Usually this will
   * be the most recently created translation.
   *
   * This function can be safely called without holding the write
   * lease.
   */
  TCA getTopTranslation() const {
    return atomic_acquire_load(&m_topTranslation);
  }

  /*
   * The following functions are used during creation of new
   * translations or when inserting debug guards.  May only be called
   * when holding the translator write lease.
   */
  void setFuncInfo(const Func* f);
  void chainFrom(Asm& a, IncomingBranch br);
  void emitFallbackJump(Asm &a, TCA from, int cc = -1);
  void newTranslation(Asm& a, Asm &astubs, TCA newStart);
  void replaceOldTranslations(Asm& a, Asm& astubs);
  void addDebuggerGuard(Asm& a, Asm &astubs, TCA dbgGuard,
                        TCA m_dbgBranchGuardSrc);
  bool hasDebuggerGuard() const { return m_dbgBranchGuardSrc != NULL; }
  const MD5& unitMd5() const { return m_unitMd5; }

  const vector<TCA>& translations() const {
    return m_translations;
  }

  const vector<IncomingBranch>& incomingBranches() const {
    return m_incomingBranches;
  }

  void setAnchorTranslation(TCA anc) {
    ASSERT(!m_anchorTranslation);
    ASSERT(m_tailFallbackJumps.empty());
    m_anchorTranslation = anc;
  }

  const vector<IncomingBranch>& inProgressTailJumps() const {
    return m_inProgressTailJumps;
  }

  void clearInProgressTailJumps() {
    m_inProgressTailJumps.clear();
  }

private:
  TCA getFallbackTranslation() const;
  void patch(Asm* a, IncomingBranch branch, TCA dest);
  void patchIncomingBranches(Asm& a, Asm& astubs, TCA newStart);

private:
  // This either points to the most recent translation in the
  // translations vector, or if hasDebuggerGuard() it points to the
  // debug guard.  Read/write with atomic primitives only.
  TCA m_topTranslation;

  /*
   * The following members are all protected by the translator write
   * lease.  They can only be read when the lease is held.
   */

  // We chain new translations onto the end of the list, so we need to
  // track all the fallback jumps from the "tail" translation so we
  // can rewrire them to new ones.
  TCA m_anchorTranslation;
  vector<IncomingBranch> m_tailFallbackJumps;
  vector<IncomingBranch> m_inProgressTailJumps;

  vector<TCA> m_translations;
  vector<IncomingBranch> m_incomingBranches;
  MD5 m_unitMd5;
  // The branch src for the debug guard, if this has one.
  TCA m_dbgBranchGuardSrc;
};

/*
 * GrowableVector --
 *
 * We make a large number of these, and they typically only have one entry.
 * It's a shame to use a 24-byte std::vector for this.
 *
 * Only gets larger. Non-standard interface because we may realloc
 * at push_back() time.
 */
template<typename T>
struct GrowableVector {
  uint32_t m_size;
  T m_data[1]; // Actually variable length
  GrowableVector() : m_size(0) { }
  size_t size() const {
    return m_size;
  }
  T& operator[](const size_t idx) {
    ASSERT(idx < m_size);
    return m_data[idx];
  }
  const T& operator[](const size_t idx) const {
    ASSERT(idx < m_size);
    return m_data[idx];
  }
  GrowableVector* push_back(const T& datum) {
    GrowableVector* gv;
    // m_data always has room for at least one element due to the m_data[1]
    // declaration, so the realloc() code first has to kick in when a second
    // element is about to be pushed.
    if (Util::isPowerOfTwo(m_size)) {
      gv = (GrowableVector*)realloc(this,
                                    offsetof(GrowableVector<T>, m_data) +
                                    2 * m_size * sizeof(T));
    } else {
      gv = this;
    }
    gv->m_data[gv->m_size++] = datum;
    return gv;
  }
};

class SrcDB : boost::noncopyable {
  // SrcKeys that depend on a particular file go here.
  typedef hphp_hash_map<const Eval::PhpFile*,
          GrowableVector<SrcKey>*,
          pointer_hash<Eval::PhpFile> > FileDepMap;
  FileDepMap m_deps;

  // Although it seems tempting, in an experiment, trying to stash the
  // top TCA in place in the hashtable did worse than dereferencing a
  // SrcRec* to get it.  Maybe could be possible with a better hash
  // function or lower max load factor.  (See D450383.)
  typedef TreadHashMap<SrcKey::AtomicInt,SrcRec*,int64_hash> THM;
  THM m_map;
public:
  explicit SrcDB()
    : m_map(1024)
  {}

  typedef THM::iterator iterator;
  typedef THM::const_iterator const_iterator;

  iterator begin()             { return m_map.begin(); }
  iterator end()               { return m_map.end(); }
  const_iterator begin() const { return m_map.begin(); }
  const_iterator end()   const { return m_map.end(); }

  SrcRec* find(const SrcKey& sk) const {
    SrcRec* const* p = m_map.find(sk.toAtomicInt());
    return p ? *p : 0;
  }

  SrcRec* insert(const SrcKey& sk) {
    return *m_map.insert(sk.toAtomicInt(), new SrcRec);
  }

  inline void recordDependency(const Eval::PhpFile* file, const SrcKey& sk) {
    if (RuntimeOption::RepoAuthoritative) return;
    recordDependencyWork(file, sk);
  }
  void recordDependencyWork(const Eval::PhpFile* file, const SrcKey& sk);
  size_t invalidateCode(const Eval::PhpFile* file);
};

} } }

#endif
