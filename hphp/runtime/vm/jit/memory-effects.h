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
 * General effect information used by most IR instructions.  The instruction
 * may load or store to different alias classes, as well as have any other
 * side-effects.
 *
 * Locations that contain PHP values are assumed (except for `moves', see
 * below) to be affected with normal VM semantics with regard to memory
 * barriers---that is, if the instruction loads from a `loads' location and
 * keeps a pointer to the value, it must IncRef it, and if the instruction
 * stores to a `stores' location, it must decref the old value that was in
 * memory (if there could be any) and incref the one it is storing.  The reason
 * to make this expectation clear is for the use of this module by refcount
 * optimizations.  Note that this means that instructions which do nothing but
 * store a php value to a location (without decreffing the old value that may
 * have been there) cannot use GeneralEffects to represent that (they must use
 * PureStore)---the same thing does not /quite/ apply to loads.
 *
 * Furthermore, all locations that contain values the instruction may decref
 * should be included in `stores', even if (as an "optimization") we don't need
 * to store anything over it.  This requirement basically only applies to
 * GenericRetDecRefs.
 *
 * The exception to the "normal VM semantics" is for values in the class
 * `moves', which must always be a subclass of `loads'.  This set of locations
 * may be loaded by the instruction, but in order to `transfer' them to another
 * memory location (which must be somewhere in its `stores' set), without an
 * IncRef.  Note: this set is also may-information---and it is required to
 * contain any location this instruction may do this to for correct analysis in
 * refcount optimizations (it must be conservatively "too big" rather than too
 * small).
 *
 * Additionally, there is a set `kills'.  For this set of locations, when the
 * instruction executes (before any of its other effects), it becomes
 * semantically illegal for any part of the program to read from those
 * locations again (without writing to them first).  This is used for the
 * ReturnHook to prevent uses of the stack and frame, and for killing stack
 * slots below the re-entry depth for potentially re-entering instructions.
 */
struct GeneralEffects   { AliasClass loads;
                          AliasClass stores;
                          AliasClass moves;
                          AliasClass kills; };

/*
 * The effect of definitely loading from an abstract location, without
 * performing any other work.  Instructions with these memory effects can be
 * removed and replaced with a Mov if we have a value available that's known to
 * be in the location.
 */
struct PureLoad       { AliasClass src; };

/*
 * The effect of definitely storing `value' to a location, without performing
 * any other work.  Instructions with these memory effects can be removed if we
 * know the value being stored does not change the value of the location, or if
 * we know the location can never be loaded from again.
 */
struct PureStore    { AliasClass dst; SSATmp* value; };

/*
 * Spilling pre-live ActRecs are somewhat unusual, but effectively still just
 * pure stores.  They store to a range of stack slots, and don't store a PHP
 * value, so they get their own branch of the union.
 *
 * The `stk' class is the entire stack range the instruction stores to, and the
 * `ctx' class is a subclass of `stk' that is the stack slot where it'll store
 * the context for the pre-live ActRec.
 *
 * The `stk' range should be interpreted as an exact AliasClass, not an upper
 * bound: it is guaranteed to be kNumActRecCells in size---no bigger than the
 * actual range of stack slots a SpillFrame instruction affects.
 */
struct PureSpillFrame { AliasClass stk; AliasClass ctx; };

/*
 * Calls are somewhat special enough that they get a top-level effect.
 *
 * The `destroys_locals' flag indicates whether the call can change locals in
 * the calling frame (e.g. extract() or parse_str(), when called with FCall).
 *
 * The `kills' set are locations that cannot be read by this instruction unless
 * it writes to them first, and which it generally may write to.  (This is used
 * for killing stack slots below the call depth.)
 *
 * The `stack' set contains stack locations the call will read as arguments, as
 * well as stack locations it may read or write via other means
 * (e.g. debug_backtrace, or pointers to stack slots to a CallBuiltin).
 * Locations in any intersection between `stack' and `kills' may be assumed to
 * be killed.
 *
 * Note that calls that have been weakened to CallBuiltin use GeneralEffects,
 * not CallEffects.
 */
struct CallEffects    { bool destroys_locals;
                        AliasClass kills;
                        AliasClass stack; };

/*
 * ReturnEffects is a return, either from the php function or an inlined
 * function.  You may assume if the instruction it came from is not an
 * InlineReturn, it is a return from the entire region.  It does not cover
 * suspending a resumable, but it covers returning from a suspended resumable.
 *
 * All locals on the returning frame may be considered dead after
 * ReturnEffects.  However, the stack is a little more complicated.  The
 * `kills' set is an additional alias class of locations that cannot be read
 * after the return, which is used to provide the range of stack that can be
 * considered dead.  In normal functions it will effectively be AStackAny, but
 * in generators a return may still leave part of the eval stack alive for the
 * caller.  When returning from an inlined function, the locals may all be
 * considered dead, and `kills' will contain the whole inlined function's
 * stack.
 */
struct ReturnEffects  { AliasClass kills; };

/*
 * ExitEffects contains two sets of alias classes, representing locations that
 * are considered live exiting the region, and locations that will never be
 * read (unless written again) after exiting the region (`kills').  Various
 * instructions that exit regions populate these in different ways.
 *
 * If there is an overlap between `live' and `kills', then `kills' takes
 * precedence for locations that are contained in both (i.e. those locations
 * should be treated as actually killed).
 */
struct ExitEffects    { AliasClass live; AliasClass kills; };

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

using MemEffects = boost::variant< GeneralEffects
                                 , PureLoad
                                 , PureStore
                                 , PureSpillFrame
                                 , CallEffects
                                 , ReturnEffects
                                 , ExitEffects
                                 , IrrelevantEffects
                                 , UnknownEffects
                                 >;

//////////////////////////////////////////////////////////////////////

/*
 * Return information about the kinds of memory effects an instruction may
 * have.  See the above branches of MemEffects for more information.
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

/*
 * Get the frame from a DefInlineFP.
 *
 * Returns: an (uncanonicalized, precise) AliasClass containing the stack slots
 * corresponding to the ActRec that is being converted from a pre-live to a
 * live ActRec by this instruction.
 *
 * Pre: inst->is(DefInlineFP)
 */
AliasClass inline_fp_frame(const IRInstruction* inst);

//////////////////////////////////////////////////////////////////////

}}

#endif
