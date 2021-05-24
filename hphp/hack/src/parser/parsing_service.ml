(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let neutral = (Relative_path.Map.empty, Errors.empty, Relative_path.Set.empty)

(* Parsing a file without failing
 * acc is a file_info
 * errorl is a list of errors
 * error_files is Relative_path.Set.t of files that we failed to parse
 *)
let process_parse_result
    ctx
    ?(ide = false)
    ~quick
    ~trace
    ~start_time
    (acc, errorl, error_files)
    fn
    res
    popt =
  let start_parse_time = start_time in
  let start_process_time = Unix.gettimeofday () in
  let (errorl', { Parser_return.file_mode; comments = _; ast; content }) =
    res
  in
  let ast =
    if
      Relative_path.(is_hhi (Relative_path.prefix fn))
      && ParserOptions.deregister_php_stdlib popt
    then
      Nast.deregister_ignored_attributes ast
    else
      ast
  in
  let bytes = String.length content in
  let content =
    if ide then
      File_provider.Ide content
    else
      File_provider.Disk content
  in
  if Option.is_some file_mode then (
    let (funs, classes, record_defs, typedefs, consts) = Nast.get_defs ast in
    (* If this file was parsed from a tmp directory,
      save it to the main directory instead *)
    let fn =
      match Relative_path.prefix fn with
      | Relative_path.Tmp -> Relative_path.to_root fn
      | _ -> fn
    in
    (* We only have to write to the disk heap on initialization, and only *)
    (* if quick mode is on: otherwise Full Asts means the ParserHeap will *)
    (* never use the DiskHeap, and the Ide services update DiskHeap directly *)
    if quick then File_provider.provide_file_hint fn content;
    let mode =
      if quick then
        Ast_provider.Decl
      else
        Ast_provider.Full
    in
    Ast_provider.provide_ast_hint fn ast mode;
    let comments = None in
    let decls = Decl.nast_to_decls [] ctx ast in
    let hash = Some (Direct_decl_parser.decls_hash decls) in
    let defs =
      {
        FileInfo.hash;
        funs;
        classes;
        record_defs;
        typedefs;
        consts;
        comments;
        file_mode;
      }
    in
    let acc = Relative_path.Map.add acc ~key:fn ~data:defs in
    let errorl = Errors.merge errorl' errorl in
    let error_files =
      if Errors.is_empty errorl' then
        error_files
      else
        Relative_path.Set.add error_files fn
    in
    if trace then
      Hh_logger.log
        "[%dk, %.0f+%.0fms] %s - %s"
        (bytes / 1024)
        ((start_process_time -. start_parse_time) *. 1000.0)
        ((Unix.gettimeofday () -. start_process_time) *. 1000.0)
        (Relative_path.suffix fn)
        (FileInfo.to_string defs);
    (acc, errorl, error_files)
  ) else (
    File_provider.provide_file_hint fn content;
    (* we also now keep in the file_info regular php files
     * as we need at least their names in hack build
     *)
    let acc = Relative_path.Map.add acc ~key:fn ~data:FileInfo.empty_t in
    (acc, errorl, error_files)
  )

let parse ctx ~quick ~show_all_errors ~trace popt acc fn =
  if not @@ FindUtils.path_filter fn then
    acc
  else
    let start_time = Unix.gettimeofday () in
    let res =
      Errors.do_with_context fn Errors.Parsing @@ fun () ->
      Full_fidelity_ast.defensive_from_file ~quick ~show_all_errors popt fn
    in
    process_parse_result ctx ~quick ~start_time ~trace acc fn res popt

(* Merging the results when the operation is done in parallel *)
let merge_parse (acc1, status1, files1) (acc2, status2, files2) =
  ( Relative_path.Map.union acc1 acc2,
    Errors.merge status1 status2,
    Relative_path.Set.union files1 files2 )

let parse_files
    ctx ?(quick = false) ?(show_all_errors = false) ~trace popt acc fnl =
  List.fold_left
    fnl
    ~init:acc
    ~f:(parse ctx ~quick ~show_all_errors ~trace popt)

let parse_parallel
    ctx ?(quick = false) ?(show_all_errors = false) ~trace workers get_next popt
    =
  MultiWorker.call
    workers
    ~job:(parse_files ctx ~quick ~show_all_errors ~trace popt)
    ~neutral
    ~merge:merge_parse
    ~next:get_next

let parse_sequential ctx ~quick ~show_all_errors ~trace fn content acc popt =
  if not @@ FindUtils.path_filter fn then
    acc
  else
    let start_time = Unix.gettimeofday () in
    let res =
      Errors.do_with_context fn Errors.Parsing (fun () ->
          (* DISGUSTING: so far, parser was used only for text files from disk, and
      * those files are (or our reading primitives make them?) terminated with
      * newline. Files that come from memory don't have this guarantee, and it
      * breaks the parser in few places. Appending a sentinel newline doesn't
      * change the parse tree, and is much easier than debugging the parser. *)
          let length = String.length content in
          let content =
            if length > 0 && not (Char.equal content.[length - 1] '\n') then
              content ^ "\n"
            else
              content
          in
          Full_fidelity_ast.defensive_program
            ~quick
            ~show_all_errors
            popt
            fn
            content)
    in
    process_parse_result ctx ~ide:true ~quick ~trace ~start_time acc fn res popt

(*****************************************************************************)
(* Main entry points *)
(*****************************************************************************)
let go
    (ctx : Provider_context.t)
    ?(quick = false)
    ?(show_all_errors = false)
    (workers : MultiWorker.worker list option)
    (files_set : Relative_path.Set.t)
    ~(get_next : Relative_path.t list MultiWorker.Hh_bucket.next)
    (popt : ParserOptions.t)
    ~(trace : bool) :
    FileInfo.t Relative_path.Map.t * Errors.t * Relative_path.Set.t =
  let acc =
    parse_parallel ctx ~quick ~show_all_errors ~trace workers get_next popt
  in
  let (fast, errorl, failed_parsing) =
    Relative_path.Set.fold files_set ~init:acc ~f:(fun fn acc ->
        let content = File_provider.get_ide_contents_unsafe fn in
        parse_sequential ctx ~quick ~show_all_errors ~trace fn content acc popt)
  in
  (fast, errorl, failed_parsing)
