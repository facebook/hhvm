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
#ifndef incl_HPHP_MEMORY_EFFECTS_H_
#define incl_HPHP_MEMORY_EFFECTS_H_

#include <boost/variant.hpp>

namespace HPHP { namespace jit {

struct SSATmp;
struct IRInstruction;

//////////////////////////////////////////////////////////////////////

/*
 * Has the effect of possibly reading a local slot, or two.
 */
struct ReadLocal      { SSATmp* fp; uint32_t id; };
struct ReadLocal2     { SSATmp* fp; uint32_t id1; uint32_t id2; };

/*
 * The effect of definitely storing to a local slot.  The NT variation means it
 * is not storing a type tag.
 */
struct StoreLocal     { SSATmp* fp; uint32_t id; };
struct StoreLocalNT   { SSATmp* fp; uint32_t id; };

/*
 * An instruction that does KillFrameLocals prevents any upward exposed uses of
 * frame local slots.  This is for example an effect of a return instruction.
 */
struct KillFrameLocals { SSATmp* fp; };

/*
 * An instruction that potentially reads any local.
 */
struct ReadAllLocals  {};

/*
 * "Irrelevant" effects means it doesn't do anything we currently care about
 * for consumers of this module.  If you want to care about a new kind of
 * memory effect, you get to re-audit everything---have fun. :)
 */
struct IrrelevantEffects {};

/*
 * Instruction hasn't been audited for effects that we care about.
 */
struct UnknownEffects {};

using MemEffects = boost::variant< UnknownEffects
                                 , IrrelevantEffects
                                 , ReadLocal
                                 , ReadLocal2
                                 , StoreLocal
                                 , StoreLocalNT
                                 , KillFrameLocals
                                 , ReadAllLocals
                                 >;

//////////////////////////////////////////////////////////////////////

/*
 * Return information about the kinds of memory effects an instruction may
 * have.
 *
 * This is used for eliminating redundant stores, so it can be conservative in
 * the direction of claiming more reads than there are, or fewer writes than
 * there are.  It must not erroneously claim that a memory location is being
 * redefined, and it must not fail to say a memory location is potentially
 * read.
 */
MemEffects memory_effects(const IRInstruction&);

/*
 * Produces a string about some MemEffects for debug-printing.
 */
const char* show(MemEffects);

//////////////////////////////////////////////////////////////////////

}}


#endif
