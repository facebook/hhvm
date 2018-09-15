open Hh_core

module SourceText = Full_fidelity_source_text
module TokenKind = Full_fidelity_token_kind
module ParserEnv = Full_fidelity_parser_env

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module State = struct
    type r = Syntax.t [@@deriving show]
    type t = bool list [@@deriving show]
    let env = ref @@ ParserEnv.make ()

    let initial env' = env := env'; []
    let next state args =
      let h, t = List.split_n state (List.length args) in
      let res = List.fold_left ~f:(||) ~init:false h in
      res :: t
  end

  module SyntaxSC = SyntaxSmartConstructors.WithSyntax(Syntax)
  include SyntaxSC.WithState(State)

  let missing = Syntax.make_missing SourceText.empty 0
  let yield = Syntax.make_token @@ Syntax.Token.make TokenKind.Yield SourceText.empty 0 0 [] []
  let yield_list = Syntax.make_list SourceText.empty 0 [yield]

  let replace_body body saw_yield =
    match Syntax.syntax body with
    | Syntax.CompoundStatement {compound_left_brace; compound_right_brace; _ } ->
      let stmts = if saw_yield then yield_list else missing in
      Syntax.make_compound_statement compound_left_brace stmts compound_right_brace
    | _ -> body

  let make_yield_expression _a1 _a2 = function
    | _ :: _ :: t ->
      true :: t, missing
    | _ -> failwith "Invalid state"

  let make_yield_from_expression _a1 _a2 _a3 = function
    | _ :: _ :: _ :: t ->
      true :: t, missing
    | _ -> failwith "Invalid state"

  let make_lambda_expression a1 a2 a3 a4 a5 body = function
    | saw_yield :: _ :: _ :: _ :: _ :: _ :: t ->
      let body = replace_body body saw_yield in
      false :: t, Syntax.make_lambda_expression a1 a2 a3 a4 a5 body
    | _ -> failwith "Invalid state"

  let make_anonymous_function a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 body = function
    | saw_yield :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: t ->
      let body = replace_body body saw_yield in
      false :: t, Syntax.make_anonymous_function a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 body
    | _ -> failwith "Invalid state"

  let make_php7_anonymous_function a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 body = function
    | saw_yield :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: _ :: t ->
      let body = replace_body body saw_yield in
      false :: t, Syntax.make_php7_anonymous_function a1 a2 a3 a4 a5 a6 a7 a8 a9 a10 a11 a12 body
    | _ -> failwith "Invalid state"

  let make_awaitable_creation_expression a1 a2 a3 body = function
    | saw_yield :: _ :: _ :: _ :: t ->
      let body = replace_body body saw_yield in
      false :: t, Syntax.make_awaitable_creation_expression a1 a2 a3 body
    | _ -> failwith "Invalid state"

  let make_methodish_declaration a1 a2 body a3 = function
    | _ :: saw_yield :: _ :: _ :: t ->
      let body = replace_body body saw_yield in
      false :: t, Syntax.make_methodish_declaration a1 a2 body a3
    | _ -> failwith "Invalid state"

  let make_function_declaration a1 a2 body = function
    | saw_yield :: _ :: _ :: t ->
      let body = replace_body body saw_yield in
      false :: t, Syntax.make_function_declaration a1 a2 body
    | _ -> failwith "Invalid state"

end (* WithSyntax *)
