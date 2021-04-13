(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Ban reuse of traits that have final methods. *)

open Hh_prelude
open Aast
module Cls = Decl_provider.Class

type tgenv = {
  ctx: Provider_context.t;
  tracing_info: Decl_counters.tracing_info;
}

let strip_ns = Utils.strip_ns

let is_class_kind (k : Ast_defs.class_kind) : bool =
  match k with
  | Ast_defs.Cabstract -> true
  | Ast_defs.Cnormal -> true
  | _ -> false

let is_trait_kind (k : Ast_defs.class_kind) : bool =
  match k with
  | Ast_defs.Ctrait -> true
  | _ -> false

let is_trait_name tgenv (type_name : string) : bool =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> is_trait_kind (Cls.kind decl)
  | None -> false

let is_class_name tgenv (type_name : string) : bool =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> is_class_kind (Cls.kind decl)
  | None -> false

(* All the parent classes, used traits, and implemented interfaces of
   [type_name]. This is the flattened inheritance tree. *)
let all_ancestor_names tgenv (type_name : string) : string list =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> Decl_provider.Class.all_ancestor_names decl
  | None -> []

(* Does trait/class [type_name] use [trait_name]? Includes indirect
   usage. *)
let type_uses_trait tgenv (type_name : string) (trait_name : string) : bool =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> Cls.has_ancestor decl trait_name
  | None -> false

(* Return the position where class/trait [type_name] is defined. *)
let classish_def_pos tgenv type_name : Pos_or_decl.t =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> Cls.pos decl
  | None -> Pos_or_decl.none

(* Return all the ancestors of class/trait [type_name] that use
   [trait_name]. *)
let find_ancestors_using tgenv (type_name : string) (trait_name : string) :
    string list =
  let ancestors = all_ancestor_names tgenv type_name in
  List.filter ancestors ~f:(fun name -> type_uses_trait tgenv name trait_name)

(* Sort a list of class/trait names according to how many ancestors
   they have, in ascending order. *)
let sort_by_num_ancestors tgenv (type_names : string list) : string list =
  let num_ancestors cls_name =
    List.length (all_ancestor_names tgenv cls_name)
  in
  let compare cls_x cls_y =
    Int.compare (num_ancestors cls_x) (num_ancestors cls_y)
  in
  List.sort type_names ~compare

(* Given a non-empty list of class/trait names, return the name with
   the fewest parents. *)
let uppermost_classish tgenv (names : string list) : string =
  List.hd_exn (sort_by_num_ancestors tgenv names)

let lowermost_classish tgenv (names : string list) : string =
  List.hd_exn (List.rev (sort_by_num_ancestors tgenv names))

(* Find the uppermost class in the hierarchy that is using [trait_name],
   directly or via another trait. *)
let find_using_class tgenv cls_name (trait_name : string) : string =
  let using_ancestors = find_ancestors_using tgenv cls_name trait_name in
  let class_ancestors =
    cls_name :: List.filter using_ancestors ~f:(is_class_name tgenv)
  in
  uppermost_classish tgenv class_ancestors

(* Find the first use site in class/trait [type_name] of
   [trait_name], directly or via another trait. *)
let trait_use_pos tgenv (type_name : string) (trait_name : string) :
    Pos_or_decl.t =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl ->
    let trait_ty = Cls.get_ancestor decl trait_name in
    (match trait_ty with
    | Some trait_ty -> Typing_defs_core.get_pos trait_ty
    | None -> Pos_or_decl.none)
  | None -> Pos_or_decl.none

(* The final methods in [type_name] (excluding inherited methods),
   both instance and static methods. *)
let final_methods tgenv (type_name : string) :
    (string * Typing_defs.class_elt) list =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl ->
    let methods = Cls.methods decl @ Cls.smethods decl in
    List.filter
      ~f:(fun (_, m) ->
        String.equal m.Typing_defs.ce_origin type_name
        && Typing_defs.get_ce_final m)
      methods
  | None -> []

let has_final_method tgenv (type_name : string) : bool =
  match final_methods tgenv type_name with
  | _ :: _ -> true
  | [] -> false

(* Return a list showing a route from [type_name] to [trait_name]. If
 * multiple routes exist, prefer the longest one.
 *
 * trait One {}
 * trait Two { use One; }
 * class Three { use Two; }
 *
 * The route from Three to One in this example is ["Three"; "Two"; "One"].
 *)
let trait_use_route tgenv type_name trait_name : string list =
  let rec traits_between type_name trait_name seen : string list =
    if SSet.mem type_name seen then
      (* The code is bad: it has a cyclic trait definition. Ensure
         this check still terminates. *)
      []
    else
      (* Find all the ancestors that use this trait. *)
      let ancestors = find_ancestors_using tgenv type_name trait_name in
      (* Choose the ancestor closest to [type_name] by taking the ancestor
     that has the most ancestors itself. *)
      let sorted_ancestors = List.rev (sort_by_num_ancestors tgenv ancestors) in
      match sorted_ancestors with
      | [] -> []
      | [ancestor] -> [ancestor]
      | anc :: _ ->
        anc :: traits_between anc trait_name (SSet.add type_name seen)
  in
  [type_name] @ traits_between type_name trait_name SSet.empty @ [trait_name]

