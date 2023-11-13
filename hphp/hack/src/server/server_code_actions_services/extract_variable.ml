(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let placeholder_regexp = Str.regexp {|\$placeholder\([0-9]+\)|}

type candidate = {
  stmt_pos: Pos.t;
  pos: Pos.t;
  placeholder_n: int;
}

(**
We don't want to extract variables for lambdas like this: `() ==> 200`.
The AST of such a lambda is indistinguishable from `() ==> { return 200; }`
so we peek at the source *)
let might_be_expression_lambda ~f_body:Aast.{ fb_ast } ~pos ~source_text =
  match fb_ast with
  | [(stmt_pos, _)] ->
    let length = Pos.start_offset stmt_pos - Pos.start_offset pos in
    if length > 0 then
      let src = Full_fidelity_source_text.sub_of_pos source_text pos ~length in
      not @@ String.is_substring ~substring:"{" src
    else
      (*  length can be negative to curlies in default params: `(($a = () ==> {}) ==> ...` *)
      true
  | _ -> false

let positions_visitor (selection : Pos.t) ~source_text =
  let stmt_pos = ref Pos.none in
  let expression_lambda_pos = ref None in
  let placeholder_n = ref 0 in
  let expr_positions_overlapping_selection = ref [] in
  let ensure_selection_common_root =
    (* filter out invalid selection like this:
          (1 + 2) +  3
               ^-----^ selection
    *)
    Option.filter ~f:(fun candidate ->
        List.for_all !expr_positions_overlapping_selection ~f:(fun p ->
            Pos.(contains candidate.pos p || contains p candidate.pos)))
  in

  object
    inherit [candidate option] Tast_visitor.reduce as super

    method zero = None

    method plus = Option.first_some

    method! on_method_ env meth =
      ensure_selection_common_root @@ super#on_method_ env meth

    method! on_fun_def env fd =
      ensure_selection_common_root @@ super#on_fun_def env fd

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
      if Pos.overlaps selection pos then
        expr_positions_overlapping_selection :=
          pos :: !expr_positions_overlapping_selection;

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
          Pos.contains selection pos
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
let top_visitor (selection : Pos.t) ~source_text =
  let should_traverse outer = Pos.contains outer selection in
  object (self)
    inherit [candidate option] Tast_visitor.reduce

    method zero = None

    method plus = Option.first_some

    method! on_def env =
      function
      | Aast.Fun fun_def when should_traverse Aast.(fun_def.fd_fun.f_span) ->
        (positions_visitor selection ~source_text)#on_fun_def env fun_def
      | Aast.Class class_def when should_traverse Aast.(class_def.c_span) ->
        self#on_class_ env class_def
      | Aast.Stmt stmt when should_traverse (fst stmt) ->
        (positions_visitor selection ~source_text)#on_stmt env stmt
      | _ -> None

    method! on_method_ env meth =
      if should_traverse meth.Aast.m_span then
        (positions_visitor selection ~source_text)#on_method_ env meth
      else
        None
  end

(** Generate a snippet from the placeholder number.
This relies on a nonstandard LSP extension recognized by the client:
https://fburl.com/code/0vzkqds8. We can implement non-hackily if LSP is updated:
https://github.com/microsoft/language-server-protocol/issues/592 *)
let placeholder_name_of_n (n : int) = Format.sprintf "$${0:placeholder%d}" n

let refactor_of_candidate ~source_text ~path { stmt_pos; pos; placeholder_n } =
  let placeholder = placeholder_name_of_n placeholder_n in
  let exp_string = Full_fidelity_source_text.sub_of_pos source_text pos in
  let change_expression = Code_action_types.{ pos; text = placeholder } in
  let change_add_assignment =
    let indent =
      let (_, character) = Pos.line_column stmt_pos in
      String.make character ' '
    in
    let pos = Pos.shrink_to_start stmt_pos in
    Code_action_types.
      {
        pos;
        text = Printf.sprintf "%s = %s;\n%s" placeholder exp_string indent;
      }
  in
  let edits =
    lazy
      (Relative_path.Map.singleton
         path
         [change_add_assignment; change_expression])
  in
  Code_action_types.Refactor.{ title = "Extract into variable"; edits }

let find ~entry selection ctx =
  let path = entry.Provider_context.path in
  let source_text = Ast_provider.compute_source_text ~entry in
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  (top_visitor selection ~source_text)#go
    ctx
    tast.Tast_with_dynamic.under_normal_assumptions
  |> Option.map ~f:(refactor_of_candidate ~source_text ~path)
  |> Option.to_list
