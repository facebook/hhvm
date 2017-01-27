(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module K = Full_fidelity_syntax_kind
module P = Full_fidelity_positioned_syntax
module TK = Full_fidelity_token_kind
module PT = Full_fidelity_positioned_token

let string_of_expr thing =
   Sexp.to_string_hum @@ Sof.sexp_of_expr thing
let string_of_stmt x = Sexp.to_string_hum (Sof.sexp_of_stmt x)

(* For debugging *)
let treeDBG =
  let rec go indent node = begin match P.syntax node with
    | P.Token t -> Printf.eprintf "> %sToken: '%s'\n" indent (PT.full_text t)
    | _ -> Printf.eprintf ">  %s%s '%s'\n"
        indent
        (P.kind node |> K.to_string)
        (P.full_text node)
    end;
    List.iter (go @@ "  " ^ indent) (P.children node)
  in
  go ""
let dbg_clean s = Printf.eprintf "\n\n------------------| %s\n\n" s
let dbg ?msg:(s="") n = match P.kind n with
  | K.Missing -> ()
  | k -> Printf.eprintf "Debugging %s '%s':<%s>\n" s (P.text n) (K.to_string k)
let dbg_ ?msg:(s="") n = PT.(
  Printf.eprintf "Debugging %s '%s':<%s>\n" s (text n) (kind n |> TK.to_string)
)
let string_of_fun_kind = function
  | Ast.FAsync -> "FAsync"
  | Ast.FSync  -> "FSync"
  | Ast.FGenerator -> "FGenerator"
  | Ast.FAsyncGenerator -> "FAsyncGenerator"



let drop_fst : string -> string = fun n -> String.sub n 1 (String.length n - 1)



type env = {
  (* Context of the file being parsed. *)
  mode      : FileInfo.mode;
  language  : string;
  filePath  : string;
  (* "Local" context. *)
  ancestry  : P.t list;
  saw_yield : bool ref;
}

type +'a parser      = P.t        -> env -> 'a
type +'a parser2     = P.t -> P.t -> env -> 'a
type +'a tokenparser = string     -> env -> 'a
type +'a parser'     = P.t list   -> env -> 'a

type ('a, 'b) metaparser  = 'a parser  -> 'b parser
type ('a, 'b) metaparser' = 'a parser' -> 'b parser'


let ppConst = fun x -> fun _ _ -> x
let ppPos = fun node env ->
  let text = P.source_text node in
  let mk_pos offset =
    let line, column =
      Full_fidelity_source_text.offset_to_position text offset
    in
    File_pos.of_line_column_offset ~line ~column ~offset
  in
  Pos.make_from_file_pos
    ~pos_file:(Relative_path.(create Dummy env.filePath))
    ~pos_start:(P.start_offset node |> mk_pos)
    ~pos_end:(P.end_offset node |> mk_pos)
let ppDescend : env parser = fun node env ->
  { env with ancestry = node :: env.ancestry }

let parent : env -> P.t = fun env -> List.hd env.ancestry
let where_am_I : env -> Pos.t = fun env -> ppPos (parent env) env


(* For debugging *)
let pDbg : string -> string -> unit parser = fun context name node env ->
  match P.kind node with
  | K.Missing -> Printf.eprintf "MISSING (%s->%s)\n" context name
  | k ->
    let line, col, line', col' = Pos.destruct_range (where_am_I env) in
    if line <> line'
    then Printf.eprintf "\nDebugging %s->%s <%s@(%d,%d)-(%d,%d)>: '%s'"
           context name
           (K.to_string k) line col line' col'
           (P.text node)
    else Printf.eprintf "\nDebugging %s->%s <%s@%d,%d-%d>: '%s'"
           context name
           (K.to_string k) line col col'
           (P.text node)
    ;
    Printf.eprintf "\nFull tree:\n";
    treeDBG node
let pDbgl : string -> string list -> P.t list -> env -> unit =
  fun context namel nodel env ->
    List.iter2 (fun name node -> pDbg context name node env) namel nodel






type 'a parser_spec = (K.t * 'a parser') list

exception API_parse_error of K.t list * env * P.t
exception API_token_parse_error of string list * env * string
exception Parser_fail of string



let (|.>) : ('a -> 'b) -> ('b -> 'c) -> ('a -> 'c) = fun g f -> fun x -> f (g x)
let (<.|) : ('b -> 'c) -> ('a -> 'b) -> ('a -> 'c) = fun f g -> fun x -> f (g x)
let (<$>) = fun f p n -> f <.| p n
let (<&>) = fun f f' e -> f e, f' e
let (<<>) = fun f f' e -> f (f' e) e
let (<|>) = fun p p' -> fun n e ->
  try p n e with
  | API_parse_error _
  | API_token_parse_error _ -> p' n e
