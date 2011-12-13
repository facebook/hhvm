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

#include "asm-x64.h"
#include "util/trace.h"
#include "translator.h"

namespace HPHP {
namespace VM {
namespace Transl {

struct IncomingBranch {
  enum BranchType {
    JMP,
    JZ,
    JNZ
  };

  BranchType type;
  TCA src;
};

/*
 * RAII bookmark for temporarily rewinding a.code.frontier.
 */
class CodeCursor {
    typedef HPHP::x64::X64Assembler Asm;
    Asm& m_a;
    TCA m_oldFrontier;
  public:
    CodeCursor(Asm& a, TCA newFrontier) :
      m_a(a), m_oldFrontier(a.code.frontier) {
      m_a.code.frontier = newFrontier;
      TRACE_MOD(Trace::trans, 1, "RewindTo: %p (from %p)\n",
               m_a.code.frontier, m_oldFrontier);
    }
    ~CodeCursor() {
      m_a.code.frontier = m_oldFrontier;
      TRACE_MOD(Trace::trans, 1, "Restore: %p\n",
                m_a.code.frontier);
    }
};
/*
 * SrcRec: record of translator output for a given source location.
 */
struct SrcRec {
  typedef HPHP::x64::X64Assembler Asm;
  vector<TCA> translations;
  vector<IncomingBranch> incomingBranches;
  static const unsigned int kMaxTranslations = 4;
  SrcRec() { }

  TCA getFallbackTranslation() const;
  TCA getTopTranslation() const;
  void chainFrom(Asm& a, IncomingBranch::BranchType type, TCA src = NULL);
  void emitFallbackJump(Asm &a, IncomingBranch::BranchType type);
  void newTranslation(Asm& a, TCA newStart);

  private:
  void patch(Asm& a, IncomingBranch branch, TCA dest);
};

typedef hphp_hash_map<SrcKey, SrcRec, SrcKey> SrcDB;

} } }

#endif
