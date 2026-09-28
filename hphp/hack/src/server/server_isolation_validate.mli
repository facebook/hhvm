(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Check whether [files] could be a strict-isolation package, by making them
    one in memory and typechecking.

    Nothing is written to disk: the package is synthesized into a
    [Provider_context], so neither the repository nor [PACKAGES.toml] is
    touched. This exercises the typechecker's own enforcement rather than a
    model of it.

    It is not free of effect on the server, though. Making the candidate a
    package means rebuilding its decls under a configuration the repository does
    not have, in heaps the whole process shares — see
    [Server_isolation_decls.with_repackaged_decls]. Run this on a server given
    over to the analysis, not one also answering a developer's typechecks, and
    restart that server when the run is done.

    Only files the dependency graph says might reference the candidate are
    checked, so a reference it does not record is invisible. This can show a
    candidate is not isolatable; it cannot show that nothing was missed.

    A candidate path the naming table does not know is reported in
    [unknown_files] rather than skipped, which would read as a clean result. *)
val go :
  Server_env.genv ->
  Server_env.env ->
  files:Relative_path.t list ->
  Server_command_types.Isolation_validation.result
