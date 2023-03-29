(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let placeholder_base = "$placeholder"

let placeholder_regexp = Str.regexp {|\$placeholder\([0-9]+\)|}

type candidate = {
  stmt_pos: Pos.t;
  pos: Pos.t;
  placeholder_n: int;
}

let lsp_range_contains_pos (range : Lsp.range) pos =
  let (start_line, start_character, end_line, end_character) =
    Pos.destruct_range pos
  in
  Lsp.(
    let start_contains =
      range.start.line < start_line
      || range.start.line = start_line
         && range.start.character <= start_character
    in
    let end_contains =
      range.end_.line > end_line
      || (range.end_.line = end_line && range.end_.character >= end_character)
    in
    start_contains && end_contains)

let lsp_range_of_pos (pos : Pos.t) : Lsp.range =
  let (first_line, first_col) = Pos.line_column pos in
  let (last_line, last_col) = Pos.end_line_column pos in
  {
    Lsp.start = { Lsp.line = first_line - 1; character = first_col };
    end_ = { Lsp.line = last_line - 1; character = last_col };
  }

let source_slice ~source_text ~start_pos ~length =
  let offset =
    let (first_line, first_col) = Pos.line_column start_pos in
    Full_fidelity_source_text.position_to_offset
      source_text
      (first_line, first_col + 1)
  in
  Full_fidelity_source_text.sub source_text offset length

(**
We don't want to extract variables for lambdas like this: `() ==> 200`.
The AST of such a lambda is indistinguishable from `() ==> { return 200; }`
so we peek at the source *)
let might_be_expression_lambda ~f_body:Aast.{ fb_ast } ~pos ~source_text =
  match fb_ast with
  | [(stmt_pos, _)] ->
    let length = Pos.start_offset stmt_pos - Pos.start_offset pos in
    if length > 0 then
      let src = source_slice ~source_text ~start_pos:pos ~length in
      not @@ String.is_substring ~substring:"{" src
    else
      (*  length can be negative to curlies in default params: `(($a = () ==> {}) ==> ...` *)
      true
  | _ -> false

let positions_visitor (range : Lsp.range) ~source_text =
  let stmt_pos = ref Pos.none in
  let expression_lambda_pos = ref None in
  let placeholder_n = ref 0 in
  let reset () =
    expression_lambda_pos := None;
    placeholder_n := 0
  in

  object
    inherit [candidate option] Tast_visitor.reduce as super

    method zero = None

    method plus = Option.first_some

    method! on_method_ env meth =
      reset ();
      super#on_method_ env meth

    method! on_fun_def env fd =
      reset ();
      super#on_fun_def env fd

    method! on_lid env lid =
      let name = Local_id.get_name @@ snd lid in
      if Str.string_match placeholder_regexp name 0 then
        Str.matched_group 1 name
        |> int_of_string_opt
        |> Option.iter ~f:(fun n -> placeholder_n := max (n + 1) !placeholder_n);
      super#on_lid env lid

    method! on_func_body env fb =
      let acc = super#on_func_body env fb in
      (match List.hd fb.Aast.fb_ast with
      | Some (pos, _) -> stmt_pos := pos
      | _ -> ());
      match acc with
      | Some acc -> Some { acc with placeholder_n = !placeholder_n }
      | None -> None

    method! on_stmt env stmt =
      stmt_pos := fst stmt;
      super#on_stmt env stmt

    method! on_expr env expr =
      let (_, pos, expr_) = expr in
      match expr_ with
      | Aast.(Binop { bop = Ast_defs.Eq _; lhs = (_, lhs_pos, _); rhs = _ }) ->
        let acc = super#on_expr env expr in
        Option.filter acc ~f:(fun candidate ->
            not @@ Pos.contains lhs_pos candidate.pos)
      | Aast.Lfun (Aast.{ f_body; _ }, _) ->
        expression_lambda_pos :=
          Option.some_if
            (might_be_expression_lambda ~f_body ~pos ~source_text)
            pos;
        super#on_expr env expr
      | Aast.Efun _ ->
        expression_lambda_pos := None;
        super#on_expr env expr
      | _ ->
        if
          lsp_range_contains_pos range pos
          && (not @@ Pos.equal !stmt_pos Pos.none)
          && not
               (Option.map !expression_lambda_pos ~f:(fun lpos ->
                    Pos.contains lpos pos)
               |> Option.value ~default:false)
        then
          Some
            {
              stmt_pos = !stmt_pos;
              pos;
              placeholder_n = 0 (* will be adjusted on the way up *);
            }
        else
          super#on_expr env expr
  end

(** ensures that `positions_visitor` only traverses
functions and methods such that
the function body contains the selected range *)
let top_visitor ctx (range : Lsp.range) ~source_text =
  let should_traverse span =
    let outer = Lsp_helpers.pos_to_lsp_range span in
    Lsp_helpers.lsp_range_contains ~outer range
  in
  object
    inherit [candidate option] Tast_visitor.reduce as super

    method zero = None

    method plus = Option.first_some

    method! on_class_ env class_ =
      let acc = super#on_class_ env class_ in
      (* strip vars and consts because it doesn't make sense to "Extract Variable" there *)
      let class_ = Aast.{ class_ with c_vars = []; c_consts = [] } in
      class_.Aast.c_methods
      |> List.fold ~init:acc ~f:(fun acc meth ->
             let class_ = Aast.{ class_ with c_methods = [meth] } in
             let span = meth.Aast.m_span in
             if Option.is_none acc && should_traverse span then
               (positions_visitor range ~source_text)#go ctx [Aast.Class class_]
             else
               acc)

    method! on_fun_def env fun_def =
      let acc = super#on_fun_def env fun_def in
      let span = Aast.(fun_def.fd_fun.f_span) in
      if Option.is_none acc && should_traverse span then
        (positions_visitor range ~source_text)#go ctx [Aast.Fun fun_def]
      else
        acc
  end

let command_or_action_of_candidate
    ~source_text ~path { stmt_pos; pos; placeholder_n } =
  let placeholder = Format.sprintf "%s%d" placeholder_base placeholder_n in
  let exp_string =
    source_slice ~source_text ~start_pos:pos ~length:(Pos.length pos)
  in
  let change_expression =
    Lsp.TextEdit.{ range = lsp_range_of_pos pos; newText = placeholder }
  in
  let change_add_assignment =
    let (line, character) =
      Pos.line_column stmt_pos |> Tuple2.map_fst ~f:(( + ) (-1))
    in
    let indent = String.make character ' ' in
    Lsp.
      {
        TextEdit.range =
          { start = { line; character }; end_ = { line; character } };
        newText = Printf.sprintf "%s = %s;\n%s" placeholder exp_string indent;
      }
  in
  let changes =
    SMap.singleton path [change_add_assignment; change_expression]
  in
  let code_action =
    {
      Lsp.CodeAction.title = "Extract into variable";
      kind = Lsp.CodeActionKind.refactor;
      diagnostics = [];
      action = Lsp.CodeAction.EditOnly Lsp.WorkspaceEdit.{ changes };
    }
  in
  Lsp.CodeAction.Action code_action

let find ~(range : Lsp.range) ~path ~entry ctx tast =
  let is_range_selection =
    Lsp.(
      range.start.line < range.end_.line
      || range.start.line = range.end_.line
         && range.start.character < range.end_.character)
  in
  match entry.Provider_context.source_text with
  | None -> []
  | Some source_text ->
    if is_range_selection then
      (top_visitor ctx range ~source_text)#go ctx tast
      |> Option.map ~f:(command_or_action_of_candidate ~source_text ~path)
      |> Option.to_list
    else
      []
