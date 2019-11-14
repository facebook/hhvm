(*
 * Performs a regular parse, but determines whether the coroutine keyword
 * appears in the file so that we know whether to lower coroutines through
 * a conversion of the positioned syntax to an editable positioned syntax
 *)

open Core_kernel
module ParserEnv = Full_fidelity_parser_env
module TokenKind = Full_fidelity_token_kind

let ppl_macro_string = "__PPL"

module WithSyntax (Syntax : Positioned_syntax_sig.PositionedSyntax_S) = struct
  module State = struct
    type r = Syntax.t [@@deriving show]

    type t = bool [@@deriving show]

    let env : ParserEnv.t ref = ref @@ ParserEnv.make ()

    let initial env' =
      env := env';
      false

    let next state _ = state
  end

  module SyntaxSC_ = SyntaxSmartConstructors.WithSyntax (Syntax)
  module SyntaxSC = SyntaxSC_.WithState (State)

  include SyntaxSC.WithRustParser (struct
    type r = Syntax.t

    type t = bool

    let rust_parse = Syntax.rust_parse_with_coroutine_sc
  end)

  let is_coroutine = Syntax.is_coroutine

  let make_token token state =
    let token =
      if ParserEnv.codegen !State.env then
        Token.with_trailing [] @@ Token.with_leading [] token
      else
        token
    in
    (state, Syntax.make_token token)

  let make_function_declaration_header
      modifiers r2 r3 r4 r5 r6 r7 r8 r9 r10 state =
    let state =
      state
      || Syntax.syntax_node_to_list modifiers |> List.exists ~f:is_coroutine
    in
    ( state,
      Syntax.make_function_declaration_header
        modifiers
        r2
        r3
        r4
        r5
        r6
        r7
        r8
        r9
        r10 )

  let make_closure_type_specifier r1 coroutine r3 r4 r5 r6 r7 r8 r9 state =
    let state = state || is_coroutine coroutine in
    (state, Syntax.make_closure_type_specifier r1 coroutine r3 r4 r5 r6 r7 r8 r9)

  let make_anonymous_function
      r1 r2 r3 coroutine r5 r6 r7 r8 r9 r10 r11 r12 state =
    let state = state || is_coroutine coroutine in
    ( state,
      Syntax.make_anonymous_function
        r1
        r2
        r3
        coroutine
        r5
        r6
        r7
        r8
        r9
        r10
        r11
        r12 )

  let make_lambda_expression r1 r2 coroutine r3 r4 r5 state =
    let state = state || is_coroutine coroutine in
    (state, Syntax.make_lambda_expression r1 r2 coroutine r3 r4 r5)

  let make_awaitable_creation_expression r1 r2 coroutine r4 state =
    let state = state || is_coroutine coroutine in
    (state, Syntax.make_awaitable_creation_expression r1 r2 coroutine r4)

  let make_old_attribute_specification left attribute_name right state =
    Syntax.(
      let is_ppl_attribute_folder has_seen_ppl constructor_call =
        if has_seen_ppl then
          has_seen_ppl
        else
          match Syntax.syntax constructor_call with
          | Syntax.ConstructorCall
              {
                constructor_call_type;
                constructor_call_left_paren;
                constructor_call_argument_list;
                constructor_call_right_paren;
              } ->
            Syntax.is_missing constructor_call_left_paren
            && Syntax.is_missing constructor_call_argument_list
            && Syntax.is_missing constructor_call_right_paren
            && Option.value_map
                 (Syntax.extract_text constructor_call_type)
                 ~default:false
                 ~f:(fun text -> text = ppl_macro_string)
          | _ -> false
      in
      let state =
        state
        || syntax_list_fold
             ~init:false
             ~f:is_ppl_attribute_folder
             attribute_name
      in
      (state, Syntax.make_old_attribute_specification left attribute_name right))
end
