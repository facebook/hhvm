(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open ServerEnv
open ServerRefactorTypes

let maybe_add_dollar s =
  if s.[0] != '$' then "$" ^ s
  else s

let get_fixme_patches codes (env: env) =
  let fixmelist = Errors.get_applied_fixmes env.errorl in
  let poslist = Fixmes.get_unused_fixmes codes fixmelist env.files_info in
  List.map ~f:(fun pos -> Remove (Pos.to_absolute pos)) poslist

let find_def_filename current_filename definition =
  let open SymbolDefinition in
  if Pos.filename definition.pos = ServerIdeUtils.path then
    (* When the definition is in an IDE buffer with local changes, the filename
       in the definition will be empty. *)
    current_filename
  else Pos.filename definition.pos

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
~(func_decl_text: string)
~(params_text_list: string list)
~(col_start: int)
~(returns_void: bool)
~(is_async: bool)
~(func_ref: deprecated_wrapper_function_ref)
 (new_name: string)
: string =
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
  let parameter_input = String.concat ", " params_text_list in
  let maybe_return =
    if returns_void
    then ""
    else "return "
  in
  let maybe_await =
    if is_async
    then "await "
    else ""
  in
  let maybe_this_or_self = match func_ref with
    | DeprecatedStaticMethodRef -> "self::"
    | DeprecatedNonStaticMethodRef -> "$this->"
    | DeprecatedFunctionRef -> ""
  in
  let return_statement =
    return_indentation ^
    maybe_return ^ maybe_await ^ maybe_this_or_self ^
    new_name ^ "(" ^ parameter_input ^ ");"
  in
  "\n" ^ deprecated_header ^
  "\n" ^ func_decl ^ " {" ^
  "\n" ^ return_statement ^
  "\n" ^ base_indentation ^ "}" ^
  "\n"

let get_pos_before_docblock_from_cst_node filename node =
  let open Full_fidelity_positioned_syntax in
  let source_text = source_text node in
  let start_offset = leading_start_offset node in
  SourceText.relative_pos filename source_text start_offset start_offset

let get_deprecated_wrapper_patch ~filename ~definition new_name =
  let open SymbolDefinition in
  let open Full_fidelity_positioned_syntax in
  let open Option.Monad_infix in
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
  let _, col_start_plus1, _, _ = Pos.destruct_range definition.span in
  let col_start = col_start_plus1 - 1 in
  let filename_server_type = ServerCommandTypes.FileName filename in
  let cst_node =
    ServerSymbolDefinition.get_definition_cst_node filename_server_type definition in
  cst_node >>= fun cst_node ->
  let get_params_modifier_info func_decl = match syntax func_decl with
    | FunctionDeclarationHeader {
        function_parameter_list = params;
        function_type = ret_type;
        function_modifiers = modifiers; _
      } ->
        let params_text_list = match syntax params with
          | SyntaxList params ->
            let params_text_list = List.map params ~f:begin fun param ->
              let param = match syntax param with
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
                | VariadicParameter _ ->
                  let num_of_nonvariadic_params = string_of_int ((List.length params) - 1) in
                  "...Vec\\drop(func_get_args(), " ^ num_of_nonvariadic_params ^ ")"
                | _ -> failwith "Expected some parameter type"
            end in
            Some params_text_list
          | Missing -> Some []
          | _ -> None
        in
        let returns_void = match syntax ret_type with
          | GenericTypeSpecifier {
              generic_class_type = generic_type;
              generic_argument_list = { syntax =
                TypeArguments {
                  type_arguments_types = { syntax =
                    SyntaxList [{ syntax =
                      ListItem {
                        list_item = { syntax =
                          SimpleTypeSpecifier {
                            simple_type_specifier = type_spec
                          }; _
                        }; _
                      }; _
                    }]; _
                  }; _
                }; _
              }; _
            } -> (text generic_type) = "Awaitable" && (text type_spec) = "void"
          | SimpleTypeSpecifier { simple_type_specifier = type_spec } ->
            (text type_spec) = "void"
          | _ -> false
        in
        let is_async, is_static = match syntax modifiers with
          | SyntaxList modifiers ->
            let is_async =
              List.exists modifiers ~f:(fun modifier -> (text modifier) = "async")
            in
            let is_static =
              List.exists modifiers ~f:(fun modifier -> (text modifier) = "static")
            in
            is_async, is_static
          | _ -> false, false
        in
        params_text_list, returns_void, is_async, is_static
    | _ -> None, false, false, false
  in
  begin match syntax cst_node with
  | MethodishDeclaration { methodish_function_decl_header = func_decl; _ } ->
      let func_decl_text = text func_decl in
      let params_text_list, returns_void, is_async, is_static =
        get_params_modifier_info func_decl
      in
      let func_ref =
        if is_static
        then DeprecatedStaticMethodRef
        else DeprecatedNonStaticMethodRef
      in
      params_text_list >>= fun params_text_list ->
      Some (func_decl_text, params_text_list, returns_void, is_async, func_ref)
  | FunctionDeclaration { function_declaration_header = func_decl; _ } ->
      let func_decl_text = text func_decl in
      let params_text_list, returns_void, is_async, _ =
        get_params_modifier_info func_decl
      in
      let func_ref = DeprecatedFunctionRef in
      params_text_list >>= fun params_text_list ->
      Some (func_decl_text, params_text_list, returns_void, is_async, func_ref)
  | _ -> None
  end >>| fun (func_decl_text, params_text_list, returns_void, is_async, func_ref) ->
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
    find_def_filename (Relative_path.create_detect_prefix filename) definition
  in
  let deprecated_wrapper_pos = get_pos_before_docblock_from_cst_node filename cst_node in
  let patch = {
    pos = Pos.to_absolute deprecated_wrapper_pos;
    text = deprecated_wrapper_stub;
  } in
  Insert patch

let go action genv env =
  let module Types = ServerCommandTypes.Find_refs in
  let find_refs_action, new_name = match action with
    | ClassRename (old_name, new_name) ->
        Types.Class old_name, new_name
    | ClassConstRename (class_name, old_name, new_name) ->
        Types.Member (class_name, Types.Class_const old_name),
          new_name
    | MethodRename { class_name; old_name; new_name; _ } ->
        Types.Member (class_name, Types.Method old_name),
          new_name
    | FunctionRename { old_name; new_name; _ } ->
        Types.Function old_name, new_name
    | LocalVarRename { filename; file_content; line; char; new_name } ->
        Types.LocalVar { filename; file_content; line; char }, new_name in
  let include_defs = true in
  let refs = ServerFindRefs.get_refs find_refs_action include_defs genv env in
  let changes = List.fold_left refs ~f:begin fun acc x ->
    let replacement = {
      pos  = Pos.to_absolute (snd x);
      text = new_name;
    } in
    let patch = Replace replacement in
    patch :: acc
  end ~init:[] in
  let deprecated_wrapper_patch = match action with
    | FunctionRename { filename; definition; _ }
    | MethodRename { filename; definition; _ } ->
      get_deprecated_wrapper_patch ~filename ~definition new_name
    | ClassRename _
    | ClassConstRename _
    | LocalVarRename _ -> None
  in
  Option.value_map deprecated_wrapper_patch ~default:changes
    ~f:begin fun patch -> patch :: changes end

let go_ide (filename, line, char) new_name genv env =
  let open SymbolDefinition in
  let file_content = ServerFileSync.get_file_content (ServerCommandTypes.FileName filename) in
  let definitions = ServerIdentifyFunction.go_absolute file_content line char env.tcopt in
  match definitions with
  | (_, Some definition) :: [] -> begin
    let {full_name; kind; _} = definition in
    let pieces = Str.split (Str.regexp "::") full_name in
    match kind, pieces with
    | Function, [function_name] ->
      let command =
        ServerRefactorTypes.FunctionRename {
          filename = Some filename;
          definition = Some definition;
          old_name = function_name;
          new_name;
        } in
      go command genv env
    | Enum, [enum_name] ->
      let command =
        ServerRefactorTypes.ClassRename (enum_name, new_name) in
      go command genv env
    | Class, [class_name] ->
      let command =
        ServerRefactorTypes.ClassRename (class_name, new_name) in
      go command genv env
    | Const, [class_name; const_name] ->
      let command =
        ServerRefactorTypes.ClassConstRename (class_name, const_name, new_name) in
      go command genv env
    | Method, [class_name; method_name] ->
      let command =
        ServerRefactorTypes.MethodRename {
          filename = Some filename;
          definition = Some definition;
          class_name;
          old_name = method_name;
          new_name;
        } in
      go command genv env
    | LocalVar, _ ->
      let command =
        ServerRefactorTypes.LocalVarRename {
          filename = Relative_path.create_detect_prefix filename;
          file_content;
          line;
          char;
          new_name = maybe_add_dollar new_name;
        } in
      go command genv env
    | _, _ -> [] end
  | _ -> [] (* We have 0 or >1 definitions so correct behavior is unknown *)
