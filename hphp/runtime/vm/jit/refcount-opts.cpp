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

//////////////////////////////////////////////////////////////////////

/*

Welcome to refcount-opts.  Theoretically reading this block comment first will
make the rest of this file make more sense.


-- Overview --

This file contains passes that attempt to reduce the number and strength of
reference counting operations in an IR program.  It uses a few strategies, but
fundamentally most of what's going on is about trying to prove that an IncRef
is post-dominated by a DecRef that provably can't go to zero, with no events in
between that can tell if the IncRef happened, and if so, removing the pair.

This doc comment is going to explain a few concepts, interleaved with
discussion on how they are used by the various analysis and optimization passes
in this module.


-- Must/May Alias Sets --

The passes in this file operate on groups of SSATmp's called "must-alias-set"s
(or often "asets" in the code).  These are sets of SSATmp names that are known
to alias the same object, in a "semi"-flow-insensitive way (see below).  Every
SSATmp that may have a reference counted type is assigned to a must-alias-set.
Crucially, if two SSATmps belong to different must-alias-sets, they still /may/
point to the same object.  For SSATmps a and b, in this module we use
(a&b).maybe(Counted) as a flow-insensitive May-Alias(a,b) predicate: if two
tmps may alias but only in a way that is not reference counted, we don't care
for our purposes.

A subtle point worth considering is that it is possible (and not uncommon) that
some of the SSATmps in one must-alias-set May-Alias some but not all of the
SSATmps in another must-alias-set: the reason is that the must-alias-sets are
still subject to SSA rules about where tmps are defined, and some of the tmps
in a set may be defined by instructions that take conditional jumps if the
object doesn't satisfy some condition (e.g. CheckType).  This is why it may
make sense to think of the must-alias-sets as "semi"-flow-insensitive: it's
globally true that the names all refer to the same object, but the names
involved aren't globally defined.

The first thing this module does is run a function to map SSATmps to their
must-alias-sets, and then, for each must-alias-set S, compute which of the
other must-alias-sets contain any tmps that May-Alias any tmp from S.


-- Weakening DecRefs --

This file contains a relatively cheap pass that can weaken DecRefs into
DecRefNZ by proving that they can't go to zero (unless there is already a bug
in the program).

The way this works is to do a backward dataflow analysis, computing
"will_be_used_again" information.  This dataflow analysis has a boolean for
each must-alias-set, indicating whether all paths from a program point contain
a use of that object in a way that implies their reference count is not zero
(for example, if every path decrefs it again).  Then, it converts any DecRef
instruction on tmps whose must-alias-sets say they "will_be_used_again" to
DecRefNZ.

One rule this pass relies on is that it is illegal to DecRef an object in a way
that takes its refcount to zero, and then IncRef it again after that.  This is
not illegal for trivial reasons, because object __destruct methods can
ressurect an object in PHP.  But within one JIT region, right now we declare it
illegal to generate IR that uses an object after a DecRef that might take it to
zero.

Since this pass converts instructions that may (in general) re-enter the
VM---running arbitrary PHP code for a destructor---it's potentially profitable
to run it earlier than other parts of refcount opts.  For example, it can allow
heap memory accesses to be proven redundant that otherwise would not be, and
can prevent the rest of this analysis from assuming some DecRefs can re-enter
that actually can't.


-- RC Flowgraphs --

Other optimizations in this file are performed on "RC flowgraphs", which are an
abstract representation of only the effects of the IR program that matter for
the optimization, on a single must-alias-set at a time.  The RC graphs contain
explicit control flow nodes ("phi" nodes for joins, "sigma" nodes for splits),
as well as nodes for things like decref instructions, incref instructions, and
"req" nodes that indicate that the reference count of an object may be observed
at that point up to some level.  Nodes in an RC graph each come with a "lower
bound" on the reference count for the graph's must-alias-set at that program
point (more about lower bounds below)---these lower bounds are the lower bound
before that node in the flowgraph.  We build independent graphs for each
must-alias-set, and they do not need to contain specific nodes relating to
possible cross-set effects (based on May-Alias relationships)---that
information is available in these graphs through the "req" nodes and lower
bound information.

The graphs are constructed after first computing information that allows us to
process each must-alias-set independently.  Then they are processed one at a
time with a set of "legal transformation rules".  The rules are applied in a
single pass over the flowgraph, going forwards, but potentially backtracking
when certain rules apply, since they may enable more rules to apply to previous
nodes.  At this point it might help to go look at one or two of the
transformation rule examples below (e.g. rule_inc_dec_fold), but that
documentation is not duplicated here.

The intention is that these rules are smaller and easier to understand the
correctness of than trying to do these transformations without an explicit data
structure, but a disadvantage is that this pass needs to allocate a lot of
temporary information in these graphs.  The backtracking also seemed a bit
convoluted to do directly on the IR.  We may eventually change this to work
without the extra data structure, but that's how it works right now.

Most of the analysis code in this module is about computing the information we
need to build these flowgraphs, before we do the actual optimizations on them.
The rest of this doc-comment talks primarily about that analysis---see the
comments near the rule_* functions for more about the flowgraph optimizations
themselves, and the comments near the Node structure for a description of the
node types in these graphs.


-- RC "lower bounds" --

A lower bound on the reference count of a must-alias-set indicates a known
minimum for the value of its object's count field at that program point.  This
minimum value can be interpreted as a minimum value of the actual integer in
memory at each point, if the program were not modified by this pass.  A lower
bound is therefore always non-negative.

The first utility of this information is pretty obvious: if a DecRef
instruction is encountered when the lower bound of must-alias-set is greater
than one, that DecRef instruction can be converted to DecRefNZ, since it can't
possibly free the object.  (See the flowgraph rule_decnz.)  Knowledge of the
lower bound is also required for folding unobservable incref/decref pairs, and
generally this information is inspected by most of the things done as RC
flowgraph transformations.

The lower bound must be tracked conservatively to ensure that our
transformations are correct.  This means we can increase a lower bound only
when we see instructions that /must/ imply an increase in the object's count
field, but we must decrease a lower bound whenever we see instructions that
/may/ imply a decrease in the count.  It might clarify this a little to list
the reasons that a must-alias-set's lower bounds can be increased:

   o An explicit IncRef instruction in the instruction stream of a tmp in the
     must-alias-set.

   o Instructions that "produce references" (generally instructions that
     allocate new objects).

   o Some situations with loads from memory (described later).

A must-alias-set's lower bound can be decreased in many situations, including:

   o An explicit DecRef or DecRefNZ of an SSATmp that maps to this
     must-alias-set.

   o In some situations, executing an instruction that could decref pointers
     that live in memory, for example by re-entering and running arbitrary php
     code.  (Memory is discussed more shortly; this concept is called "memory
     support".)

   o An SSATmp in this must-alias-set is passed as an argument to an IR
     instruction that may decref it.  (This is the "consumes reference"
     IRInstruction flag.)

   o We see an instruction that may reduce the lower bound of a different
     must-alias-set, for any reason, and that different set May-Alias this set.

If the last point were the exact rule we used, it would potentially mean lots
of reductions in lower bounds, which could be very pessimistic, so to obviate
the need to do it all the time we introduce an "exclusivity" principle on
tracking lower bounds.  What this principle means is the following: if we see
some reason in the IR to increment the lower bound in an alias set A, we can
/only/ increment the lower bound of A, even if that same information could also
be used to increase the lower bound of other asets.  If we could theoretically
use the same information to increase the lower bound of a different set B, we
can't do that at the same time---we have to choose one to apply it to.  (This
situation comes up with loads and is discussed further in "About Loads".)

This exclusivity principle provides the following rule for dealing with
instructions that may decrease reference counts because of May-Alias
relationships: when we need to decrease the lower bound of a must-alias-set, if
its lower bound is currently non-zero, we have no obligation to decrement the
lower bound in any other must-alias-set, regardless of May-Alias relationships.
The exclusivity of the lower bound means we know we're just cancelling out
something that raised the lower bound on this set and no other, so the state on
other sets can't be affected.

The pessimistic case still applies, however, if you need to reduce the lower
bound on a must-alias-set S that currently has a lower bound of zero.  Then all
the other sets that May-Alias S must have their lower bound reduced as well.


-- Memory Support --

This section is going to introduce the "memory support" concept, but details
will be fleshed out further in following sections.

The key idea behind this concept is that we can keep lower bounds higher than
we would otherwise be able to by tracking at least some of the pointers to the
object that may be in memory.  An alternative, conservative approach to stores,
for example, might be to eagerly attempt to reduce the lower bound on the must
alias set for value being stored at the location of the store itself.  By
instead keeping track of the fact that that memory location may contain a
pointer to that must-alias-set until it may be decref'd later, we can keep the
lower bound higher for longer.

The state of memory support for each must-alias-set is a bitvector of the
memory locations AliasAnalysis has identified in the program.  If a bit is set,
it indicates that that memory location may contain a pointer to that must alias
set.  When a must-alias-set has any memory support bits set, it is going to be
analyzed more conservatively than if it doesn't.  And importantly, the memory
support bits are may-information: just because a bit is set, doesn't mean that
we know for sure that memory location contains a pointer to this object.  It
just means that it might, and that our lower bound may have been "kept higher
for longer" using that knowledge at some point.

The primary thing we need to do more conservatively with must-alias-sets that
have memory support is reduce their lower bound if they could be decref'd
through that pointer in memory.  Since this effect on the analysis just reduces
lower bounds, it would never be incorrect to leave the memory support bit set
forever in this situation, which is also conceptually necessary for this to
work as may-information.

However, if we see an instruction that could DecRef one of these objects
through a pointer in memory and its lower_bound is currently non-zero, we can
be sure we've accounted for that may-DecRef by balancing it with a IncRef of
some sort that we've already observed.  In this situation, we can remove the
memory support bit to avoid futher reductions in the lower bound of that set
via that memory location.

Since this is may-information that makes analysis more conservative, the memory
support bits should conceptually be or'd at merge points.  It is fine to think
of it that way for general understanding of the analysis here, but in this
implementation we don't actually treat it that way when merging.  Because we
want to be able to quickly find memory-supported must-alias-sets from a given
memory location when analyzing memory effects of IR instructions (i.e. without
iterating every tracked alias set), we restrict the state to require that at
most one must-alias-set is supported by a given memory location during the
analysis.  If we reach situations that would break that restriction, we must
handle it conservatively (using a `pessimized' state, which is discussed some
later, as a last resort).  The merge_memory_support function elaborates on the
details of how this is done.

Another thing to note about memory support is that we may have more bits set
than the current lower bound for an object.  This situation can arise due to
conservatively reducing the lower bound, or due to pure stores happening before
IncRef instructions that raise the lower bound for that new pointer.

Most of the complexity in this analysis is related to instructions that load or
store from memory, and therefore interacts with memory support.  There are
enough details to discuss it futher in next several sections of this doc.


-- About Loads --

On entry to a region, it is assumed that normal VM reference count invariants
hold for all memory---specificaly, each reference count on each object in the
heap is exactly the number of live pointers to that object.  And in general,
accesses to memory must maintain this invariant when they are done outside of
small regions that may temporarily break that invariant.  We make use of this
fact to increase object lower bounds.

Accesses to memory within an IR compilation unit may be lowered into
instructions that separate reference count manipulation from stores and loads
(which is necessary for this pass to be able to optimize the former), so we
can't just assume loading a value from somewhere implies that there is a
reference count on the object, since our input program itself may have changed
that.  Furthermore, our input program may contain complex instructions other
than lowered stores that can "store over" memory locations we've already loaded
from, with a decref of the old value, and our analysis pass needs to reduce
lower bounds when we see those situations if we were using that memory location
to increase a lower bound on a loaded value.

To accomplish this, first this module performs a forward dataflow analysis to
compute program locations at which each memory location assigned an id by
AliasAnalysis are known to be "balanced" with regard to reference counting.
The gist of this is that if the last thing to manipulate a memory location must
have been code outside of this region, future loads from the memory location
define SSATmps that we know must have a lower bound of 1, corresponding to the
live pointer in that memory location.  However, when this analysis observes a
PureStore---i.e. a lowered, within-region store that does not imply reference
counting---a future load does not imply anything about the reference count,
because the program may have potentially written a pointer there that is not
yet "balanced" (we would need to see an IncRef or some other instruction
associated with the stored value to know that it has a reference).

Using the results of this analysis, we can add to the lower bound of some
must-alias-sets when we see loads from locations that are known to be balanced
at that program point.  When we do this, we must also track that the object has
a pointer in memory, which could cause a reduction in the lower bound later if
someone could decref it through that pointer, so the location must be added to
the memory support bitvector for that must-alias-set.  Whenever we see complex
instructions that may store to memory with the normal VM semantics of decrefing
old values, if they could overwrite locations that are currently "supporting"
one of our alias sets, we need to remove one from the alias set's lower bound
in case it decided to overwrite (and decref) the pointer that was in memory.

The "exclusivity" guarantee of our lower bounds requires that if we want to
raise the lower bound of an object because it was loaded from a memory location
known to be "balanced", then we only raise the lower bound for this reason on
at most one must-alias-set at a time.  This means if we see a load from a
location that is known to contain a balanced pointer, but we were already using
that location as memory support on a different set, we either need to remove
one from the lower bound of the other set before adding one to the new set, or
leave everything alone.  This commonly happens across php calls right now,
where values must be reloaded from memory because SSATmps can't span calls.

The way this pass currently handles this is the following: if we can reduce the
lower bound on the other set (because it has a non-zero lower bound), we'll
take the first choice, since the previously supported aset will probably not be
used again if we're spanning a call.  On the other hand, if the old set has a
lower bound of zero, so we can't compensate for removing it, we leave
everything alone.


-- Effects of Pure Stores on Memory Support --

There are two main kinds of stores from the perspective of this module.  There
are lowered stores (PureStore and PureSpillFrame) that happen within our IR
compilation unit, and don't imply reference count manipulation, and there are
stores that happen with "hhbc semantics" outside of the visibility of this
compilation unit, which imply decreffing the value that used to live in a
memory location as it's replaced with a new one.  This module needs some
understanding of both types, and both of these types of stores affect memory
support, but in different ways.

For any instruction that may do non-lowered stores outside of our unit ("stores
with hhbc semantics"), if the location(s) it may be storing to could be
supporting the lower bound in any must-alias-set, we should remove the support
and decrement the lower bound, because it could DecRef the value in order to
replace it with something else.  If we can't actually reduce the lower bound
(because it's already zero), we must leave the memory support flag alone,
because we haven't really accounted for the reference that lived in that memory
location, and it might not have actually been DecRef'd at that program point,
and could be DecRef'd later after we've seen another IncRef.  If we didn't
leave the bit alone in this situation, the lower bound could end up too high
after a later IncRef.

On the other hand, for a PureStore with a known destination, we don't need to
reduce the lower bound of any set that was supported by that location, since it
never implies a DecRef.  If the input IR program itself is trying to store to
that location "with hhbc semantics", then the program will also explicitly
contain the other lowered parts of this high level store, including any
appropriate loads and DecRefs of the old value, so we won't miss their effects.
So, for a PureStore we can simply mark the location as no-longer providing
memory support on the set it used to, but leave the lower bound alone.

The final case is a PureStore to an unknown location (either because it was not
supplied a AliasAnalysis id, or because it stored to something like a PtrToGen
that could refer to anything in memory).  In this situation, it may or may not
be overwriting a location we had been using for memory support---however, it's
harmless to leave that state alone, with the following rationale:

If it doesn't actually overwrite it, then obviously things are the same, and
we're good.  On the other hand, if it does actually overwrite it, then we don't
need to adjust the lower bound still, because it's a pure store (i.e. for the
same reason we didn't reduce the lower bound in the case above where we knew
where the store was going).  If we do nothing to our state, the only difference
from the known location, then, is that we may have "unnecessarily" left a
must-alias-set marked as getting memory support when it doesn't need to be
anymore.  But the point of marking part of a lower bound as coming from memory
support is just so that future stores (or loads) can potentially /reduce/ its
lower bound, so at worst it could reduce it later when it wouldn't really have
needed to if we had better information about where the store was going.  In
other words, it can be thought of as an optimization to clear the memory
support state when we see a PureStore with a known target location: it's not
required for correctness.


-- Effects of Pure Stores on the Must-Alias-Set Being Stored --

The other thing to take into account with stores is that they put a (possibly
new) pointer in memory, which means it now could be loaded and DecRef'd later,
possibly by code we can't directly see in our compilation unit.  To handle
this, we can divide things into four situations, based on two boolean
attributes: whether or not we have an AliasAnalysis bit for the location being
stored to ("known" vs "unknown" location), and whether or not the lower bound
on the must-alias-set for the value being stored is currently above zero.

The reason the lower bound matters when we see the store is the following:
we've possibly created a pointer in memory, which could be DecRef'd later, but
if the lower bound is zero we don't have a way to account for that, since it
can't go negative.  It's not ok to just ignore this.  Take the following
example, where t1 has a lower bound of zero:

   StMem ptr, t1
   IncRef t1
   IncRef t1
   RaiseWarning "something"  // any instruction that can re-enter and decref t1
   DecRef t1
   ...

If we simply ignored the fact that a new pointer has been created at the store,
that means the lower bound would be two after the two IncRefs, with no memory
support flags.  Then when we see the RaiseWarning, we won't know we need to
reduce the lower bound, since we didn't account for the store, and now we'll
think we can change the DecRef to DecRefNZ, but this is not actually a sound
transformation.

If the input program is not malformed, it will in fact be doing a 'balancing'
IncRef for any new pointers it creates, before anything could access it---in
fact it may have done that before the store, but our analysis in general
could've lost that information in the tracked lower bound because of a
May-Alias decref, or because it was done through an SSATmp that is mapped to a
different must-alias-set that actually is the same object (although we don't
know).

With this in mind, we'll discuss all four cases:

  Unknown target, Zero LB:

     We flag all must-alias-sets as "pessimized".  This state inserts a Halt
     node in each of the RC flowgraphs, and stops all optimizations along that
     control flow path: it prevents us from doing anything else in any
     successor blocks.

  Known target, Zero LB:

     Unlike the above, this case is not that uncommon.  Since we know where it
     is going, we don't have to give up on everything.  Instead, we leave the
     lower bound at zero, but set a memory support bit for the new location.
     Recall that we can in general have more memory support locations for one
     aset than the tracked lower bound---this is one situation that can cause
     that.

  Unknown target, Non-Zero LB:

     We don't know where the store is going, but we can account for balancing
     the possibly-new pointer.  In this case, we decrement the lower bound and
     just eagerly behave as if the must-alias-set for the stored value may be
     decref'd right there.  Since the lower bound is non-zero, we don't need to
     worry about changing lower bounds in other sets that May-Alias this one,
     because of the "exclusivity" rule for lower bounds.

  Known target, Non-Zero LB:

     Since we know where the new pointer will be, similar to the second case,
     we don't need to reduce the lower bound yet---we can wait until we see an
     instruction that might decref our object through that pointer.  In this
     case, we can just mark the target location as memory support for the
     must-alias-set for the stored value, and leave its lower bound alone.


-- More about Memory --

Another consideration about memory in this module arises from the fact that our
analysis passes make no attempt to track which object pointers may be escaped.
For that matter, much of the optimization we currently do here is removing
redundant reference counting of locals and eval stack slots, which arises from
lowering the HHBC stack machine semantics to HHIR---generally speaking these
values could be accessible through the heap as far as we know.  This is
important because it means that we can make no transformations to the program
that would affect the behavior of increfs or decrefs in memory locations we
aren't tracking, on the off chance they happen to contain a pointer to one of
our tracked objects.

The way we maintain correctness here is to never move or eliminate reference
counting operations unless we know about at least /two/ references to the
object being counted.  The importance of this is easiest to illustrate with
delayed increfs (relevant to rules inc_pass_req, inc_pass_phi, and
inc_pass_sig), although it applies to inc/dec pair removal also: it is fine to
move an incref forward in the IR instruction stream, as long as nothing could
observe the difference between the reference count the object "should" have,
and the one it will have after we delay the incref.  We need to consider how
reachability from the heap can affect this.

If the lower bound at an incref instruction is two or greater, we know we can
push the incref down as much as we want (basically until we reach an exit from
the compilation unit, or until we reach something that may decref the object
and reduce the lower bound).  On the other hand, if the lower bound before the
incref is zero, in order to move the incref forward, we would need to stop at
any instruction that could decref /anything/ in any memory location, since
we're making the assumption that there may be other live pointers to the
object---if we were to push that incref forward, we could change whether other
pointers to the object are considered the last reference, and cause a decref to
free the object when it shouldn't.  (We could try to do this on the rc
flowgraphs, but at least in a trivial implementation it would lead to a much
larger number of flowgraph nodes, so instead we leave easy cases to a separate,
local, "remove_trivial_incdecs" pass and ignore hard cases.)

The above two cases are relatively straightforward.  The remaining case is when
the lower bound before an incref is one.  It turns out to be safe to sink in
this case, and it fits the idea that we "know about two references".  Whatever
caused the lower bound to be one before the incref will ensure that the
object's liveness is not affected---here's why:

There are two possibilities: the object is either reachable through at least
one unknown pointer, or it isn't.  If it isn't, then the safety of moving the
incref is relatively straight-forward: we'll be pushing the actual /second/
reference down, and it is safe to push it as long as we don't move it through
something that may decref it (or until we reach an exit from the compilation
unit).  For the other possibility, it is sufficient to consider only having one
unknown pointer: in this situation, we're pushing the actual /third/ reference
down, and if anything decrefs the object through the pointer we don't know
about, it will still know not to free it because we left the second reference
alone (whatever was causing our lower bound to be one), and therefore a decref
through this unknown pointer won't think it is removing the last reference.

Also worth discussing is that there are several runtime objects in the VM with
operations that have behavioral differences based on whether the reference
count is greater than one.  For instance, types like KindOfString and
KindOfArray do in place updates when they have a refcount of one, and KindOfRef
is treated "observably" as a php reference only if the refcount is greater than
one.  Making sure we don't change these situations is actually the same
condition as discussed above: by the above scheme for not changing whether
pointers we don't know about constitute the last counted reference to an
object, we are both preventing decrefs from going to zero when they shouldn't,
and modifications to objects from failing to COW when they should.

A fundamental meta-rule that arises out of all the above considerations for any
of the RC flowgraph transformation rules is that we cannot move (or remove)
increfs unless the lower bound on the incref node is at least one (meaning
after the incref we "know about two references").  Similarly, anything that
could reduce the lower bound must put a node in the RC flowgraph to update that
information (a Req{1} node usually) so we don't push increfs too far or remove
them when we shouldn't.


-- "Trivial" incdec removal pass --

This module also contains a local optimization that removes IncRef/DecRefNZ
pairs in a block that have no non-"pure" memory-accessing instructions in
between them.

This optimization can be performed without regard to the lower bound of any
objects involved, and the DecRef -> DecRefNZ transformations the rest of the
code makes can create situations where these opportunities are visible.  Some
of these situations would be removable by the main pass if we had a more
complicated scheme for dealing with "unknown heap pointers" (i.e. the stuff in
the "more about memory" section described above).  But other situations may
also occur because the main pass may create unnecessary Req nodes in the middle
of code sequences that don't really observe references when we're dealing with
unrelated PureStores of possibly-aliasing tmps that have lower bounds of zero.

In general it is a simple pass to reason about the correctness of, and it
cleans up some things we can miss, so it is easier to do some of the work this
way than to complicate the main pass further.

*/

