(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let get_default_provider_context () =
  let () =
    Relative_path.set_path_prefix Relative_path.Root (Path.make_unsafe "root")
  in
  let () =
    Relative_path.set_path_prefix Relative_path.Hhi (Path.make_unsafe "hhi")
  in
  Provider_backend.set_local_memory_backend_with_defaults_for_test ();
  let hh_parser_options = GlobalOptions.default in
  let provider_backend = Provider_backend.get () in
  Provider_context.empty_for_tool
    ~popt:hh_parser_options
    ~tcopt:hh_parser_options
    ~backend:provider_backend
    ~deps_mode:(Typing_deps_mode.InMemoryMode None)
    ~package_info:Package.Info.empty

let get_naming_table_and_errors provider_context path =
  let sqlite_path = Some path in
  let errors_path = Str.replace_first (Str.regexp "_naming.sql") ".err" path in
  SaveStateService.load_saved_state_exn
    ~naming_table_fallback_path:sqlite_path
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

let is_nonempty
    ({ added_files; removed_files; changed_files; removed_errors; added_errors } :
      diff) =
  match
    (added_files, removed_files, changed_files, removed_errors, added_errors)
  with
  | ([], [], [], [], []) -> false
  | _ -> true

let calculate_diff naming_table1 naming_table2 errors1 errors2 =
  let diff =
    Naming_table.fold
      naming_table1
      ~init:empty_diff
      ~f:(fun path fileinfo1 acc ->
        match Naming_table.get_file_info naming_table2 path with
        | None -> { acc with removed_files = path :: acc.removed_files }
        | Some fileinfo2 -> begin
          match FileInfo.diff fileinfo1 fileinfo2 with
          | None -> acc
          | Some file_diff ->
            { acc with changed_files = (path, file_diff) :: acc.changed_files }
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

let print_diff print_count diff =
  Hh_logger.log "Diffing the naming tables...";
  let no_difference =
    List.is_empty diff.added_files
    && List.is_empty diff.removed_files
    && List.is_empty diff.changed_files
  in
  if no_difference then
    Hh_logger.log "The naming tables are identical!"
  else if print_count then (
    let print_category description change =
      if not (List.is_empty change) then
        Hh_logger.log "%s: %s" description (string_of_int @@ List.length change)
    in
    print_category "Added files" diff.added_files;
    print_category "Removed files" diff.removed_files;
    print_category "Changed files" diff.changed_files;
    print_category "Added errors" diff.added_errors;
    print_category "Removed errors" diff.removed_errors
  ) else
    let concat_paths paths =
      let paths =
        List.map ~f:(fun path -> Relative_path.S.to_string path) paths
      in
      String.concat paths ~sep:"\n"
    in
    let print_category description change =
      if not (List.is_empty change) then
        Hh_logger.log "%s:\n%s" description (concat_paths change)
    in
    print_category "Added files" diff.added_files;
    print_category "Removed files" diff.removed_files;
    if not (List.is_empty diff.changed_files) then (
      Hh_logger.log "Changed files";
      List.iter diff.changed_files ~f:(fun (path, d) ->
          Hh_logger.log "%s" (file_info_diff_to_string path d))
    );
    print_category "Added errors" diff.added_errors;
    print_category "Removed errors" diff.removed_errors

(*
  Prints the diff of test and control (naming table, error) pairs, and
  returns whether test and control differ.
*)
let diff control_path test_path =
  let provider_context = get_default_provider_context () in
  let (control_naming_table, control_errors) =
    get_naming_table_and_errors provider_context control_path
  in
  let (test_naming_table, test_errors) =
    get_naming_table_and_errors provider_context test_path
  in
  let diff =
    calculate_diff
      control_naming_table
      test_naming_table
      control_errors
      test_errors
  in
  print_diff false diff;
  is_nonempty diff
