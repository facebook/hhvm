{
(* Yoann Padioleau
 *
 * Copyright (C) 2009-2012 Facebook
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation, with the
 * special exception on linking described in file license.txt.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file
 * license.txt for more details.
 *)

open Phparser

(* Lexer State
 * ===========
 * In most languages the lexer has no state and all strings are always
 * encoded in the same way, in the same token, wherever the string is
 * located in the file (except for strings inside comments). In PHP
 * some characters, e.g. "'", as in "I don't like you" or "'foo'" can
 * mean different things. Indeed the PHP language in fact supports
 * multiple languages or "modes" inside the same script (which also
 * make emacs mode for such language harder to define).
 *
 * Inside the PHP script code part, the quote is the start of a string
 * and there must be a corresponding quote ending the string. Inside
 * the HTML part of a PHP file it's just a character like any other
 * character. Inside heredocs (the '<<<XXX' construct) it is again
 * considered as any other character. In the same way some strings such
 * as 'if' can again mean different things; when they are preceded by a
 * '->' they correspond to the possible name of a field, otherwise
 * they are special PHP keywords.
 *
 * Because all of this, the lexer has multiple states which are
 * represented below and adjusted via some push/pop_mode function
 * below. Depending on the state the lexer behaves differently.
 *)

type state_mode =
  (* aka HTML mode *)
  | ST_INITIAL
  (* started with <?php or <?, finished by ?> *)
  | ST_IN_SCRIPTING
  (* started with <?=, finished by ?> *)
  | ST_IN_SCRIPTING2
  (* handled by using ocamllex ability to define multiple lexers
   * ST_COMMENT | ST_DOC_COMMENT | ST_ONE_LINE_COMMENT *)
  (* started with ", finished with ". In most languages strings
   * are a single tokens but PHP allow interpolation which means
   * a string can contain nested PHP variables or expressions.
   *)
  | ST_DOUBLEQUOTE
  (* started with "`", finished with "`" *)
  | ST_BACKQUOTE
  (* started with ->, finished after reading one fieldname *)
  | ST_LOOKING_FOR_PROPERTY
  (* started with ${ *)
  | ST_LOOKING_FOR_VARNAME
  (* started with $xxx[ *)
  | ST_VAR_OFFSET
  (* started with <<<XXX, finished by XXX; *)
  | ST_START_HEREDOC of string
  (* started with <<<'XXX', finished by XXX; *)
  | ST_START_NOWDOC of string
  (* started with <xx when preceded by a certain token (e.g. 'return' '<xx'),
   * finished by '>' by transiting to ST_IN_XHP_TEXT, or really finished
   * by '/>'.  *)
  | ST_IN_XHP_TAG of string list (* the current tag, e,g, ["x";"frag"] *)
  (* started with the '>' of an opening tag, finished when '</x>' *)
  | ST_IN_XHP_TEXT of string list (* the current tag *)

(* Prelude
 * =======
 * The PHP lexer.
 *
 * There are a few tricks to go around ocamllex restrictions
 * because PHP has different lexing rules depending on some "contexts"
 * (this is similar to Perl, e.g. the <<<END context).
 *)

type state = {
  strict_lexer: bool;
  verbose_lexer: bool;
  case_sensitive: bool;
  xhp_builtin: bool;
  facebook_lang_extensions: bool;
  mutable last_non_whitespace_like_token: token;
  mutable mode_stack : state_mode list;
  mutable lexeme_stack_resume_pos : int;
  mutable lexeme_stack : (token * int * int) list;
}

let return st lexbuf (token, startp, endp) =
  assert (st.lexeme_stack_resume_pos = -1);
  st.lexeme_stack_resume_pos <- lexbuf.Lexing.lex_curr_pos;
  lexbuf.Lexing.lex_start_pos <- startp;
  lexbuf.Lexing.lex_curr_pos <- endp;
  token

let return_many st lexbuf = function
  | [] -> invalid_arg "return_many: empty list"
  | x :: xs ->
    st.lexeme_stack <- xs;
    return st lexbuf x

let error_lexeme () lexbuf = Lexing.lexeme lexbuf

let join_tag () tags = String.concat ":" tags

let lexeme lexbuf ?(s=0) ?(e=0) token =
  let start_pos = lexbuf.Lexing.lex_start_pos in
  let curr_pos = lexbuf.Lexing.lex_curr_pos in
  let startp = if s >= 0 then start_pos + s else curr_pos + s in
  let endp = if e <= 0 then curr_pos + e else start_pos + e in
  (token, startp, endp)

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

exception Error of string

let error st fmt =
  let strict_k s = raise (Error s) in
  let verbose_k s = prerr_endline ("LEXER: " ^ s) in
  if st.strict_lexer then
    Printf.ksprintf strict_k fmt
  else if st.verbose_lexer then
    Printf.ksprintf verbose_k fmt
  else
    Printf.ifprintf () fmt

let unlex_characters buf n = (
  let open Lexing in
  if n < 0 then
    invalid_arg "unlex_characters: offset should be positive";
  if n > buf.lex_curr_pos - buf.lex_start_pos then
    invalid_arg "unlex_characters: offset larger than lexeme";
  buf.lex_curr_pos <- buf.lex_curr_pos - n;
  if buf.lex_curr_p != Lexing.dummy_pos then
    buf.lex_curr_p <- {buf.lex_start_p
                       with pos_cnum = buf.lex_abs_pos + buf.lex_curr_pos};
)

let set_lexeme_length buf n = (
  let open Lexing in
  if n < 0 then
    invalid_arg "set_lexeme_length: offset should be positive";
  if n > buf.lex_curr_pos - buf.lex_start_pos then
    invalid_arg "set_lexeme_length: offset larger than lexeme";
  buf.lex_curr_pos <- buf.lex_start_pos + n;
  if buf.lex_curr_p != Lexing.dummy_pos then
    buf.lex_curr_p <- {buf.lex_start_p
                       with pos_cnum = buf.lex_abs_pos + buf.lex_curr_pos};
)

let string_split char str =
  let rec loop str from char =
    match String.index_from str from char with
    | exception Not_found ->
      let len = String.length str in
      [String.sub str from (len - from)]
    | delim ->
      let s = String.sub str from (delim - from) in
      s :: loop str (delim + 1) char
  in
  match String.index str char with
  | exception Not_found -> [str]
  | delim ->
    let s = String.sub str 0 delim in
    s :: loop str (delim + 1) char

(* all string passed to IDENT or VARIABLE should go through case_str *)

(* An implementation of String.lowercase that does not allocate
 * if not necessary. *)

let lowercase = (
  let rec loop bytes len i =
    if i < len then
      let c = Bytes.unsafe_get bytes i in
      Bytes.unsafe_set bytes i (Char.lowercase c);
      loop bytes len (i + 1)
    else
      Bytes.unsafe_to_string bytes
  in
  let rec check str len i =
    if i < len then
      let c = String.unsafe_get str i in
      if c = Char.lowercase c then
        check str len (i + 1)
      else
        loop (Bytes.of_string str) len i
    else
      str
  in
  fun str -> check str (String.length str) 0
) [@ocaml.warning "-3"]

let case_str st lexeme =
  if st.case_sensitive
  then lexeme
  else lowercase lexeme

let xhp_or_t_ident st ~f lexeme =
  if st.xhp_builtin
  then f lexeme
  else IDENT (case_str st lexeme)

let lang_ext_or_t_ident st ~f lexeme =
  if st.facebook_lang_extensions
  then f lexeme
  else IDENT (case_str st lexeme)

(* Keywords
 * ========
 * opti: less convenient, but using a hash is faster than using a match.
 * Note that PHP allows those keywords to be used in certain places,
 * for instance as object fields as in $o->while, so the transformation
 * from a LABEL to those keywords is done only in a few cases.
 *
 * note: PHP is case insensitive so this hash table is used on
 * a lowercased string so don't put strings in uppercase below because
 * such keyword would never be reached!
 *
 * coupling: if you add a new keyword, don't forget to also modify
 * the xhp_attr_name_atom grammar rule in parser_php.mly
 *
 * http://php.net/manual/en/reserved.keywords.php
 *
 * todo: callable, goto
 *)

type keyword_kind =
  | Lang_Normal
  | Lang_XHP
  | Lang_Facebook
  | Lang_FB_sensitive

let keyword_table =
  let of_list l =
    let table = Hashtbl.create (List.length l) in
    List.iter (fun (k, v) ->
      assert (k = String.lowercase_ascii k);
      Hashtbl.add table k v;
    ) l;
    table
  in
  of_list
  [ "while"      , (Lang_Normal, WHILE)
  ; "do"         , (Lang_Normal, DO)
  ; "for"        , (Lang_Normal, FOR)
  ; "foreach"    , (Lang_Normal, FOREACH)
  (* ; "endwhile"   , (Lang_Normal, ENDWHILE)
     ; "endfor"     , (Lang_Normal, ENDFOR)
     ; "endforeach" , (Lang_Normal, ENDFOREACH) *)

  (* Those tokens were not in the original PHP lexer. This allowed to
   * have "self"/"parent" to be used at more places, e.g. as a function
   * name which is tolerated by PHP but should not IMHO. Those idents
   * have a special meaning and this should be reflected in the lexer,
   * especially since PHP 5.3 which allows static:: in addition to
   * self::, parent::. 'static' is a keyword so there is no reason
   * to not make self/parent keywords too.
   *
   * todo: should do something similar for $this.
   *)
  ; "self"         , (Lang_Normal, SELF)
  ; "parent"       , (Lang_Normal, PARENT)
  ; "if"           , (Lang_Normal, IF)
  ; "else"         , (Lang_Normal, ELSE)
  ; "elseif"       , (Lang_Normal, ELSEIF)
  ; "break"        , (Lang_Normal, BREAK)
  ; "continue"     , (Lang_Normal, CONTINUE)
  ; "switch"       , (Lang_Normal, SWITCH)
  ; "case"         , (Lang_Normal, CASE)
  ; "default"      , (Lang_Normal, DEFAULT)
  ; "return"       , (Lang_Normal, RETURN)
  ; "try"          , (Lang_Normal, TRY)
  ; "catch"        , (Lang_Normal, CATCH)
  ; "finally"      , (Lang_Normal, FINALLY)
  ; "throw"        , (Lang_Normal, THROW)
  ; "exit"         , (Lang_Normal, EXIT)
  ; "array"        , (Lang_FB_sensitive, ARRAY)
  ; "darray"       , (Lang_FB_sensitive, ARRAY)
  ; "varray"       , (Lang_FB_sensitive, ARRAY)
  ; "vec"          , (Lang_FB_sensitive, ARRAY)
  ; "dict"         , (Lang_FB_sensitive, ARRAY)
  ; "keyset"       , (Lang_FB_sensitive, ARRAY)
  ; "list"         , (Lang_FB_sensitive, LIST)

  (* used for traits too *)
  ; "as"           , (Lang_Normal, AS)
  ; "include"      , (Lang_Normal, INCLUDE)
  ; "include_once" , (Lang_Normal, INCLUDE_ONCE)
  ; "require"      , (Lang_Normal, REQUIRE)
  ; "require_once" , (Lang_Normal, REQUIRE_ONCE)
  ; "class"        , (Lang_Normal, CLASS)
  ; "interface"    , (Lang_Normal, INTERFACE)
  ; "extends"      , (Lang_Normal, EXTENDS)
  ; "implements"   , (Lang_Normal, IMPLEMENTS)
  ; "new"          , (Lang_Normal, NEW)
  ; "clone"        , (Lang_Normal, CLONE)
  ; "instanceof"   , (Lang_Normal, INSTANCEOF)

  (* php 5.4 traits ('use' and 'as' are used for traits and other things) *)
  ; "trait"     , (Lang_Normal, TRAIT)
  ; "insteadof" , (Lang_Normal, INSTEADOF)

  (* php 5.3 namespace *)
  ; "namespace" , (Lang_Normal, NAMESPACE)

  (* used for traits and namespace *)
  ; "use"           , (Lang_Normal, USE)
  ; "abstract"      , (Lang_Normal, ABSTRACT)
  ; "final"         , (Lang_Normal, FINAL)
  ; "public"        , (Lang_Normal, PUBLIC)
  ; "protected"     , (Lang_Normal, PROTECTED)
  ; "private"       , (Lang_Normal, PRIVATE)
  ; "echo"          , (Lang_Normal, ECHO)
  ; "print"         , (Lang_Normal, PRINT)
  ; "eval"          , (Lang_Normal, EVAL)
  ; "global"        , (Lang_Normal, GLOBAL)
  ; "function"      , (Lang_Normal, FUNCTION)
  ; "empty"         , (Lang_Normal, EMPTY)
  ; "const"         , (Lang_Normal, CONST)
  ; "var"           , (Lang_Normal, VAR)
  ; "declare"       , (Lang_Normal, DECLARE)
  ; "static"        , (Lang_Normal, STATIC)
  ; "unset"         , (Lang_Normal, UNSET)
  ; "isset"         , (Lang_Normal, ISSET)
  ; "__line__"      , (Lang_Normal, LINE)
  ; "__file__"      , (Lang_Normal, FILE)
  ; "__dir__"       , (Lang_Normal, DIR)
  ; "__function__"  , (Lang_Normal, FUNC_C)
  ; "__method__"    , (Lang_Normal, METHOD_C)
  ; "__class__"     , (Lang_Normal, CLASS_C)
  ; " __trait__"    , (Lang_Normal, TRAIT_C)
  ; "__namespace__" , (Lang_Normal, NAMESPACE_C) (* was called NS_C *)
  (* ; "endif"      , (Lang_Normal, ENDIF)
     ; "endswitch"  , (Lang_Normal, ENDSWITCH)
     ; "enddeclare" , (Lang_Normal, ENDDECLARE); *)

  (* old: ; "__halt_compiler", HALT_COMPILER; *)
  ; "async"         , (Lang_FB_sensitive, ASYNC)
  ; "super"         , (Lang_FB_sensitive, SUPER)
  ; "coroutine"     , (Lang_FB_sensitive, COROUTINE)
  ; "suspend"       , (Lang_FB_sensitive, SUSPEND)

  (* php-facebook-ext: *)
  ; "yield"         , (Lang_Facebook, YIELD)
  ; "await"         , (Lang_Facebook, AWAIT)
  ; "using"         , (Lang_Facebook, USING)
  ; "concurrent"    , (Lang_FB_sensitive, CONCURRENT)

  (* php-facebook-ext: *)
  ; "type"          , (Lang_FB_sensitive, TYPE)
  ; "newtype"       , (Lang_Facebook, NEWTYPE)
  ; "shape"         , (Lang_FB_sensitive, SHAPE)
  ; "is"            , (Lang_Facebook, IS)
  ; "inout"         , (Lang_Facebook, INOUT)
  ; "where"         , (Lang_FB_sensitive, WHERE)

  (* xhp: having those XHP keywords handled here could mean they can not
   * be used for entities like functions or class names. We could
   * avoid this by introducing some lexer/parser hacks so that those
   * keywords are recognized only in certain contexts (e.g. just after
   * the '{' of a class) but that complicates the full parser (note
   * also that IMHO it's better to not let the user overload those
   * special names). A simpler solution, instead of extending the lexer,
   * is to extend the grammar by having a 'ident:' rule that allows
   * the regular IDENT as well as those XHP tokens. See parser_php.mly.
   *)
  ; "attribute" , (Lang_XHP, XHP_ATTRIBUTE)
  ; "children"  , (Lang_XHP, XHP_CHILDREN)
  ; "category"  , (Lang_XHP, XHP_CATEGORY)
  (* for attribute declarations and Hack first class enums *)
  ; "enum"      , (Lang_FB_sensitive, ENUM)
  (* for children declarations *)
  ; "any"       , (Lang_XHP, XHP_ANY)
  (* "empty" is already a PHP keyword, see EMPTY *)
  ; "pcdata"    , (Lang_XHP, XHP_PCDATA)

  (* obsolete: now that use hphp instead of xdebug for coverage analysis
   ; "class_xdebug",    (fun ii -> CLASS_XDEBUG ii)
   ; "resource_xdebug", (fun ii -> RESOURCE_XDEBUG ii)
   *)
  ]

let ident_or_keyword st lexeme =
  let lexeme' = case_str st lexeme in
  match Hashtbl.find keyword_table lexeme' with
  | Lang_Normal,   token -> token
  | Lang_XHP,      token when st.xhp_builtin && lexeme = lexeme' -> token
  | Lang_Facebook, token when st.facebook_lang_extensions -> token
  | Lang_FB_sensitive, token
    when st.facebook_lang_extensions && lexeme = lexeme' -> token
  | _ -> IDENT lexeme
  | exception Not_found -> IDENT lexeme

(* The logic to modify _last_non_whitespace_like_token is in the
 * caller of the lexer, that is in Parse_php.tokens.
 * Used for XHP.
 *)

let last_non_whitespace_like_token st =
  st.last_non_whitespace_like_token

let new_state ~strict_lexer ~verbose_lexer ~case_sensitive ~xhp_builtin
              ~facebook_lang_extensions () =
  { strict_lexer; verbose_lexer; case_sensitive; xhp_builtin;
    facebook_lang_extensions; mode_stack = [ST_INITIAL];
    last_non_whitespace_like_token = EOF;
    lexeme_stack_resume_pos = -1;
    lexeme_stack = [];
  }

let current_mode st =
  match st.mode_stack with
  | mode :: _ -> mode
  | [] ->
    error st "mode_stack is empty, defaulting to ST_INITIAL";
    st.mode_stack <- [ST_INITIAL];
    ST_INITIAL

let push_mode st mode =
  st.mode_stack <- mode :: st.mode_stack

let pop_mode st =
  match st.mode_stack with
  | [] -> ()
  | x :: xs -> st.mode_stack <- xs

let set_mode st mode =
  let (_ :: xs | ([] as xs)) = st.mode_stack in
  st.mode_stack <- mode :: xs

let protect_start_pos fn st lexbuf =
  let start_pos = lexbuf.Lexing.lex_start_pos in
  let result = fn st lexbuf in
  lexbuf.Lexing.lex_start_pos <- start_pos;
  result

(* Here is an example of state transition. Given a php file like:
 *
 *   <?php return <x>foo<y>bar</y></x>; ?>
 *
 * we start with the stack in [ST_INITIAL]. The transitions are then:
 *
 * '<?php'  -> [IN_SCRIPTING], via set_mode()
 * ' '      -> [IN_SCRIPTING]
 * 'return' -> [IN_SCRIPTING]
 * '<x'     -> [IN_XHP_TAG "x"; IN_SCRIPTING], via push_mode()
 * '>'      -> [IN_XHP_TEXT "x"; IN_SCRIPTING], via set_mode()
 * 'foo'    -> [IN_XHP_TEXT "x"; IN_SCRIPTING]
 * '<y'     -> [IN_XHP_TAG "y";IN_XHP_TEXT "x"; IN_SCRIPTING], via push_mode()
 * '>'      -> [IN_XHP_TEXT "y"; IN_XHP_TEXT "x";IN_SCRIPTING], via set_mode()
 * 'bar'    -> [IN_XHP_TEXT "y"; IN_XHP_TEXT "x"; IN_SCRIPTING]
 * '</y>'   -> [IN_XHP_TEXT "x"; IN_SCRIPTING], via pop_mode()
 * '</x>'   -> [IN_SCRIPTING], via pop_mode()
 * ';'      -> [IN_SCRIPTING]
 * ' '      -> [IN_SCRIPTING]
 * '?>      -> [INITIAL], via set_mode()
 *
 *)

(* xhp: the function below is used to disambiguate the use
 * of ":" and "%" as either a way to start an XHP identifier or as
 * a binary operator. Note that we use a whitelist approach
 * for detecting ':' as a binary operator whereas HPHP and
 * XHPAST use a whitelist approach for detecting ':' as the
 * start of an XHP identifier.
 *
 * How to know the following lists of tokens is correct ?
 * We should compute FOLLOW(lexeme) for  all tokens and check
 * if "%" or ":" can be in it ?
 *)
let is_in_binary_operator_position st =
  match st.last_non_whitespace_like_token with
    (* if we are after a number or any kind of scalar, then it's ok to have a
     * binary operator *)
  | LNUMBER _ | DNUMBER _ | CONSTANT_ENCAPSED_STRING _
  | DOUBLEQUOTE | BACKQUOTE
    (* same for ']' or ')'; anything that "terminates" an expression *)
  | RBRACKET | RPAREN
  | IDENT _ | VARIABLE _ -> true
  | _ -> false

(* ugly: in code like 'function foo( (function(string):string) $callback){}'
 * we want to parse the '(string)' not as a STRING_CAST but
 * as an open paren followed by other tokens. The right fix would
 * be to not have those ugly lexing rules for cast, but this would
 * lead to some grammar ambiguities or require other parsing hacks anyway.
 *)
let lang_ext_or_cast st lexbuf token =
  if st.facebook_lang_extensions then
    match st.last_non_whitespace_like_token with
    | FUNCTION ->
      (* just keep the open parenthesis *)
      set_lexeme_length lexbuf 1;
      LPAREN
    | _ -> token
  else token

}

(*****************************************************************************)
(* Regexps aliases *)
(*****************************************************************************)
let ANY_CHAR = (_ | ['\n'] )
(* \x7f-\xff ???*)
let WHITESPACE = [' ' '\n' '\r' '\t']
let TABS_AND_SPACES = [' ''\t']*
let NEWLINE = ("\r"|"\n"|"\r\n")
let LABEL0 = ['a'-'z''A'-'Z''_']
let LABEL = LABEL0 ['a'-'z''A'-'Z''0'-'9''_']*
let LNUM = ['0'-'9']+
let DNUM = (['0'-'9']*['.']['0'-'9']+) | (['0'-'9']+['.']['0'-'9']* )

let EXPONENT_DNUM = ((LNUM|DNUM)['e''E']['+''-']?LNUM)
let HEXNUM = ("0x" | "0X")['0'-'9''a'-'f''A'-'F']+
let BINNUM = "0b"['0'-'1']+
(*/*
 * LITERAL_DOLLAR matches unescaped $ that aren't followed by a label character
 * or a { and therefore will be taken literally. The case of literal $ before
 * a variable or "${" is handled in a rule for each string type
 *
 * TODO: \x7f-\xff
 */
 *)
let DOUBLE_QUOTES_LITERAL_DOLLAR =
  ("$"+([^'a'-'z''A'-'Z''_''$''"''\\' '{']|('\\' ANY_CHAR)))
let BACKQUOTE_LITERAL_DOLLAR =
  ("$"+([^'a'-'z''A'-'Z''_''$''`''\\' '{']|('\\' ANY_CHAR)))
(*/*
 * CHARS matches everything up to a variable or "{$"
 * {'s are matched as long as they aren't followed by a $
 * The case of { before "{$" is handled in a rule for each string type
 *
 * For heredocs, matching continues across/after newlines if/when it's known
 * that the next line doesn't contain a possible ending label
 */
 *)
let DOUBLE_QUOTES_CHARS =
  ("{"*([^'$''"''\\''{']|
    ("\\" ANY_CHAR))| DOUBLE_QUOTES_LITERAL_DOLLAR)
let BACKQUOTE_CHARS =
  ("{"*([^'$' '`' '\\' '{']|('\\' ANY_CHAR))| BACKQUOTE_LITERAL_DOLLAR)
let XHPLABEL = LABEL
let XHPTAG = XHPLABEL ([':''-'] XHPLABEL)*
let XHPATTR = XHPTAG

(*****************************************************************************)
(* Rule in script *)
(*****************************************************************************)
rule st_in_scripting st = parse

(* ----------------------------------------------------------------------- *)
(* spacing/comments *)
(* ----------------------------------------------------------------------- *)
  | "/*" { protect_start_pos st_comment st lexbuf; COMMENT }

  | "/**/" { COMMENT }

  | "/**" { protect_start_pos st_comment st lexbuf; DOC_COMMENT }

  | ("#" | "//") { protect_start_pos st_one_line_comment st lexbuf; COMMENT }

  (* old: | WHITESPACE { WHITESPACE } *)

  | [' '  '\t']+ { SPACES }
  | ['\n' '\r']  { NEWLINE }

(* ----------------------------------------------------------------------- *)
(* Symbols *)
(* ----------------------------------------------------------------------- *)
  | '+'   { PLUS }
  | '-'   { MINUS }
  | '*'   { MUL }
  | '/'   { DIV }
  | '%'   { MOD }
  | "++"  { INC }
  | "--"  { DEC }
  | "="   { EQUAL }
  | "+="  { PLUS_EQUAL }
  | "-="  { MINUS_EQUAL }
  | "*="  { MUL_EQUAL }
  | "/="  { DIV_EQUAL }
  | "%="  { MOD_EQUAL }
  | "&="  { AND_EQUAL }
  | "|="  { OR_EQUAL }
  | "^="  { XOR_EQUAL }
  | "<<=" { SHL_EQUAL }
  | ">>=" { SHR_EQUAL }
  | ".="  { CONCAT_EQUAL }
  | "=="  { IS_EQUAL }
  | "!="  { IS_NOT_EQUAL }
  | "===" { IS_IDENTICAL }
  | "!==" { IS_NOT_IDENTICAL }
  | "<>"  { IS_NOT_EQUAL }
  | "<=>" { SPACESHIP }
  | "<="  { IS_SMALLER_OR_EQUAL }
  | ">="  { IS_GREATER_OR_EQUAL }
  | "<"   { LANGLE }
  | ">"   { RANGLE }
  | "&&"  { BOOLEAN_AND }
  | "||"  { BOOLEAN_OR }
  | "|>"  { PIPE_ANGLE }
  | "<<"  { SHL }
  | ">>"  { set_lexeme_length lexbuf 1; SHR_PREFIX }
  | "**"  { POW }
  | "**=" { POW_EQUAL }
  | "&"   { AND }
  | "|"   { OR }
  | "^"   { XOR }
  | "OR"  { LOGICAL_OR }
  | "AND" { LOGICAL_AND }
  | "XOR" { LOGICAL_XOR }
  | "or"  { LOGICAL_OR }
  | "and" { LOGICAL_AND }
  | "xor" { LOGICAL_XOR }

 (* Flex/Bison allow to use single characters directly as-is in the grammar
  * by adding this in the lexer:
  *
  *       <ST_IN_SCRIPTING>{TOKENS} { return yytext[0];}
  *
  * We don't, so we have transformed all those tokens in proper tokens with
  * a name in the parser, and return them in the lexer.
  *)

  | '.'  { CONCAT }
  | ','  { COMMA }
  | '@'  { AT }

  (* was called DOUBLE_ARROW but we actually now have a real ==> *)
  | "=>" { DOUBLE_ARROW }
  | "~"  { TILDE }
  | ";"  { SEMICOLON }
  | "!"  { BANG }
  | "::" { COLONCOLON } (* was called T_PAAMAYIM_NEKUDOTAYIM *)
  | "\\" { ANTISLASH  } (* was called T_NS_SEPARATOR *)

  | '(' { LPAREN }
  | ')' { RPAREN }

  | '[' { LBRACKET }
  | ']' { RBRACKET }

  | ":" { COLON }
  | "?" { QUESTION }
  | ":" { COLON }
  | "??" { QUESTION_QUESTION }
  | "??=" { QUESTION_QUESTION_EQUAL }

  (* semantic grep or var args extension *)
  | "..." { ELLIPSIS }

  (* facebook-ext: short lambdas *)
  | "==>" { LONG_DOUBLE_ARROW }

  (* we may come from a st_looking_for_xxx context, like in string
   * interpolation, so seeing a } we pop_mode!  *)
  | '}'
    { pop_mode st; RBRACE }
  | '{'
    { push_mode st ST_IN_SCRIPTING; LBRACE }
  (*| (("->" | "?->") as sym) (WHITESPACE* as white) (LABEL as label)
    { (* todo: use yyback() instead of using pending_token with push_token.
       * buggy: push_mode st ST_LOOKING_FOR_PROPERTY; *)
      push_token st (IDENT (case_str st label, lblinfo));
     (* todo: could be newline ... *)
      push_token st (SPACES (whiteinfo));
      OBJECT_OPERATOR(syminfo)
    }*)
  | "->" { ARROW }
  | "?->" { QUESTION_ARROW }

  (* see also VARIABLE below. lex use longest matching strings so this
   * rule is used only in a last resort, for code such as $$x, ${, etc
   *)
  | "$" { DOLLAR }
  | "$$" { DOLLARDOLLAR }

  (* XHP "elements".
   *
   * In XHP the ":" and "%" characters are used to identify
   * XHP tags, e.g. :x:frag. There is some possible ambiguity though
   * with their others use in PHP: ternary expr and cases for ":" and
   * the modulo binary operator for "%". It is legal in PHP to do
   * e?1:null; or case 1:null. We thus can not blindly considerate ':null'
   * as a single token. Fortunately it's not too hard
   * to disambiguate by looking at the token before and see if ":" or "%"
   * is used as a unary or binary operator.
   *
   * An alternative would be to return the same token in both cases
   * (TCOLON) and let the grammar disambiguate and build XHP tags
   * from multiple tokens (e.g. [TCOLON ":"; IDENT "x"; TCOLON ":";
   * TIDENT "frag"]). But this would force in the grammar to check
   * if there is no space between those tokens. This would also add
   * extra rules for things that really should be more handled at a
   * lexical level.
   *)
  | ":" (XHPTAG as tag)
    { if st.xhp_builtin && not (is_in_binary_operator_position st) then
        XHP_COLONID_DEF (string_split ':' tag)
      else
        (set_lexeme_length lexbuf 1; COLON)
    }

  | "%" (XHPTAG as tag)
    { if st.xhp_builtin && not (is_in_binary_operator_position st) then
        XHP_PERCENTID_DEF (string_split ':' tag)
      else
        (set_lexeme_length lexbuf 1; MOD)
    }

   (* xhp: we need to disambiguate the different use of '<' to know whether
    * we are in a position where an XHP construct can be started. Knowing
    * what was the previous token seems enough; no need to hack the
    * grammar to have a global shared by the lexer and parser.
    *
    * We could maybe even return a LANGLE in both cases and still
    * not generate any conflict in the grammar, but it feels cleaner to
    * generate a different token, because we will really change the lexing
    * mode when we will see a '>' which makes the parser enter in the
    * ST_IN_XHP_TEXT state where it's ok to write "I don't like you"
    * in which the quote does not need to be ended.
    *
    * note: no leading ":" for the tag when in "use" position.
    *)
    | "<" (XHPTAG as tag)
      { match st.last_non_whitespace_like_token with
          (* todo? How to compute the correct list of tokens that
           * are possibly before a XHP construct ? trial-and-error ?
           * Usually having a '<' after a punctuation means XHP.
           * Indeed '<' is a binary operator which excepts scalar.
           *
           * RPAREN? no, because it's ok to do (1) < (2)!
           *)
        | LPAREN | ECHO | PRINT | CLONE | SEMICOLON | COMMA
        | LBRACE | RBRACE | RETURN | YIELD | AWAIT | EQUAL | LBRACKET
        | CONCAT_EQUAL | DOUBLE_ARROW | LONG_DOUBLE_ARROW | QUESTION | COLON
        | QUESTION_QUESTION | QUESTION_QUESTION_EQUAL
        | PIPE_ANGLE | IS_NOT_IDENTICAL | IS_IDENTICAL
        | EOF (* when in sgrep/spatch mode, < is the first token,
                 and last_non_whitespace_like_token defaults to EOF *)
          when st.xhp_builtin ->
            let xs = string_split ':' tag in
            push_mode st (ST_IN_XHP_TAG xs);
            XHP_OPEN_TAG xs
        | _ ->
            set_lexeme_length lexbuf 1;
            LANGLE
      }

  | "@required"
    { if st.xhp_builtin then
        XHP_REQUIRED
      else
        (set_lexeme_length lexbuf 1; AT)
    }

  (* Keywords and ident
   * ------------------
   * ugly: 'self' and 'parent' should be keywords forbidden to be used
   * as regular identifiers. But PHP is case insensitive and does not
   * consider self/parent or SELF/PARENT as keywords. I think it's
   * bad so I now consider self/parent as keywords, but still allow
   * at least the uppercase form to be used as identifier, hence those
   * two rules below.
   *)
  | "SELF"   { IDENT (case_str st "SELF") }
  | "PARENT" { IDENT (case_str st "PARENT") }

  (* ugly: some code is using ASYNC as a constant, so one way to fix
   * the conflict is to return the ASYNC only when it's used
   * as lowercase. Note that because some code is using 'async'
   * as a method we then need to extend ident_method_name
   * in parser_php.mly. The alternative would be to lex
   * "async" as a ASYNC only when it's followed by a FUNCTION
   * but this is also ugly.
   *)
  | "async" { ASYNC }

  | LABEL
    { match st.last_non_whitespace_like_token with
      | ARROW | ANTISLASH (*| LONG_DOUBLE_ARROW*) ->
        IDENT (Lexing.lexeme lexbuf)
      | _ -> ident_or_keyword st (Lexing.lexeme lexbuf)
    }

  (* Could put a special rule for "$this", but there are multiple places here
   * where we can generate a VARIABLE, and we can have even expressions
   * like ${this}, so it is simpler to do the "this-analysis" in the grammar,
   * later when we generate a Var or This.
   *)
  | "$" (LABEL as s)
    { VARIABLE (case_str st s) }

  | "$$" LABEL0
    { set_lexeme_length lexbuf 1; DOLLAR }

  (* Constant
     -------- *)
  | LNUM | BINNUM | HEXNUM
    { (* more? cf original lexer *)
      let s = Lexing.lexeme lexbuf in
      match int_of_string s with
      | _ -> LNUMBER s
      | exception Failure _ -> DNUMBER s
    }
  | DNUM | EXPONENT_DNUM
    { DNUMBER (Lexing.lexeme lexbuf) }


  (* Strings
   * -------
   *
   * The original PHP lexer does a few things to make the
   * difference at parsing time between static strings (which do not
   * contain any interpolation) and dynamic strings. So some regexps
   * below are quite hard to understand ... but apparently it works.
   * When the lexer thinks it's a dynamic strings, it let the grammar
   * do most of the hard work. See the rules using DOUBLEQUOTE in the grammar
   * (and here in the lexer).
   *
   * The optional  'b' at the beginning is for binary strings.
   *
   * /*
   *   ("{"*|"$"* ) handles { or $ at the end of a string (or the entire
   *  contents)
   *
   *
   * int bprefix = (yytext[0] != '"') ? 1 : 0;
   * zend_scan_escape_string(zendlval, yytext+bprefix+1, yyleng-bprefix-2, '"'
   * TSRMLS_CC);
   */
   *)

  (* static strings *)
  | 'b'? (['"'] ((DOUBLE_QUOTES_CHARS* ("{"*|"$"* )) as s) ['"'])
      { CONSTANT_ENCAPSED_STRING s }

  | 'b'? (['\''] (([^'\'' '\\']|('\\' ANY_CHAR))* as s)  ['\''])
      { (* more? cf original lexer *)
        CONSTANT_ENCAPSED_STRING s }

  (* dynamic strings *)
  | '"'
    { push_mode st ST_DOUBLEQUOTE; DOUBLEQUOTE }
  | '`'
    { push_mode st ST_BACKQUOTE; BACKQUOTE }
  | 'b'? "<<<" TABS_AND_SPACES (LABEL as s) NEWLINE
    { set_mode st (ST_START_HEREDOC s); START_HEREDOC }

  | 'b'? "<<<" TABS_AND_SPACES "'" (LABEL as s) "'" NEWLINE
    { set_mode st (ST_START_NOWDOC s);
      (* could use another token, but simpler to reuse *)
      START_HEREDOC }
  | "re\"" (* regular expression literal *)
    { push_mode st ST_DOUBLEQUOTE; DOUBLEQUOTE }

  (* Misc
   * ----
   * ugly: the cast syntax in PHP is newline and even comment sensitive. Hmm.
   * You cannot write for instance '$o = (int/*comment*/) foo();'.
   * We would really like to have different tokens for '(', space,
   * idents, and a grammar rule like 'expr: LPAREN TIdent RPAREN'
   * but then the grammar would be ambiguous with 'expr: LPAREN expr RPAREN'
   * unless like in C typenames have a special token type and you can
   * have a rule like 'expr: LPAREN TTypename RPAREN.
   * This could have been done in PHP if those typenames were reserved
   * tokens, but PHP allows to have functions or methods called e.g.
   * string(). So what they have done is this ugly lexing hack.
   *)
  | "(" WHITESPACE* ("int"|"integer") WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf INT_CAST }
  | "(" WHITESPACE* ("real"|"double"|"float") WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf DOUBLE_CAST }
  | "(" WHITESPACE* "string" WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf STRING_CAST }
  | "(" WHITESPACE* "binary" WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf STRING_CAST }
  | "(" WHITESPACE* (['v' 'd']? "array"|"vec"|"dict"|"keyset") WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf ARRAY_CAST }
  | "(" WHITESPACE* "object" WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf OBJECT_CAST }
  | "(" WHITESPACE* ("bool"|"boolean") WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf BOOL_CAST }
  (* PHP is case insensitive for many things *)
  | "(" WHITESPACE* "Array" WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf ARRAY_CAST }
  | "(" WHITESPACE* "Object" WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf OBJECT_CAST }
  | "(" WHITESPACE* ("Bool"|"Boolean") WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf BOOL_CAST }
  | "(" WHITESPACE* ("unset") WHITESPACE* ")"
    { lang_ext_or_cast st lexbuf UNSET_CAST }
  | "?>"
    { (* because of XHP and my token merger:
         old: | "</script" WHITESPACE * ">" NEWLINE?
         see tests/xhp/pb_cant_merge2.php *)
      match current_mode st with
      | ST_IN_SCRIPTING ->
        set_mode st ST_INITIAL;
        (* implicit ';' at php-end tag
           todo? ugly, could instead generate a FakeToken or
           ExpandedToken, but then some code later may assume
           right now that all tokens from the lexer are
           origin tokens, so may be hard to change.

           old: (CLOSE_TAG)
           note that CLOSE_TAG was skipped anyway in Parse_php.parse_php
         *)
        CLOSE_TAG (*SEMICOLON*)
      | ST_IN_SCRIPTING2 ->
        set_mode st ST_INITIAL;
        CLOSE_TAG_OF_ECHO
      | _ -> assert false
    }

