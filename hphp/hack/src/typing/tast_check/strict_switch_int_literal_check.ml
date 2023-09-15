(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Aast

(* Checks that all integer literals are in the same format (hex, bin, oct, dec).
   Relied upon by `Strict_switch_check.ml` because otherwise int literals could
   represent the same value (e.g. "0b10" and "2") but not be caught as redundant. *)
let check_int_literal_format_same
    env (cases : ((Tast.ty, Tast.saved_env) Aast.expr * _) list) =
  let f case acc =
    match case with
    | ((_, pos, Int lit), _) -> (pos, lit) :: acc
    | _ -> acc
  in
  match List.fold_right cases ~init:[] ~f with
  | [] -> ()
  | (expected_pos, c) :: cases ->
    let hex = Str.regexp "0x[0-9a-fA-F_]+" in
    let bin = Str.regexp "0b[01_]+" in
    let oct = Str.regexp "0[0-7_]+" in
    let dec = Str.regexp "0$\\|[1-9][0-9_]*" in
    let matches regex str = Str.string_match regex str 0 in
    let format c =
      if matches dec c then
        (dec, "decimal")
      else if matches hex c then
        (hex, "hexadecimal")
      else if matches bin c then
        (bin, "binary")
      else
        (oct, "octal")
    in
    let (same_format, expected) = format c in
    List.find cases ~f:(fun (_, x) -> not @@ matches same_format x)
    |> Option.iter ~f:(fun (pos, c) ->
           let (_, actual) = format c in
           Typing_error_utils.add_typing_error
             ~env:(Tast_env.tast_env_as_typing_env env)
           @@ Typing_error.(
                enum
                @@ Primary.Enum.Enum_switch_inconsistent_int_literal_format
                     { expected; actual; pos; expected_pos }))

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt env x =
      match snd x with
      | Switch (_, cases, _) -> check_int_literal_format_same env cases
      | _ -> ()
  end
