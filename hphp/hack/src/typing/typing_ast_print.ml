(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let print_tast ctx tast =
  let dummy_filename = Relative_path.default in
  let env = Typing_env.empty ctx dummy_filename ~droot:None in
  let print_pos_and_ty (pos, ty) =
    Format.asprintf "(%a, %s)" Pos.pp pos (Typing_print.full_strip_ns env ty)
  in
  let pp_fb fmt _ = Format.pp_print_string fmt "()" in
  let pp_en fmt _ = Format.pp_print_string fmt "()" in
  let pp_ex fmt ex = Format.pp_print_string fmt (print_pos_and_ty ex) in
  let pp_hi fmt ty =
    Format.asprintf "(%s)" (Typing_print.full_strip_ns env ty)
    |> Format.pp_print_string fmt
  in
  let formatter = Format.formatter_of_out_channel Stdlib.stdout in
  Format.pp_set_margin formatter 200;
  Aast.pp_program pp_ex pp_fb pp_en pp_hi formatter tast