//////////////////////////////////////////////////////////////////////

#include "hphp/runtime/vm/jit/opt.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <limits>
#include <sstream>
#include <array>
#include <tuple>

#include <folly/Format.h>
#include <folly/ScopeGuard.h>
#include <folly/Conv.h>

#include <boost/dynamic_bitset.hpp>

#include "hphp/util/safe-cast.h"
#include "hphp/util/dataflow-worklist.h"
#include "hphp/util/match.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/pass-tracer.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/memory-effects.h"
#include "hphp/runtime/vm/jit/alias-analysis.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

namespace {

TRACE_SET_MOD(hhir_refcount);

//////////////////////////////////////////////////////////////////////

/*
 * Id's of must-alias-sets.  We use -1 as an invalid id.
 */
using ASetID = int32_t;

struct MustAliasSet {
  explicit MustAliasSet(Type widestType, SSATmp* representative)
    : widestType(widestType)
    , representative(representative)
  {}

  /*
   * Widest type for this MustAliasSet, used for computing may-alias
   * information.
   *
   * Because of how we build MustAliasSets (essentially canonical(), or groups
   * of LdCtx instructions), it is guaranteed that this widestType includes all
   * possible values for the set.  However it is not the case that every tmp in
   * the set necessarily has a subtype of widestType, because of situations
   * that can occur with AssertType and interface types.  This does not affect
   * correctness, but it's worth being aware of.
   */
  Type widestType;

  /*
   * A representative of the set.  This is only used for debug tracing, and is
   * currently the first instruction (in an rpo traversal) that defined a tmp
   * in the must-alias-set.  (I.e. it'll be the canonical() tmp, or the first
   * LdCtx we saw.)
   */
  SSATmp* representative;

  /*
   * Set of ids of the other MustAliasSets that this set may alias, in a flow
   * insensitive way, and not including this set itself.  This is based only on
   * the type of the representative.  See the comments at the top of this file.
   */
  jit::flat_set<ASetID> may_alias;
};

//////////////////////////////////////////////////////////////////////

// Analysis results for memory locations known to contain balanced reference
// counts.  See populate_mrinfo.
struct MemRefAnalysis {
  struct BlockInfo {
    uint32_t rpoId;
    ALocBits avail_in;
    ALocBits avail_out;
    ALocBits kill;
    ALocBits gen;
  };

  explicit MemRefAnalysis(IRUnit& unit) : info(unit, BlockInfo{}) {}

  StateVector<Block,BlockInfo> info;
};

//////////////////////////////////////////////////////////////////////

// Per must-alias-set state information for rc_analyze.
struct ASetInfo {
  /*
   * A lower bound of the actual reference count of the object that this alias
   * set refers to.  See "RC lower bounds" in the documentation---there are
   * some subtleties here.
   */
  int32_t lower_bound{0};

  /*
   * Set of memory location ids that are being used to support the lower bound
   * of this object.  The purpose of this set is to reduce lower bounds when we
   * see memory events that might decref a pointer: this means it's never
   * incorrect to leave a bit set in memory_support conservatively, but there
   * are situations where we must set bits here or our analysis will be wrong.
   *
   * An important note is that the bits in memory_support can represent memory
   * locations that possibly alias (via ALocMeta::conflicts).  Setting only one
   * bit from the conflict set is sufficient when we know something must be in
   * memory in the set---any memory effects that can affect other may-aliasing
   * locations will still apply to all of them as needed.
   *
   * However, whenever we handle removing memory support, if you need to remove
   * one bit, you generally speaking are going to need to remove the support
   * for the whole conflict set.
   */
  ALocBits memory_support;

  /*
   * Sometimes we lose too much track of what's going on to do anything useful.
   * In this situation, all the sets get flagged as `pessimized', we don't do
   * anything to them anymore, and a Halt node is added to all graphs.
   *
   * Note: right now this state is per-ASetInfo, but we must pessimize
   * everything at once if we pessimize anything, because of how the analyzer
   * will lose track of aliasing effects.  (We will probably either change it to
   * be per-RCState later or fix the alias handling.)
   */
  bool pessimized{false};
};

// State structure for rc_analyze.
struct RCState {
  bool initialized{false};
  jit::vector<ASetInfo> asets;

  /*
   * MemRefAnalysis availability state.  This is just part of this struct for
   * convenience when stepping through RCAnalysis results.  It is used to know
   * when loads can provide memory support.
   */
  ALocBits avail;

  /*
   * Map from AliasClass ids to the must-alias-set that has it as
   * memory_support, if any do.  At most one ASet will be supported by any
   * location at a time, to fit the "exclusivity" condition on lower bounds.
   * The mapped value is -1 if no ASet is currently supported by that location.
   */
  std::array<ASetID,kMaxTrackedALocs> support_map;
};

// The analysis result structure for rc_analyze.  This structure gets fed into
// build_graphs to create our RC graphs.
struct RCAnalysis {
  struct BlockInfo {
    uint32_t rpoId;
    RCState state_in;
  };

  explicit RCAnalysis(IRUnit& unit) : info(unit, BlockInfo{}) {}

  StateVector<Block,BlockInfo> info;
};

//////////////////////////////////////////////////////////////////////

struct Env {
  explicit Env(IRUnit& unit)
    : unit(unit)
    , rpoBlocks(rpoSortCfg(unit))
    , idoms(findDominators(unit, rpoBlocks, numberBlocks(unit, rpoBlocks)))
    , ainfo(collect_aliases(unit, rpoBlocks))
    , mrinfo(unit)
    , asetMap(unit, -1)
  {}

