(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

let check_reifiable env tc attr_pos =
  let check_impl kind ty_opt =
    match ty_opt with
    | Some ty ->
      let emit_err = Errors.reifiable_attr attr_pos kind in
      Typing_reified_check.validator#validate_type
        env
        (fst tc.ttc_name |> Pos_or_decl.unsafe_to_raw_pos)
        ty
        ~reification:Type_validator.Unresolved
        emit_err
    | None -> ()
  in
  check_impl "type" tc.ttc_type;
  check_impl "constraint" tc.ttc_as_constraint;
  check_impl "super_constraint" tc.ttc_super_constraint;
  match tc.ttc_abstract with
  | TCAbstract default_ty -> check_impl "type" default_ty
  | _ -> ()
