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
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    false
  | Decl_entry.Found c -> Cls.kind c |> Ast_defs.is_c_class

(** Return the position where class/trait [type_name] is defined. *)
let classish_def_pos env type_name : Pos_or_decl.t =
  let decl = get_class env type_name in
  match decl with
  | Decl_entry.Found decl -> Cls.pos decl
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    Pos_or_decl.none

(** The final methods in [type_name] (excluding inherited methods),
  both instance and static methods. *)
let final_methods env (type_name : string) :
    (string * Typing_defs.class_elt) list =
  let decl = get_class env type_name in
  match decl with
  | Decl_entry.Found decl ->
    let methods = Cls.methods decl @ Cls.smethods decl in
    List.filter
      ~f:(fun (_, m) ->
        String.equal m.Typing_defs.ce_origin type_name
        && Typing_defs.get_ce_final m)
      methods
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    []

(** Return a list of positions explaining why a final method is reused. *)
let relevant_positions
    (env : Typing_env_types.env)
    ~class_name
    ~first_using_parent_or_trait_name
    ~second_using_trait_name
    ~reused_trait : string * (Pos_or_decl.t * string) list =
  let route =
    Ancestor_route.find_route
      env
      ~classish:first_using_parent_or_trait_name
      ~ancestor:reused_trait
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
      Ancestor_route.describe_route
        env
        ~classish:second_using_trait_name
        ~ancestor:reused_trait
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
  let result =
    result
    @ Ancestor_route.describe_route
        env
        ~classish:using_cls_name
        ~ancestor:reused_trait
  in

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
               (Ancestor_route.describe_route_via
                  env
                  ~classish:class_name
                  ~ancestor:trait_name
                  ~via:(Cls.name second_using_trait));
           trace2 =
             lazy
               (Ancestor_route.describe_route_via
                  env
                  ~classish:class_name
                  ~ancestor:trait_name
                  ~via:(Cls.name first_using_trait));
         })

let property_import_via_diamond_error
    ~generic
    env
    (class_name_pos, class_name)
    (property_name, class_elt)
    ~first_using_trait
    ~second_using_trait =
  let trait_name = class_elt.Typing_defs.ce_origin in
  Typing_error_utils.add_typing_error
    ~env
    Typing_error.(
      primary
      @@ Primary.Property_import_via_diamond
           {
             generic;
             pos = class_name_pos;
             class_name;
             property_pos =
               Typing_defs.get_pos (Lazy.force class_elt.Typing_defs.ce_type);
             property_name;
             trace1 =
               lazy
                 (Ancestor_route.describe_route_via
                    env
                    ~classish:class_name
                    ~ancestor:trait_name
                    ~via:(Cls.name second_using_trait));
             trace2 =
               lazy
                 (Ancestor_route.describe_route_via
                    env
                    ~classish:class_name
                    ~ancestor:trait_name
                    ~via:(Cls.name first_using_trait));
           })
