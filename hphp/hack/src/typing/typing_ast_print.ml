(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let print_tast ctx =
  let dummy_filename = Relative_path.default in
  let env = Typing_env.empty ctx dummy_filename ~droot:None in
  let print_pos_and_ty (pos, ty) =
    Format.asprintf "(%a, %s)" Pos.pp pos (Typing_print.full_strip_ns env ty)
  in
  let pp_fb fmt fb =
    let s =
      match fb with
      | Tast.HasUnsafeBlocks -> "Has unsafe blocks"
      | Tast.NoUnsafeBlocks -> "No unsafe blocks"
    in
    Format.pp_print_string fmt s
  in
  let pp_en fmt _ = Format.pp_print_string fmt "()" in
  let pp_ex fmt ex = Format.pp_print_string fmt (print_pos_and_ty ex) in
  let pp_hi fmt ty =
    Format.asprintf "(%s)" (Typing_print.full_strip_ns env ty)
    |> Format.pp_print_string fmt
  in
  fun tast ->
    Printf.printf "%s\n" (Aast.show_program pp_ex pp_fb pp_en pp_hi tast)

let print_tast_for_rust (tast : Tast.program) : unit =
  (* TODO: do we need real versions of any of those?*)
  let popt = ParserOptions.default in
  let tcopt = TypecheckerOptions.default in
  let ctx = Provider_context.empty_for_test ~popt ~tcopt in
  print_tast ctx tast
