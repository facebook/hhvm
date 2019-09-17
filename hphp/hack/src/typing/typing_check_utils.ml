(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

let type_file tcopt fn { FileInfo.funs; classes; typedefs; consts; _ } =
  let (errors, tast) =
    Errors.do_with_context fn Errors.Typing (fun () ->
        let (fs, _) =
          List.filter_map funs (fun (_, x) ->
              Typing_check_service.type_fun tcopt fn x)
          |> List.unzip
        in
        let (cs, _) =
          List.filter_map classes (fun (_, x) ->
              Typing_check_service.type_class tcopt fn x)
          |> List.unzip
        in
        let ts =
          List.filter_map typedefs (fun (_, x) ->
              Typing_check_service.check_typedef tcopt fn x)
        in
        let gcs =
          List.filter_map consts (fun (_, x) ->
              Typing_check_service.check_const tcopt fn x)
        in
        fs @ cs @ ts @ gcs)
  in
  (tast, errors)

(*****************************************************************************)
(* Usually used when we want to run typing hooks *)
(*****************************************************************************)
let check_defs tcopt fn fi = snd (type_file tcopt fn fi)
