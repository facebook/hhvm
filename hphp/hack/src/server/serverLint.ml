(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open ServerEnv
open Utils

module RP = Relative_path

let output_json oc el =
  let errors_json = List.map el Lint.to_json in
  let res =
    Hh_json.JSON_Object [
        "errors", Hh_json.JSON_Array errors_json;
        "version", Hh_json.JSON_String Build_id.build_id_ohai;
    ] in
  output_string oc (Hh_json.json_to_string res);
  flush stderr

let output_text oc el =
  (* Essentially the same as type error output, except that we only have one
   * message per error, and no additional 'typing reasons' *)
  if el = []
  then output_string oc "No lint errors!\n"
  else begin
    let sl = List.map el Lint.to_string in
    List.iter sl begin fun s ->
      Printf.fprintf oc "%s\n%!" s;
    end
  end;
  flush oc

(* For linting from stdin, we pass the file contents in directly because there's
no other way to get ahold of the contents of stdin from a worker process. But
when linting from disk, we want each individual worker to read the file off disk
by itself, so that we don't need to read all the files at the beginning and hold
them all in memory. *)
type lint_target = { filename: RP.t; contents: string option }

let lint tcopt _acc (files_with_contents : lint_target list) =
  List.fold_left files_with_contents ~f:begin fun acc { filename; contents } ->
    let errs, () =
      Lint.do_ begin fun () ->
        Errors.ignore_ begin fun () ->
          let contents = match contents with
            | Some contents -> contents
            | None -> Sys_utils.cat (Relative_path.to_absolute filename)
          in
          Linting_service.lint tcopt filename contents
        end
      end in
    errs @ acc
  end ~init:[]

let lint_and_filter tcopt code acc fnl =
  let lint_errs = lint tcopt acc fnl in
  List.filter lint_errs (fun err -> Lint.get_code err = code)

let lint_all genv env code =
  let next = compose
    (fun lst -> lst
      |> List.map ~f:(fun fn ->
          { filename = RP.create RP.Root fn; contents = None })
      |> Bucket.of_list)
    (genv.indexer FindUtils.is_php) in
  let errs = MultiWorker.call
    genv.workers
    ~job:(lint_and_filter env.tcopt code)
    ~merge:List.rev_append
    ~neutral:[]
    ~next in
  List.map errs Lint.to_absolute

let create_rp : string -> RP.t = Relative_path.create Relative_path.Root

let prepare_errors_for_output errs = errs
  |> List.sort ~cmp:(fun x y -> Pos.compare (Lint.get_pos x) (Lint.get_pos y))
  |> List.map ~f:Lint.to_absolute

let go genv env fnl =
  let files_with_contents = List.map fnl
    ~f:(fun filename -> { filename = create_rp filename; contents = None })
  in
  let errs =
    if List.length files_with_contents > 10
    then
      MultiWorker.call
        genv.workers
        ~job:(lint env.tcopt)
        ~merge:List.rev_append
        ~neutral:[]
        ~next:(MultiWorker.next genv.workers files_with_contents)
    else
      lint env.tcopt [] files_with_contents in
  prepare_errors_for_output errs

let go_stdin env ~(filename : string) ~(contents : string) =
  let file_with_contents =
    { filename = create_rp filename; contents = Some contents }
  in
  lint env.tcopt [] [file_with_contents]
  |> prepare_errors_for_output