(* Given a list of [items], apply the function [f] to every two
 * adjacent items.
 *
 * E.g. for [1; 2; 3] we return [(f 1 2); (f 2 3)].
 *)
let rec pairwise_map (items : 'a list) (f : 'a -> 'a -> 'b) : 'b list =
  match items with
  | [] -> []
  | [_] -> []
  | x :: y :: rest -> f x y :: pairwise_map (y :: rest) f

(* Return a list of positions that show how we ended up using this
   trait. *)
let relevant_positions tgenv using_cls_name used_trait reused_trait :
    (Pos_or_decl.t * string) list =
  let describe_route (type_name : string) (trait_name : string) :
      (Pos_or_decl.t * string) list =
    let route = trait_use_route tgenv type_name trait_name in
    pairwise_map route (fun type_name trait_name ->
        ( trait_use_pos tgenv type_name trait_name,
          Printf.sprintf
            "`%s` uses `%s`"
            (strip_ns type_name)
            (strip_ns trait_name) ))
  in

  let result =
    if String.equal used_trait reused_trait then
      []
    else
      describe_route used_trait reused_trait
  in

  (* Show the position of the ancestor class that also uses this trait. *)
  let result =
    result
    @ [
        ( classish_def_pos tgenv using_cls_name,
          Printf.sprintf "`%s` is defined here" (strip_ns using_cls_name) );
      ]
  in

  (* Since traits can use multiple other traits, show the full trait
     path so users can see how the trait reuse occurred. *)
  let result = result @ describe_route using_cls_name reused_trait in

  (* Finally, show the final method in the trait. *)
  let (meth_name, meth) = List.hd_exn (final_methods tgenv reused_trait) in
  result
  @ [
      ( Lazy.force meth.Typing_defs.ce_pos,
        Printf.sprintf
          "`%s` has a final method `%s`"
          (strip_ns reused_trait)
          meth_name );
    ]

(* All the traits that occur as `use Foo;` in this class, plus the
   parent class name (if we have one). *)
let traits_and_parent (c : Nast.class_) : (Pos.t * string) list =
  let traits_used =
    List.filter_map c.c_uses ~f:(fun (pos, trait_hint) ->
        match trait_hint with
        | Happly ((_, trait_name), _) -> Some (pos, trait_name)
        | _ -> None)
  in
  match c.c_extends with
  | [(_, Happly (pstring, _))] when is_class_kind c.c_kind ->
    pstring :: traits_used
  | _ -> traits_used

(* Given a trait name, return the full list of traits it uses.
 *
 * trait One {}
 * trait Two { use One; }
 * trait Three { use Two; }
 *
 * For "Three", we return ["Three"; "Two"; "One"] in some order.
 *)
let all_trait_ancestors ctx (trait_name : string) : string list =
  let ancestors = all_ancestor_names ctx trait_name in
  (* Trait ancestors can be other traits or interfaces. *)
  let ancestor_traits = List.filter ancestors ~f:(is_trait_name ctx) in
  trait_name :: ancestor_traits

(* Return the union of the two sets given, plus a list of values in common. *)
let union_report_overlaps (s1 : SSet.t) (s2 : SSet.t) : SSet.t * string list =
  SSet.fold
    (fun elt (acc, dupes) ->
      if SSet.mem elt acc then
        (acc, elt :: dupes)
      else
        (SSet.add elt acc, dupes))
    s2
    (s1, [])

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_class_ env c =
      let tgenv =
        {
          ctx = env.Nast_check_env.ctx;
          tracing_info =
            {
              Decl_counters.origin = Decl_counters.NastCheck;
              file = fst c.c_name |> Pos.filename;
            };
        }
      in
      let cls_name = snd c.c_name in

      let (_ : SSet.t) =
        List.fold
          (traits_and_parent c)
          ~init:SSet.empty
          ~f:(fun seen_traits (p, type_name) ->
            let trait_ancestors = all_trait_ancestors tgenv type_name in
            let traits_with_final_meths =
              List.filter ~f:(has_final_method tgenv) trait_ancestors
              |> SSet.of_list
            in
            match union_report_overlaps seen_traits traits_with_final_meths with
            | (seen_traits, []) -> seen_traits
            | (_, dupes) ->
              let dupe = lowermost_classish tgenv dupes in
              let using_class = find_using_class tgenv cls_name dupe in
              let trace = relevant_positions tgenv using_class type_name dupe in
              Errors.trait_reuse_with_final_method p dupe using_class trace;
              SSet.empty)
      in
      ()
  end
