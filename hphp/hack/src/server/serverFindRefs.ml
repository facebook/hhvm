(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Option.Monad_infix
open ServerEnv
open Reordered_argument_collections
open ServerCommandTypes.Find_refs
open ServerCommandTypes.Done_or_retry

let to_json input =
  let entries =
    List.map input (fun (name, pos) ->
        let filename = Pos.filename pos in
        let (line, start, end_) = Pos.info_pos pos in
        Hh_json.JSON_Object
          [
            ("name", Hh_json.JSON_String name);
            ("filename", Hh_json.JSON_String filename);
            ("line", Hh_json.int_ line);
            ("char_start", Hh_json.int_ start);
            ("char_end", Hh_json.int_ end_);
          ])
  in
  Hh_json.JSON_Array entries

let add_ns name =
  if Char.equal name.[0] '\\' then
    name
  else
    "\\" ^ name

let strip_ns results = List.map results (fun (s, p) -> (Utils.strip_ns s, p))

let search ctx target include_defs files genv =
  if Hh_logger.Level.passes_min_level Hh_logger.Level.Debug then
    List.iter files ~f:(fun file ->
        Hh_logger.debug
          "ServerFindRefs.search file %s"
          (Relative_path.to_absolute file));
  (* Get all the references to the provided target in the files *)
  let res =
    FindRefsService.find_references ctx genv.workers target include_defs files
  in
  strip_ns res

let handle_prechecked_files ctx genv env dep f =
  (* We need to handle prechecked files here to get accurate results. *)
  let deps_mode = Provider_context.get_deps_mode ctx in
  let dep = Typing_deps.DepSet.singleton deps_mode dep in
  (* All the callers of this should be listed in ServerCommand.rpc_command_needs_full_check,
   * and server should never call this before completing full check *)
  assert (is_full_check_done env.full_check_status);
  let start_time = Unix.gettimeofday () in
  let (env, _telemetry) =
    ServerPrecheckedFiles.update_after_local_changes genv env dep ~start_time
  in
  (* If we added more things to recheck, we can't handle this command now, and
   * tell the client to retry instead. *)
  if is_full_check_done env.full_check_status then
    let () = Hh_logger.debug "ServerFindRefs.handle_prechecked_files: Done" in
    let callback_result = f () in
    (env, Done callback_result)
  else
    let () = Hh_logger.debug "ServerFindRefs.handle_prechecked_files: Retry" in
    (env, Retry)

let search_function ctx function_name include_defs genv env =
  let deps_mode = Provider_context.get_deps_mode ctx in
  let function_name = add_ns function_name in
  Hh_logger.debug "ServerFindRefs.search_function: %s" function_name;
  handle_prechecked_files
    ctx
    genv
    env
    Typing_deps.(Dep.(make (hash_mode deps_mode) (Fun function_name)))
  @@ fun () ->
  let files =
    FindRefsService.get_dependent_files_function
      ctx
      genv.ServerEnv.workers
      function_name
    |> Relative_path.Set.elements
  in
  search ctx (FindRefsService.IFunction function_name) include_defs files genv

let search_member ctx class_name member include_defs genv env =
  let class_name = add_ns class_name in
  let class_name =
    FindRefsService.get_origin_class_name ctx class_name member
  in
  let deps_mode = Provider_context.get_deps_mode ctx in
  handle_prechecked_files
    ctx
    genv
    env
    Typing_deps.(Dep.(make (hash_mode deps_mode) (Type class_name)))
  @@ fun () ->
  (* Find all the classes that extend this one *)
  let files = FindRefsService.get_child_classes_files ctx class_name in
  let all_classes =
    FindRefsService.find_child_classes ctx class_name env.naming_table files
  in
  let all_classes = SSet.add all_classes class_name in
  (* Get all the files that reference those classes *)
  let files =
    FindRefsService.get_dependent_files ctx genv.ServerEnv.workers all_classes
    |> Relative_path.Set.elements
  in
  let target =
    FindRefsService.IMember (FindRefsService.Class_set all_classes, member)
  in
  search ctx target include_defs files genv

let search_gconst ctx cst_name include_defs genv env =
  let cst_name = add_ns cst_name in
  let deps_mode = Provider_context.get_deps_mode ctx in
  handle_prechecked_files
    ctx
    genv
    env
    Typing_deps.(Dep.(make (hash_mode deps_mode) (GConst cst_name)))
  @@ fun () ->
  let files =
    FindRefsService.get_dependent_files_gconst
      ctx
      genv.ServerEnv.workers
      cst_name
    |> Relative_path.Set.elements
  in
  search ctx (FindRefsService.IGConst cst_name) include_defs files genv

let search_class ctx class_name include_defs include_all_ci_types genv env =
  let class_name = add_ns class_name in
  let target =
    if include_all_ci_types then
      FindRefsService.IClass class_name
    else
      FindRefsService.IExplicitClass class_name
  in
  let deps_mode = Provider_context.get_deps_mode ctx in
  handle_prechecked_files
    ctx
    genv
    env
    Typing_deps.(Dep.(make (hash_mode deps_mode) (Type class_name)))
  @@ fun () ->
  let files =
    FindRefsService.get_dependent_files
      ctx
      genv.ServerEnv.workers
      (SSet.singleton class_name)
    |> Relative_path.Set.elements
  in
  search ctx target include_defs files genv

let search_record ctx record_name include_defs genv env =
  let record_name = add_ns record_name in
  let deps_mode = Provider_context.get_deps_mode ctx in
  handle_prechecked_files
    ctx
    genv
    env
    Typing_deps.(Dep.(make (hash_mode deps_mode) (Type record_name)))
  @@ fun () ->
  let files =
    FindRefsService.get_dependent_files
      ctx
      genv.ServerEnv.workers
      (SSet.singleton record_name)
    |> Relative_path.Set.elements
  in
  search ctx (FindRefsService.IRecord record_name) include_defs files genv

let search_localvar ~ctx ~entry ~line ~char =
  let results = ServerFindLocals.go ~ctx ~entry ~line ~char in
  match results with
  | first_pos :: _ ->
    let content = Provider_context.read_file_contents_exn entry in
    let var_text = Pos.get_text_from_pos ~content first_pos in
    List.map results (fun x -> (var_text, x))
  | [] -> []

let go ctx action include_defs genv env =
  match action with
  | Member (class_name, member) ->
    search_member ctx class_name member include_defs genv env
  | Function function_name ->
    search_function ctx function_name include_defs genv env
  | Class class_name ->
    let include_all_ci_types = true in
    search_class ctx class_name include_defs include_all_ci_types genv env
  | ExplicitClass class_name ->
    let include_all_ci_types = false in
    search_class ctx class_name include_defs include_all_ci_types genv env
  | Record record_name -> search_record ctx record_name include_defs genv env
  | GConst cst_name -> search_gconst ctx cst_name include_defs genv env
  | LocalVar { filename; file_content; line; char } ->
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents
        ~ctx
        ~path:filename
        ~contents:file_content
    in
    (env, Done (search_localvar ~ctx ~entry ~line ~char))

let to_absolute res = List.map res (fun (r, pos) -> (r, Pos.to_absolute pos))

let to_ide symbol_name res =
  Some (symbol_name, List.map ~f:snd (to_absolute res))

let get_action symbol (filename, file_content, line, char) =
  let name = symbol.SymbolOccurrence.name in
  match symbol.SymbolOccurrence.type_ with
  | SymbolOccurrence.Class _ -> Some (Class name)
  | SymbolOccurrence.Function -> Some (Function name)
  | SymbolOccurrence.Method (class_name, method_name) ->
    Some (Member (class_name, Method method_name))
  | SymbolOccurrence.Property (class_name, prop_name)
  | SymbolOccurrence.XhpLiteralAttr (class_name, prop_name) ->
    Some (Member (class_name, Property prop_name))
  | SymbolOccurrence.Record -> Some (Record name)
  | SymbolOccurrence.ClassConst (class_name, const_name) ->
    Some (Member (class_name, Class_const const_name))
  | SymbolOccurrence.Typeconst (class_name, tconst_name) ->
    Some (Member (class_name, Typeconst tconst_name))
  | SymbolOccurrence.GConst -> Some (GConst name)
  | SymbolOccurrence.LocalVar ->
    Some (LocalVar { filename; file_content; line; char })
  | SymbolOccurrence.Attribute _ -> None
  (* TODO(toyang): find references doesn't work for enum atoms yet *)
  | SymbolOccurrence.EnumAtom _ -> None

let go_from_file_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : (string * ServerCommandTypes.Find_refs.action) option =
  (* Find the symbol at given position *)
  ServerIdentifyFunction.go_quarantined ~ctx ~entry ~line ~column
  |> (* If there are few, arbitrarily pick the first *)
  List.hd
  >>= fun (occurrence, definition) ->
  (* Ignore symbols that lack definitions *)
  definition >>= fun definition ->
  let source_text = Ast_provider.compute_source_text ~entry in
  get_action
    occurrence
    ( entry.Provider_context.path,
      source_text.Full_fidelity_source_text.text,
      line,
      column )
  >>= fun action -> Some (definition.SymbolDefinition.full_name, action)
