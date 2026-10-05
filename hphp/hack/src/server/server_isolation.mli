(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Isolatable clusters: groups of files that reference the monorepo but that
    nothing outside the group references.

    A cluster starts as its seed's inbound closure — a seed from a list or an
    entry-point family may have inbound references, unlike one found by scanning
    — and grows by absorbing files the cluster reaches, either when all their
    dependents are already inside, or together with everything that depends on
    them.

    A grown cluster is disjoint from the others, and unless marked [truncated] it
    is closed under inbound edges: nothing outside references into it. A
    truncated one stopped at the size cap, which is exactly the guarantee it
    drops.

    Under [no_growth] it is the disjointness that is lost, not the closure: each
    set is one seed's closure, so it is closed under inbound edges by
    construction and is never [truncated]. Two of them overlap wherever a file
    depends on both seeds, and one contains another whenever a seed lies inside
    another seed's closure. *)
val go :
  Server_isolation_types.options ->
  Server_env.genv ->
  Server_env.env ->
  Server_isolation_types.result
