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
module SN = Naming_special_names

let unwrap_class_hint = function
  | (_, Happly ((pos, class_name), type_parameters)) ->
    (pos, class_name, type_parameters)
  | (p, Habstr _) ->
    Errors.expected_class ~suffix:" or interface but got a generic" p;
    (Pos.none, "", [])
  | (p, _) ->
    Errors.expected_class ~suffix:" or interface" p;
    (Pos.none, "", [])

let unwrap_class_type ty =
  match deref ty with
  | (r, Tapply (name, tparaml)) -> (r, name, tparaml)
  | (r, Tgeneric _) ->
    let p = Typing_reason.to_pos r in
    (r, (p, ""), [])
  | (r, _) ->
    let p = Typing_reason.to_pos r in
    (r, (p, ""), [])

(* Given sets A and B return a tuple (AnB, A\B), i.e split A into the part
 * that is common with B, and which is unique to A *)
let split_sets defs split_if_in_defs =
  SSet.partition (SSet.mem split_if_in_defs) defs

(* Map split_sets over all sets in FileInfo *)
let split_defs defs split_if_in_defs =
  FileInfo.(
    let (n_funs1, n_funs2) = split_sets defs.n_funs split_if_in_defs.n_funs in
    let (n_classes1, n_classes2) =
      split_sets defs.n_classes split_if_in_defs.n_classes
    in
    let (n_record_defs1, n_record_defs2) =
      split_sets defs.n_record_defs split_if_in_defs.n_record_defs
    in
    let (n_types1, n_types2) =
      split_sets defs.n_types split_if_in_defs.n_types
    in
    let (n_consts1, n_consts2) =
      split_sets defs.n_consts split_if_in_defs.n_consts
    in
    let r1 =
      {
        n_funs = n_funs1;
        n_classes = n_classes1;
        n_record_defs = n_record_defs1;
        n_types = n_types1;
        n_consts = n_consts1;
      }
    in
    let r2 =
      {
        n_funs = n_funs2;
        n_classes = n_classes2;
        n_record_defs = n_record_defs2;
        n_types = n_types2;
        n_consts = n_consts2;
      }
    in
    (r1, r2))

let infer_const expr_ =
  match expr_ with
  | String _ -> Some Tstring
  | True
  | False ->
    Some Tbool
  | Int _ -> Some Tint
  | Float _ -> Some Tfloat
  | Null -> Some Tnull
  | Unop ((Ast_defs.Uminus | Ast_defs.Uplus), (_, Int _)) -> Some Tint
  | Unop ((Ast_defs.Uminus | Ast_defs.Uplus), (_, Float _)) -> Some Tfloat
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
   *   public function f(): void {
   *     new static();
   *   }
   * }
   * final class D<reify T> {}
   *
   * Even though D's consistency locally comes from the final class, calling
   * new static() will cause a runtime exception because D has reified generics. *)
  match parent with
  | Inconsistent -> current
  | ConsistentConstruct -> parent
  (* This case is unreachable, because parent would have to be a final class *)
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