(* ----------------------------------------------------------------------- *)
  | eof { EOF }
  | _
    { error st "unrecognised symbol, in token rule: %a" error_lexeme lexbuf;
      UNKNOWN
    }

(*****************************************************************************)
(* Rule initial (html) *)
(*****************************************************************************)
and initial st = parse
  | "<?php" ([' ' '\t']|NEWLINE)
  (* php-facebook-ext: fbstrict extensions *)
  | "<?hh" ([' ' '\t']|NEWLINE)
     { (* I now do a yyback to not eat the newline which is more
          consistent with how I treat newlines elsewhere *)
       unlex_characters lexbuf 1;
       set_mode st ST_IN_SCRIPTING;
       OPEN_TAG
     }
  | "<?hh//"
     { unlex_characters lexbuf 2;
       set_mode st ST_IN_SCRIPTING;
       OPEN_TAG
     }

  | "<?PHP"([' ''\t'] | NEWLINE)
  | "<?Php"([' ''\t'] | NEWLINE)
      {
        (* "BAD USE OF <PHP at initial state, replace by <?php"; *)
        set_mode st ST_IN_SCRIPTING;
        OPEN_TAG
      }

  | ([^'<'] | "<" [^ '?' '%' 's' '<'])+ | "<s" | "<"
    { (* more? cf original lexer *)
      INLINE_HTML (Lexing.lexeme lexbuf) }

  | "<?="
    { (* less: if short_tags normally, otherwise INLINE_HTML *)
      set_mode st ST_IN_SCRIPTING2;
      (* todo? ugly, may be better ot generate a real ECHO token with maybe a
       * FakeToken or ExpandedToken. *)
      OPEN_TAG_WITH_ECHO
    }

  | "<?" | "<script" WHITESPACE+ "language" WHITESPACE* "=" WHITESPACE*
           ("php"|"\"php\""|"\'php\'") WHITESPACE* ">"
    { (* XXX if short_tags normally otherwise INLINE_HTML *)
      (* pr2 "BAD USE OF <? at initial state, replace by <?php"; *)
      set_mode st ST_IN_SCRIPTING;
      OPEN_TAG;
    }

  (*------------------------------------------------------------------------ *)

  | eof { EOF }
  | _ (* ANY_CHAR *)
    { error st "unrecognised symbol, in token rule: %a" error_lexeme lexbuf;
      UNKNOWN
    }



