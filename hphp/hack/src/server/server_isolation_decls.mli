(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** [with_repackaged_decls ctx naming_table files ~f] runs [f] with the decls of
    [files] rebuilt under the package configuration [ctx] carries, then puts the
    heaps back as they were.

    A symbol's package is a field on its decl, written when the file was parsed,
    so adding a package to a context changes nothing until those decls go;
    removing them makes the next lookup recompute from the context that asked.

    {2 Only for a server dedicated to this analysis}

    The decl heaps belong to the whole process. While [f] runs, the candidate's
    symbols are either absent or carry a package no repository declares, so
    anything else in the server that reads them in that window sees something
    the repository does not say. Do not run this on a server that is also
    answering a developer's typechecks; give the analysis its own server, and
    restart it afterwards rather than carrying the run's residue into whatever
    that server does next.

    {2 What restoring does and does not cover}

    Restoration covers the ordinary path and an exception out of [f]: a
    local-changes stack per heap the backend uses, popped afterwards, and a
    second deletion for any heap that stack does not cover. It does not cover
    the process dying mid-run, which leaves the stacks pushed and the decls
    missing — the reason to restart rather than trust the server.

    Raises unless [allow_repackaging] is set, which the caller takes from
    [isolation_allow_decl_repackaging] in the server config. The warning above
    is a convention until an operator opts in; this makes it one the server
    enforces.

    Raises on a backend whose heaps it does not know, before removing anything,
    rather than deleting decls it has no way to put back.

    The process-local caches above the heaps are invalidated before [f] runs as
    well as after: they hold folded classes carrying the package they were
    folded under, which no local-changes stack reaches.

    [Typing_deps.trace] is off throughout: edges found while typechecking are
    appended to the dependency graph, which no local-changes stack covers and no
    restart short of a fresh init will undo. *)
val with_repackaged_decls :
  Provider_context.t ->
  Naming_table.t ->
  Relative_path.t list ->
  allow_repackaging:bool ->
  f:(unit -> 'a) ->
  'a
