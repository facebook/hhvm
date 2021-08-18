(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let cache_decls ctx decls =
  let open Shallow_decl_defs in
  let open Typing_defs in
  match Provider_context.get_backend ctx with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory ->
    List.iter decls ~f:(function
        | (name, Class decl) ->
          Shallow_classes_heap.Classes.add name decl;
          if
            TypecheckerOptions.shallow_class_decl
              (Provider_context.get_tcopt ctx)
          then
            Shallow_classes_heap.MemberFilters.add decl
        | (name, Fun decl) -> Decl_store.((get ()).add_fun name decl)
        | (name, Record decl) -> Decl_store.((get ()).add_recorddef name decl)
        | (name, Typedef decl) -> Decl_store.((get ()).add_typedef name decl)
        | (name, Const decl) -> Decl_store.((get ()).add_gconst name decl))
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
        | (name, Record decl) ->
          let (_ : record_def_type option) =
            Provider_backend.Decl_cache.find_or_add
              decl_cache
              ~key:(Provider_backend.Decl_cache_entry.Record_decl name)
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
          ())
  | Provider_backend.Decl_service _ ->
    failwith
      "Direct_decl_utils.cache_file_decls not implemented for Decl_service"

let get_file_contents ctx filename =
  match
    Relative_path.Map.find_opt (Provider_context.get_entries ctx) filename
  with
  | Some entry ->
    let source_text = Ast_provider.compute_source_text ~entry in
    Some (Full_fidelity_source_text.text source_text)
  | None -> File_provider.get_contents filename

let direct_decl_parse_and_cache ?(decl_hash = false) ctx file =
  match get_file_contents ctx file with
  | None -> None
  | Some contents ->
    let popt = Provider_context.get_popt ctx in
    let opts = DeclParserOptions.from_parser_options popt in
    let (decls, mode, hash) =
      Direct_decl_parser.parse_decls_and_mode_ffi opts file contents decl_hash
    in
    let deregister_php_stdlib =
      Relative_path.is_hhi (Relative_path.prefix file)
      && ParserOptions.deregister_php_stdlib popt
    in
    let is_stdlib_fun fun_decl = fun_decl.Typing_defs.fe_php_std_lib in
    let is_stdlib_class c =
      List.exists c.Shallow_decl_defs.sc_user_attributes ~f:(fun a ->
          String.equal
            Naming_special_names.UserAttributes.uaPHPStdLib
            (snd a.Typing_defs_core.ua_name))
    in
    let decls =
      if not deregister_php_stdlib then
        decls
      else
        let open Shallow_decl_defs in
        List.filter_map decls ~f:(function
            | (_, Fun f) when is_stdlib_fun f -> None
            | (_, Class c) when is_stdlib_class c -> None
            | (name, Class c) ->
              let keep_prop sp = not (sp_php_std_lib sp) in
              let keep_meth sm = not (sm_php_std_lib sm) in
              let c =
                {
                  c with
                  sc_props = List.filter c.sc_props ~f:keep_prop;
                  sc_sprops = List.filter c.sc_sprops ~f:keep_prop;
                  sc_methods = List.filter c.sc_methods ~f:keep_meth;
                  sc_static_methods =
                    List.filter c.sc_static_methods ~f:keep_meth;
                }
              in
              Some (name, Class c)
            | name_and_decl -> Some name_and_decl)
    in
    cache_decls ctx decls;
    Some (decls, mode, hash)