(*****************************************************************************)
(* Rule looking_for_xxx *)
(*****************************************************************************)

and st_looking_for_property st = parse
  | "->" { ARROW }
  | "?->" { QUESTION_ARROW }
  | LABEL { pop_mode st; IDENT (case_str st (Lexing.lexeme lexbuf)) }

(* | ANY_CHAR { (* XXX yyback(0) ?? *) pop_mode(); } *)

and st_looking_for_varname st = parse
  | LABEL
    { set_mode st ST_IN_SCRIPTING;
      VARNAME (Lexing.lexeme lexbuf) }
  | _
    { unlex_characters lexbuf 1;
      set_mode st ST_IN_SCRIPTING;
      st_in_scripting st lexbuf }

(*****************************************************************************)

and st_var_offset st = parse
  | LNUM | HEXNUM | BINNUM
    { (* /* Offset must be treated as a string */ *)
      NUM_STRING (Lexing.lexeme lexbuf) }
  | "$" (LABEL as s) { VARIABLE (case_str st s) }
  | LABEL            { IDENT (case_str st (Lexing.lexeme lexbuf))  }
  | "]"              { pop_mode st; RBRACKET }
  | eof              { EOF }
  | _
    { error st "unrecognised symbol, in st_var_offset rule: %a" error_lexeme lexbuf;
      UNKNOWN }

