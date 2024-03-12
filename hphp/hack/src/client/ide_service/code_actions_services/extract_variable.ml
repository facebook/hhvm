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
  prev_stmt_pos: Pos.t option;  (** used for calculating indentation *)
  stmt_pos: Pos.t;  (** used for calculating indentation *)
  pos: Pos.t;
      (** position of expr or statement corresponding to the selection *)
  placeholder_n: int;
      (** used for naming the placeholder of the generated variable *)
}

let visitor (selection : Pos.t) =
  let should_traverse outer = Pos.contains outer selection in
  let prev_stmt_pos = ref None in
  let stmt_pos = ref None in
  let set_stmt_pos pos =
    if not Pos.(equal none pos) then begin
      prev_stmt_pos := !stmt_pos;
      stmt_pos := Some pos
    end
  in
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

  object (self)
    inherit [candidate option] Tast_visitor.reduce as super

    method zero = None

    method plus = Option.first_some

    method! on_def env =
      function
      | Aast.Fun fun_def when should_traverse Aast.(fun_def.fd_fun.f_span) ->
        self#on_fun_def env fun_def
      | Aast.Class class_def when should_traverse Aast.(class_def.c_span) ->
        self#on_class_ env class_def
      | Aast.Stmt stmt when should_traverse (fst stmt) -> self#on_stmt env stmt
      | _ -> None

    method! on_method_ env meth =
      if should_traverse meth.Aast.m_span then
        ensure_selection_common_root @@ super#on_method_ env meth
      else
        None

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
      | Some (pos, _) -> set_stmt_pos pos
      | _ -> ());
      match acc with
      | Some acc -> Some { acc with placeholder_n = !placeholder_n }
      | None -> None

    method! on_stmt env stmt =
      set_stmt_pos (fst stmt);
      super#on_stmt env stmt

    method! on_expr env expr =
      let (_, pos, expr_) = expr in
      if Pos.overlaps selection pos then
        expr_positions_overlapping_selection :=
          pos :: !expr_positions_overlapping_selection;

      match (!stmt_pos, expr_) with
      | (Some stmt_pos, _) when Pos.contains selection pos ->
        Some
          {
            prev_stmt_pos = !prev_stmt_pos;
            stmt_pos;
            pos;
            placeholder_n = 0 (* will be adjusted on the way up *);
          }
      | (_, Aast.(Binop { bop = Ast_defs.Eq _; lhs = (_, lhs_pos, _); rhs = _ }))
        ->
        let acc = super#on_expr env expr in
        Option.filter acc ~f:(fun candidate ->
            not @@ Pos.contains lhs_pos candidate.pos)
      | (_, Aast.Efun _) -> super#on_expr env expr
      | _ -> super#on_expr env expr
  end

(** Generate a snippet from the placeholder number.
This relies on a nonstandard LSP extension recognized by the client:
https://fburl.com/code/0vzkqds8. We can implement non-hackily if LSP is updated:
https://github.com/microsoft/language-server-protocol/issues/592 *)
let placeholder_name_of_n (n : int) = Format.sprintf "$${0:placeholder%d}" n

let offset_of_pos source_text pos =
  let (line, start, _) = Pos.info_pos pos in
  Full_fidelity_source_text.position_to_offset source_text (line, start)

(**
  true iff the expr or stmt at `pos` is in a lambda where the body is not wrapped in curly braces
  (single expression with an implicit return).  For example:
    - `true` for `() ==>  ...   expr  ... `
    - `false` for `() ==>  { ... expr ... }`
*)
let is_in_braceless_lambda source_text positioned_tree pos =
  let offset = offset_of_pos source_text pos in
  let nodes =
    let root = Provider_context.PositionedSyntaxTree.root positioned_tree in
    Full_fidelity_positioned_syntax.parentage root offset
  in
  let matching_lambda_body = function
    | Full_fidelity_positioned_syntax.LambdaExpression { lambda_body; _ }
      when Full_fidelity_positioned_syntax.start_offset lambda_body = offset ->
      Some lambda_body
    | _ -> None
  in
  nodes
  |> List.map ~f:Full_fidelity_positioned_syntax.syntax
  |> List.find_map ~f:matching_lambda_body
  |> Option.map
       ~f:(Fn.non Full_fidelity_positioned_syntax.is_compound_statement)
  |> Option.value ~default:false

let indent_of_pos pos =
  let (_, character) = Pos.line_column pos in
  String.make character ' '

let slice
    (source_text : Full_fidelity_source_text.t) (start : Pos.t) (end_ : Pos.t) :
    string =
  let start_offset = offset_of_pos source_text start in
  let end_offset = offset_of_pos source_text end_ in
  Full_fidelity_source_text.sub
    source_text
    start_offset
    (end_offset - start_offset)

let edits_of_candidate ctx entry { prev_stmt_pos; stmt_pos; pos; placeholder_n }
    : Code_action_types.edit list =
  let positioned_tree = Ast_provider.compute_cst ~ctx ~entry in
  let source_text = Ast_provider.compute_source_text ~entry in

  let placeholder = placeholder_name_of_n placeholder_n in
  let stmt_text =
    let before = slice source_text stmt_pos pos in
    let after =
      slice source_text (Pos.shrink_to_end pos) (Pos.shrink_to_end stmt_pos)
    in
    before ^ placeholder ^ after
  in
  let stmt_indent = indent_of_pos stmt_pos in
  let assignment_text =
    let exp_string = Full_fidelity_source_text.sub_of_pos source_text pos in
    Printf.sprintf "%s = %s;\n" placeholder exp_string
  in
  let text =
    if is_in_braceless_lambda source_text positioned_tree stmt_pos then
      let prev_indent =
        prev_stmt_pos
        |> Option.map ~f:indent_of_pos
        |> Option.value ~default:stmt_indent
      in
      let body_indent =
        let default_indent =
          String.make Format_env.(default.indent_width) ' '
        in
        prev_indent ^ default_indent
      in
      Format.sprintf
        "{\n%sreturn %s;\n%s}"
        (body_indent ^ assignment_text ^ body_indent)
        stmt_text
        prev_indent
    else
      assignment_text ^ stmt_indent ^ stmt_text
  in
  [Code_action_types.{ pos = stmt_pos; text }]

let refactor_of_candidate ctx entry path candidate =
  let edits =
    lazy
      (Relative_path.Map.singleton
         path
         (edits_of_candidate ctx entry candidate))
  in
  Code_action_types.{ title = "Extract into variable"; edits; kind = `Refactor }

let find ~entry selection ctx =
  let path = entry.Provider_context.path in
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  (visitor selection)#go ctx tast.Tast_with_dynamic.under_normal_assumptions
  |> Option.map ~f:(refactor_of_candidate ctx entry path)
  |> Option.to_list
