(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let type_file_with_global_tvenvs
    tcopt fn { FileInfo.funs; classes; record_defs; typedefs; consts; _ } =
  let (errors, (tast, global_tvenvs)) =
    Errors.do_with_context fn Errors.Typing (fun () ->
        let (fs, f_global_tvenvs) =
          List.map funs ~f:snd
          |> List.filter_map ~f:(fun x -> Typing_check_job.type_fun tcopt fn x)
          |> List.unzip
        in
        let (cs, c_global_tvenvs) =
          List.map classes ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_job.type_class tcopt fn x)
          |> List.unzip
        in
        let rs =
          List.map record_defs ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_job.type_record_def tcopt fn x)
        in
        let ts =
          List.map typedefs ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_job.check_typedef tcopt fn x)
        in
        let gcs =
          List.map consts ~f:snd
          |> List.filter_map ~f:(fun x ->
                 Typing_check_job.check_const tcopt fn x)
        in
        ( fs @ cs @ rs @ ts @ gcs,
          lazy (List.concat (f_global_tvenvs :: c_global_tvenvs)) ))
  in
  (tast, global_tvenvs, errors)

let type_file tcopt fn fi =
  let (tast, _, errors) = type_file_with_global_tvenvs tcopt fn fi in
  (tast, errors)

(*****************************************************************************)
(* Usually used when we want to run typing hooks *)
(*****************************************************************************)
let check_defs tcopt fn fi = snd (type_file tcopt fn fi)
