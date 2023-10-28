(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
module T = Extract_method_types
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)

let tree_from_string s =
  let source_text = Full_fidelity_source_text.make Relative_path.default s in
  let env = Full_fidelity_parser_env.make ~mode:FileInfo.Mstrict () in
  let tree = SyntaxTree.make ~env source_text in

  if List.is_empty (SyntaxTree.all_errors tree) then
    Some tree
  else
    None

let hackfmt src =
  let prefix = "<?hh\n" in
  match tree_from_string (prefix ^ src) with
  | Some tree ->
    tree |> Libhackfmt.format_tree |> String.chop_prefix_if_exists ~prefix
  | None -> src

let indent ~(indent_amount : int) (s : string) : string =
  let indentation = String.make indent_amount ' ' in
  s
  |> String.split_lines
  |> List.map ~f:(fun line -> indentation ^ line)
  |> String.concat ~sep:"\n"

let return_type_string_of_candidate
    ~(return : (string * Code_action_types.Type_string.t) list)
    T.{ selection_kind; is_async; iterator_kind; _ } =
  let wrap_return_type =
    match (is_async, iterator_kind) with
    | (true, Some T.Iterator) -> Fn.const "AsyncIterator<_, _, _>"
    | (true, Some T.KeyedIterator) -> Fn.const "AsyncKeyedIterator<_, _, _>"
    | (false, Some T.Iterator) -> Fn.const "Iterator<_>"
    | (false, Some T.KeyedIterator) -> Fn.const "KeyedIterator<_>"
    | (true, None) -> Format.sprintf "Awaitable<%s>"
    | (false, None) -> Fn.id
  in
  match selection_kind with
  | T.SelectionKindExpression type_string ->
    wrap_return_type (Code_action_types.Type_string.to_string type_string)
  | T.SelectionKindStatement ->
    wrap_return_type
    @@
    (match return with
    | [] -> "void"
    | [(_, type_string)] -> Code_action_types.Type_string.to_string type_string
    | _ ->
      return
      |> List.map ~f:(fun (_, type_string) ->
             Code_action_types.Type_string.to_string type_string)
      |> String.concat ~sep:", "
      |> Format.sprintf "(%s)")

let body_string_of_candidate
    ~source_text
    ~(return : (string * Code_action_types.Type_string.t) list)
    T.{ selection_kind; pos; iterator_kind; method_pos; _ } =
  let raw_body_string =
    let (first_line, first_col) = Pos.line_column pos in
    let exp_offset =
      Full_fidelity_source_text.position_to_offset
        source_text
        (first_line, first_col + 1)
    in
    Full_fidelity_source_text.sub source_text exp_offset (Pos.length pos)
  in
  match selection_kind with
  | T.SelectionKindExpression _ -> Format.sprintf "return %s;" raw_body_string
  | T.SelectionKindStatement ->
    if Option.is_some iterator_kind then
      raw_body_string
    else
      let method_indent_amount = snd @@ Pos.line_column method_pos in
      let format_as_return : string -> string =
        let whitespace = String.make (2 * method_indent_amount) ' ' in
        Format.sprintf "\n%sreturn %s;" whitespace
      in
      let return_string =
        match return with
        | [] -> ""
        | [(var_name, _)] -> format_as_return var_name
        | _ ->
          return
          |> List.map ~f:fst
          |> String.concat ~sep:", "
          |> Format.sprintf "tuple(%s)"
          |> format_as_return
      in
      raw_body_string ^ return_string

let method_string_of_candidate
    ~source_text
    ~(params : (string * Code_action_types.Type_string.t) list)
    ~(return : (string * Code_action_types.Type_string.t) list)
    ~(snippet : string)
    (T.{ method_is_static; is_async; method_pos; _ } as candidate) =
  let return_type_string = return_type_string_of_candidate ~return candidate in
  let body_string = body_string_of_candidate ~source_text ~return candidate in
  let add_modifiers : string -> string =
    let static_string =
      if method_is_static then
        (*
      The extracted function is static iff the function we are extracting from is static.
      We could "Principle of Least Privilege" and default to `static` if `this` isn't used *but*
      that would make things harder to mock and some people like mocking.
      *)
        "static "
      else
        ""
    in
    let function_kind_string =
      if is_async then
        "async "
      else
        ""
    in
    Format.sprintf "private %s%s%s" static_string function_kind_string
  in
  let params_string =
    params
    |> List.map ~f:(fun (name, type_string) ->
           Format.sprintf
             "%s %s"
             (Code_action_types.Type_string.to_string type_string)
             name)
    |> String.concat ~sep:", "
  in
  (* we format as a function before adding modifiers, since a function is hackfmt-able (a valid top-level form) *)
  let placeholder_to_replace_with_snippet = "the_function_name" in
  let raw_function_string =
    Format.sprintf
      "function %s(%s): %s {\n%s\n}"
      placeholder_to_replace_with_snippet
      params_string
      return_type_string
      body_string
  in
  let indent_amount = snd @@ Pos.line_column method_pos in
  let add_suffix s = s ^ "\n\n" in
  raw_function_string
  |> hackfmt
  |> String.substr_replace_first
       ~pattern:placeholder_to_replace_with_snippet
       ~with_:snippet
  |> add_modifiers
  |> indent ~indent_amount
  |> add_suffix

