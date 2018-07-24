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
let construct_deprec_wrapper_text ~func_decl_text ~params_text_list ~col_start new_name =
  (* Since the starting column position points to the beginning of the function
      declaration header, we can use it to figure out the indentation level
      of the function, and insert whitespace accordingly *)
  let base_indentation = String.make col_start ' ' in
  let deprecated_header =
    base_indentation ^ "<<__Deprecated(\"Deprecated: Use `" ^ new_name ^ "` instead\")>>"
  in
  let func_decl = base_indentation ^ func_decl_text in
  (* The immediate body of a function is indented by 2 extra spaces *)
  let func_body_indentation = String.make 2 ' ' in
  let return_indentation = base_indentation ^ func_body_indentation in
  let parameter_input = String.concat ", " params_text_list in
  let return_statement =
    return_indentation ^ "return $this->" ^ new_name ^ "(" ^ parameter_input ^ ");"
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

let get_deprec_wrapper_patch ~filename ~definition new_name =
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
  begin match cst_node with
    | {
        syntax =
          MethodishDeclaration {
            methodish_function_decl_header = func_decl; _
          }; _
      } ->
      let func_decl_text = text func_decl in
      let params_text_list = match syntax func_decl with
        | FunctionDeclarationHeader {
            function_parameter_list = params; _
          } ->
            begin match syntax params with
            | SyntaxList params ->
              let params_text_list = List.map params ~f:begin fun param ->
                let param = match syntax param with
                  | ListItem { list_item; _ } -> list_item
                  | _ -> failwith "Expected ListItem"
                in
                match syntax param with
                  | ParameterDeclaration { parameter_name = name; _ } -> text name
                  | VariadicParameter { variadic_parameter_call_convention = callconv; _ } ->
                    (* TODO: Will add support here in next revision *)
                    (* Some (text callconv) *)
                    failwith (Printf.sprintf "VariadicParam: %s" (text callconv))
                  | _ -> failwith "Expected some parameter type"
              end in
              Some params_text_list
            | _ -> None
            end
        | _ -> None
      in
      params_text_list >>= fun params_text_list ->
      Some (func_decl_text, params_text_list)
    | _ -> None
  end >>| fun (func_decl_text, params_text_list) ->
  let deprec_wrapper_text =
    construct_deprec_wrapper_text ~func_decl_text ~params_text_list ~col_start new_name
  in
  let filename =
    find_def_filename (Relative_path.create_detect_prefix filename) definition
  in
  let deprec_pos = get_pos_before_docblock_from_cst_node filename cst_node in
  let patch = {
    pos = Pos.to_absolute deprec_pos;
    text = deprec_wrapper_text;
  } in
  Insert patch

let go action genv env =
  let module Types = ServerCommandTypes.Find_refs in
  let find_refs_action, new_name, deprec_wrapper_patch = match action with
    | ClassRename (old_name, new_name) ->
        Types.Class old_name, new_name, None
    | ClassConstRename (class_name, old_name, new_name) ->
        Types.Member (class_name, Types.Class_const old_name),
          new_name, None
    | MethodRename { filename; definition; class_name; old_name; new_name } ->
        Types.Member (class_name, Types.Method old_name),
          new_name,
          get_deprec_wrapper_patch ~filename ~definition new_name
    | FunctionRename (old_name, new_name) ->
        Types.Function old_name, new_name, None
    | LocalVarRename { filename; file_content; line; char; new_name } ->
        Types.LocalVar { filename; file_content; line; char }, new_name, None in
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
  Option.value_map deprec_wrapper_patch ~default:changes
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
        ServerRefactorTypes.FunctionRename (function_name, new_name) in
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
