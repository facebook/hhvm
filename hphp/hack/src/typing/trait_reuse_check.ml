(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Ban reuse of traits that have final methods. *)

open Hh_prelude
module Cls = Decl_provider.Class
module Env = Typing_env

let strip_ns = Utils.strip_ns

let get_class env name =
  Decl_provider.get_class
    ?tracing_info:(Env.get_tracing_info env)
    (Env.get_ctx env)
    name

let is_class env name =
  match get_class env name with
  | None -> false
  | Some c -> Cls.kind c |> Ast_defs.is_c_class

(** All the parent classes, used traits, and implemented interfaces of
  [type_name]. This is the flattened inheritance tree. *)
let all_ancestor_names env (type_name : string) : string list =
  let decl = get_class env type_name in
  match decl with
  | Some decl -> Decl_provider.Class.all_ancestor_names decl
  | None -> []

(** Does trait/class [type_name] use [trait_name]? Includes indirect usage. *)
let type_uses_trait env (type_name : string) (trait_name : string) : bool =
  let decl = get_class env type_name in
  match decl with
  | Some decl -> Cls.has_ancestor decl trait_name
  | None -> false

(** Return the position where class/trait [type_name] is defined. *)
let classish_def_pos env type_name : Pos_or_decl.t =
  let decl = get_class env type_name in
  match decl with
  | Some decl -> Cls.pos decl
  | None -> Pos_or_decl.none

(** Return all the ancestors of class/trait [type_name] that use
  [trait_name]. *)
let find_ancestors_using env (type_name : string) (trait_name : string) :
    string list =
  let ancestors = all_ancestor_names env type_name in
  List.filter ancestors ~f:(fun name -> type_uses_trait env name trait_name)

(** Sort a list of class/trait names according to how many ancestors
  they have, in ascending order. *)
let sort_by_num_ancestors env (type_names : string list) : string list =
  let num_ancestors cls_name = List.length (all_ancestor_names env cls_name) in
  let compare cls_x cls_y =
    Int.compare (num_ancestors cls_x) (num_ancestors cls_y)
  in
  List.dedup_and_sort type_names ~compare

(** Find the first use site in class/trait [type_name] of
  [trait_name], directly or via another trait. *)
let trait_use_pos env (type_name : string) (trait_name : string) : Pos_or_decl.t
    =
  let decl = get_class env type_name in
  match decl with
  | Some decl ->
    let trait_ty = Cls.get_ancestor decl trait_name in
    (match trait_ty with
    | Some trait_ty -> Typing_defs_core.get_pos trait_ty
    | None -> Pos_or_decl.none)
  | None -> Pos_or_decl.none

(** The final methods in [type_name] (excluding inherited methods),
  both instance and static methods. *)
let final_methods env (type_name : string) :
    (string * Typing_defs.class_elt) list =
  let decl = get_class env type_name in
  match decl with
  | Some decl ->
    let methods = Cls.methods decl @ Cls.smethods decl in
    List.filter
      ~f:(fun (_, m) ->
        String.equal m.Typing_defs.ce_origin type_name
        && Typing_defs.get_ce_final m)
      methods
  | None -> []

(** Return a list showing a route from [type_name] to [trait_name]. If
  multiple routes exist, it prefers the longest one.

    trait One {}
    trait Two { use One; }
    class Three { use Two; }

  The route from Three to One in this example is ["Three"; "Two"; "One"]. *)
let trait_use_route env type_name trait_name : string list =
  let rec traits_between type_name trait_name seen : string list =
    if SSet.mem type_name seen then
      (* The code is bad: it has a cyclic trait definition. Ensure
         this check still terminates. *)
      []
    else
      (* Find all the ancestors that use this trait. *)
      let ancestors = find_ancestors_using env type_name trait_name in
      (* Choose the ancestor closest to [type_name] by taking the ancestor
         that has the most ancestors itself. *)
      let sorted_ancestors = List.rev (sort_by_num_ancestors env ancestors) in
      match sorted_ancestors with
      | [] -> []
      | [ancestor] -> [ancestor]
      | anc :: _ ->
        anc :: traits_between anc trait_name (SSet.add type_name seen)
  in
  [type_name] @ traits_between type_name trait_name SSet.empty @ [trait_name]

(** Given a list of [items], apply the function [f] to every two
  adjacent items.

  E.g. for [1; 2; 3] we return [(f 1 2); (f 2 3)]. *)
let rec pairwise_map (items : 'a list) (f : 'a -> 'a -> 'b) : 'b list =
  match items with
  | [] -> []
  | [_] -> []
  | x :: y :: rest -> f x y :: pairwise_map (y :: rest) f

