(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type parsed_file_with_hashes = Direct_decl_parser.parsed_file_with_hashes = {
  pfh_mode: FileInfo.mode option;
  pfh_hash: FileInfo.pfh_hash;
  pfh_decls: (string * Shallow_decl_defs.decl * Int64.t) list;
}

(* If any decls in the list have the same name, retain only the first
   declaration of each symbol in the sequence. *)
(* NB: Must be manually kept in sync with hackrs functions
   [shallow_decl_provider::LazyShallowDeclProvider::dedup_and_add_decls] and
   [hackrs_provider_backend::HhServerProviderBackend::dedup_and_add_decls]. *)
let dedup_decls decls =
  let open Shallow_decl_defs in
  let seen_types = String.Table.create () in
  let seen_funs = String.Table.create () in
  let seen_consts = String.Table.create () in
  let seen_modules = String.Table.create () in
  Sequence.filter decls ~f:(fun decl ->
      match decl with
      | (name, Class _)
      | (name, Typedef _) ->
        if String.Table.mem seen_types name then
          false
        else
          let () = String.Table.add_exn seen_types ~key:name ~data:() in
          true
      | (name, Fun _) ->
        if String.Table.mem seen_funs name then
          false
        else
          let () = String.Table.add_exn seen_funs ~key:name ~data:() in
          true
      | (name, Const _) ->
        if String.Table.mem seen_consts name then
          false
        else
          let () = String.Table.add_exn seen_consts ~key:name ~data:() in
          true
      | (name, Module _) ->
        if String.Table.mem seen_modules name then
          false
        else
          let () = String.Table.add_exn seen_modules ~key:name ~data:() in
          true)

(* If a symbol was also declared in another file, and that file was determined
   to be the winner in the naming table, remove its decl from the list.

   Do not remove decls if there is no entry for them in the naming table. This
   ensures that this path can populate the decl heap during full inits. This may
   result in the decl heap containing an incorrect decl in the event of a naming
   conflict, which might be confusing. But the user will be obligated to fix the
   naming conflict, and this behavior is limited to full inits, so maybe we can
   live with it. *)
(* NB: Must be manually kept in sync with hackrs functions
   [shallow_decl_provider::LazyShallowDeclProvider::remove_naming_conflict_losers] and
   [hackrs_provider_backend::HhServerProviderBackend::remove_naming_conflict_losers]. *)
let remove_naming_conflict_losers ctx file decls =
  let open Shallow_decl_defs in
  Sequence.filter decls ~f:(fun decl ->
      match decl with
      | (name, Class _)
      | (name, Typedef _) ->
        (match Naming_provider.get_type_path ctx name with
        | Some nfile -> Relative_path.equal nfile file
        | None -> true)
      | (name, Fun _) ->
        (match Naming_provider.get_fun_path ctx name with
        | Some nfile -> Relative_path.equal nfile file
        | None -> true)
      | (name, Const _) ->
        (match Naming_provider.get_const_path ctx name with
        | Some nfile -> Relative_path.equal nfile file
        | None -> true)
      | (name, Module _) ->
        (match Naming_provider.get_module_path ctx name with
        | Some nfile -> Relative_path.equal nfile file
        | None -> true))

let cache_decls ctx file decls =
  let open Shallow_decl_defs in
  let open Typing_defs in
  let decls =
    decls
    |> List.rev_map (* direct decl parser produces reverse of syntactic order *)
         ~f:(fun (name, decl, _hash) -> (name, decl))
    |> Sequence.of_list
    |> dedup_decls
    |> remove_naming_conflict_losers ctx file
    |> Sequence.to_list
  in
  match Provider_context.get_backend ctx with
  | Provider_backend.Pessimised_shared_memory _ ->
    (* We must never perform caching here. Otherwise, we may overwrite earlier
       pessimisation results with unpessimised types *)
    failwith "invalid"
  | Provider_backend.Analysis
  | Provider_backend.Shared_memory ->
    List.iter decls ~f:(function
        | (name, Class decl) -> Shallow_classes_heap.Classes.add name decl
        | (name, Fun decl) -> Decl_store.((get ()).add_fun name decl)
        | (name, Typedef decl) -> Decl_store.((get ()).add_typedef name decl)
        | (name, Const decl) -> Decl_store.((get ()).add_gconst name decl)
        | (name, Module decl) -> Decl_store.((get ()).add_module name decl))
  | Provider_backend.Rust_provider_backend backend ->
    Rust_provider_backend.Decl.add_shallow_decls backend decls
  | Provider_backend.(Local_memory { decl_cache; shallow_decl_cache; _ }) ->
    List.iter decls ~f:(function
        | (name, Class decl) ->
          let (_ : shallow_class option) =
            Provider_backend.Shallow_decl_cache.find_or_add
              shallow_decl_cache
              ~key:
                (Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl
                   name)
              ~default:(fun () -> Some decl)
          in
          ()
        | (name, Fun decl) ->
          let (_ : fun_elt option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Fun_decl name)
              ~default:(fun () -> Some decl)
          in
          ()
        | (name, Typedef decl) ->
          let (_ : typedef_type option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Typedef_decl name)
              ~default:(fun () -> Some decl)
          in
          ()
        | (name, Const decl) ->
          let (_ : const_decl option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Gconst_decl name)
              ~default:(fun () -> Some decl)
          in
          ()
        | (name, Module decl) ->
          let (_ : module_decl option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Module_decl name)
              ~default:(fun () -> Some decl)
          in
          ())

let get_file_contents ~ignore_file_content_caches ctx filename =
  let from_entries =
    if ignore_file_content_caches then
      None
    else
      Naming_provider.get_entry_contents ctx filename
  in
  match from_entries with
  | Some _ as contents_opt -> contents_opt
  | None ->
    File_provider.get_contents
      ~force_read_disk:ignore_file_content_caches
      filename

let direct_decl_parse ?(ignore_file_content_caches = false) ctx file =
  Counters.count Counters.Category.Get_decl @@ fun () ->
  match get_file_contents ~ignore_file_content_caches ctx file with
  | None -> None
  | Some contents ->
    let popt = Provider_context.get_popt ctx in
    let opts = DeclParserOptions.from_parser_options popt in
    let deregister_php_stdlib_if_hhi =
      ParserOptions.deregister_php_stdlib popt
    in
    let parsed_file =
      Direct_decl_parser.parse_and_hash_decls
        opts
        deregister_php_stdlib_if_hhi
        file
        contents
    in
    Some parsed_file

let direct_decl_parse_and_cache ctx file =
  match Provider_context.get_backend ctx with
  | Provider_backend.Rust_provider_backend backend ->
    Counters.count Counters.Category.Get_decl @@ fun () ->
    get_file_contents ~ignore_file_content_caches:false ctx file
    |> Option.map ~f:(fun contents ->
           Rust_provider_backend.Decl.direct_decl_parse_and_cache
             backend
             file
             contents)
  | _ ->
    let result = direct_decl_parse ctx file in
    (match result with
    | Some parsed_file -> cache_decls ctx file parsed_file.pfh_decls
    | None -> ());
    result

let decls_to_fileinfo = Direct_decl_parser.decls_to_fileinfo
