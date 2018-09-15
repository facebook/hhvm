(**
 * Performs a regular parse, but determines whether the coroutine keyword
 * appears in the file so that we know whether to lower coroutines through
 * a conversion of the positioned syntax to an editable positioned syntax
 *)

open Core_kernel

module ParserEnv = Full_fidelity_parser_env
module TokenKind = Full_fidelity_token_kind

let ppl_macro_string = "__PPL"

module WithSyntax(Syntax : Positioned_syntax_sig.PositionedSyntax_S) = struct
  module State = struct
    type r = Syntax.t [@@deriving show]
    type t = bool [@@deriving show]
    let env : ParserEnv.t ref = ref @@ ParserEnv.make ()

    let initial env' = env := env'; false
    let next state _ = state
  end

  module SyntaxSC = SyntaxSmartConstructors.WithSyntax(Syntax)
  include SyntaxSC.WithState(State)

  let is_coroutine = Syntax.is_coroutine

  let make_token token state =
    let token =
      if ParserEnv.codegen !State.env
      then Token.with_trailing [] @@  Token.with_leading [] token
      else token in
    state, Syntax.make_token token

  let make_function_declaration_header modifiers r2 r3 r4 r5 r6 r7 r8 r9 r10 r11 state =
    let state = state ||
      (Syntax.syntax_node_to_list modifiers |>
      List.exists ~f:is_coroutine) in
    state, Syntax.make_function_declaration_header modifiers r2 r3 r4 r5 r6 r7 r8 r9 r10 r11

  let make_closure_type_specifier r1 coroutine r3 r4 r5 r6 r7 r8 r9 state =
    let state = state ||
      is_coroutine coroutine in
    state, Syntax.make_closure_type_specifier r1 coroutine r3 r4 r5 r6 r7 r8 r9

  let make_anonymous_function r1 r2 r3 coroutine r5 r6 r7 r8 r9 r10 r11 r12 r13 state =
    let state = state ||
      is_coroutine coroutine in
    state, Syntax.make_anonymous_function r1 r2 r3 coroutine r5 r6 r7 r8 r9 r10 r11 r12 r13

  let make_php7_anonymous_function r1 r2 r3 coroutine r5 r6 r7 r8 r9 r10 r11 r12 r13 state =
    let state = state ||
      is_coroutine coroutine in
    state, Syntax.make_php7_anonymous_function r1 r2 r3 coroutine r5 r6 r7 r8 r9 r10 r11 r12 r13

  let make_lambda_expression r1 r2 coroutine r3 r4 r5 state =
    let state = state ||
      is_coroutine coroutine in
    state, Syntax.make_lambda_expression r1 r2 coroutine r3 r4 r5

  let make_awaitable_creation_expression r1 r2 coroutine r4 state =
    let state = state ||
      is_coroutine coroutine in
    state, Syntax.make_awaitable_creation_expression r1 r2 coroutine r4

  let make_attribute_specification left attribute_name right state =
    let attribute_string = Syntax.extract_text attribute_name in
    let state = state ||
      Option.value_map attribute_string ~default:false ~f:(fun name -> name = "__PPL") in
    state, Syntax.make_attribute_specification left attribute_name right

end (* WithSyntax *)