(** Return a list of positions from one class/trait to another. *)
let describe_route
    (env : Typing_env_types.env) (type_name : string) (trait_name : string) :
    (Pos_or_decl.t * string) list =
  let route = trait_use_route env type_name trait_name in
  pairwise_map route (fun type_name trait_name ->
      ( trait_use_pos env type_name trait_name,
        Printf.sprintf
          "`%s` uses `%s`"
          (strip_ns type_name)
          (strip_ns trait_name) ))

(** Return a list of positions that show how a class ends up using a
  trait via another trait. *)
let class_to_trait_via_trait
    env ~(class_name : string) ~(trait_name : string) ~(via_trait : string) :
    (Pos_or_decl.t * string) list =
  let result =
    if String.equal via_trait trait_name then
      []
    else
      describe_route env via_trait trait_name
  in
  ( trait_use_pos env class_name via_trait,
    Printf.sprintf "`%s` uses `%s`" (strip_ns class_name) (strip_ns via_trait)
  )
  :: result

(** Return a list of positions explaining why a final method is reused. *)
let relevant_positions
    (env : Typing_env_types.env)
    ~class_name
    ~first_using_parent_or_trait_name
    ~second_using_trait_name
    ~reused_trait : string * (Pos_or_decl.t * string) list =
  let route =
    trait_use_route env first_using_parent_or_trait_name reused_trait
  in
  let using_cls_name =
    match List.find (List.rev route) ~f:(is_class env) with
    | None -> class_name
    | Some c -> c
  in
  let result =
    if String.equal second_using_trait_name reused_trait then
      []
    else
      describe_route env second_using_trait_name reused_trait
  in

  (* Show the position of the ancestor class that also uses this trait. *)
  let result =
    result
    @ [
        ( classish_def_pos env using_cls_name,
          Printf.sprintf "`%s` is defined here" (strip_ns using_cls_name) );
      ]
  in

  (* Since traits can use multiple other traits, show the full trait
     path so users can see how the trait reuse occurred. *)
  let result = result @ describe_route env using_cls_name reused_trait in

  (* Finally, show the final method in the trait. *)
  let (meth_name, meth) = List.hd_exn (final_methods env reused_trait) in
  ( using_cls_name,
    result
    @ [
        ( Lazy.force meth.Typing_defs.ce_pos,
          Printf.sprintf
            "`%s` has a final method `%s`"
            (strip_ns reused_trait)
            meth_name );
      ] )

let trait_reuse_with_final_method_error
    env
    class_elt
    ~class_name
    ~first_using_parent_or_trait
    ~second_using_trait:(second_using_trait_pos, second_using_trait) =
  let trait_name = class_elt.Typing_defs.ce_origin in
  let first_using_parent_or_trait_name = Cls.name first_using_parent_or_trait in
  let second_using_trait_name = Cls.name second_using_trait in
  let trace =
    lazy
      (relevant_positions
         env
         ~class_name
         ~first_using_parent_or_trait_name
         ~second_using_trait_name
         ~reused_trait:trait_name)
  in
  Typing_error.(
    primary
    @@ Primary.Trait_reuse_with_final_method
         {
           pos = second_using_trait_pos;
           trait_name;
           parent_cls_name = Lazy.map trace ~f:fst;
           trace = Lazy.map trace ~f:snd;
         })

let method_import_via_diamond_error
    env
    (class_name_pos, class_name)
    (method_name, class_elt)
    ~first_using_trait
    ~second_using_trait =
  let trait_name = class_elt.Typing_defs.ce_origin in
  Typing_error.(
    primary
    @@ Primary.Method_import_via_diamond
         {
           pos = class_name_pos;
           class_name;
           method_pos = force class_elt.Typing_defs.ce_pos;
           method_name;
           trace1 =
             lazy
               (class_to_trait_via_trait
                  env
                  ~class_name
                  ~trait_name
                  ~via_trait:(Cls.name second_using_trait));
           trace2 =
             lazy
               (class_to_trait_via_trait
                  env
                  ~class_name
                  ~trait_name
                  ~via_trait:(Cls.name first_using_trait));
         })

let generic_property_import_via_diamond_error
    env
    (class_name_pos, class_name)
    (property_name, class_elt)
    ~first_using_trait
    ~second_using_trait =
  let trait_name = class_elt.Typing_defs.ce_origin in
  Errors.add_typing_error
    Typing_error.(
      primary
      @@ Primary.Generic_property_import_via_diamond
           {
             pos = class_name_pos;
             class_name;
             property_pos = Lazy.force class_elt.Typing_defs.ce_pos;
             property_name;
             trace1 =
               lazy
                 (class_to_trait_via_trait
                    env
                    ~class_name
                    ~trait_name
                    ~via_trait:(Cls.name second_using_trait));
             trace2 =
               lazy
                 (class_to_trait_via_trait
                    env
                    ~class_name
                    ~trait_name
                    ~via_trait:(Cls.name first_using_trait));
           })
