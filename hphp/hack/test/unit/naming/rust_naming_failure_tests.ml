(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

open Hh_prelude

type naming_kind =
  | Type
  | Fun
  | Const
  | Module

external naming_lookup_with_pending_gc :
  naming_kind -> Rust_provider_backend.t -> string -> bool
  = "naming_lookup_with_pending_gc"

let warm_cache backend name = function
  | Type ->
    ignore (Rust_provider_backend.Naming.Types.get_pos backend None name)
  | Fun -> ignore (Rust_provider_backend.Naming.Funs.get_pos backend None name)
  | Const ->
    ignore (Rust_provider_backend.Naming.Consts.get_pos backend None name)
  | Module ->
    ignore (Rust_provider_backend.Naming.Modules.get_pos backend None name)

let check_cached_absence backend =
  let name = "\\Missing_sqlite_gc_test_symbol" in
  List.iter [Type; Fun; Const; Module] ~f:(fun kind ->
      warm_cache backend name kind;
      let young_name = String.init (String.length name) ~f:(String.get name) in
      Asserter.Bool_asserter.assert_equals
        true
        (naming_lookup_with_pending_gc kind backend young_name)
        "A cached absence should return without reusing the moved name")

let make_backend path =
  let backend =
    Hh_server_provider_backend.make
      (Decl_fold_options.from_global_options Global_options.default)
      (Decl_parser_options.from_parser_options Parser_options.default)
  in
  Rust_provider_backend.Naming.set_db_path backend (Naming_sqlite.Db_path path);
  backend

let test_cached_absence () =
  Tempfile.with_real_tempdir (fun dir ->
      Relative_path.set_path_prefix Relative_path.Root dir;
      Relative_path.set_path_prefix Relative_path.Hhi dir;
      Relative_path.set_path_prefix Relative_path.Tmp dir;
      let path = Path.to_string (Path.concat dir "naming.sqlite") in
      ignore
        (Naming_table.save (Naming_table.create Relative_path.Map.empty) path
          : Naming_sqlite.save_result);
      check_cached_absence (make_backend path);
      true)

let () =
  let config =
    Shared_mem.
      {
        default_config with
        shm_use_sharded_hashtbl = true;
        shm_cache_size = 2 * 1024 * 1024 * 1024;
      }
  in
  let (_ : Shared_mem.handle) = Shared_mem.init config ~num_workers:0 in
  Event_logger.init_fake ();
  Unit_test.run_all [("rust cached naming absences", test_cached_absence)]
