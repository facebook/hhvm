(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Cls = Decl_provider.Class
module Env = Typing_env

(* Find dependency routes between classish types.

   This is used in error reporting, to show how a class ultimately
   inherited something (e.g. an abstract method) that caused a type
   error. *)

let get_class env name =
  Decl_provider.get_class
    ?tracing_info:(Env.get_tracing_info env)
    (Env.get_ctx env)
    name

(** All the parent classes, used traits, and implemented interfaces of
  [type_name]. This is the flattened inheritance tree. *)
let all_ancestor_names env (type_name : string) : string list =
  let decl = get_class env type_name in
  match decl with
  | Decl_entry.Found decl -> Cls.all_ancestor_names decl
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    []

let has_ancestor env (classish_name : string) (ancestor_name : string) : bool =
  match get_class env classish_name with
  | Decl_entry.Found decl -> Cls.has_ancestor decl ancestor_name
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    false

(** Return the ancestors of [classish_name] that also depend on [ancestor_name]. *)
let find_ancestors_using env (classish_name : string) (ancestor_name : string) :
    string list =
  let ancestors = all_ancestor_names env classish_name in
  List.filter ancestors ~f:(fun name -> has_ancestor env name ancestor_name)

(** Sort a list of classish names according to how many ancestors they
    have, in ascending order. *)
let sort_by_num_ancestors env (names : string list) : string list =
  let num_ancestors cls_name = List.length (all_ancestor_names env cls_name) in
  let compare cls_x cls_y =
    Int.compare (num_ancestors cls_x) (num_ancestors cls_y)
  in
  List.dedup_and_sort names ~compare

(** Return a list showing a route from [classish] name to [ancestor]
    name. If multiple routes exist, return the longest one.

    trait One {}
    trait Two { use One; }
    class Three { use Two; }

    The route from Three to One in this example is
    ["Three"; "Two"; "One"]. *)
let find_route env ~classish ~ancestor : string list =
  let rec classish_between classish ancestor seen : string list =
    if SSet.mem classish seen then
      (* The class hierarchy is bad: it contains a cycle. Ensure we
         still terminate. *)
      []
    else
      (* Find all the ancestors that use this trait. *)
      let ancestors = find_ancestors_using env classish ancestor in
      (* Choose the ancestor closest to [classish] by taking the ancestor
         that has the most ancestors itself. *)
      let sorted_ancestors = List.rev (sort_by_num_ancestors env ancestors) in
      match sorted_ancestors with
      | [] -> []
      | [ancestor] -> [ancestor]
      | anc :: _ ->
        anc :: classish_between anc ancestor (SSet.add classish seen)
  in
  [classish] @ classish_between classish ancestor SSet.empty @ [ancestor]

(** Find the position in [classish_name] that means it depends on
    [ancestor_name].

    For example, if [ancestor_name] is a trait, this will return the
   `use Foo;` position.  *)
let reason_pos env (classish_name : string) (ancestor_name : string) :
    Pos_or_decl.t =
  match get_class env classish_name with
  | Decl_entry.Found decl ->
    (match Cls.get_ancestor decl ancestor_name with
    | Some trait_ty -> Typing_defs_core.get_pos trait_ty
    | None -> Pos_or_decl.none)
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    Pos_or_decl.none

(** Given a list of [items], apply the function [f] to every two
  adjacent items.

  E.g. for [1; 2; 3] we return [(f 1 2); (f 2 3)]. *)
let rec pairwise_map (items : 'a list) (f : 'a -> 'a -> 'b) : 'b list =
  match items with
  | [] -> []
  | [_] -> []
  | x :: y :: rest -> f x y :: pairwise_map (y :: rest) f

(** Return a list of positions from [classish] name to [ancestor] name. *)
let describe_route
    (env : Typing_env_types.env) ~(classish : string) ~(ancestor : string) :
    (Pos_or_decl.t * string) list =
  let route = find_route env ~classish ~ancestor in
  pairwise_map route (fun classish parent ->
      ( reason_pos env classish parent,
        Printf.sprintf
          "`%s` uses `%s`"
          (Utils.strip_ns classish)
          (Utils.strip_ns parent) ))

(** Show that [classish] uses [via], and the full path from [via] to
   [ancestor]. *)
let describe_route_via
    env ~(classish : string) ~(ancestor : string) ~(via : string) :
    (Pos_or_decl.t * string) list =
  let result =
    if String.equal via ancestor then
      []
    else
      describe_route env ~classish:via ~ancestor
  in
  ( reason_pos env classish via,
    Printf.sprintf
      "`%s` uses `%s`"
      (Utils.strip_ns classish)
      (Utils.strip_ns via) )
  :: result
