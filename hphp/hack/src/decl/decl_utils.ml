(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections
open Aast
open Typing_defs
open Hh_prelude
module SN = Naming_special_names

let unwrap_class_hint = function
  | (_, Happly ((pos, class_name), type_parameters)) ->
    ((pos, class_name, type_parameters), None)
  | (p, Habstr _) ->
    let err =
      Typing_error.(
        primary
        @@ Primary.Expected_class
             { suffix = Some (lazy " or interface but got a generic"); pos = p })
    in
    ((Pos.none, "", []), Some err)
  | (p, _) ->
    let err =
      Typing_error.(
        primary
        @@ Primary.Expected_class
             { suffix = Some (lazy " or interface"); pos = p })
    in
    ((Pos.none, "", []), Some err)

let unwrap_class_type ty : _ Reason.t_ * pos_id * decl_ty list =
  match deref ty with
  | (r, Tapply (name, tparaml)) -> (r, name, tparaml)
  | (r, Tgeneric _) ->
    let p = Typing_reason.to_pos r in
    (r, (p, ""), [])
  | (r, _) ->
    let p = Typing_reason.to_pos r in
    (r, (p, ""), [])

let parentish_names
    {
      Shallow_decl_defs.sc_mode = _;
      sc_final = _;
      sc_abstract = _;
      sc_is_xhp = _;
      sc_internal = _;
      sc_has_xhp_keyword = _;
      sc_kind = _;
      sc_module = _;
      sc_name = _;
      sc_tparams = _;
      sc_extends;
      sc_uses;
      sc_xhp_attr_uses;
      sc_xhp_enum_values = _;
      sc_xhp_marked_empty = _;
      sc_req_extends;
      sc_req_implements;
      sc_req_constraints = _;
      sc_implements;
      sc_support_dynamic_type = _;
      sc_consts = _;
      sc_typeconsts = _;
      sc_props = _;
      sc_sprops = _;
      sc_constructor = _;
      sc_static_methods = _;
      sc_methods = _;
      sc_user_attributes = _;
      sc_enum_type = _;
      sc_docs_url = _;
      sc_package = _;
    } =
  let get_names tys acc =
    List.fold tys ~init:acc ~f:(fun acc ty ->
        let (_, (_, name), _) = unwrap_class_type ty in
        SSet.add acc name)
  in
  SSet.empty
  |> get_names sc_extends
  |> get_names sc_implements
  |> get_names sc_uses
  |> get_names sc_req_extends
  |> get_names sc_req_implements
  |> get_names sc_xhp_attr_uses

(** Given sets A and B return a tuple (AnB, A\B), i.e split A into the part
    that is common with B, and which is unique to A *)
let split_sets defs split_if_in_defs =
  SSet.partition (SSet.mem split_if_in_defs) defs

(** Given name sets A and B return a tuple (AnB, A\B), i.e split A into the part
    that is common with B, and which is unique to A *)
let split_defs defs split_if_in_defs =
  FileInfo.(
    let (n_funs1, n_funs2) = split_sets defs.n_funs split_if_in_defs.n_funs in
    let (n_classes1, n_classes2) =
      split_sets defs.n_classes split_if_in_defs.n_classes
    in
    let (n_types1, n_types2) =
      split_sets defs.n_types split_if_in_defs.n_types
    in
    let (n_consts1, n_consts2) =
      split_sets defs.n_consts split_if_in_defs.n_consts
    in
    let (n_modules1, n_modules2) =
      split_sets defs.n_modules split_if_in_defs.n_modules
    in
    let r1 =
      {
        n_funs = n_funs1;
        n_classes = n_classes1;
        n_types = n_types1;
        n_consts = n_consts1;
        n_modules = n_modules1;
      }
    in
    let r2 =
      {
        n_funs = n_funs2;
        n_classes = n_classes2;
        n_types = n_types2;
        n_consts = n_consts2;
        n_modules = n_modules2;
      }
    in
    (r1, r2))

let infer_const expr_ =
  match expr_ with
  | Aast.String _ -> Some Tstring
  | True
  | False ->
    Some Tbool
  | Int _ -> Some Tint
  | Float _ -> Some Tfloat
  | Null -> Some Tnull
  | Unop ((Ast_defs.Uminus | Ast_defs.Uplus), (_, _, Int _)) -> Some Tint
  | Unop ((Ast_defs.Uminus | Ast_defs.Uplus), (_, _, Float _)) -> Some Tfloat
  | _ ->
    (* We can't infer the type of everything here. Notably, if you
     * define a const in terms of another const, we need an annotation,
     * since the other const may not have been declared yet.
     *
     * Also note that a number of expressions are considered invalid
     * as constant initializers, even if we can infer their type; see
     * Naming.check_constant_expr. *)
    None

let coalesce_consistent parent current =
  (* If the parent's constructor is consistent via <<__ConsistentConstruct>>, then
   * we want to carry this forward even if the child is final. Example:
   *
   * <<__ConsistentConstruct>>
   * class C {
   *   public static function f(): void {
   *     new static();
   *   }
   * }
   * final class D<reify T> extends C {}
   *
   * Even though D's consistency locally comes from the fact that D was
   * declared as a final class, calling [new static()] via [D::f] will cause
   * a runtime exception because D has reified generics. *)
  match parent with
  | Inconsistent -> current
  | ConsistentConstruct -> parent
  (* This case is unreachable in a correct program, because parent would have to
     be a final class *)
  | FinalClass -> parent

let consistent_construct_kind cls : consistent_kind =
  Shallow_decl_defs.(
    if cls.sc_final then
      FinalClass
    else
      let consistent_attr_present =
        Attributes.mem
          SN.UserAttributes.uaConsistentConstruct
          cls.sc_user_attributes
      in
      if consistent_attr_present then
        ConsistentConstruct
      else
        Inconsistent)
