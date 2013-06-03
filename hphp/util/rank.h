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

#ifndef incl_HPHP_RANK_H_
#define incl_HPHP_RANK_H_

#include <limits.h>

namespace HPHP {

/*
 * The order in which we acquire blocking resources cannot produce
 * deadlock if paths respect a partial order on the resources.
 */

enum Rank {
  RankUnranked = -1, // Unranked locks can be inserted in any order.
  /*
   * Base rank locks are only ever acquired while no other locks are held.
   * In a wedding-cake diagram of the system, these are the upper layers of
   * frosting.
   */
  RankBase = 0,

  /*
   * Fbml is currently the lowest ranked lock because we reenter the VM while
   * holding it. This is probably a bad idea and is not intended to be
   * permanent.
   */
  RankFbml = RankBase,

  RankUnitInit,
  RankEvaledUnits,
  RankWriteLease,

  RankStatCache,

  RankFileRepo,
  RankStatCacheNode = RankFileRepo,
  RankFileMd5,

  RankInstanceCounts,
  RankInstanceBits = RankInstanceCounts,

  RankTreadmill,

  /*
   * Leaf-rank locks are the deepest resources in the system; once you've
   * acquired one, you can acquire no further resources without releasing
   * one.
   */
  RankLeaf
};

#ifdef DEBUG
extern Rank currentRank();
extern void checkRank(Rank r);
extern void pushRank(Rank r);
extern void popRank(Rank r);
extern void insertRank(Rank r);
#else
#define currentRank() RankBase
#define checkRank(r) do { } while(0)
#define pushRank(r) do { } while(0)
#define popRank(r) do { } while(0)
#define insertRank(r) do { } while(0)
#endif

}

#endif
