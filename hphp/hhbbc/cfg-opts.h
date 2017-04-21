/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef incl_HHBBC_CFG_OPTS_H_
#define incl_HHBBC_CFG_OPTS_H_

namespace HPHP { namespace HHBBC {

struct Index;
struct FuncAnalysis;

//////////////////////////////////////////////////////////////////////

/*
 * Assist in removing blocks that aren't reachable by removing
 * conditional jumps that are never taken.  Conditional jumps that are
 * always taken are turned into unconditional jumps in first_pass.
 *
 * If options.RemoveDeadBlocks is off, this function just replaces
 * blocks we believe are unreachable with fatal opcodes.
 */
void remove_unreachable_blocks(const FuncAnalysis&);

/*
 * Simplify control flow, and create Switch and SSwitch bytecodes.
 */
bool control_flow_opts(const FuncAnalysis&);

/*
 * Simplify the exception tree.
 */
bool rebuild_exn_tree(const FuncAnalysis& ainfo);

//////////////////////////////////////////////////////////////////////

}}

#endif