let method_call_string_of_candidate
    ~(params : (string * Code_action_types.Type_string.t) list)
    ~(return : (string * Code_action_types.Type_string.t) list)
    ~(snippet : string)
    T.
      {
        method_is_static;
        selection_kind;
        is_async;
        iterator_kind;
        pos;
        method_pos;
        _;
      } =
  let args_string = params |> List.map ~f:fst |> String.concat ~sep:", " in
  let receiver_string =
    if method_is_static then
      "self::"
    else
      "$this->"
  in
  let call_expr =
    Format.sprintf "%s%s(%s)" receiver_string snippet args_string
  in
  match iterator_kind with
  | None ->
    (* examples:
       - `foo($arg1)`
       - `await foo($arg1, $arg2)`
    *)
    let call_expr =
      if is_async then
        Format.sprintf "await %s" call_expr
      else
        call_expr
    in
    (match selection_kind with
    | T.SelectionKindExpression _ -> call_expr
    | T.SelectionKindStatement ->
      let fmt_assignment lhs_string =
        Format.sprintf "%s = %s;" lhs_string call_expr
      in
      (match return with
      | [] -> call_expr ^ ";"
      | [(var_name, _)] -> fmt_assignment var_name
      | _ ->
        return
        |> List.map ~f:fst
        |> String.concat ~sep:", "
        |> Format.sprintf "list(%s)"
        |> fmt_assignment))
  | Some iterator_kind ->
    (* example:
       foreach(self::foo() as $value__) {

       }
    *)
    let await_string =
      if is_async then
        "await "
      else
        ""
    in
    let as_string =
      match iterator_kind with
      | T.Iterator -> "$value__"
      | T.KeyedIterator -> "$key__ => $value__"
    in
    let comment_and_whitespace =
      (* generate comments like: "/* TODO: assign to $x, $y */"
         TODO(T152359779): do more work for the user to handle assignments
      *)
      let indent_amount = snd @@ Pos.line_column method_pos in
      let call_site_indent_amount = snd @@ Pos.line_column pos in
      let outer_indent = String.make call_site_indent_amount ' ' in
      let inner_indent =
        String.make (call_site_indent_amount + indent_amount) ' '
      in
      let of_var_name_string var_names_string =
        Format.sprintf
          "\n%s/* TODO: assign to %s */\n%s\n%s"
          inner_indent
          var_names_string
          inner_indent
          outer_indent
      in
      match return with
      | [] -> Format.sprintf "\n%s\n%s" inner_indent outer_indent
      | [(var_name, _)] -> of_var_name_string var_name
      | _ ->
        return
        |> List.map ~f:fst
        |> String.concat ~sep:", "
        |> of_var_name_string
    in
    Format.sprintf
      "foreach (%s %sas %s) {%s}"
      call_expr
      await_string
      as_string
      comment_and_whitespace

let edit_of_candidate
    ~source_text
    ~path
    (T.{ method_pos; params; return; pos; placeholder_name; _ } as candidate) :
    Lsp.WorkspaceEdit.t =
  let type_assoc_list_of map =
    map
    |> String.Map.to_alist ~key_order:`Increasing
    |> List.dedup_and_sort ~compare:(fun (s1, _) (s2, _) ->
           String.compare s1 s2)
  in
  let params = type_assoc_list_of params in
  let return = type_assoc_list_of return in
  let snippet = Format.sprintf "${0:%s}" placeholder_name in
  let change_add_call =
    let call_string =
      method_call_string_of_candidate ~params ~return ~snippet candidate
    in
    {
      Lsp.TextEdit.range =
        Lsp_helpers.hack_pos_to_lsp_range ~equal:Relative_path.equal pos;
      newText = call_string;
    }
  in
  let change_add_method =
    let line = (fst @@ Pos.line_column method_pos) - 1 in
    let character = 0 in
    let method_string =
      method_string_of_candidate ~source_text ~params ~return ~snippet candidate
    in
    Lsp.
      {
        Lsp.TextEdit.range =
          { start = { line; character }; end_ = { line; character } };
        newText = method_string;
      }
  in
  let changes =
    Lsp.DocumentUri.Map.singleton
      (Lsp_helpers.path_to_lsp_uri path)
      [change_add_method; change_add_call]
  in
  Lsp.WorkspaceEdit.{ changes }

let of_candidate ~source_text ~path candidate =
  let edit = lazy (edit_of_candidate ~source_text ~path candidate) in
  Code_action_types.Refactor.{ title = "Extract into method"; edit }
