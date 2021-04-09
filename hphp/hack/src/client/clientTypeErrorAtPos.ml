(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let print_json result =
  Nuclide_rpc_message_printer.(
    print_json
    @@ infer_type_error_response_to_json
    @@ Option.value_map
         result
         ~default:(None, None, None, None)
         ~f:(fun InferErrorAtPosService.
                   {
                     actual_ty_string;
                     expected_ty_string;
                     actual_ty_json;
                     expected_ty_json;
                   }
                 ->
           ( Some actual_ty_string,
             Some actual_ty_json,
             Some expected_ty_string,
             Some expected_ty_json )))

let print_string result =
  print_endline
  @@ Option.value_map
       result
       ~default:"(unknown)"
       ~f:(fun InferErrorAtPosService.
                 { actual_ty_string; expected_ty_string; _ }
               ->
         Format.sprintf
           {|actual: %s, expected: %s|}
           actual_ty_string
           expected_ty_string)

let go result output_json =
  if output_json then
    print_json result
  else
    print_string result
