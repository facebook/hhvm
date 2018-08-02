(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
open Prim_defs
open Hh_core
(* What we are lowering to *)
open Ast

(* Don't allow expressions to nest deeper than this to avoid stack overflow *)
let recursion_limit = 30000

(* Context of the file being parsed, as (hopefully some day read-only) state. *)
type env =
  { is_hh_file               : bool
  ; codegen                  : bool
  ; systemlib_compat_mode    : bool
  ; php5_compat_mode         : bool
  ; elaborate_namespaces     : bool
  ; include_line_comments    : bool
  ; keep_errors              : bool
  ; quick_mode               : bool
  ; lower_coroutines         : bool
  ; enable_hh_syntax         : bool
  ; fail_open                : bool
  ; parser_options           : ParserOptions.t
  ; fi_mode                  : FileInfo.mode
  ; file                     : Relative_path.t
  ; stats                    : Stats_container.t option
  ; hacksperimental          : bool
  ; top_level_statements     : bool (* Whether we are (still) considering TLSs*)
  ; inside_declare           : bool (* Whether we're inside a declare directive. *)
  (* Changing parts; should disappear in future. `mutable` saves allocations. *)
  ; mutable ignore_pos       : bool
  ; mutable max_depth        : int    (* Filthy hack around OCaml bug *)
  ; mutable saw_yield        : bool   (* Information flowing back up *)
  ; mutable unsafes          : ISet.t (* Offsets of UNSAFE_EXPR in trivia *)
  ; mutable saw_std_constant_redefinition: bool
  (* Whether we've seen COMPILER_HALT_OFFSET. The value of COMPILER_HALT_OFFSET
    defaults to 0 if HALT_COMPILER isn't called.
    None -> COMPILER_HALT_OFFSET isn't in the source file
    Some 0 -> COMPILER_HALT_OFFSET is in the source file, but HALT_COMPILER isn't
    Some x -> COMPILER_HALT_OFFSET is in the source file,
              HALT_COMPILER is at x bytes offset in the file.
  *)
  ; saw_compiler_halt_offset : (int option) ref
  ; recursion_depth : int ref
  }[@@deriving show]

let make_env
  ?(codegen                  = false                   )
  ?(systemlib_compat_mode    = false                   )
  ?(php5_compat_mode         = false                   )
  ?(elaborate_namespaces     = true                    )
  ?(include_line_comments    = false                   )
  ?(keep_errors              = true                    )
  ?(ignore_pos               = false                   )
  ?(quick_mode               = false                   )
  ?(lower_coroutines         = true                    )
  ?(enable_hh_syntax         = false                   )
  ?(fail_open                = true                    )
  ?(parser_options           = ParserOptions.default   )
  ?(fi_mode                  = FileInfo.Mpartial       )
  ?(is_hh_file               = false                   )
  ?stats
  ?(hacksperimental          = false                   )
  (file : Relative_path.t)
  : env
  = let parser_options = ParserOptions.with_hh_syntax_for_hhvm parser_options
      (codegen && (enable_hh_syntax || is_hh_file)) in
    { is_hh_file
    ; codegen = codegen || systemlib_compat_mode
    ; systemlib_compat_mode
    ; php5_compat_mode
    ; elaborate_namespaces
    ; include_line_comments
    ; keep_errors
    ; quick_mode =
         not codegen
      && (match fi_mode with
         | FileInfo.Mdecl
         | FileInfo.Mphp -> true
         | _ -> quick_mode
         )
    ; lower_coroutines
    ; enable_hh_syntax
    ; parser_options
    ; fi_mode
    ; fail_open
    ; file
    ; stats
    ; hacksperimental
    ; top_level_statements = true
    ; inside_declare = false
    ; ignore_pos
    ; max_depth = 42
    ; saw_yield = false
    ; unsafes = ISet.empty
    ; saw_std_constant_redefinition = false
    ; saw_compiler_halt_offset = ref None
    ; recursion_depth = ref 0
    }

type result =
  { fi_mode  : FileInfo.mode
  ; is_hh_file : bool
  ; ast      : Ast.program
  ; content  : string
  ; file     : Relative_path.t
  ; comments : (Pos.t * comment) list
  } [@@deriving show]

module WithPositionedSyntax(Syntax : Positioned_syntax_sig.PositionedSyntax_S) = struct

(* What we're lowering from *)
open Syntax

type node = Syntax.t

module Token = Syntax.Token
module Trivia = Token.Trivia
module TriviaKind = Trivia.TriviaKind

module SyntaxKind = Full_fidelity_syntax_kind
module TK = Full_fidelity_token_kind
module SourceText = Trivia.SourceText

module NS = Namespaces

let is_hack (env : env) = env.is_hh_file || env.enable_hh_syntax
let is_typechecker env =
   is_hack env && (not env.codegen)

let drop_pstr : int -> pstring -> pstring = fun cnt (pos, str) ->
  let len = String.length str in
  pos, if cnt >= len then "" else String.sub str cnt (len - cnt)

let non_tls env = if not env.top_level_statements then env else
  { env with top_level_statements = false }

type +'a parser = node -> env -> 'a
type ('a, 'b) metaparser = 'a parser -> 'b parser

let underscore = Str.regexp "_"
let quoted = Str.regexp "[ \t\n\r\012]*\"\\(\\(.\\|\n\\)*\\)\""
let whitespace = Str.regexp "[ \t\n\r\012]+"
let hashbang = Str.regexp "^#!.*\n"
let ignore_error = Str.regexp "HH_\\(FIXME\\|IGNORE_ERROR\\)[ \\t\\n]*\\[?\\([0-9]+\\)\\]?"
let namespace_use = Str.regexp "[^\\\\]*$"

let mode_annotation = function
  | FileInfo.Mphp -> FileInfo.Mdecl
  | m -> m

let syntax_to_list include_separators node  =
  let rec aux acc syntax_list =
    match syntax_list with
    | [] -> acc
    | h :: t ->
      begin
        match syntax h with
        | ListItem { list_item; list_separator } ->
          let acc = list_item :: acc in
          let acc =
            if include_separators then (list_separator :: acc ) else acc in
          aux acc t
        | _ -> aux (h :: acc) t
      end in
  match syntax node with
  | Missing -> [ ]
  | SyntaxList s -> List.rev (aux [] s)
  | ListItem { list_item; list_separator } ->
    if include_separators then [ list_item; list_separator ] else [ list_item ]
  | _ -> [ node ]

let syntax_to_list_no_separators = syntax_to_list false

let pPos : Pos.t parser = fun node env ->
  if env.ignore_pos
  then Pos.none
  else Option.value ~default:Pos.none (position_exclusive env.file node)

let raise_parsing_error env node_or_pos msg =
  if not env.quick_mode && env.keep_errors then
    let p = match node_or_pos with
      | `Pos pos -> pos
      | `Node node -> pPos node env
    in
    Errors.parsing_error (p, msg)
  else if env.codegen && not env.lower_coroutines then
    let p = match node_or_pos with
      | `Pos pos -> pos
      | `Node node -> (Option.value (position env.file node) ~default:Pos.none)
    in
    let (s, e) = Pos.info_raw p in
    let e = SyntaxError.make ~error_type:SyntaxError.ParseError s e msg in
    raise @@ SyntaxError.ParserFatal (e, p)
  else ()

(* HHVM starts range of function declaration from the 'function' keyword *)
let pFunction node env =
  let p = pPos node env in
  match syntax node with
  | FunctionDeclaration  { function_declaration_header = h; _ }
  | MethodishDeclaration { methodish_function_decl_header = h; _ }
    when env.codegen ->
    begin match syntax h with
    | FunctionDeclarationHeader { function_keyword = f; _ }
      when not (is_missing f) ->
      (* For continuation compilation, we end up with spans across files :-( *)
      Pos.btw_nocheck (pPos f env) p
    | _ -> p
    end
  | _ -> p

exception Lowerer_invariant_failure of string * string
let invariant_failure node msg env =
  let pos = Pos.string (Pos.to_absolute (pPos node env)) in
  raise (Lowerer_invariant_failure (pos, msg))

let scuba_table = Scuba.Table.of_name "hh_missing_lowerer_cases"

let log_missing ?(caught = false) ~(env:env) ~expecting node : unit =
  EventLogger.log_if_initialized @@ fun () ->
    let source = source_text node in
    let start = start_offset node in
    let end_ = end_offset node in
    let pos = SourceText.relative_pos env.file source start end_ in
    let file = Relative_path.to_absolute env.file in
    let contents =
      let context_size = 5000 in
      let start = max 0 (start - context_size) in
      let length = min (2 * context_size) (SourceText.length source - start) in
      SourceText.sub source start length
    in
    let kind = SyntaxKind.to_string (Syntax.kind node) in
    let line = Pos.line pos in
    let column = Pos.start_cnum pos in
    let synthetic = is_synthetic node in
    Scuba.new_sample (Some scuba_table)
    |> Scuba.add_normal "filename" file
    |> Scuba.add_normal "expecting" expecting
    |> Scuba.add_normal "contents" contents
    |> Scuba.add_normal "found_kind" kind
    |> Scuba.add_int "line" line
    |> Scuba.add_int "column" column
    |> Scuba.add_int "is_synthetic" (if synthetic then 1 else 0)
    |> Scuba.add_int "caught" (if caught then 1 else 0)
    |> EventLogger.log

exception API_Missing_syntax of string * env * node
let missing_syntax : ?fallback:'a -> string -> node -> env -> 'a =
  fun ?fallback expecting node env ->
    match fallback with
    | Some x when env.fail_open ->
      let () = log_missing ~env ~expecting node in
      x
    | _ -> raise (API_Missing_syntax (expecting, env, node))

let runP : 'a parser -> node -> env -> 'a = fun pThing thing env ->
  try pThing thing env with
  | API_Missing_syntax (s, env, n) ->
    let pos = Pos.string (Pos.to_absolute (pPos n env)) in
    let msg = Printf.sprintf
      "missing case in %s.
   - pos: %s
   - unexpected: '%s'
   - kind: %s
   "
    s
    pos
    (text n)
    (SyntaxKind.to_string (kind n))
    in
    raise (Failure msg)

(* TODO: Cleanup this hopeless Noop mess *)
let mk_noop pos : stmt list -> stmt list = function
  | [] -> [pos, Noop]
  | s -> s
let mpStripNoop pThing node env = match pThing node env with
  | [_, Noop] -> []
  | stmtl -> stmtl

let mpOptional : ('a, 'a option) metaparser = fun p -> fun node env ->
  match syntax node with
    | Missing -> None
    | _ -> Some (p node env)
let mpYielding : ('a, ('a * bool)) metaparser = fun p node env ->
  let outer_saw_yield = env.saw_yield in
  let () = env.saw_yield <- false in
  let result = p node env in
  let result = result, env.saw_yield in
  let () = env.saw_yield <- outer_saw_yield in
  result

type expr_location =
  | TopLevel
  | MemberSelect
  | InDoubleQuotedString
  | InBacktickedString
  | InGlobalVar
  | AsStatement
  | RightOfAssignment
  | RightOfReturn

let in_string l =
  l = InDoubleQuotedString || l = InBacktickedString

  let pos_qualified_name node env =
  let aux p =
    match syntax p with
    | ListItem li -> (text li.list_item) ^ (text li.list_separator)
    | _ -> text p in
  let p = pPos node env in
  let name =
    match syntax node with
    | QualifiedName {
        qualified_name_parts = { syntax = SyntaxList l; _ };
      } ->
      String.concat "" @@ List.map ~f:aux l
    | _ -> missing_syntax "qualified name" node env in
  p, name

let rec pos_name node env =
  match syntax node with
  | QualifiedName _ -> pos_qualified_name node env
  | SimpleTypeSpecifier { simple_type_specifier = s } -> pos_name s env
  | _ ->
  let name = text node in
  let local_ignore_pos = env.ignore_pos in
  (* Special case for __LINE__; never ignore position for that special name *)
  if name = "__LINE__" then env.ignore_pos <- false;
  if name = "__COMPILER_HALT_OFFSET__" then env.saw_compiler_halt_offset := Some 0;
  let p = pPos node env in
  env.ignore_pos <- local_ignore_pos;
  p, name

let is_ret_by_ref node = not @@ is_missing node

let couldMap : 'a . f:'a parser -> 'a list parser = fun ~f -> fun node env ->
  let rec synmap : 'a . 'a parser -> 'a list parser = fun f node env ->
    match syntax node with
    | SyntaxList        l -> List.concat_map l ~f:(fun n -> go ~f n env)
    | ListItem          i -> [f i.list_item env]
    | _ -> [f node env]
  and go : 'a . f:'a parser -> 'a list parser = fun ~f -> function
    | node when is_missing node -> fun _env -> []
    | node -> synmap f node
  in
  go ~f node env

let as_list : node -> node list =
  let strip_list_item = function
    | { syntax = ListItem { list_item = i; _ }; _ } -> i
    | x -> x
  in function
  | { syntax = SyntaxList ({syntax = ListItem _; _}::_ as synl); _ } ->
    List.map ~f:strip_list_item synl
  | { syntax = SyntaxList synl; _ } -> synl
  | { syntax = Missing; _ } -> []
  | syn -> [syn]

let token_kind : node -> TK.t option = function
  | { syntax = Token t; _ } -> Some (Token.kind t)
  | _ -> None

let pBop : (expr -> expr -> expr_) parser = fun node env lhs rhs ->
  match token_kind node with
  | Some TK.Equal                       -> Binop (Eq None,           lhs, rhs)
  | Some TK.Bar                         -> Binop (Bar,               lhs, rhs)
  | Some TK.Ampersand                   -> Binop (Amp,               lhs, rhs)
  | Some TK.Plus                        -> Binop (Plus,              lhs, rhs)
  | Some TK.Minus                       -> Binop (Minus,             lhs, rhs)
  | Some TK.Star                        -> Binop (Star,              lhs, rhs)
  | Some TK.Or                          ->
    if not env.codegen then raise_parsing_error env (`Node node) SyntaxError.do_not_use_or;
    Binop (BArbar,            lhs, rhs)
  | Some TK.And                         ->
    if not env.codegen then raise_parsing_error env (`Node node) SyntaxError.do_not_use_and;
    Binop (AMpamp,            lhs, rhs)
  | Some TK.Xor                         ->
    if not env.codegen then raise_parsing_error env (`Node node) SyntaxError.do_not_use_xor;
    Binop (LogXor,            lhs, rhs)
  | Some TK.Carat                       -> Binop (Xor,               lhs, rhs)
  | Some TK.Slash                       -> Binop (Slash,             lhs, rhs)
  | Some TK.Dot                         -> Binop (Dot,               lhs, rhs)
  | Some TK.Percent                     -> Binop (Percent,           lhs, rhs)
  | Some TK.LessThan                    -> Binop (Lt,                lhs, rhs)
  | Some TK.GreaterThan                 -> Binop (Gt,                lhs, rhs)
  | Some TK.EqualEqual                  -> Binop (Eqeq,              lhs, rhs)
  | Some TK.LessThanEqual               -> Binop (Lte,               lhs, rhs)
  | Some TK.GreaterThanEqual            -> Binop (Gte,               lhs, rhs)
  | Some TK.StarStar                    -> Binop (Starstar,          lhs, rhs)
  | Some TK.ExclamationEqual            -> Binop (Diff,              lhs, rhs)
  | Some TK.BarEqual                    -> Binop (Eq (Some Bar),     lhs, rhs)
  | Some TK.PlusEqual                   -> Binop (Eq (Some Plus),    lhs, rhs)
  | Some TK.MinusEqual                  -> Binop (Eq (Some Minus),   lhs, rhs)
  | Some TK.StarEqual                   -> Binop (Eq (Some Star),    lhs, rhs)
  | Some TK.StarStarEqual               -> Binop (Eq (Some Starstar),lhs, rhs)
  | Some TK.SlashEqual                  -> Binop (Eq (Some Slash),   lhs, rhs)
  | Some TK.DotEqual                    -> Binop (Eq (Some Dot),     lhs, rhs)
  | Some TK.PercentEqual                -> Binop (Eq (Some Percent), lhs, rhs)
  | Some TK.CaratEqual                  -> Binop (Eq (Some Xor),     lhs, rhs)
  | Some TK.AmpersandEqual              -> Binop (Eq (Some Amp),     lhs, rhs)
  | Some TK.BarBar                      -> Binop (BArbar,            lhs, rhs)
  | Some TK.AmpersandAmpersand          -> Binop (AMpamp,            lhs, rhs)
  | Some TK.LessThanLessThan            -> Binop (Ltlt,              lhs, rhs)
  | Some TK.GreaterThanGreaterThan      -> Binop (Gtgt,              lhs, rhs)
  | Some TK.EqualEqualEqual             -> Binop (EQeqeq,            lhs, rhs)
  | Some TK.LessThanLessThanEqual       -> Binop (Eq (Some Ltlt),    lhs, rhs)
  | Some TK.GreaterThanGreaterThanEqual -> Binop (Eq (Some Gtgt),    lhs, rhs)
  | Some TK.LessThanGreaterThan         -> Binop (Diff,              lhs, rhs)
  | Some TK.ExclamationEqualEqual       -> Binop (Diff2,             lhs, rhs)
  | Some TK.LessThanEqualGreaterThan    -> Binop (Cmp,               lhs, rhs)
  | Some TK.QuestionQuestion            -> Binop (QuestionQuestion,  lhs, rhs)
  | Some TK.QuestionQuestionEqual       -> Binop (Eq (Some QuestionQuestion), lhs, rhs)
  (* The ugly duckling; In the FFP, `|>` is parsed as a
   * `BinaryOperator`, whereas the typed AST has separate constructors for
   * Pipe and Binop. This is why we don't just project onto a
   * `bop`, but a `expr -> expr -> expr_`.
   *)
  | Some TK.BarGreaterThan              -> Pipe         (lhs, rhs)
  | Some TK.QuestionColon               -> Eif          (lhs, None, rhs)
  (* TODO: Figure out why this fails silently when used in a pBlock; probably
     just caught somewhere *)
  | _ -> missing_syntax "binary operator" node env

let pImportFlavor : import_flavor parser = fun node env ->
  match token_kind node with
  | Some TK.Include      -> Include
  | Some TK.Require      -> Require
  | Some TK.Include_once -> IncludeOnce
  | Some TK.Require_once -> RequireOnce
  | _ -> missing_syntax "import flavor" node env

let pNullFlavor : og_null_flavor parser = fun node env ->
  match token_kind node with
  | Some TK.QuestionMinusGreaterThan -> OG_nullsafe
  | Some TK.MinusGreaterThan         -> OG_nullthrows
  | _ -> missing_syntax "null flavor" node env

type modifiers = {
  has_async: bool;
  has_coroutine: bool;
  kinds: kind list
}

let pModifiers check_modifier node env =
  let f (has_async, has_coroutine, kinds) node =
    let add_kind k =
      check_modifier node;
      k :: kinds
    in
    match token_kind node with
    | Some TK.Final     -> has_async, has_coroutine, add_kind Final
    | Some TK.Static    -> has_async, has_coroutine, add_kind Static
    | Some TK.Abstract  -> has_async, has_coroutine, add_kind Abstract
    | Some TK.Private   -> has_async, has_coroutine, add_kind Private
    | Some TK.Public    -> has_async, has_coroutine, add_kind Public
    | Some TK.Protected -> has_async, has_coroutine, add_kind Protected
    | Some TK.Var       -> has_async, has_coroutine, add_kind Public
    | Some TK.Async     -> true, has_coroutine, kinds
    | Some TK.Coroutine -> has_async, true, kinds
    | _ -> missing_syntax "kind" node env in
  let (has_async, has_coroutine, kinds) =
    Core_list.fold_left ~init:(false, false, []) ~f (as_list node) in
  { has_async; has_coroutine; kinds = List.rev kinds }

let pKinds check_modifier node env =
  (pModifiers check_modifier node env).kinds

let pParamKind : param_kind parser = fun node env ->
  match token_kind node with
  | Some TK.Inout -> Pinout
  | _ -> missing_syntax "param kind" node env

(* TODO: Clean up string escaping *)
let prepString2 env : node list -> node list =
  let is_double_quote_or_backtick ch = ch = '"' || ch = '`' in
  let is_binary_string_header s =
    (String.length s > 1) && (s.[0] = 'b') && (s.[1] = '"') in
  let trimLeft = Token.trim_left in
  let trimRight = Token.trim_right in
  function
  | ({ syntax = Token t; _ } as node::ss)
  when (Token.width t) > 0 &&
    ((is_double_quote_or_backtick (Token.text t).[0])
      || is_binary_string_header (Token.text t)) ->
    let rec unwind = function
      | [{ syntax = Token t; _ }]
      when (Token.width t) > 0 &&
          is_double_quote_or_backtick ((Token.text t).[(Token.width t) - 1]) ->
        let s = make_token (trimRight ~n:1 t) in
        if width s > 0 then [s] else []
      | x :: xs -> x :: unwind xs
      | _ -> raise_parsing_error env (`Node node) "Malformed String2 SyntaxList"; []
    in
    (* Trim the starting b and double quote *)
    let left_trim = if (Token.text t).[0] = 'b' then 2 else 1 in
    let s = make_token (trimLeft ~n:left_trim t) in
    if width s > 0 then s :: unwind ss else unwind ss
  | ({ syntax = Token t; _ } as node ::ss)
  when (Token.width t) > 3 && String.sub (Token.text t) 0 3 = "<<<" ->
    let rec unwind = function
      | [{ syntax = Token t; _ }] when (Token.width t) > 0 ->
        let content = Token.text t in
        let len = (Token.width t) in
        let n = len - (String.rindex_from content (len - 2) '\n') in
        let s = make_token (trimRight ~n t) in
        if width s > 0 then [s] else []
      | x :: xs -> x :: unwind xs
      | _ -> raise_parsing_error env (`Node node) "Malformed String2 SyntaxList"; []
    in
    let content = Token.text t in
    let n = (String.index content '\n') + 1 in
    let s = make_token (trimLeft ~n t) in
    if width s > 0 then s :: unwind ss else unwind ss
  | x -> x (* unchanged *)

let extract_unquoted_string ~start ~len content =
  try (* Using String.sub; Invalid_argument when str too short *)
    if len >= 3 && String.sub content start 3 = "<<<" (* The heredoc case *)
    then
       (* These types of strings begin with an opening line containing <<<
        * followed by a string to use as a terminator (which is optionally
        * quoted) and end with a line containing only the terminator and a
        * semicolon followed by a blank line. We need to drop the opening line
        * as well as the blank line and preceding terminator line.
        *)
       let start_ = String.index_from content start '\n' + 1 in
       let end_ = String.rindex_from content (start + len - 2) '\n' in
       (* An empty heredoc, this way, will have start >= end *)
       if start_ >= end_ then "" else String.sub content start_ (end_ - start_)
    else
      match String.get content start, String.get content (start + len - 1) with
        | '"', '"' | '\'', '\'' | '`', '`' ->
            String.sub content (start + 1) (len - 2)
        | _ ->
            if start = 0 && len = String.length content then
              content
            else
              String.sub content start len
  with Invalid_argument _ | Not_found -> content

let mkStr env node : (string -> string) -> string -> string = fun unescaper content ->
  let content = if String.length content > 0 && content.[0] = 'b'
    then String.sub content 1 (String.length content - 1) else content in
  let len = String.length content in
  let no_quotes = extract_unquoted_string ~start:0 ~len content in
  try unescaper no_quotes with
  | Php_escaping.Invalid_string _ ->
    raise_parsing_error env
      (`Node node) (Printf.sprintf "Malformed string literal <<%s>>" no_quotes);
    ""

let unempty_str = function
  | "''" | "\"\"" -> ""
  | s -> s
let unesc_dbl s = unempty_str @@ Php_escaping.unescape_double s
let get_quoted_content s =
  let open Str in
  if string_match (quoted) s 0
  then matched_group 1 s
  else s
let unesc_xhp s =
  Str.global_replace whitespace " " s
let unesc_xhp_attr s =
  unesc_dbl @@ get_quoted_content s

type suspension_kind =
  | SKSync
  | SKAsync
  | SKCoroutine

let mk_suspension_kind_ node env has_async has_coroutine =
  match has_async, has_coroutine with
  | false, false -> SKSync
  | true, false -> SKAsync
  | false, true -> SKCoroutine
  | true, true ->
    raise_parsing_error env (`Node node) "Coroutine functions may not be async";
    SKCoroutine

let mk_suspension_kind node env is_async is_coroutine =
  mk_suspension_kind_ node env
    (not (is_missing is_async))
    (not (is_missing is_coroutine))

let mk_fun_kind suspension_kind yield =
  match suspension_kind, yield with
  | SKSync,  true  -> FGenerator
  | SKAsync, true  -> FAsyncGenerator
  | SKSync,  false -> FSync
  | SKAsync, false -> FAsync
  | SKCoroutine, _ -> FCoroutine
  (* Yield in coroutine is not permitted, the error will be reported at NastCheck *)

let fun_template yielding node suspension_kind env =
  let p = pFunction node env in
  { f_mode            = mode_annotation env.fi_mode
  ; f_tparams         = []
  ; f_constrs         = []
  ; f_ret             = None
  ; f_ret_by_ref      = false
  ; f_name            = p, ";anonymous"
  ; f_params          = []
  ; f_body            = []
  ; f_user_attributes = []
  ; f_fun_kind        = mk_fun_kind suspension_kind yielding
  ; f_namespace       = Namespace_env.empty env.parser_options
  ; f_span            = p
  ; f_doc_comment     = None
  ; f_static          = false
  }

let param_template node env =
  { param_hint            = None
  ; param_is_reference    = false
  ; param_is_variadic     = false
  ; param_id              = pos_name node env
  ; param_expr            = None
  ; param_modifier        = None
  ; param_callconv        = None
  ; param_user_attributes = []
  }

let pShapeFieldName : shape_field_name parser = fun name env ->
  match syntax name with
  | ScopeResolutionExpression
    { scope_resolution_qualifier; scope_resolution_name; _ } ->
      SFclass_const
      ( pos_name scope_resolution_qualifier env
      , pos_name scope_resolution_name env
      )
  | LiteralExpression {
      literal_expression = { syntax = Token t; _ }
    } when Token.kind t = TK.SingleQuotedStringLiteral ||
           Token.kind t = TK.DoubleQuotedStringLiteral ->
    let p, n = pos_name name env in SFlit_str (p, mkStr env name unesc_dbl n)
  | _ ->
    raise_parsing_error env (`Node name) SyntaxError.invalid_shape_field_name;
    missing_syntax "shape field name" name env

let mpShapeExpressionField : ('a, (shape_field_name * 'a)) metaparser =
  fun hintParser node env ->
    match syntax node with
    | FieldInitializer
      { field_initializer_name = name; field_initializer_value = ty; _ } ->
        let name = pShapeFieldName name env in
        let ty = hintParser ty env in
        name, ty
    | _ -> missing_syntax "shape field" node env

let mpShapeField : ('a, shape_field) metaparser =
  fun hintParser node env ->
    match syntax node with
    | FieldSpecifier { field_question; field_name; field_type; _ } ->
        let sf_optional = not (is_missing field_question) in
        let sf_name = pShapeFieldName field_name env in
        let sf_hint = hintParser field_type env in
        { sf_optional; sf_name; sf_hint }
    | _ ->
        let sf_name, sf_hint = mpShapeExpressionField hintParser node env in
        (* Shape expressions can never have optional fields. *)
        { sf_optional = false; sf_name; sf_hint }

let mpClosureParameter : ('a, hint * param_kind option) metaparser =
  fun hintParser node env ->
    match syntax node with
    | ClosureParameterTypeSpecifier
      { closure_parameter_call_convention
      ; closure_parameter_type
      } ->
        let cp_kind =
          mpOptional pParamKind closure_parameter_call_convention env in
        let cp_hint = hintParser closure_parameter_type env in
        cp_hint, cp_kind
    | _ -> missing_syntax "closure parameter" node env

(* In some cases, we need to unwrap an extra layer of Block due to lowering
 * from CompoundStatement. This applies to `if`, `while` and other control flow
 * statements which allow optional curly braces.
 *
 * In other words, we want these to be lowered into the same Ast
 * `if ($b) { func(); }` and `if ($b) func();`
 * rather than the left hand side one having an extra `Block` in the Ast
 *)
let unwrap_extra_block (stmt : block) : block =
  let de_noop = function
  | [_, Noop] -> []
  | stmts -> stmts
  in
  match stmt with
  | [pos, Unsafe; _, Block b] -> (pos, Unsafe) :: de_noop b
  | [_, Block b] -> de_noop b
  | blk -> blk

let rec pHint : hint parser = fun node env ->
  let rec pHint_ : hint_ parser = fun node env ->
    match syntax node with
    (* Dirty hack; CastExpression can have type represented by token *)
    | Token _
    | SimpleTypeSpecifier _
    | QualifiedName _
      -> Happly (pos_name node env, [])
    | ShapeTypeSpecifier { shape_type_fields; shape_type_ellipsis; _ } ->
      let si_allows_unknown_fields =
        not (is_missing shape_type_ellipsis)
      in
      (* if last element lacks a separator and ellipsis is present, error *)
      Option.iter (List.last (syntax_to_list true shape_type_fields)) (fun last ->
        if is_missing last && si_allows_unknown_fields then
        raise_parsing_error env (`Node node) SyntaxError.shape_type_ellipsis_without_trailing_comma
      );
      let si_shape_field_list =
        couldMap ~f:(mpShapeField pHint) shape_type_fields env in
      Hshape { si_allows_unknown_fields; si_shape_field_list }
    | TupleTypeSpecifier { tuple_types; _ } ->
      Htuple (couldMap ~f:pHint tuple_types env)
    | KeysetTypeSpecifier { keyset_type_keyword = kw; keyset_type_type = ty; _ }
    | VectorTypeSpecifier { vector_type_keyword = kw; vector_type_type = ty; _ }
    | ClassnameTypeSpecifier {classname_keyword = kw; classname_type   = ty; _ }
    | TupleTypeExplicitSpecifier
      { tuple_type_keyword = kw
      ; tuple_type_types   = ty
      ; _ }
    | VarrayTypeSpecifier
      { varray_keyword = kw
      ; varray_type    = ty
      ; _ }
    | VectorArrayTypeSpecifier
      { vector_array_keyword = kw
      ; vector_array_type    = ty
      ; _ }
      -> Happly (pos_name kw env, couldMap ~f:pHint ty env)

    | DarrayTypeSpecifier
      { darray_keyword = kw
      ; darray_key     = key
      ; darray_value   = value
      ; _ }
    | MapArrayTypeSpecifier
      { map_array_keyword = kw
      ; map_array_key     = key
      ; map_array_value   = value
      ; _ } ->
        Happly
        ( pos_name kw env
        , pHint key env :: couldMap ~f:pHint value env
        )
    | DictionaryTypeSpecifier
      { dictionary_type_keyword = kw
      ; dictionary_type_members = members
      ; _ } -> Happly (pos_name kw env, couldMap ~f:pHint members env)
    | GenericTypeSpecifier { generic_class_type; generic_argument_list } ->
      Happly
      ( pos_name generic_class_type env
      , match syntax generic_argument_list with
        | TypeArguments { type_arguments_types; _ }
          -> couldMap ~f:pHint type_arguments_types env
        | _ -> missing_syntax "generic type arguments" generic_argument_list env
      )
    | NullableTypeSpecifier { nullable_type; _ } ->
      Hoption (pHint nullable_type env)
    | SoftTypeSpecifier { soft_type; _ } ->
      Hsoft (pHint soft_type env)
    | ClosureTypeSpecifier {
        closure_parameter_list;
        closure_return_type;
        closure_coroutine; _} ->
      let make_variadic_hint variadic_type =
        if is_missing variadic_type
        then Hvariadic (None)
        else Hvariadic (Some (pHint variadic_type env))
      in
      let (param_list, variadic_hints) =
        List.partition_map ~f:(fun x ->
          match syntax x with
          | VariadicParameter { variadic_parameter_type = vtype; _ } ->
            `Snd (make_variadic_hint vtype)
          | _ -> `Fst (mpClosureParameter pHint x env))
        (as_list closure_parameter_list)
      in
      let hd_variadic_hint hints =
        if List.length hints > 1 then begin
          let msg = Printf.sprintf
            "%d variadic parameters found. There should be no more than one."
            (List.length hints)
          in
          invariant_failure node msg env
        end;
        match List.hd hints with
        | Some h -> h
        | None -> Hnon_variadic
      in
      let is_coroutine = not (is_missing closure_coroutine) in
      let param_type_hints = List.map param_list fst in
      let param_callconvs = List.map param_list snd in
      Hfun
      ( is_coroutine
      , param_type_hints
      , param_callconvs
      , hd_variadic_hint variadic_hints
      , pHint closure_return_type env
      )
    | TypeConstant { type_constant_left_type; type_constant_right_type; _ } ->
      let child = pos_name type_constant_right_type env in
      (match pHint_ type_constant_left_type env with
      | Haccess (b, c, cs) -> Haccess (b, c, cs @ [child])
      | Happly (b, []) -> Haccess (b, child, [])
      | _ -> missing_syntax "type constant base" node env
      )
    | ReifiedTypeArgument _ ->
      raise_parsing_error env (`Node node) SyntaxError.invalid_reified;
      missing_syntax "type hint - reified" node env
    | _ -> missing_syntax "type hint" node env
  in
  pPos node env, pHint_ node env

let pTarg node env =
  match syntax node with
    | ReifiedTypeArgument { reified_type_argument_type = t; _} ->
      pHint t env, true
    | _ -> pHint node env, false

type fun_hdr =
  { fh_suspension_kind : suspension_kind
  ; fh_name            : pstring
  ; fh_constrs         : (hint * constraint_kind * hint) list
  ; fh_type_parameters : tparam list
  ; fh_parameters      : fun_param list
  ; fh_return_type     : hint option
  ; fh_param_modifiers : fun_param list
  ; fh_ret_by_ref      : bool
  }

let empty_fun_hdr =
  { fh_suspension_kind = SKSync
  ; fh_name            = Pos.none, "<ANONYMOUS>"
  ; fh_constrs         = []
  ; fh_type_parameters = []
  ; fh_parameters      = []
  ; fh_return_type     = None
  ; fh_param_modifiers = []
  ; fh_ret_by_ref      = false
  }

let prevent_intrinsic_generic env node ty =
  if not (is_missing ty) && not env.codegen then
    raise_parsing_error env (`Node node) SyntaxError.collection_intrinsic_generic

let rec pSimpleInitializer node env =
  match syntax node with
  | SimpleInitializer { simple_initializer_value; _ } ->
    pExpr simple_initializer_value env
  | _ -> missing_syntax "simple initializer" node env

and pFunParamDefaultValue node env =
  match syntax node with
  | SimpleInitializer { simple_initializer_value; _ } ->
    begin match syntax simple_initializer_value with
    | ListExpression _ ->
      raise_parsing_error env (`Node node) (SyntaxError.invalid_default_argument "A list destructuring")
    | YieldExpression _
    | YieldFromExpression _ ->
      raise_parsing_error env (`Node node) (SyntaxError.invalid_default_argument "A yield")
    | PrefixUnaryExpression {
      prefix_unary_operator = { syntax = Token t; _ }; _ } when Token.kind t = TK.Await ->
      raise_parsing_error env (`Node node) (SyntaxError.invalid_default_argument "An await")
    | _ -> () end;
    mpOptional pExpr simple_initializer_value env
  | _ -> None


and pFunParam : fun_param parser = fun node env ->
  match syntax node with
  | ParameterDeclaration
    { parameter_attribute
    ; parameter_visibility
    ; parameter_call_convention
    ; parameter_type
    ; parameter_name
    ; parameter_default_value
    } ->
    let is_reference, is_variadic, name =
      match syntax parameter_name with
      | DecoratedExpression
        { decorated_expression_decorator; decorated_expression_expression } ->
          (* There is a chance that the expression might be nested with an
           additional decorator, check this *)
          begin match syntax decorated_expression_expression with
          | DecoratedExpression
            { decorated_expression_decorator = nested_decorator
            ; decorated_expression_expression = nested_expression } ->
            let decorator = text decorated_expression_decorator in
            let nested_decorator = text nested_decorator in
            decorator = "&" || nested_decorator = "&",
            decorator = "..." || nested_decorator = "...",
            nested_expression
          | _ ->
            let decorator = text decorated_expression_decorator in
            decorator = "&", decorator = "...", decorated_expression_expression
          end
      | _ -> false, false, parameter_name
    in
    { param_hint            = mpOptional pHint parameter_type env
    ; param_is_reference    = is_reference
    ; param_is_variadic     = is_variadic
    ; param_id              = pos_name name env
    ; param_expr            = pFunParamDefaultValue parameter_default_value env
    ; param_user_attributes = pUserAttributes env parameter_attribute
    ; param_callconv        =
      mpOptional pParamKind parameter_call_convention env
    (* implicit field via constructor parameter.
     * This is always None except for constructors and the modifier
     * can be only Public or Protected or Private.
     *)
    ; param_modifier =
      let rec go = function
      | [] -> None
      | x :: _ when List.mem [Private; Public; Protected] x -> Some x
      | _ :: xs -> go xs
      in
      go (pKinds (fun _ -> ()) parameter_visibility env)
    }
  | VariadicParameter _
  | Token _ when text node = "..."
    -> { (param_template node env) with param_is_variadic = true }
  | _ -> missing_syntax "function parameter" node env
and pUserAttribute : user_attribute list parser = fun node env ->
  match syntax node with
  | AttributeSpecification { attribute_specification_attributes; _ } ->
    couldMap attribute_specification_attributes env ~f:begin function
      | { syntax = Attribute { attribute_name; attribute_values; _}; _ } ->
        fun env ->
          { ua_name   = pos_name attribute_name env
          ; ua_params = couldMap ~f:pExpr attribute_values env
          }
      | node -> missing_syntax "attribute" node
    end
  | _ -> missing_syntax "attribute specification" node env

and pUserAttributes env attrs =
  List.concat @@ couldMap ~f:pUserAttribute attrs env
and pAField : afield parser = fun node env ->
  match syntax node with
  | ElementInitializer { element_key; element_value; _ } ->
    AFkvalue (pExpr element_key env, pExpr element_value env)
  | _ -> AFvalue (pExpr node env)
and pString2: expr_location -> node list -> env -> expr list =
  let rec convert_name_to_lvar location env n =
    match syntax n with
    | Token token when Token.kind token = TK.Name ->
      let pos, name = pos_name n env in
      let id = Lvar (pos, "$" ^ name) in
      Some (pos, id)
    | SubscriptExpression { subscript_receiver; subscript_index; _ } ->
      begin match convert_name_to_lvar location env subscript_receiver with
      | Some recv ->
        let index = mpOptional (pExpr ~location) subscript_index env in
        Some (pPos n env, Array_get (recv, index))
      | _ -> None
      end
    | _ -> None in

  let rec aux loc l env acc =
    (* in PHP "${x}" in strings is treated as if it was written "$x",
       here we recognize pattern: Dollar; EmbeddedBracedExpression { QName (Token.Name) }
       produced by FFP and lower it into Lvar.
    *)
    match l with
    | [] -> List.rev acc
    | ({ syntax = Token token; _ })::
      ({ syntax = EmbeddedBracedExpression {
          embedded_braced_expression_expression = e; _ }; _
        } as expr_with_braces)::
      tl when Token.kind token = TK.Dollar ->
        let e =
          begin match convert_name_to_lvar loc env e with
          | Some e -> e
          | None ->
            let e = pExpr ~location:loc expr_with_braces env in
            fst e, Dollar (fst e, BracedExpr e)
          end in
        aux loc tl env (e::acc)
    | x::xs -> aux loc xs env ((pExpr ~location:loc x env)::acc)
  in
  fun loc l env -> aux loc l env []
and pExprL node env =
  (pPos node env, Expr_list (couldMap ~f:pExpr node env))

(* TODO: this function is a hotspot, deep recursion on huge files, attempt more optimization *)
and pMember node env =
  match syntax node with
  | ElementInitializer { element_key; element_value; _ } ->
    (pExpr element_key env, pExpr element_value env)
  | _ -> missing_syntax "darray intrinsic expression element" node env

and pExpr ?location:(location=TopLevel) : expr parser = fun node env ->
  let split_args_varargs arg_list =
    match List.rev (as_list arg_list) with
    | { syntax = DecoratedExpression
        { decorated_expression_decorator =
          { syntax = Token token; _ }
        ; decorated_expression_expression = e
        }
      ; _
      } :: xs when Token.kind token = TK.DotDotDot ->
      let args = List.rev_map xs (fun x -> pExpr x env) in
      let vararg = pExpr e env in
      args, [vararg]
    | _ ->
      let args = couldMap ~f:pExpr arg_list env in
      args, [] in
  let rec pExpr_ : expr_ parser = fun node env ->
    env.recursion_depth := !(env.recursion_depth) + 1;
    if !(env.recursion_depth) > recursion_limit then
      failwith "Expression recursion limit reached";
    let pos = pPos node env in
    let result = match syntax node with
    | LambdaExpression {
        lambda_async; lambda_coroutine; lambda_signature; lambda_body;
        lambda_attribute_spec; _ } ->
      let suspension_kind = mk_suspension_kind node env lambda_async lambda_coroutine in
      let f_params, f_ret =
        match syntax lambda_signature with
        | LambdaSignature { lambda_parameters; lambda_type; _ } ->
          ( couldMap ~f:pFunParam lambda_parameters env
          , mpOptional pHint lambda_type env
          )
        | Token _ -> ([param_template lambda_signature env], None)
        | _ -> missing_syntax "lambda signature" lambda_signature env
      in
      let f_body, yield =
        mpYielding pFunctionBody lambda_body
          (if not (is_compound_statement lambda_body) then env else non_tls env)
      in
      Lfun
      { (fun_template yield node suspension_kind env) with
        f_ret
      ; f_params
      ; f_body
      ; f_user_attributes = pUserAttributes env lambda_attribute_spec
      }

    | BracedExpression        { braced_expression_expression        = expr; _ }
    | EmbeddedBracedExpression
      { embedded_braced_expression_expression = expr; _ }
    | ParenthesizedExpression { parenthesized_expression_expression = expr; _ }
      -> pExpr_ expr env

    | DictionaryIntrinsicExpression
      { dictionary_intrinsic_keyword = kw
      ; dictionary_intrinsic_explicit_type = ty
      ; dictionary_intrinsic_members = members
      ; _ }
    | KeysetIntrinsicExpression
      { keyset_intrinsic_keyword = kw
      ; keyset_intrinsic_explicit_type = ty
      ; keyset_intrinsic_members = members
      ; _ }
    | VectorIntrinsicExpression
      { vector_intrinsic_keyword = kw
      ; vector_intrinsic_explicit_type = ty
      ; vector_intrinsic_members = members
      ; _ }
      ->
      prevent_intrinsic_generic env node ty;
      if env.is_hh_file || env.enable_hh_syntax then
        Collection (pos_name kw env, couldMap ~f:pAField members env)
      else
        (* If php, this is a subscript expression, not a collection. *)
        let subscript_receiver = pExpr kw env in
        let members = couldMap ~f:pExpr members env in
        let subscript_index = match members with
        | [] -> None
        | [x] -> Some x
        | _ ->
          let msg = "Hack keyword " ^ (text kw) ^ " used improperly in php." in
          invariant_failure node msg env in
        Array_get (subscript_receiver, subscript_index)
    | CollectionLiteralExpression
      { collection_literal_name         = collection_name
      ; collection_literal_initializers = members
      ; _ } ->
      let collection_name =
        match syntax collection_name with
        | SimpleTypeSpecifier { simple_type_specifier = class_type } ->
          pos_name class_type env
        | GenericTypeSpecifier { generic_class_type = class_type; _ } ->
          prevent_intrinsic_generic env node class_type;
          pos_name class_type env
        | _ -> pos_name collection_name env in
      Collection (collection_name, couldMap ~f:pAField members env)

    | VarrayIntrinsicExpression
    { varray_intrinsic_members = members
    ; varray_intrinsic_explicit_type = ty; _ } ->
      prevent_intrinsic_generic env node ty;
      Varray (couldMap ~f:pExpr members env)
    | DarrayIntrinsicExpression
    { darray_intrinsic_members = members
    ; darray_intrinsic_explicit_type = ty; _ } ->
      prevent_intrinsic_generic env node ty;
      Darray (couldMap ~f:pMember members env)
    | ArrayIntrinsicExpression { array_intrinsic_members = members; _ }
    | ArrayCreationExpression  { array_creation_members  = members; _ }
    ->
      (* TODO: Or tie in with other intrinsics and post-process to Array *)
      Array (couldMap ~f:pAField members env)

    | ListExpression { list_members = members; _ } ->
      (* TODO: Or tie in with other intrinsics and post-process to List *)
      let pBinderOrIgnore node env = match syntax node with
        | Missing -> (Pos.none, Omitted)
        | _ -> pExpr node env
      in
      List (couldMap ~f:pBinderOrIgnore members env)

    | EvalExpression  { eval_keyword  = recv; eval_argument       = args; _ }
    | EmptyExpression { empty_keyword = recv; empty_argument      = args; _ }
    | IssetExpression { isset_keyword = recv; isset_argument_list = args; _ }
    | TupleExpression
      { tuple_expression_keyword = recv
      ; tuple_expression_items   = args
      ; _ }
      ->
      let pos_if_has_parens =
        match syntax recv with
        | ParenthesizedExpression _ -> Some (pPos recv env)
        | _ -> None in
      let recv = pExpr recv env in
      let recv =
        match snd recv, pos_if_has_parens with
        | (Obj_get _ | Class_get _), Some p -> p, ParenthesizedExpr recv
        | _ -> recv in
      let args, varargs = split_args_varargs args in
      Call (recv, [], args, varargs)
    | FunctionCallExpression
      { function_call_receiver = recv
      ; function_call_argument_list =
        { syntax = SyntaxList
          [ { syntax = ListItem
              { list_item =
                { syntax = LiteralExpression { literal_expression = expr }
                ; _
                }
              ; _
              }
            ; _
          } ]
        ; _
        }
      ; _
      } when text recv = "__hhas_adata"
        && token_kind expr = Some TK.NowdocStringLiteral ->
      let literal_expression_pos = pPos expr env in
      let s =
        expr
          |> source_text
          |> SourceText.text
          |> extract_unquoted_string
              ~start:(start_offset expr)
              ~len:(width expr) in
      Call (
        pExpr recv env,
        [],
        [ literal_expression_pos, String s ],
        []
      )
    | FunctionCallExpression
      { function_call_receiver      = recv
      ; function_call_argument_list = args
      ; _ }
      ->
      let hints =
        begin match (syntax recv) with
          | GenericTypeSpecifier { generic_argument_list; _ } ->
            begin match syntax generic_argument_list with
              | TypeArguments { type_arguments_types; _ }
                -> couldMap ~f:pTarg type_arguments_types env
              | _ -> []
            end
          | _ -> []
        end
      in
      (* preserve parens on receiver of call expression
         to allow distinguishing between
         ($a->b)() // invoke on callable property
         $a->b()   // method call *)
      let pos_if_has_parens =
        match syntax recv with
        | ParenthesizedExpression _ -> Some (pPos recv env)
        | _ -> None in
      let recv = pExpr recv env in
      let recv =
        match snd recv, pos_if_has_parens with
        | (Obj_get _ | Class_get _), Some p -> p, ParenthesizedExpr recv
        | _ -> recv in
      let args, varargs = split_args_varargs args in
      Call (recv, hints, args, varargs)
    | FunctionCallWithTypeArgumentsExpression
      { function_call_with_type_arguments_receiver = recv
      ; function_call_with_type_arguments_type_args = type_args
      ; function_call_with_type_arguments_argument_list = args
      ; _ }
      ->
      let hints =
        begin match (syntax type_args) with
          | TypeArguments { type_arguments_types; _ } ->
            couldMap ~f:pTarg type_arguments_types env
          | _ -> missing_syntax "type arguments" type_args env
        end in
      Call (pExpr recv env, hints, couldMap ~f:pExpr args env, [])
    | QualifiedName _ ->
      if in_string location
      then
        let _, n = pos_qualified_name node env in
        String n
      else Id (pos_qualified_name node env)

    | VariableExpression { variable_expression } ->
      Lvar (pos_name variable_expression env)

    | PipeVariableExpression _ ->
      Lvar (pos, "$$")

    | InclusionExpression { inclusion_require; inclusion_filename } ->
      Import
      ( pImportFlavor inclusion_require env
      , pExpr inclusion_filename env
      )

    | MemberSelectionExpression
      { member_object   = recv
      ; member_operator = op
      ; member_name     = name
      }
    | SafeMemberSelectionExpression
      { safe_member_object   = recv
      ; safe_member_operator = op
      ; safe_member_name     = name
      }
    | EmbeddedMemberSelectionExpression
      { embedded_member_object   = recv
      ; embedded_member_operator = op
      ; embedded_member_name     = name
      }
      ->
        let recv = pExpr recv env in
        let name = pExpr ~location:MemberSelect name env in
        let op = pNullFlavor op env in
        Obj_get (recv, name, op)

    | PrefixUnaryExpression
      { prefix_unary_operator = operator
      ; prefix_unary_operand  = operand
      }
    | PostfixUnaryExpression
      { postfix_unary_operand  = operand
      ; postfix_unary_operator = operator
      }
    | DecoratedExpression
      { decorated_expression_expression = operand
      ; decorated_expression_decorator  = operator
      }
      ->
        let expr = pExpr operand env in
        (**
         * FFP does not destinguish between ++$i and $i++ on the level of token
         * kind annotation. Prevent duplication by switching on `postfix` for
         * the two operatores for which AST /does/ differentiate between
         * fixities.
         *)
        let postfix = kind node = SyntaxKind.PostfixUnaryExpression in
        let kind = token_kind operator in
        (match kind with
        | Some TK.PlusPlus   when postfix -> Unop (Upincr, expr)
        | Some TK.MinusMinus when postfix -> Unop (Updecr, expr)
        | Some TK.PlusPlus                -> Unop (Uincr,  expr)
        | Some TK.MinusMinus              -> Unop (Udecr,  expr)
        | Some TK.Exclamation             -> Unop (Unot,   expr)
        | Some TK.Tilde                   -> Unop (Utild,  expr)
        | Some TK.Plus                    -> Unop (Uplus,  expr)
        | Some TK.Minus                   -> Unop (Uminus, expr)
        | Some TK.Ampersand               -> Unop (Uref,   expr)
        | Some TK.At     when env.codegen -> Unop (Usilence, expr)
        | Some TK.At                      -> snd expr
        | Some TK.Inout                   -> Callconv (Pinout, expr)
        | Some TK.Await                   -> Await expr
        | Some TK.Suspend                 -> Suspend expr
        | Some TK.Clone                   -> Clone expr
        | Some TK.Print                   ->
          Call ((pos, Id (pos, "echo")), [], [expr], [])
        | Some TK.Dollar                  ->
          (match expr with
          | p, String s
          | p, Int s
          | p, Float s when location <> InGlobalVar ->
            if not env.codegen
            then raise_parsing_error env (`Node operator) SyntaxError.invalid_variable_name;
            Lvar (p, "$" ^ s)
          | _ -> Dollar expr
          )

        | _ -> missing_syntax "unary operator" node env
        )
    | BinaryExpression
      { binary_left_operand; binary_operator; binary_right_operand }
      ->
        let bop_ast_node =
          let rlocation =
            if location = AsStatement && token_kind binary_operator = Some TK.Equal
            then RightOfAssignment else TopLevel in
          pBop binary_operator env
            (pExpr binary_left_operand  env)
            (pExpr binary_right_operand ~location:rlocation env)
        in
        begin match env.inside_declare, bop_ast_node with
        | false, Binop (Eq _, lhs, _) ->
          Ast_check.check_lvalue (fun pos error -> raise_parsing_error env (`Pos pos) error) lhs
        | _ -> ()
        end;
        bop_ast_node

    | Token t ->
      (match location, Token.kind t with
      | MemberSelect, TK.Variable -> Lvar (pos_name node env)
      | InDoubleQuotedString, TK.HeredocStringLiteral
      | InDoubleQuotedString, TK.HeredocStringLiteralHead
      | InDoubleQuotedString, TK.HeredocStringLiteralTail ->
          String (Php_escaping.unescape_heredoc (text node))
      | InDoubleQuotedString, _ -> String (unesc_dbl (text node))
      | InBacktickedString, _ -> String (Php_escaping.unescape_backtick (text node))
      | MemberSelect, _
      | InGlobalVar, _
      | TopLevel, _
      | AsStatement, _
      | RightOfAssignment, _
      | RightOfReturn, _ -> Id (pos_name node env)
      )

    | YieldExpression { yield_operand; _ } ->
      env.saw_yield <- true;
      if location <> AsStatement && location <> RightOfAssignment
      then raise_parsing_error env (`Node node) SyntaxError.invalid_yield;
      if text yield_operand = "break"
      then Yield_break
      else
      if is_missing yield_operand
      then Yield (AFvalue (pos, Null))
      else Yield (pAField yield_operand env)

    | YieldFromExpression { yield_from_operand; _ } ->
      env.saw_yield <- true;
      if location <> AsStatement && location <> RightOfAssignment && location <> RightOfReturn
      then raise_parsing_error env (`Node node) SyntaxError.invalid_yield_from;
      Yield_from (pExpr yield_from_operand env)

    | DefineExpression { define_keyword; define_argument_list; _ } -> Call
      ( (let name = pos_name define_keyword env in fst name, Id name)
      , []
      , List.map ~f:(fun x -> pExpr x env) (as_list define_argument_list)
      , []
      )

    | ScopeResolutionExpression
      { scope_resolution_qualifier; scope_resolution_name; _ } ->
      let qual =
        match pExpr scope_resolution_qualifier env with
        | p, Lvar v when not env.codegen -> p, Id v
        | qual -> qual
      in
      begin match syntax scope_resolution_name with
      | Token token when Token.kind token = TK.Variable ->
        let name =
          ( pPos scope_resolution_name env
          , Lvar (pos_name scope_resolution_name env)
          )
        in
        Class_get (qual, name)
      | _ ->
        let name = pExpr scope_resolution_name env in
        begin match name with
        | p, String id | _, Id (p, id) -> Class_const (qual, (p, id))
        | _ -> Class_get (qual, name)
        end
      end
    | CastExpression { cast_type; cast_operand; _ } ->
      Cast (pHint cast_type env, pExpr cast_operand env)
    | ConditionalExpression
      { conditional_test; conditional_consequence; conditional_alternative; _ }
      -> Eif
      ( pExpr conditional_test env
      , mpOptional pExpr conditional_consequence env
      , pExpr conditional_alternative env
      )
    | SubscriptExpression { subscript_receiver; subscript_index; _ } ->
      Array_get
      ( pExpr subscript_receiver env
      , mpOptional pExpr subscript_index env
      )
    | EmbeddedSubscriptExpression
      { embedded_subscript_receiver; embedded_subscript_index; _ } ->
      Array_get
      ( pExpr embedded_subscript_receiver env
      , mpOptional (pExpr ~location) embedded_subscript_index env
      )
    | ShapeExpression { shape_expression_fields; _ } ->
      Shape (
        couldMap ~f:(mpShapeExpressionField pExpr) shape_expression_fields env
      )
    | ObjectCreationExpression { object_creation_object = obj; _ } ->
      pExpr_ obj env
    | ConstructorCall
      { constructor_call_argument_list; constructor_call_type; _ } ->
      let args, varargs = split_args_varargs constructor_call_argument_list in
      let e, hl = match syntax constructor_call_type with
        | GenericTypeSpecifier { generic_class_type; generic_argument_list } ->
          let name = pos_name generic_class_type env in
          let hints =
            match syntax generic_argument_list with
            | TypeArguments { type_arguments_types; _ }
              -> couldMap ~f:pTarg type_arguments_types env
            | _ ->
              missing_syntax "generic type arguments" generic_argument_list env
          in
          (fst name, Id name), hints
        | SimpleTypeSpecifier _ ->
          let name = pos_name constructor_call_type env in
          (fst name, Id name), []
        | _ -> pExpr constructor_call_type env, [] in
      New
      ( e
      , hl
      , args
      , varargs
      )
    | AnonymousClass
      { anonymous_class_argument_list = args
      ; anonymous_class_extends_list = extends
      ; anonymous_class_implements_list = implements
      ; anonymous_class_body =
        { syntax = ClassishBody { classish_body_elements = elts; _ }; _ }
      ; _ } ->
      let args, varargs = split_args_varargs args in
      let c_mode = mode_annotation env.fi_mode in
      let c_user_attributes = [] in
      let c_final = false in
      let c_is_xhp = false in
      let c_name = pos, "anonymous" in
      let c_tparams = [] in
      let c_extends = couldMap ~f:pHint extends env in
      let c_implements = couldMap ~f:pHint implements env in
      let c_body = List.concat (couldMap ~f:pClassElt elts env) in
      let c_namespace = Namespace_env.empty env.parser_options in
      let c_enum = None in
      let c_span = pPos node env in
      let c_kind = Cnormal in
      let c_doc_comment = None in
      let cls =
        { c_mode
        ; c_user_attributes
        ; c_final
        ; c_is_xhp
        ; c_name
        ; c_tparams
        ; c_extends
        ; c_implements
        ; c_body
        ; c_namespace
        ; c_enum
        ; c_span
        ; c_kind
        ; c_doc_comment
        } in
      NewAnonClass (args, varargs, cls)
    | GenericTypeSpecifier
      { generic_class_type
      ; _
      } ->
        let name = pos_name generic_class_type env in
        (* TODO: We are dropping generics here *)
        Id name
    | LiteralExpression { literal_expression = expr } ->
      (match syntax expr with
      | Token _ ->
        let s = text expr in
        (match location, token_kind expr with
        (* TODO(T21285960): Inside strings, int indices "should" be string indices *)
        | InDoubleQuotedString, _ when env.codegen -> String (mkStr env expr unesc_dbl s)
        | InBacktickedString, _ when env.codegen ->
          String (mkStr env expr Php_escaping.unescape_backtick s)
        | _, Some TK.OctalLiteral
          when is_typechecker env &&
            String_utils.fold_left ~f:(fun b c -> b || c = '8' || c = '9') ~acc:false s ->
          raise_parsing_error env (`Node node) SyntaxError.invalid_octal_integer;
          missing_syntax "octal int" expr env (* this should never get hit *)
        | _, Some TK.DecimalLiteral
        | _, Some TK.OctalLiteral
        | _, Some TK.HexadecimalLiteral
        (* We allow underscores while lexing the integer literals. This gets rid of them before
         * the literal is created. *)
        | _, Some TK.BinaryLiteral             -> Int    (Str.global_replace (underscore) "" s)
        | _, Some TK.FloatingLiteral           -> Float  s
        | _, Some TK.SingleQuotedStringLiteral ->
          String (mkStr env expr Php_escaping.unescape_single s)
        | _, Some TK.DoubleQuotedStringLiteral ->
          String (mkStr env expr Php_escaping.unescape_double s)
        | _, Some TK.HeredocStringLiteral      ->
          String (mkStr env expr Php_escaping.unescape_heredoc s)
        | _, Some TK.NowdocStringLiteral       ->
          String (mkStr env expr Php_escaping.unescape_nowdoc s)
        | _, Some TK.NullLiteral               ->
          if not env.codegen && s <> String.lowercase_ascii s then
            Lint.lowercase_constant pos s;
          Null
        | _, Some TK.BooleanLiteral            ->
          if not env.codegen && s <> String.lowercase_ascii s then
            Lint.lowercase_constant pos s;
          (match String.lowercase_ascii s with
          | "false" -> False
          | "true"  -> True
          | _       -> missing_syntax ("boolean (not: " ^ s ^ ")") expr env
          )
        | _, Some TK.ExecutionStringLiteral ->
          if is_typechecker env then
            raise_parsing_error env (`Node node) SyntaxError.execution_operator;
          Execution_operator [pos, String (mkStr env expr Php_escaping.unescape_backtick s)]
        | _ -> missing_syntax "literal" expr env
        )
      | SyntaxList ({ syntax = Token token; _ } :: _ as ts)
        when Token.kind token = TK.ExecutionStringLiteralHead ->
        if is_typechecker env then
          raise_parsing_error env (`Node node) SyntaxError.execution_operator;
        Execution_operator (pString2 InBacktickedString (prepString2 env ts) env)
      | SyntaxList ts -> String2 (pString2 InDoubleQuotedString (prepString2 env ts) env)
      | _ -> missing_syntax "literal expression" expr env
      )
    | PrefixedStringExpression
      { prefixed_string_name = name
      ; prefixed_string_str = str } ->
        (* Temporarily allow only`re`- prefixed strings *)
        let name_text = text name in
        if name_text <> "re"
        then raise_parsing_error env (`Node node) SyntaxError.non_re_prefix;
        PrefixedString (text name, pExpr str env)
    | InstanceofExpression
      { instanceof_left_operand; instanceof_right_operand; _ } ->
      let ty =
        match pExpr instanceof_right_operand env with
        | p, Class_const ((_, pid), (_, "")) -> p, pid
        | ty -> ty
      in
      InstanceOf (pExpr instanceof_left_operand env, ty)
      (* TODO: Priority fix? *)
      (*match pExpr instanceof_left_operand env with
      | p, Unop (o,e) -> Unop (0, (p, InstanceOf (e, ty)))
      | e -> InstanceOf (e, ty)
      *)
    | IsExpression
      { is_left_operand; is_right_operand; _ } ->
      Is (pExpr is_left_operand env, pHint is_right_operand env)
    | AsExpression
      { as_left_operand; as_right_operand; _ } ->
      As (pExpr as_left_operand env, pHint as_right_operand env, false)
    | NullableAsExpression
      { nullable_as_left_operand; nullable_as_right_operand; _ } ->
      As (pExpr nullable_as_left_operand env,
          pHint nullable_as_right_operand env,
          true)
    | AnonymousFunction
      { anonymous_attribute_spec = attribute_spec
      ; anonymous_static_keyword
      ; anonymous_async_keyword
      ; anonymous_coroutine_keyword
      ; anonymous_ampersand
      ; anonymous_parameters
      ; anonymous_type
      ; anonymous_use
      ; anonymous_body
      ; _ }
    | Php7AnonymousFunction
      { php7_anonymous_attribute_spec = attribute_spec
      ; php7_anonymous_static_keyword = anonymous_static_keyword
      ; php7_anonymous_async_keyword = anonymous_async_keyword
      ; php7_anonymous_coroutine_keyword = anonymous_coroutine_keyword
      ; php7_anonymous_ampersand = anonymous_ampersand
      ; php7_anonymous_parameters = anonymous_parameters
      ; php7_anonymous_type = anonymous_type
      ; php7_anonymous_use = anonymous_use
      ; php7_anonymous_body = anonymous_body
      ; _ } ->
        begin match syntax node with
          | Php7AnonymousFunction _ when is_typechecker env ->
            raise_parsing_error env (`Node node) SyntaxError.php7_anonymous_function
          | _ -> ()
        end;
        let pArg node env =
          match syntax node with
          | PrefixUnaryExpression
            { prefix_unary_operator = op; prefix_unary_operand = v } ->
              pos_name v env, token_kind op = Some TK.Ampersand
          | Token _ -> pos_name node env, false
          | _ -> missing_syntax "use variable" node env
          in
        let pUse node =
          match syntax node with
          | AnonymousFunctionUseClause { anonymous_use_variables; _ } ->
            couldMap ~f:pArg anonymous_use_variables
          | _ -> fun _env -> []
          in
        let suspension_kind =
          mk_suspension_kind
            node
            env
            anonymous_async_keyword
            anonymous_coroutine_keyword in
        let f_body, yield =
          mpYielding pFunctionBody anonymous_body (non_tls env)
        in
        let doc_comment = match extract_docblock node with
          | Some _ as doc_comment -> doc_comment
          | None -> top_docblock() in
        let f_ret_by_ref = is_ret_by_ref anonymous_ampersand in
        let user_attributes = pUserAttributes env attribute_spec in
        Efun
        ( { (fun_template yield node suspension_kind env) with
            f_ret         = mpOptional pHint anonymous_type env
          ; f_params      = couldMap ~f:pFunParam anonymous_parameters env
          ; f_body
          ; f_static      = not (is_missing anonymous_static_keyword)
          ; f_ret_by_ref
          ; f_doc_comment = doc_comment
          ; f_user_attributes = user_attributes
          }
        , try pUse anonymous_use env with _ -> []
        )

    | AwaitableCreationExpression
      { awaitable_async; awaitable_coroutine; awaitable_compound_statement;
        awaitable_attribute_spec; } ->
      let suspension_kind =
        mk_suspension_kind node env awaitable_async awaitable_coroutine in
      let blk, yld = mpYielding pFunctionBody awaitable_compound_statement env in
      let user_attributes =
        let attrs = pUserAttributes env awaitable_attribute_spec in
        (* inject uaRxOfScope into the list of arguments if
           it does not already have it or any other reactivity annotations *)
        let module UA = Naming_special_names.UserAttributes in
        let check attr =
          attr = UA.uaRxOfScope || attr = UA.uaReactive ||
          attr = UA.uaShallowReactive || attr = UA.uaLocalReactive in
        (* for now add uaRxOfScope only for typechecker *)
        if not (is_typechecker env) ||
           Core_list.exists attrs ~f:(fun { ua_name = (_, n); _ } -> check n)
        then attrs
        else { ua_name = (Pos.none, UA.uaRxOfScope); ua_params = [] } :: attrs in
      let body =
        { (fun_template yld node suspension_kind env) with
           f_body = mk_noop (pPos awaitable_compound_statement env) blk;
           f_user_attributes = user_attributes }
      in
      Call ((pPos node env, Lfun body), [], [], [])
    | XHPExpression
      { xhp_open =
        { syntax = XHPOpen { xhp_open_name; xhp_open_attributes; _ }; _ }
      ; xhp_body = body
      ; _ } ->
      env.ignore_pos <- false;
      let name =
        let pos, name = pos_name xhp_open_name env in
        (pos, ":" ^ name)
      in
      let combine b e =
        make_token Token.(
          concatenate b e
        )
      in
      let aggregate_tokens node =
        let rec search = function (* scroll through non-token things *)
        | [] -> []
        | t :: xs when token_kind t = Some TK.XHPComment -> search xs
        | { syntax = Token b; _ } as t :: xs -> track t b None xs
        | x :: xs -> x :: search xs
        and track t b oe = function (* keep going through consecutive tokens *)
        | { syntax = Token e; _ } :: xs
          when Token.kind e <> TK.XHPComment -> track t b (Some e) xs
        | xs -> Option.value_map oe ~default:t ~f:(combine b) :: search xs
        in
        search (as_list node)
      in
      let pEmbedded escaper node env =
        match syntax node with
        | Token token
          when env.codegen && Token.kind token = TK.XHPStringLiteral ->
          let p = pPos node env in
          (* for XHP string literals (attribute values) just extract
              value from quotes and decode HTML entities  *)
          let text =
            Html_entities.decode @@ get_quoted_content (full_text node) in
          p, String text
        | Token token
          when env.codegen && Token.kind token = TK.XHPBody ->
          let p = pPos node env in
          (* for XHP body - only decode HTML entities *)
          let text = Html_entities.decode @@ unesc_xhp (full_text node) in
          p, String text
        | Token _ ->
          let p = pPos node env in
          p, String (escaper (full_text node))
        | _ ->
          match pExpr node env with
          | _, BracedExpr e -> e
          | e -> e
      in
      let pAttr = fun node env ->
        match syntax node with
        | XHPSimpleAttribute { xhp_simple_attribute_name; xhp_simple_attribute_expression; _ } ->
          let name = pos_name xhp_simple_attribute_name env in
          let expr =
            if is_braced_expression xhp_simple_attribute_expression
            && env.fi_mode = FileInfo.Mdecl && not env.codegen
            then Pos.none, Null
            else pEmbedded unesc_xhp_attr xhp_simple_attribute_expression env
          in
          Xhp_simple (name, expr)
        | XHPSpreadAttribute { xhp_spread_attribute_expression; _ } ->
          Xhp_spread ( pEmbedded unesc_xhp_attr xhp_spread_attribute_expression env )
        | _ -> missing_syntax "XHP attribute" node env
      in
      let attrs = couldMap ~f:pAttr xhp_open_attributes env in
      let exprs =
        List.map ~f:(fun x -> pEmbedded unesc_xhp x env) (aggregate_tokens body)
      in
      Xml (name, attrs, exprs)
    (* FIXME; should this include Missing? ; "| Missing -> Null" *)
    | _ -> missing_syntax ?fallback:(Some Null) "expression" node env
    in
    env.recursion_depth := !(env.recursion_depth) - 1;
    assert (!(env.recursion_depth) >= 0);
    result
  in
  let outer_unsafes = env.unsafes in
  let is_safe =
    (* when in codegen ignore UNSAFE_EXPRs *)
    if env.codegen then true
    else
    match leading_token node with
    | None -> true
    | Some t ->
      let us =
        if Token.has_trivia_kind t TriviaKind.UnsafeExpression
        then Token.filter_leading_trivia_by_kind t TriviaKind.UnsafeExpression
        else []
      in
      let f acc u = ISet.add (Trivia.start_offset u) acc in
      let us = List.fold ~f ~init:ISet.empty us in
      (* Have we already dealt with 'these' UNSAFE_EXPRs? *)
      let us = ISet.diff us outer_unsafes in
      let safe = ISet.is_empty us in
      if not safe then env.unsafes <- ISet.union us outer_unsafes;
      safe
  in
  let result =
    match syntax node with
    | BracedExpression        { braced_expression_expression        = expr; _ }
    | ParenthesizedExpression { parenthesized_expression_expression = expr; _ }
      -> (**
          * Peeling off braced or parenthesised expresions. When there is XHP
          * inside, we want the XHP node to have this own positions, rather than
          * those of enclosing parenthesised/braced expressions.
          *)
         let inner = pExpr ~location expr env in
         if Syntax.is_braced_expression node
         then
           (* We elide the braces in {$x}, as it makes compilation easier *)
           begin match inner with
           | _, (Lvar _ | String _ | Int _ | Float _ ) -> inner
           | p, _ -> p, BracedExpr inner
           end
         else inner
    | _ ->
      (**
       * Since we need positions in XHP, regardless of the ignore_pos flag, we
       * parenthesise the call to pExpr_ so that the XHP expression case can
       * flip the switch. The key part is that `pPos` happens before the old
       * setting is restored.
       *)
      let local_ignore_pos = env.ignore_pos in
      let expr_ = pExpr_ node env in
      let p = pPos node env in
      env.ignore_pos <- local_ignore_pos;
      p, expr_
  in
  if is_safe then result else fst result, Unsafeexpr result
and pBlock : block parser = fun node env ->
   let rec fix_last acc = function
   | x :: (_ :: _ as xs) -> fix_last (x::acc) xs
   | [_, Block block] -> List.rev acc @ block
   | stmt -> List.rev acc @ stmt
   in
   let stmt = pStmtUnsafe node env in
   fix_last [] stmt
and pFunctionBody : block parser = fun node env ->
  match syntax node with
  | Missing -> []
  | CompoundStatement
    { compound_statements = {syntax = Missing; _}
    ; compound_right_brace = { syntax = Token t; _ }
    ; _} ->
      [ Pos.none
      , if Token.has_trivia_kind t TriviaKind.Unsafe then Unsafe else Noop
      ]
  | CompoundStatement {compound_statements = {syntax = SyntaxList [t]; _}; _}
    when Syntax.is_specific_token TK.Yield t ->
    env.saw_yield <- true;
    [ Pos.none, Noop ]
  | CompoundStatement _ ->
    let block = pBlock node env in
    if not env.top_level_statements
    && (  env.fi_mode = FileInfo.Mdecl && not env.codegen
       || env.quick_mode)
    then [ Pos.none, Noop ]
    else block
  | _ ->
    let p, r = pExpr node env in
    [p, Return (Some (p, r))]
and pStmtUnsafe : stmt list parser = fun node env ->
  let stmt = pStmt node env in
  match leading_token node with
  | Some t when Token.has_trivia_kind t TriviaKind.Unsafe -> [Pos.none, Unsafe; stmt]
  | _ -> [stmt]
and pStmt : stmt parser = fun node env ->
  extract_and_push_docblock node;
  let pos = pPos node env in
  let result = match syntax node with
  | AlternateElseClause _ | AlternateIfStatement _ | AlternateElseifClause _
  | AlternateLoopStatement _  | AlternateSwitchStatement _ when is_typechecker env ->
    raise_parsing_error env (`Node node) SyntaxError.alternate_control_flow;
    missing_syntax "alternative control flow" node env (* this should never get hit *)
  | SwitchStatement { switch_expression=expr; switch_sections=sections; _ }
  | AlternateSwitchStatement { alternate_switch_expression=expr;
    alternate_switch_sections=sections; _ } ->
    let pSwitchLabel : (block -> case) parser = fun node env cont ->
      match syntax node with
      | CaseLabel { case_expression; _ } ->
        Case (pExpr case_expression env, cont)
      | DefaultLabel _ -> Default cont
      | _ -> missing_syntax "pSwitchLabel" node env
    in
    let pSwitchSection : case list parser = fun node env ->
      match syntax node with
      | SwitchSection
        { switch_section_labels
        ; switch_section_statements
        ; switch_section_fallthrough
        } ->
        let rec null_out cont = function
          | [x] -> [x cont]
          | (x::xs) -> x [] :: null_out cont xs
          | _ -> raise_parsing_error env (`Node node) "Malformed block result"; []
        in
        let blk = List.concat @@
          couldMap ~f:pStmtUnsafe switch_section_statements env in
        let blk =
          if is_missing switch_section_fallthrough
          then blk
          else blk @ [Pos.none, Fallthrough]
        in
        null_out blk (couldMap ~f:pSwitchLabel switch_section_labels env)
      | _ -> missing_syntax "switch section" node env
    in
    pos,
    Switch
    ( pExpr expr env
    , List.concat @@ couldMap ~f:pSwitchSection sections env
    )
  | IfStatement
    { if_condition=cond;
      if_statement=stmt;
      if_elseif_clauses=elseif_clause;
      if_else_clause=else_clause; _ }
  | AlternateIfStatement
    { alternate_if_condition=cond;
      alternate_if_statement=stmt;
      alternate_if_elseif_clauses=elseif_clause;
      alternate_if_else_clause=else_clause; _ } ->
    (* Because consistency is for the weak-willed, Parser_hack does *not*
     * produce `Noop`s for compound statements **in if statements**
     *)
    let if_condition = pExpr cond env in
    let if_statement = unwrap_extra_block @@ pStmtUnsafe stmt env in
    let if_elseif_statement =
      let pElseIf : (block -> block) parser = fun node env ->
        match syntax node with
        | ElseifClause { elseif_condition=ei_cond; elseif_statement=ei_stmt; _ }
        | AlternateElseifClause { alternate_elseif_condition=ei_cond;
          alternate_elseif_statement=ei_stmt; _ } ->
          fun next_clause ->
            let elseif_condition = pExpr ei_cond env in
            let elseif_statement = unwrap_extra_block @@ pStmtUnsafe ei_stmt env in
            [ pos, If (elseif_condition, elseif_statement, next_clause) ]
        | _ -> missing_syntax "elseif clause" node env
      in
      List.fold_right ~f:(@@)
          (couldMap ~f:pElseIf elseif_clause env)
          ~init:( match syntax else_clause with
            | ElseClause { else_statement=e_stmt; _ }
            | AlternateElseClause { alternate_else_statement=e_stmt; _ } ->
              unwrap_extra_block @@ pStmtUnsafe e_stmt env
            | Missing -> [Pos.none, Noop]
            | _ -> missing_syntax "else clause" else_clause env
          )
    in
    pos, If (if_condition, if_statement, if_elseif_statement)
  | ExpressionStatement { expression_statement_expression; _ } ->
    if is_missing expression_statement_expression
    then pos, Noop
    else pos, Expr (pExpr ~location:AsStatement expression_statement_expression env)
  | CompoundStatement { compound_statements; compound_right_brace; _ } ->
    let tail =
      match leading_token compound_right_brace with
      | Some t when Token.has_trivia_kind t TriviaKind.Unsafe -> [pos, Unsafe]
      | _ -> []
    in
    handle_loop_body pos compound_statements tail env
  | AlternateLoopStatement { alternate_loop_statements; _ } ->
    handle_loop_body pos alternate_loop_statements [] env
  | SyntaxList _ ->
    handle_loop_body pos node [] env
  | ThrowStatement { throw_expression; _ } ->
    pos, Throw (pExpr throw_expression env)
  | DoStatement { do_body; do_condition; _ } ->
    pos, Do (pBlock do_body env, pExpr do_condition env)
  | WhileStatement { while_condition; while_body; _ } ->
    pos, While (pExpr while_condition env, unwrap_extra_block @@ pStmtUnsafe while_body env)
  | DeclareDirectiveStatement { declare_directive_expression; _ } ->
    pos, Declare (false, pExpr declare_directive_expression { env with inside_declare = true }, [])
  | DeclareBlockStatement { declare_block_expression; declare_block_body; _ } ->
    let env = { env with inside_declare = true } in
    pos, Declare (true, pExpr declare_block_expression env,
             unwrap_extra_block @@ pStmtUnsafe declare_block_body env)
  | UsingStatementBlockScoped
    { using_block_await_keyword
    ; using_block_expressions
    ; using_block_body
    ; _ } ->
    pos, Using {
      us_is_block_scoped = true;
      us_has_await = not (is_missing using_block_await_keyword);
      us_expr = pExprL using_block_expressions env;
      us_block = pBlock using_block_body env;
    }
  | UsingStatementFunctionScoped
    { using_function_await_keyword
    ; using_function_expression
    ; _ } ->
    (* All regular function scoped using statements should
     * be rewritten by this point
     * If this gets run, it means that this using statement is the only one
     * in the block, hence it is not in a compound statement *)
     pos, Using {
       us_is_block_scoped = false;
       us_has_await = not (is_missing using_function_await_keyword);
       us_expr = pExpr using_function_expression env;
       us_block = [Pos.none, Noop];
     }
  | LetStatement
    { let_statement_name; let_statement_type; let_statement_initializer; _ } ->
    let id = pos_name let_statement_name env in
    let ty = mpOptional pHint let_statement_type env in
    let expr = pSimpleInitializer let_statement_initializer env in
    pos, Let (id, ty, expr)
  | ForStatement
    { for_initializer; for_control; for_end_of_loop; for_body; _ } ->
    let ini = pExprL for_initializer env in
    let ctr = pExprL for_control env in
    let eol = pExprL for_end_of_loop env in
    let blk = unwrap_extra_block @@ pStmtUnsafe for_body env in
    pos, For (ini, ctr, eol, blk)
  | ForeachStatement
    { foreach_collection
    ; foreach_await_keyword
    ; foreach_key
    ; foreach_value
    ; foreach_body
    ; _ } ->
    let col = pExpr foreach_collection env in
    let akw =
      match syntax foreach_await_keyword with
      | Token token when Token.kind token = TK.Await ->
        Some (pPos foreach_await_keyword env)
      | _ -> None
    in
    let akv =
      let value = pExpr foreach_value env in
      match syntax foreach_key with
        | Missing -> As_v value
        | _ ->
          let key = pExpr foreach_key env in
          As_kv (key, value)
    in
    let blk = unwrap_extra_block @@ pStmtUnsafe foreach_body env in
    pos, Foreach (col, akw, akv, blk)
  | TryStatement
    { try_compound_statement; try_catch_clauses; try_finally_clause; _ } ->
    pos, Try
    ( pBlock try_compound_statement env
    , couldMap try_catch_clauses env ~f:begin fun node env ->
      match syntax node with
      | CatchClause { catch_type; catch_variable; catch_body; _ } ->
        ( pos_name catch_type env
        , pos_name catch_variable env
        , mpStripNoop pBlock catch_body env
        )
      | _ -> missing_syntax "catch clause" node env
      end
    , match syntax try_finally_clause with
      | FinallyClause { finally_body; _ } ->
        pBlock finally_body env
      | _ -> []
    )
  | FunctionStaticStatement { static_declarations; _ } ->
    let pStaticDeclarator node env =
      match syntax node with
      | StaticDeclarator { static_name; static_initializer } ->
        let lhs =
          match pExpr static_name env with
          | p, Id (p', s) -> p, Lvar (p', s)
          | x -> x
        in
        (match syntax static_initializer with
        | SimpleInitializer { simple_initializer_value; _ } ->
          ( pPos static_initializer env
          , Binop (Eq None, lhs, pExpr simple_initializer_value env)
          )
        | _ -> lhs
        )
      | _ -> missing_syntax "static declarator" node env
    in
    pos, Static_var (couldMap ~f:pStaticDeclarator static_declarations env)
  | ReturnStatement { return_expression; _ } ->
    let expr = match syntax return_expression with
      | Missing -> None
      | _ -> Some (pExpr ~location:RightOfReturn return_expression env)
    in
    pos, Return (expr)
  | Syntax.GotoLabel { goto_label_name; _ } ->
    let pos_label = pPos goto_label_name env in
    let label_name = text goto_label_name in
    pos, Ast.GotoLabel (pos_label, label_name)
  | GotoStatement { goto_statement_label_name; _ } ->
    pos, Goto  (pos_name goto_statement_label_name env)
  | EchoStatement  { echo_keyword  = kw; echo_expressions = exprs; _ }
  | UnsetStatement { unset_keyword = kw; unset_variables  = exprs; _ }
    -> pos, Expr
      ( pPos node env
      , Call
        ( (match syntax kw with
          | QualifiedName _
          | SimpleTypeSpecifier _
          | Token _
            -> let name = pos_name kw env in fst name, Id name
          | _ -> missing_syntax "id" kw env
          )
        , []
        , couldMap ~f:pExpr exprs env
        , []
      ))
  | BreakStatement { break_level=level; _ } ->
    pos, Break (pBreak_or_continue_level env level)
  | ContinueStatement { continue_level=level; _ } ->
    pos, Continue (pBreak_or_continue_level env level)
  | GlobalStatement { global_variables; _ } ->
    pos, Global_var (couldMap ~f:(pExpr ~location:InGlobalVar) global_variables env)
  | MarkupSection _ -> pMarkup node env
  | _ when env.max_depth > 0 && env.codegen ->
    (* OCaml optimisers; Forgive them, for they know not what they do!
     *
     * The max_depth is only there to stop the *optimised* version from an
     * unbounded recursion. Sad times.
     *
     * As for the code gen check, we only want to have a blanket assumption that
     * a statement we don't recognize is an inline definition when we're in
     * code generation mode, since typechecking runs with env.codegen set to
     * false, and typechecking needs to support ASTs with missing nodes to
     * support IDE features, such as autocomplete.
     *)
    let outer_max_depth = env.max_depth in
    let () = env.max_depth <- outer_max_depth - 1 in
    let result =
      match pDef node env with
      | [def] -> pos, Def_inline def
      | _ -> failwith "This should be impossible; inline definition was list."
    in
    let () = env.max_depth <- outer_max_depth in
    result
  | _ -> missing_syntax ?fallback:(Some (Pos.none,Noop)) "statement" node env in
  pop_docblock ();
  result

and is_hashbang text =
  match Syntax.extract_text text with
  | None -> false
  | Some text ->
    let count = List.length @@ String_utils.split_on_newlines text in
    count = 1 && Str.string_match hashbang text 0 && String.equal (Str.matched_string text) text

and pMarkup node env =
  match syntax node with
  | MarkupSection { markup_prefix; markup_text; markup_expression; _ } ->
    let pos = pPos node env in
    if env.is_hh_file then
      if (is_missing markup_prefix) &&
      (width markup_text) > 0 && not (is_hashbang markup_text) then
        raise_parsing_error env (`Node node) SyntaxError.error1001
      else if (token_kind markup_prefix) = Some TK.QuestionGreaterThan then
        raise_parsing_error env (`Node node) SyntaxError.error2067;
    let expr =
      match syntax markup_expression with
      | Missing -> None
      | ExpressionStatement {
          expression_statement_expression = e
        ; _} -> Some (pExpr e env)
      | _ -> failwith "expression expected"
    in
    pos, Markup ((pos, text markup_text), expr)
  | _ -> failwith "invalid node"

and pBreak_or_continue_level env level =
  mpOptional pExpr level env

and pTConstraintTy : hint parser = fun node ->
  match syntax node with
  | TypeConstraint { constraint_type; _ } -> pHint constraint_type
  | _ -> missing_syntax "type constraint" node

and pTConstraint : (constraint_kind * hint) parser = fun node env ->
  match syntax node with
  | TypeConstraint { constraint_keyword; constraint_type } ->
    ( (match token_kind constraint_keyword with
      | Some TK.As    -> Constraint_as
      | Some TK.Super -> Constraint_super
      | Some TK.Equal -> Constraint_eq
      | _ -> missing_syntax "constraint operator" constraint_keyword env
      )
    , pHint constraint_type env
    )
  | _ -> missing_syntax "type constraint" node env

and pTParaml : tparam list parser = fun node env ->
  let pTParam : tparam parser = fun node env ->
    match syntax node with
    | TypeParameter
      { type_reified; type_variance; type_name; type_constraints } ->
      ( (match token_kind type_variance with
        | Some TK.Plus  -> Covariant
        | Some TK.Minus -> Contravariant
        | _ -> Invariant
        )
      , pos_name type_name env
      , couldMap ~f:pTConstraint type_constraints env
      , not @@ is_missing type_reified
      )
    | _ -> missing_syntax "type parameter" node env
  in
  match syntax node with
  | Missing -> []
  | TypeParameters { type_parameters_parameters; _ } ->
    couldMap ~f:pTParam type_parameters_parameters env
  | _ -> missing_syntax "type parameter list" node env

and pFunHdr check_modifier : fun_hdr parser = fun node env ->
  match syntax node with
  | FunctionDeclarationHeader
    { function_modifiers
    ; function_ampersand
    ; function_name
    ; function_where_clause
    ; function_type_parameter_list
    ; function_parameter_list
    ; function_type
    ; _ } ->
      let is_autoload =
        String.lowercase_ascii @@ (text function_name)
          = Naming_special_names.SpecialFunctions.autoload in
      let num_params = List.length (syntax_to_list_no_separators function_parameter_list) in
      if is_autoload && num_params > 1 then
        raise_parsing_error env (`Node node) SyntaxError.autoload_takes_one_argument;
      let modifiers = pModifiers check_modifier function_modifiers env in
      let fh_parameters = couldMap ~f:pFunParam function_parameter_list env in
      let fh_return_type = mpOptional pHint function_type env in
      let fh_suspension_kind =
        mk_suspension_kind_ node env modifiers.has_async modifiers.has_coroutine in
      let fh_name = pos_name function_name env in
      let fh_constrs =
        match syntax function_where_clause with
        | Missing -> []
        | WhereClause { where_clause_constraints; _ } ->
          let rec f node =
            match syntax node with
            | ListItem { list_item; _ } -> f list_item
            | WhereConstraint
              { where_constraint_left_type
              ; where_constraint_operator
              ; where_constraint_right_type
              } ->
                let l = pHint where_constraint_left_type env in
                let o =
                  match syntax where_constraint_operator with
                  | Token token when Token.kind token = TK.Equal ->
                    Constraint_eq
                  | Token token when Token.kind token = TK.As -> Constraint_as
                  | Token token when Token.kind token = TK.Super -> Constraint_super
                  | _ -> missing_syntax "constraint operator" where_constraint_operator env
                in
                let r = pHint where_constraint_right_type env in
                (l,o,r)
            | _ -> missing_syntax "where constraint" node env
          in
          List.map ~f (syntax_node_to_list where_clause_constraints)
        | _ -> missing_syntax "function header constraints" node env
      in
      let fh_type_parameters = pTParaml function_type_parameter_list env in
      let fh_param_modifiers =
        List.filter ~f:(fun p -> Option.is_some p.param_modifier) fh_parameters
      in
      let fh_ret_by_ref = is_ret_by_ref function_ampersand in
      { fh_suspension_kind
      ; fh_name
      ; fh_constrs
      ; fh_type_parameters
      ; fh_parameters
      ; fh_return_type
      ; fh_param_modifiers
      ; fh_ret_by_ref
      }
  | LambdaSignature { lambda_parameters; lambda_type; _ } ->
    { empty_fun_hdr with
      fh_parameters  = couldMap ~f:pFunParam lambda_parameters env
    ; fh_return_type = mpOptional pHint lambda_type env
    }
  | Token _ -> empty_fun_hdr
  | _ -> missing_syntax "function header" node env

and docblock_stack = Stack.create ()

and extract_docblock = fun node ->
  let source_text = leading_text node in
  let parse (str : string) : string option =
      let length = String.length str in
      let mk (start : int) (end_ : int) : string =
        String.sub source_text start (end_ - start + 1)
      in
      let rec go start state idx : string option =
        if idx = length (* finished? *)
        then None
        else begin
          let next = idx + 1 in
          match state, str.[idx] with
          | `LineCmt,     '\n' -> go next `Free next
          | `EndEmbedded, '/'  -> go next `Free next
          | `EndDoc, '/' -> begin match go next `Free next with
            | Some doc -> Some doc
            | None -> Some (mk start idx)
          end
          (* PHP has line comments delimited by a # *)
          | `Free,     '#'              -> go next `LineCmt      next
          (* All other comment delimiters start with a / *)
          | `Free,     '/'              -> go idx   `SawSlash    next
          (* After a / in trivia, we must see either another / or a * *)
          | `SawSlash, '/'              -> go next  `LineCmt     next
          | `SawSlash, '*'              -> go start `MaybeDoc    next
          | `MaybeDoc, '*'              -> go start `MaybeDoc2   next
          | `MaybeDoc, _                -> go start `EmbeddedCmt next
          | `MaybeDoc2, '/'             -> go next  `Free        next
          (* Doc comments have a space after the second star *)
          | `MaybeDoc2, (' ' | '\t' | '\n' | '\r') -> go start `DocComment idx
          | `MaybeDoc2, _               -> go start `EmbeddedCmt  next
          | `DocComment, '*'            -> go start `EndDoc      next
          | `DocComment, _              -> go start `DocComment  next
          | `EndDoc, _                  -> go start `DocComment  next
          (* A * without a / does not end an embedded comment *)
          | `EmbeddedCmt, '*'           -> go start `EndEmbedded next
          | `EndEmbedded, '*'           -> go start `EndEmbedded next
          | `EndEmbedded,  _            -> go start `EmbeddedCmt next
          (* Whitespace skips everywhere else *)
          | _, (' ' | '\t' | '\n' | '\r')      -> go start state        next
          (* When scanning comments, anything else is accepted *)
          | `LineCmt,     _             -> go start state        next
          | `EmbeddedCmt, _             -> go start state        next
          (* Anything else; bail *)
          | _ -> None
        end
      in
      go 0 `Free 0
  in (* Now that we have a parser *)
  parse (leading_text node)

and extract_and_push_docblock node =
  let docblock = extract_docblock node in
  Stack.push docblock docblock_stack

and handle_loop_body pos stmts tail env =
  let rec conv acc stmts =
    match stmts with
    | [] -> List.concat @@ List.rev acc
    | { syntax = UsingStatementFunctionScoped
        { using_function_await_keyword = await_kw
        ; using_function_expression = expression
        ; _ }
      ; _ } :: rest ->
      let body = conv [] rest in
      let using = Using {
        us_is_block_scoped = false;
        us_has_await = not (is_missing await_kw);
        us_expr = pExprL expression env;
        us_block = body; } in
      List.concat @@ List.rev ([pos, using] :: acc)
    | h :: rest ->
      let h = pStmtUnsafe h env in
      conv (h :: acc) rest
  in
  let blk = conv [] (as_list stmts) in
  begin match List.filter ~f:(fun (_, x) -> x <> Noop) blk @ tail with
  | [] -> pos, Block [Pos.none, Noop]
  | blk -> pos, Block blk
  end

and pop_docblock () =
  try
    let _ = Stack.pop docblock_stack in ()
  with
  | Stack.Empty -> ()

and top_docblock () =
  try
    Stack.top docblock_stack
  with
  | Stack.Empty -> None

and pClassElt : class_elt list parser = fun node env ->
  let doc_comment_opt = extract_docblock node in
  let pClassElt_ = function
  | ConstDeclaration
    { const_abstract; const_type_specifier; const_declarators; _ } ->
      let ty = mpOptional pHint const_type_specifier env in
      let res =
        couldMap const_declarators env ~f:begin function
          | { syntax = ConstantDeclarator
              { constant_declarator_name; constant_declarator_initializer }
            ; _ } -> fun env ->
              ( pos_name constant_declarator_name env
              (* TODO: Parse error when const is abstract and has inits *)
              , if is_missing const_abstract
                then mpOptional pSimpleInitializer constant_declarator_initializer env
                else None
              )
          | node -> missing_syntax "constant declarator" node env
        end
    in
    let rec aux absts concrs = function
    | (id, None  ) :: xs -> aux (AbsConst (ty, id) :: absts) concrs xs
    | (id, Some x) :: xs -> aux absts ((id, x) :: concrs) xs
    | [] when concrs = [] -> List.rev absts
    | [] -> Const (ty, List.rev concrs) :: List.rev absts
    in
    aux [] [] res
  | TypeConstDeclaration
    { type_const_abstract
    ; type_const_name
    ; type_const_type_parameters
    ; type_const_type_constraint
    ; type_const_type_specifier
    ; _ } ->
      [ TypeConst
        { tconst_abstract   = not (is_missing type_const_abstract)
        ; tconst_name       = pos_name type_const_name env
        ; tconst_tparams    = pTParaml type_const_type_parameters env
        ; tconst_constraint = mpOptional pTConstraintTy type_const_type_constraint env
        ; tconst_type       = mpOptional pHint type_const_type_specifier env
        ; tconst_span       = pPos node env
        }
      ]
  | PropertyDeclaration
    { property_attribute_spec
    ; property_modifiers
    ; property_type
    ; property_declarators
    ; _
    } ->
      (* TODO: doc comments do not have to be at the beginning, they can go in
       * the middle of the declaration, to be associated with individual
       * properties, right now we don't handle this *)
      let doc_comment_opt = extract_docblock node in
      let typecheck = is_typechecker env in (* so we don't capture the env in the closure *)
      let check_modifier node =
        if is_final node
        then raise_parsing_error env (`Node node) SyntaxError.final_property;
        if typecheck && is_var node
        then raise_parsing_error env (`Node node) SyntaxError.var_property in

      [ ClassVars
        { cv_kinds = pKinds check_modifier property_modifiers env
        ; cv_hint = mpOptional pHint property_type env
        ; cv_is_promoted_variadic = false
        ; cv_names = couldMap property_declarators env ~f:begin fun node env ->
          match syntax node with
          | PropertyDeclarator { property_name; property_initializer } ->
            ( let _, n as name = pos_name property_name env in
              ( pPos node env
              , ( if String.length n > 0 && n.[0] = '$'
                  then drop_pstr 1 name
                  else name
                )
              , mpOptional pSimpleInitializer property_initializer env
              )
            )
          | _ -> missing_syntax "property declarator" node env
          end
        ; cv_doc_comment = if env.quick_mode then None else doc_comment_opt
        ; cv_user_attributes = List.concat @@
          couldMap ~f:pUserAttribute property_attribute_spec env
        }
      ]
  | MethodishDeclaration
    { methodish_attribute
    ; methodish_function_decl_header = {
        syntax = FunctionDeclarationHeader h; _
      } as header
    ; methodish_function_body
    ; _ } ->
      let classvar_init : fun_param -> stmt * class_elt = fun param ->
        let p, _ as cvname = drop_pstr 1 param.param_id in (* Drop the '$' *)
        let span =
          match param.param_expr with
          | Some (pos_end, _) -> Pos.btw p pos_end
          | None -> p
        in
        ( (p, Expr (p, Binop (Eq None,
            (p, Obj_get((p, Lvar (p, "$this")), (p, Id cvname), OG_nullthrows)),
            (p, Lvar param.param_id))
          ))
        , ClassVars
          { cv_kinds = Option.to_list param.param_modifier
          ; cv_hint = param.param_hint
          ; cv_is_promoted_variadic = param.param_is_variadic
          ; cv_names = [span, cvname, None]
          ; cv_doc_comment = None
          ; cv_user_attributes = param.param_user_attributes
          }
        )
      in
      let hdr = pFunHdr (fun _ -> ()) header env in
      let member_init, member_def =
        List.unzip @@
          List.filter_map hdr.fh_parameters ~f:(fun p ->
            Option.map ~f: (fun _ -> classvar_init p) p.param_modifier
          )
      in
      let pBody = fun node env ->
        let body = pFunctionBody node env in
        let member_init =
          if env.codegen
          then List.rev member_init
          else member_init
        in
        member_init @ body
      in
      let body, body_has_yield = mpYielding pBody methodish_function_body env in
      let kind = pKinds (fun _ -> ()) h.function_modifiers env in
      member_def @ [Method
      { m_kind            = kind
      ; m_tparams         = hdr.fh_type_parameters
      ; m_constrs         = hdr.fh_constrs
      ; m_name            = hdr.fh_name
      ; m_params          = hdr.fh_parameters
      ; m_body            = body
      ; m_user_attributes = pUserAttributes env methodish_attribute
      ; m_ret             = hdr.fh_return_type
      ; m_ret_by_ref      = hdr.fh_ret_by_ref
      ; m_span            = pFunction node env
      ; m_fun_kind        = mk_fun_kind hdr.fh_suspension_kind body_has_yield
      ; m_doc_comment     = doc_comment_opt
      }]
  | TraitUseConflictResolution
    { trait_use_conflict_resolution_names
    ; trait_use_conflict_resolution_clauses
    ; _
    } ->
    let pTraitUseConflictResolutionItem node env =
      match syntax node with
      | TraitUsePrecedenceItem
        { trait_use_precedence_item_name = name
        ; trait_use_precedence_item_removed_names = removed_names
        ; _
        } ->
        let qualifier, name =
          match syntax name with
          | ScopeResolutionExpression
            { scope_resolution_qualifier; scope_resolution_name; _ } ->
            pos_name scope_resolution_qualifier env,
            pos_name scope_resolution_name env
          | _ -> missing_syntax "trait use precedence item" node env
        in
        let removed_names =
          couldMap ~f:(fun n _e -> pos_name n env) removed_names env
        in
        ClassUsePrecedence (qualifier, name, removed_names)
      | TraitUseAliasItem
        { trait_use_alias_item_aliasing_name = aliasing_name
        ; trait_use_alias_item_modifiers = modifiers
        ; trait_use_alias_item_aliased_name = aliased_name
        ; _
        } ->
        let qualifier, name =
          match syntax aliasing_name with
          | ScopeResolutionExpression
            { scope_resolution_qualifier; scope_resolution_name; _ } ->
            Some (pos_name scope_resolution_qualifier env),
            pos_name scope_resolution_name env
          | _ -> None, pos_name aliasing_name env
        in
        let modifiers = pKinds (fun _ -> ()) modifiers env in
        Core_list.iter modifiers ~f:(fun modifier ->
          match modifier with
          | Public | Private | Protected | Final ->  ();
          | _ ->
            raise_parsing_error env (`Node node)
              SyntaxError.trait_alias_rule_allows_only_final_and_visibility_modifiers
        );
        let is_visibility = function
        | Public | Private | Protected -> true
        | _ -> false in
        let modifiers =
          if List.is_empty modifiers || List.exists modifiers ~f:is_visibility
          then modifiers
          else Public :: modifiers in
        let aliased_name =
          Option.some_if (not (is_missing aliased_name)) (pos_name aliased_name env)
        in
        ClassUseAlias (qualifier, name, aliased_name, modifiers)
     | _ -> missing_syntax "trait use conflict resolution item" node env
    in
    (couldMap ~f:(fun n e ->
        ClassUse (pHint n e)) trait_use_conflict_resolution_names env)
    @ (couldMap ~f:pTraitUseConflictResolutionItem
                    trait_use_conflict_resolution_clauses env)
  | TraitUse { trait_use_names; _ } ->
    couldMap ~f:(fun n e -> ClassUse (pHint n e)) trait_use_names env
  | RequireClause { require_kind; require_name; _ } ->
    [ ClassTraitRequire
      ( (match token_kind require_kind with
        | Some TK.Implements -> MustImplement
        | Some TK.Extends    -> MustExtend
        | _ -> missing_syntax "trait require kind" require_kind env
        )
      , pHint require_name env
      )
    ]
  | XHPClassAttributeDeclaration { xhp_attribute_attributes; _ } ->
    let pXHPAttr node env =
      match syntax node with
      | XHPClassAttribute
        { xhp_attribute_decl_type        = ty
        ; xhp_attribute_decl_name        = name
        ; xhp_attribute_decl_initializer = init
        ; xhp_attribute_decl_required    = req
        } ->
          let (p, name) = pos_name name env in
          let pos = if is_missing init then p else Pos.btw p (pPos init env) in
          (* we can either have a typehint or an xhp enum *)
          let hint, enum = match syntax ty with
            | XHPEnumType { xhp_enum_optional; xhp_enum_values; _ } ->
              let p = pPos ty env in
              let opt = not (is_missing xhp_enum_optional) in
              let vals = couldMap ~f:pExpr xhp_enum_values env in
              None, Some (p, opt, vals)
            | _ -> Some (pHint ty env), None
          in
          XhpAttr
          ( hint
          , (pos, (p, ":" ^ name), mpOptional pSimpleInitializer init env)
          , not (is_missing req)
          , enum
          )
      | XHPSimpleClassAttribute { xhp_simple_class_attribute_type = attr } ->
        XhpAttrUse (pPos attr env, Happly (pos_name attr env, []))
      | Token _ ->
        XhpAttrUse (pPos node env, Happly (pos_name node env, []))
      | _ -> missing_syntax "XHP attribute" node env
    in
    couldMap ~f:pXHPAttr xhp_attribute_attributes env
  | XHPChildrenDeclaration { xhp_children_expression; _; } ->
    let p = pPos node env in
    [ XhpChild (p, pXhpChild xhp_children_expression env) ]
  | XHPCategoryDeclaration { xhp_category_categories = cats; _ } ->
    let p = pPos node env in
    let pNameSansPercent node _env = drop_pstr 1 (pos_name node env) in
    [ XhpCategory (p, couldMap ~f:pNameSansPercent cats env) ]
  | _ -> missing_syntax "class element" node env
  in
  try pClassElt_ (syntax node) with
  | API_Missing_syntax (expecting, env, node) when env.fail_open ->
    let () = log_missing ~caught:true ~env ~expecting node in
    []
and pXhpChild : xhp_child parser = fun node env ->
  match syntax node with
  | Token _ -> ChildName (pos_name node env)
  | PostfixUnaryExpression { postfix_unary_operand; postfix_unary_operator;} ->
    let operand = pXhpChild postfix_unary_operand env in
    let operator =
      begin
        match token_kind postfix_unary_operator with
          | Some TK.Question -> ChildQuestion
          | Some TK.Plus -> ChildPlus
          | Some TK.Star -> ChildStar
          | _ -> missing_syntax "xhp children operator" node env
      end in
    ChildUnary(operand, operator)
  | BinaryExpression
    { binary_left_operand; binary_right_operand; _ } ->
    let left = pXhpChild binary_left_operand env in
    let right = pXhpChild binary_right_operand env in
    ChildBinary(left, right)
  | XHPChildrenParenthesizedList {xhp_children_list_xhp_children; _} ->
    let children = as_list xhp_children_list_xhp_children in
    let children = List.map ~f:(fun x -> pXhpChild x env) children in
    ChildList children
  | _ -> missing_syntax "xhp children" node env


(*****************************************************************************(
 * Parsing definitions (AST's `def`)
)*****************************************************************************)
and pNamespaceUseClause ~prefix env kind node =
  match syntax node with
  | NamespaceUseClause
    { namespace_use_name  = name
    ; namespace_use_alias = alias
    ; namespace_use_clause_kind = clause_kind
    ; _ } ->
    let p, n as name =
      match prefix, pos_name name env with
      | None, (p, n) -> (p, n)
      | Some prefix, (p, n) -> p, (snd @@ pos_name prefix env) ^ n
    in
    let x = Str.search_forward (namespace_use) n 0 in
    let key = drop_pstr x name in
    let kind = if is_missing clause_kind then kind else clause_kind in
    let alias = if is_missing alias then key else pos_name alias env in
    begin match String.lowercase_ascii (snd alias) with
    | "null" | "true" | "false" -> env.saw_std_constant_redefinition <- true
    | _ -> ()
    end;
    let kind =
      match syntax kind with
      | Token token when Token.kind token = TK.Namespace -> NSNamespace
      | Token token when Token.kind token = TK.Type      -> NSClass
      | Token token when Token.kind token = TK.Function  -> NSFun
      | Token token when Token.kind token = TK.Const     -> NSConst
      | Missing                             -> NSClassAndNamespace
      | _ -> missing_syntax "namespace use kind" kind env
    in
    ( kind
    , (p, if String.length n > 0 && n.[0] = '\\' then n else "\\" ^ n)
    , alias
    )
  | _ -> missing_syntax "namespace use clause" node env

and pDef : def list parser = fun node env ->
  let doc_comment_opt = extract_docblock node in
  match syntax node with
  | FunctionDeclaration
    { function_attribute_spec; function_declaration_header; function_body } ->
      let env = non_tls env in
      let check_modifier node =
        raise_parsing_error env (`Node node) (SyntaxError.function_modifier (text node)) in
      let hdr = pFunHdr check_modifier function_declaration_header env in
      let block, yield =
        if is_semicolon function_body then [], false else
          mpYielding pFunctionBody function_body env
      in
      [ Fun
      { (fun_template yield node hdr.fh_suspension_kind env) with
        f_tparams         = hdr.fh_type_parameters
      ; f_ret             = hdr.fh_return_type
      ; f_constrs         = hdr.fh_constrs
      ; f_name            = hdr.fh_name
      ; f_params          = hdr.fh_parameters
      ; f_ret_by_ref      = hdr.fh_ret_by_ref
      ; f_body            =
        begin
          let containsUNSAFE node =
            let tokens = all_tokens node in
            let has_unsafe t = Token.has_trivia_kind t TriviaKind.Unsafe in
            List.exists ~f:has_unsafe tokens
          in
          match block with
          | [p, Noop] when containsUNSAFE function_body -> [p, Unsafe]
          | b -> b
        end
      ; f_user_attributes = pUserAttributes env function_attribute_spec
      ; f_doc_comment = doc_comment_opt
      }]
  | ClassishDeclaration
    { classish_attribute       = attr
    ; classish_modifiers       = mods
    ; classish_keyword         = kw
    ; classish_name            = name
    ; classish_type_parameters = tparaml
    ; classish_extends_list    = exts
    ; classish_implements_list = impls
    ; classish_body            =
      { syntax = ClassishBody { classish_body_elements = elts; _ }; _ }
    ; _ } ->
      let env = non_tls env in
      let c_mode = mode_annotation env.fi_mode in
      let c_user_attributes = pUserAttributes env attr in
      let kinds = pKinds (fun _ -> ()) mods env in
      let c_final = List.mem kinds Final in
      let c_is_xhp =
        match token_kind name with
        | Some (TK.XHPElementName | TK.XHPClassName) -> true
        | _ -> false
      in
      let c_name = pos_name name env in
      let c_tparams = pTParaml tparaml env in
      let c_extends = couldMap ~f:pHint exts env in
      let c_implements = couldMap ~f:pHint impls env in
      let c_body =
        let rec aux acc ns =
          match ns with
          | [] -> acc
          | n :: ns ->
            let elt = pClassElt n env in
            aux (elt :: acc) ns
        in
        List.concat @@ List.rev (aux [] (as_list elts))
      in
      let c_namespace = Namespace_env.empty env.parser_options in
      let c_enum = None in
      let c_span = pPos node env in
      let c_kind =
        let is_abs = List.mem kinds Abstract in
        match token_kind kw with
        | Some TK.Class when is_abs -> Cabstract
        | Some TK.Class             -> Cnormal
        | Some TK.Interface         -> Cinterface
        | Some TK.Trait             -> Ctrait
        | Some TK.Enum              -> Cenum
        | _ -> missing_syntax "class kind" kw env
      in
      let c_doc_comment = doc_comment_opt in
      [ Class
      { c_mode
      ; c_user_attributes
      ; c_final
      ; c_is_xhp
      ; c_name
      ; c_tparams
      ; c_extends
      ; c_implements
      ; c_body
      ; c_namespace
      ; c_enum
      ; c_span
      ; c_kind
      ; c_doc_comment
      }]
  | ConstDeclaration { const_keyword = kw; _ }
    when env.fi_mode = FileInfo.Mdecl
      && "const" <>
        Option.value_map ~default:"" ~f:Token.text (Syntax.get_token kw)
      -> [] (* Legacy parity; case-sensitive global const declarations *)
  | ConstDeclaration
    { const_type_specifier = ty
    ; const_declarators    = decls
    ; _ } ->
      let declarations = List.map ~f:syntax (as_list decls) in
      let f = function
        | ConstantDeclarator
            { constant_declarator_name        = name
            ; constant_declarator_initializer = init
            }
          -> Constant
            { cst_mode      = mode_annotation env.fi_mode
            ; cst_kind      = Cst_const
            ; cst_name      = pos_name name env
            ; cst_type      = mpOptional pHint ty env
            ; cst_value     = pSimpleInitializer init env
            ; cst_namespace = Namespace_env.empty env.parser_options
            ; cst_span      = pPos node env
            }
        | _ -> missing_syntax "constant declaration" decls env
      in
      List.map ~f declarations
  | AliasDeclaration
    { alias_attribute_spec    = attr
    ; alias_keyword           = kw
    ; alias_name              = name
    ; alias_generic_parameter = tparams
    ; alias_constraint        = constr
    ; alias_type              = hint
    ; _ } ->
      [ Typedef
      { t_id              = pos_name name env
      ; t_tparams         = pTParaml tparams env
      ; t_constraint      = Option.map ~f:snd @@
          mpOptional pTConstraint constr env
      ; t_user_attributes = List.concat @@
          List.map ~f:(fun x -> pUserAttribute x env) (as_list attr)
      ; t_namespace       = Namespace_env.empty env.parser_options
      ; t_mode            = mode_annotation env.fi_mode
      ; t_kind            =
        match token_kind kw with
        | Some TK.Newtype -> NewType (pHint hint env)
        | Some TK.Type    -> Alias   (pHint hint env)
        | _ -> missing_syntax "kind" kw env
      }]
  | EnumDeclaration
    { enum_attribute_spec = attrs
    ; enum_name           = name
    ; enum_base           = base
    ; enum_type           = constr
    ; enum_enumerators    = enums
    ; _ } ->
      let pEnumerator node =
        match syntax node with
        | Enumerator { enumerator_name = name; enumerator_value = value; _ } ->
          fun env -> Const (None, [pos_name name env, pExpr value env])
        | _ -> missing_syntax "enumerator" node
      in
      [ Class
      { c_mode            = mode_annotation env.fi_mode
      ; c_user_attributes = pUserAttributes env attrs
      ; c_final           = false
      ; c_kind            = Cenum
      ; c_is_xhp          = false
      ; c_name            = pos_name name env
      ; c_tparams         = []
      ; c_extends         = []
      ; c_implements      = []
      ; c_body            = couldMap enums env ~f:pEnumerator
      ; c_namespace       = Namespace_env.empty env.parser_options
      ; c_span            = pPos node env
      ; c_enum            = Some
        { e_base       = pHint base env
        ; e_constraint = mpOptional pTConstraintTy constr env
        }
      ; c_doc_comment = doc_comment_opt
      }]
  | InclusionDirective
    { inclusion_expression
    ; inclusion_semicolon = _
    } when env.fi_mode <> FileInfo.Mdecl && env.fi_mode <> FileInfo.Mphp
        || env.codegen ->
      let expr = pExpr inclusion_expression env in
      [ Stmt (pPos node env, Expr expr) ]
  | NamespaceDeclaration
    { namespace_name = name
    ; namespace_body =
      { syntax = NamespaceBody { namespace_declarations = decls; _ }; _ }
    ; _ } ->
      let env = non_tls env in
      [ Namespace
      ( pos_name name env
      , List.concat_map ~f:(fun x -> pDef x env) (as_list decls)
      )]
  | NamespaceDeclaration { namespace_name = name; _ } ->
    [ Namespace (pos_name name env, []) ]
  | NamespaceGroupUseDeclaration
    { namespace_group_use_kind = kind
    ; namespace_group_use_prefix = prefix
    ; namespace_group_use_clauses = clauses
    ; _ } ->
      let f = pNamespaceUseClause env kind ~prefix:(Some prefix) in
      [ NamespaceUse (List.map ~f (as_list clauses)) ]
  | NamespaceUseDeclaration
    { namespace_use_kind    = kind
    ; namespace_use_clauses = clauses
    ; _ } ->
      let f = pNamespaceUseClause env kind ~prefix:None in
      [ NamespaceUse (List.map ~f (as_list clauses)) ]
  | _ when env.fi_mode = FileInfo.Mdecl || env.fi_mode = FileInfo.Mphp
        && not env.codegen -> []
  | _ -> [ Stmt (pStmt node env) ]
let pProgram : program parser = fun node env  ->
  let rec post_process program acc =
    let span (p : 'a -> bool) =
      let rec go yes = function
      | (x::xs) when p x -> go (x::yes) xs
      | xs -> (List.rev yes, xs)
      in go []
    in
    let not_namespace = function
    | Namespace _ -> false
    | _ -> true
    in
    match program with
    | [] -> List.rev acc
    | (Namespace (n, [])::el) ->
      let body, remainder = span not_namespace el in
       post_process remainder (Namespace (n, body) :: acc)
    | (Namespace (n, il)::el) ->
      let result = post_process il [] in
      post_process el (Namespace (n, result) :: acc)
    | (Stmt (_, Noop) :: el) -> post_process el acc
    | ((Stmt (_, Expr (pos, (Call
        ( (_, (Id (_, "define")))
        , []
        , [ (_, (String name))
          ; value
          ]
        , []
        )
      )))) :: el) ->
      let const = Constant
        { cst_mode      = mode_annotation env.fi_mode
        ; cst_kind      = Cst_define
        ; cst_name      = pos, name
        ; cst_type      = None
        ; cst_value     = value
        ; cst_namespace = Namespace_env.empty env.parser_options
        ; cst_span      = pos
        } in
      post_process el (const :: acc)
    | (Stmt (_, Markup _) as e)::el
    | (Stmt (_, Expr (_, Import _)) as e)::el ->
      post_process el (e :: acc)
    (* Toplevel statements not allowed in strict mode *)
    | (Stmt (p, _) as e)::el
        when (env.keep_errors) && (not env.codegen) && env.fi_mode = FileInfo.Mstrict ->
      (* We've already lowered at this point, so raise_parsing_error doesn't
        really fit. This is only a typechecker error anyway, so just add it
        directly *)
      Errors.parsing_error (p, SyntaxError.toplevel_statements);
      post_process el (e :: acc)
    | (e::el) -> post_process el (e :: acc)
  in

  (* The list of top-level things in a file is somewhat special. *)
  let rec aux env acc = function
  | []
  (* EOF happens only as the last token in the list. *)
  | [{ syntax = EndOfFile _; _ }]
    -> List.concat (List.rev acc)
  (* HaltCompiler stops processing the list *)
  | { syntax = ExpressionStatement
      { expression_statement_expression =
        { syntax = HaltCompilerExpression _ ; _ } ; _ } ; _ } as cur_node :: nodel
    ->
    (* If we saw COMPILER_HALT_OFFSET, calculate the position of HALT_COMPILER *)
    if !(env.saw_compiler_halt_offset) <> None then
      begin
      let local_ignore_pos = env.ignore_pos in
      let () = env.ignore_pos <- false in
      let pos = pPos cur_node env in
      (* __COMPILER_HALT_OFFSET__ takes value equal to halt_compiler's end position *)
      let s = Pos.end_cnum pos in
      let () = env.saw_compiler_halt_offset := Some s in
      env.ignore_pos <- local_ignore_pos
      end;
    aux env acc nodel
  (* There's an incompatibility between the Full-Fidelity (FF) and the AST view
   * of the world; `define` is an *expression* in FF, but a *definition* in AST.
   * Luckily, `define` only happens at the level of definitions.
   *)
  | { syntax = ExpressionStatement
      { expression_statement_expression =
        { syntax = DefineExpression
          { define_keyword; define_argument_list = args; _ }
        ; _ }
      ; _ }
    ; _ } as cur_node :: nodel when not env.quick_mode ->
      let def =
        match List.map ~f:(fun x -> pExpr x env) (as_list args) with
        | [ pos, String name; e ] -> Constant
          { cst_mode      = mode_annotation env.fi_mode
          ; cst_kind      = Cst_define
          ; cst_name      = pos, name
          ; cst_type      = None
          ; cst_value     = e
          ; cst_namespace = Namespace_env.empty env.parser_options
          ; cst_span      = pPos cur_node env
          }
        | args ->
          let name = pos_name define_keyword env in
          Stmt (pPos cur_node env,
            Expr (fst name, Call ((fst name, Id name), [], args, [])))
      in
      aux env ([def] :: acc) nodel
  | node :: nodel ->
    let acc =
      match pDef node env with
      | exception API_Missing_syntax (expecting, env, node)
        when env.fail_open ->
          let () = log_missing ~caught:true ~env ~expecting node in
          acc
      | def -> def :: acc
    in
    aux env acc nodel
  in
  let nodes = as_list node in
  let nodes = aux env [] nodes in
  post_process nodes []

let pScript node env =
  match syntax node with
  | Script { script_declarations; _ } -> pProgram script_declarations env
  | _ -> missing_syntax "script" node env

(* The full fidelity parser considers all comments "simply" trivia. Some
 * comments have meaning, though. This meaning can either be relevant for the
 * type checker (like UNSAFE, HH_FIXME, etc.), but also for other uses, like
 * Codex, where comments are used for documentation generation.
 *
 * Inlining the scrape for comments in the lowering code would be prohibitively
 * complicated, but a separate pass is fine.
 *)

type fixmes = Pos.t IMap.t IMap.t
type scoured_comment = Pos.t * comment
type scoured_comments = scoured_comment list
type accumulator = scoured_comments * fixmes

let scour_comments
  (path        : Relative_path.t)
  (source_text : SourceText.t)
  ~(collect_fixmes: bool)
  ~(include_line_comments: bool)
  (tree        : node)
  (env         : env)
  : accumulator =
    let pos_of_offset = SourceText.relative_pos path source_text in
    let go (node : node) (cmts, fm as acc : accumulator) (t : Trivia.t)
        : accumulator =
      match Trivia.kind t with
      | TriviaKind.WhiteSpace
      | TriviaKind.EndOfLine
      | TriviaKind.Unsafe
      | TriviaKind.UnsafeExpression
      | TriviaKind.FallThrough
      | TriviaKind.ExtraTokenError
      | TriviaKind.AfterHaltCompiler
        -> acc
      | TriviaKind.DelimitedComment ->
        (* For legacy compliance, block comments should have the position of
         * their end
         *)
        let start = Trivia.start_offset t + 2 (* for the '/*' *) in
        let end_  = Trivia.end_offset t in
        let len   = end_ - start - 1 in
        let p = pos_of_offset (end_ - 1) end_ in
        let t = String.sub (Trivia.text t) 2 len in
        (p, CmtBlock t) :: cmts, fm
      | TriviaKind.SingleLineComment ->
        if not include_line_comments then cmts, fm
        else
        let text = SourceText.text (Trivia.source_text t) in
        let start = Trivia.start_offset t in
        let start = start + if text.[start] = '#' then 1 else 2 in
        let end_  = Trivia.end_offset t in
        let len   = end_ - start + 1 in
        let p = pos_of_offset start end_ in
        let t = String.sub text start len in
        (p, CmtLine (t ^ "\n")) :: cmts, fm
      | TriviaKind.FixMe
      | TriviaKind.IgnoreError
        -> if not collect_fixmes then cmts, fm
           else
           let open Str in
           let pos = pPos node env in
           let line = Pos.line pos in
           let ignores = try IMap.find line fm with Not_found -> IMap.empty in
           let txt = Trivia.text t in
           try
             ignore (search_forward ignore_error txt 0);
             let p = pos_of_offset (Trivia.start_offset t) (Pos.start_cnum pos) in
             let code = int_of_string (matched_group 2 txt) in
             let ignores = IMap.add code p ignores in
             cmts, IMap.add line ignores fm
           with
           | Not_found ->
             Errors.fixme_format pos;
             cmts, fm
    in
    let rec aux (_cmts, _fm as acc : accumulator) (node : node) : accumulator =
      let recurse () = List.fold_left ~f:aux ~init:acc (children node) in
      match syntax node with
      | Token t ->
        if Token.has_trivia_kind t TriviaKind.DelimitedComment
        || (include_line_comments && Token.has_trivia_kind t TriviaKind.SingleLineComment)
        || (collect_fixmes && (
            Token.has_trivia_kind t TriviaKind.FixMe ||
            Token.has_trivia_kind t TriviaKind.IgnoreError))
        then
          let f = go node in
          let trivia = Token.leading t in
          let acc = List.fold_left ~f ~init:acc trivia in
          let trivia = Token.trailing t in
          List.fold_left ~f ~init:acc trivia
        else recurse ()
      | _ -> recurse ()
    in
    aux ([], IMap.empty) tree

(*****************************************************************************(
 * Front-end matter
)*****************************************************************************)

let elaborate_toplevel_and_std_constants ast (env: env) source_text =
  let autoimport =
    env.is_hh_file || ParserOptions.enable_hh_syntax_for_hhvm env.parser_options
  in
  match env.elaborate_namespaces, env.saw_std_constant_redefinition with
  | true, true ->
    let elaborate_std_constants nsenv def =
      let visitor = object
        inherit [_] endo as super
        method! on_expr env expr =
          match expr with
          | p, True | p, False | p, Null when p <> Pos.none ->
            let s = Pos.start_cnum p in
            let e = Pos.end_cnum p in
            let text = SourceText.sub source_text s (e - s) in
            let was_renamed, _ =
              NS.elaborate_id_impl
                ~autoimport:true
                nsenv
                NS.ElaborateConst
                (p, text) in
            if was_renamed then p, Ast.Id (p, text)
            else expr
          | _ -> super#on_expr env expr
      end in
      visitor#on_def nsenv def in
    let parser_options = env.parser_options in
    NS.elaborate_map_toplevel_defs
      ~autoimport parser_options ast elaborate_std_constants
  | true, false ->
    NS.elaborate_toplevel_defs ~autoimport env.parser_options ast
  | _ -> ast

let elaborate_halt_compiler ast env source_text  =
  match !(env.saw_compiler_halt_offset) with
    | Some x ->
    let elaborate_halt_compiler_const defs =
      let visitor = object
        inherit [_] endo as super
        method! on_expr env expr =
          match expr with
          | p, Id (_, "__COMPILER_HALT_OFFSET__") ->
            let start_offset = Pos.start_cnum p in
            (* Construct a new position and id *)
            let id = string_of_int x in
            let end_offset = start_offset + (String.length id) in
            let pos_file = Pos.filename p in
            let pos = SourceText.relative_pos pos_file source_text start_offset end_offset in
            pos, Ast.Int id
          | _ -> super#on_expr env expr
      end in
      visitor#on_program () defs in
    elaborate_halt_compiler_const ast
  | None -> ast


let lower env ~source_text ~script comments : result =
  let ast = runP pScript script env in
  let ast = elaborate_toplevel_and_std_constants ast env source_text in
  let ast = elaborate_halt_compiler ast env source_text in
  let content = if env.codegen then "" else SourceText.text source_text in
  { fi_mode = env.fi_mode
  ; is_hh_file = env.is_hh_file
  ; ast
  ; content
  ; comments
  ; file = env.file
  }
end (* WithPositionedSyntax *)

(* TODO: Make these not default to positioned_syntax *)
include Full_fidelity_ast_types
module ParserErrors_ = Full_fidelity_parser_errors.WithSyntax(PositionedSyntax)
module ParserErrors = ParserErrors_.WithSmartConstructors(CoroutineSC)

module SourceText = Full_fidelity_source_text
module DeclModeSC = DeclModeSmartConstructors.WithSyntax(PositionedSyntax)
module DeclModeParser_ = Full_fidelity_parser.WithSyntax(PositionedSyntax)
module DeclModeParser = DeclModeParser_.WithSmartConstructors(DeclModeSC)
module FromPositionedSyntax = WithPositionedSyntax(PositionedSyntax)
module FromEditablePositionedSyntax =
  WithPositionedSyntax(Full_fidelity_editable_positioned_syntax)

let parse_text
  (env : env)
  (source_text : SourceText.t)
: (FileInfo.file_type * FileInfo.mode * PositionedSyntaxTree.t) =
  let lang, mode = Full_fidelity_parser.get_language_and_mode source_text in
  let mode = Option.value ~default:FileInfo.Mphp mode in
  let env =  { env with fi_mode = mode } in
  let env = { env with quick_mode = not env.codegen
      && (match mode with
         | FileInfo.Mdecl
         | FileInfo.Mphp -> true
         | _ -> env.quick_mode
         ) } in
  if mode = FileInfo.Mexperimental && env.codegen && (not env.hacksperimental) then begin
    let e = SyntaxError.make 0 0 SyntaxError.experimental_in_codegen_without_hacksperimental in
    let p =
      SourceText.relative_pos env.file source_text
        (Full_fidelity_syntax_error.start_offset e)
        (Full_fidelity_syntax_error.end_offset e) in
    raise @@ SyntaxError.ParserFatal (e, p)
  end;
  let tree =
    let env' =
      Full_fidelity_parser_env.make
        ~hhvm_compat_mode:env.codegen
        ~codegen:env.codegen
        ~php5_compat_mode:env.php5_compat_mode
        ~lang:lang
        ~mode
        ()
    in
    if env.quick_mode then
      let parser = DeclModeParser.make env' source_text in
      let (parser, root) = DeclModeParser.parse_script parser in
      let errors = DeclModeParser.errors parser in
      PositionedSyntaxTree.create source_text root errors lang (Some mode) false
    else
      PositionedSyntaxTree.make ~env:env' source_text
  in
  (lang, mode, tree)

let scour_comments_and_add_fixmes (env : env) source_text script =
  let comments, fixmes =
    FromPositionedSyntax.scour_comments env.file source_text script env
      ~collect_fixmes:env.keep_errors
      ~include_line_comments:env.include_line_comments
      in
  let () = if env.keep_errors then
    if env.quick_mode then
      Fixmes.DECL_HH_FIXMES.add env.file fixmes
    else
      Fixmes.HH_FIXMES.add env.file fixmes in
  comments

let lower_tree
  (env : env)
  (source_text : SourceText.t)
  (lang : FileInfo.file_type)
  (mode : FileInfo.mode)
  (tree : PositionedSyntaxTree.t)
: result =
  let env =
    { env with lower_coroutines =
        env.lower_coroutines &&
        PositionedSyntaxTree.sc_state tree &&
        env.codegen
    } in
  let script = PositionedSyntaxTree.root tree in
  let comments = scour_comments_and_add_fixmes env source_text script in
  let () =
    if env.codegen && not env.lower_coroutines then
      let hhvm_compat_mode = if env.systemlib_compat_mode
        then ParserErrors.SystemLibCompat
        else ParserErrors.HHVMCompat in
      let error_env = ParserErrors.make_env tree
        ~hhvm_compat_mode
        ~enable_hh_syntax:env.enable_hh_syntax
        ~codegen:env.codegen
      in
      let errors = ParserErrors.parse_errors error_env in
      (* Prioritize runtime errors *)
      let runtime_errors =
        List.filter errors ~f:SyntaxError.(fun e -> error_type e = RuntimeError)
      in
      match errors, runtime_errors with
      | [], [] -> ()
      | _, e :: _
      | e :: _, _
        ->
        let p =
          SourceText.relative_pos env.file source_text
            (Full_fidelity_syntax_error.start_offset e)
            (Full_fidelity_syntax_error.end_offset e) in
        raise @@ SyntaxError.ParserFatal (e, p)
    else if env.keep_errors then
      let pos_and_message_of error =
        let so = SyntaxError.start_offset error in
        let eo = SyntaxError.end_offset error in
        let p = SourceText.relative_pos env.file source_text so eo in
        p, SyntaxError.message error
      in
      let is_hhi =
        String_utils.string_ends_with Relative_path.(suffix env.file) "hhi"
      in
      match PositionedSyntaxTree.errors tree with
      | [] when env.quick_mode -> ()
      | [] ->
        let error_env = ParserErrors.make_env tree
          ~hhvm_compat_mode:ParserErrors.HHVMCompat
          ~enable_hh_syntax:env.enable_hh_syntax
          ~codegen:env.codegen
          ~hhi_mode:is_hhi
        in
        let errors = ParserErrors.parse_errors error_env in
        let f e = Errors.parsing_error (pos_and_message_of e) in
        List.iter ~f errors
      | e::_ ->
        Errors.parsing_error (pos_and_message_of e);
  in
  let mode = if lang = FileInfo.PhpFile then FileInfo.Mphp else mode in
  let mode = if mode = FileInfo.Mdecl && env.codegen then FileInfo.Mphp else mode in
  let env = { env with fi_mode = mode } in
  let env = { env with is_hh_file = lang == FileInfo.HhFile } in
  let popt = env.parser_options in
  (* If we are generating code and this is an hh file or hh syntax is enabled,
   * then we want to inject auto import types into HH namespace during namespace
   * resolution.
   *)
  let popt = ParserOptions.with_hh_syntax_for_hhvm popt
    (env.codegen && (ParserOptions.enable_hh_syntax_for_hhvm popt || env.is_hh_file)) in
  let env = { env with parser_options = popt } in
  if env.lower_coroutines
  then
    let script =
      Full_fidelity_editable_positioned_syntax.from_positioned_syntax script
      |> Ppl_class_rewriter.rewrite_ppl_classes
      |> Coroutine_lowerer.lower_coroutines in
    FromEditablePositionedSyntax.lower
      env
      ~source_text
      ~script
      comments
  else
    FromPositionedSyntax.lower
      env
      ~source_text
      ~script
      comments

let from_text (env : env) (source_text : SourceText.t) : result =
  let (lang, mode, tree) = parse_text env source_text in
  lower_tree env source_text lang mode tree

let from_file (env : env) : result =
  let source_text = SourceText.from_file env.file in
  from_text env source_text

(*****************************************************************************(
 * Backward compatibility matter (should be short-lived)
)*****************************************************************************)

let legacy (x : result) : Parser_return.t =
  { Parser_return.file_mode = Option.some_if (x.fi_mode <> FileInfo.Mphp) x.fi_mode
  ; Parser_return.is_hh_file = x.is_hh_file
  ; Parser_return.comments  = x.comments
  ; Parser_return.ast       = x.ast
  ; Parser_return.content   = x.content
  }

let from_text_with_legacy (env : env) (content : string)
  : Parser_return.t =
    let source_text = SourceText.make env.file content in
    legacy @@ from_text env source_text

let from_file_with_legacy env = legacy (from_file env)


(******************************************************************************(
 * For cut-over purposes only; this should be removed as soon as Parser_hack
 * is removed.
)******************************************************************************)

let defensive_program
  ?(hacksperimental=false)
  ?(quick=false)
  ?(fail_open=false)
  ?(elaborate_namespaces=true)
  parser_options fn content =
  try begin
    let source = Full_fidelity_source_text.make fn content in
    (* If we fail open, we don't want errors. *)
    let env = make_env
      ~fail_open
      ~quick_mode:quick
      ~elaborate_namespaces
      ~keep_errors:(not fail_open)
      ~parser_options
      ~hacksperimental
      fn
    in
    legacy @@ from_text env source
  end with e ->
    (* If we fail to lower, try to just make a source text and get the file mode *)
    (* If even THAT fails, we just have to give up and return an empty php file*)
    let mode =
    try
      let source = Full_fidelity_source_text.make fn content in
      snd @@ Full_fidelity_parser.get_language_and_mode source
    with _ -> None in
    let err = Printexc.to_string e in
    let fn = Relative_path.suffix fn in
    (* If we've already found a parsing error, it's okay for lowering to fail *)
    if not (Errors.currently_has_errors ()) then
      Hh_logger.log "Warning, lowering failed for %s\n  - error: %s\n" fn err;

    { Parser_return.file_mode = mode
    ; Parser_return.comments = []
    ; Parser_return.ast = []
    ; Parser_return.content = content
    ; Parser_return.is_hh_file = mode <> None
    }

let defensive_from_file ?quick popt fn =
  let content = try Sys_utils.cat (Relative_path.to_absolute fn) with _ -> "" in
  defensive_program ?quick popt fn content

let defensive_from_file_with_default_popt ?quick fn =
  defensive_from_file ?quick ParserOptions.default fn

let defensive_program_with_default_popt
  ?hacksperimental
  ?quick
  ?fail_open
  ?elaborate_namespaces
  fn content =
  defensive_program
    ?hacksperimental
    ?quick
    ?fail_open
    ?elaborate_namespaces
    (ParserOptions.default) fn content
