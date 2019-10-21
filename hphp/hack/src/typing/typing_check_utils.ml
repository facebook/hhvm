(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

let type_file
    tcopt fn { FileInfo.funs; classes; record_defs; typedefs; consts; _ } =
  let (errors, tast) =
    Errors.do_with_context fn Errors.Typing (fun () ->
        let (fs, _) =
          List.map funs ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_service.type_fun tcopt fn x)
          |> List.unzip
        in
        let (cs, _) =
          List.map classes ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_service.type_class tcopt fn x)
          |> List.unzip
        in
        let rs =
          List.map record_defs ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_service.type_record_def tcopt fn x)
        in
        let ts =
          List.map typedefs ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_service.check_typedef tcopt fn x)
        in
        let gcs =
          List.map consts ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_service.check_const tcopt fn x)
        in
        fs @ cs @ rs @ ts @ gcs)
  in
  (tast, errors)

(*****************************************************************************)
(* Usually used when we want to run typing hooks *)
(*****************************************************************************)
let check_defs tcopt fn fi = snd (type_file tcopt fn fi)
