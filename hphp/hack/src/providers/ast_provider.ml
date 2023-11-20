(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* Table containing all the Abstract Syntax Trees (cf ast.ml) for each file.*)
(*****************************************************************************)

(* We store only the names and declarations in the ParserHeap.
   The full flag in each function runs a full parsing with method bodies. *)

type parse_type =
  | Decl
  | Full

module ParserHeap =
  SharedMem.HeapWithLocalCache
    (SharedMem.ImmediateBackend (SharedMem.Evictable)) (Relative_path.S)
    (struct
      type t = Nast.program * parse_type

      let description = "Ast_Parser"
    end)
    (struct
      let capacity = 1000
    end)

module LocalParserCache =
  SharedMem.FreqCache
    (Relative_path.S)
    (struct
      type t = Nast.program

      let description = "Ast_ParserLocal"
    end)
    (struct
      let capacity = 1000
    end)

let parse
    (popt : ParserOptions.t)
    ~(full : bool)
    ~(source_text : Full_fidelity_source_text.t) : Errors.t * Parser_return.t =
  let path = source_text.Full_fidelity_source_text.file_path in
  let parser_env =
    Full_fidelity_ast.make_env ~quick_mode:(not full) ~parser_options:popt path
  in
  let (err, result) =
    Errors.do_with_context path @@ fun () ->
    Full_fidelity_ast.from_source_text_with_legacy parser_env source_text
  in
  let ast = result.Parser_return.ast in
  let ast =
    if
      Relative_path.(is_hhi (prefix path))
      && ParserOptions.deregister_php_stdlib popt
    then
      Nast.deregister_ignored_attributes ast
    else
      ast
  in
  (err, { result with Parser_return.ast })

