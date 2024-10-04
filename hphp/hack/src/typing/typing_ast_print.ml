(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let print_tast_internal apply_to_ex_ty print_ex ctx tast =
  let dummy_filename = Relative_path.default in
  let env = Typing_env_types.empty ctx dummy_filename ~droot:None in
  let print_ex ex =
    apply_to_ex_ty (Typing_print.full_strip_ns ~hide_internals:false env) ex
    |> print_ex
  in
  let pp_en fmt _ = Format.pp_print_string fmt "()" in
  let pp_ex fmt ex = Format.pp_print_string fmt (print_ex ex) in
  let formatter = Format.formatter_of_out_channel Stdlib.stdout in
  Format.pp_set_margin formatter 200;
  Aast.pp_program pp_ex pp_en formatter tast

let print_tast ctx tast =
  let apply_to_ex_ty f ty = f ty in
  let print_pos_and_ty ty = Format.asprintf "(%s)" ty in
  print_tast_internal apply_to_ex_ty print_pos_and_ty ctx tast

let print_tast_without_position ctx tast =
  let remove_pos =
    object
      inherit [_] Aast.map

      method! on_pos _ _pos = Pos.none

      method on_'ex _ ty = ty

      method on_'en _ en = en
    end
  in
  let tast = remove_pos#on_program () tast in
  let apply_to_ex_ty f ty = f ty in
  let print_pos_and_ty ty = Format.asprintf "%s" ty in
  print_tast_internal apply_to_ex_ty print_pos_and_ty ctx tast