(*****************************************************************************)
(* Rule strings *)
(*****************************************************************************)

and st_double_quotes st = parse
  | DOUBLE_QUOTES_CHARS+  { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf) }
  (* todo? was in original scanner ? *)
  | "{"                   { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf)  }
  | "$" (LABEL as s)      { VARIABLE (case_str st s) }

  | "$" (LABEL as s) "["
    { push_mode st ST_VAR_OFFSET;
      return_many st lexbuf [
        lexeme lexbuf ~e:(-1) (VARIABLE (case_str st s));
        lexeme lexbuf ~s:(-1) LBRACKET;
      ]
    }
  (* bugfix: can have strings like "$$foo$" *)
  | "$"
    { ENCAPSED_AND_WHITESPACE "$" }

  | "{$"
    { unlex_characters lexbuf 1;
      push_mode st ST_IN_SCRIPTING;
      CURLY_OPEN;
    }
  | "${"
    { push_mode st ST_LOOKING_FOR_VARNAME;
      DOLLAR_OPEN_CURLY_BRACES;
    }

  | '"'
    { (* was originally set_mode st ST_IN_SCRIPTING, but with XHP
       * the context for a double quote may not be anymore always
       * ST_IN_SCRIPTING
       *)
      pop_mode st;
      DOUBLEQUOTE
    }
  | eof { EOF }
  | _
     { error st "unrecognised symbol, in st_double_quotes rule: %a"
         error_lexeme lexbuf;
       UNKNOWN
     }