let get_from_local_cache ~full ctx file_name =
  let with_no_err ast = (Errors.empty, ast) in
  let fn = Relative_path.to_absolute file_name in
  match LocalParserCache.get file_name with
  | Some ast -> with_no_err ast
  | None ->
    let popt = Provider_context.get_popt ctx in
    let f contents =
      let contents =
        if FindUtils.file_filter fn then
          contents
        else
          ""
      in
      match Ide_parser_cache.get_ast_if_active popt file_name contents with
      | Some ast -> with_no_err ast.Parser_return.ast
      | None ->
        let source = Full_fidelity_source_text.make file_name contents in
        (match Full_fidelity_parser.parse_mode source with
        | None -> with_no_err []
        | Some _ ->
          (* It's up to Parsing_service to add parsing errors. *)
          let (err, result) =
            Errors.do_with_context file_name @@ fun () ->
            Full_fidelity_ast.defensive_program
              ~quick:(not full)
              popt
              file_name
              contents
          in
          (err, result.Parser_return.ast))
    in
    let (err, ast) =
      Option.value_map
        ~default:(with_no_err [])
        ~f
        (File_provider.get_contents file_name)
    in
    let ast =
      if
        Relative_path.(is_hhi (prefix file_name))
        && ParserOptions.deregister_php_stdlib popt
      then
        Nast.deregister_ignored_attributes ast
      else
        ast
    in
    if full && Errors.is_empty err then LocalParserCache.add file_name ast;
    (err, ast)

let compute_source_text ~(entry : Provider_context.entry) :
    Full_fidelity_source_text.t =
  match entry with
  | { Provider_context.source_text = Some source_text; _ } -> source_text
  | _ ->
    let contents = Provider_context.read_file_contents_exn entry in
    let source_text =
      Full_fidelity_source_text.make entry.Provider_context.path contents
    in
    entry.Provider_context.source_text <- Some source_text;
    source_text

(* Note that some callers may not actually need the AST errors. This could be
   improved with a method similar to the TAST-and-errors generation, where the TAST
   errors are not generated unless necessary. *)
let compute_parser_return_and_ast_errors
    ~(popt : ParserOptions.t) ~(entry : Provider_context.entry) :
    Parser_return.t * Errors.t =
  match entry with
  | {
   Provider_context.ast_errors = Some ast_errors;
   parser_return = Some parser_return;
   _;
  } ->
    (parser_return, ast_errors)
  | _ ->
    let source_text = compute_source_text ~entry in
    let (ast_errors, parser_return) = parse popt ~full:true ~source_text in
    entry.Provider_context.ast_errors <- Some ast_errors;
    entry.Provider_context.parser_return <- Some parser_return;
    (parser_return, ast_errors)

let compute_cst ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Provider_context.PositionedSyntaxTree.t =
  (* TODO: use parser options inside ctx *)
  let _ = ctx in
  match entry.Provider_context.cst with
  | Some cst -> cst
  | None ->
    let source_text = compute_source_text ~entry in
    let cst = Provider_context.PositionedSyntaxTree.make source_text in
    entry.Provider_context.cst <- Some cst;
    cst

let compute_ast_with_error
    ~(popt : ParserOptions.t) ~(entry : Provider_context.entry) :
    Errors.t * Nast.program =
  let ({ Parser_return.ast; _ }, ast_errors) =
    compute_parser_return_and_ast_errors ~popt ~entry
  in
  (ast_errors, ast)

let compute_ast ~(popt : ParserOptions.t) ~(entry : Provider_context.entry) :
    Nast.program =
  compute_ast_with_error ~popt ~entry |> snd

let compute_comments ~(popt : ParserOptions.t) ~(entry : Provider_context.entry)
    : Parser_return.comments =
  let ({ Parser_return.comments; _ }, _ast_errors) =
    compute_parser_return_and_ast_errors ~popt ~entry
  in
  comments

let compute_file_info
    ~(popt : ParserOptions.t) ~(entry : Provider_context.entry) : FileInfo.t =
  let ast = compute_ast ~popt ~entry in
  Nast.get_def_names ast

let get_ast_with_error ~(full : bool) ctx path =
  Counters.count Counters.Category.Get_ast @@ fun () ->
  let parse_from_disk_no_caching ~apply_file_filter =
    let absolute_path = Relative_path.to_absolute path in
    if (not apply_file_filter) || FindUtils.file_filter absolute_path then
      let contents = Sys_utils.cat absolute_path in
      let source_text = Full_fidelity_source_text.make path contents in
      let (err, { Parser_return.ast; _ }) =
        parse (Provider_context.get_popt ctx) ~full ~source_text
      in
      (err, ast)
    else
      (Errors.empty, [])
  in

  (* If there's a ctx, and this file is in the ctx, then use ctx. *)
  (* Otherwise, the way we fetch/cache ASTs depends on the provider. *)
  let entry_opt =
    Relative_path.Map.find_opt (Provider_context.get_entries ctx) path
  in
  match (entry_opt, Provider_context.get_backend ctx) with
  | (_, Provider_backend.Pessimised_shared_memory info)
    when not info.Provider_backend.allow_ast_caching ->
    parse_from_disk_no_caching ~apply_file_filter:true
  | (Some entry, _) ->
    (* See documentation on `entry` for its invariants.
       The compute_ast function will use the cached (full) AST if present,
       and otherwise will compute a full AST and cache it and return it.
       It's okay for get_ast to return a full AST even if only asked for
       a partial one. Our principle is that an ctx entry always indicates that
       the file is open in the IDE, and so will benefit from a full AST at
       some time, so we might as well get it now. *)
    compute_ast_with_error ~popt:(Provider_context.get_popt ctx) ~entry
  | ( _,
      ( Provider_backend.Rust_provider_backend _
      | Provider_backend.Shared_memory
      | Provider_backend.Pessimised_shared_memory _ ) ) -> begin
    (* Note that we might be looking up the shared ParserHeap directly, *)
    (* or maybe into a local-change-stack due to quarantine. *)
    match (ParserHeap.get path, full) with
    | (None, true)
    | (Some (_, Decl), true) ->
      (* If we need full, and parser-heap can't provide it, then we *)
      (* don't want to write a full decl into the parser heap. *)
      get_from_local_cache ~full ctx path
    | (None, false) ->
      (* This is the case where we will write into the parser heap. *)
      let (err, ast) = get_from_local_cache ~full ctx path in
      if Errors.is_empty err then ParserHeap.add path (ast, Decl);
      (err, ast)
    | (Some (ast, _), _) ->
      (* It's in the parser-heap! hurrah! *)
      (Errors.empty, ast)
  end
  | (_, Provider_backend.Analysis) -> begin
    (* Zoncolan has its own caching layers and does not make use of Hack's.
       However, for unit tests, split files end up in the parser heap.
    *)
    match (ParserHeap.get path, full) with
    | (Some (ast, Full), _) ->
      (* It's in the parser-heap! hurrah! *)
      (Errors.empty, ast)
    | _ -> parse_from_disk_no_caching ~apply_file_filter:false
  end
  | (_, Provider_backend.Local_memory _) ->
    (* We never cache ASTs for this provider. There'd be no use. *)
    (* The only valuable caching is to cache decls. *)
    parse_from_disk_no_caching ~apply_file_filter:false

let get_ast ~(full : bool) ctx path = get_ast_with_error ~full ctx path |> snd

let get_def
    ~(full : bool)
    ctx
    file_name
    (node_getter : Nast.def -> ('a * string) option)
    (name_matcher : string -> bool) : 'a option =
  let defs = get_ast ~full ctx file_name in
  let rec get acc defs =
    List.fold_left defs ~init:acc ~f:(fun acc def ->
        match def with
        | Aast.Namespace (_, defs) -> get acc defs
        | _ -> begin
          match node_getter def with
          | Some (node, name) when name_matcher name -> Some node
          | _ -> acc
        end)
  in
  get None defs

let find_class_impl (def : Nast.def) : (Nast.class_ * string) option =
  match def with
  | Aast.Class c -> Some (c, snd c.Aast.c_name)
  | _ -> None

let find_fun_impl def =
  match def with
  | Aast.Fun f -> Some (f, snd f.Aast.fd_name)
  | _ -> None

let find_typedef_impl def =
  match def with
  | Aast.Typedef t -> Some (t, snd t.Aast.t_name)
  | _ -> None

let find_const_impl def =
  match def with
  | Aast.Constant cst -> Some (cst, snd cst.Aast.cst_name)
  | _ -> None

let find_module_impl def =
  match def with
  | Aast.Module md -> Some (md, snd md.Aast.md_name)
  | _ -> None

let iequal name =
  let name = Caml.String.lowercase_ascii name in
  (fun s -> String.equal name (Caml.String.lowercase_ascii s))

let find_class_in_file ~(full : bool) ctx file_name name =
  get_def ~full ctx file_name find_class_impl (String.equal name)

let find_iclass_in_file ctx file_name iname =
  get_def ctx file_name find_class_impl (iequal iname) ~full:false

let find_fun_in_file ~(full : bool) ctx file_name name =
  get_def ~full ctx file_name find_fun_impl (String.equal name)

let find_ifun_in_file ctx file_name iname =
  get_def ctx file_name find_fun_impl (iequal iname) ~full:false

let find_typedef_in_file ~(full : bool) ctx file_name name =
  get_def ~full ctx file_name find_typedef_impl (String.equal name)

let find_itypedef_in_file ctx file_name iname =
  get_def ctx file_name find_typedef_impl (iequal iname) ~full:false

let find_gconst_in_file ~(full : bool) ctx file_name name =
  get_def ~full ctx file_name find_const_impl (String.equal name)

let find_module_in_file ~(full : bool) ctx file_name name =
  get_def ~full ctx file_name find_module_impl (String.equal name)

let local_changes_push_sharedmem_stack () =
  ParserHeap.LocalChanges.push_stack ()

let local_changes_pop_sharedmem_stack () = ParserHeap.LocalChanges.pop_stack ()

let provide_ast_hint
    (path : Relative_path.t) (program : Nast.program) (parse_type : parse_type)
    : unit =
  match Provider_backend.get () with
  | Provider_backend.Analysis -> failwith "Should not write into parser heap"
  | Provider_backend.Rust_provider_backend _
  | Provider_backend.Pessimised_shared_memory _
  | Provider_backend.Shared_memory ->
    ParserHeap.write_around path (program, parse_type)
  | Provider_backend.Local_memory _ -> ()

let remove_batch paths = ParserHeap.remove_batch paths

let has_for_test (path : Relative_path.t) : bool = ParserHeap.mem path

let clear_parser_cache () = ParserHeap.Cache.clear ()

let clear_local_cache () = LocalParserCache.clear ()
