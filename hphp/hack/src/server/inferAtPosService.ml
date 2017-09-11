(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_message

(* The first string is the pretty-printed type, the second is the JSON *)
type result = (string * string) option

let infer_result_to_ide_response typename =
  match typename with
  | None ->
    Infer_type_response { type_string = None; type_json = None }
  | Some (str, _json) ->
    Infer_type_response { type_string = Some str; type_json = None }

(* Remember (when we care) the type found at a position *)
let save_infer result_ty target_line target_column ty pos env =
  if Pos.inside pos target_line target_column && !result_ty = None
  then begin
    let ty_string = Typing_print.full_strip_ns env ty in
    let ty_with_constraints_string =
      match Typing_print.constraints_for_type env ty with
      | None -> ty_string
      | Some s -> ty_string ^ "\n" ^ s in
    result_ty := Some (ty_with_constraints_string,
      Hh_json.json_to_string (Typing_print.to_json env ty))
  end

let attach_hooks line column =
  let result_ty = ref None in
  let get_result () = !result_ty  in
  Typing_hooks.attach_infer_ty_hook
    (save_infer result_ty line column);
  get_result

let detach_hooks () =
  Typing_hooks.remove_all_hooks ()
