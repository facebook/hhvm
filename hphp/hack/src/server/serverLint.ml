(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open ServerEnv
open Utils
module Hack_bucket = Bucket
open Hh_prelude
module RP = Relative_path

(* For linting from stdin, we pass the file contents in directly because there's
   no other way to get ahold of the contents of stdin from a worker process. But
   when linting from disk, we want each individual worker to read the file off disk
   by itself, so that we don't need to read all the files at the beginning and hold
   them all in memory. *)
type lint_target = {
  filename: RP.t;
  contents: string option;
}

let lint tcopt _acc (files_with_contents : lint_target list) =
  List.fold_left
    files_with_contents
    ~f:
      begin
        fun acc { filename; contents } ->
          let (errs, ()) =
            Lints_core.do_ (fun () ->
                Errors.ignore_ (fun () ->
                    let contents =
                      match contents with
                      | Some contents -> contents
                      | None ->
                        Sys_utils.cat (Relative_path.to_absolute filename)
                    in
                    Linting_main.lint tcopt filename contents))
          in
          errs @ acc
      end
    ~init:[]

let lint_and_filter tcopt code acc fnl =
  let lint_errs = lint tcopt acc fnl in
  List.filter lint_errs ~f:(fun err -> Lints_core.get_code err = code)

let lint_all genv ctx code =
  let tcopt = Provider_context.get_tcopt ctx in
  let sample_rate = TypecheckerOptions.typecheck_sample_rate tcopt in
  let next =
    compose
      (fun lst ->
        lst
        |> List.map ~f:(fun fn ->
               { filename = RP.create RP.Root fn; contents = None })
        |> List.filter ~f:(fun { filename; _ } ->
               FindUtils.sample_filter ~sample_rate filename)
        |> Hack_bucket.of_list)
      (genv.indexer FindUtils.is_hack)
  in
  let errs =
    MultiWorker.call
      genv.workers
      ~job:(lint_and_filter ctx code)
      ~merge:List.rev_append
      ~neutral:[]
      ~next
  in
  List.map errs ~f:Lints_core.to_absolute

let create_rp : string -> RP.t = Relative_path.create Relative_path.Root

let prepare_errors_for_output errs =
  errs
  |> List.sort ~compare:(fun x y ->
         Pos.compare (Lints_core.get_pos x) (Lints_core.get_pos y))
  |> List.map ~f:Lints_core.to_absolute

let go genv ctx fnl =
  let files_with_contents =
    List.map fnl ~f:(fun filename ->
        { filename = create_rp filename; contents = None })
  in
  let errs =
    if List.length files_with_contents > 10 then
      MultiWorker.call
        genv.workers
        ~job:(lint ctx)
        ~merge:List.rev_append
        ~neutral:[]
        ~next:(MultiWorker.next genv.workers files_with_contents)
    else
      lint ctx [] files_with_contents
  in
  prepare_errors_for_output errs

let go_stdin ctx ~(filename : string) ~(contents : string) =
  let file_with_contents =
    { filename = create_rp filename; contents = Some contents }
  in
  lint ctx [] [file_with_contents] |> prepare_errors_for_output
