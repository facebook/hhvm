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

#include <string>

#include <folly/Optional.h>

#include <boost/variant.hpp>

#include "hphp/runtime/vm/jit/alias-class.h"

namespace HPHP { namespace jit {

struct SSATmp;
struct IRInstruction;

//////////////////////////////////////////////////////////////////////

/*
 * Has the effect of possibly loading from a location, and possibly storing to
 * another.  Either may be empty locations, we don't know the value potentially
 * stored, and the instruction may have other non-pure side effects.
 */
struct MayLoadStore   { AliasClass loads; AliasClass stores; };

/*
 * The effect of definitely loading from an abstract location, without
 * performing any other work.  Instructions with these memory effects can be
 * removed and replaced with a Mov if we have a value available that's known to
 * be in the location.
 */
struct PureLoad       { AliasClass src; };

/*
 * The effect of definitely storing to a location, without performing any other
 * work.  Instructions with these memory effects can be removed if we know the
 * value being stored does not change the value of the location, or if we know
 * the location can never be loaded from again.
 *
 * The NT variation means it is not storing a type tag.
 */
struct PureStore    { AliasClass dst; SSATmp* value; };
struct PureStoreNT  { AliasClass dst; SSATmp* value; };

/*
 * Iterator instructions are special enough that they just have their own
 * top-level memory effect type.  In general, they can both read and write to
 * the relevant locals, and can read and write anything non-local.
 */
struct IterEffects    { SSATmp* fp; uint32_t id; };
struct IterEffects2   { SSATmp* fp; uint32_t id1; uint32_t id2; };

/*
 * Calls are somewhat special enough that they get a top-level effect.  The
 * destroys_locals flag indicates whether the call can change locals in the
 * calling frame (e.g. extract() or parse_str().)
 */
struct CallEffects    { bool destroys_locals; };

/*
 * ReturnEffects is a real return from the php function.  It does not cover
 * things like suspending a resumable or "returning" from an inlined function.
 */
struct ReturnEffects  {};

/*
 * InterpOne instructions carry a bunch of information about how they may
 * affect locals.  It's special enough that we just pass it through.
 */
struct InterpOneEffects {};

/*
 * An instruction that does KillFrameLocals prevents any upward exposed uses of
 * frame local slots, with no other memory effects.  This effect means that the
 * instruction semantically prevents any future loads from locals on that
 * frame.
 */
struct KillFrameLocals { SSATmp* fp; };

/*
 * "Irrelevant" effects means it doesn't do anything we currently care about
 * for consumers of this module.  If you want to care about a new kind of
 * memory effect, you get to re-audit everything---have fun. :)
 */
struct IrrelevantEffects {};

/*
 * Instruction hasn't been audited for effects that we care about.  Users of
 * this module should do the most pessimistic thing possible.
 */
struct UnknownEffects {};

using MemEffects = boost::variant< MayLoadStore
                                 , PureLoad
                                 , PureStore
                                 , PureStoreNT
                                 , KillFrameLocals
                                 , IterEffects
                                 , IterEffects2
                                 , CallEffects
                                 , ReturnEffects
                                 , InterpOneEffects
                                 , IrrelevantEffects
                                 , UnknownEffects
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
 * Replace any abstract locations in a MemEffects structure with their
 * canonical name (chasing passthrough instructions like canonical() from
 * analysis.h)
 */
MemEffects canonicalize(MemEffects);

/*
 * Produces a string about some MemEffects for debug-printing.
 */
std::string show(MemEffects);

//////////////////////////////////////////////////////////////////////

}}

#endif
