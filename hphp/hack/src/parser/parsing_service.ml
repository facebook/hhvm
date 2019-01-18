(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let neutral = (
  Relative_path.Map.empty, Errors.empty,
  Relative_path.Set.empty
  )

let empty_file_info : FileInfo.t = {
  hash = None;
  file_mode = None;
  FileInfo.funs = [];
  classes = [];
  typedefs = [];
  consts = [];
  comments = Some [];
}

let legacy_php_file_info = ref (fun _fn ->
  empty_file_info
)

(* Parsing a file without failing
 * acc is a file_info
 * errorl is a list of errors
 * error_files is Relative_path.Set.t of files that we failed to parse
 *)
let process_parse_result
  ?(ide = false) ~quick (acc, errorl, error_files) fn res popt =
  let errorl', {Parser_return.file_mode; comments = _; ast; content; is_hh_file = _} = res in
  let ast =
  if (Relative_path.prefix fn = Relative_path.Hhi)
  && ParserOptions.deregister_php_stdlib popt
  then Ast_utils.deregister_ignored_attributes ast
  else ast in
  let content = if ide then File_heap.Ide content else File_heap.Disk content in
  if file_mode <> None then begin
    let funs, classes, typedefs, consts = Ast_utils.get_defs ast in
    (* If this file was parsed from a tmp directory,
      save it to the main directory instead *)
    let fn = if Relative_path.prefix fn =
      Relative_path.Tmp then Relative_path.to_root fn else fn in
    (* We only have to write to the disk heap on initialization, and only *)
    (* if quick mode is on: otherwise Full Asts means the ParserHeap will *)
    (* never use the DiskHeap, and the Ide services update DiskHeap directly *)
    if quick then File_heap.FileHeap.write_around fn content;
    let mode = if quick then Parser_heap.Decl else Parser_heap.Full in
    Parser_heap.ParserHeap.write_around fn (ast, mode);
    let comments = None in
    let hash = Some (Ast_utils.generate_ast_decl_hash ast) in
    let defs =
      {FileInfo.hash; funs; classes; typedefs; consts; comments; file_mode}
    in
    let acc = Relative_path.Map.add acc ~key:fn ~data:defs in
    let errorl = Errors.merge errorl' errorl in
    let error_files =
      if Errors.is_empty errorl'
      then error_files
      else Relative_path.Set.add error_files fn
    in
    acc, errorl, error_files
  end
  else begin
    File_heap.FileHeap.write_around fn content;
    let info = try !legacy_php_file_info fn with _ -> empty_file_info in
    (* we also now keep in the file_info regular php files
     * as we need at least their names in hack build
     *)
    let acc = Relative_path.Map.add acc ~key:fn ~data:info in
    acc, errorl, error_files
  end

let parse ~quick popt acc fn =
  if not @@ FindUtils.path_filter fn then acc else
  let res = Errors.do_with_context fn Errors.Parsing @@
    fun () -> Full_fidelity_ast.defensive_from_file ~quick popt fn
  in
  process_parse_result ~quick acc fn res popt

(* Merging the results when the operation is done in parallel *)
let merge_parse
    (acc1, status1, files1)
    (acc2, status2, files2) =
  Relative_path.Map.union acc1 acc2,
  Errors.merge status1 status2,
  Relative_path.Set.union files1 files2

let parse_files ?(quick = false) popt acc fnl =
  let parse =
    if !Utils.profile
    then (fun acc fn ->
      let t = Unix.gettimeofday () in
      let result = parse ~quick popt acc fn in
      let t' = Unix.gettimeofday () in
      let msg =
        Printf.sprintf "%f %s [parsing]" (t' -. t) (Relative_path.suffix fn) in
      !Utils.log msg;
      result)
    else parse ~quick popt in
  List.fold_left fnl ~init:acc ~f:parse

let parse_parallel ?(quick = false) workers get_next popt =
  MultiWorker.call
      workers
      ~job:(parse_files ~quick popt)
      ~neutral:neutral
      ~merge:merge_parse
      ~next:get_next

let parse_sequential ~quick fn content acc popt =
  if not @@ FindUtils.path_filter fn then acc else
  let res =
    Errors.do_with_context fn Errors.Parsing begin fun () ->
      (* DISGUSTING: so far, parser was used only for text files from disk, and
      * those files are (or our reading primitives make them?) terminated with
      * newline. Files that come from memory don't have this guarantee, and it
      * breaks the parser in few places. Appending a sentinel newline doesn't
      * change the parse tree, and is much easier than debugging the parser. *)
      let length = String.length content in
      let content =
        if length > 0 && content.[length - 1] <> '\n' then
          content ^ "\n"
        else
          content
      in
        Full_fidelity_ast.defensive_program ~quick popt fn content
    end
  in
  process_parse_result ~ide:true ~quick acc fn res popt

let log_parsing_results fast =
  Relative_path.(Map.iter fast ~f:begin fun path info ->
    Hh_logger.log "%s - %s" (suffix path) (FileInfo.to_string info)
  end)

(*****************************************************************************)
(* Main entry points *)
(*****************************************************************************)

let go ?(quick = false) workers files_set ~get_next popt ~trace =
  let acc = parse_parallel ~quick workers get_next popt in
  let fast, errorl, failed_parsing =
    Relative_path.Set.fold files_set ~init:acc ~f:(
      fun fn acc ->
        let content = File_heap.get_ide_contents_unsafe fn in
        parse_sequential ~quick fn content acc popt
      ) in
  if trace then log_parsing_results fast;
  fast, errorl, failed_parsing
