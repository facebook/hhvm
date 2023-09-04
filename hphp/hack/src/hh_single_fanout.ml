(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type options = { debug: bool }

let deps_mode = Typing_deps_mode.InMemoryMode None

let tcopt =
  {
    GlobalOptions.default with
    GlobalOptions.tco_enable_modules = true;
    tco_allow_all_files_for_module_declarations = true;
    tco_saved_state =
      {
        GlobalOptions.default_saved_state with
        GlobalOptions.rollouts =
          {
            Saved_state_rollouts.default with
            Saved_state_rollouts.optimized_member_fanout = true;
          };
      };
  }

let popt =
  {
    GlobalOptions.default with
    GlobalOptions.po_disable_xhp_element_mangling = false;
  }

let parse_defs (ctx : Provider_context.t) (files : Relative_path.t list) :
    FileInfo.t Relative_path.Map.t =
  let workers = None in
  let done_ = ref false in
  Direct_decl_service.go
    ctx
    ~trace:false
    ~cache_decls:
      (* Not caching here, otherwise oldification done in redo_type_decl will
         * oldify the new version (and override the real old versions*)
      false
    workers
    ~ide_files:Relative_path.Set.empty
    ~get_next:(fun () ->
      if !done_ then
        Bucket.Done
      else (
        done_ := true;
        Bucket.Job files
      ))

let redecl_make_new_naming_table
    (ctx : Provider_context.t)
    options
    (old_naming_table : Naming_table.t)
    (files_with_changes : Relative_path.t list) : Naming_table.t =
  let defs_per_file_parsed = parse_defs ctx files_with_changes in
  let new_naming_table =
    Naming_table.update_many old_naming_table defs_per_file_parsed
  in
  if options.debug then
    Printf.printf "%s\n" (Naming_table.show new_naming_table);
  new_naming_table

let get_old_and_new_defs
    options
    (files_with_changes : Relative_path.t list)
    (old_naming_table : Naming_table.t)
    (new_naming_table : Naming_table.t) : Naming_table.defs_per_file =
  let add_file_names naming_table filenames acc =
    List.fold filenames ~init:acc ~f:(fun acc file ->
        match Naming_table.get_file_info naming_table file with
        | None -> acc
        | Some file_info ->
          let current_file_names =
            match Relative_path.Map.find_opt acc file with
            | None -> FileInfo.empty_names
            | Some current_file_names -> current_file_names
          in
          let file_names = FileInfo.simplify file_info in
          let new_file_names =
            FileInfo.merge_names current_file_names file_names
          in
          Relative_path.Map.add acc ~key:file ~data:new_file_names)
  in
  let defs =
    Relative_path.Map.empty
    |> add_file_names old_naming_table files_with_changes
    |> add_file_names new_naming_table files_with_changes
  in
  if options.debug then
    Printf.printf
      "The following defs will be diffed for each file:\n%s\n"
      (Naming_table.show_defs_per_file defs);
  defs

let get_symbols_for_deps
    (deps : Typing_deps.DepSet.t)
    (dep_to_symbol_map : _ Typing_deps.Dep.variant Typing_deps.DepMap.t) :
    SSet.t =
  Typing_deps.DepSet.fold deps ~init:SSet.empty ~f:(fun dep acc ->
      let symbol =
        match Typing_deps.DepMap.find_opt dep dep_to_symbol_map with
        | None -> Typing_deps.Dep.to_hex_string dep
        | Some variant -> Typing_deps.Dep.variant_to_string variant
      in
      SSet.add symbol acc)

let compute_fanout ctx options (old_and_new_defs : Naming_table.defs_per_file) :
    Typing_deps.DepSet.t =
  let { Decl_redecl_service.fanout = { Fanout.to_recheck; changed }; _ } =
    Decl_redecl_service.redo_type_decl
      ctx
      ~during_init:false
      None
      ~bucket_size:500
      (fun _ -> SSet.empty)
      ~previously_oldified_defs:FileInfo.empty_names
      ~defs:old_and_new_defs
  in
  if options.debug then (
    Printf.printf "Hashes of changed:%s\n" (Typing_deps.DepSet.show changed);
    Printf.printf "Hashes to recheck:%s\n" (Typing_deps.DepSet.show to_recheck)
  );
  to_recheck

let update_naming_table_and_compute_fanout
    (ctx : Provider_context.t)
    (options : options)
    files_with_changes
    old_naming_table
    (dep_to_symbol_map : _ Typing_deps.Dep.variant Typing_deps.DepMap.t) :
    SSet.t =
  let new_naming_table =
    redecl_make_new_naming_table ctx options old_naming_table files_with_changes
  in
  let old_and_new_defs =
    get_old_and_new_defs
      options
      files_with_changes
      old_naming_table
      new_naming_table
  in
  let to_recheck = compute_fanout ctx options old_and_new_defs in
  let to_recheck = get_symbols_for_deps to_recheck dep_to_symbol_map in
  to_recheck

module FileSystem = struct
  let read_test_file_and_provide_before (file_path : string) :
      Relative_path.t list =
    File_provider.local_changes_push_sharedmem_stack ();
    let files = Multifile.States.base_files file_path in
    Relative_path.Map.iter files ~f:File_provider.provide_file_for_tests;
    Relative_path.Map.keys files

  let read_test_file_and_provide_after (file_path : string) :
      Relative_path.t list =
    let files = Multifile.States.changed_files file_path in
    Relative_path.Map.fold
      files
      ~init:[]
      ~f:(fun file_path content changed_files ->
        let old_content = File_provider.get_contents file_path in
        let changed =
          match old_content with
          | None -> true
          | Some old_content -> not (String.equal old_content content)
        in
        if changed then (
          File_provider.provide_file_for_tests file_path content;
          file_path :: changed_files
        ) else
          changed_files)
end

let print_fanout fanout = SSet.iter (Printf.printf "%s\n") fanout

let init_paths (hhi_root : Path.t) : unit =
  (* dummy path, not actually used *)
  Relative_path.set_path_prefix Relative_path.Root (Path.make "/");
  Hhi.set_hhi_root_for_unit_test hhi_root;
  Relative_path.set_path_prefix Relative_path.Hhi hhi_root;
  (* dummy path, not actually used *)
  Relative_path.set_path_prefix Relative_path.Tmp (Path.make "tmp");
  ()

module DepToSymbolsMap : sig
  val get :
    unit ->
    Typing_deps.Dep.dependency Typing_deps.Dep.variant Typing_deps.DepMap.t

  val callback :
    Typing_deps.Dep.dependent Typing_deps.Dep.variant ->
    Typing_deps.Dep.dependency Typing_deps.Dep.variant ->
    unit
end = struct
  let map :
      Typing_deps.Dep.dependency Typing_deps.Dep.variant Typing_deps.DepMap.t
      ref =
    ref Typing_deps.DepMap.empty

  let add (dep : _ Typing_deps.Dep.variant) : unit =
    let hash = Typing_deps.Dep.make dep in
    map :=
      Typing_deps.DepMap.add
        hash
        (Typing_deps.Dep.dependency_of_variant dep)
        !map

  let callback
      (dependent : Typing_deps.Dep.dependent Typing_deps.Dep.variant)
      (dependency : Typing_deps.Dep.dependency Typing_deps.Dep.variant) : unit =
    add dependent;
    add dependency

  let get () = !map
end

let make_nast ctx (filename : Relative_path.t) : Nast.program =
  Ast_provider.get_ast ~full:true ctx filename |> Naming.program ctx

let make_dep_to_symbol_map ctx options (files : Relative_path.t list) :
    _ Typing_deps.Dep.variant Typing_deps.DepMap.t =
  let dep_to_symbol_map =
    Typing_deps.DepMap.union
      (DepToSymbolsMap.get ())
      (Dep_hash_to_symbol.from_nasts (List.map files ~f:(make_nast ctx)))
  in
  if options.debug then
    Printf.printf
      "%s\n"
      (Typing_deps.DepMap.show Typing_deps.Dep.pp_variant dep_to_symbol_map);
  dep_to_symbol_map

(** Initialize a number of backend structures and global states necessary
  for the typechecker to function. *)
let init (hhi_root : Path.t) : Provider_context.t =
  EventLogger.init_fake ();
  let (_ : SharedMem.handle) =
    SharedMem.init ~num_workers:0 SharedMem.default_config
  in
  init_paths hhi_root;
  let ctx = Provider_context.empty_for_test ~popt ~tcopt ~deps_mode in
  Typing_deps.add_dependency_callback
    ~name:"dep_to_symbol"
    DepToSymbolsMap.callback;
  ctx

let commit_dep_edges () : unit =
  Typing_deps.flush_ideps_batch deps_mode
  |> Typing_deps.register_discovered_dep_edges

let make_reverse_naming_table
    ctx (defs_per_file : FileInfo.t Relative_path.Map.t) : unit =
  Relative_path.Map.iter defs_per_file ~f:(fun file file_info ->
      Naming_global.ndecl_file_skip_if_already_bound ctx file file_info)

(** Build and return the naming table and build the reverse naming table as a side-effect. *)
let make_naming_table
    ctx options (defs_per_file : FileInfo.t Relative_path.Map.t) :
    Naming_table.t =
  let naming_table = Naming_table.create defs_per_file in
  if options.debug then Printf.printf "%s\n" @@ Naming_table.show naming_table;
  make_reverse_naming_table ctx defs_per_file;
  naming_table

(** Type check given files. Create the dependency graph as a side effect. *)
let type_check_make_depgraph ctx options (files : Relative_path.t list) : unit =
  List.iter files ~f:(fun file ->
      let full_ast = Ast_provider.get_ast ctx file ~full:true in
      let (_ : Errors.t * Tast.by_names) =
        Typing_check_job.calc_errors_and_tast ctx file ~full_ast
      in
      ());
  if options.debug then Typing_deps.dump_current_edge_buffer_in_memory_mode ();
  commit_dep_edges ();
  ()

(** Build the naming table and typecheck the base file to create the depgraph.
  Only the naming table is returned. The reverse naming table and depgraph are
  produced as side effects. *)
let process_pre_changes ctx options (files : Relative_path.t list) :
    Naming_table.t =
  (* TODO builtins and hhi stuff *)
  let defs_per_file = parse_defs ctx files in
  let naming_table = make_naming_table ctx options defs_per_file in
  type_check_make_depgraph ctx options files;
  naming_table

(** Process changed files by making those files available in the typechecker as a side effect
  and returning the list of changed files. *)
let process_changed_files options (test_file : string) : Relative_path.t list =
  let files_with_changes =
    FileSystem.read_test_file_and_provide_after test_file
  in
  Ast_provider.clear_local_cache ();
  Ast_provider.clear_parser_cache ();
  if options.debug then
    Printf.printf
      "Changed files: %s\n"
      (String.concat
         ~sep:", "
         (List.map files_with_changes ~f:Relative_path.show));
  files_with_changes

(** This takes the path of a file specifying the base and changed versions of
  files as a multifile, and prints the computed fanout for that change to standard output.
  To do so, it typechecks the base version to create a depgraph and naming table,
  then uses those to compute a fanout. *)
let go (test_file : string) options =
  Tempfile.with_tempdir @@ fun hhi_root ->
  let ctx = init hhi_root in
  let files = FileSystem.read_test_file_and_provide_before test_file in
  let naming_table = process_pre_changes ctx options files in
  let files_with_changes = process_changed_files options test_file in
  let dep_to_symbol_map =
    make_dep_to_symbol_map ctx options files_with_changes
  in
  let fanout =
    update_naming_table_and_compute_fanout
      ctx
      options
      files_with_changes
      naming_table
      dep_to_symbol_map
  in
  print_fanout fanout

let die str =
  let oc = stderr in
  Out_channel.output_string oc str;
  Out_channel.close oc;
  exit 2

let parse_args () : string * options =
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let fn_ref = ref [] in
  let debug = ref false in
  let options = [("--debug", Arg.Set debug, "print debug information")] in
  Arg.parse options (fun fn -> fn_ref := fn :: !fn_ref) usage;
  let options = { debug = !debug } in
  let files = !fn_ref in
  match files with
  | [file] -> (file, options)
  | _ -> die usage

let () =
  let (test_file, options) = parse_args () in
  go test_file options
