(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type candidate = {
  expr_pos: Pos.t;
  containing_stmt_pos: Pos.t;
}

let offset_of_pos source_text pos =
  let (line, start, _) = Pos.info_pos pos in
  Full_fidelity_source_text.position_to_offset source_text (line, start)

let find_candidate ctx tast selection : candidate option =
  let visitor =
    let expr_positions_overlapping_selection = ref [] in
    (* We don't want to provide the refactoring in cases like this:
       /*range-start*/
       $x = 1;
       $y = gen_foo();
       /*range-end*/
    *)
    let ensure_valid_selection =
      Option.filter ~f:(fun candidate ->
          List.for_all
            !expr_positions_overlapping_selection
            ~f:(Pos.contains candidate.expr_pos))
    in
    let in_await = ref false in
    let stmt_pos : Pos.t option ref = ref None in
    object
      inherit [candidate option] Tast_visitor.reduce as super

      method zero = None

      method plus = Option.first_some

      method! on_def env def =
        let def_pos =
          match def with
          | Aast.Fun fun_def -> Aast.(fun_def.fd_fun.f_span)
          | Aast.Class class_def -> Aast.(class_def.c_span)
          | Aast.Stmt (pos, _) -> pos
          | _ -> Pos.none
        in
        (* Avoid expensive unnecessary traversals *)
        if Pos.contains def_pos selection then
          ensure_valid_selection @@ super#on_def env def
        else
          None

      method! on_fun_ env fun_ =
        in_await := false;
        super#on_fun_ env fun_

      method! on_Await env await =
        in_await := true;
        super#on_Await env await

      method! on_stmt env stmt =
        stmt_pos := Some (fst stmt);
        super#on_stmt env stmt

      method! on_expr env expr =
        let (ty, expr_pos, _) = expr in
        if Pos.contains selection expr_pos then
          expr_positions_overlapping_selection :=
            expr_pos :: !expr_positions_overlapping_selection;
        match super#on_expr env expr with
        | None when Pos.contains selection expr_pos ->
          let needs_await =
            match Typing_defs_core.get_node ty with
            | Typing_defs_core.Tclass ((_, c_name), _, _) ->
              (not !in_await)
              && String.equal c_name Naming_special_names.Classes.cAwaitable
            | _ -> false
          in
          if needs_await then
            Option.map !stmt_pos ~f:(fun containing_stmt_pos ->
                { expr_pos; containing_stmt_pos })
          else
            None
        | candidate_opt -> candidate_opt
    end
  in
  visitor#go ctx tast

let calc_modifier_and_signature_edits path source_text positioned_tree pos :
    Code_action_types.edit list option =
  let module Syn = Full_fidelity_positioned_syntax in
  let open Option.Let_syntax in
  let pos_of_offset offset : Pos.t =
    let (line, col) =
      Full_fidelity_source_text.offset_to_position source_text offset
    in
    let bol = offset - col + 1 in
    let triple = (line, bol, offset) in
    Pos.make_from_lnum_bol_offset
      ~pos_file:path
      ~pos_start:triple
      ~pos_end:triple
  in

  let signature_edit_of_type_node (type_node : Syn.t) :
      Code_action_types.edit option =
    let return_type_string = Syn.text type_node in
    if
      (not @@ Syn.is_missing type_node)
      && (not @@ String.is_prefix return_type_string ~prefix:"Awaitable")
      && (not @@ String.is_prefix return_type_string ~prefix:"~Awaitable")
    then
      let+ return_type_offset = Syn.offset type_node in
      let pos =
        let start_pos = pos_of_offset return_type_offset in
        let end_pos = pos_of_offset (Syn.end_offset type_node + 1) in
        Pos.merge start_pos end_pos
      in
      let text = Printf.sprintf "Awaitable<%s>" return_type_string in
      Code_action_types.{ pos; text }
    else
      None
  in

  let parents =
    let root = Provider_context.PositionedSyntaxTree.root positioned_tree in
    Syn.parentage root (offset_of_pos source_text pos)
  in
  parents
  |> List.map ~f:Syn.syntax
  |> List.find_map ~f:(function
         | Syn.LambdaExpression
             {
               lambda_async = Syn.{ syntax = modifier; _ } as node;
               lambda_signature = Syn.{ syntax = signature; _ };
               _;
             } -> begin
           let signature_edit_opt =
             begin
               match signature with
               | Syn.LambdaSignature { lambda_type; _ } ->
                 signature_edit_of_type_node lambda_type
               | _ -> None
             end
           in
           let modifier_edit_opt =
             begin
               match modifier with
               | Syn.Missing ->
                 let+ offset = Syn.offset node in
                 Code_action_types.
                   { pos = pos_of_offset offset; text = "async " }
               | _ -> None
             end
           in
           Some (List.filter_opt [signature_edit_opt; modifier_edit_opt])
         end
         | Syn.FunctionDeclaration
             {
               function_declaration_header =
                 Syn.
                   {
                     syntax =
                       FunctionDeclarationHeader
                         {
                           function_modifiers;
                           function_keyword;
                           function_type;
                           _;
                         };
                     _;
                   };
               _;
             }
         | Syn.MethodishDeclaration
             {
               methodish_function_decl_header =
                 Syn.
                   {
                     syntax =
                       FunctionDeclarationHeader
                         {
                           function_modifiers;
                           function_keyword;
                           function_type;
                           _;
                         };
                     _;
                   };
               _;
             } ->
           let signature_edit_opt = signature_edit_of_type_node function_type in
           let modifier_edit_opt =
             let has_async =
               match Syn.syntax function_modifiers with
               | Syn.SyntaxList elements ->
                 elements
                 |> List.map ~f:Syn.syntax
                 |> List.exists ~f:(function
                        | Syn.Token
                            Full_fidelity_positioned_token.
                              { kind = TokenKind.Async; _ } ->
                          true
                        | _ -> false)
               | Syn.Missing -> false
               | _ -> false
             in
             if has_async then
               None
             else
               let+ function_keyword_offset = Syn.offset function_keyword in
               Code_action_types.
                 {
                   pos = pos_of_offset function_keyword_offset;
                   text = "async ";
                 }
           in
           Some (List.filter_opt [signature_edit_opt; modifier_edit_opt])
         | _ -> None)

let text_before_and_after_expr_in_stmt
    source_text { containing_stmt_pos; expr_pos } : string * string =
  let stmt_offset = offset_of_pos source_text containing_stmt_pos in
  let expr_offset = offset_of_pos source_text expr_pos in
  let expr_end_offset =
    offset_of_pos source_text (Pos.shrink_to_end expr_pos)
  in
  let stmt_end_offset =
    offset_of_pos source_text (Pos.shrink_to_end containing_stmt_pos)
  in
  let before_text =
    Full_fidelity_source_text.sub
      source_text
      stmt_offset
      (expr_offset - stmt_offset)
  in
  let after_text =
    Full_fidelity_source_text.sub
      source_text
      expr_end_offset
      (stmt_end_offset - expr_end_offset)
  in
  (before_text, after_text)

(** Enables us to distinguish whether a change to source code affects the structure of expressions in the AST.
 *
 * Examples
 * `equiv popt ~stmt1 ~stmt2` is `false` given:
 * stmt1:
 *   await gen_int() -> meth();
 * stmt2:
 *   (await gen_int()) -> meth();
 *
 * `equiv popt ~stm1 ~stmt2` is `true` given:
 * stmt1:
 *   await gen_int() + 3;
 * stmt2:
 *   (await gen_int()) + 3;
 *
 * We only pay attention to the shape of the tree, not what's in a node. `equiv popt ~stm1 ~stmt2` is `true` given:
 * stmt1:
 *   await $z || 3000;
 * stmt2:
 *   await gen_int() + 3;
 *)
module Expr_structure : sig
  val equiv : ParserOptions.t -> stmt1:string -> stmt2:string -> bool
end = struct
  type t =
    | Zero
    | Plus of t * t
    | On_expr of t
  [@@deriving eq]

  let of_nast (nast : Nast.program) =
    let visitor =
      object
        inherit [_] Aast.reduce as super

        method zero = Zero

        method plus a b = Plus (a, b)

        method! on_expr env expr = On_expr (super#on_expr env expr)
      end
    in
    nast
    |> List.map ~f:(visitor#on_def ())
    |> List.fold ~init:visitor#zero ~f:visitor#plus

  let nast_of_program_source_code popt s =
    let source_text = Full_fidelity_source_text.make Relative_path.default s in
    let parser_env =
      Full_fidelity_ast.make_env
        ~quick_mode:false
        ~parser_options:popt
        Relative_path.default
    in
    let Parser_return.{ ast; _ } =
      Full_fidelity_ast.from_source_text_with_legacy parser_env source_text
    in
    ast

  let wrap_as_program : string -> string =
    (* Convert a statement into a program, so we can parse it.
        We include an extra semicolon because not all statements end in semicolons.
        For example, in braceless lambdas: `() ==> expr_statement`.
        Double-semicolon parses OK, so we're all right.
    *)
    Printf.sprintf "<?hh\nfunction foo(): void { %s; }"

  let of_stmt_source_code popt statement_source_code =
    statement_source_code
    |> wrap_as_program
    |> nast_of_program_source_code popt
    |> of_nast

  let equiv popt ~stmt1 ~stmt2 =
    let structure1 = of_stmt_source_code popt stmt1 in
    let structure2 = of_stmt_source_code popt stmt2 in
    equal structure1 structure2
end

let calc_await_edit
    ctx source_text ({ expr_pos; containing_stmt_pos } as candidate) :
    Code_action_types.edit =
  let (before_text, after_text) =
    text_before_and_after_expr_in_stmt source_text candidate
  in
  let orig_expr_text =
    Full_fidelity_source_text.sub_of_pos source_text expr_pos
  in
  let with_parentheses =
    Printf.sprintf "%s(await %s)%s" before_text orig_expr_text after_text
  in
  let without_parentheses =
    Printf.sprintf "%sawait %s%s" before_text orig_expr_text after_text
  in
  let text =
    let popt = Provider_context.get_popt ctx in
    if
      Expr_structure.equiv
        popt
        ~stmt1:with_parentheses
        ~stmt2:without_parentheses
    then
      without_parentheses
    else
      with_parentheses
  in
  Code_action_types.{ pos = containing_stmt_pos; text }

let edits_of_candidate ctx entry ({ expr_pos; _ } as candidate) :
    Code_action_types.edit list =
  let path = entry.Provider_context.path in
  let source_text = Ast_provider.compute_source_text ~entry in
  let positioned_tree = Ast_provider.compute_cst ~ctx ~entry in

  let modifier_and_signature_edits =
    calc_modifier_and_signature_edits path source_text positioned_tree expr_pos
    |> Option.value ~default:[]
  in
  let await_edit = calc_await_edit ctx source_text candidate in
  modifier_and_signature_edits @ [await_edit]

let refactor_of_candidate ctx entry candidate =
  let edits =
    lazy
      (Relative_path.Map.singleton
         entry.Provider_context.path
         (edits_of_candidate ctx entry candidate))
  in
  Code_action_types.Refactor.{ title = "await expression"; edits }

let find ~entry selection ctx =
  if Pos.length selection <> 0 then
    let { Tast_provider.Compute_tast.tast; _ } =
      Tast_provider.compute_tast_quarantined ~ctx ~entry
    in
    find_candidate ctx tast.Tast_with_dynamic.under_normal_assumptions selection
    |> Option.map ~f:(refactor_of_candidate ctx entry)
    |> Option.to_list
  else
    []
