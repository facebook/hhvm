(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type env = {
  popt: ParserOptions.t;
  tcopt: TypecheckerOptions.t;
  local_memory: Provider_backend.local_memory;
}

let make_empty_ctx { popt; tcopt; local_memory } =
  Provider_context.empty_for_tool
    ~popt
    ~tcopt
    ~backend:(Provider_backend.Local_memory local_memory)
    ~deps_mode:(Typing_deps_mode.InMemoryMode None)

let show_env { local_memory; _ } =
  let open Provider_backend in
  let shallow =
    Shallow_decl_cache.fold
      local_memory.shallow_decl_cache
      ~init:[]
      ~f:(fun element acc ->
        match element with
        | Shallow_decl_cache.Element
            (Shallow_decl_cache_entry.Shallow_class_decl name, _) ->
          name :: acc)
  in
  let folded =
    Folded_class_cache.fold
      local_memory.folded_class_cache
      ~init:[]
      ~f:(fun element acc ->
        match element with
        | Folded_class_cache.Element
            (Folded_class_cache_entry.Folded_class_decl name, _) ->
          name :: acc)
  in
  let decl =
    Decl_cache.fold local_memory.decl_cache ~init:[] ~f:(fun element acc ->
        match element with
        | Decl_cache.Element (Decl_cache_entry.Class_decl name, _) ->
          name :: acc
        | Decl_cache.Element (Decl_cache_entry.Fun_decl name, _) -> name :: acc
        | Decl_cache.Element (Decl_cache_entry.Gconst_decl name, _) ->
          name :: acc
        | Decl_cache.Element (Decl_cache_entry.Module_decl name, _) ->
          name :: acc
        | Decl_cache.Element (Decl_cache_entry.Typedef_decl name, _) ->
          name :: acc)
  in
  let concat names =
    names
    |> List.map ~f:Utils.strip_ns
    |> List.sort ~compare:String.compare
    |> String.concat ~sep:","
  in
  Printf.sprintf
    "[Shallow]%s [Folded]%s [Decl]%s"
    (concat shallow)
    (concat folded)
    (concat decl)

let make_entry_ctx env path contents =
  let ctx = make_empty_ctx env in
  Provider_context.add_or_overwrite_entry_contents ~ctx ~path ~contents

let run_test (repo : (Relative_path.t * string) list) ~(f : env -> unit) : unit
    =
  let tcopt =
    GlobalOptions.set
      ~tco_sticky_quarantine:true
      ~tco_lsp_invalidation:true
      GlobalOptions.default
  in
  Provider_backend.set_local_memory_backend_with_defaults_for_test ();
  let local_memory =
    match Provider_backend.get () with
    | Provider_backend.Local_memory local_memory -> local_memory
    | _ -> failwith "expected local_memory"
  in
  let env = { popt = tcopt; tcopt; local_memory } in
  let ctx = make_empty_ctx env in
  Tempfile.with_real_tempdir (fun path ->
      Relative_path.set_path_prefix Relative_path.Root path;
      (* Lay down disk files and initialize the reverse naming table *)
      List.iter repo ~f:(fun (path, contents) ->
          Disk.write_file ~file:(Relative_path.to_absolute path) ~contents;
          let fileinfo =
            Direct_decl_utils.direct_decl_parse ctx path
            |> Option.value_exn
            |> Direct_decl_utils.decls_to_fileinfo path
          in
          let (_dupes : Relative_path.Set.t) =
            Naming_global.ndecl_file_and_get_conflict_files
              ctx
              path
              fileinfo.FileInfo.ids
          in
          ());
      f env)
