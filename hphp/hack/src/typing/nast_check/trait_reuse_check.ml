(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Ban reuse of traits that have final methods. *)

open Hh_prelude
open Aast
open Typing_defs
module Cls = Decl_provider.Class
module SN = Naming_special_names

type tgenv = {
  ctx: Provider_context.t;
  tracing_info: Decl_counters.tracing_info;
}

let strip_ns = Utils.strip_ns

let is_class_kind (k : Ast_defs.classish_kind) : bool = Ast_defs.is_c_class k

let is_trait_kind (k : Ast_defs.classish_kind) : bool = Ast_defs.is_c_trait k

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

(** All the parent classes, used traits, and implemented interfaces of
  [type_name]. This is the flattened inheritance tree. *)
let all_ancestor_names tgenv (type_name : string) : string list =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> Decl_provider.Class.all_ancestor_names decl
  | None -> []

(** Does trait/class [type_name] use [trait_name]? Includes indirect usage. *)
let type_uses_trait tgenv (type_name : string) (trait_name : string) : bool =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> Cls.has_ancestor decl trait_name
  | None -> false

(** Return the position where class/trait [type_name] is defined. *)
let classish_def_pos tgenv type_name : Pos_or_decl.t =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> Cls.pos decl
  | None -> Pos_or_decl.none

(** Return all the ancestors of class/trait [type_name] that use
  [trait_name]. *)
let find_ancestors_using tgenv (type_name : string) (trait_name : string) :
    string list =
  let ancestors = all_ancestor_names tgenv type_name in
  List.filter ancestors ~f:(fun name -> type_uses_trait tgenv name trait_name)

(** Sort a list of class/trait names according to how many ancestors
  they have, in ascending order. *)
let sort_by_num_ancestors tgenv (type_names : string list) : string list =
  let num_ancestors cls_name =
    List.length (all_ancestor_names tgenv cls_name)
  in
  let compare cls_x cls_y =
    Int.compare (num_ancestors cls_x) (num_ancestors cls_y)
  in
  List.sort type_names ~compare

(** Given a non-empty list of class/trait names, return the name with
  the fewest parents. *)
let uppermost_classish tgenv (names : string list) : string =
  List.hd_exn (sort_by_num_ancestors tgenv names)

let lowermost_classish tgenv (names : string list) : string =
  List.hd_exn (List.rev (sort_by_num_ancestors tgenv names))

(** Find the uppermost class in the hierarchy that is using [trait_name],
  directly or via another trait. *)
let find_using_class tgenv cls_name (trait_name : string) : string =
  let using_ancestors = find_ancestors_using tgenv cls_name trait_name in
  let class_ancestors =
    cls_name :: List.filter using_ancestors ~f:(is_class_name tgenv)
  in
  uppermost_classish tgenv class_ancestors

