(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(*****************************************************************************)
(* Table containing all the Abstract Syntax Trees (cf ast.ml) for each file.*)
(*****************************************************************************)

(* We store only the names and declarations in the ParserHeap.
   The full flag in each function runs a full parsing with method bodies. *)

type parse_type =
  | Decl
  | Full

module ParserHeap =
  SharedMem.WithCache (SharedMem.ProfiledImmediate) (Relative_path.S)
    (struct
      type t = Nast.program * parse_type

      let prefix = Prefix.make ()

      let description = "Ast_Parser"
    end)
    (struct
      let capacity = 1000
    end)

module LocalParserCache =
  SharedMem.LocalCache
    (Relative_path.S)
    (struct
      type t = Nast.program

      let prefix = Prefix.make ()

      let description = "Ast_ParserLocal"
    end)
    (struct
      let capacity = 1000
    end)

let parse
    ~(full : bool)
    ~(keep_errors : bool)
    ~(source_text : Full_fidelity_source_text.t) : Parser_return.t =
  let popt = Parser_options_provider.get () in
  let path = source_text.Full_fidelity_source_text.file_path in
  let parser_env =
    Full_fidelity_ast.make_env
      ~quick_mode:(not full)
      ~keep_errors
      ~parser_options:popt
      path
  in
  let result =
    Full_fidelity_ast.from_source_text_with_legacy parser_env source_text
  in
  let ast = result.Parser_return.ast in
  let ast =
    if
      Relative_path.prefix path = Relative_path.Hhi
      && ParserOptions.deregister_php_stdlib popt
    then
      Nast.deregister_ignored_attributes ast
    else
      ast
  in
  { result with Parser_return.ast }

let get_from_local_cache ~full file_name =
  let fn = Relative_path.to_absolute file_name in
  match LocalParserCache.get file_name with
  | Some ast -> ast
  | None ->
    let popt = Parser_options_provider.get () in
    let f contents =
      let contents =
        if FindUtils.file_filter fn then
          contents
        else
          ""
      in
      match Ide_parser_cache.get_ast_if_active popt file_name contents with
      | Some ast -> ast.Parser_return.ast
      | None ->
        let source = Full_fidelity_source_text.make file_name contents in
        (match Full_fidelity_parser.parse_mode source with
        | None
        | Some FileInfo.Mphp ->
          []
        | Some _ ->
          (Full_fidelity_ast.defensive_program
             ~quick:(not full)
             popt
             file_name
             contents)
            .Parser_return.ast)
    in
    let ast =
      Option.value_map ~default:[] ~f (File_provider.get_contents file_name)
    in
    let ast =
      if
        Relative_path.prefix file_name = Relative_path.Hhi
        && ParserOptions.deregister_php_stdlib popt
      then
        Nast.deregister_ignored_attributes ast
      else
        ast
    in
    let () = if full then LocalParserCache.add file_name ast in
    ast

let get_class ?(case_insensitive = false) defs class_name =
  let class_name =
    if case_insensitive then
      Caml.String.lowercase_ascii class_name
    else
      class_name
  in
  let rec get acc defs =
    List.fold_left defs ~init:acc ~f:(fun acc def ->
        match def with
        | Aast.Class c ->
          let def_name = snd c.Aast.c_name in
          let def_name =
            if case_insensitive then
              Caml.String.lowercase_ascii def_name
            else
              def_name
          in
          if def_name = class_name then
            Some c
          else
            acc
        | Aast.Namespace (_, defs) -> get acc defs
        | _ -> acc)
  in
  get None defs

let get_record_def ?(case_insensitive = false) defs record_name =
  let record_name =
    if case_insensitive then
      Caml.String.lowercase_ascii record_name
    else
      record_name
  in
  let rec get acc defs =
    List.fold_left defs ~init:acc ~f:(fun acc def ->
        match def with
        | Aast.RecordDef rd ->
          let def_name = snd rd.Aast.rd_name in
          let def_name =
            if case_insensitive then
              Caml.String.lowercase_ascii def_name
            else
              def_name
          in
          if def_name = record_name then
            Some rd
          else
            acc
        | Aast.Namespace (_, defs) -> get acc defs
        | _ -> acc)
  in
  get None defs

let get_fun ?(case_insensitive = false) defs fun_name =
  let fun_name =
    if case_insensitive then
      Caml.String.lowercase_ascii fun_name
    else
      fun_name
  in
  let rec get acc defs =
    List.fold_left defs ~init:acc ~f:(fun acc def ->
        match def with
        | Aast.Fun f ->
          let def_name = snd f.Aast.f_name in
          let def_name =
            if case_insensitive then
              Caml.String.lowercase_ascii def_name
            else
              def_name
          in
          if def_name = fun_name then
            Some f
          else
            acc
        | Aast.Namespace (_, defs) -> get acc defs
        | _ -> acc)
  in
  get None defs

let get_typedef ?(case_insensitive = false) defs name =
  let name =
    if case_insensitive then
      Caml.String.lowercase_ascii name
    else
      name
  in
  let rec get acc defs =
    List.fold_left defs ~init:acc ~f:(fun acc def ->
        match def with
        | Aast.Typedef typedef ->
          let def_name = snd typedef.Aast.t_name in
          let def_name =
            if case_insensitive then
              Caml.String.lowercase_ascii def_name
            else
              def_name
          in
          if def_name = name then
            Some typedef
          else
            acc
        | Aast.Namespace (_, defs) -> get acc defs
        | _ -> acc)
  in
  get None defs

let get_gconst defs name =
  let rec get acc defs =
    List.fold_left defs ~init:acc ~f:(fun acc def ->
        match def with
        | Aast.Constant cst when snd cst.Aast.cst_name = name -> Some cst
        | Aast.Namespace (_, defs) -> get acc defs
        | _ -> acc)
  in
  get None defs

let get_ast ?(full = false) ctx path =
  Counters.count_get_ast @@ fun () ->
  (* If there's a ctx, and this file is in the ctx, then use ctx. *)
  (* Otherwise, the way we fetch/cache ASTs depends on the provider. *)
  let entry_opt =
    Relative_path.Map.find_opt ctx.Provider_context.entries path
  in
  match (entry_opt, ctx.Provider_context.backend) with
  | ( Some { Provider_context.parser_return = Some { Parser_return.ast; _ }; _ },
      _ ) ->
    ast
  | (_, Provider_backend.Shared_memory) ->
    begin
      (* Note that we might be looking up the shared ParserHeap directly, *)
      (* or maybe into a local-change-stack due to quarantine. *)
      match (ParserHeap.get path, full) with
      | (None, true)
      | (Some (_, Decl), true) ->
        (* If we need full, and parser-heap can't provide it, then we *)
        (* don't want to write a full decl into the parser heap. *)
        get_from_local_cache ~full path
      | (None, false) ->
        (* This is the case where we will write into the parser heap. *)
        let ast = get_from_local_cache ~full path in
        ParserHeap.add path (ast, Decl);
        ast
      | (Some (ast, _), _) ->
        (* It's in the parser-heap! hurrah! *)
        ast
    end
  | (_, Provider_backend.Local_memory _) ->
    (* We never cache ASTs for this provider. There'd be no use. *)
    (* The only valuable caching is to cache decls. *)
    let contents = Sys_utils.cat (Relative_path.to_absolute path) in
    let source_text = Full_fidelity_source_text.make path contents in
    let { Parser_return.ast; _ } =
      parse ~full ~keep_errors:false ~source_text
    in
    ast
  | (_, Provider_backend.Decl_service _) ->
    failwith "Ast_provider.get_ast not supported with decl memory provider"

let find_class_in_file
    ?(full = false) ?(case_insensitive = false) ctx file_name class_name =
  get_class (get_ast ~full ctx file_name) ~case_insensitive class_name

let find_record_def_in_file
    ?(full = false) ?(case_insensitive = false) ctx file_name record_name =
  get_record_def (get_ast ~full ctx file_name) ~case_insensitive record_name

let find_fun_in_file
    ?(full = false) ?(case_insensitive = false) ctx file_name fun_name =
  get_fun (get_ast ~full ctx file_name) ~case_insensitive fun_name

let find_typedef_in_file
    ?(full = false) ?(case_insensitive = false) ctx file_name name =
  get_typedef (get_ast ~full ctx file_name) ~case_insensitive name

let find_gconst_in_file ?(full = false) ctx file_name name =
  get_gconst (get_ast ~full ctx file_name) name

let local_changes_push_stack () = ParserHeap.LocalChanges.push_stack ()

let local_changes_pop_stack () = ParserHeap.LocalChanges.pop_stack ()

let local_changes_commit_batch paths =
  ParserHeap.LocalChanges.commit_batch paths

let local_changes_revert_batch paths =
  ParserHeap.LocalChanges.revert_batch paths

let provide_ast_hint
    (path : Relative_path.t) (program : Nast.program) (parse_type : parse_type)
    : unit =
  match Provider_backend.get () with
  | Provider_backend.Shared_memory ->
    ParserHeap.write_around path (program, parse_type)
  | Provider_backend.Local_memory _
  | Provider_backend.Decl_service _ ->
    ()

let remove_batch paths = ParserHeap.remove_batch paths

let has_for_test (path : Relative_path.t) : bool = ParserHeap.mem path
