(*
 * Copyright (c) 2020, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
let token_to_string (tok : Phparser.token) =
  match tok with
  | Phparser.YIELD_FROM -> "YIELD_FROM"
  | Phparser.YIELD -> "YIELD"
  | Phparser.XOR_EQUAL -> "XOR_EQUAL"
  | Phparser.XOR -> "XOR"
  | Phparser.XHP_TEXT _ -> "XHP_TEXT _"
  | Phparser.XHP_SLASH_GT -> "XHP_SLASH_GT"
  | Phparser.XHP_REQUIRED -> "XHP_REQUIRED"
  | Phparser.XHP_PERCENTID_DEF _ -> "XHP_PERCENTID_DEF _"
  | Phparser.XHP_PCDATA -> "XHP_PCDATA"
  | Phparser.XHP_OPEN_TAG _ -> "XHP_OPEN_TAG _"
  | Phparser.XHP_GT -> "XHP_GT"
  | Phparser.XHP_COLONID_DEF _ -> "XHP_COLONID_DEF _"
  | Phparser.XHP_CLOSE_TAG _ -> "XHP_CLOSE_TAG _"
  | Phparser.XHP_CHILDREN -> "XHP_CHILDREN"
  | Phparser.XHP_CATEGORY -> "XHP_CATEGORY"
  | Phparser.XHP_ATTRIBUTE -> "XHP_ATTRIBUTE"
  | Phparser.XHP_ATTR _ -> "XHP_ATTR _"
  | Phparser.XHP_ANY -> "XHP_ANY"
  | Phparser.WHILE -> "WHILE"
  | Phparser.VARNAME _ -> "VARNAME _"
  | Phparser.VARIABLE _ -> "VARIABLE _"
  | Phparser.VAR -> "VAR"
  | Phparser.USE -> "USE"
  | Phparser.UNSET_CAST -> "UNSET_CAST"
  | Phparser.UNSET -> "UNSET"
  | Phparser.UNKNOWN -> "UNKNOWN"
  | Phparser.TYPE -> "TYPE"
  | Phparser.TRY -> "TRY"
  | Phparser.TRAIT_C -> "TRAIT_C"
  | Phparser.TRAIT -> "TRAIT"
  | Phparser.TILDE -> "TILDE"
  | Phparser.THROW -> "THROW"
  | Phparser.SWITCH -> "SWITCH"
  | Phparser.STRING_CAST -> "STRING_CAST"
  | Phparser.STATIC -> "STATIC"
  | Phparser.START_HEREDOC -> "START_HEREDOC"
  | Phparser.SPACESHIP -> "SPACESHIP"
  | Phparser.SPACES -> "SPACES"
  | Phparser.SHR_EQUAL -> "SHR_EQUAL"
  | Phparser.SHR_PREFIX -> "SHR_PREFIX"
  | Phparser.SHL_EQUAL -> "SHL_EQUAL"
  | Phparser.SHL -> "SHL"
  | Phparser.SHAPE -> "SHAPE"
  | Phparser.SEMICOLON -> "SEMICOLON"
  | Phparser.SELF -> "SELF"
  | Phparser.RPAREN -> "RPAREN"
  | Phparser.RETURN -> "RETURN"
  | Phparser.REQUIRE_ONCE -> "REQUIRE_ONCE"
  | Phparser.REQUIRE -> "REQUIRE"
  | Phparser.RBRACKET -> "RBRACKET"
  | Phparser.RBRACE -> "RBRACE"
  | Phparser.RANGLE -> "RANGLE"
  | Phparser.QUESTION_ARROW -> "QUESTION_ARROW"
  | Phparser.QUESTION -> "QUESTION"
  | Phparser.PUBLIC -> "PUBLIC"
  | Phparser.PROTECTED -> "PROTECTED"
  | Phparser.PRIVATE -> "PRIVATE"
  | Phparser.PRINT -> "PRINT"
  | Phparser.POW_EQUAL -> "POW_EQUAL"
  | Phparser.POW -> "POW"
  | Phparser.PLUS_EQUAL -> "PLUS_EQUAL"
  | Phparser.PLUS -> "PLUS"
  | Phparser.PIPE_ANGLE -> "PIPE_ANGLE"
  | Phparser.PARENT -> "PARENT"
  | Phparser.OR_EQUAL -> "OR_EQUAL"
  | Phparser.OR -> "OR"
  | Phparser.OPEN_TAG_WITH_ECHO -> "OPEN_TAG_WITH_ECHO"
  | Phparser.OPEN_TAG -> "OPEN_TAG"
  | Phparser.OBJECT_CAST -> "OBJECT_CAST"
  | Phparser.NUM_STRING _ -> "NUM_STRING _"
  | Phparser.NEWTYPE -> "NEWTYPE"
  | Phparser.NEWLINE -> "NEWLINE"
  | Phparser.NEW -> "NEW"
  | Phparser.NAMESPACE_C -> "NAMESPACE_C"
  | Phparser.NAMESPACE -> "NAMESPACE"
  | Phparser.MUL_EQUAL -> "MUL_EQUAL"
  | Phparser.MUL -> "MUL"
  | Phparser.MOD_EQUAL -> "MOD_EQUAL"
  | Phparser.MOD -> "MOD"
  | Phparser.MINUS_EQUAL -> "MINUS_EQUAL"
  | Phparser.MINUS -> "MINUS"
  | Phparser.METHOD_C -> "METHOD_C"
  | Phparser.LPAREN -> "LPAREN"
  | Phparser.LONG_DOUBLE_ARROW -> "LONG_DOUBLE_ARROW"
  | Phparser.LOGICAL_XOR -> "LOGICAL_XOR"
  | Phparser.LOGICAL_OR -> "LOGICAL_OR"
  | Phparser.LOGICAL_AND -> "LOGICAL_AND"
  | Phparser.LNUMBER _ -> "LNUMBER _"
  | Phparser.LIST -> "LIST"
  | Phparser.LINE -> "LINE"
  | Phparser.LBRACKET -> "LBRACKET"
  | Phparser.LBRACE -> "LBRACE"
  | Phparser.LANGLE -> "LANGLE"
  | Phparser.IS_SMALLER_OR_EQUAL -> "IS_SMALLER_OR_EQUAL"
  | Phparser.IS_NOT_IDENTICAL -> "IS_NOT_IDENTICAL"
  | Phparser.IS_NOT_EQUAL -> "IS_NOT_EQUAL"
  | Phparser.IS_IDENTICAL -> "IS_IDENTICAL"
  | Phparser.IS_GREATER_OR_EQUAL -> "IS_GREATER_OR_EQUAL"
  | Phparser.IS_EQUAL -> "IS_EQUAL"
  | Phparser.ISSET -> "ISSET"
  | Phparser.INT_CAST -> "INT_CAST"
  | Phparser.INTERFACE -> "INTERFACE"
  | Phparser.INSTEADOF -> "INSTEADOF"
  | Phparser.INSTANCEOF -> "INSTANCEOF"
  | Phparser.INLINE_HTML _ -> "INLINE_HTML _"
  | Phparser.INCLUDE_ONCE -> "INCLUDE_ONCE"
  | Phparser.INCLUDE -> "INCLUDE"
  | Phparser.INC -> "INC"
  | Phparser.IMPLEMENTS -> "IMPLEMENTS"
  | Phparser.IF -> "IF"
  | Phparser.IDENT s -> Printf.sprintf "IDENT %S" s
  | Phparser.HALT_COMPILER -> "HALT_COMPILER"
  | Phparser.GOTO -> "GOTO"
  | Phparser.GLOBAL -> "GLOBAL"
  | Phparser.FUNC_C -> "FUNC_C"
  | Phparser.FUNCTION -> "FUNCTION"
  | Phparser.FOREACH -> "FOREACH"
  | Phparser.FOR -> "FOR"
  | Phparser.FINALLY -> "FINALLY"
  | Phparser.FINAL -> "FINAL"
  | Phparser.FILE -> "FILE"
  | Phparser.EXTENDS -> "EXTENDS"
  | Phparser.EXIT -> "EXIT"
  | Phparser.EVAL -> "EVAL"
  | Phparser.EQUAL -> "EQUAL"
  | Phparser.EOF -> "EOF"
  | Phparser.ENUM -> "ENUM"
  | Phparser.END_HEREDOC -> "END_HEREDOC"
  | Phparser.ENDIF -> "ENDIF"
  | Phparser.ENCAPSED_AND_WHITESPACE _ -> "ENCAPSED_AND_WHITESPACE _"
  | Phparser.EMPTY -> "EMPTY"
  | Phparser.ELSEIF -> "ELSEIF"
  | Phparser.ELSE -> "ELSE"
  | Phparser.ELLIPSIS -> "ELLIPSIS"
  | Phparser.ECHO -> "ECHO"
  | Phparser.DOUBLE_CAST -> "DOUBLE_CAST"
  | Phparser.DOUBLE_ARROW -> "DOUBLE_ARROW"
  | Phparser.DOUBLEQUOTE -> "DOUBLEQUOTE"
  | Phparser.DOLLAR_OPEN_CURLY_BRACES -> "DOLLAR_OPEN_CURLY_BRACES"
  | Phparser.DOLLARDOLLAR -> "DOLLARDOLLAR"
  | Phparser.DOLLAR -> "DOLLAR"
  | Phparser.DOC_COMMENT -> "DOC_COMMENT"
  | Phparser.DO -> "DO"
  | Phparser.DNUMBER _ -> "DNUMBER _"
  | Phparser.DIV_EQUAL -> "DIV_EQUAL"
  | Phparser.DIV -> "DIV"
  | Phparser.DIR -> "DIR"
  | Phparser.DEFAULT -> "DEFAULT"
  | Phparser.DECLARE -> "DECLARE"
  | Phparser.DEC -> "DEC"
  | Phparser.CURLY_OPEN -> "CURLY_OPEN"
  | Phparser.CONTINUE -> "CONTINUE"
  | Phparser.CONSTANT_ENCAPSED_STRING _ -> "CONSTANT_ENCAPSED_STRING _"
  | Phparser.CONST -> "CONST"
  | Phparser.CONCAT_EQUAL -> "CONCAT_EQUAL"
  | Phparser.CONCAT -> "CONCAT"
  | Phparser.COMMENT -> "COMMENT"
  | Phparser.COMMA -> "COMMA"
  | Phparser.COLONCOLON -> "COLONCOLON"
  | Phparser.COLON -> "COLON"
  | Phparser.COALESCE -> "COALESCE"
  | Phparser.CLOSE_TAG_OF_ECHO -> "CLOSE_TAG_OF_ECHO"
  | Phparser.CLOSE_TAG -> "CLOSE_TAG"
  | Phparser.CLONE -> "CLONE"
  | Phparser.CLASS_C -> "CLASS_C"
  | Phparser.CLASS -> "CLASS"
  | Phparser.CATCH -> "CATCH"
  | Phparser.CASE -> "CASE"
  | Phparser.CALLABLE -> "CALLABLE"
  | Phparser.BREAK -> "BREAK"
  | Phparser.BOOL_CAST -> "BOOL_CAST"
  | Phparser.BOOLEAN_OR -> "BOOLEAN_OR"
  | Phparser.BOOLEAN_AND -> "BOOLEAN_AND"
  | Phparser.BANG -> "BANG"
  | Phparser.BACKQUOTE -> "BACKQUOTE"
  | Phparser.AWAIT -> "AWAIT"
  | Phparser.AT -> "AT"
  | Phparser.ASYNC -> "ASYNC"
  | Phparser.AS -> "AS"
  | Phparser.ARROW -> "ARROW"
  | Phparser.ARRAY_CAST -> "ARRAY_CAST"
  | Phparser.ARRAY -> "ARRAY"
  | Phparser.ANTISLASH -> "ANTISLASH"
  | Phparser.AND_EQUAL -> "AND_EQUAL"
  | Phparser.AND -> "AND"
  | Phparser.ABSTRACT -> "ABSTRACT"
  | Phparser.QUESTION_QUESTION -> "QUESTION_QUESTION"
  | Phparser.QUESTION_QUESTION_EQUAL -> "QUESTION_QUESTION_EQUAL"
  | Phparser.IS -> "IS"
  | Phparser.INOUT -> "INOUT"
  | Phparser.USING -> "USING"
  | Phparser.CONCURRENT -> "CONCURRENT"
  | Phparser.FORK -> "FORK"
  | Phparser.WHERE -> "WHERE"
  | Phparser.SUPER -> "SUPER"
  | Phparser.SUSPEND -> "SUSPEND"
  | Phparser.FOREACH_AS -> "FOREACH_AS"
  | Phparser.COROUTINE -> "COROUTINE"
  | Phparser.FORK_LANGLE _ -> "FORK_LANGLE"

let token_to_string tok =
  try token_to_string tok
  with _ -> "FIX token_to_string"

module I = Phparser.MenhirInterpreter

let rec normalize = function
  | (I.Shifting _ | I.AboutToReduce _) as cp ->
    normalize (I.resume cp)
  | cp -> cp

type t =
  | Normal of {
      state: unit I.checkpoint;
      rest: t;
    }
  | Fork of {
      state: unit I.checkpoint;
      tokens: (Phparser.token * Lexing.position * Lexing.position) list;
      branch_point: (bool ref * unit I.checkpoint);
      rest: t;
    }
  | Done

exception Accept

let verbose = ref false

let rec concat a b =
  match a with
  | Normal r -> Normal {r with rest = concat r.rest b}
  | Fork r -> Fork {r with rest = concat r.rest b}
  | Done -> b

let concat a = function
  | Done -> a
  | b -> concat a b

let prepare_after_fork tokens =
  let rec aux acc = function
    | [] -> assert false
    | [Phparser.FORK_LANGLE reduced, startp, endp] ->
      assert (not !reduced);
      (Phparser.LANGLE, startp, endp) :: acc
    | token :: tokens ->
      aux (token :: acc) tokens
  in
  aux [] tokens

let rec progress token = function
  | Done -> Done
  | Normal { state; rest } ->
    begin match normalize (I.offer state token) with
      | I.Accepted () -> raise Accept
      | I.HandlingError _ | I.Rejected -> progress token rest
      | (I.InputNeeded _) as cp ->
        Normal { state = cp; rest = progress token rest }
    end
  | Fork { state; tokens; branch_point; rest } ->
    begin match normalize (I.offer state token) with
      | I.Accepted () -> raise Accept
      | I.Rejected -> assert false
      | I.HandlingError env ->
        let reduced, state' = branch_point in
        if !reduced then
          progress token rest
        else
          let resumed =
            List.fold_left (fun a (tok, _, _ as b) -> match a with
                | Done -> (*print_endline "replaying failed!";*) Done
                | a ->
                  (*print_endline ("replaying: " ^ token_to_string tok);*)
                  progress b a)
              (Normal { state = state'; rest = Done })
              (prepare_after_fork (token :: tokens))
          in
          concat resumed (progress token rest)
      | (I.InputNeeded _) as cp ->
        let reduced, _ = branch_point in
        if !reduced then
          Normal { state = cp; rest = progress token rest }
        else
          Fork { state = cp;
                 tokens = token :: tokens; branch_point;
                 rest = progress token rest }
    end

let rec fork reduced = function
  | Normal { state; rest }
  | Fork { state; rest; branch_point = ({contents = true}, _); _ } ->
    Fork { state; tokens = []; branch_point = (reduced, state);
           rest = fork reduced rest }
  | Fork r ->
    Fork {r with rest = fork reduced r.rest }
  | Done -> Done

let progress token parser =
  let parser =
    match token with
    | Phparser.LPAREN, startp, _ ->
      concat parser (progress (Phparser.FORK, startp, startp) parser)
    | _ -> parser
  in
  match token with
  | Phparser.AS, startp, endp ->
    begin match progress (Phparser.FOREACH_AS, startp, endp) parser with
      | Done -> progress token parser
      | result -> result
    end
  | Phparser.LANGLE, startp, endp ->
    let reduced = ref false in
    let parser = fork reduced parser in
    progress (Phparser.FORK_LANGLE reduced, startp, endp) parser
  | Phparser.CLOSE_TAG, _, _ -> parser
  | _ -> progress token parser

let state_number = function
  | I.InputNeeded env ->
    let rec pop = function
      | _, None | 0, _ -> []
      | n, Some env ->
        let x = I.current_state_number env in
        x :: pop (n - 1, I.pop env)
    in
    pop (-1, Some env)
  | _ -> assert false

let rec take n = function
  | x :: xs when n > 0 -> x :: take (n - 1) xs
  | _ -> []

let dump_parser = function
  | Normal r ->
    let state = state_number r.state in
    "Normal " ^
    String.concat "," (List.map string_of_int state) ^ ":\n\n" ^
    String.concat "\n" (List.map Describe.describe_state (take 2 state))
  | Fork r ->
    let state = state_number r.state in
    "Fork " ^
    String.concat "," (List.map string_of_int state) ^ ":\n\n" ^
    String.concat "\n" (List.map Describe.describe_state (take 2 state))
  | Done -> assert false

let rec get_parsers = function
  | Normal r as parser -> parser :: get_parsers r.rest
  | Fork r as parser -> parser :: get_parsers r.rest
  | Done -> []

let parse ~verbose token lexbuf =
  let rec aux parser =
    let tok = token lexbuf in
    let tok = (tok, lexbuf.Lexing.lex_start_p, lexbuf.Lexing.lex_curr_p) in
    match progress tok parser with
    | Done -> (* No continuation: parsing failed,
                 FIXME: produce error message *)
      if verbose then (
        let parsers = get_parsers parser in
        print_endline ("Status: " ^ String.concat "; " (List.map dump_parser parsers));
      );
      raise Phparser.Error
    | parser' -> aux parser'
  in
  try
    aux (Normal { state = Phparser.Incremental.start lexbuf.Lexing.lex_start_p;
                  rest = Done })
  with
  | Accept -> ()
