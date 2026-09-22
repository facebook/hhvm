(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let go results output_json =
  if output_json then
    let results =
      List.map results ~f:Autocomplete_service.autocomplete_result_to_json
    in
    print_endline (Yojson.Safe.to_string (`List results))
  else
    List.iter results ~f:(fun res ->
        let name = res.Autocomplete_types.res_label in
        let ty = res.Autocomplete_types.res_detail in
        print_endline (name ^ " " ^ ty))