  IRUnit& unit;
  BlockList rpoBlocks;
  IdomVector idoms;
  Arena arena;
  AliasAnalysis ainfo;
  MemRefAnalysis mrinfo;

  StateVector<SSATmp,ASetID> asetMap;  // -1 is invalid (not-Counted tmps)
  jit::vector<MustAliasSet> asets;
};

//////////////////////////////////////////////////////////////////////

/*
 * Nodes in the RC flowgraphs.
 */
enum class NT : uint8_t { Inc, Dec, Req, Phi, Sig, Halt, Empty };
struct Node {
  Node* next{nullptr};
  Node* prev{nullptr};  // unused for Phi nodes; as they may have >1 preds
  int32_t lower_bound{0};
  NT type;

  // Counter used by optimize pass to wait to visit Phis until after
  // non-backedge predecessors.
  int16_t visit_counter{0};

protected:
  explicit Node(NT type) : type(type) {}
  Node(const Node&) = default;
  Node& operator=(const Node&) = default;
};

/*
 * IncRef and DecRef{NZ,} nodes.
 */
struct NInc : Node {
  explicit NInc(IRInstruction* inst) : Node(NT::Inc), inst(inst) {}
  IRInstruction* inst;
};
struct NDec : Node {
  explicit NDec(IRInstruction* inst) : Node(NT::Dec), inst(inst) {}
  IRInstruction* inst;
};

/*
 * Control flow splits and joins.
 */
struct NPhi : Node {
  explicit NPhi(Block* block) : Node(NT::Phi), block(block) {}
  Block* block;
  Node** pred_list{0};
  uint32_t pred_list_cap{0};
  uint32_t pred_list_sz{0};
  uint32_t back_edge_preds{0};
};
struct NSig : Node {
  explicit NSig(Block* block) : Node(NT::Sig), block(block) {}
  Block* block;
  Node* taken{nullptr};
};

/*
 * Halt means to stop processing along this control flow path---something
 * during analysis had to pessimize and we can't continue.
 *
 * When we've pessimized a set, we also guarantee that all successors have a
 * lower_bound of zero, which will block all rcfg transformation rules from
 * applying, so it's actually not necessary to halt---it just prevents
 * processing parts of the graph unnecessarily.
 *
 * For the case of join points which were halted on one side, optimize_graph
 * will not process through the join because the visit_counter will never be
 * high enough.  In the case of back edges, it may process through the loop
 * unnecessarily, but it won't make any illegal transformations because the
 * lower_bound will be zero.
 */
struct NHalt : Node { explicit NHalt() : Node(NT::Halt) {} };

/*
 * Empty nodes are useful for building graphs, since not every node type can
 * have control flow edges, but it has no meaning later.
 */
struct NEmpty : Node { explicit NEmpty() : Node(NT::Empty) {} };

/*
 * Req nodes mean the reference count of the object may be observed, up to some
 * "level".  The level is a number we have to keep the lower_bound above to
 * avoid changing program behavior.  It will be INT32_MAX on exits from the
 * compilation unit.
 */
struct NReq : Node {
  explicit NReq(int32_t level) : Node(NT::Req), level(level) {}
  int32_t level;
};

#define X(Kind, kind)                               \
  UNUSED N##Kind* to_##kind(Node* n) {              \
    assertx(n->type == NT::Kind);                    \
    return static_cast<N##Kind*>(n);                \
  }                                                 \
  UNUSED const N##Kind* to_##kind(const Node* n) {  \
    return to_##kind(const_cast<Node*>(n));         \
  }

X(Inc, inc)
X(Dec, dec)
X(Req, req)
X(Phi, phi)
X(Sig, sig)
X(Halt, halt)
X(Empty, empty)

#undef X

//////////////////////////////////////////////////////////////////////

template<class Kill, class Gen>
void mrinfo_step_impl(Env& env,
                      const IRInstruction& inst,
                      Kill kill,
                      Gen gen) {
  auto do_store = [&] (AliasClass dst, SSATmp* value) {
    /*
     * Pure stores potentially (temporarily) break the heap's reference count
     * invariants on a memory location, but only if the value being stored is
     * possibly counted.
     */
    if (value->type().maybe(TCounted)) {
      kill(env.ainfo.may_alias(canonicalize(dst)));
    }
  };

  auto const effects = memory_effects(inst);
  match<void>(
    effects,
    [&] (IrrelevantEffects) {},
    [&] (ExitEffects)      {},
    [&] (ReturnEffects)    {},
    [&] (GeneralEffects)   {},
    [&] (UnknownEffects)   { kill(ALocBits{}.set()); },
    [&] (PureStore x)      { do_store(x.dst, x.value); },

    /*
     * Note that loads do not kill a location.  In fact, it's possible that the
     * IR program itself could cause a location to not be `balanced' using only
     * PureLoads.  (For example, it could load a local to decref it as part of
     * a return sequence.)
     *
     * It's safe not to add it to the kill set, though, because if the IR
     * program is destroying a memory location, it is already malformed if it
     * loads the location again and then uses it in a way that relies on the
     * pointer still being dereferenceable.  Moreover, in these situations,
     * even though the avail bit from mrinfo will be set on the second load, we
     * won't be able to remove support from the previous aset, and won't raise
     * the lower bound on the new loaded value.
     */
    [&] (PureLoad) {},

    /*
     * Since there's no semantically correct way to do PureLoads from the
     * locations in a PureSpillFrame unless something must have stored over
     * them again first, we don't need to kill anything here.
     */
    [&] (PureSpillFrame x) {},

    [&] (CallEffects x) {
      /*
       * Because PHP callees can side-exit (or for that matter throw from their
       * prologue), the program is ill-formed unless we have balanced reference
       * counting for all memory locations.  Even if the call has the
       * destroys_locals flag this is the case---after it destroys the locals
       * the new value will have a fully synchronized reference count.
       *
       * This may need modifications after we allow php values to span calls in
       * SSA registers.
       */
      gen(ALocBits{}.set());
    }
  );
}

// Helper for stepping after we've created a MemRefAnalysis.
void mrinfo_step(Env& env, const IRInstruction& inst, ALocBits& avail) {
  mrinfo_step_impl(
    env,
    inst,
    [&] (ALocBits kill) { avail &= ~kill; },
    [&] (ALocBits gen)  { avail |= gen; }
  );
}

/*
 * Perform an analysis to determine memory locations that are known to hold
 * "balanced" values with respect to reference counting.  This means the
 * location "owns" a reference in the normal sense---i.e. the count on the
 * object is at least one on account of the pointer in that location.
 *
 * Normal ("hhbc-semantics") operations on php values in memory all preserve
 * balanced reference counts (i.e. a pointer in memory corresponds to one value
 * in the count field of the pointee).  However, when we lower hhbc opcodes to
 * HHIR, some opcodes split up the reference counting operations from the
 * memory operations: when we observe a "pure store" instruction, therefore,
 * the location involved may no longer be "balanced" with regard to reference
 * counting.  See further discussion in the doc comment at the top of this
 * file.
 */
void populate_mrinfo(Env& env) {
  FTRACE(1, "populate_mrinfo ---------------------------------------\n");
  FTRACE(3, "locations:\n{}\n", show(env.ainfo));

  /*
   * 1. Compute block summaries.
   */
  for (auto& blk : env.rpoBlocks) {
    for (auto& inst : blk->instrs()) {
      mrinfo_step_impl(
        env,
        inst,
        [&] (ALocBits kill) {
          env.mrinfo.info[blk].kill |= kill;
          env.mrinfo.info[blk].gen &= ~kill;
        },
        [&] (ALocBits gen) {
          env.mrinfo.info[blk].gen |= gen;
          env.mrinfo.info[blk].kill &= ~gen;
        }
      );
    }
  }

  FTRACE(3, "summaries:\n{}\n",
    [&] () -> std::string {
      auto ret = std::string{};
      for (auto& blk : env.rpoBlocks) {
        folly::format(&ret, "  B{: <3}: {}\n"
                            "      : {}\n",
          blk->id(),
          show(env.mrinfo.info[blk].kill),
          show(env.mrinfo.info[blk].gen)
        );
      }
      return ret;
    }()
  );

  /*
   * 2. Find fixed point of avail_in:
   *
   *   avail_out = avail_in - kill + gen
   *   avail_in  = isect(pred) avail_out
   *
   * Locations that are marked "avail" mean they imply a non-zero lower bound
   * on the object they point to, if they contain a reference counted type, and
   * assuming they are actually legal to load from.
   */

  auto incompleteQ = dataflow_worklist<uint32_t>(env.rpoBlocks.size());
  for (auto rpoId = uint32_t{0}; rpoId < env.rpoBlocks.size(); ++rpoId) {
    env.mrinfo.info[env.rpoBlocks[rpoId]].rpoId = rpoId;
  }
  // avail_outs all default construct to zeros.
  // avail_in on the entry block (with no preds) will be set to all 1 below.
  incompleteQ.push(0);

  do {
    auto const blk = env.rpoBlocks[incompleteQ.pop()];
    auto& binfo = env.mrinfo.info[blk];

    binfo.avail_in.set();
    blk->forEachPred([&] (Block* pred) {
      binfo.avail_in &= env.mrinfo.info[pred].avail_out;
    });

    auto const old = binfo.avail_out;
    binfo.avail_out = (binfo.avail_in & ~binfo.kill) | binfo.gen;
    if (binfo.avail_out != old) {
      if (auto const t = blk->taken()) {
        incompleteQ.push(env.mrinfo.info[t].rpoId);
      }
      if (auto const n = blk->next()) {
        incompleteQ.push(env.mrinfo.info[n].rpoId);
      }
    }
  } while (!incompleteQ.empty());

  FTRACE(4, "fixed point:\n{}\n",
    [&] () -> std::string {
      auto ret = std::string{};
      for (auto& blk : env.rpoBlocks) {
        folly::format(&ret, "  B{: <3}: {}\n",
          blk->id(),
          show(env.mrinfo.info[blk].avail_in)
        );
      }
      return ret;
    }()
  );
}

//////////////////////////////////////////////////////////////////////

using HPHP::jit::show;
DEBUG_ONLY std::string show(const boost::dynamic_bitset<>& bs) {
  std::ostringstream out;
  out << bs;
  return out.str();
}

/*
 * This helper for weaken_decrefs reports uses of reference-counted values that
 * imply that their reference count cannot be zero (or it would be a bug).
 * This includes any use of an SSATmp that implies the pointer isn't already
 * freed.
 *
 * For now, it's limited to the reference counting operations on these values,
 * because other types of uses need will to be evaluated on a per-instruction
 * basis: we can't just check instruction srcs blindly to find these types of
 * uses, because in general a use of an SSATmp with a reference-counted pointer
 * type (like Obj, Arr, etc), implies only a use of the SSA-defined pointer
 * value (i.e. the pointer bits sitting in a virtual SSA register), not
 * necessarily of the value pointed to, which is what we care about here and
 * isn't represented in SSA.
 */
template<class Gen>
void weaken_decref_step(const Env& env, const IRInstruction& inst, Gen gen) {
  switch (inst.op()) {
  case DecRef:
  case DecRefNZ:
  case IncRef:
    {
      auto const asetID = env.asetMap[inst.src(0)];
      if (asetID != -1) gen(asetID);
    }
    break;
  default:
    break;
  }
}

/*
 * Backward pass that weakens DecRefs to DecRefNZ if they cannot go to zero
 * based on future use of the value that they are DecRefing.  See "Weakening
 * DecRefs" in the doc comment at the top of this file.
 */
void weaken_decrefs(Env& env) {
  FTRACE(2, "weaken_decrefs ----------------------------------------\n");
  auto const poBlocks = [&] {
    auto ret = env.rpoBlocks;
    std::reverse(begin(ret), end(ret));
    return ret;
  }();

  /*
   * 0. Initialize block state structures and put all blocks in the worklist.
   */
  auto incompleteQ = dataflow_worklist<uint32_t>(poBlocks.size());
  struct BlockInfo {
    BlockInfo() {}
    uint32_t poId;
    boost::dynamic_bitset<> in_used;
    boost::dynamic_bitset<> out_used;
    boost::dynamic_bitset<> gen;
  };
  StateVector<Block,BlockInfo> blockInfos(env.unit, BlockInfo{});
  for (auto poId = uint32_t{0}; poId < poBlocks.size(); ++poId) {
    auto const blk = poBlocks[poId];
    blockInfos[blk].out_used.resize(env.asets.size());
    blockInfos[blk].in_used.resize(env.asets.size());
    blockInfos[blk].gen.resize(env.asets.size());
    blockInfos[blk].poId = poId;
    incompleteQ.push(poId);
  }

  /*
   * 1. Compute a transfer function for each block.  Add must-alias-set ids to
   * the block's GEN set if it has a use in the block that implies its
   * reference count can't be zero on entry to the block.
   */
  for (auto& blk : poBlocks) {
    auto& binfo = blockInfos[blk];
    for (auto& inst : blk->instrs()) {
      weaken_decref_step(env, inst, [&] (ASetID id) {
        binfo.gen.set(id);
      });
    }
  }
  FTRACE(5, "summaries:\n{}\n",
    [&] () -> std::string {
      auto ret = std::string{};
      for (auto& blk : poBlocks) {
        folly::format(&ret, " B{: <3}: {}\n",
          blk->id(),
          show(blockInfos[blk].gen)
        );
      }
      return ret;
    }()
  );

  /*
   * 2. Compute fixed point on the out_used sets.
   *
   *   out_used = isect(succ) in_used
   *    in_used = out_used | gen
   *
   * Note out_used is empty if there are no successors.
   */
  auto old_in_buf = boost::dynamic_bitset<>{};
  old_in_buf.resize(env.asets.size());
  do {
    auto const blk = poBlocks[incompleteQ.pop()];
    auto& binfo = blockInfos[blk];

    // Update the output set.
    auto const next = blk->next();
    auto const taken = blk->taken();
    if (next || taken) {
      binfo.out_used.set();
      if (next)  binfo.out_used &= blockInfos[next].in_used;
      if (taken) binfo.out_used &= blockInfos[taken].in_used;
    }

    // Propagate it to the in set.
    old_in_buf = binfo.in_used;
    binfo.in_used = binfo.out_used;
    binfo.in_used |= binfo.gen;
    auto const changed = old_in_buf != binfo.in_used;

    // Schedule each predecessor if the input set changed.
    if (changed) {
      blk->forEachPred([&] (Block* pred) {
        incompleteQ.push(blockInfos[pred].poId);
      });
    }
  } while (!incompleteQ.empty());

  FTRACE(5, "fixed point:\n{}\n",
    [&] () -> std::string {
      auto ret = std::string{};
      for (auto& blk : poBlocks) {
        folly::format(&ret, "  B{: <3}: {}\n",
          blk->id(),
          show(blockInfos[blk].out_used)
        );
      }
      return ret;
    }()
  );

  /*
   * 3. Convert DecRefs to DecRefNZ when we've proven the tmp must be used
   * again.
   */
  auto will_be_used = boost::dynamic_bitset<>{};
  for (auto& blk : poBlocks) {
    FTRACE(4, "B{}:\n", blk->id());
    will_be_used = blockInfos[blk].out_used;
    for (auto it = blk->instrs().rbegin(); it != blk->instrs().rend(); ++it) {
      auto& inst = *it;
      FTRACE(4, "  {}\n", inst);

      if (inst.is(DecRef)) {
        auto const id = env.asetMap[inst.src(0)];
        if (id != -1 && will_be_used.test(id)) {
          FTRACE(2, "    ** weakening {} to DecRefNZ\n", inst);
          inst.setOpcode(DecRefNZ);
        }
      }

      weaken_decref_step(env, inst, [&] (ASetID id) {
        will_be_used.set(id);
      });
      FTRACE(5, "    {}\n", show(will_be_used));
    }
  }

  FTRACE(2, "\n");
}

