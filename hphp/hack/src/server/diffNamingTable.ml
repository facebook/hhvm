(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type args = {
  base_naming_table: string;
  base_naming_table_sqlite: bool;
  new_naming_table: string;
  new_naming_table_sqlite: bool;
  print_count: bool;
}

let die str =
  prerr_endline str;
  exit 2

let parse_options () =
  let naming_table_paths_ref = ref [] in
  let first_file_sqlite = ref false in
  let second_file_sqlite = ref false in
  let count = ref false in
  let options =
    [
      ( "--f1-sqlite",
        Arg.Set first_file_sqlite,
        "The first file is a sqlite file; Default is a marshaled ocaml blob" );
      ( "--f2-sqlite",
        Arg.Set second_file_sqlite,
        "The second file is a sqlite file; Default is a marshaled ocaml blob" );
      ( "--count",
        Arg.Set count,
        "Print a count of the difference; Default is full text print" );
    ]
  in
  let usage =
    Printf.sprintf
      "Usage: diffNamingTable (%s) first_naming_table second_naming_table"
      Sys.argv.(0)
  in
  let () =
    Arg.parse
      options
      (fun naming_table_path ->
        naming_table_paths_ref := !naming_table_paths_ref @ [naming_table_path])
      usage
  in
  match !naming_table_paths_ref with
  | [path1; path2] ->
    {
      base_naming_table = path1;
      base_naming_table_sqlite = !first_file_sqlite;
      new_naming_table = path2;
      new_naming_table_sqlite = !second_file_sqlite;
      print_count = !count;
    }
  | _ -> die usage

let get_default_provider_context () =
  let () =
    Relative_path.set_path_prefix Relative_path.Root (Path.make_unsafe "root")
  in
  let () =
    Relative_path.set_path_prefix Relative_path.Hhi (Path.make_unsafe "hhi")
  in
  let () = Provider_backend.set_local_memory_backend_with_defaults () in
  let hh_parser_options = GlobalOptions.default in
  let provider_backend = Provider_backend.get () in
  Provider_context.empty_for_tool
    ~popt:hh_parser_options
    ~tcopt:hh_parser_options
    ~backend:provider_backend
    ~deps_mode:Typing_deps_mode.SQLiteMode

let get_naming_table_and_errors provider_context path is_sqlite =
  let (sqlite_path, marshaled_blob_path) =
    if is_sqlite then
      (Some path, "")
    else
      (None, path)
  in
  let hot_decls_path_default =
    {
      State_loader.legacy_hot_decls_path = "";
      State_loader.shallow_hot_decls_path = "";
    }
  in
  let errors_path = path ^ ".err" in
  SaveStateService.load_saved_state
    ~load_decls:false
    ~shallow_decls:false
    ~naming_table_fallback_path:sqlite_path
    ~naming_table_path:marshaled_blob_path
    ~hot_decls_paths:hot_decls_path_default
    ~errors_path
    provider_context

type diff = {
  added_files: Relative_path.t list;
  removed_files: Relative_path.t list;
  changed_files: (Relative_path.t * FileInfo.diff) list;
  removed_errors: Relative_path.t list;
  added_errors: Relative_path.t list;
}

let empty_diff =
  {
    added_files = [];
    removed_files = [];
    changed_files = [];
    removed_errors = [];
    added_errors = [];
  }

let calculate_diff naming_table1 naming_table2 errors1 errors2 =
  let diff =
    Naming_table.fold
      naming_table1
      ~init:empty_diff
      ~f:(fun path fileinfo1 acc ->
        match Naming_table.get_file_info naming_table2 path with
        | None -> { acc with removed_files = path :: acc.removed_files }
        | Some fileinfo2 ->
          begin
            match FileInfo.diff fileinfo1 fileinfo2 with
            | None -> acc
            | Some file_diff ->
              {
                acc with
                changed_files = (path, file_diff) :: acc.changed_files;
              }
          end)
  in
  let diff =
    Naming_table.fold naming_table2 ~init:diff ~f:(fun path _ acc ->
        match Naming_table.get_file_info naming_table1 path with
        | None -> { acc with added_files = path :: acc.added_files }
        | _ -> acc)
  in
  let errors1 =
    List.fold
      ~f:(fun acc (_phase, path_set) -> Relative_path.Set.union path_set acc)
      ~init:Relative_path.Set.empty
      errors1
  in
  let errors2 =
    List.fold
      ~f:(fun acc (_phase, path_set) -> Relative_path.Set.union path_set acc)
      ~init:Relative_path.Set.empty
      errors2
  in
  let removed_errors =
    Relative_path.Set.elements (Relative_path.Set.diff errors1 errors2)
  in
  let added_errors =
    Relative_path.Set.elements (Relative_path.Set.diff errors2 errors1)
  in
  { diff with removed_errors; added_errors }

let file_info_diff_to_string path d =
  let open FileInfo in
  let set_to_string sset = String.concat (SSet.elements sset) ~sep:", " in
  let helper acc (description, s) =
    if SSet.is_empty s then
      acc
    else
      description :: set_to_string s :: acc
  in
  let t =
    List.fold
      ~f:helper
      ~init:[]
      [
        ("Removed consts:", d.removed_consts);
        ("Added consts:", d.added_consts);
        ("Removed types:", d.removed_types);
        ("Added types:", d.added_types);
        ("Removed classes:", d.removed_classes);
        ("Added classes:", d.added_classes);
        ("Removed functions:", d.removed_funs);
        ("Added functions", d.added_funs);
      ]
  in
  String.concat ~sep:"\n" (Relative_path.S.to_string path :: t)

let print_diff args diff =
  let no_difference =
    List.is_empty diff.added_files
    && List.is_empty diff.removed_files
    && List.is_empty diff.changed_files
  in
  if no_difference then
    print_string "The naming tables are identical"
  else if args.print_count then
    let print_category description change =
      if not (List.is_empty change) then
        print_string
          (description ^ " " ^ string_of_int (List.length change) ^ "\n")
    in
    let () = print_category "Added files:" diff.added_files in
    let () = print_category "Removed files:" diff.removed_files in
    let () = print_category "Changed files:" diff.changed_files in
    let () = print_category "Added errors:" diff.added_errors in
    let () = print_category "Removed errors:" diff.removed_errors in
    ()
  else
    let concat_paths paths =
      let paths =
        List.map ~f:(fun path -> Relative_path.S.to_string path) paths
      in
      String.concat paths ~sep:", "
    in
    let print_category description change =
      if not (List.is_empty change) then
        let () = print_string (description ^ "\n") in
        print_string (concat_paths change ^ "\n")
    in
    let () = print_category "Added files:" diff.added_files in
    let () = print_category "Removed files:" diff.removed_files in
    let () =
      if not (List.is_empty diff.changed_files) then
        let () = print_string "Changed files:\n" in
        List.iter diff.changed_files ~f:(fun (path, d) ->
            print_string (file_info_diff_to_string path d))
    in
    let () = print_category "Added errors:" diff.added_errors in
    let () = print_category "Removed errors:" diff.removed_errors in
    ()

let () =
  let args = parse_options () in
  let provider_context = get_default_provider_context () in
  let (naming_table1, errors1) =
    get_naming_table_and_errors
      provider_context
      args.base_naming_table
      args.base_naming_table_sqlite
  in
  let (naming_table2, errors2) =
    get_naming_table_and_errors
      provider_context
      args.new_naming_table
      args.new_naming_table_sqlite
  in
  let diff = calculate_diff naming_table1 naming_table2 errors1 errors2 in
  let () = print_diff args diff in
  ()
