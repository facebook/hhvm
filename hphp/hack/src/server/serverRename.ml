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
open ServerCommandTypes.Done_or_retry

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

let get_type_params_type_rewrite_patches ctx files =
  List.concat_map files ~f:(fun file ->
      ServerRewriteTypeParamsType.get_patches
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

(* This function will capture a variadic parameter and give it a name if it is
 * anonymous.  Example:
 *
 * public static function newName(int $x, ...): string {
 *
 * would become:
 *
 * public static function newName(int $x, mixed ...$args): string {
 *
 *)
let fixup_anonymous_variadic
    (func_decl : Full_fidelity_positioned_syntax.t)
    (has_anonymous_variadic : bool) : string =
  Full_fidelity_positioned_syntax.(
    if has_anonymous_variadic then
      let r = Str.regexp "\\.\\.\\." in
      Str.global_replace r "mixed ...$args" (text func_decl)
    else
      text func_decl)

(* Contains just enough information to properly wrap a function *)
type wrapper_call_signature_info = {
  params_text_list: string list;
  returns_void: bool;
  is_async: bool;
  is_static: bool;
  has_anonymous_variadic: bool;
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
                     calling `text parameter_name` will return the entire "...$nums"
                     string, including the ellipsis.

                   `VariadicParameter` addresses the unnamed variadic parameter
                     "...". In this case, we provide as a parameter a function call
                     that outputs only the variadic params (and dropping the
                     non-variadic ones).
                *)
                | ParameterDeclaration { parameter_name = name; _ } -> text name
                | VariadicParameter _ -> "...$args"
                | _ -> failwith "Expected some parameter type")
          in
          params_text_list
        | Missing -> []
        | _ -> []
      in
      let has_anonymous_variadic =
        match syntax params with
        | SyntaxList params ->
          List.exists params ~f:(fun param ->
              let param =
                match syntax param with
                | ListItem { list_item; _ } -> list_item
                | _ -> failwith "Expected ListItem"
              in
              match syntax param with
              | VariadicParameter _ -> true
              | _ -> false)
        | Missing -> false
        | _ -> false
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
      {
        params_text_list;
        returns_void;
        is_async;
        is_static;
        has_anonymous_variadic;
      }
    | _ ->
      {
        params_text_list = [];
        returns_void = false;
        is_async = false;
        is_static = false;
        has_anonymous_variadic = false;
      })

let classish_is_interface (ctx : Provider_context.t) (name : string) : bool =
  match Decl_provider.get_class ctx (Utils.add_ns name) with
  | None -> false
  | Some cls ->
    (match Decl_provider.Class.kind cls with
    | Ast_defs.Cinterface -> true
    | _ -> false)

(* Produce a "deprecated" version of the old function so that calls to it can be rerouted *)
let get_deprecated_wrapper_patch
    ~(filename : string option)
    ~(definition : string SymbolDefinition.t option)
    ~(ctx : Provider_context.t)
    (new_name : string) : patch option =
  SymbolDefinition.(
    Full_fidelity_positioned_syntax.(
      Option.Monad_infix.(
        filename >>= fun filename ->
        definition >>= fun definition ->
        let definition = SymbolDefinition.to_relative definition in

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
          Provider_context.add_entry_if_missing
            ~ctx
            ~path:(Relative_path.create_detect_prefix filename)
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
            let func_decl_text =
              fixup_anonymous_variadic
                func_decl
                call_signature.has_anonymous_variadic
            in
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
            let func_decl_text =
              fixup_anonymous_variadic
                func_decl
                call_signature.has_anonymous_variadic
            in
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
        let filename =
          find_def_filename
            (Relative_path.create_detect_prefix filename)
            definition
        in
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

let upcast_visitor =
  object (self)
    inherit [_] Tast_visitor.reduce as super

    val mutable upcasted_id = ""

    method zero = []

    method plus = ( @ )

    method set_upcasted_fid fid = upcasted_id <- fid

    method private check_id id pos =
      if String.equal id upcasted_id then
        [pos]
      else
        self#zero

    method! on_expr env expr =
      let acc =
        let (_ty, pos, expr_) = expr in
        match expr_ with
        | Aast.Upcast (e, _hint) -> begin
          match e with
          | (_, _, Aast.FunctionPointer (Aast.FP_id (_, id), _)) ->
            self#check_id id pos
          | (_, _, Aast.New ((_, _, Aast.CI (_, id)), _, _, _, _)) ->
            self#check_id id pos
          | (_, _, Aast.Class_const ((_, _, Aast.CI (_, id)), _)) ->
            self#check_id id pos
          | _ -> self#zero
        end
        | _ -> self#zero
      in
      self#plus acc (super#on_expr env expr)
  end

let method_might_support_dynamic ctx ~class_name ~method_name =
  let open Option.Monad_infix in
  let module Class = Decl_provider.Class in
  let sd_enabled =
    TypecheckerOptions.enable_sound_dynamic @@ Provider_context.get_tcopt ctx
  in
  (not sd_enabled)
  || Decl_provider.get_class ctx @@ Utils.add_ns class_name
     >>= (fun class_ ->
           Option.first_some
             (Class.get_smethod class_ method_name)
             (Class.get_method class_ method_name))
     >>| (fun elt ->
           let flags = elt.Typing_defs.ce_flags in
           Typing_defs_flags.ClassElt.(
             is_dynamicallycallable flags || supports_dynamic_type flags))
     |> Option.value ~default:true

let get_upcast_locations ctx files element_name =
  let element_name = ServerFindRefs.add_ns element_name in
  (* [files] can legitimately refer to non-existent files, e.g.
     if they've been deleted since the depgraph was created.
     This is how we'll filter them out. *)
  let is_entry_valid entry =
    entry |> Provider_context.get_file_contents_if_present |> Option.is_some
  in
  (* These are the tasts for all the 'fileinfo_l' passed in *)
  let tasts_of_files : (Relative_path.t * Tast.program) list =
    List.filter_map files ~f:(fun path ->
        let (_ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
        try
          let { Tast_provider.Compute_tast.tast; _ } =
            Tast_provider.compute_tast_unquarantined ~ctx ~entry
          in
          Some (path, tast)
        with
        | _ when not (is_entry_valid entry) -> None)
  in
  let () = upcast_visitor#set_upcasted_fid element_name in
  let upcast_positions =
    List.rev_map tasts_of_files ~f:(fun (_, tast) -> upcast_visitor#go ctx tast)
    |> List.concat
  in
  let buf = Buffer.create (31 * List.length upcast_positions) in
  let refactor_check_sd_string = "Refactor-check-sound-dynamic: " in
  let len_string =
    Printf.sprintf
      "Number of upcast positions for %s is %d\n"
      element_name
      (List.length upcast_positions)
  in
  Hh_logger.log "%s%s" refactor_check_sd_string len_string;
  Buffer.add_string buf len_string;
  let () =
    List.iter
      ~f:(fun pos ->
        let pos_string =
          Printf.sprintf "%s\n" (Pos.string (Pos.to_absolute pos))
        in
        Hh_logger.log "%sposition = %s" refactor_check_sd_string pos_string;
        Buffer.add_string buf pos_string)
      upcast_positions
  in
  Buffer.contents buf

let go_sound_dynamic ctx action genv env =
  if not (TypecheckerOptions.enable_sound_dynamic env.tcopt) then
    ( env,
      Done
        "Server is NOT using sound dynamic. Change the .hhconfig file to enable sound dynamic. \n"
    )
  else
    let is_sound_dynamic_str = "Server is using sound dynamic. \n" in
    match action with
    | ServerRenameTypes.FunctionRename { old_name; _ } ->
      let function_name = ServerFindRefs.add_ns old_name in
      ServerFindRefs.handle_prechecked_files
        genv
        env
        Typing_deps.(Dep.(make (Fun function_name)))
      @@ fun () ->
      let files =
        FindRefsService.get_dependent_files_function
          ctx
          genv.ServerEnv.workers
          function_name
        |> Relative_path.Set.elements
      in
      is_sound_dynamic_str ^ get_upcast_locations ctx files old_name
    | ServerRenameTypes.ClassRename (class_name, _) ->
      let class_name = ServerFindRefs.add_ns class_name in
      ServerFindRefs.handle_prechecked_files
        genv
        env
        Typing_deps.(Dep.(make (Type class_name)))
      @@ fun () ->
      let files =
        FindRefsService.get_dependent_files
          ctx
          genv.ServerEnv.workers
          (SSet.singleton class_name)
        |> Relative_path.Set.elements
      in
      is_sound_dynamic_str ^ get_upcast_locations ctx files class_name
    | _ -> (env, Done "")

let go ctx action genv env =
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
  ServerFindRefs.go ctx find_refs_action include_defs genv env
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
           match action with
           | FunctionRename { filename; definition; _ } ->
             get_deprecated_wrapper_patch ~filename ~definition ~ctx new_name
           | MethodRename { filename; definition; class_name; old_name; _ } ->
             if
               method_might_support_dynamic
                 ctx
                 ~class_name
                 ~method_name:old_name
             then
               get_deprecated_wrapper_patch ~filename ~definition ~ctx new_name
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
                { pos = Pos.to_absolute (snd x); text = new_name }
              in
              let patch = Replace replacement in
              patch :: acc)
            ~init:[]
      >>| fun patches -> Some patches
    in
    changes
  | _ -> Error action

(**
  Like go_ide, but rather than looking up a symbolDefinition manually from a file and
  converting a ServerRenameTypes.action to a Find_refs.action, we supply a Find_refs.action
  directly.
*)
let go_ide_with_find_refs_action
    ctx ~find_refs_action ~new_name ~filename ~symbol_definition genv env =
  let include_defs = true in
  ServerFindRefs.go ctx find_refs_action include_defs genv env
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
               ~filename:(Some filename)
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
                 ~filename:(Some filename)
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

let go_ide ctx (filename, line, column) new_name genv env =
  let open SymbolDefinition in
  let (ctx, entry) =
    Provider_context.add_entry_if_missing
      ~ctx
      ~path:(Relative_path.create_detect_prefix filename)
  in
  let file_content = Provider_context.read_file_contents_exn entry in
  let definitions =
    ServerIdentifyFunction.go_quarantined_absolute ~ctx ~entry ~line ~column
  in
  match definitions with
  | [(_, Some definition)] ->
    let { full_name; kind; _ } = definition in
    let pieces = Str.split (Str.regexp "::") full_name in
    (match (kind, pieces) with
    | (Function, [function_name]) ->
      let command =
        ServerRenameTypes.FunctionRename
          {
            filename = Some filename;
            definition = Some definition;
            old_name = function_name;
            new_name;
          }
      in
      Ok (go ctx command genv env)
    | (Enum, [enum_name]) ->
      let command = ServerRenameTypes.ClassRename (enum_name, new_name) in
      Ok (go ctx command genv env)
    | (Class, [class_name]) ->
      let command = ServerRenameTypes.ClassRename (class_name, new_name) in
      Ok (go ctx command genv env)
    | (Typedef, [class_name]) ->
      let command = ServerRenameTypes.ClassRename (class_name, new_name) in
      Ok (go ctx command genv env)
    | (ClassConst, [class_name; const_name]) ->
      let command =
        ServerRenameTypes.ClassConstRename (class_name, const_name, new_name)
      in
      Ok (go ctx command genv env)
    | (Method, [class_name; method_name]) ->
      let command =
        ServerRenameTypes.MethodRename
          {
            filename = Some filename;
            definition = Some definition;
            class_name;
            old_name = method_name;
            new_name;
          }
      in
      Ok (go ctx command genv env)
    | (LocalVar, _) ->
      let command =
        ServerRenameTypes.LocalVarRename
          {
            filename = Relative_path.create_detect_prefix filename;
            file_content;
            line;
            char = column;
            new_name = maybe_add_dollar new_name;
          }
      in
      Ok (go ctx command genv env)
    | (_, _) -> Error "Tried to rename a non-renameable symbol")
  (* We have 0 or >1 definitions so correct behavior is unknown *)
  | _ -> Error "Tried to rename a non-renameable symbol"
