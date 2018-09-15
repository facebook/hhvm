(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

{
open Lexing
open Hhas_parser

exception SyntaxError of string

let next_line lexbuf =
  let pos = lexbuf.lex_curr_p in
  lexbuf.lex_curr_p <-
    { pos with pos_bol = lexbuf.lex_curr_pos;
               pos_lnum = pos.pos_lnum + 1
    }

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

}

let int = '-'? ['0'-'9'] ['0'-'9']*
let octaldigit=['0'-'7']
let octaldigits = octaldigit octaldigit octaldigit
let digit = ['0'-'9']
let frac = '.' digit*
let exp = ['e' 'E'] ['-' '+']? digit+
let float = '-'? ((digit+ frac? exp?) | frac exp?)

let white = [' ' '\t']+
let newline = '\r' | '\n' | "\r\n"
let sillyend = ';' digit+
let inoutend = '$' ['0' - '9' ';']+ '$' "inout"
let id = '?'? (digit* ['a'-'z' 'A'-'Z' '_']
     (['a'-'z' 'A'-'Z' '0'-'9' '_' '\\' '$' '#' '@' '\x7f'-'\xff' ] | "::")* inoutend? sillyend?)
let vname = (['\x21'-'\xff'] # [';' ')' ','])*
let escapequote = "\\\""
let comment = '#' [^ '\r' '\n']* newline
let nonquote = [^ '"']
let quote = '"'
let nontriple = (nonquote | quote nonquote | quote quote nonquote)* quote? quote?
let triplequoted = quote quote quote nontriple quote quote quote
let doccomment = ".doc" white triplequoted ';'
let assertconstraint = id '<'? '=' id

rule read =
  parse
  | white               {read lexbuf}
  | comment             {read lexbuf}
  | doccomment          {read lexbuf}
  | ".function"         {FUNCTIONDIRECTIVE}
  | ".main"             {MAINDIRECTIVE}
  | ".class"            {CLASSDIRECTIVE}
  | ".includes"         {read_paths (Buffer.create 300) lexbuf}
  | ".constant_refs"    {CONSTANTREFSDIRECTIVE}
  | ".function_refs"    {FUNCTIONREFSDIRECTIVE}
  | ".class_refs"       {CLASSREFSDIRECTIVE}
  | ".declvars"         {DECLVARSDIRECTIVE}
  | ".ismemoizewrapper" {ISMEMOIZEWRAPPERDIRECTIVE}
  | ".ismemoizewrapperlsb" {ISMEMOIZEWRAPPERLSBDIRECTIVE}
  | ".adata"            {DATADECLDIRECTIVE}
  | ".numiters"         {NUMITERSDIRECTIVE}
  | ".method"           {METHODDIRECTIVE}
  | ".const"            {CONSTDIRECTIVE}
  | ".enum_ty"          {ENUMTYDIRECTIVE}
  | ".use"              {USESDIRECTIVE}
  | ".numclsrefslots"   {NUMCLSREFSLOTSDIRECTIVE}
  | ".try"              {TRYDIRECTIVE}
  | ".catch"            {CATCHDIRECTIVE}
  | ".try_fault"        {TRYFAULTDIRECTIVE}
  | ".try_catch"        {TRYCATCHDIRECTIVE}
  | ".property"         {PROPERTYDIRECTIVE}
  | ".filepath"         {FILEPATHDIRECTIVE}
  | ".alias"            {ALIASDIRECTIVE}
  | ".strict"           {STRICTDIRECTIVE}
  | ".hh_file"          {HHFILE}
  | ".static"           {STATICDIRECTIVE}
  | ".require"          {REQUIREDIRECTIVE}
  | ".srcloc"           {SRCLOCDIRECTIVE}
  | assertconstraint    {ASSERTCONSTRAINT (Lexing.lexeme lexbuf)}
  | ".metadata"         {METADATADIRECTIVE}
  | id                  {ID (Lexing.lexeme lexbuf)}
  | triplequoted as lxm {TRIPLEQUOTEDSTRING (String.sub lxm 3 (String.length lxm - 6))}
  | escapequote         {read_php_escaped_string (Buffer.create 17) lexbuf}
  | '"'                 { read_string (Buffer.create 17) lexbuf}
  | newline             {NEWLINE}
  | '<'                 {LANGLE}
  | '>'                 {RANGLE}
  | int as lxm          {INT (Int64.of_string lxm)}
  | float as lxm        {DOUBLE lxm}
  | "..."               {DOTDOTDOT}
  | '@'                 {AT}
  | '_'                 {UNDERSCORE}
  | '{'                 {LBRACE}
  | '}'                 {RBRACE}
  | '('                 {LPAR}
  | ')'                 {RPAR}
  | '['                 {LBRACK}
  | ']'                 {RBRACK}
  | ';'                 {SEMI}
  | ':'                 {COLON}
  | ','                 {COMMA}
  | '$'                 {read_variable_name (Buffer.create 17) lexbuf}
  | '='                 {EQUALS}
  | '&'                 {AMPERSAND}
  | '+'                 {PLUS}
  | '-'                 {MINUS}
  | eof                 {EOF}
  | _                   {raise (SyntaxError "read")}
and read_variable_name buf =
  parse
  | vname       {VNAME (Lexing.lexeme lexbuf)}
  | _           {raise (SyntaxError "read_variable_name")}
and read_php_escaped_string buf =
  parse
  | escapequote  {ESCAPEDSTRING (Buffer.contents buf)}
  | [^ '"' '\\']+
    {Buffer.add_string buf (Lexing.lexeme lexbuf); read_php_escaped_string buf lexbuf
    (* TODO: work out what's really supposed to happen with escaping here!
      Maybe need to store the last integer seen in a ref and use that as the length
    counter to decide how to lex escaped strings. Yuck. *)}
and read_string buf =
  parse
  | '"'          {STRING (Buffer.contents buf)}
  | '\\' '"'     {Buffer.add_char buf '"'; read_string buf lexbuf}
  | '\\' '\\'    {Buffer.add_char buf '\\'; read_string buf lexbuf}
  | '\\' 'n'    {Buffer.add_char buf '\n'; read_string buf lexbuf}
  | '\\' 'r'    {Buffer.add_char buf '\r'; read_string buf lexbuf}
  | '\\' 't'    {Buffer.add_char buf '\t'; read_string buf lexbuf}
  | '\\' '?'    {Buffer.add_char buf '?'; read_string buf lexbuf}
  | '\\' octaldigits as lxm {Buffer.add_char buf
    (char_of_int (int_of_string ("0o" ^
                                 (String.sub lxm 1 (String.length lxm - 1)))));
              read_string buf lexbuf}
  | [^ '"' '\\']+
     {Buffer.add_string buf (Lexing.lexeme lexbuf); read_string buf lexbuf}
and read_paths buf =
  parse
  | '{'  {read_paths buf lexbuf}
  | '}'  {INCLUDESDIRECTIVE (Buffer.contents buf)}
  | [^ '}'] as lxm {Buffer.add_char buf lxm; read_paths buf lexbuf}
