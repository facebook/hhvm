(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module Reason = Typing_reason
module TUtils = Typing_utils
module Cls = Typing_classes_heap

(* Only applied to classes. Checks that all the requirements of the traits
 * and interfaces it uses are satisfied. *)
let check_fulfillment env get_impl (parent_pos, req_ty) =
  match TUtils.try_unwrap_class_type req_ty with
  | None -> ()
  | Some (_r, (_p, req_name), _paraml) ->
    match get_impl req_name with
    | None ->
      let req_pos = Reason.to_pos (fst req_ty) in
      Errors.unsatisfied_req parent_pos req_name req_pos;
      ()
    | Some impl_ty ->
      Typing_ops.sub_type_decl parent_pos Reason.URclass_req env
        impl_ty req_ty

let check_class env tc =
  match (Cls.kind tc) with
  | Ast.Cnormal | Ast.Cabstract ->
    Sequence.iter (Cls.all_ancestor_reqs tc) (check_fulfillment env (Cls.get_ancestor tc))
  | Ast.Ctrait | Ast.Cinterface | Ast.Cenum | Ast.Crecord -> ()