(** Find the first use site in class/trait [type_name] of
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

(** All the inherited properties of [type_name], including static properties. *)
let properties tgenv (type_name : string) :
    (string * Typing_defs.class_elt) list =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> Cls.props decl @ Cls.sprops decl
  | None -> []

(** all the inherited methods of [type_name], both instance and static methods *)
let methods tgenv (type_name : string) : (string * Typing_defs.class_elt) list =
  let decl =
    Decl_provider.get_class ~tracing_info:tgenv.tracing_info tgenv.ctx type_name
  in
  match decl with
  | Some decl -> Cls.methods decl @ Cls.smethods decl
  | None -> []

(** The final methods in [type_name] (excluding inherited methods),
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

(** Return a list showing a route from [type_name] to [trait_name]. If
  multiple routes exist, it prefers the longest one.

    trait One {}
    trait Two { use One; }
    class Three { use Two; }

  The route from Three to One in this example is ["Three"; "Two"; "One"]. *)
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

(** Given a list of [items], apply the function [f] to every two
  adjacent items.

  E.g. for [1; 2; 3] we return [(f 1 2); (f 2 3)]. *)
let rec pairwise_map (items : 'a list) (f : 'a -> 'a -> 'b) : 'b list =
  match items with
  | [] -> []
  | [_] -> []
  | x :: y :: rest -> f x y :: pairwise_map (y :: rest) f

(** Return a list of positions from one class/trait to another. *)
let describe_route tgenv (type_name : string) (trait_name : string) :
    (Pos_or_decl.t * string) list =
  let route = trait_use_route tgenv type_name trait_name in
  pairwise_map route (fun type_name trait_name ->
      ( trait_use_pos tgenv type_name trait_name,
        Printf.sprintf
          "`%s` uses `%s`"
          (strip_ns type_name)
          (strip_ns trait_name) ))

(** Return a list of positions that show how a class ends up using a
  trait via another trait. *)
let class_to_trait_via_trait
    tgenv (base_class : string) (first_trait : string) (reused_trait : string) :
    (Pos_or_decl.t * string) list =
  let result =
    if String.equal first_trait reused_trait then
      []
    else
      describe_route tgenv first_trait reused_trait
  in
  ( trait_use_pos tgenv base_class first_trait,
    Printf.sprintf "`%s` uses `%s`" (strip_ns base_class) (strip_ns first_trait)
  )
  :: result

(** Return a list of positions explaining why a final method is reused. *)
let relevant_positions tgenv using_cls_name used_trait reused_trait :
    (Pos_or_decl.t * string) list =
  let result =
    if String.equal used_trait reused_trait then
      []
    else
      describe_route tgenv used_trait reused_trait
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
  let result = result @ describe_route tgenv using_cls_name reused_trait in

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

(** All the traits that occur as [use Foo;] directly in this class *)
let traits (c : Nast.class_) : (Pos.t * string) list =
  List.filter_map c.c_uses ~f:(fun (pos, trait_hint) ->
      match trait_hint with
      | Happly ((_, trait_name), _) -> Some (pos, trait_name)
      | _ -> None)

let parent_class (c : Nast.class_) : (Pos.t * string) option =
  match c.c_extends with
  | [(_, Happly (pstring, _))] when is_class_kind c.c_kind -> Some pstring
  | _ -> None

(** All the traits that occur as `use Foo;` in this class, plus the
   parent class name (if we have one). *)
let traits_and_parent (c : Nast.class_) : (Pos.t * string) list =
  let traits_used = traits c in
  match parent_class c with
  | Some pstring -> pstring :: traits_used
  | None -> traits_used

(** Given a trait name, return the full list of traits it uses.

    trait One {}
    trait Two { use One; }
    trait Three { use Two; }

  For "Three", we return ["Three"; "Two"; "One"] in some order. *)
let all_trait_ancestors ctx (trait_name : string) : string list =
  let ancestors = all_ancestor_names ctx trait_name in
  (* Trait ancestors can be other traits or interfaces. *)
  let ancestor_traits = List.filter ancestors ~f:(is_trait_name ctx) in
  trait_name :: ancestor_traits

(** Return the union of the two sets given, plus a list of values in common. *)
let union_report_overlaps (s1 : SSet.t) (s2 : SSet.t) : SSet.t * string list =
  SSet.fold
    (fun elt (acc, dupes) ->
      if SSet.mem elt acc then
        (acc, elt :: dupes)
      else
        (SSet.add elt acc, dupes))
    s2
    (s1, [])

(** Return the names of the methods defined locally in class c *)
let local_method_names (c : Nast.class_) : string list =
  List.map c.c_methods ~f:(fun m -> snd m.m_name)

(** Check reuse of trait with a final method *)
let check_reuse_final_method tgenv (c : Nast.class_) : unit =
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
          let cls_name = snd c.c_name in
          let dupe = lowermost_classish tgenv dupes in
          let using_class = find_using_class tgenv cls_name dupe in
          let trace =
            lazy (relevant_positions tgenv using_class type_name dupe)
          in
          Errors.add_typing_error
            Typing_error.(
              primary
              @@ Primary.Trait_reuse_with_final_method
                   {
                     pos = p;
                     trait_name = dupe;
                     parent_cls_name = using_class;
                     trace;
                   });
          SSet.empty)
  in
  ()

(** Check reuse of trait with a final method ignoring diamond inclusion via use chains *)
let check_reuse_final_method_allow_diamond tgenv (c : Nast.class_) : unit =
  match parent_class c with
  | Some (_, pstring) ->
    let traits_with_final_methods_via_parent =
      let trait_ancestors = all_trait_ancestors tgenv pstring in
      List.filter ~f:(has_final_method tgenv) trait_ancestors |> SSet.of_list
    in
    List.iter (traits c) ~f:(fun (p, type_name) ->
        let trait_ancestors = all_trait_ancestors tgenv type_name in
        let traits_with_final_methods =
          List.filter ~f:(has_final_method tgenv) trait_ancestors
          |> SSet.of_list
        in
        let dupes =
          SSet.inter
            traits_with_final_methods_via_parent
            traits_with_final_methods
        in
        if not (SSet.is_empty dupes) then
          let cls_name = snd c.c_name in
          let dupe = lowermost_classish tgenv (SSet.elements dupes) in
          let using_class = find_using_class tgenv cls_name dupe in
          let trace =
            lazy (relevant_positions tgenv using_class type_name dupe)
          in
          Errors.add_typing_error
            Typing_error.(
              primary
              @@ Primary.Trait_reuse_with_final_method
                   {
                     pos = p;
                     trait_name = dupe;
                     parent_cls_name = using_class;
                     trace;
                   }))
  | None -> ()

(** Check reuse of a trait with a non-overridden non-abstract method *)
let check_reuse_method_without_override tgenv (c : Nast.class_) : unit =
  let local_method_names = SSet.of_list (local_method_names c) in
  (* [seen_method_origins] maps origins of methods from traits to the trait
   * the method comes from. E.g. for T a trait used directly in C, m a method from
   * T and O the origin of m, [seen_method_origins] will map O to T. *)
  let seen_method_origins = ref SMap.empty in
  List.iter (traits c) ~f:(fun (_, used_trait_name) ->
      let used_trait_methods_origins = ref SSet.empty in
      let all_trait_ancestors =
        SSet.of_list (all_trait_ancestors tgenv used_trait_name)
      in
      List.iter (methods tgenv used_trait_name) ~f:(fun (m_name, m) ->
          if
            SSet.mem m.ce_origin all_trait_ancestors
            && (not (get_ce_abstract m))
            && not (get_ce_final m)
          then
            (* If we've seen a path to this before, we're reusing that trait. *)
            if
              (* if we haven't complained about this already *)
              (not (SSet.mem m.ce_origin !used_trait_methods_origins))
              && (* and we've seen the origin before via a different path *)
              SMap.mem m.ce_origin !seen_method_origins
            then (
              if not (SSet.mem m_name local_method_names) then
                (* The reused trait has a method that we haven't overridden in this trait/class. *)
                let (cls_pos, cls_name) = c.c_name in
                let trace1 =
                  lazy
                    (class_to_trait_via_trait
                       tgenv
                       cls_name
                       used_trait_name
                       m.ce_origin)
                in
                let trace2 =
                  lazy
                    (class_to_trait_via_trait
                       tgenv
                       cls_name
                       (SMap.find m.ce_origin !seen_method_origins)
                       m.ce_origin)
                in
                Errors.add_typing_error
                  Typing_error.(
                    primary
                    @@ Primary.Method_import_via_diamond
                         {
                           pos = cls_pos;
                           class_name = cls_name;
                           method_pos = force m.ce_pos;
                           method_name = m_name;
                           trace1;
                           trace2;
                         })
            ) else begin
              (* We haven't seen a path to this trait before, record it. *)
              seen_method_origins :=
                SMap.add m.ce_origin used_trait_name !seen_method_origins;
              used_trait_methods_origins :=
                SSet.add m.ce_origin !used_trait_methods_origins
            end))

(** With diamond inclusion of traits, it is possible to write

    trait Tr<T> { .. }
    trait T1 { use Tr<int>; ..}
    trait T2 { use Tr<string>; .. }
    class C { use T1, T2; .. }

  This is sound only if trait Tr has no mutable state of type T.

  The check_diamond_import_property function below ensures that all properties
  of reused traits do not define parametric properties. *)
let require_value_restriction t =
  Option.is_some (Typing_utils.contains_generic_decl t)

let check_diamond_import_property tgenv (c : Nast.class_) : unit =
  (* For each trait t used by c, we update the map below, storing for each property p in t
   * its origin and the fact that it was first imported via t
   * So seen_properties_origins: property_name -> (origin, trait_name)
   * If a property with a generic type is encountered via the traits t1 and t2 used by c,
   * and the two instances of the property same origin, then report an error.
   *)
  let seen_properties_origins = ref SMap.empty in

  List.iter (traits c) ~f:(fun (_, used_trait_name) ->
      let all_trait_ancestors =
        SSet.of_list (all_trait_ancestors tgenv used_trait_name)
      in
      List.iter (properties tgenv used_trait_name) ~f:(fun (p_name, p) ->
          if SSet.mem p.ce_origin all_trait_ancestors then
            match SMap.find_opt p_name !seen_properties_origins with
            | Some (origin, trait) when String.equal origin p.ce_origin ->
              (match
                 Decl_provider.get_class
                   ~tracing_info:tgenv.tracing_info
                   tgenv.ctx
                   origin
               with
              | Some class_decl ->
                (match Cls.get_prop class_decl p_name with
                | Some class_elt ->
                  if require_value_restriction (Lazy.force class_elt.ce_type)
                  then
                    (* A property with a generic type is defined in a trait that *)
                    let (cls_pos, cls_name) = c.c_name in
                    let trace1 =
                      lazy
                        (class_to_trait_via_trait
                           tgenv
                           cls_name
                           used_trait_name
                           p.ce_origin)
                    in
                    let trace2 =
                      lazy
                        (class_to_trait_via_trait
                           tgenv
                           cls_name
                           trait
                           p.ce_origin)
                    in
                    let p_pos = Lazy.force p.ce_type |> get_pos in
                    Errors.add_typing_error
                      Typing_error.(
                        primary
                        @@ Primary.Generic_property_import_via_diamond
                             {
                               pos = cls_pos;
                               class_name = cls_name;
                               property_pos = p_pos;
                               property_name = p_name;
                               trace1;
                               trace2;
                             })
                | None -> ())
              | None -> ())
            | _ ->
              seen_properties_origins :=
                SMap.add
                  p_name
                  (p.ce_origin, used_trait_name)
                  !seen_properties_origins))

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

      check_diamond_import_property tgenv c;
      if
        Naming_attributes.mem
          SN.UserAttributes.uaEnableMethodTraitDiamond
          c.c_user_attributes
      then
        check_reuse_final_method_allow_diamond tgenv c
      else begin
        check_reuse_final_method tgenv c;
        check_reuse_method_without_override tgenv c
      end
  end