(* ----------------------------------------------------------------------- *)
(* mostly copy paste of st_double_quotes; just the end regexp is different *)
and st_backquote st = parse
  | BACKQUOTE_CHARS+  { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf) }
  | "$" (LABEL as s)  { VARIABLE (case_str st s) }

  | "$" (LABEL as s) "["
    { push_mode st ST_VAR_OFFSET;
      return_many st lexbuf [
        lexeme lexbuf ~e:(-1) (VARIABLE s);
        lexeme lexbuf ~s:(-1) LBRACKET;
      ]
    }

  (* bugfix: can have strings like "$$foo$" *)
  | "$" { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf) }

  | "{$"
    { unlex_characters lexbuf 1;
      push_mode st ST_IN_SCRIPTING;
      CURLY_OPEN
    }

  | "${"
    { push_mode st ST_LOOKING_FOR_VARNAME;
      DOLLAR_OPEN_CURLY_BRACES
    }

  | '`'
    { set_mode st ST_IN_SCRIPTING;
      BACKQUOTE
    }

  | eof { EOF }

  | _
    { error st "unrecognised symbol, in st_backquote rule: %a"
        error_lexeme lexbuf;
      UNKNOWN
    }

(* ----------------------------------------------------------------------- *)
(* As heredoc have some of the semantic of double quote strings, again some
 * rules from st_double_quotes are copy pasted here.
 *
 * todo? the rules below are not what was in the original Zend lexer,
 * but the original lexer was doing very complicated stuff ...
 *)
