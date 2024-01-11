(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
INVARIANTS FOR CACHE CORRECTNESS

This comment concerns the following caches:
* [Provider_backend.local_memory.shallow_decl_cache]
* [Provider_backend.local_memory.folded_class_cache]
* [Provider_backend.local_memory.decl_cache]
* [Provider_backend.local_memory.decls_reflect_this_file]
* [Provider_context.entry.all_errors]
* [Provider_context.entry.tast]

The way things are used in ClientIdeDaemon is
1. ClientIdeDaemon holds a single [local_memory] which it uses for all requests
2. ClientIdeDaemon holds a Provider_context.entry cache for each open tab.
It has to: it's designed to work with unsaved file contents in editor buffers.
3. When it's time to do something (autocomplete, compute a tast, ...) then it
creates a Provider_context.t with just one single entry, corresponding to the
tab for which it's doing its work
4. The hope is that for most of the time, we can just re-use the cached tast.
5. Also, sometimes disk-change events happen.

The way things are used in hh_server e.g. for "hh --type-at-pos" is
1. Not a good design!
2. It's backed by rust-shared-mem, not by local_memory
3. Some places use Provider_context.entry because they want to hang a cached TAST off it.
However the content of that entry is simply whatever is on disk.
4. Hh_server used to be able to handle ephemeral contents "cat a.php | hh --type-at-pos line:col -"
but it no longer does; now it's supposed to solely handles truth as it is on disk.
(If there are any lingering places which allow ephemeral input that disagrees with disk,
they likely have bugs).
5. Lots of places call "enter quarantine" even though it's not meaningful. That
might be because they use functions that are also used by the IDE, and we
had the mistaken idea that it was important.

Our goal informally is that TASTs must be correct: if we read from a cached TAST then it
must be up-to-date, and if we recompute a TAST from decls then those decls must be up-to-date.
The way we enforce this is by "invalidating" i.e. removing from the cache any stale items as
soon as they become stale.
More precisely, here's the correctness invariant:
1. In [Provider_context.entry], the cached [tast] and [all_errors] reflect what we'd get by
typechecking this entry (including its contents even if they're different from disk) against
all other files as they are on disk.
2. For ClientIdeDaemon, in [Provider_backend.local_memory], the three decl caches reflect truth
for the entry if any in [decls_reflect_this_file], and reflects truth as it is on disk for all other files.
3. For hh_server, the backend decl caches reflect truth as it is in disk.

The ClientIdeDaemon case is interesting. It uses [decls_reflect_this_file] for performance.
The idea is that most of the time the user continues to work in a single file hence we might
as well uphold invariant 2 by just leaving the decl caches correct for whichever was the most recent
file that the user worked in. The other idea behind it is that most of the time user keystrokes
aren't even in signatures, and if they're not in signatures then no decls at all even need be invalidated.

BUT, THIS IS TOO SLOW.

I tried several approaches but the cost of doing correct invalidation was just too slow
for the IDE. Let us consider three approaches:
* STICKY QUARANTINE. That's what's described above with [decls_reflect_this_file]. It is
gated under the [tco_sticky_quarantine] flag, plus an extra flag [tco_lsp_invalidation]
for a little extra data-gathering. The writeup is here: https://fb.workplace.com/groups/342676619986174/posts/1406304853623340
I tried it twice, first with just invalidating everything when needed, second time with
surgically figuring out the exact right thing to invalidate. As per the writeup, it slowed
things down.
* CURRENT STATUS QUO. Currently [tco_sticky_quarantine] is turned off by default. Instead
we use an invalidation strategy so ugly (and wrong) that I can't bear to describe it.
It invalidates the current file's folded and shallow decls, and nothing else.
The user effect is that users end up sometimes with errors that don't go away, e.g. if they're
editing a.php with class A, and there's another file b.php with class B extends A, but back
in a.php there's another class C extends B. (this turns out to be a common code pattern, I think
because of partial codegen). Sometimes users get just decl-bug crashes in the file that never
go away. Sometimes they get wrong squiggles.
* WHAT IS THE OPTIMAL SOLUTION? We will have to invent a new technique, one that combines
the speed of the current status quo, and the correctness of sticky quarantine. Here's
how I believe it will work: (1) when the user types, we do a combination of sticky-quarantine
to avoid invalidating if the user's changes didn't affect anything, plus status-quo where
even if they did affect something then we *WRONGLY* invalidate folded decls for only the
current file. (2) when the user saves a file, then we have to pay back the debt that was
incurred earlier with our wrong invalidations, by invalidating the right things.
In this optimal solution, the invariants around [decls_reflect_this_file] aren't powerful
enough because they're not able to express a "debt that has to be paid back".

An API oddity is that we used to follow a different paradigm, that of "quarantine for an entry".
The idea there was that the decl cache always reflected truth on disk, but when we start handling
an IDE request or recompute a TAST for an entry then we'd invalidate decls relating to that entry,
so they could be lazily repopulated with decls that reflect the ephemeral contents of that entry,
and then we'd flush them upon leaving quarantine. Yuck. This lead to much needless invalidation,
and hence much needless refolding of decls. We're still stuck with the quarantine API. But at least,
entering and leaving quarantine-for-an-entry is now simply a no-op when [decls_reflect_this_file]
agrees with that entry.

Another API oddity is that quarantine notionally supports multiple entries at a time. It shouldn't,
and [decls_reflect_this_file] doesn't, and the efficient invalidation doesn't. I hope we can change
the API to only support a single ephemeral entry at a time.

Another API oddity is that hh_server uses quarantine as well. It needn't, and indeed shouldn't.
The issue is that things like "hh_server --type-at-pos-batch" want to cache the AST and TAST,
but the only means they have to do this is by setting up an entry, but that means that everything
involving naming-table and decls is now a bit slower because it has to go through the possibility
(needed by ClientIdeDaemon but not hh_server) that the contents of the entry disagrees with
what's on disk.

Another API oddity has to do with the moment at which we invalidate cached [tast] and [all_errors].
One good idea would be to invalidate them at the same time we invalidate cached decls. But the
way things are set up, ClientIdeDaemon maintains its own list of one entry for each open tab,
hence only ClientIdeDaemon has the power to invalidate all cached [tast] and [all_errors].
It achieves this by passing its list of entries to [invalidate_upon_file_changes] to be
invalidated then. *)

(** This function will (1) enter quarantine for all entries in the file, (2) do the callback "f",
(3) leave quarantine. If an exception arises during step (1,2) then nevertheless we guarantee that
quarantine is safely left. If an exception arises during step (3) then we'll raise an exception but
the program state has become unstable... *)
val respect_but_quarantine_unsaved_changes :
  ctx:Provider_context.t -> f:(unit -> 'a) -> 'a

(** This function will invalidate what's necessary from TAST and error caches in [entries],
and decl-caches in [local_memory]. *)
val invalidate_upon_file_changes :
  ctx:Provider_context.t ->
  local_memory:Provider_backend.local_memory ->
  changes:FileInfo.change list ->
  entries:Provider_context.entries ->
  Telemetry.t

(** Construct a [Provider_context.t] from the configuration information
contained within a [ServerEnv.env]. *)
val ctx_from_server_env : ServerEnv.env -> Provider_context.t