//////////////////////////////////////////////////////////////////////

// Helper for removing instructions in the rest of this file---if a debugging
// mode is enabled, it will replace it with a debugging instruction if
// appropriate instead of removing it.
void remove_helper(IRInstruction* inst) {
  if (!RuntimeOption::EvalHHIRGenerateAsserts) {
    inst->convertToNop();
    return;
  }

  switch (inst->op()) {
  case IncRef:
  case DecRef:
  case DecRefNZ:
    inst->setOpcode(DbgAssertRefCount);
    break;
  default:
    always_assert_flog(
      false,
      "Unsupported remove_helper instruction: {}\n",
      *inst
    );
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Walk through each block, and remove nearby IncRef/DecRefNZ pairs that
 * operate on the same must-alias-set, if there are obviously no instructions
 * in between them that could read the reference count of that object.
 */
void remove_trivial_incdecs(Env& env) {
  FTRACE(2, "remove_trivial_incdecs ---------------------------------\n");
  auto incs = jit::vector<IRInstruction*>{};
  for (auto& blk : env.rpoBlocks) {
    incs.clear();

    for (auto& inst : *blk) {
      if (inst.is(IncRef)) {
        incs.push_back(&inst);
        continue;
      }

      if (inst.is(DecRefNZ)) {
        if (incs.empty()) continue;
        auto const setID = env.asetMap[inst.src(0)];
        auto const to_rm = [&] () -> IRInstruction* {
          for (auto it = begin(incs); it != end(incs); ++it) {
            auto const candidate = *it;
            if (env.asetMap[candidate->src(0)] == setID) {
              incs.erase(it);
              return candidate;
            }
          }
          // This DecRefNZ may rely on one of the IncRefs, since we aren't
          // handling may-alias stuff here.
          incs.clear();
          return nullptr;
        }();
        if (to_rm == nullptr) continue;

        FTRACE(3, "    ** trivial pair: {}, {}\n", *to_rm, inst);
        remove_helper(to_rm);
        remove_helper(&inst);
        continue;
      }


      auto const effects = memory_effects(inst);
      match<void>(
        effects,
        // Pure loads, stores, and IrrelevantEffects do not read or write any
        // object reference counts.
        [&] (PureLoad) {},
        [&] (PureStore) {},
        [&] (PureSpillFrame) {},
        [&] (IrrelevantEffects) {},

        // Everything else may.
        [&] (GeneralEffects)    { incs.clear(); },
        [&] (CallEffects)       { incs.clear(); },
        [&] (ReturnEffects)     { incs.clear(); },
        [&] (ExitEffects)       { incs.clear(); },
        [&] (UnknownEffects)    { incs.clear(); }
      );
    }
  }
}

//////////////////////////////////////////////////////////////////////

void find_alias_sets(Env& env) {
  FTRACE(2, "find_alias_sets --------------------------------------\n");

  auto frame_to_ctx = sparse_idptr_map<SSATmp,ASetID>(env.unit.numTmps());

  auto add = [&] (SSATmp* tmp) {
    if (!tmp->type().maybe(TCounted)) return;

    auto& id = env.asetMap[tmp];
    if (id != -1) return;

    /*
     * There's one MustAliasSet for each frame's context, no matter how many
     * times we see loads of it.  We take advantage of this in pure_load to
     * bump the lower bound to at least one (if they weren't all in one set,
     * we'd not be able to do that without violating exclusivity of lower
     * bounds).
     */
    if (tmp->inst()->is(LdCtx)) {
      assertx(tmp == canonical(tmp));

      auto const fp = tmp->inst()->src(0);
      if (frame_to_ctx.contains(fp)) {
        id = frame_to_ctx[fp];
      } else {
        id = env.asets.size();
        frame_to_ctx[fp] = id;
        assertx(canonical(tmp) == tmp);
        env.asets.push_back(MustAliasSet { tmp->type(), tmp });
      }

      FTRACE(2,  "  t{} -> {} ({})\n", tmp->id(), id, tmp->toString());
      return;
    }

    auto const canon = canonical(tmp);
    if (env.asetMap[canon] != -1) {
      id = env.asetMap[canon];
    } else {
      id = env.asets.size();
      env.asetMap[canon] = id;
      env.asets.push_back(MustAliasSet { canon->type(), canon });
    }

    FTRACE(2,  "  t{} -> {} ({})\n", tmp->id(), id, canon->toString());
  };

  for (auto& blk : env.rpoBlocks) {
    for (auto& inst : blk->instrs()) {
      for (auto src : inst.srcs()) add(src);
      for (auto dst : inst.dsts()) add(dst);
    }
  }

  auto const num_sets = env.asets.size();
  FTRACE(2, "   {} must alias sets\n", num_sets);

  // Populate the may-alias-sets for each must-alias-set.
  for (auto i = uint32_t{0}; i < num_sets; ++i) {
    for (auto j = i + 1; j < num_sets; ++j) {
      auto& ai = env.asets[i];
      auto& aj = env.asets[j];
      bool const may_alias =
        (ai.widestType & aj.widestType).maybe(TCounted);
      if (may_alias) {
        ai.may_alias.insert(j);
        aj.may_alias.insert(i);
      }
    }
  }

  FTRACE(3, "may-alias-info:\n{}\n",
    [&] () -> std::string {
      auto ret = std::string{};
      for (auto asetID = uint32_t{0}; asetID < num_sets; ++asetID) {
        folly::format(&ret, "  {: <2}:", asetID);
        for (auto other : env.asets[asetID].may_alias) {
          folly::format(&ret, " {: <2}", other);
        }
        ret.push_back('\n');
      }
      return ret;
    }()
  );
}

//////////////////////////////////////////////////////////////////////

DEBUG_ONLY bool check_state(const RCState& state) {
  for (auto asetID = uint32_t{0}; asetID < state.asets.size(); ++asetID) {
    auto& set = state.asets[asetID];

    // All reference count bounds are non-negative.
    always_assert(set.lower_bound >= 0);

    // If this set has support bits, then the reverse map in the state is
    // consistent with it.
    if (set.memory_support.any()) {
      for (auto id = uint32_t{0}; id < set.memory_support.size(); ++id) {
        if (!set.memory_support[id]) continue;
        always_assert(state.support_map[id] == asetID);
      }
    }

    if (set.pessimized) {
      always_assert(set.lower_bound == 0);
      always_assert(set.memory_support.none());
    }
  }

  // Check other direction on the support_map.
  for (auto id = uint32_t{0}; id < state.support_map.size(); ++id) {
    auto const asetID = state.support_map[id];
    if (asetID != -1) {
      always_assert_flog(
        state.asets[asetID].memory_support[id],
        "expected aset {} to have support in location {}",
        asetID,
        id
      );
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

RCState entry_rc_state(Env& env) {
  auto ret = RCState{};
  ret.initialized = true;
  ret.asets.resize(env.asets.size());
  ret.avail = env.mrinfo.info[env.rpoBlocks.front()].avail_in;
  ret.support_map.fill(-1);
  return ret;
}

bool pessimize_for_merge(ASetInfo& aset) {
  if (aset.pessimized) return false;
  aset.pessimized = true;
  aset.lower_bound = 0;
  aset.memory_support.reset();
  return true;
}

bool merge_into(ASetInfo& dst, const ASetInfo& src) {
  auto changed = false;

  // Catch any issues with this early, instead of waiting for the full check
  // function.
  assertx(src.lower_bound >= 0);
  assertx(dst.lower_bound >= 0);

  auto const new_lower_bound = std::min(dst.lower_bound, src.lower_bound);
  if (dst.lower_bound != new_lower_bound) {
    dst.lower_bound = new_lower_bound;
    changed = true;
  }

  auto const new_pessimized = dst.pessimized || src.pessimized;
  if (dst.pessimized != new_pessimized) {
    assertx(new_pessimized);
    DEBUG_ONLY auto pess_changed = pessimize_for_merge(dst);
    assertx(pess_changed);
    changed = true;
  }

  return changed;
}

bool merge_memory_support(RCState& dstState, const RCState& srcState) {
  auto changed = false;
  for (auto asetID = uint32_t{0}; asetID < dstState.asets.size(); ++asetID) {
    auto& dst = dstState.asets[asetID];
    auto& src = srcState.asets[asetID];
    /*
     * If both the src and dst sets have enough memory support for their lower
     * bound, merge memory support by keeping the intersection of support
     * locations, and dropping the lower bound to compensate for the other ones
     * (i.e. we're acting like it might have been decref'd right here through
     * any memory locations that aren't in the intersection).  Since they both
     * have enough lower bound for their support, we know we can account for
     * everything here.
     *
     * On the other hand, if one or both of them has more memory support bits
     * than lower bound, we just pessimize everything.
     *
     * We do all of this conservative merging to simplify things during each
     * block's analysis.  If we weren't merging this way, we would have to
     * union the incoming memory bits, which easily leads to situations where
     * more than one must alias set is supported by the same memory location.
     * But we want our state structures to have support_map pointing to at most
     * one must-alias-set for each location.
     */
    if (dst.lower_bound >= dst.memory_support.count() &&
        src.lower_bound >= src.memory_support.count()) {
      auto const new_memory_support = dst.memory_support & src.memory_support;
      if (dst.memory_support != new_memory_support) {
        auto const old_count = dst.memory_support.count();
        auto const new_count = new_memory_support.count();
        auto const delta     = old_count - new_count;
        assertx(delta > 0);

        dst.lower_bound -= delta;
        dst.memory_support = new_memory_support;
        changed = true;

        assertx(dst.lower_bound >= 0);
        assertx(dst.lower_bound >= dst.memory_support.count());
      }
      continue;
    }

    FTRACE(5, "     {} pessimizing during merge\n", asetID);
    for (auto other = uint32_t{0}; other < dstState.asets.size(); ++other) {
      if (pessimize_for_merge(dstState.asets[other])) {
        changed = true;
      }
    }
  }

  return changed;
}

bool merge_into(Env& env, RCState& dst, const RCState& src) {
  if (!dst.initialized) {
    dst = src;
    return true;
  }

  always_assert(dst.asets.size() == src.asets.size());

  // We'll reconstruct the support_map vector after merging each aset.
  dst.support_map.fill(-1);

  auto changed = false;

  // First merge memory support information.  This is a separate step, because
  // this merge can cause changes to other may-alias sets at each stage (it may
  // pessimize all the sets in some situations).
  if (merge_memory_support(dst, src)) changed = true;

  for (auto asetID = uint32_t{0}; asetID < dst.asets.size(); ++asetID) {
    if (merge_into(dst.asets[asetID], src.asets[asetID])) {
      changed = true;
    }

    auto const& mem = dst.asets[asetID].memory_support;
    if (mem.none()) continue;
    for (auto loc = uint32_t{0}; loc < mem.size(); ++loc) {
      if (!mem[loc]) continue;
      assertx(dst.support_map[loc] == -1);
      dst.support_map[loc] = asetID;
    }
  }

  assertx(check_state(dst));

  return changed;
}

//////////////////////////////////////////////////////////////////////

template<class Fn>
void for_aset(Env& env, RCState& state, SSATmp* tmp, Fn fn) {
  auto const asetID = env.asetMap[tmp];
  if (asetID == -1) { assertx(!tmp->type().maybe(TCounted)); return; }
  if (state.asets[asetID].pessimized) return;
  fn(asetID);
}

template<class Fn>
void for_each_bit(ALocBits bits, Fn fn) {
  for (auto i = uint32_t{0}; i < bits.size(); ++i) {
    if (!bits[i]) continue;
    fn(i);
  }
}

void reduce_lower_bound(Env& env, RCState& state, uint32_t asetID) {
  FTRACE(5, "      reduce_lower_bound {}\n", asetID);
  auto& aset = state.asets[asetID];
  aset.lower_bound = std::max(aset.lower_bound - 1, 0);
}

template<class NAdder>
void pessimize_one(Env& env, RCState& state, ASetID asetID, NAdder add_node) {
  auto& aset = state.asets[asetID];
  if (aset.pessimized) return;
  FTRACE(2, "      {} pessimized\n", asetID);
  aset.lower_bound = 0;
  if (aset.memory_support.any()) {
    for (auto id = uint32_t{0}; id < aset.memory_support.size(); ++id) {
      if (!aset.memory_support[id]) continue;
      state.support_map[id] = -1;
    }
    aset.memory_support.reset();
  }
  aset.pessimized = true;
  add_node(asetID, NHalt{});
}

template<class NAdder>
void pessimize_all(Env& env, RCState& state, NAdder add_node) {
  FTRACE(3, "    pessimize_all\n");
  for (auto asetID = uint32_t{0}; asetID < state.asets.size(); ++asetID) {
    pessimize_one(env, state, asetID, add_node);
  }
}

template<class NAdder>
void observe(Env& env, RCState& state, ASetID asetID, NAdder add_node) {
  auto constexpr level = 2;
  add_node(asetID, NReq{level});
  auto const diff = level - state.asets[asetID].lower_bound;
  if (diff <= 1) {
    /*
     * This would be a Req{1} in the may-alias sets, but there's always an
     * implicit Req{1} everywhere because we aren't allowed to change reference
     * counts unless we "know about two references" (see the "more about
     * memory" section in the docs).  The only time we need to explicitly put
     * Req{1} nodes into RCFGs is if we've reduced the lower bound, which we
     * aren't going to do here, so we can just leave it out.
     */
    return;
  }
  FTRACE(3, "    unbalanced observe (diff: {}):\n", diff);
  for (auto may_id : env.asets[asetID].may_alias) {
    add_node(may_id, NReq{diff});
  }
}

template<class NAdder>
void observe_all(Env& env, RCState& state, NAdder add_node) {
  FTRACE(3, "    observe_all\n");
  for (auto asetID = uint32_t{0}; asetID < state.asets.size(); ++asetID) {
    add_node(asetID, NReq{std::numeric_limits<int32_t>::max()});
  }
}

/*
 * When we call builtin functions, we need to make sure that we don't change
 * the return value of VRefParam::isReferenced on any possibly-KindOfRef
 * arguments.  We accomplish this with req nodes at level 2 for all asets that
 * could be boxed before we see builtin calls (we could do it only to the ones
 * that could be args, but we don't bother).
 *
 * The reason we have this unusual case only when dealing with builtin calls is
 * that in that situation, we're actually tracking references and memory
 * locations associated with the call.  This means it doesn't fall into the
 * usual category of not changing whether an "unknown pointer" could be the
 * last reference (as described in the "More about memory" section at the top
 * of this file)---we need to avoid changing whether a known pointer (the one
 * in memory for the CallBuiltin arg) is the last reference.  Basically,
 * CallBuiltin observes the reference count (at level 2) for their
 * possibly-boxed args, even though they can't decref the pointer through the
 * memory locations for those args.
 */
template<class NAdder>
void observe_for_is_referenced(Env& env, RCState& state, NAdder add_node) {
  FTRACE(3, "    observe_for_is_referenced\n");
  for (auto asetID = uint32_t{0}; asetID < state.asets.size(); ++asetID) {
    if (env.asets[asetID].widestType.maybe(TBoxedCell)) {
      add_node(asetID, NReq{2});
    }
  }
}

template<class NAdder>
void may_decref(Env& env, RCState& state, ASetID asetID, NAdder add_node) {
  auto& aset = state.asets[asetID];

  auto const old_lower_bound = aset.lower_bound;
  reduce_lower_bound(env, state, asetID);
  add_node(asetID, NReq{1});
  FTRACE(3, "    {} lb: {}\n", asetID, aset.lower_bound);

  if (old_lower_bound >= 1) return;
  FTRACE(4, "    unbalanced decref:\n");
  for (auto may_id : env.asets[asetID].may_alias) {
    reduce_lower_bound(env, state, may_id);
    add_node(may_id, NReq{1});
  }
}

// Returns true if we actually removed the support (i.e. we accounted for it by
// reducing a lower bound, or the location wasn't actually supporting anything
// right now).
template<class NAdder>
bool reduce_support_bit(Env& env,
                        RCState& state,
                        uint32_t locID,
                        NAdder add_node) {
  auto const current_set = state.support_map[locID];
  if (current_set == -1) return true;
  FTRACE(3, "      {} removing support\n", current_set);
  auto& aset = state.asets[current_set];
  if (aset.lower_bound == 0) {
    /*
     * We can't remove the support bit, and we have no way to account for the
     * reduction in lower bound.  There're two cases to consider:
     *
     *   o If the event we're processing actually DecRef'd this must-alias-set
     *     through this memory location, the lower bound is still zero, and
     *     leaving the bit set conservatively is not incorrect.
     *
     *   o If the event we're processing did not actually DecRef this object
     *     through this memory location, then we must not remove the bit,
     *     because something in the future (after we've seen other IncRefs)
     *     still may decref it through this location.
     *
     * So in this case we leave the lower bound alone, and also must not remove
     * the bit.
     */
    return false;
  }
  aset.memory_support.reset(locID);
  state.support_map[locID] = -1;
  reduce_lower_bound(env, state, current_set);
  // Because our old lower bound is non-zero (from the memory support), we
  // don't need to deal with the possibility of may-alias observes or may-alias
  // decrefs here.
  add_node(current_set, NReq{1});
  return true;
}

// Returns true if reduce_support_bit succeeded on every support bit in the
// set.
template<class NAdder>
bool reduce_support_bits(Env& env,
                         RCState& state,
                         ALocBits set,
                         NAdder add_node) {
  FTRACE(2, "    remove: {}\n", show(set));
  auto ret = true;
  for_each_bit(set, [&] (uint32_t locID) {
    if (!reduce_support_bit(env, state, locID, add_node)) ret = false;
  });
  return ret;
}

// Returns true if we completely accounted for removing the support.  If it
// returns false, the support bits may still be marked on some must-alias-sets.
// (See pure_load.)
template<class NAdder>
bool reduce_support(Env& env,
                    RCState& state,
                    AliasClass aclass,
                    NAdder add_node) {
  if (aclass == AEmpty) return true;
  FTRACE(3, "    reduce support {}\n", show(aclass));
  auto const alias = env.ainfo.may_alias(aclass);
  return reduce_support_bits(env, state, alias, add_node);
}

void drop_support_bit(Env& env, RCState& state, uint32_t bit) {
  auto const current_set = state.support_map[bit];
  if (current_set != -1) {
    FTRACE(3, "    {} dropping support {}\n", current_set, bit);
    state.asets[current_set].memory_support.reset(bit);
    state.support_map[bit] = -1;
  }
}

void drop_support_bits(Env& env, RCState& state, ALocBits bits) {
  FTRACE(3, "    drop support {}\n", show(bits));
  for_each_bit(bits, [&] (uint32_t bit) {
    drop_support_bit(env, state, bit);
  });
}

template<class NAdder>
void create_store_support(Env& env,
                          RCState& state,
                          AliasClass dst,
                          SSATmp* tmp,
                          NAdder add_node) {
  for_aset(env, state, tmp, [&] (ASetID asetID) {
    auto& aset = state.asets[asetID];

    auto const meta = env.ainfo.find(dst);
    if (!meta && aset.lower_bound == 0) {
      FTRACE(3, "    {} causing pessimize\n", asetID);
      pessimize_all(env, state, add_node);
      return;
    }

    if (meta) {
      FTRACE(3, "    {} adding support in {}\n", asetID, show(dst));

      /*
       * This exact bit can't be set anymore (because the caller of this
       * function cleared it via a call to drop_support_bit, if we had a meta).
       *
       * Since we're doing a store, we don't need to worry about whether other
       * bits from meta->conflicts are supporting something, even if one of
       * them is actually supporting this same set.  The reason to consider
       * whether it would matter is that we have to make sure we don't violate
       * the exclusivity rule of lower bounds: this won't happen because we're
       * not increasing our lower bound here, and we're only changing which
       * memory locations should cause reductions in the lower bound later.
       *
       * Basically if we assume the lower bound "exclusivity" property is
       * holding before this store operation, it will still hold after it:
       * we'll still clear the support if we do another load or store from
       * anything inside this conflict set, and memory operations that could
       * decref any one of them will reduce the support for all of them.
       */
      assertx(state.support_map[meta->index] == -1);

      state.support_map[meta->index] = asetID;
      aset.memory_support.set(meta->index);
      return;
    }

    FTRACE(3, "    {} treating store as may_decref\n", asetID);
    may_decref(env, state, asetID, add_node);
  });
}

//////////////////////////////////////////////////////////////////////

template<class NAdder>
void handle_call(Env& env,
                 RCState& state,
                 const IRInstruction& inst,
                 CallEffects e,
                 NAdder add_node) {
  // We have to block all incref motion through a PHP call, by observing at the
  // max.  This is fundamentally required because the callee can side-exit or
  // throw an exception without a catch trace, so everything needs to be
  // balanced.
  observe_all(env, state, add_node);

  // Figure out locations the call may cause stores to, then remove any memory
  // support on those locations.
  auto bset = ALocBits{};
  if (e.destroys_locals) bset |= env.ainfo.all_frame;
  bset |= env.ainfo.may_alias(e.stack);
  bset |= env.ainfo.may_alias(AHeapAny);
  bset &= ~env.ainfo.expand(e.kills);
  reduce_support_bits(env, state, bset, add_node);
}

template<class NAdder>
void pure_load(Env& env,
               RCState& state,
               AliasClass src,
               SSATmp* dst,
               NAdder add_node) {
  auto const meta = env.ainfo.find(src);
  if (!meta) return;
  if (!state.avail.test(meta->index)) return;

  /*
   * If currently supporting a different ASet, try to clear any support there
   * first.  It is possible we won't successfully clear the support if it was
   * supporting a set that currently has a lower bound of zero---in that
   * situation we can't raise the lower bound or support the new loaded set,
   * because it could violate the exclusivity rule.
   */
  FTRACE(2, "    load: {}\n", show(src));
  if (!reduce_support(env, state, src, add_node)) {
    FTRACE(2, "    couldn't remove all pre-existing support\n");
    return;
  }

  for_aset(env, state, dst, [&] (ASetID asetID) {
    FTRACE(2, "    {} adding support: {}\n", asetID, show(src));
    auto& aset = state.asets[asetID];
    ++aset.lower_bound;
    aset.memory_support.set(meta->index);
    state.support_map[meta->index] = asetID;
  });
}

template<class NAdder>
void pure_store(Env& env,
                RCState& state,
                AliasClass dst,
                SSATmp* tmp,
                NAdder add_node) {
  /*
   * First, handle the effects of the store on memory support.  See the docs
   * above in "Effects of Pure Stores on Memory Support" for an explanation.
   */
  if (auto const meta = env.ainfo.find(dst)) {
    drop_support_bit(env, state, meta->index);
  }

  /*
   * Now handle the effects of the store on the aset for the value being
   * stored.
   */
  create_store_support(env, state, dst, tmp, add_node);
}

template<class NAdder>
void pure_spill_frame(Env& env,
                      RCState& state,
                      PureSpillFrame psf,
                      SSATmp* ctx,
                      NAdder add_node) {
  /*
   * First, the effects of PureStores on memory support.  A SpillFrame will
   * store over kNumActRecCells stack slots, and just like normal PureStores we
   * can drop any support bits for them without reducing their lower bounds.
   */
  drop_support_bits(env, state, env.ainfo.expand(psf.stk));

  /*
   * Now the effects on the set being stored.  Pre-live frames are slightly
   * different from other pure stores, because eventually they may become live
   * frames even within our region (via DefInlineFP).  However, in the
   * meantime, we need to treat the store of the context like a normal
   * pure_store, because there are various IR instructions that can decref the
   * context on a pre-live ActRec through memory (e.g. LdObjMethod).
   *
   * If the frame becomes live via DefInlineFP, we don't need to treat it as
   * memory support for this set anymore, for the same reason that LdCtx
   * doesn't need that.  The only way that reference can be DecRef'd in a
   * semantically correct program is in a return sequence, and if it's done
   * inside this region, we will see the relevant DecRef instructions and
   * handle that.
   */
  create_store_support(env, state, psf.ctx, ctx, add_node);
}

//////////////////////////////////////////////////////////////////////

template<class NAdder>
void analyze_mem_effects(Env& env,
                         IRInstruction& inst,
                         RCState& state,
                         NAdder add_node) {
  auto const effects = canonicalize(memory_effects(inst));
  FTRACE(4, "    mem: {}\n", show(effects));
  match<void>(
    effects,
    [&] (IrrelevantEffects) {},

    [&] (GeneralEffects x)  {
      if (inst.is(CallBuiltin)) {
        observe_for_is_referenced(env, state, add_node);
      }

      // Locations that are killed don't need to be tracked as memory support
      // anymore, because nothing can load that pointer (and then decref it)
      // anymore without storing over the location first.
      drop_support_bits(env, state, env.ainfo.expand(x.kills));
      // Locations in the stores set may be stored to with a 'normal write
      // barrier', decreffing the pointer that used to be there.
      reduce_support(env, state, x.stores, add_node);
      // For the moves set, we have no way to track where the pointers may be
      // moved to, so we need to account for it, conservatively, as if it were
      // a decref right now.
      reduce_support(env, state, x.moves, add_node);
    },

    [&] (ReturnEffects)     { observe_all(env, state, add_node); },
    [&] (ExitEffects)       { observe_all(env, state, add_node); },

    [&] (UnknownEffects)    { pessimize_all(env, state, add_node); },

    [&] (CallEffects e) { handle_call(env, state, inst, e, add_node); },
    [&] (PureStore x)   { pure_store(env, state, x.dst, x.value, add_node); },
    [&] (PureLoad x)    { pure_load(env, state, x.src, inst.dst(), add_node); },

    [&] (PureSpillFrame x) {
      pure_spill_frame(env, state, x, inst.src(2), add_node);
    }
  );
}

/*
 * Return whether an IRInstruction may decref one of its sources, if its taken
 * edge is traversed.
 */
bool consumes_reference_taken(const IRInstruction& inst, uint32_t srcID) {
  switch (inst.op()) {
  // The following consume some arguments only in the event of an exception.
  case LookupClsMethod:
    return srcID == 1;
  case LdArrFuncCtx:
  case LdArrFPushCuf:
  case LdStrFPushCuf:
  case SuspendHookE:
  case ReturnHook:
    return srcID == 0;
  default:
    return false;
  }
}

/*
 * Return whether an IRInstruction may decref one of its sources, if its next
 * edge is traversed, but not if its taken edge is traversed.
 *
 * If an instruction consumes a reference both for taken and next, this
 * function should return false.
 */
bool consumes_reference_next_not_taken(const IRInstruction& inst,
                                       uint32_t srcID) {
  switch (inst.op()) {
  // The following have the old CRc flag, but don't observe or consume from
  // this module's perspective.  We'll clean this up when the old pass is gone.
  case StLoc:
  case StStk:
  case SpillFrame:
  case StMem:
    return false;
  default:
    return inst.consumesReference(srcID);
  }
}

/*
 * Return whether an IRInstruction may observe the reference count of a
 * specific argument.
 */
bool observes_reference(const IRInstruction& inst, uint32_t srcID) {
  return consumes_reference_next_not_taken(inst, srcID) ||
         consumes_reference_taken(inst, srcID);
}

//////////////////////////////////////////////////////////////////////

template<class Propagate, class NAdder>
void rc_analyze_inst(Env& env,
                     IRInstruction& inst,
                     RCState& state,
                     Propagate propagate,
                     NAdder add_node) {
  FTRACE(2, "  {}\n", inst);

  switch (inst.op()) {
  case IncRef:
    for_aset(env, state, inst.src(0), [&] (ASetID asetID) {
      add_node(asetID, NInc{&inst});
      auto& aset = state.asets[asetID];
      ++aset.lower_bound;
      FTRACE(3, "    {} lb: {}\n", asetID, aset.lower_bound);
    });
    return;
  case DecRef:
  case DecRefNZ:
    {
      auto old_lb = int32_t{0};
      for_aset(env, state, inst.src(0), [&] (ASetID asetID) {
        add_node(asetID, NDec{&inst});
        old_lb = state.asets[asetID].lower_bound;
        may_decref(env, state, asetID, add_node);
      });
      if (old_lb <= 1) analyze_mem_effects(env, inst, state, add_node);
    }
    return;
  case DefInlineFP:
    // We're converting a pre-live actrec to a live actrec, which effectively
    // changes some stack AliasClasses to not exist anymore.  See comments in
    // pure_spill_frame for an explanation of why we don't need any support
    // bits on this now.
    drop_support_bits(env, state,
      env.ainfo.expand(canonicalize(inline_fp_frame(&inst))));
    break;
  default:
    break;
  }

  /*
   * Before the decision on whether taken branch is traversed, instructions may
   * observe the reference count of their args.  In general these instructions
   * may make decisions based on whether this is the last reference to a tmp,
   * so we have to observe at level 2.
   */
  for (auto srcID = uint32_t{0}; srcID < inst.numSrcs(); ++srcID) {
    if (observes_reference(inst, srcID)) {
      for_aset(env, state, inst.src(srcID), [&] (ASetID asetID) {
        observe(env, state, asetID, add_node);
      });
    }
  }

  analyze_mem_effects(env, inst, state, add_node);

  /*
   * Propagate across the taken edge if there is one, first doing any
   * may-decrefs that only occur if the taken edge is traversed.  Right now we
   * conservatively leave the effects of those may-decrefs on the next() path
   * also.
   */
  if (auto const taken = inst.taken()) {
    for (auto srcID = uint32_t{0}; srcID < inst.numSrcs(); ++srcID) {
      if (consumes_reference_taken(inst, srcID)) {
        for_aset(env, state, inst.src(srcID), [&] (ASetID asetID) {
          may_decref(env, state, asetID, add_node);
        });
      }
    }
    propagate(taken);
  }

  /*
   * Handle consumes that only remove the reference if the taken branch is not
   * traversed, after we've propagated to taken.
   *
   * Note that the call to `consumes_reference_next_not_taken' is false if it
   * is consumed for both next /and/ taken, but in that case we'll have already
   * put the effects into the state above, since `consumes_reference_taken'
   * will be true.
   */
  for (auto srcID = uint32_t{0}; srcID < inst.numSrcs(); ++srcID) {
    if (consumes_reference_next_not_taken(inst, srcID)) {
      assertx(!consumes_reference_taken(inst, srcID));
      for_aset(env, state, inst.src(srcID), [&] (ASetID asetID) {
        may_decref(env, state, asetID, add_node);
      });
    }
  }

  /*
   * If the instruction produces a reference, we can assume the SSATmp it
   * defines corresponds to a new increment on the lower_bound.
   */
  if (inst.producesReference()) {
    for_aset(env, state, inst.dst(), [&] (ASetID asetID) {
      auto& aset = state.asets[asetID];
      ++aset.lower_bound;
      FTRACE(3, "    {} produced: lb {}\n", asetID, aset.lower_bound);
    });
  }

  /*
   * We assume that LdCtx on the main frame requires that the $this pointer has
   * a non-zero reference count, without any need for memory support.  The
   * reason for this is that nothing is allowed to DecRef the context of a
   * frame, except code that tears down that frame (either via a return or
   * unwinding).
   *
   * If this IR program contains a return sequence, it might DecRef the frame
   * context below one (possibly freeing it).  It is legal IR to have a LdCtx
   * after this sort of decref, but it would be a semantically incorrect
   * program if it does anything with the context after loading it that cares
   * about whether it's freed.  So we're safe assuming any load of the context
   * has a lower bound of one, because if it is loaded in a program position
   * where that doesn't hold, the program can't do anything that cares about
   * that without being broken already.
   *
   * One final note: because of the exclusivity rule on lower bounds, we cannot
   * assume a lower bound of one for a LdCtx unless it comes from the main
   * frame.  We are guaranteed (from find_alias_sets) that all the main frame
   * contexts are in the same must alias set, but the value stored to an
   * inlined frame may be in a different must alias set than the inlined LdCtx
   * set, and its lower bound could still be non-zero.  In practice, we expect
   * to rarely see LdCtx instructions inside of an inlined function---normally
   * the actual context object should be copy propagated.
   */
  if (inst.is(LdCtx) && inst.src(0) == env.unit.mainFP()) {
    for_aset(env, state, inst.dst(), [&] (ASetID asetID) {
      auto& aset = state.asets[asetID];
      aset.lower_bound = std::max(aset.lower_bound, 1);
      FTRACE(3, "    {} lb: {}\n", asetID, aset.lower_bound);
    });
  }
}

/*
 * This is the per-instruction analyze routine for rc_analyze, also used for
 * build_graphs.
 *
 * The NAdder function controls whether it's actually building a graph or just
 * performing analysis.
 */
template<class Propagate, class NAdder>
void rc_analyze_step(Env& env,
                     IRInstruction& inst,
                     RCState& state,
                     Propagate propagate,
                     NAdder add_node) {
  rc_analyze_inst(env, inst, state, propagate, add_node);
  // Note: we could use the gen set here to remove support entries when we step
  // the mrinfo, but it's not useful because only CallEffects causes it right
  // now, and SSATmps can't span calls.
  mrinfo_step(env, inst, state.avail);
  assertx(check_state(state));
}

//////////////////////////////////////////////////////////////////////

struct EmptyAdder {
  template<class T> void operator()(ASetID, const T&) const {}
};

RCAnalysis rc_analyze(Env& env) {
  FTRACE(1, "rc_analyze -----------------------------------------\n");

  auto ret = RCAnalysis{env.unit};

  /*
   * Initialize worklist to contain the entry block, and set up all the
   * BlockInfo rpoIds.
   */
  auto incompleteQ = dataflow_worklist<uint32_t>(env.rpoBlocks.size());
  for (auto rpoId = uint32_t{0}; rpoId < env.rpoBlocks.size(); ++rpoId) {
    auto& binfo          = ret.info[env.rpoBlocks[rpoId]];
    binfo.rpoId          = rpoId;
    binfo.state_in.avail = env.mrinfo.info[env.rpoBlocks[rpoId]].avail_in;
  }
  incompleteQ.push(0);

  /*
   * Initialize the entry block's in state.
   */
  ret.info[env.rpoBlocks[0]].state_in = entry_rc_state(env);

  /*
   * Find fixed point.
   */
  do {
    auto const blk = env.rpoBlocks[incompleteQ.pop()];
    FTRACE(2, "B{}:\n", blk->id());
    auto state = ret.info[blk].state_in;

    auto propagate = [&] (Block* target) {
      FTRACE(2, "   -> {}\n", target->id());
      auto& tinfo = ret.info[target];
      auto const changed = merge_into(env, tinfo.state_in, state);
      if (changed) incompleteQ.push(tinfo.rpoId);
    };

    for (auto& inst : blk->instrs()) {
      rc_analyze_step(env, inst, state, propagate, EmptyAdder{});
    }
    if (auto const next = blk->next()) propagate(next);
  } while (!incompleteQ.empty());

  return ret;
}

//////////////////////////////////////////////////////////////////////

DEBUG_ONLY std::string show_analysis(Env& env, const RCAnalysis& analysis) {
  auto ret = std::string{};

  for (auto asetID = uint32_t{0}; asetID < env.asets.size(); ++asetID) {
    folly::format(&ret, "{}: {}\n", asetID,
      *env.asets[asetID].representative);
    for (auto& blk : env.rpoBlocks) {
      auto& state = analysis.info[blk].state_in;
      if (!state.initialized) continue;
      auto& sinfo = state.asets[asetID];
      folly::format(
        &ret,
        "B{: <3}:  {: <2}\n",
        blk->id(),
        sinfo.lower_bound
      );
    }
    ret.push_back('\n');
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

DEBUG_ONLY bool direct_successor(Node* n, Node* succ) {
  return n->next == succ || (n->type == NT::Sig && to_sig(n)->taken == succ);
}

bool is_phi_pred(const NPhi* phi, Node* pred) {
  auto const last = phi->pred_list + phi->pred_list_sz;
  return std::find(phi->pred_list, last, pred) != last;
}

void add_phi_pred(Env& env, NPhi* nphi, Node* pred) {
  assertx(direct_successor(pred, nphi));
  auto const phi = to_phi(nphi);
  if (phi->pred_list_sz + 1 >= phi->pred_list_cap) {
    ++phi->pred_list_cap;
    auto const new_list = new (env.arena) Node*[phi->pred_list_cap];
    std::copy(phi->pred_list, phi->pred_list + phi->pred_list_sz, new_list);
    phi->pred_list = new_list;
  }
  phi->pred_list[phi->pred_list_sz++] = pred;
}

void rm_phi_pred(NPhi* phi, Node* n) {
  assertx(is_phi_pred(phi, n));

  // Only remove the first occurance of "n".  (A Sig node may be a predecessor
  // of the same Phi more than once.)
  auto const last = phi->pred_list + phi->pred_list_sz;
  auto const it = std::find(phi->pred_list, last, n);
  *it = last[-1];  // may self-assign if size is 1, but that's ok.
  --phi->pred_list_sz;
}

/*
 * Replace one of the `first' Node's successor pointers to `current' with
 * `replace'
 *
 * Pre: direct_successor(first, current)
 */
void rechain_forward(Node* first, Node* current, Node* replace) {
  assertx(direct_successor(first, current));
  if (first->next == current) {
    first->next = replace;
    return;
  }
  always_assert(first->type == NT::Sig && to_sig(first)->taken == current);
  to_sig(first)->taken = replace;
}

/*
 * Given a sequence of nodes in the graph:
 *
 *     pred  --->  middle  --->  last
 *
 * Mutate the pointers so that middle is unlinked, and last succeeds pred in
 * the same way that middle did.  (That is to say, if pred->next is middle,
 * pred->next will become last, and if pred->taken is middle, pred->taken will
 * become last.)  Handles Phi nodes in any of the three positions.
 *
 * Note that the number of predecessors of both types (backedge or normal) on a
 * Phi node in last or pred is conserved under this operation.
 *
 * Requires that middle has no other nodes attached to it.  (I.e. it has no
 * predecessors other than `pred', and no successors other than `last'.)  This
 * means middle can't be a Sig unless one of it's next or taken pointers is
 * nullptr.
 *
 * Pre:  direct_successor(pred, middle)
 *       direct_successor(middle, last)
 *
 * Post: direct_successor(pred, last) and middle is unlinked entirely
 */
void node_skip_over(Env& env, Node* pred, Node* middle, Node* last) {
  assertx(direct_successor(pred, middle));
  assertx(direct_successor(middle, last));
  assertx(middle->type != NT::Sig ||
    (middle->next == nullptr || to_sig(middle)->taken == nullptr));

  // Unlink middle node.
  if (last->type == NT::Phi) {
    rm_phi_pred(to_phi(last), middle);
  } else {
    if (debug) last->prev = nullptr;
  }
  if (middle->type == NT::Phi) {
    rm_phi_pred(to_phi(middle), pred);
  } else {
    middle->prev = nullptr;
    middle->next = nullptr;
  }

  // Rechain prev's forward pointer from pred to last.
  rechain_forward(pred, middle, last);

  // Set backward pointers from last to pred.
  if (last->type == NT::Phi) {
    add_phi_pred(env, to_phi(last), pred);
  } else {
    last->prev = pred;
  }
}

/*
 * Allocate and insert a new node between two adjacent nodes, returning the new
 * node, handling backlinking appropriately.  Either node may be an NT::Phi,
 * but the new node may not be a Phi.
 *
 * Pre: direct_successor(pred, succ)
 */
template<class T>
Node* add_between(Env& env,
                  Node* pred,
                  Node* succ,
                  const T& new_data) {
  assertx(direct_successor(pred, succ));
  auto const new_node = new (env.arena) T(new_data);
  assertx(new_node->type != NT::Phi);

  // Unlink backward pointers.
  if (succ->type == NT::Phi) {
    rm_phi_pred(to_phi(succ), pred);
  } else {
    if (debug) succ->prev = nullptr;
  }

  // Add forward pointers.
  rechain_forward(pred, succ, new_node);
  new_node->next = succ;

  // Add backward pointers.
  if (succ->type == NT::Phi) {
    add_phi_pred(env, to_phi(succ), new_node);
  } else {
    succ->prev = new_node;
  }
  new_node->prev = pred;

  return new_node;
}

//////////////////////////////////////////////////////////////////////

std::string show(const Node* node) {
  using folly::sformat;
  return sformat(
    "{} - {}",
    [&] () -> std::string {
      switch (node->type) {
      case NT::Empty:   return "empty";
      case NT::Halt:    return "halt";
      case NT::Sig:     return u8"\u03c3";
      case NT::Dec:     return sformat("dec({})", to_dec(node)->inst->id());
      case NT::Phi:
        return sformat(u8"\u03c6({},{})", to_phi(node)->pred_list_sz,
          to_phi(node)->back_edge_preds);
      case NT::Inc:
        return sformat("inc({})", to_inc(node)->inst->id());
      case NT::Req:
        return to_req(node)->level == std::numeric_limits<int32_t>::max()
          ? "req!"
          : sformat("req{}", to_req(node)->level);
      }
      not_reached();
    }(),
    node->lower_bound
  );
}

void find_nodes(jit::vector<Node*>& accum,
                jit::hash_set<Node*>& seen,
                Node* n) {
  if (n == nullptr || seen.count(n)) return;
  accum.push_back(n);
  seen.insert(n);
  if (n->type == NT::Sig) find_nodes(accum, seen, to_sig(n)->taken);
  find_nodes(accum, seen, n->next);
}

std::string graph_dot_nodes(SSATmp* representative, size_t graph_id, Node* g) {
  auto ret = std::string{};
  auto nodes = jit::vector<Node*>{};
  auto seen = jit::hash_set<Node*>{};
  find_nodes(nodes, seen, g);

  static const bool debug_back_links = getenv("RC_GRAPH_BACKLINKS");
  constexpr int node_id_shift = 8;

  jit::hash_map<Node*,uint32_t> node_to_id;
  for (auto idx = uint32_t{0}; idx < nodes.size(); ++idx) {
    node_to_id[nodes[idx]] = (idx + 1) << node_id_shift | graph_id;
  }

  folly::format(
    &ret,
    "N{} [shape=box,label=\"t{} :: {}\"]; "
      "node [shape=plaintext]; N{} -> N{};\n",
    graph_id,
    representative->id(),
    representative->type().toString(),
    graph_id,
    1 << node_id_shift | graph_id
  );

  for (auto idx = uint32_t{0}; idx < nodes.size(); ++idx) {
    auto const n = nodes[idx];
    auto const node_num = (idx + 1) << node_id_shift | graph_id;

    folly::format(&ret, "N{} [label=\"{}\"];", node_num, show(n));

    if (n->next) {
      folly::format(&ret, " N{} -> N{};", node_num, node_to_id[n->next]);
      assertx(n->next->type == NT::Phi || n->next->prev == n);
    }

    if (debug_back_links && n->prev) {
      folly::format(&ret, " N{} -> N{} [color=cyan];", node_num,
        node_to_id[n->prev]);
    }

    if (n->type == NT::Sig && to_sig(n)->taken) {
      folly::format(&ret, " N{} -> N{} [color=green]", node_num,
        node_to_id[to_sig(n)->taken]);
    }

    if (debug_back_links && n->type == NT::Phi) {
      for (auto pred_i = uint32_t{0};
           pred_i < to_phi(n)->pred_list_sz;
           ++pred_i) {
        auto const pred = to_phi(n)->pred_list[pred_i];
        if (debug_back_links) {
          folly::format(&ret, "N{} -> N{} [color=red];\n",
            node_num, node_to_id[pred]);
        }
      }
    }

    folly::format(&ret, "\n");
  }

  return ret;
}

std::string graphs_dot_string(const jit::vector<MustAliasSet>& asets,
                              const jit::vector<Node*>& heads) {
  assertx(asets.size() == heads.size());
  auto ret = std::string{};
  ret = "digraph G {\n";
  for (auto graph_id = size_t{0}; graph_id < heads.size(); ++graph_id) {
    ret += graph_dot_nodes(
      asets[graph_id].representative,
      graph_id,
      heads[graph_id]
    );
  }
  ret.push_back('}');
  ret.push_back('\n');
  return ret;
}

DEBUG_ONLY std::string show_graphs(const jit::vector<MustAliasSet>& asets,
                        const jit::vector<Node*>& heads) {
  char fileBuf[] = "/tmp/hhvmXXXXXX";
  int fd = mkstemp(fileBuf);
  if (fd == -1) {
    return folly::sformat("couldn't open temporary file: {}\n",
      folly::errnoStr(errno));
  }
  SCOPE_EXIT { close(fd); };
  auto file = fdopen(fd, "w");
  std::fprintf(file, "%s", graphs_dot_string(asets, heads).c_str());
  std::fflush(file);
  return folly::sformat("dot -T xlib < {} &\n", fileBuf);
}

//////////////////////////////////////////////////////////////////////

bool check_graph(Node* graph) {
  auto nodes = jit::vector<Node*>{};
  auto seen = jit::hash_set<Node*>{};
  find_nodes(nodes, seen, graph);

  for (auto& n : nodes) {
    always_assert(n->lower_bound >= 0);

    if (n->prev) {
      always_assert(direct_successor(n->prev, n));
    }

    if (n->next) {
      if (n->next->type == NT::Phi) {
        always_assert(is_phi_pred(to_phi(n->next), n));
      } else {
        always_assert(n->next->prev == n);
      }
    }

    switch (n->type) {
    case NT::Inc:
      always_assert(to_inc(n)->inst->is(IncRef));
      break;
    case NT::Dec:
      always_assert(to_dec(n)->inst->is(DecRef, DecRefNZ));
      break;
    case NT::Phi:
      always_assert(to_phi(n)->block != nullptr);
      // At least one predecessor.  Normally there's two, but some of the
      // clean_graphs code can create single predecessor phis (it also has
      // rules to clean those up, but we'd rather have cleaning those up be
      // optional instead of required as an invariant).
      always_assert(to_phi(n)->pred_list_sz >= 1);
      // At least one non-back edge predecessor.
      always_assert(to_phi(n)->back_edge_preds < to_phi(n)->pred_list_sz);
      // Size is always <= capacity.
      always_assert(to_phi(n)->pred_list_sz <= to_phi(n)->pred_list_cap);
      // Each pred has a forward link to the phi.
      for (auto i = uint32_t{0}; i < to_phi(n)->pred_list_sz; ++i) {
        auto const pred = to_phi(n)->pred_list[i];
        always_assert(pred != nullptr);
        always_assert(direct_successor(pred, n));
      }
      break;
    case NT::Sig:
      always_assert(to_sig(n)->block != nullptr);
      if (auto const taken = to_sig(n)->taken) {
        if (taken->type == NT::Phi) {
          always_assert(is_phi_pred(to_phi(taken), n));
        } else {
          always_assert(taken->prev == n);
        }
      }
      break;
    case NT::Req:
      // We should never have Req{0} nodes.
      always_assert(to_req(n)->level >= 1);
      break;
    case NT::Empty:
    case NT::Halt:
      break;
    }
  }
  return true;
}

DEBUG_ONLY bool check_graphs(const jit::vector<Node*>& graphs) {
  for (auto& g : graphs) always_assert(check_graph(g));
  return true;
}

//////////////////////////////////////////////////////////////////////

void do_clean_graph(Env& env,
                    jit::queue<std::pair<Node*,Node*>>& workQ,
                    jit::hash_set<Node*>& seen_set,
                    bool& changed,
                    Node* prev,
                    Node* cur) {
  while (cur != nullptr) {
    switch (cur->type) {
    case NT::Req:
    case NT::Inc:
    case NT::Dec:
    case NT::Halt:
      // Normal nodes that we just keep
      prev = cur;
      cur = cur->next;
      continue;

    case NT::Phi:
      if (seen_set.count(cur)) {
        // Don't reprocess the phi.
        return;
      }
      if (to_phi(cur)->pred_list_sz == 1 &&
          to_phi(cur)->back_edge_preds == 0) {
        auto const next = cur->next;
        node_skip_over(env, prev, cur, next);
        changed = true;
        cur = next;
        continue;
      }
      seen_set.insert(cur);
      prev = cur;
      cur = cur->next;
      continue;

    case NT::Sig:
      {
        auto const next = cur->next;
        auto const taken = to_sig(cur)->taken;
        if (next == nullptr && taken != nullptr) {
          node_skip_over(env, prev, cur, taken);
          changed = true;
          cur = taken;
          continue;
        }
        if (next != nullptr && taken == nullptr) {
          node_skip_over(env, prev, cur, next);
          changed = true;
          cur = next;
          continue;
        }
        if (next == taken && next != nullptr) {
          assertx(next->type == NT::Phi);
          auto const phi = to_phi(next);
          // We only apply this rule when it isn't a back_edge_preds because we
          // always want a Phi involved in loops.
          if (phi->pred_list_sz == 2 && phi->back_edge_preds == 0) {
            rm_phi_pred(phi, cur); // Leaving one of the preds.
            assertx(is_phi_pred(phi, cur));
            static_assert(sizeof(NEmpty) < sizeof(NPhi), "");
            cur->type = NT::Empty;          // Let the empty rule remove it.
            changed = true;
            continue;
          }
        }

        // Schedule taken for later, and continue doing the next path now.
        assertx(taken && next);
        workQ.emplace(cur, taken);
        prev = cur;
        cur = next;
        continue;
      }

    case NT::Empty:
      if (prev && cur->next) {
        auto const next = cur->next;
        node_skip_over(env, prev, cur, next);
        changed = true;
        cur = next;
        continue;
      }
      prev = cur;
      cur = cur->next;
      continue;
    }

    always_assert(0);
  }
}

Node* clean_graph(Env& env, Node* head) {
  // Only used for phi nodes, to avoid processing them more than once.
  auto seen_set = jit::hash_set<Node*>{};

  // When we see control flow splits, we need want to process both paths.  We
  // use this workQ containing (prev, cur) to delay one side.
  auto workQ = jit::queue<std::pair<Node*,Node*>>{};

  bool changed;
  do {
    changed = false;
    assertx(workQ.empty());
    workQ.emplace(nullptr, head);
    do {
      Node* prev;
      Node* cur;
      std::tie(prev, cur) = workQ.front();
      workQ.pop();
      do_clean_graph(env, workQ, seen_set, changed, prev, cur);
    } while (!workQ.empty());
  } while (changed);

  return head;
}

jit::vector<Node*> clean_graphs(Env& env, jit::vector<Node*> heads) {
  for (auto& h : heads) h = clean_graph(env, h);
  return heads;
}

//////////////////////////////////////////////////////////////////////

using ChainProgress = jit::vector<Node*>;
using Incoming      = jit::vector<ChainProgress>;

void add_node(const RCState& state,
              ChainProgress& chains,
              int32_t asetID,
              Node* node) {
  node->lower_bound = state.asets[asetID].lower_bound;
  if (node->type != NT::Phi && node->type != NT::Sig) {
    FTRACE(2, "      {} += {}\n", asetID, show(node));
  }
  auto& tail = chains[asetID];
  tail->next = node;
  node->prev = tail;
  tail = node;
}

struct NodeAdder {
  explicit NodeAdder(Env& env, RCState& state, ChainProgress& chains)
    : env(env)
    , state(state)
    , chains(chains)
  {}

  template<class NodeT>
  void operator()(ASetID asetID, const NodeT& n) const {
    add_node(state, chains, asetID, new (env.arena) NodeT{n});
  }

  void operator()(ASetID asetID, const NReq& req) const {
    auto& tail = chains[asetID];
    if (tail->type == NT::Req) {
      // Combine adjacent Req nodes.
      to_req(tail)->level = std::max(to_req(tail)->level, req.level);
      tail->lower_bound = std::min(
        tail->lower_bound,
        state.asets[asetID].lower_bound
      );
      FTRACE(2, "      {} += combining req {}\n", asetID, show(tail));
      return;
    }
    add_node(state, chains, asetID, new (env.arena) NReq{req});
  }

private:
  Env& env;
  RCState& state;
  ChainProgress& chains;
};

jit::vector<Node*> make_heads(Env& env) {
  auto ret = ChainProgress{};
  ret.resize(env.asets.size());
  for (auto& n : ret) {
    n = new (env.arena) NEmpty{};
  }
  return ret;
}

ChainProgress merge_incoming(Env& env,
                             const RCState& state,
                             Block* blk,
                             const Incoming& incoming) {
  assertx(!incoming.empty());
  auto ret = ChainProgress{};
  ret.resize(env.asets.size());
  auto const incoming_sz = safe_cast<uint32_t>(incoming.size());
  for (auto asetID = uint32_t{0}; asetID < env.asets.size(); ++asetID) {
    auto const phi = new (env.arena) NPhi{blk};
    phi->pred_list = new (env.arena) Node*[incoming_sz];
    phi->pred_list_cap = incoming_sz;
    phi->lower_bound = state.asets[asetID].lower_bound;
    ret[asetID] = phi;
    for (auto& inc : incoming) {
      inc[asetID]->next = phi;
      add_phi_pred(env, phi, inc[asetID]);
    }
  }
  return ret;
}

template<class T>
void add_node_all(Env& env,
                  const RCState& state,
                  ChainProgress& chains,
                  const T& data) {
  for (auto asetID = uint32_t{0}; asetID < env.asets.size(); ++asetID) {
    add_node(state, chains, asetID, new (env.arena) T(data));
  }
}

//////////////////////////////////////////////////////////////////////

jit::vector<Node*> build_graphs(Env& env, const RCAnalysis& analysis) {
  FTRACE(1, "build_graphs -----------------------------------------\n");

  StateVector<Block,Incoming> incoming(env.unit, Incoming{});
  auto pending_phis = sparse_idptr_map<Block,ChainProgress>(
    env.unit.numBlocks()
  );

  auto heads = make_heads(env);

  for (auto& blk : env.rpoBlocks) {
    FTRACE(2, "B{}:\n", blk->id());

    bool const missing_back_edges = incoming[blk].size() != blk->numPreds();
    auto state = analysis.info[blk].state_in;
    auto chains =
      blk == env.rpoBlocks.front() ? heads :
      !missing_back_edges &&
        incoming[blk].size() == 1 ? incoming[blk].front() :
      merge_incoming(env, state, blk, incoming[blk]);
    if (missing_back_edges) {
      pending_phis[blk] = chains;
      if (debug) {
        for (auto& n : pending_phis[blk]) always_assert(n->type == NT::Phi);
      }
    }

    auto node_adder = NodeAdder{env, state, chains};
    for (auto& inst : blk->instrs()) {
      auto propagate = [&] (Block* target) {
        add_node_all(env, state, chains, NSig{blk});

        if (!pending_phis.contains(target)) {
          incoming[target].push_back(chains);
          auto asetID = uint32_t{0};
          for (auto& ch : incoming[target].back()) {
            auto const sig = ch;
            auto const empty = new (env.arena) NEmpty{};
            to_sig(sig)->taken = empty;
            empty->prev = sig;
            empty->lower_bound = state.asets[asetID].lower_bound;
            ch = empty;
            ++asetID;
          }
          return;
        }

        auto const& phis = pending_phis[target];
        for (auto asetID = uint32_t{0}; asetID < chains.size(); ++asetID) {
          auto const phi = phis[asetID];
          to_sig(chains[asetID])->taken = phi;
          ++to_phi(phi)->back_edge_preds;
          add_phi_pred(env, to_phi(phi), chains[asetID]);
        }
      };

      rc_analyze_step(env, inst, state, propagate, node_adder);
    }

    if (auto const next = blk->next()) {
      if (!pending_phis.contains(next)) {
        incoming[next].emplace_back(std::move(chains));
      } else {
        auto const& phis = pending_phis[next];
        for (auto asetID = uint32_t{0}; asetID < chains.size(); ++asetID) {
          auto const phi = phis[asetID];
          chains[asetID]->next = phi;
          ++to_phi(phi)->back_edge_preds;
          add_phi_pred(env, to_phi(phi), chains[asetID]);
        }
      }
    }
  }

  return heads;
}

//////////////////////////////////////////////////////////////////////

/*
 * RC flowgraph rules.  ("rule_foo" functions)
 *
 * Each transformation on the RC flowgraph has a shape of the graph it matches
 * against, set of preconditions, and a "reprocess" point if it makes a
 * transformation.  Each rule has a diagram explaining the graph transformation
 * it makes, the changes it makes to the underlying IR, the preconditions for
 * the rule applying, and where it tries to reprocess.
 *
 * Often, the rules will want to "reprocess" by backing up a node.  This means
 * the transformations work with backtracking and infinite lookahead---but
 * because of the set of applicable rules, the infinite lookahead is limited to
 * sections of the graph that contain no control flow nodes.  Also, no
 * backtracking will occur unless a rule applies.
 *
 * The reason to allow back-tracking is easily shown by a series of foldable
 * incs and decs.  Consider the graph fragment:
 *
 *  ... ->  inc-2  -->  inc-3  -->  inc-4  -->  dec-5  -->  dec-4  --> ...
 *
 * Which we should be able to turn into just "inc-2".  The rule_inc_dec_fold
 * will first apply when we're pointing to the "inc-4" node, removing "inc-4"
 * and "dec-5".  Instead of proceeding to "dec-4" after it applies, it moves
 * back to reprocess at "inc-3", which lets the rule apply again to remove
 * "inc-3" and "dec-4".
 *
 * When scheduling reprocess nodes these functions use `reprocess_helper',
 * which prevent scheduling sigma nodes an extra time, since it's not
 * profitable and can cause us to get to phi nodes before we've processed their
 * predecessors (which can miss optimization opportunities).
 */

Node* reprocess_helper(Node* pred, Node* succ) {
  return pred->type == NT::Sig ? succ : pred;
}

bool can_sink(Env& env, const IRInstruction* inst, const Block* block) {
  assertx(inst->is(IncRef));
  if (!block->taken() || !block->next()) return false;
  if (inst->src(0)->inst()->is(DefConst)) return true;
  auto const defBlock = findDefiningBlock(inst->src(0), env.idoms);
  return dominates(defBlock, block->taken(), env.idoms) &&
         dominates(defBlock, block->next(), env.idoms);
}

bool all_preds_are_sinkable_incs(const NPhi& phi) {
  return std::all_of(
    phi.pred_list,
    phi.pred_list + phi.pred_list_sz,
    [&] (const Node* n) {
      return n->type == NT::Inc && n->lower_bound >= 1;
    }
  );
}

IRInstruction* find_sinkable_pred(const Env& env, const NPhi& phi) {
  assertx(all_preds_are_sinkable_incs(phi));
  auto const block = phi.block;
  auto const it = std::find_if(
    phi.pred_list,
    phi.pred_list + phi.pred_list_sz,
    [&] (const Node* pred) {
      auto const defBlock = findDefiningBlock(to_inc(pred)->inst->src(0),
                                              env.idoms);
      return dominates(defBlock, block, env.idoms);
    }
  );
  if (it == phi.pred_list + phi.pred_list_sz) return nullptr;
  return to_inc(*it)->inst;
}

/*
 * Rule "inc_dec_fold":
 *
 *      [ A ]  |  x >= 1
 *        |    |  y >= 2
 *      inc-x  |
 *        |    |
 *      dec-y  |
 *        |    |
 *      [ B ]  |
 *     -----------------
 *
 *           [ A ]  <-- reprocess
 *             |
 *           [ B ]
 *
 * The IncRef and DecRef{NZ,} instructions in the underlying program are
 * removed.
 */
Node* rule_inc_dec_fold(Env& env, Node* node) {
  bool const applies =
    node->type == NT::Inc && node->next &&
    node->next->type == NT::Dec &&
    node->next->lower_bound >= 2 &&
    node->lower_bound >= 1;
  if (!applies) return node;
  auto const ninc  = node;
  auto const ndec  = node->next;
  auto const nprev = ninc->prev;
  auto const nsucc = ndec->next;
  FTRACE(2, "    ** inc_dec_fold: {}, {}\n", *to_inc(ninc)->inst,
    *to_dec(ndec)->inst);
  remove_helper(to_inc(ninc)->inst);
  remove_helper(to_dec(ndec)->inst);
  node_skip_over(env, ninc, ndec, ndec->next);
  node_skip_over(env, nprev, ninc, ninc->next);
  return reprocess_helper(nprev, nsucc);
}

/*
 * Rule "inc_pass_req":
 *
 *      [ A ]   |  x >= 1
 *        |     |  y - 1 >= N
 *      inc-x   |
 *        |     |
 *     reqN-y   |
 *        |     |
 *      [ B ]   |
 *   ---------------------
 *
 *         [ A ]     <--- reprocess
 *           |
 *        reqN-(y-1)
 *           |
 *         inc-(y-1)
 *           |
 *         [ B ]
 *
 * No change to the underlying IR program.
 */
Node* rule_inc_pass_req(Env& env, Node* node) {
  bool const applies =
    node->type == NT::Inc && node->next &&
    node->next->type == NT::Req &&
    (node->next->lower_bound - 1) >= to_req(node->next)->level &&
    node->lower_bound >= 1;
  if (!applies) return node;
  FTRACE(2, "    ** inc_pass_req\n");
  auto const ninc  = node;
  auto const nreq  = node->next;
  auto const nprev = ninc->prev;
  node_skip_over(env, ninc, nreq, nreq->next);
  rechain_forward(nprev, ninc, nreq);
  ninc->prev = nreq;
  nreq->prev = nprev;
  nreq->next = ninc;
  ninc->lower_bound = std::max(nreq->lower_bound - 1, 0);
  nreq->lower_bound = std::max(nreq->lower_bound - 1, 0);
  return reprocess_helper(nprev, ninc);
}

/*
 * Rule "inc_pass_sig":
 *
 *      [ A ]    |  x >= 1
 *        |      |  y >= 2
 *      inc-x    |
 *        |      |  the inc'd tmp is defined in B and C
 *     sigma-y   |
 *      /   \    |  B != C (normally removed by clean)
 *   [ B ] [ C ] |
 *  -----------------------------------------------
 *
 *            [ A ]      <--- reprocess
 *              |
 *         sigma-(y-1)
 *           /     \
 *     inc-(y-1)  inc-(y-1)
 *         |        |
 *       [ B ]    [ C ]
 *
 * The change to the RC graph is reflected in the underlying IR program.  We
 * remove the IncRef before the control flow split, and insert new copies on
 * the next and taken sides.
 */
Node* rule_inc_pass_sig(Env& env, Node* node) {
  bool const applies =
    node->type == NT::Inc && node->next &&
    node->next->type == NT::Sig &&
    node->next->next != nullptr &&
    to_sig(node->next)->taken != nullptr &&
    node->next->next != to_sig(node->next)->taken &&
    can_sink(env, to_inc(node)->inst, to_sig(node->next)->block) &&
    node->lower_bound >= 1 &&
    node->next->lower_bound >= 2;
  if (!applies) return node;

  auto const nold_inc = to_inc(node);
  auto const nprev    = nold_inc->prev;
  auto const nsig     = to_sig(node->next);

  auto const value     = nold_inc->inst->src(0);
  auto const marker    = nold_inc->inst->marker();
  auto const new_taken = env.unit.gen(IncRef, marker, value);
  auto const new_next  = env.unit.gen(IncRef, marker, value);

  FTRACE(2, "    ** inc_pass_sig: {} -> {}, {}\n",
    *nold_inc->inst, *new_taken, *new_next);

  node_skip_over(env, nprev, nold_inc, nsig);
  auto const ntaken = add_between(env, nsig, nsig->taken, NInc{new_taken});
  auto const nnext  = add_between(env, nsig, nsig->next, NInc{new_next});

  nnext->lower_bound  = std::max(nsig->lower_bound - 1, 0);
  ntaken->lower_bound = std::max(nsig->lower_bound - 1, 0);
  nsig->lower_bound   = std::max(nsig->lower_bound - 1, 0);

  remove_helper(nold_inc->inst);
  nsig->block->taken()->prepend(new_taken);
  nsig->block->next()->prepend(new_next);

  return reprocess_helper(nprev, nsig);
}

/*
 * Rule "inc_pass_phi":
 *
 *     [ A ]  [ B ] ... |  all pred lower_bounds x, y, ... >= 1
 *       |      |       |  z >= 2
 *     inc-x  inc-y     |
 *        \   /         |  at least one tmp defined at join
 *        phi-z         |
 *          |           |
 *        [ C ]         |
 *  --------------------------------
 *
 *          [ A ]  [ B ] ...
 *             \    /
 *            phi-(z-1)    <--- reprocess
 *               |
 *            inc-(z-1)
 *               |
 *             [ C ]
 *
 * The change to the RC graph is reflected in the underlying IR program.  We
 * remove the IncRefs for each incoming node, and insert a new one after the
 * join point.
 *
 * Note: it may seem like we should need a precondition on this rule that each
 * incoming node is distinct, since Phi nodes don't necessarily have unique
 * predecessor pointers.  However, only Sig nodes can have multiple successors,
 * so this situation doesn't apply.
 */
Node* rule_inc_pass_phi(Env& env, Node* node) {
  bool const applies =
    node->type == NT::Phi &&
    node->lower_bound >= 2 &&
    all_preds_are_sinkable_incs(*to_phi(node));
  if (!applies) return node;
  auto const sink = find_sinkable_pred(env, *to_phi(node));
  if (!sink) return node;

  auto const nphi     = to_phi(node);
  auto const new_inc  = env.unit.gen(IncRef, sink->marker(), sink->src(0));
  auto const nnew_inc = add_between(env, nphi, nphi->next, NInc{new_inc});

  nnew_inc->lower_bound = std::max(nphi->lower_bound - 1, 0);
  nphi->lower_bound     = std::max(nphi->lower_bound - 1, 0);

  FTRACE(2, "    ** inc_pass_phi: {}\n", *new_inc);
  nphi->block->prepend(new_inc);

  assertx(nphi->prev == nullptr);
  for (auto i = uint32_t{0}; i < nphi->pred_list_sz; ++i) {
    auto& pred_ptr = nphi->pred_list[i];
    auto const inc = to_inc(pred_ptr);
    auto const inc_pred = inc->prev;
    rechain_forward(inc_pred, inc, nphi);
    inc->prev = nullptr;
    inc->next = nullptr;
    remove_helper(inc->inst);
    pred_ptr = inc_pred;
  }

  return nphi;
}

/*
 * Rule "decnz":
 *
 *     [ A ]   | x >= 2
 *       |     |
 *     dec-x   |
 *       |     |
 *     [ B ]   |
 *  ----------------------
 *
 *  Convert DecRef to DecRefNZ in the underlying IR program---no change to
 *  the RC flowgraph.
 */
Node* rule_decnz(Env& env, Node* node) {
  bool const applies =
    node->type == NT::Dec &&
    node->lower_bound >= 2 &&
    to_dec(node)->inst->is(DecRef);
  if (!applies) return node;
  FTRACE(2, "    ** decnz:  {}\n", *to_dec(node)->inst);
  to_dec(node)->inst->setOpcode(DecRefNZ);
  return node->next;
}

Node* optimize_node(Env& env, Node* node, jit::queue<Node*>& workQ) {
  if (node->type == NT::Phi) {
    ++node->visit_counter;
    if (node->visit_counter !=
        to_phi(node)->pred_list_sz - to_phi(node)->back_edge_preds) {
      // Wait until we've processed all the forward predecessors before looking
      // at the Phi node.
      return nullptr;
    }
  }

  for (;;) {
    auto const orig_node = node;
    FTRACE(3, "  {}\n", show(node));
    if (node->type == NT::Halt) return nullptr;

    node = rule_inc_dec_fold(env, node);
    node = rule_inc_pass_req(env, node);
    node = rule_inc_pass_sig(env, node);
    node = rule_inc_pass_phi(env, node);
    node = rule_decnz(env, node);

    if (node == nullptr || node == orig_node) break;
  }

  if (!node) return nullptr;
  if (node->type == NT::Sig) {
    if (auto const t = to_sig(node)->taken) {
      workQ.push(t);
    }
  }
  // Return the next, or nullptr if it doesn't have a next:
  return node->next;
}

void optimize_graph(Env& env, Node* head) {
  auto workQ = jit::queue<Node*>{};
  workQ.push(head);
  do {
    auto current = workQ.front();
    workQ.pop();
    do {
      current = optimize_node(env, current, workQ);
    } while (current != nullptr);
  } while (!workQ.empty());
}

void optimize_graphs(Env& env, const jit::vector<Node*>& graphs) {
  FTRACE(1, "optimize_graphs -----------------------------------------\n");
  for (auto asetID = uint32_t{0}; asetID < graphs.size(); ++asetID) {
    FTRACE(2, "{} {}\n", asetID, *env.asets[asetID].representative);
    optimize_graph(env, graphs[asetID]);
  }
  FTRACE(1, "{}", show_graphs(env.asets, graphs));
}

void rcgraph_opts(Env& env) {
  // Get analysis results that let us build the rc flowgraphs.
  auto const rcAnalysis = rc_analyze(env);
  FTRACE(1, "\nRCAnalysis:\n\n{}\n", show_analysis(env, rcAnalysis));

  // Build the graphs.
  auto graphs = build_graphs(env, rcAnalysis);
  FTRACE(1, "rc arena size: {}\n", env.arena.size());
  assertx(check_graphs(graphs));
  FTRACE(1, "{}", show_graphs(env.asets, graphs));

  // Clean the graphs up, so they're easier to pattern match against in the
  // optimize pass.
  graphs = clean_graphs(env, std::move(graphs));
  assertx(check_graphs(graphs));
  FTRACE(1, "{}", show_graphs(env.asets, graphs));

  // Optimize each graph.
  optimize_graphs(env, graphs);
  assertx(check_graphs(graphs));
  FTRACE(1, "rc arena size: {}\n", env.arena.size());
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void optimizeRefcounts2(IRUnit& unit) {
  Timer timer(Timer::optimize_refcountOpts);
  splitCriticalEdges(unit);

  PassTracer tracer{&unit, Trace::hhir_refcount, "optimizeRefcounts"};
  Env env { unit };

  find_alias_sets(env);
  if (env.asets.size() == 0) return;

  populate_mrinfo(env);
  weaken_decrefs(env);
  rcgraph_opts(env);
  remove_trivial_incdecs(env);

  // We may have pushed IncRefs past CheckTypes, which could allow us to
  // specialize them.
  insertNegativeAssertTypes(unit, env.rpoBlocks);
  refineTmps(unit, env.rpoBlocks, env.idoms);
}

}}
