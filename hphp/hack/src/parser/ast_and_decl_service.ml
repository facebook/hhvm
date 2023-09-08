(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let process_parse_result
    ctx
    ~quick
    ~trace
    ~cache_decls
    ~start_time
    (acc, errorl, error_files)
    fn
    (errorl', (ast, decls))
    popt =
  let start_parse_time = start_time in
  let start_process_time = Unix.gettimeofday () in
  let { Parser_return.file_mode; comments = _; ast; content } = ast in
  (* This surely is a bug?? We're now degistering php_stdlib from the AST, but
     the decls we got (and which we're now about to cache) have not been deregistered.
     The two will be inconsistent! *)
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
  let content = File_provider.Disk content in
  if Option.is_some file_mode then (
    (* If this file was parsed from a tmp directory,
       save it to the main directory instead *)
    let fn =
      match Relative_path.prefix fn with
      | Relative_path.Tmp -> Relative_path.to_root fn
      | _ -> fn
    in
    if quick then File_provider.provide_file_hint fn content;
    let mode =
      if quick then
        Ast_provider.Decl
      else
        Ast_provider.Full
    in
    Ast_provider.provide_ast_hint fn ast mode;
    if cache_decls then
      Direct_decl_utils.cache_decls ctx fn decls.Direct_decl_utils.pfh_decls;
    let defs = Direct_decl_parser.decls_to_fileinfo fn decls in
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

let parse ctx ~quick ~show_all_errors ~trace ~cache_decls popt acc fn =
  if not @@ FindUtils.path_filter fn then
    acc
  else
    let start_time = Unix.gettimeofday () in
    let res =
      Errors.do_with_context fn @@ fun () ->
      Full_fidelity_ast.ast_and_decls_from_file ~quick ~show_all_errors popt fn
    in
    process_parse_result
      ctx
      ~quick
      ~start_time
      ~trace
      ~cache_decls
      acc
      fn
      res
      popt

let merge_parse (acc1, status1, files1) (acc2, status2, files2) =
  ( Relative_path.Map.union acc1 acc2,
    Errors.merge status1 status2,
    Relative_path.Set.union files1 files2 )

let go
    (ctx : Provider_context.t)
    ?(quick = false)
    ?(show_all_errors = false)
    (workers : MultiWorker.worker list option)
    ~(get_next : Relative_path.t list MultiWorker.Hh_bucket.next)
    (popt : ParserOptions.t)
    ~(trace : bool)
    ~(cache_decls : bool)
    ~(worker_call : MultiWorker.call_wrapper) :
    FileInfo.t Relative_path.Map.t * Errors.t * Relative_path.Set.t =
  worker_call.MultiWorker.f
    workers
    ~job:(fun init ->
      List.fold_left
        ~init
        ~f:(parse ctx ~quick ~show_all_errors ~trace ~cache_decls popt))
    ~neutral:(Relative_path.Map.empty, Errors.empty, Relative_path.Set.empty)
    ~merge:merge_parse
    ~next:get_next
