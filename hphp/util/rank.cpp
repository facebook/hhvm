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

#include <stack>

#include "hphp/util/base.h"
#include "hphp/util/mutex.h"
#include "hphp/util/process.h"

namespace HPHP {
#ifdef DEBUG
static const int kMaxLockDepth=256;
static __thread Rank tl_rankStack[kMaxLockDepth];
static __thread int tl_curRankDepth;

Rank currentRank() {
  // Our current rank is the most recent ranked lock we've acquired.
  for (int i = tl_curRankDepth - 1; i >= 0; i--) {
    if (tl_rankStack[i] != RankUnranked) return tl_rankStack[i];
  }
  return RankBase;
}

void checkRank(Rank r) {
  if (tl_curRankDepth >= 1) {
    Rank prev = currentRank();
    /*
     * >= implies the ranks are a total order. If you want to allow
     * partial orders, make the constituent locks unranked.
     */
    if (prev >= r && r != RankUnranked) {
      if (r == RankLeaf) {
        fprintf(stderr,
                "Rank violation in thr%" PRIx64 "! leaf lock from leaf rank; ",
                Process::GetThreadIdForTrace());
      } else {
        fprintf(stderr, "Rank violation in thr%" PRIx64 "! lock of rank %d; ",
                Process::GetThreadIdForTrace(), r);
      }
      fprintf(stderr, "held locks:\n");
      for (int i = tl_curRankDepth - 1; i >= 0; --i) {
        fprintf(stderr, "%10d\n", tl_rankStack[i]);
      }
      assert(false);
    }
  }
}

void pushRank(Rank r) {
  checkRank(r);
  assert(tl_curRankDepth < kMaxLockDepth);
  tl_rankStack[tl_curRankDepth++] = r;
}

/*
 * Insert a rank that may be lower than the current rank in an
 * appropriate position in the stack. This should only be used after a
 * successful trylock of a lock with rank r.
 */
void insertRank(Rank r) {
  assert(r != RankUnranked);
  if (currentRank() <= r) {
    pushRank(r);
    return;
  }
  // Find the first real rank < r
  int i;
  for (i = tl_curRankDepth; i >= 0; i--) {
    if (tl_rankStack[i] < r && tl_rankStack[i] != RankUnranked) {
      break;
    }
  }
  memmove(&tl_rankStack[i+1], &tl_rankStack[i],
          sizeof(Rank) * (tl_curRankDepth - i));
  tl_rankStack[i] = r;
}

void popRank(Rank rank) {
  /*
   * We may safely release locks out of order; this can't disturb the
   * global order.
   */
  assert(tl_curRankDepth >= 1);
  for (int i = tl_curRankDepth; i >= 0; i--) {
    if (tl_rankStack[i] == rank) {
      memmove(&tl_rankStack[i], &tl_rankStack[i + 1],
              sizeof(Rank) * (tl_curRankDepth - i));
      tl_curRankDepth--;
      return;
    }
  }
  // If you hit this, you popped a rank that was never pushed.
  NOT_REACHED();
}
#endif

}
