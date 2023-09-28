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
#pragma once

#include <string>

#include <boost/variant.hpp>

#include "hphp/runtime/vm/jit/alias-class.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vm-reg-liveness.h"

namespace HPHP::jit {

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
 *
 * The set `inout' is used to track values that may be loaded or stored, but
 * which would otherwise pessimize the union for loads/stores. These are
 * primarily used for inout stack slots with CallBuiltin.
 *
 * `backtrace' represents the set of locals that may be accessed due to
 * backtracing. They are currently separated out from loads and by frame to
 * avoid pessimizing the AliasClass.
 *
 * If there is an overlap between `loads' and `kills', then `kills' takes
 * precedence for locations that are contained in both (i.e. those locations
 * should be treated as actually killed).

 */
struct GeneralEffects   { AliasClass loads;
                          AliasClass stores;
                          AliasClass moves;
                          AliasClass kills;
                          AliasClass inout;
                          jit::vector<AliasClass> backtrace; };

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
 * we know the location can never be loaded from again. `value' can be a
 * nullptr, in which case the store can still be elided if it is known to never
 * be loaded afterwards.
 *
 * dep is a "base" address that the store is relative to. This is used
 * so we can mark stores altered across its definition (which can only
 * happen in loops).
 */
struct PureStore    { AliasClass dst; SSATmp* value; SSATmp* dep; };

/*
 * The effect of simulating a Call to an inlined function. This effect is a pure
 * store of fp into the AFBasePtr location, base. The instruction also simulates
 * a load of fields of ActRec which may be read by the VM once fp is live (e.g.
 * the AFFunc and AFMeta locations).
 */
struct PureInlineCall { AliasClass base; SSATmp* fp; AliasClass actrec; };

/*
 * Calls are somewhat special enough that they get a top-level effect.
 *
 * The `kills' set are locations that cannot be read by this instruction unless
 * it writes to them first, and which it generally may write to.  (This is used
 * for killing stack slots below the call depth and MInstrState locations)
 *
 * The `uninits' set contains stack locations of inputs semantically containing
 * TUninit, but with the responsibility of the callee to store that TUninit.
 * Caller can treat these locations as killed.
 *
 * The `inputs' set contains stack locations the call will read as arguments.
 * In case of InlineSideExit, this is the complete frame.
 *
 * The `actrec' set contains stack locations the call will write ActRec to.
 *
 * The `outputs' set contains stack locations the call will write inout
 * variables to.
 *
 * The `locals` set contains frame locations that the call might read. There
 * is one AliasClass per frame, as we pessimize unions of ALocals of different
 * frames.
 *
 * Note that calls that have been weakened to CallBuiltin use GeneralEffects,
 * not CallEffects.
 */
struct CallEffects    { AliasClass kills;
                        AliasClass uninits;
                        AliasClass inputs;
                        AliasClass actrec;
                        AliasClass outputs;
                        jit::vector<AliasClass> backtrace; };

/*
 * ReturnEffects is a return from the top level php function, ending the entire
 * region. It does not cover suspending a resumable, but it covers returning
 * from a suspended resumable.
 *
 * All locals on the returning frame may be considered dead after
 * ReturnEffects.  However, the stack is a little more complicated.  The
 * `kills' set is an additional alias class of locations that cannot be read
 * after the return, which is used to provide the range of stack that can be
 * considered dead.  In normal functions it will effectively be AStackAny, but
 * in generators a return may still leave part of the eval stack alive for the
 * caller.
 */
struct ReturnEffects  { AliasClass kills; };

/*
 * ExitEffects contains two sets of alias classes, representing locations that
 * are considered live exiting the region, and locations that will never be
 * read (unless written again) after exiting the region (`kills' and 'uninits').
 * Various instructions that exit regions populate these in different ways.
 *
 * ExitEffects instructions require inlined frames to be spilled immediatelly
 * prior to the instruction, see spillInlinedFrames() for more details.
 */
struct ExitEffects    { AliasClass live;
                        AliasClass kills;
                        AliasClass uninits; };

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
                                 , PureInlineCall
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
MemEffects canonicalize(const MemEffects&);

/*
 * Return an alias class representing the pointee of the given value,
 * which must maybe be a TMem.
 */
AliasClass pointee(const SSATmp*);
AliasClass pointee(const Type&);

/*
 * Produces a string about some MemEffects for debug-printing.
 */
std::string show(const MemEffects&);

/*
 * Given a known VMReg liveness, computes a more precise GeneralEffects.
 *
 * For Dead VMRegState, the VMRegs may not be loaded.
 * For Live VMRegState, the VMRegs may not be stored.
 */
GeneralEffects general_effects_for_vmreg_liveness(
    const GeneralEffects& l, KnownRegState liveness);

//////////////////////////////////////////////////////////////////////

/*
 * Calculate more precise effects on the minstr base than just
 * memory-effects would give.
 *
 * An instruction is supported if hasMInstrBaseEffects returns true.
 *
 * Given the current base type and an instruction, return a new type
 * reflecting the changed base. If std::nullopt is returned, the
 * instruction will not modify the base.
 */
bool hasMInstrBaseEffects(const IRInstruction& inst);
Optional<Type> mInstrBaseEffects(const IRInstruction& inst, Type old);

template<class F>
void for_each_frame_location(SSATmp* fp, const Func* func, F fun) {
  fun(AActRec { fp }, "ActRec");
  fun(AMIStateAny, "MInstr State");

  if (auto const nlocals = func->numLocals()) {
    fun(ALocal { fp, AliasIdSet::IdRange(0, nlocals) }, "Local");
  }
  if (auto const niters = func->numIterators()) {
    fun(aiter_range(fp, 0, niters), "Iterator");
  }
}

///////////////////////////////////////////////////////////////////////////////

}
