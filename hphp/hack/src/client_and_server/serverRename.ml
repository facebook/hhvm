(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv
open ServerRenameTypes

let maybe_add_dollar s =
  if not (Char.equal s.[0] '$') then
    "$" ^ s
  else
    s

let get_fixme_patches codes (env : env) =
  let fixmelist =
    Errors.get_error_list ~drop_fixmed:false env.errorl
    |> List.filter ~f:(fun e -> e.User_error.is_fixmed)
    |> List.map ~f:(fun e -> (User_error.get_pos e, User_error.get_code e))
  in
  let poslist =
    Fixme_provider.get_unused_fixmes
      ~codes
      ~applied_fixmes:fixmelist
      ~fold:(Naming_table.fold ~warn_on_naming_costly_iter:true)
      ~files_info:env.naming_table
  in
  List.map ~f:(fun pos -> Remove (Pos.to_absolute pos)) poslist

let get_dead_unsafe_cast_patches (env : env) =
  Remove_dead_unsafe_casts.get_patches
    ~is_test:false
    ~fold:(Naming_table.fold ~warn_on_naming_costly_iter:true)
    ~files_info:env.naming_table

let get_lambda_parameter_rewrite_patches ctx files =
  List.concat_map files ~f:(fun file ->
      ServerRewriteLambdaParameters.get_patches
        ctx
        (Relative_path.from_root ~suffix:file))

let find_def_filename current_filename definition =
  SymbolDefinition.(
    if Relative_path.equal (Pos.filename definition.pos) Relative_path.default
    then
      (* When the definition is in an IDE buffer with local changes, the filename
         in the definition will be empty. *)
      current_filename
    else
      Pos.filename definition.pos)

(*
  We construct the text for the deprecated wrapper here.

  Example of deprecated wrapper & its relation to the newly named function:

    <<__Deprecated("Deprecated: Use `newlyNamedFunction` instead")>>
    public function oldFunctionName(int $x, SomeClass $y, ...$nums): string {
      return $this->newlyNamedFunction($x, $y, ...$nums);
    }

    /**
     * Some docblock
     *
     */
    public function newlyNamedFunction(int $x, SomeClass $y, ...$nums): string {
      // some function body
    }

*)
let construct_deprecated_wrapper_stub
    ~(func_decl_text : string)
    ~(params_text_list : string list)
    ~(col_start : int)
    ~(returns_void : bool)
    ~(is_async : bool)
    ~(func_ref : deprecated_wrapper_function_ref)
    (new_name : string) : string =
  (* Since the starting column position points to the beginning of the function
      declaration header, we can use it to figure out the indentation level
      of the function, and insert whitespace accordingly *)
  let base_indentation = String.make col_start ' ' in
  let deprecated_header =
    base_indentation ^ "<<__Deprecated(\"Use `" ^ new_name ^ "` instead\")>>"
  in
  let func_decl = base_indentation ^ func_decl_text in
  (* The immediate body of a function is indented by 2 extra spaces *)
  let func_body_indentation = String.make 2 ' ' in
  let return_indentation = base_indentation ^ func_body_indentation in
  let parameter_input = String.concat ~sep:", " params_text_list in
  let maybe_return =
    if returns_void then
      ""
    else
      "return "
  in
  let maybe_await =
    if is_async then
      "await "
    else
      ""
  in
  let maybe_this_or_self =
    match func_ref with
    | DeprecatedStaticMethodRef -> "self::"
    | DeprecatedNonStaticMethodRef -> "$this->"
    | DeprecatedFunctionRef -> ""
  in
  let return_statement =
    return_indentation
    ^ maybe_return
    ^ maybe_await
    ^ maybe_this_or_self
    ^ new_name
    ^ "("
    ^ parameter_input
    ^ ");"
  in
  "\n"
  ^ deprecated_header
  ^ "\n"
  ^ func_decl
  ^ " {"
  ^ "\n"
  ^ return_statement
  ^ "\n"
  ^ base_indentation
  ^ "}"
  ^ "\n"

let get_pos_before_docblock_from_cst_node filename node =
  Full_fidelity_positioned_syntax.(
    let source_text = source_text node in
    let start_offset = leading_start_offset node in
    SourceText.relative_pos filename source_text start_offset start_offset)

(* Contains just enough information to properly wrap a function *)
type wrapper_call_signature_info = {
  params_text_list: string list;
  returns_void: bool;
  is_async: bool;
  is_static: bool;
}

(* Identify key information about a function so we can produce a deprecated wrapper *)
let get_call_signature_for_wrap (func_decl : Full_fidelity_positioned_syntax.t)
    : wrapper_call_signature_info =
  Full_fidelity_positioned_syntax.(
    match syntax func_decl with
    | FunctionDeclarationHeader
        {
          function_parameter_list = params;
          function_type = ret_type;
          function_modifiers = modifiers;
          _;
        } ->
      let params_text_list =
        match syntax params with
        | SyntaxList params ->
          let params_text_list =
            List.map params ~f:(fun param ->
                let param =
                  match syntax param with
                  | ListItem { list_item; _ } -> list_item
                  | _ -> failwith "Expected ListItem"
                in
                match syntax param with
                (* NOTE:
                    `ParameterDeclaration` includes regular params like "$x" and
                     _named_ variadic parameters like "...$nums". For the latter case,
                     we prepend the name with the ellipsis, in order to construct
                     the wrapped call.
                *)
                | ParameterDeclaration
                    { parameter_name = name; parameter_ellipsis = ellipsis; _ }
                  -> begin
                  match syntax ellipsis with
                  | Missing -> text name
                  | _ -> "..." ^ text name
                end
                | _ -> failwith "Expected some parameter type")
          in
          params_text_list
        | Missing -> []
        | _ -> []
      in
      let returns_void =
        match syntax ret_type with
        | GenericTypeSpecifier
            {
              generic_class_type = generic_type;
              generic_argument_list =
                {
                  syntax =
                    TypeArguments
                      {
                        type_arguments_types =
                          {
                            syntax =
                              SyntaxList
                                [
                                  {
                                    syntax =
                                      ListItem
                                        {
                                          list_item =
                                            {
                                              syntax =
                                                SimpleTypeSpecifier
                                                  {
                                                    simple_type_specifier =
                                                      type_spec;
                                                  };
                                              _;
                                            };
                                          _;
                                        };
                                    _;
                                  };
                                ];
                            _;
                          };
                        _;
                      };
                  _;
                };
              _;
            } ->
          String.equal (text generic_type) "Awaitable"
          && String.equal (text type_spec) "void"
        | SimpleTypeSpecifier { simple_type_specifier = type_spec } ->
          String.equal (text type_spec) "void"
        | _ -> false
      in
      let (is_async, is_static) =
        match syntax modifiers with
        | SyntaxList modifiers ->
          let is_async =
            List.exists modifiers ~f:(fun modifier ->
                String.equal (text modifier) "async")
          in
          let is_static =
            List.exists modifiers ~f:(fun modifier ->
                String.equal (text modifier) "static")
          in
          (is_async, is_static)
        | _ -> (false, false)
      in
      { params_text_list; returns_void; is_async; is_static }
    | _ ->
      {
        params_text_list = [];
        returns_void = false;
        is_async = false;
        is_static = false;
      })

let classish_is_interface (ctx : Provider_context.t) (name : string) : bool =
  match Decl_provider.get_class ctx (Utils.add_ns name) with
  | Decl_entry.DoesNotExist
  | Decl_entry.NotYetAvailable ->
    false
  | Decl_entry.Found cls ->
    (match Decl_provider.Class.kind cls with
    | Ast_defs.Cinterface -> true
    | _ -> false)

(* Produce a "deprecated" version of the old [definition] so that calls to it can be rerouted.
   If [definition] is None, this is a no-op. *)
let get_deprecated_wrapper_patch
    ~(definition : Relative_path.t SymbolDefinition.t option)
    ~(ctx : Provider_context.t)
    (new_name : string) : patch option =
  let filename =
    Option.bind definition ~f:(fun definition ->
        let filename = Pos.filename definition.SymbolDefinition.pos in
        let is_dummy = Relative_path.equal filename Relative_path.default in
        if is_dummy then
          HackEventLogger.invariant_violation_bug
            "--refactor has empty filename";
        Option.some_if (not is_dummy) filename)
  in
  SymbolDefinition.(
    Full_fidelity_positioned_syntax.(
      Option.Monad_infix.(
        filename >>= fun filename ->
        definition >>= fun definition ->
        (* We need the number of spaces that the function declaration is offsetted so that we can
           format our wrapper properly with the correct indent (i.e. we need 0-indexed columns).

           However, even though column offsets are already indexed accordingly when
           stored in positions, `destruct_range` adds 1 in order to
           return an [inclusive, exclusive) span.

           Thus, we subtract 1.
        *)
        let (_, col_start_plus1, _, _) = Pos.destruct_range definition.span in
        let col_start = col_start_plus1 - 1 in
        let (_ctx, entry) =
          Provider_context.add_entry_if_missing ~ctx ~path:filename
        in
        let cst_node =
          ServerSymbolDefinition.get_definition_cst_node_ctx
            ~ctx
            ~entry
            ~kind:definition.kind
            ~pos:definition.pos
        in

        cst_node >>= fun cst_node ->
        begin
          match syntax cst_node with
          | MethodishDeclaration
              { methodish_function_decl_header = func_decl; _ } ->
            let call_signature = get_call_signature_for_wrap func_decl in
            let func_decl_text = text func_decl in
            let func_ref =
              if call_signature.is_static then
                DeprecatedStaticMethodRef
              else
                DeprecatedNonStaticMethodRef
            in

            (match definition.class_name with
            | Some name when classish_is_interface ctx name ->
              (* We can't add a stub that calls the new name in
                 interfaces, as methods can't have bodies there. *)
              None
            | _ ->
              Some
                ( func_decl_text,
                  call_signature.params_text_list,
                  call_signature.returns_void,
                  call_signature.is_async,
                  func_ref ))
          | FunctionDeclaration { function_declaration_header = func_decl; _ }
            ->
            let call_signature = get_call_signature_for_wrap func_decl in
            let func_decl_text = text func_decl in
            let func_ref = DeprecatedFunctionRef in
            Some
              ( func_decl_text,
                call_signature.params_text_list,
                call_signature.returns_void,
                call_signature.is_async,
                func_ref )
          | _ -> None
        end
        >>| fun ( func_decl_text,
                  params_text_list,
                  returns_void,
                  is_async,
                  func_ref ) ->
        let deprecated_wrapper_stub =
          construct_deprecated_wrapper_stub
            ~func_decl_text
            ~params_text_list
            ~col_start
            ~returns_void
            ~is_async
            ~func_ref
            new_name
        in
        let filename = find_def_filename filename definition in
        let deprecated_wrapper_pos =
          get_pos_before_docblock_from_cst_node filename cst_node
        in
        let patch =
          {
            pos = Pos.to_absolute deprecated_wrapper_pos;
            text = deprecated_wrapper_stub;
          }
        in
        Insert patch)))

let method_might_support_dynamic ctx ~class_name ~method_name =
  let open Option.Monad_infix in
  let module Class = Decl_provider.Class in
  let sd_enabled =
    TypecheckerOptions.enable_sound_dynamic @@ Provider_context.get_tcopt ctx
  in
  (not sd_enabled)
  || Decl_provider.get_class ctx @@ Utils.add_ns class_name
     |> Decl_entry.to_option
     >>= (fun class_ ->
           Option.first_some
             (Class.get_smethod class_ method_name)
             (Class.get_method class_ method_name))
     >>| (fun elt ->
           let flags = elt.Typing_defs.ce_flags in
           Typing_defs_flags.ClassElt.(
             is_dynamicallycallable flags || supports_dynamic_type flags))
     |> Option.value ~default:true

let go
    ctx
    action
    genv
    env
    ~(definition_for_wrapper : Relative_path.t SymbolDefinition.t option) =
  let module Types = ServerCommandTypes.Find_refs in
  let (find_refs_action, new_name) =
    match action with
    | ClassRename (old_name, new_name) ->
      (Types.ExplicitClass old_name, new_name)
    | ClassConstRename (class_name, old_name, new_name) ->
      (Types.Member (class_name, Types.Class_const old_name), new_name)
    | MethodRename { class_name; old_name; new_name; _ } ->
      (Types.Member (class_name, Types.Method old_name), new_name)
    | FunctionRename { old_name; new_name; _ } ->
      (Types.Function old_name, new_name)
    | LocalVarRename { filename; file_content; line; char; new_name } ->
      (Types.LocalVar { filename; file_content; line; char }, new_name)
  in
  let include_defs = true in
  ServerFindRefs.go
    ctx
    find_refs_action
    include_defs
    ~stream_file:None
    ~hints:[]
    genv
    env
  |> ServerCommandTypes.Done_or_retry.map_env ~f:(fun refs ->
         let changes =
           let fold_to_positions_and_patches (positions, patches) (_, pos) =
             if Pos.Set.mem pos positions then
               (* Don't rename at the same position twice. Double-renames were happening (~~T157645473~~) because
                * ServerRename uses ServerFindRefs which searches the tast, which thinks Self::TWhatever
                * is a use of the current class at the position of the declaration of the current class.
                * *)
               (positions, patches)
             else
               let positions = Pos.Set.add pos positions in
               let replacement =
                 { pos = Pos.to_absolute pos; text = new_name }
               in
               let patch = Replace replacement in
               let patches = patch :: patches in
               (positions, patches)
           in
           refs
           |> List.fold_left
                ~init:(Pos.Set.empty, [])
                ~f:fold_to_positions_and_patches
           |> snd
         in
         let deprecated_wrapper_patch =
           match action with
           | FunctionRename _ ->
             get_deprecated_wrapper_patch
               ~definition:definition_for_wrapper
               ~ctx
               new_name
           | MethodRename { class_name; old_name; _ } ->
             if
               method_might_support_dynamic
                 ctx
                 ~class_name
                 ~method_name:old_name
             then
               get_deprecated_wrapper_patch
                 ~definition:definition_for_wrapper
                 ~ctx
                 new_name
             else
               None
           | ClassRename _
           | ClassConstRename _
           | LocalVarRename _ ->
             None
         in
         Option.value_map
           deprecated_wrapper_patch
           ~default:changes
           ~f:(fun patch -> patch :: changes))

let go_for_localvar ctx action new_name =
  let open Result.Monad_infix in
  let module Types = ServerCommandTypes.Find_refs in
  match action with
  | Types.LocalVar _ ->
    let changes =
      ServerFindRefs.go_for_localvar ctx action
      >>| List.fold_left
            ~f:(fun acc x ->
              let replacement =
                {
                  pos = Pos.to_absolute (snd x);
                  text = maybe_add_dollar new_name;
                }
              in
              let patch = Replace replacement in
              patch :: acc)
            ~init:[]
      >>| fun patches -> Some patches
    in
    changes
  | _ -> Error action

let go_for_single_file
    ctx ~find_refs_action ~new_name ~filename ~symbol_definition ~naming_table =
  let action =
    match find_refs_action with
    | ServerCommandTypes.Find_refs.Class str ->
      (* Note that [ServerRename.go] remaps ClassRename to Find_refs.ExplicitClass,
         so we manually handle that here for parity.
      *)
      ServerCommandTypes.Find_refs.ExplicitClass str
    | action -> action
  in
  ( ServerFindRefs.go_for_single_file
      ~ctx
      ~action
      ~filename
      ~name:symbol_definition.SymbolDefinition.full_name
      ~naming_table
  |> fun refs ->
    let changes =
      List.fold_left
        refs
        ~f:
          begin
            fun acc x ->
              let replacement =
                { pos = Pos.to_absolute (snd x); text = new_name }
              in
              let patch = Replace replacement in
              patch :: acc
          end
        ~init:[]
    in
    let should_write_deprecated_wrapper_patch =
      (* Context: D46818062
         When we rename symbols looking at one file at a time we always pass the [SymbolDefinition.t] along.
         Generating a deprecated_wrapper_patch looks at the symbol definition pos, checks to see if there's a matching
         CST node and if so, will send back a deprecated wrapper patch to write.

         However, if we're renaming a symbol in a file where the (method or function) is NOT defined,
         we should reject any deprecated wrapper patches. We only want a patch IFF the rename action is editing
         the file matching [symbol_definition.pos.file]
      *)
      Relative_path.equal
        filename
        (Pos.filename symbol_definition.SymbolDefinition.pos)
    in
    let definition =
      Option.some_if should_write_deprecated_wrapper_patch symbol_definition
    in
    let deprecated_wrapper_patch =
      let open ServerCommandTypes.Find_refs in
      match find_refs_action with
      | Function _ -> get_deprecated_wrapper_patch ~definition ~ctx new_name
      | Member (class_name, Method old_name) ->
        if method_might_support_dynamic ctx ~class_name ~method_name:old_name
        then
          get_deprecated_wrapper_patch ~definition ~ctx new_name
        else
          None
      | Class _
      | Member _
      | ExplicitClass _
      | LocalVar _
      | GConst _ ->
        None
    in
    if should_write_deprecated_wrapper_patch then
      Option.value_map
        deprecated_wrapper_patch
        ~default:changes
        ~f:(fun patch -> patch :: changes)
    else
      changes )
  |> fun rename_patches -> Ok rename_patches

(**
  Like go_ide, but rather than looking up a symbolDefinition manually from a file and
  converting a ServerRenameTypes.action to a Find_refs.action, we supply a Find_refs.action
  directly.
*)
let go_ide_with_find_refs_action
    ctx ~find_refs_action ~new_name ~symbol_definition genv env =
  let include_defs = true in
  ServerFindRefs.go
    ctx
    find_refs_action
    include_defs
    ~stream_file:None
    ~hints:[]
    genv
    env
  |> ServerCommandTypes.Done_or_retry.map_env ~f:(fun refs ->
         let changes =
           List.fold_left
             refs
             ~f:
               begin
                 fun acc x ->
                   let replacement =
                     { pos = Pos.to_absolute (snd x); text = new_name }
                   in
                   let patch = Replace replacement in
                   patch :: acc
               end
             ~init:[]
         in
         let deprecated_wrapper_patch =
           let open ServerCommandTypes.Find_refs in
           match find_refs_action with
           | Function _ ->
             get_deprecated_wrapper_patch
               ~definition:(Some symbol_definition)
               ~ctx
               new_name
           | Member (class_name, Method old_name) ->
             if
               method_might_support_dynamic
                 ctx
                 ~class_name
                 ~method_name:old_name
             then
               get_deprecated_wrapper_patch
                 ~definition:(Some symbol_definition)
                 ~ctx
                 new_name
             else
               None
           | Class _
           | Member _
           | ExplicitClass _
           | LocalVar _
           | GConst _ ->
             None
         in
         Option.value_map
           deprecated_wrapper_patch
           ~default:changes
           ~f:(fun patch -> patch :: changes))
  |> fun rename_patches -> Ok rename_patches
