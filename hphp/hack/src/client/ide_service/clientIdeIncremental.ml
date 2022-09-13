(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let log s = Hh_logger.log ("[ide-incremental] " ^^ s)

let log_debug s = Hh_logger.debug ("[ide-incremental] " ^^ s)

(** Print file change *)
let log_file_info_change
    ~(old_file_info : FileInfo.t option)
    ~(new_file_info : FileInfo.t option)
    ~(path : Relative_path.t) : unit =
  let verb =
    match (old_file_info, new_file_info) with
    | (Some _, Some _) -> "updated"
    | (Some _, None) -> "deleted"
    | (None, Some _) -> "added"
    | (None, None) ->
      (* May or may not indicate a bug in either the language client or the
         language server.

         - Could happen if the language client sends spurious notifications.
         - Could happen if the editor writes files in a certain way, such as if
           they delete the file before moving a new one into place.
         - Could happen if the language server was not able to read the file,
           despite it existing on disk (e.g. due to permissions). In this case,
           we would fail to generate its [FileInfo.t] and assume that it was
           deleted. This is correct from a certain point of view.
         - Could happen due to a benign race condition where we process
           file-change notifications more slowly than they happen. If a file is
           quickly created, then deleted before we process the create event,
           we'll think it was deleted twice. This is the correct way to handle
           the race condition.
      *)
      "spuriously updated"
  in
  log_debug "File changed: %s %s" (Relative_path.to_absolute path) verb

(** This fetches the new names out of the modified file.
Returns (new_file_info * new_facts) *)
let compute_fileinfo_for_path
    (popt : ParserOptions.t) (contents : string option) (path : Relative_path.t)
    : FileInfo.t option =
  match contents with
  | None -> None
  (* The file couldn't be read from disk. Assume it's been deleted or is
     otherwise inaccessible. Our caller will delete the entries from the
     naming and reverse naming table; there's nothing for us to do here. *)
  | Some contents ->
    Some
      (Direct_decl_parser.parse_and_hash_decls
         (DeclParserOptions.from_parser_options popt)
         path
         contents
      |> Direct_decl_parser.decls_to_fileinfo path)

type changed_file_results = {
  naming_table: Naming_table.t;
  sienv: SearchUtils.si_env;
  old_file_info: FileInfo.t option;
  new_file_info: FileInfo.t option;
}

let update_naming_tables_for_changed_file
    ~(ctx : Provider_context.t)
    ~(naming_table : Naming_table.t)
    ~(sienv : SearchUtils.si_env)
    ~(path : Relative_path.t) : changed_file_results =
  if
    Relative_path.is_root (Relative_path.prefix path)
    && FindUtils.path_filter path
  then begin
    let contents = File_provider.get_contents path in
    let new_file_info =
      compute_fileinfo_for_path (Provider_context.get_popt ctx) contents path
    in
    let old_file_info = Naming_table.get_file_info naming_table path in
    log_file_info_change ~old_file_info ~new_file_info ~path;
    (* update the reverse-naming-table, which is mutable storage owned by backend *)
    Naming_provider.update
      ~backend:(Provider_context.get_backend ctx)
      ~path
      ~old_file_info
      ~new_file_info;
    (* remove old FileInfo from forward-naming-table, then add new FileInfo *)
    let naming_table =
      match old_file_info with
      | None -> naming_table
      | Some _ -> Naming_table.remove naming_table path
    in
    let naming_table =
      match new_file_info with
      | None -> naming_table
      | Some new_file_info ->
        Naming_table.update naming_table path new_file_info
    in
    (* update search index *)
    let sienv =
      match new_file_info with
      | None ->
        SymbolIndexCore.remove_files
          ~sienv
          ~paths:(Relative_path.Set.singleton path)
      | Some info ->
        SymbolIndexCore.update_files
          ~ctx
          ~sienv
          ~paths:[(path, info, SearchUtils.TypeChecker)]
    in
    { naming_table; sienv; old_file_info; new_file_info }
  end else begin
    log "Ignored change to file %s" (Relative_path.to_absolute path);
    { naming_table; sienv; old_file_info = None; new_file_info = None }
  end
