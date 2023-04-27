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
    tcopt
    fn
    {
      FileInfo.funs;
      classes;
      typedefs;
      consts;
      modules;
      hash = _;
      file_mode = _;
      comments = _;
    } =
  let snd (_, x, _) = x in
  let (errors, (tast, global_tvenvs)) =
    Errors.do_with_context fn Errors.Typing (fun () ->
        let check ~f defs =
          List.map defs ~f:snd |> List.filter_map ~f:(fun x -> f tcopt fn x)
        in
        let (fs, f_global_tvenvs) =
          check ~f:Typing_check_job.type_fun funs |> List.unzip
        in
        let fs = List.concat fs in
        let (cs, c_global_tvenvs) =
          check ~f:Typing_check_job.type_class classes |> List.unzip
        in
        let ts = check ~f:Typing_check_job.check_typedef typedefs in
        let gcs = check ~f:Typing_check_job.check_const consts in
        let mds = check ~f:Typing_check_job.check_module modules in
        ( fs @ cs @ ts @ gcs @ mds,
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
