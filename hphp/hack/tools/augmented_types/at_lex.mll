{
(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let yyback n lexbuf =
  Lexing.(
  lexbuf.lex_curr_pos <- lexbuf.lex_curr_pos - n;
  let currp = lexbuf.lex_curr_p in
  lexbuf.lex_curr_p <-
    { currp with pos_cnum = currp.pos_cnum - n }
 )

let back lb =
  let n = Lexing.lexeme_end lb - Lexing.lexeme_start lb in
  yyback n lb

type token =
  | Tstar
  | Tor
  | Tbrackets
  | Tlp
  | Trp
  | Tident
  | Teof
  | Terror
}

(* Not technically the syntax of PHP identifiers, but close enough. *)
let ident = ['0'-'9''a'-'z''A'-'Z''_''-''\\'':']+

rule token = parse
  | "*"          { Tstar }
  | "|"          { Tor }
  | "[]"         { Tbrackets }
  | "("          { Tlp }
  | ")"          { Trp }
  | ident        { Tident }
  | eof          { Teof }
  | _            { Terror }
