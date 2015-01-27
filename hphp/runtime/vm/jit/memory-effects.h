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
 * Has the effects of possibly loading from one alias class, possibly storing
 * to another.  The instruction may also have other side-effects, and generally
 * can't be either removed or moved based on load-store analysis only.
 *
 * A note about instructions that can re-enter the VM:
 *
 *   If an instruction can re-enter, it can both read and write to the eval
 *   stack below some depth.  However, it can only legally read those slots if
 *   it writes them first, so we only need to include them in the may-store
 *   set, not the may-load set.  This is sufficient to ensure those stack
 *   locations don't have upward-exposed uses through a potentially re-entering
 *   instruction, and also to ensure we don't consider those locations
 *   available for load elimination after the instruction.
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
 * Spilling pre-live ActRecs are somewhat unusual, but effectively still just
 * pure stores.  They store to a range of stack slots, and don't store a PHP
 * value, so they get their own branch of the union.
 */
struct PureSpillFrame { AliasClass dst; };

/*
 * Iterator instructions are special enough that they just have their own
 * top-level memory effect type.  In general, they can both read and write to
 * the relevant locals, and can re-enter the VM and read and write anything on
 * the heap.  The `killed' set is an AliasClass that is killed by virtue of the
 * potential VM re-entry (i.e. the eval stack below some depth).
 */
struct IterEffects    { SSATmp* fp; uint32_t id; AliasClass killed; };
struct IterEffects2   { SSATmp* fp;
                        uint32_t id1;
                        uint32_t id2;
                        AliasClass killed; };

/*
 * Calls are somewhat special enough that they get a top-level effect.
 *
 * The `destroys_locals' flag indicates whether the call can change locals in
 * the calling frame (e.g. extract() or parse_str().)
 *
 * The `killed' set are locations that cannot be read by this instruction
 * unless it writes to them first, and which it generally may write to.  (This
 * is used for killing stack slots below the call depth.)
 *
 * The `stack' set contains stack locations the call will read as arguments, as
 * well as stack locations it may read or write via other means
 * (e.g. debug_backtrace, or pointers to stack slots to a CallBuiltin).
 * Locations in any intersection between `stack' and `killed' may be assumed to
 * be killed.
 */
struct CallEffects    { bool destroys_locals;
                        AliasClass killed;
                        AliasClass stack; };

/*
 * ReturnEffects is a real return from the php function.  It does not cover
 * things like suspending a resumable or "returning" from an inlined function.
 * But it may cover returning from a suspended resumable.
 *
 * All locals may be considered dead after ReturnEffects.  However, the stack
 * is a little more complicated.  The `killed' set is an additional alias class
 * of locations that cannot be read after the return, which is used to provide
 * the range of stack that can be considered dead.  In normal functions it will
 * effectively be AStackAny, but in generators a return may still leave part of
 * the eval stack alive for the caller.
 */
struct ReturnEffects  { AliasClass killed; };

/*
 * ExitEffects contains two sets of alias classes, representing locations that
 * are considered live exiting the region, and locations that will never be
 * read (unless written again) after exiting the region (`kill').  Various
 * instructions that exit regions populate these in different ways.
 *
 * If there is an overlap between `live' and `kill', then `kill' takes
 * precedence for locations that are contained in both (i.e. those locations
 * should be treated as actually killed).
 */
struct ExitEffects    { AliasClass live; AliasClass kill; };

/*
 * InterpOne instructions carry a bunch of information about how they may
 * affect locals.  It's special enough that we just pass it through.
 *
 * We don't make use of it in consumers of this module yet, except that the
 * `killed' set can't have upward exposed uses.
 */
struct InterpOneEffects { AliasClass killed; };

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
                                 , PureSpillFrame
                                 , KillFrameLocals
                                 , IterEffects
                                 , IterEffects2
                                 , CallEffects
                                 , ReturnEffects
                                 , ExitEffects
                                 , InterpOneEffects
                                 , IrrelevantEffects
                                 , UnknownEffects
                                 >;

//////////////////////////////////////////////////////////////////////

/*
 * Return information about the kinds of memory effects an instruction may
 * have.  See the above branches of MemEffects for more information.
 *
 * Important note: right now, some of the branches of MemEffects are relatively
 * specific (e.g. IterEffects) because of instructions that have odd shapes.
 * This may eventually go away, but for now this means it's very important that
 * users of this module be aware of potentially "suprising" effects of some of
 * those instructions when runtime flags like EnableArgsInBacktraces are set.
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