let const : 'a -> 'b -> 'a = fun x _ -> x
let span : ('a -> bool) -> 'a list -> 'a list * 'a list = fun p ->
  let rec go = function
    | x :: xs when p x -> let (ls, rs) = go xs in (x::ls, rs)
    | xs -> ([], xs)
  in
  go
let rec last : 'a list -> 'a = function
  | [] -> failwith "last element of an empty list"
  | [x] -> x
  | (_::xs) -> last xs


let fail : K.t list -> 'a parser = fun kindl -> fun node env ->
  raise @@ API_parse_error (kindl, env, node)

let fail' : string list -> 'a tokenparser = fun expl -> fun str env ->
  raise @@ API_token_parse_error (expl, env, str)

let handle_parse_error : K.t list -> env -> P.t -> 'a = fun kindl env node ->
  let expected = String.concat ", " (List.map K.to_string kindl) in
  let actual   = K.to_string (P.kind node) in
  let kind_stack = String.concat ", " @@
    List.map (P.kind |.> K.to_string) env.ancestry
  in
  let msg = Printf.sprintf
    "%s Parse Error:
      Expecting one of: %s
      Actually found: %s
      Kind stack: %s
    "
    (ppPos node env |> Pos.to_absolute |> Pos.string)
    expected
    actual
    kind_stack
  in
  pDbg "EXCEPTION" "TRIGGER" node env;
  Printf.eprintf "\n\n";
  Printexc.print_backtrace stderr;
  raise (Failure msg)

let handle_token_parse_error : string list -> env -> string -> 'a =
  fun exp env actual ->
    let expected = "'" ^ String.concat "', '" exp ^ "'" in
    let kind_stack = String.concat ", " @@
      List.map (P.kind |.> K.to_string) env.ancestry
    in
    let msg = Printf.sprintf
      "%s Token Parse Error:
        Expecting one of: %s
        Actually found: %s
        Kind stack: %s
      "
      (where_am_I env |> Pos.to_absolute |> Pos.string)
      expected
      actual
      kind_stack
    in
    dbg ?msg:(Some "Exception trigger") (List.hd env.ancestry);
    raise (Failure msg)


let runP : 'a parser -> P.t -> env -> 'a = fun pThing thing env ->
  try pThing thing env with
  | API_parse_error       (kl, env, n) -> handle_parse_error       kl env n
  | API_token_parse_error (sl, env, n) -> handle_token_parse_error sl env n
  | e -> raise e




let pError' : 'a parser' = fun [ err ] -> raise @@ Parser_fail (P.text err)

let mkP : 'a . 'a parser_spec -> 'a parser = fun specl -> fun node env ->
    let env = ppDescend node env in
    let k = P.kind node in
    (*pDbg "mkP" (K.to_string k) node env;*)
    try
      let _, p =
        List.find (fun x -> k = fst x) (specl @ [K.ErrorSyntax, pError'])
      in
      (* Ugly little wrapper trick for token parsers (children -> []) *)
      p (if k = K.Token then [node] else P.children node) env
    with
    | Not_found -> fail (List.map fst specl) node env
    | Match_failure _ -> raise @@ Parser_fail begin Printf.sprintf
        "Failed to parse Full Fidelity tree; this probably means classic_ast \
         and full_fidelity_synax are out of sync. Things are Very Wrong if \
         this happens and you should definitely report seeing this to the Hack \
         team.\nThe ancestry that broke things was:\n%s" @@
         String.concat " <- " @@ List.map (P.kind |.> K.to_string) env.ancestry
      end

let single : K.t -> 'a parser' -> 'a parser = fun k p -> mkP [k, p]

let positional : 'a parser -> (Pos.t * 'a) parser = fun p ->
  fun node env -> ppPos node env, p node env

let ifExists : 'a -> 'a -> 'a parser =
  fun then_ else_ -> const <.| begin function
      | K.Missing -> else_
      | _ -> then_
    end <.| P.kind

let rec oneOf : 'a parser list -> 'a parser = function
  | [p] -> p
  | (p::ps) -> fun node env ->
      try p node env with
      | API_parse_error (kl, _, _) -> try (oneOf ps) node env with
        | API_parse_error (kl', env, n) -> fail (kl @ kl') n env





open Ast

(**
 * Constructors in OCaml aren't first-order. This is syntactically noisy. Wrap
 * some constructors in functions to make them denotationally bearable.
 *
 * Naming scheme;
 *   c<constructor> x = <constructor> x
 *)
let cStmt            x  = Stmt         x
let cExpr            x  = Expr         x
let cId              x  = Id           x
let cDo          (b, c) = Do       (b, c)
let cWhile       (c, b) = While    (c, b)
let cLvar            x  = Lvar         x
let cSome            x  = Some         x
let cHapply      (p, x) = Happly   (p, x)
let cHoption         x  = Hoption      x
let cHtuple          x  = Htuple       x
let cHshape          x  = Hshape       x
let cBlock              = function [Noop] -> Block [] | x -> Block x
let cBreak           x  = Break        x
let cThrow           x  = Throw        x
let cContinue        x  = Continue     x
let cReturn      (p, x) = Return   (p, x)
let cExpr_list       x  = Expr_list    x
let cStatic_var      x  = Static_var   x
let cClass_get   (p, x) = Class_get (p, x)
let cClass_const (p, x) = Class_const (p, x)


let rec hint_to_fun_kind : hint -> fun_kind = fun (pos, hint) -> match hint with
  | Hfun (_,_,h) -> hint_to_fun_kind h
  | Happly ((_, "Generator"), _) -> FGenerator
  | Happly ((_, "AsyncGenerator"), _) -> FAsyncGenerator
  | _ -> FSync

let async_fun_kind : fun_kind -> fun_kind = function
  | FAsyncGenerator | FGenerator -> FAsyncGenerator
  | FAsync          | FSync      -> FAsync

let sync_fun_kind : fun_kind -> fun_kind = function
  | FAsyncGenerator | FGenerator -> FGenerator
  | FAsync          | FSync      -> FSync

let mk_noop : stmt list -> stmt list = function
  | [] -> [Noop]
  | s -> s

let stmt_list_of_stmt = function
  | Block block -> List.filter (fun x -> x <> Noop) block
  | stmt -> [stmt]
let mpStripNoop pThing node env = match pThing node env with
  | [Noop] -> []
  | stmtl -> stmtl
let mpOptional : ('a, 'a option) metaparser = fun p ->
  cSome <$> p <|> ppConst None



let mpSingleton = fun p ->
  (fun x -> [x]) <$> p

let mpPos' : ('a, Pos.t * 'a) metaparser' = fun p n ->
  where_am_I <&> p n

let mpArgKOfN : int -> int -> 'a parser -> 'a parser' = fun k n p -> function
  | nodel when List.length nodel = n -> p (List.nth nodel k)
  | nodel -> raise @@ Match_failure ("mpArgKOfN", n, List.length nodel)




let pos_name node env = ppPos node env, P.text node

let couldMap : 'a . f:'a parser -> 'a list parser = fun ~f -> fun node env ->
  let rec synmap : 'a . 'a parser -> 'a list parser = fun f node env -> P.(
    match syntax node with
    | SyntaxList        l -> List.flatten @@
        List.map (fun n -> go ~f n (ppDescend node env)) l
    | ListItem          i -> [f i.list_item @@ ppDescend node env]
    | _ -> [f node env]
  )
  and go : 'a . f:'a parser -> 'a list parser = fun ~f -> function
    | node when P.is_missing node -> const []
    | node -> synmap f node
  in
  go ~f node env



let pToken : f:('a tokenparser) -> 'a parser = fun ~f ->
  fun node env -> match P.syntax node with
    | P.Token t -> f (PT.text t) env
    | _ -> fail [K.Token] node env





(**
 * Terminal parsers
 * ----------------
 *
 * Constructed using mkTP, name-schema is t<result_type>.
 *)

type 'a terminal_spec = string * (TK.t -> 'a)

let mkTP : 'a terminal_spec -> 'a parser = fun (name, spec) ->
  fun node env -> (* parser *)
    try (match P.syntax node with | P.Token t -> spec (PT.kind t)) with
    (* We don't care whether it was the inner or outer match that failed. *)
    | Match_failure _ -> raise @@
        API_token_parse_error ([name], env, P.text node)

(**
 * FFP does not destinguish between ++$i and $i++ on the level of token kind
 * annotation. Prevent duplication by switching on `postfix` for the two
 * operatores for which AST /does/ differentiate between fixities.
 *)
let tUop : bool -> (expr -> expr_) terminal_spec = fun postfix ->
  "Unary operator", fun token_kind expr -> match token_kind with
    | TK.PlusPlus   when postfix -> Unop (Upincr, expr)
    | TK.MinusMinus when postfix -> Unop (Updecr, expr)
    | TK.PlusPlus                -> Unop (Uincr,  expr)
    | TK.MinusMinus              -> Unop (Udecr,  expr)
    | TK.Exclamation             -> Unop (Unot,   expr)
    | TK.Tilde                   -> Unop (Utild,  expr)
    | TK.Plus                    -> Unop (Uplus,  expr)
    | TK.Minus                   -> Unop (Uminus, expr)
    | TK.Ampersand               -> Unop (Uref,   expr)
    (* The ugly duckling; In the FFP, `await` is parsed as a UnaryOperator,
     * whereas the typed AST has separate constructors for Await and Uop. This
     * is why we don't just project onto a `uop`, but rather a `expr -> expr_`.
     *)
    | TK.Await                   -> Await expr

let tBop : (expr -> expr -> expr_) terminal_spec =
  "Binary operator", fun token_kind -> fun lhs rhs -> match token_kind with
    | TK.Equal                       -> Binop (Eq None,           lhs, rhs)
    | TK.Bar                         -> Binop (Bar,               lhs, rhs)
    | TK.Ampersand                   -> Binop (Amp,               lhs, rhs)
    | TK.Plus                        -> Binop (Plus,              lhs, rhs)
    | TK.Minus                       -> Binop (Minus,             lhs, rhs)
    | TK.Star                        -> Binop (Star,              lhs, rhs)
    | TK.Slash                       -> Binop (Slash,             lhs, rhs)
    | TK.Dot                         -> Binop (Dot,               lhs, rhs)
    | TK.Percent                     -> Binop (Percent,           lhs, rhs)
    | TK.LessThan                    -> Binop (Lt,                lhs, rhs)
    | TK.GreaterThan                 -> Binop (Gt,                lhs, rhs)
    | TK.EqualEqual                  -> Binop (Eqeq,              lhs, rhs)
    | TK.LessThanEqual               -> Binop (Lte,               lhs, rhs)
    | TK.GreaterThanEqual            -> Binop (Gte,               lhs, rhs)
    | TK.StarStar                    -> Binop (Starstar,          lhs, rhs)
    | TK.ExclamationEqual            -> Binop (Diff,              lhs, rhs)
    | TK.BarEqual                    -> Binop (Eq (Some Bar),     lhs, rhs)
    | TK.PlusEqual                   -> Binop (Eq (Some Plus),    lhs, rhs)
    | TK.MinusEqual                  -> Binop (Eq (Some Minus),   lhs, rhs)
    | TK.StarEqual                   -> Binop (Eq (Some Star),    lhs, rhs)
    | TK.SlashEqual                  -> Binop (Eq (Some Slash),   lhs, rhs)
    | TK.DotEqual                    -> Binop (Eq (Some Dot),     lhs, rhs)
    | TK.PercentEqual                -> Binop (Eq (Some Percent), lhs, rhs)
    | TK.CaratEqual                  -> Binop (Eq (Some Xor),     lhs, rhs)
    | TK.AmpersandEqual              -> Binop (Eq (Some Amp),     lhs, rhs)
    | TK.BarBar                      -> Binop (BArbar,            lhs, rhs)
    | TK.AmpersandAmpersand          -> Binop (AMpamp,            lhs, rhs)
    | TK.LessThanLessThan            -> Binop (Ltlt,              lhs, rhs)
    | TK.GreaterThanGreaterThan      -> Binop (Gtgt,              lhs, rhs)
    | TK.EqualEqualEqual             -> Binop (EQeqeq,            lhs, rhs)
    | TK.LessThanLessThanEqual       -> Binop (Eq (Some Ltlt),    lhs, rhs)
    | TK.GreaterThanGreaterThanEqual -> Binop (Eq (Some Gtgt),    lhs, rhs)
    | TK.ExclamationEqualEqual       -> Binop (Diff2,             lhs, rhs)
    (* The ugly duckling; In the FFP, `|>` is parsed as a BinaryOperator,
     * whereas the typed AST has separate constructors for Pipe and Binop. This
     * is why we don't just project onto a `bop`, but - analagous to tUop - a
     * `expr -> expr -> expr_`.
     *)
    | TK.BarGreaterThan              -> Pipe (lhs, rhs)

let tConstraintKind : constraint_kind terminal_spec =
  "Constraint kind", function
  | TK.As    -> Constraint_as
  | TK.Super -> Constraint_super
  | TK.Equal -> Constraint_eq

let tTypedefKind : (hint -> typedef_kind) terminal_spec =
  "Typedef kind", fun token_kind -> fun ty -> match token_kind with
  | TK.Newtype -> NewType ty
  | TK.Type    -> Alias   ty

let tImportFlavor : import_flavor terminal_spec =
  "Import flavor", function
  | TK.Include      -> Include
  | TK.Require      -> Require
  | TK.Include_once -> IncludeOnce
  | TK.Require_once -> RequireOnce

let tNullFlavor : og_null_flavor terminal_spec =
  "Object-get null flavor", function
  | TK.QuestionMinusGreaterThan -> OG_nullsafe
  | TK.MinusGreaterThan         -> OG_nullthrows

let tIsXHP : bool terminal_spec =
  "XHP prefix", function
  | TK.XHPElementName
  | TK.XHPClassName -> true
  | _k -> (* Printf.eprintf "!!!! %s\n" (TK.to_string k);*) false

let tKind : kind terminal_spec =
  "Kind", function
  | TK.Final     -> Final
  | TK.Static    -> Static
  | TK.Abstract  -> Abstract
  | TK.Private   -> Private
  | TK.Public    -> Public
  | TK.Protected -> Protected

let tClassKind : class_kind terminal_spec =
  "Class kind", function
  | TK.Class     -> Cnormal
  | TK.Interface -> Cinterface
  | TK.Trait     -> Ctrait
  | TK.Enum      -> Cenum


let tTraitReqKind : trait_req_kind terminal_spec =
  "Trait requirement kind", function
  | TK.Implements -> MustImplement
  | TK.Extends    -> MustExtend

let syntax_of_token : P.PositionedToken.t -> P.t = fun t ->
  { P.syntax = P.Token t
  ; P.value  = P.PositionedSyntaxValue.make t.PT.source_text
                 t.PT.offset t.PT.leading_width t.PT.width t.PT.trailing_width
  }
let prepString2 : P.t list -> P.t list =
  let trimLeft t =
    PT.({ t with leading_width = t.leading_width + 1; width = t.width - 1 })
  in
  let trimRight t =
    PT.({ t with trailing_width = t.trailing_width + 1; width = t.width - 1 })
  in
  function
  | ({P.syntax = P.Token t; _}::ss)
  when t.PT.width > 0 && (PT.text t).[0] = '"' ->
    let rec unwind = function
      | [{ P.syntax = P.Token t; _ }]
      when t.PT.width > 0 && (PT.text t).[t.PT.width - 1] = '"' ->
        let s = syntax_of_token (trimRight t) in
        if P.width s > 0 then [s] else []
      | x :: xs -> x :: unwind xs
      | _ -> raise (Invalid_argument "Malformed String2 SyntaxList")
    in
    let s = syntax_of_token (trimLeft t) in
    if P.width s > 0 then s :: unwind ss else unwind ss
  | x -> x (* unchanged *)

let mkString : (string -> string) -> string -> string = fun unescaper content ->
  let no_quotes = try
      if String.sub content 0 3 = "<<<" (* The heredoc case *)
      then String.sub content 3 (String.length content - 4)
      else String.sub content 1 (String.length content - 2)
    with
    | Invalid_argument _ -> content
  in
  try unescaper no_quotes with
  | Php_escaping.Invalid_string error -> raise @@
      Failure (Printf.sprintf "Malformed string literal <<%s>>" no_quotes)
(* TODO: Clean up string escaping *)
let unempty_str = function
  | "''" | "\"\"" -> ""
  | s -> s
let unesc_dbl s = unempty_str @@ Php_escaping.unescape_double s
let unesc_sgl s = unempty_str @@ Php_escaping.unescape_single s
let unesc_xhp s =
  let whitespace = Str.regexp "[ \t\n\r\012]+" in
  let s = Str.global_replace whitespace " " s in
  let quotes = Str.regexp " ?\"\\([^\"]*\\)\" ?" in
  if Str.string_match quotes s 0
  then Str.matched_group 1 s
  else s




let tLiteral : (Pos.t -> string -> expr_) terminal_spec =
  "Literal", fun token_kind -> fun p text -> match token_kind with
  | TK.DecimalLiteral
  | TK.OctalLiteral
  | TK.HexadecimalLiteral
  | TK.BinaryLiteral             -> Int   (p, text)
  | TK.FloatingLiteral           -> Float (p, text)
  | TK.SingleQuotedStringLiteral -> String (p, mkString unesc_sgl text)
  | TK.DoubleQuotedStringLiteral
  | TK.HeredocStringLiteral
  | TK.NowdocStringLiteral       -> String (p, mkString unesc_dbl text)
  | TK.NullLiteral               -> Null
  | TK.BooleanLiteral            ->
      (match text with "false" -> False | "true" -> True)

(* END OF TOKEN PARSERS *)


let pAwait : Pos.t parser =
  fst <$> positional (mkTP ("await", function TK.Await -> ()))


let mpShapeField : ('a, (shape_field_name * 'a)) metaparser = fun pThing ->
  fun node env ->
    let pSFlit n e = let p, n = pos_name n e in SFlit (p, mkString unesc_dbl n) in
    let pSFclass_const =
      single K.ScopeResolutionExpression @@
        fun [ qual; _op; name ] env ->
          SFclass_const (pos_name qual env, pos_name name env)
    in
    let pNamedThing' = fun [name; _arr; thing] env ->
      (pSFclass_const <|> pSFlit) name env, pThing thing env
    in
    mkP
    [ (K.FieldSpecifier,   pNamedThing')
    ; (K.FieldInitializer, pNamedThing')
    ] node env

let rec pHint : hint parser = fun eta env ->
  let pTypeArguments : hint list parser =
    single K.TypeArguments @@ mpArgKOfN 1 3 (couldMap ~f:pHint)
  in
  let pHint_ : hint_ parser = mkP
    (* Dirty hack; CastExpression can have type represented by token *)
    [ (K.Token,                  fun ns e ->
        Happly (mpArgKOfN 0 1 pos_name ns e, []))
    ; (K.SimpleTypeSpecifier,    fun ns e ->
        Happly (mpArgKOfN 0 1 pos_name ns e, []))
    ; (K.ShapeTypeSpecifier,     cHshape <$> mpArgKOfN 2 4 @@
        couldMap ~f:(mpShapeField pHint))
    ; (K.TupleTypeSpecifier,     cHtuple <$> mpArgKOfN 1 3 @@
        couldMap ~f:pHint)
    ; (K.KeysetTypeSpecifier, mpArgKOfN 2 4 @@ fun n env ->
        Happly ((where_am_I env, "keyset"), couldMap ~f:pHint n env))
    ; (K.VectorTypeSpecifier, mpArgKOfN 2 4 @@ fun n env ->
        Happly ((where_am_I env, "vec"), couldMap ~f:pHint n env))
    ; (K.VectorArrayTypeSpecifier, mpArgKOfN 2 4 @@ fun n env ->
        Happly ((where_am_I env, "array"), couldMap ~f:pHint n env))
    ; (K.MapArrayTypeSpecifier,  fun [ kw; _la; key; _comma; value; _ra ] env ->
        cHapply (pos_name kw env, List.map (fun x -> pHint x env) [key; value]))
    ; (K.DictionaryTypeSpecifier,  fun [ kw; _la; key; _comma; value; _ra ] env ->
        cHapply (pos_name kw env, List.map (fun x -> pHint x env) [key; value]))
    ; (K.ClassnameTypeSpecifier, fun [ name; _la; ty; _ra ] ->
        cHapply <.| (pos_name name <&> mpSingleton pHint ty))
    ; (K.TypeConstant,           fun [ base; _sep; child ] env -> begin
          let child = pos_name child env in
          match snd @@ pHint base env with
          | Haccess (b, c, cs) -> Haccess (b, c, cs @ [child])
          | Happly (h, []) -> Haccess (h, child, [])
        end
      )
    ; (K.GenericTypeSpecifier,   fun [class_type; arguments] ->
        cHapply <.| (pos_name class_type <&> pTypeArguments arguments))
    ; (K.NullableTypeSpecifier,  fun [ question; ty ] env ->
        ifExists cHoption snd question env (pHint ty env))
    ; (K.SoftTypeSpecifier,      fun [ _at; ty ] env -> snd @@ pHint ty env)
    ; (K.ClosureTypeSpecifier,
        fun [ _olp; _kw; _lp; params; _rp; _colon; ret; _orp ] env ->
          Hfun (couldMap ~f:pHint params env, false, pHint ret env)
      )
    ]
  in
  positional pHint_ eta env

let rec p2KeyValue : as_expr parser2 = fun key value env ->
  let value = pExpr value env in
  match mpOptional pExpr key env with
    | Some k -> As_kv (k, value)
    | None   -> As_v      value
and pSimpleInitializer : expr parser = fun eta -> eta |> single
    K.SimpleInitializer
    (fun [ _eq; value ] -> pExpr value)
and pStaticDeclarator = fun eta -> single
    K.StaticDeclarator
    (fun [ name; init ] env ->
      let lhs = match pExpr name env with
        | p, Id (p', s) -> p, Lvar (p', s)
        | x -> x
      in
      match mpOptional pSimpleInitializer init env with
        | None -> lhs
        | Some rhs -> where_am_I env, Binop (Eq None, lhs, rhs)
    )
    eta
and pFunParam : bool -> fun_param parser = fun is_constructor node env ->
  mkP
  [ (K.ParameterDeclaration, fun [ attrs; vis; ty; name; def ] env ->
      let is_variadic, param_name = match P.syntax name with
        | P.DecoratedExpression
            { P.decorated_expression_decorator = dec
            ; decorated_expression_expression = param_name
            }
          -> K.Token = P.kind dec && P.text dec = "...", param_name
        | _ -> false, name
      in
      { param_hint            = mpOptional pHint ty env
      ; param_is_reference    = false
      ; param_is_variadic     = is_variadic
      ; param_id              = pos_name param_name env
      ; param_expr            = mpOptional pSimpleInitializer def env
      (* implicit field via constructor parameter.
       * This is always None except for constructors and the modifier
       * can be only Public or Protected or Private.
       *)
      ; param_modifier        = begin
          let rec go l = begin match l with
            | [] -> None
            | x::xs when List.mem x [Private; Public; Protected] -> Some x
            | x::xs -> go xs
          end in
          go (couldMap ~f:(mkTP tKind) vis env)
        end
      ; param_user_attributes = List.flatten @@ couldMap ~f:pUserAttribute attrs env
      }
    )
  ; (K.Token, fun [t] env -> pToken ~f:(fun "..." _ ->
      { param_hint            = None
      ; param_is_reference    = false
      ; param_is_variadic     = true
      ; param_id              = pos_name t env
      ; param_expr            = None
      ; param_modifier        = None
      ; param_user_attributes = []
      }
    ) t env)
  ] node env
and pLambdaSig : (Ast.fun_param list * Ast.hint option) parser = fun node -> mkP
  [ (K.LambdaSignature, fun [ _lparen; paraml; _rparen; _colon; ret ] env ->
      couldMap ~f:(pFunParam false) paraml env, mpOptional pHint ret env
    )
  ; (K.Token, fun [t] env -> couldMap t env ~f:(pToken ~f:begin fun s env ->
        { param_hint            = None
        ; param_is_reference    = false
        ; param_is_variadic     = false
        ; param_id              = pos_name t env
        ; param_expr            = None
        ; param_modifier        = None
        ; param_user_attributes = []
        }
      end), None)
  ] node
and pLambdaBody : block parser = fun eta ->
  oneOf [ pBlock ; (fun (p,r) -> [ Return (p, Some (p, r)) ]) <$> pExpr ] eta
and pLambda : fun_ parser' = fun [ async; signature; _arrow; body ] env ->
  let p = where_am_I env in
  let previous_saw_yield = !(env.saw_yield) in
  env.saw_yield := false;
  let body = pLambdaBody body env in
  let body_has_yield = !(env.saw_yield) in
  env.saw_yield := previous_saw_yield;
  let paraml, ret = pLambdaSig signature env in
  let fun_kind =
    if body_has_yield
    then ifExists FAsyncGenerator FGenerator async env
    else ifExists FAsync FSync async env
  in
  { f_mode            = env.mode
  ; f_tparams         = []
  ; f_ret             = ret
  ; f_ret_by_ref      = false
  ; f_name            = p, ";anonymous"
  ; f_params          = paraml
  ; f_body            = body
  ; f_user_attributes = []
  ; f_fun_kind        = fun_kind
  ; f_namespace       = Namespace_env.empty_with_default_popt (*TODO*)
  ; f_span            = p
  }
and pUserAttribute : user_attribute list parser = fun eta -> eta |>
  single K.AttributeSpecification @@ mpArgKOfN 1 3 begin
    couldMap ~f:begin
      single K.Attribute @@ fun [ name; _lp; vals; _rp ] env ->
        { ua_name = pos_name name env
        ; ua_params = couldMap ~f:pExpr vals env
        }
    end
  end
and pAField : afield parser = fun node env ->
  let pElemInit = single K.ElementInitializer @@ fun [ key; _arr; value ] env ->
    AFkvalue (pExpr key env, pExpr value env)
  in
  match mpOptional pElemInit node env with
  | None   -> AFvalue (pExpr node env)
  | Some x -> x
and pExpr_EvalExpression' : expr_ parser' = fun [ _kw; _lp; arg; _rp ] env ->
  let (p, kw) as f = pos_name _kw env in
  Call ((p, Id f), [pExpr arg env], [])
and pExpr_EmptyExpression' : expr_ parser' = fun [_kw; _lp; arg; _rp ] env ->
  let (p, kw) as f = pos_name _kw env in
  Call ((p, Id f), [pExpr arg env], [])
and pExpr_IssetExpression' : expr_ parser' = fun [_kw; _lp; args; _rp ] env ->
  let (p, kw) as f = pos_name _kw env in
  Call ((p, Id f), couldMap ~f:pExpr args env, [])
and pExpr_LiteralExpression' : expr parser' = fun [lit] env -> where_am_I env,
  mkP
  [ (K.Token, fun [t] env -> mkTP tLiteral t env (where_am_I env) (P.text t))
  ; (K.SyntaxList, fun ss env -> String2
      (couldMap ~f:(pExpr ?top_level:(Some false))
        { lit with P.syntax = P.SyntaxList (prepString2 ss) } env
      )
    )
  ] lit env
and pExpr_InclusionExpression' : expr parser' = fun [ req; file ] env ->
  let flavor = mkTP tImportFlavor req env in
  where_am_I env, Import (flavor, pExpr file env)
and pExpr_ParenthesizedExpression' : expr parser' = fun [ _lp; expr; _rp ] ->
  pExpr expr
and pExpr_ScopeResolutionExpression' : expr_ parser' =
  fun [ qual; _op; name ] env ->
    let (_,n) as name = pos_name name env in
    (if n.[0] = '$'
     then cClass_get
     else cClass_const
    ) (pos_name qual env, name)
and pXHPNestedExpression : expr parser = fun eta -> eta |> mkP
  [ (K.Token, fun [str] env -> let pos = where_am_I env in
      pos, String (pos, unesc_xhp @@ P.full_text str)
    )
  ; (K.XHPExpression, fun nodel env ->
      where_am_I env, pExpr_XHPExpression' nodel env
    )
  ; (K.BracedExpression, mpArgKOfN 1 3 pExpr)
  ]
and pXHPAttribute : (id * expr) parser = fun eta -> eta |>
  single K.XHPAttribute @@ fun [ name; _eq; expr ] env ->
    (pos_name name env, pXHPNestedExpression expr env)
and pXHPOpen : (id * (id * expr) list)  parser' = fun [ name; attrs; _ra ] env ->
  let (pos, name) = pos_name name env in
  let trim_name = String.trim name in
  let name = String.sub trim_name 1 (String.length trim_name - 1) in
  ((pos, ":" ^ name), couldMap ~f:pXHPAttribute attrs env)
and pExpr_XHPExpression' : expr_ parser' = fun [ op; body; _cl ] env ->
  let name, attrs = single K.XHPOpen pXHPOpen op env in
  let is_token x = P.kind x = K.Token in
  let flatten_tokens = function
  | [t] -> t
  | (t::_) as ts ->
    let token =
      match P.syntax t with
      | P.Token tok -> tok
      | _ -> failwith "Expecting a Token"
    in
    let l = match P.syntax (last ts) with
      | P.Token tok -> tok
      | _ -> failwith "Expecting a Token"
    in
    let kind = PT.kind token in
    let source_text = PT.source_text token in
    let offset = PT.leading_start_offset token in
    let width = PT.start_offset l - PT.start_offset token + PT.width l in
    let leading = PT.leading token in
    let trailing = PT.trailing l in
    syntax_of_token @@ PT.make kind source_text offset width leading trailing
  in
  let aggregate_tokens =
    let rec go xs =
      let t, rs = match span is_token xs with
        | [], rs -> None, rs
        | ts, rs -> Some (flatten_tokens ts), rs
      in
      let tf =
        Option.value_map ~default:(fun x -> x) ~f:(fun x y -> x :: y) t
      in
      match span (fun x -> not (is_token x)) rs with
      | rs, [] -> tf rs
      | rs, ts -> tf rs @ go ts
    in
    go
  in
  let body = match P.syntax body with
    | P.SyntaxList ((_::_) as ss) ->
      { body with P.syntax = P.SyntaxList (aggregate_tokens ss) }
    | _ -> body
  in
  Xml (name, attrs, couldMap ~f:pXHPNestedExpression body env)
and pExpr ?top_level:(top_level=true) : expr parser = fun eta -> eta |>
  positional @@ mkP
  [ (K.ParenthesizedExpression,     fun [ _lp; expr; _rp ] ->
      (snd <$> pExpr) expr)
  ; (K.BracedExpression,            mpArgKOfN 1 3 (snd <$> pExpr))
  ; (K.EvalExpression,              pExpr_EvalExpression')
  ; (K.EmptyExpression,             pExpr_EmptyExpression')
  ; (K.IssetExpression,             pExpr_IssetExpression')
  ; (K.InclusionExpression,         snd <$> pExpr_InclusionExpression')
  ; (K.ArrayIntrinsicExpression,    fun [ _kw; _lp; members; _rp ] env ->
      Array (couldMap ~f:pAField members env))
  ; (K.DictionaryIntrinsicExpression, fun [ kw; _lb; members; _rb ] env ->
      Collection (pos_name kw env, couldMap ~f:pAField members env)
    )
  ; (K.KeysetIntrinsicExpression,     fun [ kw; _lb; members; _rb ] env ->
      Collection (pos_name kw env, couldMap ~f:pAField members env)
    )
  ; (K.VectorIntrinsicExpression,     fun [ kw; _lb; members; _rb ] env ->
      Collection (pos_name kw env, couldMap ~f:pAField members env)
    )
  ; (K.ShapeExpression,             fun [ _kw; _lp; fields; _rp ] env ->
      Shape (couldMap ~f:(mpShapeField pExpr) fields env))
  ; (K.CollectionLiteralExpression, fun [ name; _lb; initl; _rb ] env ->
      Collection (pos_name name env, couldMap ~f:pAField initl env))
  ; (K.DecoratedExpression,         fun [ _dec; expr ] -> (snd <$> pExpr) expr)
  ; (K.Token,                       fun [ t ] env -> if top_level
                                      then Id (pos_name t env)
                                      else
                                        let s = unesc_dbl (P.text t) in
                                        String (where_am_I env, s)
    )
  ; (K.QualifiedNameExpression,     fun [ n ] env -> Id (pos_name n env))
  ; (K.VariableExpression,          fun [ v ] env -> Lvar (pos_name v env))
  ; (K.TupleExpression,             fun [ kw; _lp; items; _rp ] env -> Call
      ( (ppPos kw env, Id (pos_name kw env))
      , couldMap ~f:pExpr items env
      , []
      )
    )
  ; (K.PipeVariableExpression,      ppConst Dollardollar)
  ; (K.MemberSelectionExpression,   fun [ obj; op; name ] env ->
      let lhs = pExpr obj env in
      let null_flavor = mkTP tNullFlavor op env in
      let rhs = ppPos name env, Id (pos_name name env) in
      Obj_get (lhs, rhs, null_flavor)
    )
  ; (K.SafeMemberSelectionExpression, fun [ obj; _op; name ] env ->
      Obj_get (pExpr obj env, pExpr name env, OG_nullsafe)
    )
  ; (K.SubscriptExpression, fun [ recv; _lb; idx; _rb ] env ->
      Array_get (pExpr recv env, mpOptional pExpr idx env))
  ; (K.ScopeResolutionExpression, pExpr_ScopeResolutionExpression')
  ; (K.FunctionCallExpression, fun [ recv; _lp; args; _rp ] env ->
        Call (pExpr recv env, couldMap ~f:pExpr args env, []))
  ; (K.LiteralExpression, snd <$> pExpr_LiteralExpression')
  ; (K.PrintExpression, fun [ _kw; expr ] env ->
      let pos = where_am_I env in
      let args = couldMap ~f:pExpr expr env in
      Call ((pos, Id (pos, "echo")), args, [])
    )
  ; (K.YieldExpression, fun [ _kw; arg ] env ->
      env.saw_yield := true;
      if P.text arg = "break"
      then Yield_break
      else Yield (pAField arg env)
    )
  ; (K.ListExpression, fun [ _kw; _lp; members; _rp ] env ->
      List (couldMap ~f:pExpr members env))
  ; (K.CastExpression, fun [ _lp; ty; _rp; arg ] env ->
      Cast (pHint ty env, pExpr arg env))
  ; (K.PrefixUnaryExpression,  fun [ op; arg ] env ->
      mkTP (tUop false) op env @@ pExpr arg env)
  ; (K.PostfixUnaryExpression, fun [ arg; op ] env ->
      mkTP (tUop true) op env @@ pExpr arg env)
  ; (K.BinaryExpression, fun [ l; op; r ] env ->
      mkTP tBop op env (pExpr l env) (pExpr r env))
  ; (K.ConditionalExpression, fun [ test; _q; then_; _colon; else_ ] env ->
      Eif (pExpr test env, Some (pExpr then_ env), pExpr else_ env)
    )
  ; (K.InstanceofExpression, fun [ thing; _op; ty ] env ->
      let ty = match pExpr ty env with
        | p, Class_const (pid, (_,"")) -> p, Id pid
        | ty -> ty
      in
      InstanceOf (pExpr thing env, ty)
    )
  ; (K.ArrayCreationExpression, fun [ _lb; members; _rb ] env ->
      Array (couldMap ~f:pAField members env))
  ; (K.ObjectCreationExpression, fun [ _new; ty; _lp; args; _rp ] env ->
      New (pExpr ty env, couldMap ~f:pExpr args env, [])
    )
  ; (K.AnonymousFunction,
      fun [ async; _kw; _lp; params; _rp; _colon; ret; use; body ] env ->
        let pUse = single K.AnonymousFunctionUseClause @@
          fun [ _kw; _lp; vars; _rp ] ->
            couldMap ~f:((fun x -> x, false) <$> pos_name) vars
        in
        let previous_saw_yield = !(env.saw_yield) in
        env.saw_yield := false;
        let body = pBlock body env in
        let body_has_yield = !(env.saw_yield) in
        env.saw_yield := previous_saw_yield;
        let fun_ =
          { f_mode            = env.mode
          ; f_tparams         = []
          ; f_ret             = mpOptional pHint ret env
          ; f_ret_by_ref      = false
          ; f_name            = where_am_I env, ";anonymous"
          ; f_params          = couldMap ~f:(pFunParam false) params env
          ; f_body            = mk_noop body
          ; f_user_attributes = []
          ; f_fun_kind        = ifExists async_fun_kind sync_fun_kind async env
              @@ if body_has_yield then FGenerator else FSync
          ; f_namespace       = Namespace_env.empty_with_default_popt
          ; f_span            = where_am_I env
          }
        in
        Efun (fun_, (pUse <|> ppConst []) use env)
    )
  ; (K.XHPExpression, pExpr_XHPExpression')
  ; (K.LambdaExpression, fun node env -> Lfun (pLambda node env))
  ; (K.AwaitableCreationExpression, fun [ _async; blk ] env ->
      let previous_saw_yield = !(env.saw_yield) in
      env.saw_yield := false;
      let block = pBlock blk env in
      let blk_has_yield = !(env.saw_yield) in
      env.saw_yield := previous_saw_yield;
      let body =
        { f_mode            = env.mode
        ; f_tparams         = []
        ; f_ret             = None
        ; f_ret_by_ref      = false
        ; f_name            = where_am_I env, ";anonymous"
        ; f_params          = []
        ; f_body            = mk_noop block
        ; f_user_attributes = []
        ; f_fun_kind        = if blk_has_yield then FAsyncGenerator else FAsync
        ; f_namespace       = Namespace_env.empty_with_default_popt
        ; f_span            = where_am_I env
        }
      in
      Call ((where_am_I env, Lfun body), [], [])
    )
  (* FIXME; should this include Missing? ; (K.Missing, ppConst Null)*)
  ]


and pBlock : block parser = fun node env -> stmt_list_of_stmt (pStmt node env)
and pSwitch : case list parser = fun node env ->
  let pTaggedStmt : [< `Stmt of stmt | `Func of block -> case ] parser =
    (fun x -> `Stmt x) <$> pStmt in
  let pTaggedFunc : [< `Stmt of stmt | `Func of block -> case ] parser =
    let optStmt stmt env block = match mpOptional pStmt stmt env with
      | None -> block
      | Some s -> s :: block
    in
    (fun x -> `Func x) <$> mkP
    [ (K.DefaultLabel, fun [ _kw; _col; stmt ] env -> fun x ->
        Default (optStmt stmt env x)
      )
    ; (K.CaseLabel, fun [ _kw; expr; _col; stmt ] env -> fun x ->
        Case (pExpr expr env, optStmt stmt env x)
      )
    ]
  in
  let pCaseLine : [< `Stmt of stmt | `Func of block -> case ] parser
      = oneOf [ pTaggedStmt ; pTaggedFunc ]
  in
  let merge thing acc = match thing, acc with
    | `Func f, (stmtl, casel) -> ([], f stmtl :: casel)
    | `Stmt x, (stmtl, casel) -> x::stmtl, casel
  in
  let inner : [< `Func of block -> case | `Stmt of stmt] list
    = single
        K.CompoundStatement
        (fun [ _lb; stmts; _rb ] -> couldMap ~f:pCaseLine stmts)
        node
        env
  in
  snd @@ List.fold_right merge inner ([],[])
and pStmt_EchoStatement' : stmt parser' = fun [ kw; exprs; _semi ] env ->
  let pEcho : env -> expr_ = fun env ->
    Call (positional (cId <$> pos_name) kw env, couldMap ~f:pExpr exprs env, [])
  in
  (cExpr <.| (where_am_I <&> pEcho)) env
and pStmt_ExpressionStatement' : stmt parser' = fun [ expr; _semi ] env ->
  ifExists (fun () -> Expr (pExpr expr env)) (fun () -> Noop) expr env ()
and pStmt_CompoundStatement' : stmt parser' = fun [ _lb; stmts; _rb ] env ->
  Block (couldMap ~f:pStmt stmts env)
and pCompoundStatement : stmt parser = fun node env ->
  ifExists (single K.CompoundStatement pStmt_CompoundStatement' node env) Noop
    node env
and pStmt_ThrowStatement' : stmt parser' = fun [ _kw; expr; _semi ] env ->
  Throw (pExpr expr env)
and pStmt_IfStatement' : stmt parser' =
  fun [ _kw; _lparen; cond; _rparen; then_; elseif; else_ ] env ->
    let pElseIf' = fun [ _kw; _lp; cond; _rp; then_ ] env -> fun cont -> [ If
      ( pExpr cond env
      , [(pCompoundStatement <|> pStmt) then_ env]
      , cont
      ) ]
    in
    let elseifs = couldMap ~f:(single K.ElseifClause pElseIf') elseif env in
    let pElseClause = single K.ElseClause @@
      mpArgKOfN 1 2 (pCompoundStatement <|> pStmt)
    in
    If
    ( pExpr cond env
    , [(pCompoundStatement <|> pStmt) then_ env]
    , List.fold_right (fun a b -> a b) elseifs [(pElseClause <|> ppConst Noop) else_ env]
    )
and pStmt_DoStatement' : stmt parser' =
  fun [ _kw_do; body; _kw_while; _lp; cond; _rp; _semi ] env ->
    Do ([Block (pBlock body env)], pExpr cond env)
and pStmt_WhileStatement' : stmt parser' =
  fun [ _kw; _lp; cond; _rp; body ] env ->
    While (pExpr cond env, [(pCompoundStatement <|> pStmt) body env])
and pStmt_ForStatement' : stmt parser' =
  fun [ _kw; _lp; init; _semi; ctrl; _semi'; eol; _rp; body ] env ->
    let pExprL = positional (cExpr_list <$> couldMap ~f:pExpr) in
      For
      ( pExprL init env
      , pExprL ctrl env
      , pExprL eol env
      , [Block (pBlock body env)]
      )
and pStmt_SwitchStatement' : stmt parser' =
  fun [ _kw; _lp; expr; _rp; body ] env ->
    Switch (pExpr expr env, pSwitch body env)
and pStmt_ForeachStatement' : stmt parser' =
  fun [ _kw; _lp; collection; await; _as; key; _arrow; value; _rp; body ] env ->
    Foreach
    ( pExpr collection env
    , mpOptional pAwait await env
    , p2KeyValue key value env
    , [(pCompoundStatement <|> pStmt) body env]
    )
and pStmt_TryStatement' : stmt parser' =
  fun [ _kw; body; catches; finally ] env -> Try
    ( [(cBlock <$> pBlock) body env]
    , couldMap catches env ~f:begin
        single K.CatchClause @@ fun [ _kw; _lp; ty; var; _rp; body ] env ->
            pos_name ty env
          , pos_name var env
          , [(cBlock <$> mpStripNoop pBlock) body env]
      end
    , (single K.FinallyClause
        (fun [ _kw; body ] env -> [(cBlock <$> pBlock) body env])
      <|> ppConst []
      ) finally env
    )
and pStmt_BreakStatement' : stmt parser' =
  fun [ _kw; _level; _semi ] -> cBreak <.| where_am_I
and pStmt_ContinueStatement' : stmt parser' =
  fun [ _kw; _level; _semi ] -> cContinue <.| where_am_I
and pStmt_ReturnStatement' : stmt parser' =
  fun [ _kw; expr; _semi ] -> cReturn <.| positional (mpOptional pExpr) expr
and pStmt_FunctionStaticStatement' : stmt parser' = fun [ _kw; decls; _semi ] ->
  cStatic_var <.| couldMap ~f:pStaticDeclarator decls
and pStmt_UnsetStatement' : stmt parser' = fun [ kw; _lp; vars; _rp; _semi ] env ->
  let (kw_pos, _) as kw = pos_name kw env in
  Expr (where_am_I env, Call ((kw_pos, Id kw), couldMap ~f:pExpr vars env, []))
and pStmt : stmt parser = fun eta -> eta |> mkP
  [ (K.EchoStatement,           pStmt_EchoStatement')
  ; (K.ExpressionStatement,     pStmt_ExpressionStatement')
  ; (K.CompoundStatement,       pStmt_CompoundStatement')
  ; (K.BreakStatement,          pStmt_BreakStatement')
  ; (K.ContinueStatement,       pStmt_ContinueStatement')
  ; (K.ReturnStatement,         pStmt_ReturnStatement')
  ; (K.FunctionStaticStatement, pStmt_FunctionStaticStatement')
  ; (K.ThrowStatement,          pStmt_ThrowStatement')
  ; (K.IfStatement,             pStmt_IfStatement')
  ; (K.DoStatement,             pStmt_DoStatement')
  ; (K.WhileStatement,          pStmt_WhileStatement')
  ; (K.ForStatement,            pStmt_ForStatement')
  ; (K.SwitchStatement,         pStmt_SwitchStatement')
  ; (K.ForeachStatement,        pStmt_ForeachStatement')
  ; (K.TryStatement,            pStmt_TryStatement')
  ; (K.UnsetStatement,          pStmt_UnsetStatement')
  ]

let pTConstraint : (constraint_kind * hint) parser =
  single K.TypeConstraint @@ fun [ kind; hint ] env ->
    mkTP tConstraintKind kind env, pHint hint env

let pTParaml : tparam list parser =
  let pTParam : tparam parser = single K.TypeParameter @@
    fun [ var; name; cstrl ] env ->
      Covariant, pos_name name env, couldMap ~f:pTConstraint cstrl env
  in
  mkP
  [ (K.Missing,        ppConst [])
  ; (K.TypeParameters, mpArgKOfN 1 3 (couldMap ~f:pTParam))
  ]

let pFunHdr :
  (fun_kind * id * tparam list * fun_param list * hint option * fun_param list)
  parser
= mkP
  [ (K.FunctionDeclarationHeader, fun
    [ async; _kw; _amp; name; tparaml; _lparen; paraml; _rparen; _colon; ret ]
    env ->
      let is_constructor = P.text name = "__construct" in
      let paraml = couldMap ~f:(pFunParam is_constructor) paraml env in
      let ret = match mpOptional pHint ret env with
        | Some x -> Some x
        | None ->
            let pos = ppPos ret env in
            match P.text name with
            | "__construct"
            | "__destruct" -> Some (pos, Happly ((pos, "void"), []))
            | _            -> None
      in
      let ifAsync t e = ifExists t e async env in
      (match Option.map ~f:hint_to_fun_kind ret with
        | Some FGenerator -> ifAsync FAsyncGenerator FGenerator
        | _ -> ifAsync FAsync FSync
      )
      , pos_name name env
      , pTParaml tparaml env
      , paraml
      , ret
      , List.filter (fun p -> Option.is_some p.param_modifier) paraml
    )
  ; (K.LambdaSignature, fun [ _lparen; paraml; _rparen; _colon; ret ] env ->
      FSync
      , (Pos.none, "<ANONYMOUS>")
      , [] (* no tparaml on lambdas *)
      , couldMap ~f:(pFunParam false) paraml env
      , mpOptional pHint ret env
      , []
    )
  ; (K.Token, ppConst (FSync, (Pos.none, "<ANONYMOUS>"), [], [], None, []))
  ]





let pDef_Fun' : def parser' = fun [ attrs; hdr; body ] env ->
  let containsUNSAFE node =
    let re = Str.regexp_string "UNSAFE" in
    try Str.search_forward re (P.full_text node) 0 >= 0 with
    | Not_found -> false
  in
  let async, name, tparaml, paraml, ret, _clsvs = pFunHdr hdr env in
  let previous_saw_yield = !(env.saw_yield) in
  env.saw_yield := false;
  let block = mpOptional pBlock body env in
  let body_has_yield = !(env.saw_yield) in
  env.saw_yield := previous_saw_yield;
  Fun
  { f_mode            = env.mode
  ; f_tparams         = tparaml
  ; f_ret             = ret
  ; f_ret_by_ref      = false
  ; f_name            = name
  ; f_params          = paraml
  ; f_body            = begin
      (* FIXME: Filthy hack to catch UNSAFE *)
      match block with
      | Some [Noop] when containsUNSAFE body -> [Unsafe]
      | Some [] -> [Noop]
      | None -> []
      | Some b -> b
    end
  ; f_user_attributes = List.flatten @@ couldMap ~f:pUserAttribute attrs env
  ; f_fun_kind        = if not body_has_yield then async else begin
      match async with
      | FAsync -> FAsyncGenerator
      | FSync -> FGenerator
      | _ -> async
    end
  ; f_namespace       = Namespace_env.empty_with_default_popt
  ; f_span            = where_am_I env
  }






let pConstDeclaration' : (hint option * (id * expr option) list) parser' =
  fun [ abs; _kw; ty; decls; _semi ] env ->
    mpOptional pHint ty env,
    couldMap decls env ~f:begin single K.ConstantDeclarator @@
      fun [name; init] env ->
        pos_name name env,
        ifExists None (mpOptional pSimpleInitializer init env) abs env
    end



let pXHPEnumDecl : (Pos.t * Ast.expr list) parser = positional @@
  single K.XHPEnumType @@ fun [ _kw; _lb; values; _rb ] ->
    couldMap ~f:pExpr values

let pClassElt_ConstDeclaration' : class_elt list parser' = fun ns env ->
  let ty, res = pConstDeclaration' ns env in
  let absts, concrs = List.partition (fun (_, x) -> Option.is_none x) res in
  let flatten : Ast.expr option -> Ast.expr = function
    | Some x -> x
    | None -> raise @@ Parser_fail "The impossible happened"
  in
  let absts = List.map (fun (id, _) -> AbsConst (ty, id)) absts in
  if concrs = [] then absts else
    Const
    ( ty
    , List.map (fun (id, opt_x) -> id, flatten opt_x) concrs
    ) :: absts


let pClassElt_TypeConst' : class_elt parser' =
  fun [ abs; _kw; _tykw; name; constr; _eq; ty; _semi ] env ->
    TypeConst
    { tconst_abstract   = ifExists true false abs env
    ; tconst_name       = pos_name name env
    ; tconst_constraint = mpOptional (snd <$> pTConstraint) constr env
    ; tconst_type       = mpOptional pHint ty env
    ; tconst_span       = where_am_I env
    }
let pClassElt_TraitUse' : class_elt parser' = fun [ _kw; names; _semi ] env ->
  ClassUse (pHint names env)
let pClassElt_RequireClause' : class_elt parser' =
  fun [ _kw; kind; name; _semi ] env ->
    ClassTraitRequire (mkTP tTraitReqKind kind env, pHint name env)
let pClassElt_PropertyDeclaration' : class_elt parser' =
  fun [ mods; ty; decls; _semi ] env ->
    ClassVars
    ( couldMap ~f:(mkTP tKind) mods env
    , mpOptional pHint ty env
    , couldMap decls env ~f:begin single K.PropertyDeclarator @@
        fun [ name; init ] env ->
          let p, n = pos_name name env in
          let n =
            if n.[0] = '$'
            then drop_fst n
            else n
          in
          where_am_I env, (p, n), mpOptional pSimpleInitializer init env
      end
    )
let pClassElt_XHPClassAttributeDeclaration' : class_elt list parser' =
  fun [ _kw; attrs; _semi ] env ->
    couldMap ~f:begin mkP
      [ (K.XHPClassAttribute, fun [ ty; name; init; req ] env ->
          let hint = mpOptional pHint ty env in
          let enum = mpOptional pXHPEnumDecl ty env in
          let (pos, name) = pos_name name env in
          let init = mpOptional pSimpleInitializer init env in
          let req = ifExists true false req env in
          XhpAttr (hint, (Pos.none, (pos, ":" ^ name), init), req, enum)
        )
      ; (K.Token, fun [ tok ] -> pToken ~f:begin function
          | _ -> fun env ->
            XhpAttrUse (where_am_I env, cHapply (pos_name tok env, []))
          end tok
        )
      ] end attrs env
let pClassElt_MethodishDeclaration' : class_elt list parser' =
  let classvar_init : fun_param -> stmt * (kind list * hint option * class_var list) = fun p ->
    let pos, name = p.param_id in
    let cvname = pos, drop_fst name in
    let span = match p.param_expr with
      | Some (pos_end, _) -> Pos.btw pos pos_end
      | None -> pos
    in
    let this = pos, "$this" in
    Expr (pos, Binop (Eq None, (pos, Obj_get((pos, Lvar this),
                                             (pos, Id cvname),
                                             OG_nullthrows)),
                     (pos, Lvar p.param_id))),
    (Option.to_list p.param_modifier, p.param_hint, [span, cvname, None])
  in
  fun [attrs; mods; hdr; body; _semi] env ->
    let async, name, tparaml, paraml, ret, clsvl = pFunHdr hdr env in
    let member_init, member_def = List.split (List.map classvar_init clsvl) in
    let previous_saw_yield = !(env.saw_yield) in
    env.saw_yield := false;
    let body = member_init @ mk_noop @@ stmt_list_of_stmt (pCompoundStatement body env) in
    let body_has_yield = !(env.saw_yield) in
    env.saw_yield := previous_saw_yield;
    List.map (fun (k,h,v) -> ClassVars (k,h,v)) member_def @ [Method
    { m_kind            = couldMap ~f:(mkTP tKind) mods env
    ; m_tparams         = tparaml
    ; m_constrs         = []
    ; m_name            = name
    ; m_params          = paraml
    ; m_body            = mk_noop body
    ; m_user_attributes = List.flatten @@ couldMap ~f:pUserAttribute attrs env
    ; m_ret             = ret
    ; m_ret_by_ref      = false
    ; m_fun_kind        = if not body_has_yield then async else begin
        match async with
        | FSync      -> FGenerator
        | FAsync     -> FAsyncGenerator
        | FGenerator | FAsyncGenerator -> async
      end
    ; m_span            = where_am_I env
    }]
let pClassElt_XHPCategoryDeclaration' : class_elt parser' =
  fun [ _kw; cats; _semi ] env ->
    let stripPercent (p, s) = (p, String.sub s 1 (String.length s - 1)) in
    XhpCategory (List.map stripPercent @@ couldMap ~f:pos_name cats env)

let pClassElt : class_elt list parser = mkP
  [ (K.ConstDeclaration,             pClassElt_ConstDeclaration')
  ; (K.TypeConstDeclaration,         mpSingleton pClassElt_TypeConst')
  ; (K.TraitUse,                     mpSingleton pClassElt_TraitUse')
  ; (K.RequireClause,                mpSingleton pClassElt_RequireClause')
  ; (K.PropertyDeclaration,          mpSingleton pClassElt_PropertyDeclaration')
  ; (K.MethodishDeclaration,         pClassElt_MethodishDeclaration')
  ; (K.XHPClassAttributeDeclaration, pClassElt_XHPClassAttributeDeclaration')
  ; (K.XHPCategoryDeclaration,       mpSingleton pClassElt_XHPCategoryDeclaration')
  ]

(*****************************************************************************(
 * Parsing definitions (AST's `def`)
)*****************************************************************************)
let pDef_Class' : def parser' =
  let pClassishBody = single K.ClassishBody @@ fun [ _lb; elts; _rb ] env ->
    List.concat (couldMap ~f:pClassElt elts env)
  in
  let p2ClassKind : class_kind parser2 = fun kind abs env ->
    let kind = mkTP tClassKind kind env in
    let regex = Str.regexp ".*abstract.*" in
    if kind = Cnormal && Str.string_match regex (P.text abs) 0
    then Cabstract
    else kind
  in
  fun [ attr; mods; kw; name; tparaml; _ext; exts; _impl; impls; body ] env ->
    Class
    { c_mode            = env.mode
    ; c_user_attributes = List.flatten @@ couldMap ~f:pUserAttribute attr env
    ; c_final           = List.mem Final @@  couldMap ~f:(mkTP tKind) mods env
    ; c_is_xhp          = mkTP tIsXHP name env
    ; c_name            = pos_name name env
    ; c_tparams         = pTParaml tparaml env
    ; c_extends         = couldMap ~f:pHint exts env
    ; c_implements      = couldMap ~f:pHint impls env
    ; c_body            = pClassishBody body env
    ; c_namespace       = Namespace_env.empty_with_default_popt
    ; c_enum            = None
    ; c_span            = where_am_I env
    (* The ugly duckling; What is one annotation (of type `class_kind`) in AST,
     * depends on /two/ branches in the FFP tree.
     *)
    ; c_kind            = p2ClassKind kw mods env
    }

let pDef_Typedef' : def parser' =
  fun [ attr; kw; name; tparams; constr; eq; hint; _semi ] env ->
    Typedef
    { t_id              = pos_name name env
    ; t_tparams         = pTParaml tparams env
    ; t_constraint      = Option.map ~f:snd (mpOptional pTConstraint constr env)
    ; t_kind            = mkTP tTypedefKind kw env (pHint hint env)
    ; t_user_attributes = List.flatten @@ couldMap ~f:pUserAttribute attr env
    ; t_namespace       = Namespace_env.empty_with_default_popt
    ; t_mode            = env.mode
    }

let pDef_Enum' : def parser' =
  fun [ _kw; name; _colon; base; ty; _lb; enums; _rb ] env ->
    Class
    { c_mode            = env.mode
    ; c_user_attributes = []
    ; c_final           = false
    ; c_kind            = Cenum
    ; c_is_xhp          = false
    ; c_name            = pos_name name env
    ; c_tparams         = []
    ; c_extends         = []
    ; c_implements      = []
    ; c_body            = couldMap enums env ~f:begin
        single K.Enumerator @@ fun [ name; _eq; value; _semi ] env ->
          Const (None, [pos_name name env, pExpr value env])
      end
    ; c_namespace       = Namespace_env.empty_with_default_popt
    ; c_enum            = Some { e_base = pHint base env; e_constraint = None }
    ; c_span            = where_am_I env
    }

let pDef_InclusionDirective' : def parser' = fun [ expr; _semi ] env ->
  Stmt (Expr (single K.InclusionExpression pExpr_InclusionExpression' expr env))

let pDef_NamespaceUseDeclaration' : def parser' =
  fun [ _kw; kind; clauses; _semi ] env ->
    let pKind n e = mkP
      [ (K.Missing, fun _ _ -> NSClass)
      ; (K.Token,   fun [n] -> mkTP ("nsusekind", function
         | TK.Function -> NSFun
         | TK.Const    -> NSConst
      ) n)] n e
    in
    let f = single K.NamespaceUseClause @@
        fun [ _kind; name; _as; alias ] env ->
          let (p, n) as name = pos_name name env in
          let x = Str.search_forward (Str.regexp "[^\\\\]*$") n 0 in
          let key = p, String.sub n x (String.length n - x) in
          ( pKind kind env
          , (if x = 0 || n.[0] = '\\' then name else (p, "\\" ^ n))
          , ifExists (pos_name alias env) key alias env
          )
    in
    let uses = couldMap ~f clauses env in
    Printf.eprintf "\n\n\nResult:\n  %s\n\n-\n" @@
      Sexp.to_string_hum @@
        Conv.sexp_of_list (fun (x,_,_) -> Sof.sexp_of_ns_kind x) uses;
    NamespaceUse uses

let pDef_NamespaceGroupUseDeclaration' : def parser' =
  fun [ _kw; kind; pfx; _lb; clauses; _rb; _semi ] env ->
    NamespaceUse []




let rec pDef_NamespaceDeclaration' : def parser' =
  fun [ _kw; name; body ] env ->
    let body = mkP
      [ (K.NamespaceBody, fun [ _lb; decls; _rb ] ->
          couldMap ~f:(oneOf [cStmt <$> pStmt; pDef]) decls
        )
      ; (K.Token,         fun [ _ ] _ -> [])
      ] body env in
    Namespace ((ppPos name env, P.text name), body)
and pDef : def parser = fun node -> mkP
  [ (K.FunctionDeclaration,          pDef_Fun')
  ; (K.ClassishDeclaration,          pDef_Class')
  ; (K.AliasDeclaration,             pDef_Typedef')
  ; (K.EnumDeclaration,              pDef_Enum')
  ; (K.InclusionDirective,           pDef_InclusionDirective')
  ; (K.NamespaceDeclaration,         pDef_NamespaceDeclaration')
  ; (K.NamespaceUseDeclaration,      pDef_NamespaceUseDeclaration')
  ; (K.NamespaceGroupUseDeclaration, pDef_NamespaceGroupUseDeclaration')
  (* The ugly duckling; Here, the FFP tree has one /fewer/ level of hierarchy,
   * so we have to "step back" and look at the node given to mkP.
   *)
  ; (K.ExpressionStatement,          fun _ env -> Stmt (pStmt node env))
  ] node
and pProgram : program parser = fun node env ->
  let rec post_process program =
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
    | [] -> []
    | (Namespace (n, [])::el) -> begin
      let body, remainder = span not_namespace el in
      Namespace (n, body) :: post_process remainder
    end
    | (Namespace (n, il)::el) ->
      Namespace (n, post_process il) :: post_process el
    | (Stmt Noop::el) -> post_process el
(*    | (Stmt (Expr (_, Import (RequireOnce, _)))::el) -> post_process el *)
    | ((Stmt (Expr (_, (Call
        ( (_, (Id (_, "define")))
        , [ (_, (String name))
          ; value
          ]
        , []
        )
      )))) :: el) -> Constant
        { cst_mode      = env.mode
        ; cst_kind      = Cst_define
        ; cst_name      = name
        ; cst_type      = None
        ; cst_value     = value
        ; cst_namespace = Namespace_env.empty_with_default_popt
        } :: post_process el
    | (e::el) -> e :: post_process el
  in
  Namespaces.elaborate_defs ParserOptions.default @@ post_process @@
    couldMap ~f:(oneOf [cStmt <$> pStmt; pDef]) node env




let from_tree : Relative_path.t -> P.SyntaxTree.t -> Ast.program =
  fun file minimal_tree ->
    let module ST = Full_fidelity_syntax_tree in
    Printexc.record_backtrace true;
    let script = P.from_tree minimal_tree in
    runP (single K.Script @@ fun [ _hdr; prog ] -> pProgram prog) script
      { language  = ST.language minimal_tree
      ; filePath  = Relative_path.suffix file
      ; ancestry  = []
      ; saw_yield = ref false
      ; mode      = match ST.mode minimal_tree with
                     | _ when ST.is_php minimal_tree -> FileInfo.Mdecl
                     | "strict"  -> FileInfo.Mstrict
                     | "decl"    -> FileInfo.Mdecl
                     | "partial" -> FileInfo.Mpartial
                     | "" -> FileInfo.Mpartial
                     | s ->
                         Printf.eprintf "Unknown hh mode '%s'\n" s;
                         raise (Failure s)
      }