and st_start_heredoc st stopdoc = parse
  | (LABEL as s) ['\n' '\r']
    { if s = stopdoc then (
        set_mode st ST_IN_SCRIPTING;
        return_many st lexbuf [
          lexeme lexbuf ~e:(-1) END_HEREDOC;
          lexeme lexbuf ~s:(-1) NEWLINE
        ]
      ) else
        ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf)
    }
  | (LABEL as s) ";" ['\n' '\r']
    { if s = stopdoc then (
        set_mode st ST_IN_SCRIPTING;
        return_many st lexbuf [
          lexeme lexbuf ~e:(-2) END_HEREDOC;
          lexeme lexbuf ~s:(-2) ~e:(-1) SEMICOLON;
          lexeme lexbuf ~s:(-1) NEWLINE;
        ]
      ) else
        ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf)
    }

  | [^ '\n' '\r' '$' '{' '\\']+
    { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf) }

  | "\\" ANY_CHAR
    { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf) }

  | "$" (LABEL as s)
    { VARIABLE (case_str st s) }

  | "$" (LABEL as s) "["
    { push_mode st ST_VAR_OFFSET;
      return_many st lexbuf [
        lexeme lexbuf ~e:(-1) (VARIABLE (case_str st s));
        lexeme lexbuf ~s:(-1) LBRACKET;
      ]
    }

  (* bugfix: can have strings like "$$foo$", or {{$foo}} *)
  | "$" { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf) }
  | "{" { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf) }
  | ['\n' '\r'] { NEWLINE }
  | "{$"
    { unlex_characters lexbuf 1;
      push_mode st ST_IN_SCRIPTING;
      CURLY_OPEN;
    }
  | "${"
    { push_mode st ST_LOOKING_FOR_VARNAME;
      DOLLAR_OPEN_CURLY_BRACES;
    }
  | eof { EOF }
  | _
    { error st "unrecognised symbol, in st_start_heredoc rule: %a" error_lexeme lexbuf;
      UNKNOWN
    }

