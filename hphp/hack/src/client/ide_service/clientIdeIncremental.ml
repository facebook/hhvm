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
    : FileInfo.t option * Facts.facts option =
  match contents with
  | None -> (None, None)
  (* The file couldn't be read from disk. Assume it's been deleted or is
    otherwise inaccessible. Our caller will delete the entries from the
    naming and reverse naming table; there's nothing for us to do here. *)
  | Some contents ->
    (* We don't want our symbols to be mangled for export.  Mangling would
      * convert :xhp:myclass to __xhp_myclass, which would fail name lookup *)
    Facts_parser.mangle_xhp_mode := false;
    let facts =
      Facts_parser.from_text
        ~php5_compat_mode:false
        ~hhvm_compat_mode:true
        ~disable_nontoplevel_declarations:false
        ~disable_legacy_soft_typehints:false
        ~allow_new_attribute_syntax:false
        ~disable_legacy_attribute_syntax:false
        ~enable_xhp_class_modifier:
          (ParserOptions.enable_xhp_class_modifier popt)
        ~disable_xhp_element_mangling:
          (ParserOptions.disable_xhp_element_mangling popt)
        ~disallow_hash_comments:(ParserOptions.disallow_hash_comments popt)
        ~filename:path
        ~text:contents
    in
    let (funs, classes, record_defs, typedefs, consts) =
      match facts with
      | None ->
        (* File failed to parse or was not a Hack file. *)
        ([], [], [], [], [])
      | Some facts ->
        let to_ids name_type names =
          List.map names ~f:(fun name ->
              let fixed_name = Utils.add_ns name in
              let pos = FileInfo.File (name_type, path) in
              (pos, fixed_name))
        in
        let funs = facts.Facts.functions |> to_ids FileInfo.Fun in
        (* Classes and typedefs are both stored under `types`. There's also a
        `typeAliases` field which only stores typedefs that we could use if we
        wanted, but we write out the pattern-matches here for
        exhaustivity-checking. *)
        let classes =
          facts.Facts.types
          |> Facts.InvSMap.filter (fun _k v ->
                 Facts.(
                   match v.kind with
                   | TKClass
                   | TKInterface
                   | TKEnum
                   | TKTrait
                   | TKUnknown
                   | TKMixed ->
                     true
                   | TKTypeAlias
                   | TKRecord ->
                     false))
          |> Facts.InvSMap.keys
          |> to_ids FileInfo.Class
        in
        let record_defs =
          facts.Facts.types
          |> Facts.InvSMap.filter (fun _k v -> Facts.(is_tk_record v.kind))
          |> Facts.InvSMap.keys
          |> to_ids FileInfo.RecordDef
        in
        let typedefs =
          facts.Facts.types
          |> Facts.InvSMap.filter (fun _k v ->
                 Facts.(
                   match v.kind with
                   | TKTypeAlias -> true
                   | TKClass
                   | TKInterface
                   | TKEnum
                   | TKTrait
                   | TKUnknown
                   | TKMixed
                   | TKRecord ->
                     false))
          |> Facts.InvSMap.keys
          |> to_ids FileInfo.Typedef
        in
        let consts = facts.Facts.constants |> to_ids FileInfo.Const in
        (funs, classes, record_defs, typedefs, consts)
    in
    let fi_mode =
      Full_fidelity_parser.parse_mode
        (Full_fidelity_source_text.make path contents)
      |> Option.value (* TODO: is this a reasonable default? *)
           ~default:FileInfo.Mstrict
    in
    ( Some
        {
          FileInfo.file_mode = Some fi_mode;
          funs;
          classes;
          record_defs;
          typedefs;
          consts;
          hash = None;
          comments = None;
        },
      facts )

type changed_file_results = {
  naming_table: Naming_table.t;
  sienv: SearchUtils.si_env;
  old_file_info: FileInfo.t option;
  new_file_info: FileInfo.t option;
}

let update_naming_tables_for_changed_file
    ~(backend : Provider_backend.t)
    ~(popt : ParserOptions.t)
    ~(naming_table : Naming_table.t)
    ~(sienv : SearchUtils.si_env)
    ~(path : Relative_path.t) : changed_file_results =
  if
    Relative_path.is_root (Relative_path.prefix path)
    && FindUtils.path_filter path
  then begin
    let contents = File_provider.get_contents path in
    let (new_file_info, facts) = compute_fileinfo_for_path popt contents path in
    let old_file_info = Naming_table.get_file_info naming_table path in
    log_file_info_change ~old_file_info ~new_file_info ~path;
    (* update the reverse-naming-table, which is mutable storage owned by backend *)
    Naming_provider.update ~backend ~path ~old_file_info ~new_file_info;
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
      match facts with
      | None ->
        SymbolIndexCore.remove_files
          ~sienv
          ~paths:(Relative_path.Set.singleton path)
      | Some facts -> SymbolIndexCore.update_from_facts ~sienv ~path ~facts
    in
    { naming_table; sienv; old_file_info; new_file_info }
  end else begin
    log "Ignored change to file %s" (Relative_path.to_absolute path);
    { naming_table; sienv; old_file_info = None; new_file_info = None }
  end
