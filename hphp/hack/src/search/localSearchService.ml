(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open SearchUtils

(* How many locally changed files are in this env? *)
let count_local_fileinfos ~(sienv : si_env) : int =
  Relative_path.Map.cardinal sienv.lss_fullitems

(* Relative paths sometimes produce doubled-up slashes, as in
 * "/path/to/root//subpath/file".  When we extract the suffix
 * from a relative_path.t, clear out any preceding slash. *)
let strip_first_char char s =
  if String.length s = 0 || s.[0] <> char then
    s
  else
    String.sub s 1 (String.length s - 1)

(* Determine a tombstone for a file path *)
let get_tombstone (path : Relative_path.t) : int64 =
  let rel_path_str = Relative_path.suffix path in
  let fixed_path_str = strip_first_char '/' rel_path_str in
  let path_hash = SharedMem.get_hash fixed_path_str in
  path_hash

(* This function is used if fast-facts-parser fails to scan the file *)
let convert_fileinfo_to_contents
    ~(info : SearchUtils.info) ~(filepath : string) : SearchUtils.si_capture =
  let append_item
      (kind : SearchUtils.si_kind)
      (acc : SearchUtils.si_capture)
      (name : string) =
    let (fixed_kind, is_abstract, is_final) =
      (* FFP produces more detailed information about classes
       * than we can get from FileInfo.t objects.  Since this function is only
       * called when a file has been modified locally, it's safe to call
       * decl_provider - this information has already been cached. *)
      if kind = SI_Class then
        match Decl_provider.get_class name with
        | Some cls ->
          let is_final = Decl_provider.Class.final cls in
          let is_abstract = Decl_provider.Class.abstract cls in
          let real_kind = Decl_provider.Class.kind cls in
          let converted_kind =
            match real_kind with
            | Ast_defs.Cnormal
            | Ast_defs.Cabstract ->
              SI_Class
            | Ast_defs.Cinterface -> SI_Interface
            | Ast_defs.Ctrait -> SI_Trait
            | Ast_defs.Cenum -> SI_Enum
          in
          (converted_kind, is_abstract, is_final)
        | None -> (kind, false, false)
      else
        (kind, false, false)
    in
    let item =
      {
        sif_name = Utils.strip_both_ns name;
        sif_kind = fixed_kind;
        sif_filepath = filepath;
        sif_is_abstract = is_abstract;
        sif_is_final = is_final;
      }
    in
    item :: acc
  in
  let fold_full
      (kind : SearchUtils.si_kind)
      (acc : SearchUtils.si_capture)
      (list : FileInfo.id list) : SearchUtils.si_capture =
    List.fold list ~init:acc ~f:(fun inside_acc (_, name) ->
        append_item kind inside_acc name)
  in
  let fold_fast
      (kind : SearchUtils.si_kind)
      (acc : SearchUtils.si_capture)
      (sset : SSet.t) : SearchUtils.si_capture =
    SSet.fold
      (fun name inside_acc -> append_item kind inside_acc name)
      sset
      acc
  in
  match info with
  | Full f ->
    let acc = fold_full SI_Function [] f.FileInfo.funs in
    let acc = fold_full SI_Class acc f.FileInfo.classes in
    let acc = fold_full SI_Typedef acc f.FileInfo.typedefs in
    let acc = fold_full SI_GlobalConstant acc f.FileInfo.consts in
    acc
  | Fast f ->
    let acc = fold_fast SI_Function [] f.FileInfo.n_funs in
    let acc = fold_fast SI_Class acc f.FileInfo.n_classes in
    let acc = fold_fast SI_Typedef acc f.FileInfo.n_types in
    let acc = fold_fast SI_GlobalConstant acc f.FileInfo.n_consts in
    acc

(* Update files when they were discovered *)
let update_file
    ~(sienv : si_env) ~(path : Relative_path.t) ~(info : SearchUtils.info) :
    si_env =
  let tombstone = get_tombstone path in
  let filepath = Relative_path.suffix path in
  let contents =
    try
      let full_filename = Relative_path.to_absolute path in
      if Sys.file_exists full_filename then
        let contents = IndexBuilder.parse_one_file ~path in
        if List.length contents = 0 then
          convert_fileinfo_to_contents ~info ~filepath
        else
          contents
      else
        convert_fileinfo_to_contents ~info ~filepath
    with _ -> convert_fileinfo_to_contents ~info ~filepath
  in
  {
    sienv with
    lss_fullitems =
      Relative_path.Map.add sienv.lss_fullitems ~key:path ~data:contents;
    lss_tombstones = Tombstone_set.add sienv.lss_tombstones tombstone;
  }

(* Remove files from local when they are deleted *)
let remove_file ~(sienv : si_env) ~(path : Relative_path.t) : si_env =
  let tombstone = get_tombstone path in
  {
    sienv with
    lss_fullitems = Relative_path.Map.remove sienv.lss_fullitems path;
    lss_tombstones = Tombstone_set.add sienv.lss_tombstones tombstone;
  }

(* Exception we use to short-circuit out of a loop if we find enough results.
 * May be worth rewriting this in the future to use `fold_until` rather than
 * exceptions *)
exception BreakOutOfScan of si_results

(* Search local changes for symbols matching this prefix *)
let search_local_symbols
    ~(sienv : si_env)
    ~(query_text : string)
    ~(max_results : int)
    ~(context : autocomplete_type option)
    ~(kind_filter : si_kind option) : si_results =
  (* case insensitive search, must include namespace, escaped for regex *)
  let query_text_regex_case_insensitive =
    Str.regexp_case_fold (Str.quote query_text)
  in
  (* case insensitive search, break out if max results reached *)
  let check_symbol_and_add_to_accumulator_and_break_if_max_reached
      ~(acc : si_results)
      ~(symbol : si_fullitem)
      ~(context : autocomplete_type option)
      ~(kind_filter : si_kind option)
      ~(path : Relative_path.t) : si_results =
    let is_valid_match =
      match (context, kind_filter) with
      | (Some Actype, _) -> SearchUtils.valid_for_actype symbol
      | (Some Acnew, _) -> SearchUtils.valid_for_acnew symbol
      | (Some Acid, _) -> SearchUtils.valid_for_acid symbol
      | (Some Actrait_only, _) -> symbol.sif_kind = SI_Trait
      | (_, Some kind_match) -> symbol.sif_kind = kind_match
      | _ -> true
    in
    if
      is_valid_match
      && Str.string_partial_match
           query_text_regex_case_insensitive
           symbol.sif_name
           0
    then
      let fullname = Utils.strip_both_ns symbol.sif_name in
      let acc_new =
        {
          si_name = fullname;
          si_kind = symbol.sif_kind;
          si_filehash = get_tombstone path;
          si_fullname = fullname;
        }
        :: acc
      in
      if List.length acc_new >= max_results then
        raise (BreakOutOfScan acc_new)
      else
        acc_new
    else
      acc
  in
  try
    let acc =
      Relative_path.Map.fold
        sienv.lss_fullitems
        ~init:[]
        ~f:(fun path fullitems acc ->
          let matches =
            List.fold fullitems ~init:[] ~f:(fun acc symbol ->
                check_symbol_and_add_to_accumulator_and_break_if_max_reached
                  ~acc
                  ~symbol
                  ~context
                  ~kind_filter
                  ~path)
          in
          List.append acc matches)
    in
    acc
  with BreakOutOfScan acc -> acc

(* Filter the results to extract any dead objects *)
let extract_dead_results
    ~(sienv : SearchUtils.si_env) ~(results : SearchUtils.si_results) :
    si_results =
  List.filter results ~f:(fun r ->
      let is_valid_result =
        not (Tombstone_set.mem sienv.lss_tombstones r.si_filehash)
      in
      is_valid_result)
