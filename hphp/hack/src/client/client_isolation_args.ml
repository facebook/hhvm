(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

exception Invalid of string

let reject message = raise (Invalid message)

let warn message = Printf.eprintf "warning: %s\n%!" message

let check
    ~max_dependents_given
    Server_isolation_types.
      {
        no_growth;
        output_file = _;
        seed_framework;
        seed_list;
        max_dependents;
        max_cluster_size;
        max_seeds;
        seed_offset;
      } =
  (match max_seeds with
  | Some n when n < 1 ->
    reject (Printf.sprintf "--isolation-max-seeds must be at least 1, got %d" n)
  | _ -> ());
  (match max_cluster_size with
  | Some n when n < 2 ->
    (* A cap of 1 would retire every cluster before it absorbed anything. *)
    reject
      (Printf.sprintf
         "--isolation-max-cluster-size must be at least 2, got %d"
         n)
  | _ -> ());
  if seed_offset < 0 then
    reject
      (Printf.sprintf
         "--isolation-seed-offset cannot be negative, got %d"
         seed_offset);
  (* Growth needs a closure for the seed before it has a cluster, so a reach
     under 1 admits nothing and the run silently reports none. *)
  if max_dependents < 1 then
    reject
      (Printf.sprintf
         "--isolation-max-dependents must be at least 1, got %d"
         max_dependents);
  (* Picking two says the caller means something the run cannot do, and
     preferring one silently answers the wrong question. *)
  if Option.is_some seed_framework && Option.is_some seed_list then
    reject
      "--isolation-seed-framework and --isolation-seed-list both name where to start; pass one";
  (* Nothing grows in this mode, so there is no cluster to cap. *)
  if no_growth && Option.is_some max_cluster_size then
    reject
      "--isolation-max-cluster-size caps a growing cluster, and --isolation-no-growth does not grow one";
  (* Growth clamps the reach to the cap, so a larger one is not refused, it is
     silently unused. Only worth saying when the caller picked the reach: the
     default sits above most caps, and naming a flag they never passed reads as
     an accusation rather than a hint. *)
  (match max_cluster_size with
  | Some cap
    when max_dependents_given && (not no_growth) && max_dependents > cap ->
    warn
      (Printf.sprintf
         "--isolation-max-dependents %d is clamped to --isolation-max-cluster-size %d for growth; a closure larger than the cluster can never be absorbed"
         max_dependents
         cap)
  | _ -> ());
  (* Legal, and almost always a mistake: it skips n and then runs to the end of
     the population, which for a large family is tens of thousands. *)
  if seed_offset > 0 && Option.is_none max_seeds then
    warn
      (Printf.sprintf
         "--isolation-seed-offset %d without --isolation-max-seeds runs to the end of the seed list, not a window of it"
         seed_offset)

let validate ~max_dependents_given options =
  match check ~max_dependents_given options with
  | () -> Ok ()
  | exception Invalid message -> Error message
