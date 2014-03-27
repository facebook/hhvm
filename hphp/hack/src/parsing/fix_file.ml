(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


open Utils
open Parser
open Autocomplete
open Find_refs

(*****************************************************************************)
(* We want to isolate expression from the rest of the structure of the
 * program.
*)
(*****************************************************************************)

type t =
  | Token of Parser.token
  | Block of Parser.token list

(*****************************************************************************)
(* Pretty? printer *)
(*****************************************************************************)

let print_token = function
  | PHP_CLASSIC _ -> "PHP_CLASSIC"
  | TRY _ -> "TRY"
  | THROW _ -> "THROW"
  | SWITCH _ -> "SWITCH"
  | RETURN _ -> "RETURN"
  | ASYNC _ -> "ASYNC"
  | AWAIT _ -> "AWAIT"
  | FUNCTION _ -> "FUNCTION"
  | FOREACH _ -> "FOREACH"
  | FOR _ -> "FOR"
  | PARENT _ -> "PARENT"
  | SELF _ -> "SELF"
  | HASHTABLE _ -> "HASHTABLE"
  | USE _ -> "USE"
  | ENUM _ -> "ENUM"
  | CHILDREN _ -> "CHILDREN"
  | CATEGORY _ -> "CATEGORY"
  | CLONE _ -> "CLONE"
  | ECHO _ -> "ECHO"
  | AS _ -> "AS"
  | ABSTRACT _ -> "ABSTRACT"
  | BREAK _ -> "BREAK"
  | LIST _ -> "LIST"
  | CASE _ -> "CASE"
  | WHILE _ -> "WHILE"
  | TRUE _ -> "TRUE"
  | FALSE _ -> "FALSE"
  | FINALLY _ -> "FINALLY"
  | DO _ -> "DO"
  | NULL _ -> "NULL"
  | INSTANCEOF _ -> "INSTANCEOF"
  | CLASS _ -> "CLASS"
  | TRAIT _ -> "TRAIT"
  | STATIC _ -> "STATIC"
  | NEW _ -> "NEW"
  | PHP _ -> "PHP"
  | EXTENDS _ -> "EXTENDS"
  | IMPLEMENTS _ -> "IMPLEMENTS"
  | CONST _ -> "CONST"
  | PRIVATE _ -> "PRIVATE"
  | PUBLIC _ -> "PUBLIC"
  | PROTECTED _ -> "PROTECTED"
  | INTERFACE _ -> "INTERFACE"
  | ATTRIBUTE _ -> "ATTRIBUTE"
  | REQUIRED _ -> "REQUIRED"
  | REQUIRE_ONCE _ -> "REQUIRE_ONCE"
  | CONTINUE _ -> "CONTINUE"
  | ARRAY _ -> "ARRAY"
  | ELSE _ -> "ELSE"
  | ELSEIF _ -> "ELSEIF"
  | CATCH _ -> "CATCH"
  | FINAL _ -> "FINAL"
  | YIELD _ -> "YIELD"
  | IF _ -> "IF"
  | DEFAULT _ -> "DEFAULT"
  | OTAG _ -> "OTAG"
  | CTAG _ -> "CTAG"
  | CONSTRUCT _ -> "CONSTRUCT"
  | ID _ -> "ID"
  | XHPNAME _ -> "XHPNAME"
  | LVAR _ -> "LVAR"
  | CHAR _ -> "CHAR"
  | INT _ -> "INT"
  | FLOAT _ -> "FLOAT"
  | ENCAPSED _ -> "ENCAPSED"
  | STRBLOB _ -> "STRBLOB"
  | STRING2 _ -> "STRING2"
  | STRING _ -> "STRING"
  | THEN _ -> "THEN"
  | TYPE _ -> "TYPE"
  | NEWTYPE _ -> "NEWTYPE"
  | OPT_ARGS _ -> "OPT_ARGS"
  | ARROW _ -> "ARROW"
  | SARROW _ -> "SARROW"
  | TILD _ -> "TILD"
  | AMPAMP _ -> "AMPAMP"
  | INCR _ -> "INCR"
  | DECR _ -> "DECR"
  | LTLT _ -> "LTLT"
  | GTGT _ -> "GTGT"
  | AMP _ -> "AMP"
  | UNDERSCORE _ -> "UNDERSCORE"
  | BARBAR _ -> "BARBAR"
  | COLON _ -> "COLON"
  | COMMA _ -> "COMMA"
  | DOT _ -> "DOT"
  | EQ _ -> "EQ"
  | EQEQ _ -> "EQEQ"
  | GT _ -> "GT"
  | GTE _ -> "GTE"
  | LB _ -> "LB"
  | LCB _ -> "LCB"
  | LP _ -> "LP"
  | LT _ -> "LT"
  | LTE _ -> "LTE"
  | DOUBLE_GT _ -> "DOUBLE_GT"
  | MINUS _ -> "MINUS"
  | RB _ -> "RB"
  | RCB _ -> "RCB"
  | RP _ -> "RP"
  | PERCENT _ -> "PERCENT"
  | PLUS _ -> "PLUS"
  | SC _ -> "SC"
  | SLASH _ -> "SLASH"
  | SLASH_GT _ -> "SLASH_GT"
  | STAR _ -> "STAR"
  | XOR _ -> "XOR"
  | QUOTE _ -> "QUOTE"
  | COLCOL _ -> "COLCOL"
  | EM _ -> "EM"
  | XOREQ _ -> "XOREQ"
  | AMPEQ _ -> "AMPEQ"
  | RSHIFTEQ _ -> "RSHIFTEQ"
  | LSHIFTEQ _ -> "LSHIFTEQ"
  | BAREQ _ -> "BAREQ"
  | PLUSEQ _ -> "PLUSEQ"
  | DOTEQ _ -> "DOTEQ"
  | MINUSEQ _ -> "MINUSEQ"
  | STAREQ _ -> "STAREQ"
  | SLASHEQ _ -> "SLASHEQ"
  | DIFF _ -> "DIFF"
  | DIFF2 _ -> "DIFF2"
  | UNSAFE -> "UNSAFE"
  | FALLTHROUGH -> "FALLTHROUGH"
  | EQEQEQ _ -> "EQEQEQ"
  | QM _ -> "QM"
  | BAR _ -> "BAR"
  | DOLLAR _ -> "DOLLAR"
  | EOF _ -> "EOF"
  | PERCENTEQ _ -> "PERCENTEQ"
  | BACKTICK _ -> "BACKTICK"
  | DECL_MODE_STRIPPED_BODY -> "DECL_MODE_STRIPPED_BODY"
  | SHAPE _ -> "SHAPE"

let rec print = function
  | Token t -> " "^print_token t
  | Block l ->
      "block(" ^ List.fold_right (^) (List.map (fun x -> " "^print_token x) l) ")"

let printl l = List.fold_right (^) (List.map print l) ""

(*****************************************************************************)
(* An extremely hacky way to give variables defined in a standalone  { }
 * scope by inserting 'if (true)'. Use right before the { is expected. *)
(*****************************************************************************)
let scope_force p acc = RP p :: TRUE p :: LP p :: IF p :: acc

(*****************************************************************************)
(* Given a file where a guy is typing, bringing it back to the closest form
 * where we can parse it. *)
(*****************************************************************************)

let rec make next back acc =
  let tok, mode = next() in
  match tok with
  | EOF _ -> List.rev acc
  | PHP_CLASSIC p -> make next back (Token (PHP p) :: acc)
  | LCB p -> begin
      let _, mode = next() in
      back();
      match mode with
      | Lexing_modes.Expr :: _ ->
          let block = make_block next back [] [] in
          (match block with
          | [LCB _; RCB _] -> make next back (Block block :: acc)
          | _ ->
              let acc = Block block :: acc in
              let acc =
                if List.mem Lexing_modes.Class mode
                then Token (RCB p) :: acc
                else acc in
              List.rev acc

          )
      | _ -> make next back (Token tok :: acc)
  end
  | _ ->
      make next back (Token tok :: acc)
(*****************************************************************************)
(* Removes any tokens that aren't LP, LB, or LCB
 * from the top of the level stack *)
(*****************************************************************************)
and clean_level level =
  match level with
  | (LP _) :: tail
  | (LB _) :: tail
  | (LCB _) :: tail -> level
  | _ :: tail -> clean_level tail
  | _ -> []

and make_block next back level acc =
  let tok, _ = next() in
  match tok with
  | PUBLIC _ | PRIVATE _ | PROTECTED _ | FINAL _  ->
      back(); (LCB Pos.none) :: (RCB Pos.none) :: []
  | EOF _ -> (LCB Pos.none) :: (RCB Pos.none) :: []
  | LVAR (p, x)
  | ID (p, x)
    when (!(auto_complete) && is_auto_complete x) || is_find_method_target p ->
      let acc = ref (acc @ [LCB p]) in
      acc := tok :: !acc;
      let level = ref level in
      (match !level with
      | LP _ :: _  ->
          let head = ref [] in
          (* Inserts a ';' after all the closing parens *)
          (try
            while true do
              match !level with
              | (FOR _ | WHILE _ | IF _ | LP _ | CATCH _) as lp :: rl ->
                  head := lp :: !head;
                  level := rl
              | _ -> raise Exit
            done
          with Exit ->
            level := List.rev_append !head (SC p :: !level)
          )
      | _ -> acc := SC p :: !acc);
      List.iter begin fun tok ->
        match tok with
        | LCB p -> acc := RCB p :: !acc;
        | LP p -> acc := RP p :: !acc;
        | LB p -> acc := RB p :: !acc;
        | WHILE _ | IF _ -> acc := RCB p :: LCB p :: !acc;
        | CATCH p -> acc := RCB p :: LCB p :: !acc;
        | SC p -> acc := SC p :: !acc;
        | FOR _ -> ();
        | _ -> assert false
      end !level;
      acc := RCB p :: !acc;
      List.rev !acc
  | WHILE _
  | IF _
  | LCB _ -> make_block next back (tok :: (clean_level level)) (tok :: acc)
  | LP _
  | LB _ -> make_block next back (tok :: level) (tok :: acc)
  | CATCH p -> make_block next back (tok :: level)
                   (tok :: RCB p :: LCB p :: TRY p :: acc)
  | FOR _ -> (let n, _ = next() in
             match n with
             | LP _ -> make_block next back (tok :: level) acc
             | _ -> back();  make_block next back level (tok :: acc))
  | RP p when (match level with
               | (FOR _) :: _ -> true
               | _ -> false
              ) -> make_block next back (List.tl level)
                       (scope_force p (SC p :: acc))
  | COMMA p when (match level with
                  | (FOR _) :: _ -> true
                  | _ -> false
                 ) -> make_block next back level (SC p :: acc)
  | RP _
  | RB _
  | RCB _ ->
      (match (clean_level level) with
      | [] -> [LCB Pos.none; RCB Pos.none]
      | _ :: level ->  make_block next back level (tok :: acc)
      )
  | TRY p -> make_block next back level (scope_force p acc)
  | _ -> make_block next back level (tok :: acc)


(*****************************************************************************)
(* We want to isolate expression from the rest of the structure of the
 * program.
*)
(*****************************************************************************)

let make_head lexer lb =
  let past = ref [] in
  let forward = ref [] in
  let next() =
    match !forward with
    | [] ->
        let tok = lexer lb in
        past := tok :: !past;
        tok
    | x :: rl ->
        past := x :: !past;
        forward := rl;
        x
  in
  let back() =
    match !past with
    | [] -> ()
    | x :: rl -> past := rl; forward := x :: !forward; ()
  in
  next, back

let make_lexer t =
  let acc = ref [] in
  List.iter begin fun x ->
    match x with
    | Token t -> acc := t :: !acc
    | Block tokl -> List.iter (fun x -> acc := x :: !acc) tokl
  end t;
  acc := List.rev !acc;
  fun _ ->
    match !acc with
    | [] -> EOF Pos.none
    | x :: rl -> acc := rl; x

let fix_file file_content =
  let lb = Lexing.from_string file_content in
  let lexer = HackedLexer.make_with_mode () in
  let next, back = make_head lexer lb in
  let t = make next back [] in
(*  let _ = Js.Unsafe.eval_string ("console.log('"^printl t^"')");  in   *)
  make_lexer t