(* ----------------------------------------------------------------------- *)
(* todo? this is not what was in the original lexer, but the original lexer
 * does complicated stuff ...
 *)
and st_start_nowdoc st stopdoc = parse
  | (LABEL as s) ['\n' '\r']
    { if s = stopdoc then (
        set_mode st ST_IN_SCRIPTING;
        return_many st lexbuf [
          lexeme lexbuf ~e:(-1) END_HEREDOC;
          lexeme lexbuf ~s:(-1) NEWLINE
        ]
      ) else
        ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf)
    }
  | (LABEL as s) ";" ['\n' '\r']
    { if s = stopdoc then (
        set_mode st ST_IN_SCRIPTING;
        return_many st lexbuf [
          lexeme lexbuf ~e:(-2) END_HEREDOC;
          lexeme lexbuf ~s:(-2) ~e:(-1) SEMICOLON;
          lexeme lexbuf ~s:(-1) NEWLINE;
        ]
      ) else
        ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf)
    }

  | [^ '\n' '\r']+
    { ENCAPSED_AND_WHITESPACE (Lexing.lexeme lexbuf) }

  | ['\n' '\r']
    { NEWLINE }

  | eof { EOF }

  | _
    { error st "unrecognised symbol, in st_start_nowdoc rule: %a" error_lexeme lexbuf;
      UNKNOWN
    }

(*****************************************************************************)
(* Rules for XHP *)
(*****************************************************************************)
(* XHP lexing states and rules *)

and st_in_xhp_tag st current_tag = parse
  (* The original XHP parser have some special handlings of
   * whitespace and enforce to use certain whitespace at
   * certain places. Not sure I need to enforce this too.
   * Simpler to ignore whitespaces.
   *
   * todo? factorize with st_in_scripting rule?
   *)
  | [' ' '\t']+ { SPACES }
  | ['\n' '\r'] { NEWLINE }
  | "/*"
    { protect_start_pos st_comment st lexbuf;
      COMMENT
    }
  | "/**/"
    { COMMENT }

  | "/**"
    { (* RESET_DOC_COMMENT(); *)
      protect_start_pos st_comment st lexbuf;
      DOC_COMMENT
    }
  | "//"
    { protect_start_pos st_one_line_comment st lexbuf;
      COMMENT
    }

  (* attribute management *)
  | XHPATTR { XHP_ATTR (Lexing.lexeme lexbuf) }
  | "="     { EQUAL }

  (* not sure if XHP strings needs the interpolation support *)
  | '"'
    { push_mode st ST_DOUBLEQUOTE;
      DOUBLEQUOTE
    }
  | "{"
    { push_mode st ST_IN_SCRIPTING;
      LBRACE
    }

  (* a singleton tag *)
  | "/>"
    { pop_mode st;
      XHP_SLASH_GT
    }

  (* When we see a ">", it means it's just the end of
   * the opening tag. Transit to IN_XHP_TEXT.
   *)
  | ">"
    { set_mode st (ST_IN_XHP_TEXT current_tag);
      XHP_GT
    }

  | eof { EOF }

  | _
    { error st "unrecognised symbol, in XHP tag: %a" error_lexeme lexbuf;
      UNKNOWN
    }

