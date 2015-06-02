/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_REGION_PRUNE_ARCS_H_
#define incl_HPHP_REGION_PRUNE_ARCS_H_

namespace HPHP { namespace jit {

struct RegionDesc;

//////////////////////////////////////////////////////////////////////

/*
 * Walk a RegionDesc using PostCondition information, and try to remove edges
 * that lead to blocks that will immediately fail guards.  Also, remove blocks
 * that are completely unreachable after these arcs are removed.
 */
void region_prune_arcs(RegionDesc&);

//////////////////////////////////////////////////////////////////////

}}


#endif
