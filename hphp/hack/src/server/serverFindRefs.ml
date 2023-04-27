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
    List.map input ~f:(fun (name, pos) ->
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

let strip_ns results = List.map results ~f:(fun (s, p) -> (Utils.strip_ns s, p))

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

let handle_prechecked_files genv env dep f =
  (* We need to handle prechecked files here to get accurate results. *)
  let dep = Typing_deps.DepSet.singleton dep in
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
  let function_name = add_ns function_name in
  Hh_logger.debug "ServerFindRefs.search_function: %s" function_name;
  handle_prechecked_files genv env Typing_deps.(Dep.(make (Fun function_name)))
  @@ fun () ->
  let files =
    FindRefsService.get_dependent_files_function
      ctx
      genv.ServerEnv.workers
      function_name
    |> Relative_path.Set.elements
  in
  search ctx (FindRefsService.IFunction function_name) include_defs files genv

let search_member
    ctx
    (class_name : string)
    (member : member)
    ~(include_defs : bool)
    (genv : genv)
    (env : env) : env * (string * Pos.t) list t =
  let class_name = add_ns class_name in
  let origin_class_name =
    FindRefsService.get_origin_class_name ctx class_name member
  in
  handle_prechecked_files
    genv
    env
    Typing_deps.(Dep.(make (Type origin_class_name)))
  @@ fun () ->
  let (descendant_class_files, member_use_files) =
    FindRefsService
    .get_files_for_descendants_and_dependents_of_members_in_descendants
      ctx
      ~class_name
      (dep_member_of member)
  in
  let descendant_classes =
    FindRefsService.find_child_classes
      ctx
      origin_class_name
      env.naming_table
      descendant_class_files
  in
  let class_and_descendants = SSet.add descendant_classes origin_class_name in
  let files =
    Relative_path.Set.union descendant_class_files member_use_files
    |> Relative_path.Set.elements
  in
  let target =
    FindRefsService.IMember
      (FindRefsService.Class_set class_and_descendants, member)
  in
  search ctx target include_defs files genv

let search_gconst ctx cst_name include_defs genv env =
  let cst_name = add_ns cst_name in
  handle_prechecked_files genv env Typing_deps.(Dep.(make (GConst cst_name)))
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
  handle_prechecked_files genv env Typing_deps.(Dep.(make (Type class_name)))
  @@ fun () ->
  let files =
    FindRefsService.get_dependent_files
      ctx
      genv.ServerEnv.workers
      (SSet.singleton class_name)
    |> Relative_path.Set.elements
  in
  search ctx target include_defs files genv

let search_localvar ~ctx ~entry ~line ~char =
  let results = ServerFindLocals.go ~ctx ~entry ~line ~char in
  match results with
  | first_pos :: _ ->
    let content = Provider_context.read_file_contents_exn entry in
    let var_text = Pos.get_text_from_pos ~content first_pos in
    List.map results ~f:(fun x -> (var_text, x))
  | [] -> []

let is_local = function
  | LocalVar _ -> true
  | _ -> false

let go ctx action include_defs genv env =
  match action with
  | Member (class_name, member) ->
    search_member ctx class_name member ~include_defs genv env
  | Function function_name ->
    search_function ctx function_name include_defs genv env
  | Class class_name ->
    let include_all_ci_types = true in
    search_class ctx class_name include_defs include_all_ci_types genv env
  | ExplicitClass class_name ->
    let include_all_ci_types = false in
    search_class ctx class_name include_defs include_all_ci_types genv env
  | GConst cst_name -> search_gconst ctx cst_name include_defs genv env
  | LocalVar { filename; file_content; line; char } ->
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents
        ~ctx
        ~path:filename
        ~contents:file_content
    in
    (env, Done (search_localvar ~ctx ~entry ~line ~char))

let go_for_localvar ctx action =
  match action with
  | LocalVar { filename; file_content; line; char } ->
    let (ctx, entry) =
      Provider_context.add_or_overwrite_entry_contents
        ~ctx
        ~path:filename
        ~contents:file_content
    in
    Ok (search_localvar ~ctx ~entry ~line ~char)
  | _ -> Error action

let to_absolute res = List.map res ~f:(fun (r, pos) -> (r, Pos.to_absolute pos))

let to_ide symbol_name res =
  Some (symbol_name, List.map ~f:snd (to_absolute res))

let get_action symbol (filename, file_content, line, char) =
  let module SO = SymbolOccurrence in
  let name = symbol.SymbolOccurrence.name in
  match symbol.SymbolOccurrence.type_ with
  | SO.Class _ -> Some (Class name)
  | SO.Function -> Some (Function name)
  | SO.Method (SO.ClassName class_name, method_name) ->
    Some (Member (class_name, Method method_name))
  | SO.Property (SO.ClassName class_name, prop_name)
  | SO.XhpLiteralAttr (class_name, prop_name) ->
    Some (Member (class_name, Property prop_name))
  | SO.ClassConst (SO.ClassName class_name, const_name) ->
    Some (Member (class_name, Class_const const_name))
  | SO.Typeconst (class_name, tconst_name) ->
    Some (Member (class_name, Typeconst tconst_name))
  | SO.GConst -> Some (GConst name)
  | SO.LocalVar -> Some (LocalVar { filename; file_content; line; char })
  | SO.TypeVar -> None
  | SO.Attribute _ -> Some (Class name)
  | SO.Keyword _
  | SO.PureFunctionContext
  | SO.BuiltInType _
  | SO.BestEffortArgument _
  | SO.HhFixme
  | SO.Method (SO.UnknownClass, _)
  | SO.Property (SO.UnknownClass, _)
  | SO.ClassConst (SO.UnknownClass, _)
  | SO.Module ->
    None
  (* TODO(toyang): find references doesn't work for enum labels yet *)
  | SO.EnumClassLabel _ -> None

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