(* ----------------------------------------------------------------------- *)
and st_in_xhp_text st current_tag = parse

  (* a nested xhp construct *)
  | "<" (XHPTAG as tag)
    { let xs = string_split ':' tag in
      push_mode st (ST_IN_XHP_TAG xs);
      XHP_OPEN_TAG xs
    }

  | "</" (XHPTAG as tag) ' '* ">"
    { let xs = string_split ':' tag in
      if (xs <> current_tag) then
        error st "XHP: wrong closing tag for, %a != %a"
                 join_tag xs join_tag current_tag;
      pop_mode st;
      XHP_CLOSE_TAG (Some xs)
    }
  (* shortcut for closing tag ? *)
  | "</>"
    { (* no check :( *)
      pop_mode st;
      XHP_CLOSE_TAG None
    }

  | "<!--"
    { protect_start_pos st_xhp_comment st lexbuf;
      (* less: make a special token XHP_COMMENT? *)
      COMMENT
    }

  (* PHP interpolation. How the user can produce a { ? &;something ? *)
  | "{" { push_mode st ST_IN_SCRIPTING; LBRACE }

  (* opti: *)
  | [^'<' '{']+ { XHP_TEXT (Lexing.lexeme lexbuf) }

  | eof { EOF }

  | _
    { error st "unrecognised symbol, in XHP text: %a" error_lexeme lexbuf;
      UNKNOWN
    }

and st_xhp_comment st = parse
  | "-->"   { () }
  | [^'-']+ { st_xhp_comment st lexbuf }
  | "-"     { st_xhp_comment st lexbuf }
  | eof { error st "end of file in xhp comment" }
  | _
    { error st "unrecognised symbol in xhp comment: %a" error_lexeme lexbuf;
      st_xhp_comment st lexbuf
    }

(*****************************************************************************)
(* Rule comment *)
(*****************************************************************************)
and st_comment st = parse
  | "*/"
    { () }

  (* noteopti: *)
  | ([^'*']+ | '*')
    { st_comment st lexbuf }

  | eof { error st "end of file in comment" }

  | (_ as c)
    { error st "unrecognised symbol in comment: %c" c;
      st_comment st lexbuf
    }

and st_one_line_comment st = parse
  | ['?' '%' '>']
    { st_one_line_comment st lexbuf }

  | [^'\n' '\r' '?' '%' '>']* (ANY_CHAR as x)
    { match x with
      | '?' | '%' | '>' ->
        unlex_characters lexbuf 1;
        st_one_line_comment st lexbuf
        (* end of recursion when new line or other character  *)
      | '\n' ->
        (* don't want the newline to be part of the comment *)
        unlex_characters lexbuf 1
      | _ -> ()
    }

  | NEWLINE
    { (* don't want the newline to be part of the comment *)
      unlex_characters lexbuf 1 }

  | "?>"
    { (* "%>" is only when use asp_tags *)
      unlex_characters lexbuf 2 }

  | eof { error st "end of file in comment" }

  | _
    { error st "unrecognised symbol, in st_one_line_comment rule: %a"
        error_lexeme lexbuf }

{

  let token st lexbuf =
    match st.lexeme_stack with
    | (token, startp, endp) :: xs ->
      lexbuf.Lexing.lex_start_pos <- startp;
      lexbuf.Lexing.lex_curr_pos <- endp;
      st.lexeme_stack <- xs;
      token
    | [] ->
      if st.lexeme_stack_resume_pos > -1 then (
        lexbuf.Lexing.lex_curr_pos <- st.lexeme_stack_resume_pos;
        st.lexeme_stack_resume_pos <- -1;
      );
      match current_mode st with
      | ST_INITIAL ->
        initial st lexbuf
      | ST_IN_SCRIPTING | ST_IN_SCRIPTING2 ->
        st_in_scripting st lexbuf
      | ST_DOUBLEQUOTE ->
        st_double_quotes st lexbuf
      | ST_BACKQUOTE ->
        st_backquote st lexbuf
      | ST_LOOKING_FOR_PROPERTY ->
        st_looking_for_property st lexbuf
      | ST_LOOKING_FOR_VARNAME ->
        st_looking_for_varname st lexbuf
      | ST_VAR_OFFSET ->
        st_var_offset st lexbuf
      | ST_START_HEREDOC s ->
        st_start_heredoc st s lexbuf
      | ST_START_NOWDOC s ->
        st_start_nowdoc st s lexbuf
      | ST_IN_XHP_TAG tag ->
        assert st.xhp_builtin;
        st_in_xhp_tag st tag lexbuf
      | ST_IN_XHP_TEXT tag ->
        assert st.xhp_builtin;
        st_in_xhp_text st tag lexbuf

  let token st lexbuf =
    match token st lexbuf with
    | COMMENT | DOC_COMMENT | SPACES | NEWLINE as result ->
      result
    | result ->
      st.last_non_whitespace_like_token <- result;
      result

  let string_of_list item lst =
    "[" ^ String.concat "; " (List.map item lst) ^ "]"

  let string_of_state_mode = function
    | ST_INITIAL              -> "ST_INITIAL"
    | ST_IN_SCRIPTING         -> "ST_IN_SCRIPTING"
    | ST_IN_SCRIPTING2        -> "ST_IN_SCRIPTING2"
    | ST_DOUBLEQUOTE          -> "ST_DOUBLEQUOTE"
    | ST_BACKQUOTE            -> "ST_BACKQUOTE"
    | ST_LOOKING_FOR_PROPERTY -> "ST_LOOKING_FOR_PROPERTY"
    | ST_LOOKING_FOR_VARNAME  -> "ST_LOOKING_FOR_VARNAME"
    | ST_VAR_OFFSET           -> "ST_VAR_OFFSET"
    | ST_START_HEREDOC str    ->
      Printf.sprintf "ST_START_HEREDOC %S" str
    | ST_START_NOWDOC str     ->
      Printf.sprintf "ST_START_NOWDOC %S" str
    | ST_IN_XHP_TAG strs      ->
      Printf.sprintf "ST_IN_XHP_TAG %s"
        (string_of_list (Printf.sprintf "%S") strs)
    | ST_IN_XHP_TEXT strs     ->
      Printf.sprintf "ST_IN_XHP_TEXT %s"
        (string_of_list (Printf.sprintf "%S") strs)

  let dump_modes st =
    string_of_list string_of_state_mode st.mode_stack
}
