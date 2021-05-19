(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(* Convert result type to tuple since it's not available to
   Nuclide_rpc_message_printer *)
let tast_holes_result_to_tuple
    TastHolesService.
      {
        pos;
        actual_ty_string;
        expected_ty_string;
        actual_ty_json;
        expected_ty_json;
      } =
  (actual_ty_string, actual_ty_json, expected_ty_string, expected_ty_json, pos)

let print_json result =
  Nuclide_rpc_message_printer.(
    print_json
    @@ tast_holes_response_to_json
    @@ List.map ~f:tast_holes_result_to_tuple result)

let print_string result =
  let print_elem
      TastHolesService.{ pos; actual_ty_string; expected_ty_string; _ } =
    print_endline
    @@ Format.sprintf
         {|%s actual type: %s, expected type: %s|}
         Pos.(string_no_file pos)
         actual_ty_string
         expected_ty_string
  in
  match result with
  | [] -> print_endline "No TAST Holes"
  | _ -> List.iter ~f:print_elem result

let go result output_json =
  if output_json then
    print_json result
  else
    print_string result
