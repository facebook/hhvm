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
  let check_impl kind ty =
    let emit_err pos ty_info =
      Typing_error_utils.add_typing_error
        Typing_error.(
          primary @@ Primary.Reifiable_attr { pos; ty_info; attr_pos; kind })
    in
    Typing_reified_check.validator#validate_type
      env
      (fst tc.ttc_name |> Pos_or_decl.unsafe_to_raw_pos)
      ty
      ~reification:Type_validator.Unresolved
      emit_err
  in
  match tc.ttc_kind with
  | TCConcrete { tc_type } -> check_impl `ty tc_type
  | TCAbstract { atc_as_constraint; atc_super_constraint; atc_default } ->
    Option.iter ~f:(check_impl `ty) atc_default;
    Option.iter ~f:(check_impl `cnstr) atc_as_constraint;
    Option.iter ~f:(check_impl `cnstr) atc_super_constraint
