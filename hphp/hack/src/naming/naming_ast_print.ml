(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let pp_unit fmt () = Format.pp_print_string fmt "()"

let print_nast_internal pp_ex nast =
  let formatter = Format.formatter_of_out_channel Stdlib.stdout in
  Format.pp_set_margin formatter 200;
  Aast.pp_program pp_ex pp_unit formatter nast;
  Format.pp_print_newline formatter ()

let print_nast nast = print_nast_internal pp_unit nast

let print_nast_without_position nast =
  let remove_pos =
    object
      inherit [_] Aast.map

      method! on_pos _ _pos = Pos.none

      method on_'ex _ _pos = ()

      method on_'en _ en = en
    end
  in
  let nast = remove_pos#on_program () nast in
  print_nast_internal pp_unit nast
