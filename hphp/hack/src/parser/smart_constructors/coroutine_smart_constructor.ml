(**
 * Performs a regular parse, but determines whether the coroutine keyword
 * appears in the file so that we know whether to lower coroutines through
 * a conversion of the positioned syntax to an editable positioned syntax
 *)
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

  let make_token token state =
    let state =
      state ||
      Token.kind token = TokenKind.Coroutine ||
      Syntax.Token.text token = "__PPL" in
    let token =
      if ParserEnv.codegen !State.env
      then Token.with_trailing [] @@  Token.with_leading [] token
      else token in
    state, Syntax.make_token token

end (* WithSyntax *)
