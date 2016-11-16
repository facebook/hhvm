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


(* For debugging *)
let treeDBG =
  let module Token = Full_fidelity_positioned_token in
  let rec go indent node = begin match P.syntax node with
    | P.Token t -> Printf.eprintf "> %sToken: '%s'\n" indent (Token.full_text t)
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
let dbg_ ?msg:(s="") n = Full_fidelity_positioned_token.(
  Printf.eprintf "Debugging %s '%s':<%s>\n" s (text n) (kind n |> TK.to_string)
)




let drop_fst n = String.sub n 1 (String.length n - 1)



type env = {
  (* Context of the file being parsed. *)
  mode     : FileInfo.mode;
  language : string;
  filePath : string;
  (* "Local" context. *)
  ancestry : P.t list;
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
  | K.Missing -> Printf.eprintf "MISSING\n"
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
let cStmt        x  = Stmt         x
let cExpr        x  = Expr         x
let cId          x  = Id           x
let cDo      (b, c) = Do       (b, c)
let cWhile   (c, b) = While    (c, b)
let cLvar        x  = Lvar         x
let cSome        x  = Some         x
let cHapply  (p, x) = Happly   (p, x)
let cHoption     x  = Hoption      x
let cHtuple      x  = Htuple       x
let cHshape      x  = Hshape       x
let cBlock       x  = Block        x
let cBreak       x  = Break        x
let cThrow       x  = Throw        x
let cContinue    x  = Continue     x
let cReturn  (p, x) = Return   (p, x)
let cExpr_list   x  = Expr_list    x
let cStatic_var  x  = Static_var   x



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
    (* OMIT | Missing             -> [] *)
    | SyntaxList        l -> List.flatten @@
        List.map (fun n -> go ~f n (ppDescend node env)) l
    | ListItem          i -> [f i.list_item @@ ppDescend node env]
    (* OMIT | CompoundStatement c -> go ~f c.compound_statements (ppDescend node env) *)
    | _ -> [f node env]
  )
  and go : 'a . f:'a parser -> 'a list parser = fun ~f -> function
    | node when P.is_missing node -> const []
    | node -> synmap f node
  in
  go ~f node env



let pToken : f:('a tokenparser) -> 'a parser = fun ~f ->
  let module Token = Full_fidelity_positioned_token in
  fun node env -> match P.syntax node with
    | P.Token t -> f (Token.text t) env
    | _ -> fail [K.Token] node env





(**
 * Terminal parsers
 * ----------------
 *
 * Constructed using mkTP, name-schema is t<result_type>.
 *)

type 'a terminal_spec = string * (TK.t -> 'a)

let mkTP : 'a terminal_spec -> 'a parser = fun (name, spec) ->
  let module Token = Full_fidelity_positioned_token in
  fun node env -> (* parser *)
    try (match P.syntax node with | P.Token t -> spec (Token.kind t)) with
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

(* TODO: Clean up string escaping *)
let mkString : string -> string = fun content ->
  let no_quotes = String.sub content 1 (String.length content - 2) in
  let unescaped = begin
    try Php_escaping.unescape_double no_quotes with
    | Php_escaping.Invalid_string error -> raise @@
        Failure (Printf.sprintf "Malformed string literal <<%s>>" no_quotes)
  end in
  (* Printf.eprintf "\n\nno_quotes:%s\nunescaped:%s\n\n" no_quotes unescaped; *)
  unescaped
let tLiteral : (Pos.t -> string -> expr_) terminal_spec =
  "Literal", fun token_kind -> fun p text -> match token_kind with
  | TK.DecimalLiteral
  | TK.OctalLiteral
  | TK.HexadecimalLiteral
  | TK.BinaryLiteral             -> Int   (p, text)
  | TK.FloatingLiteral           -> Float (p, text)
  | TK.SingleQuotedStringLiteral
  | TK.DoubleQuotedStringLiteral
  | TK.HeredocStringLiteral
  | TK.NowdocStringLiteral       -> String (p, mkString text)
  | TK.NullLiteral               -> Null
  | TK.BooleanLiteral            ->
      (match text with "false" -> False | "true" -> True)

(* END OF TOKEN PARSERS *)


let pAwait : Pos.t parser =
  fst <$> positional (mkTP ("await", function TK.Await -> ()))

let pUserAttribute : user_attribute parser =
  single K.AttributeSpecification @@ mpArgKOfN 1 3 begin
    List.hd <$> couldMap ~f:begin
      single K.Attribute @@ fun [ name; _lp; vals; _rp ] env ->
        { ua_name = pos_name name env; ua_params = [] }
    end
  end

let pTParaml : tparam list parser =
  let pTParam : tparam parser = single K.TypeParameter @@
    fun [ var; name; cstrl ] env -> Covariant, pos_name name env, []
  in
  mkP
  [ (K.Missing,        ppConst [])
  ; (K.TypeParameters, mpArgKOfN 1 3 (couldMap ~f:pTParam))
  ]


let pExpr_LiteralExpression' : expr_ parser' = fun [ lit ] env ->
  mkTP tLiteral lit env (where_am_I env) (P.text lit)



let mpShapeField : ('a, (shape_field_name * 'a)) metaparser = fun pThing ->
  fun node env ->
    let p2NamedThing = fun name thing env ->
      pDbgl "p2NamedThing" [ "name"; "thing" ] [ name; thing ] env;
      let p, n = pos_name name env in
      SFlit (p, mkString n), pThing thing env
    in
    pDbg "ShapeField" "node" node env;
    mkP
    [ (K.FieldSpecifier,   fun [ name; _arr; thing ] -> p2NamedThing name thing)
    ; (K.FieldInitializer, fun [ name; _arr; thing ] -> p2NamedThing name thing)
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
    ; (K.VectorTypeSpecifier,    mpArgKOfN 2 4 @@ fun n env ->
        Happly ((where_am_I env, "array"), couldMap ~f:pHint n env))
    ; (K.MapTypeSpecifier,       fun [ kw; _la; key; _comma; value; _ra ] env ->
        cHapply (pos_name kw env, List.map (fun x -> pHint x env) [key; value]))
    ; (K.ClassnameTypeSpecifier, fun [ name; _la; ty; _ra ] ->
        cHapply <.| (pos_name name <&> mpSingleton pHint ty))
    ; (K.TypeConstant,           fun [ base; _sep; child ] ->
        cHapply <.| (pos_name base <&> mpSingleton pHint child))
    ; (K.GenericTypeSpecifier,   fun [class_type; arguments] ->
        cHapply <.| (pos_name class_type <&> pTypeArguments arguments))
    ; (K.NullableTypeSpecifier,  fun [ question; ty ] env ->
        ifExists cHoption snd question env (pHint ty env))
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
and pSimpleInitializer : expr parser = fun eta -> single
    K.SimpleInitializer
    (fun [ _eq; value ] -> pExpr value)
    eta
and pStaticDeclarator = fun eta -> single
    K.StaticDeclarator
    (fun [ name; init ] env ->
      let lhs = pExpr name env in
      match mpOptional pSimpleInitializer init env with
        | None -> lhs
        | Some rhs -> where_am_I env, Binop (Eq None, lhs, rhs)
    )
    eta
and pFunParam : bool -> fun_param parser = fun is_constructor node -> mkP
  [ (K.ParameterDeclaration, fun [ attrs; vis; ty; name; def ] env ->
      { param_hint            = mpOptional pHint ty env
      ; param_is_reference    = false
      ; param_is_variadic     = false
      ; param_id              = pos_name name env
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
      ; param_user_attributes = couldMap ~f:pUserAttribute attrs env
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
  ] node
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
  let paraml, ret = pLambdaSig signature env in
  { f_mode            = env.mode
  ; f_tparams         = []
  ; f_ret             = ret
  ; f_ret_by_ref      = false
  ; f_name            = p, ";anonymous"
  ; f_params          = paraml
  ; f_body            = pLambdaBody body env
  ; f_user_attributes = []
  ; f_fun_kind        = FSync
  ; f_namespace       = Namespace_env.empty_with_default_popt (*TODO*)
  ; f_span            = p
  }
and pAField : afield parser = fun node env ->
  let pElemInit = single K.ElementInitializer @@ fun [ key; _arr; value ] env ->
    AFkvalue (pExpr key env, pExpr value env)
  in
  match mpOptional pElemInit node env with
  | None   -> AFvalue (pExpr node env)
  | Some x -> x
and pExpr_InclusionExpression' : expr parser' = fun [ req; file ] env ->
  let flavor = mkTP tImportFlavor req env in
  where_am_I env, Import (flavor, pExpr file env)
and pExpr_ParenthesizedExpression' : expr parser' = fun [ _lp; expr; _rp ] ->
  pExpr expr
and pExpr_ScopeResolutionExpression' : expr_ parser' =
  fun [ qual; _op; name ] env ->
    Class_const (pos_name qual env, pos_name name env)
and pExpr : expr parser = fun eta -> eta |>
  positional @@ mkP
  [ (K.ParenthesizedExpression,     fun [ _lp; expr; _rp ] ->
      (snd <$> pExpr) expr)
  ; (K.InclusionExpression,         snd <$> pExpr_InclusionExpression')
  ; (K.ArrayIntrinsicExpression,    fun [ _kw; _lp; members; _rp ] env ->
      Array (couldMap ~f:pAField members env))
  ; (K.ShapeExpression,             fun [ _kw; _lp; fields; _rp ] env ->
      Shape (couldMap ~f:(mpShapeField pExpr) fields env))
  ; (K.CollectionLiteralExpression, fun [ name; _lb; initl; _rb ] env ->
      Collection (pos_name name env, couldMap ~f:pAField initl env))
  ; (K.DecoratedExpression,         fun [ _dec; expr ] -> (snd <$> pExpr) expr)
  ; (K.Token,                       fun [ t ] env -> Id (pos_name t env))
  ; (K.QualifiedNameExpression,     fun [ n ] env -> Id (pos_name n env))
  ; (K.VariableExpression,          fun [ v ] env -> Lvar (pos_name v env))
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
  ; (K.LiteralExpression, pExpr_LiteralExpression')
  ; (K.PrintExpression, fun [ _kw; expr ] env ->
      let pos = where_am_I env in
      let args = couldMap ~f:pExpr expr env in
      Call ((pos, Id (pos, "echo")), args, [])
    )
  ; (K.YieldExpression, fun [ _kw; arg ] env -> Yield (pAField arg env))
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
      InstanceOf (pExpr thing env, pExpr ty env)
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
        let fun_ =
          { f_mode            = env.mode
          ; f_tparams         = []
          ; f_ret             = mpOptional pHint ret env
          ; f_ret_by_ref      = false
          ; f_name            = where_am_I env, ";anonymous"
          ; f_params          = couldMap ~f:(pFunParam false) params env
          ; f_body            = pBlock body env
          ; f_user_attributes = []
          ; f_fun_kind        = ifExists FAsync FSync async env
          ; f_namespace       = Namespace_env.empty_with_default_popt
          ; f_span            = where_am_I env
          } in
        Efun (fun_, (pUse <|> ppConst []) use env)
    )
  ; (K.XHPExpression, fun nodel env -> List.iter dbg nodel; Dollardollar)
  ; (K.LambdaExpression, fun node env -> Lfun (pLambda node env))
  ; (K.AwaitableCreationExpression, fun [ _async; blk ] env ->
      let body =
        { f_mode            = env.mode
        ; f_tparams         = []
        ; f_ret             = None
        ; f_ret_by_ref      = false
        ; f_name            = where_am_I env, ";anonymous"
        ; f_params          = []
        ; f_body            = pBlock blk env
        ; f_user_attributes = []
        ; f_fun_kind        = FAsync
        ; f_namespace       = Namespace_env.empty_with_default_popt
        ; f_span            = where_am_I env
        }
      in
      Call ((where_am_I env, Lfun body), [], [])
    )
  (* FIXME; should this include Missing? ; (K.Missing, ppConst Null)*)
  ]


and pBlock : block parser = fun node -> pStmt node |.> function
  | Block [] -> [Noop]
  | Block block -> block
  | stmt -> [stmt]
and pSwitch : case list parser = fun node env ->
  let pCaseLine = oneOf [ (fun x -> `Stmt x) <$> pStmt; (fun x -> `Func x) <$>
    mkP
    [ (K.DefaultStatement, fun [ _kw; _col; stmt ]       env -> fun x ->
        Default (match mpOptional pStmt stmt env with
          | None -> x
          | Some s -> s :: x
        )
      )
    ; (K.CaseStatement,    fun [ _kw; expr; _col; stmt ] env -> fun x ->
        Case (pExpr expr env, match mpOptional pStmt stmt env with
          | None -> x
          | Some s -> s :: x
        )
      )
    ]
  ] in
  let merge thing acc = match thing, acc with
    | `Func f, (stmtl, casel) -> ([], f stmtl :: casel)
    | `Stmt x, (stmtl, casel) -> x::stmtl, casel
  in
  snd @@ List.fold_right merge (couldMap ~f:pCaseLine node env) ([],[])

and pStmt_EchoStatement' : stmt parser' = fun [ kw; exprs; _semi ] ->
  let pEcho : env -> expr_ = fun env ->
    Call (positional (cId <$> pos_name) kw env, couldMap ~f:pExpr exprs env, [])
  in
  cExpr <.| (where_am_I <&> pEcho)
and pStmt_ExpressionStatement' : stmt parser' = fun [ expr; _semi ] env ->
  Expr (pExpr expr env)
and pStmt_CompoundStatement' : stmt parser' = fun [ _lb; stmts; _rb ] env ->
  Block (couldMap ~f:pStmt stmts env)
and pCompoundStatement : stmt parser = fun node env -> 
  ifExists (single K.CompoundStatement pStmt_CompoundStatement' node env) Noop
    node env
and pStmt_ThrowStatement' : stmt parser' = fun [ _kw; expr; _semi ] env ->
  Throw (pExpr expr env)
and pStmt_IfStatement' : stmt parser' =
  fun [ _kw; _lparen; cond; _rparen; then_; elseif; else_ ] env ->
    let pElseClause = single K.ElseClause @@ mpArgKOfN 1 2 pCompoundStatement in
    If
    ( pExpr cond env
    , [pCompoundStatement then_ env]
    , [(pElseClause <|> ppConst Noop) else_ env]
    )
and pStmt_DoStatement' : stmt parser' =
  fun [ _kw_do; body; _kw_while; _lp; cond; _rp; _semi ] env ->
    Do (pBlock body env, pExpr cond env)
and pStmt_WhileStatement' : stmt parser' =
  fun [ _kw; _lp; cond; _rp; body ] env ->
    (* While (pExpr cond env, pBlock body env) *)
    Unsafe

and pStmt_ForStatement' : stmt parser' =
  fun [ _kw; _lp; init; _semi; ctrl; _semi'; eol; _rp; body ] env ->
    let pExprL = positional (cExpr_list <$> couldMap ~f:pExpr) in
      For (pExprL init env, pExprL eol env, pExprL ctrl env, pBlock body env)
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
          pos_name ty env, pos_name var env, pBlock body env
      end
    , (single K.FinallyClause (fun [ _kw; body ] -> pBlock body) <|> ppConst [])
        finally env
    )
and pStmt_BreakStatement' : stmt parser' =
  fun [ _kw; _level; _semi ] -> cBreak <.| where_am_I
and pStmt_ContinueStatement' : stmt parser' =
  fun [ _kw; _level; _semi ] -> cContinue <.| where_am_I
and pStmt_ReturnStatement' : stmt parser' =
  fun [ _kw; expr; _semi ] -> cReturn <.| positional (mpOptional pExpr) expr
and pStmt_FunctionStaticStatement' : stmt parser' = fun [ _kw; decls; _semi ] ->
  cStatic_var <.| couldMap ~f:pStaticDeclarator decls
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
  ]



let pFunHdr :
  (fun_kind * id * tparam list * fun_param list * hint option * fun_param list)
  parser
= mkP
  [ (K.FunctionDeclarationHeader, fun
    [ async; _kw; _amp; name; tparaml; _lparen; paraml; _rparen; _colon; ret ] env ->
      let is_constructor = P.text name = "__construct" in
      let paraml = couldMap ~f:(pFunParam is_constructor) paraml env in
      ifExists FAsync FSync async env
      , pos_name name env
      , pTParaml tparaml env
      , paraml
      , begin match mpOptional pHint ret env with
        | Some x -> Some x
        | None ->
            let pos = ppPos ret env in
            match P.text name with
            | "__construct"
            | "__destruct" -> Some (pos, Happly ((pos, "void"), []))
            | _            -> None
        end
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





let pDef_Fun' : def parser' = fun [ attr; hdr; body ] env ->
  let containsUNSAFE node =
    let re = Str.regexp_string "UNSAFE" in
    try Str.search_forward re (P.full_text node) 0 >= 0 with
    | Not_found -> false
  in
  let async, name, tparaml, paraml, ret, clsvs = pFunHdr hdr env in
  Fun
  { f_mode            = env.mode
  ; f_tparams         = tparaml
  ; f_ret             = ret
  ; f_ret_by_ref      = false
  ; f_name            = name
  ; f_params          = paraml
  ; f_body            = begin
      (* FIXME: Filthy hack to catch UNSAFE *)
      match mpOptional pBlock body env with
      | Some [Noop] when containsUNSAFE body -> [Unsafe]
      | None -> []
      | Some b -> b
    end
  ; f_user_attributes = []
  ; f_fun_kind        = async
  ; f_namespace       = Namespace_env.empty_with_default_popt (*TODO*)
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



let pTConstraint : (constraint_kind * hint) parser =
  single K.TypeConstraint @@ fun [ kind; hint ] env ->
    mkTP tConstraintKind kind env, pHint hint env




let pClassElt_ConstDeclaration' : class_elt parser' = fun ns env ->
  let ty, res = pConstDeclaration' ns env in
  match List.hd res with
  | id, None      -> AbsConst (ty,  id       )
  | id, Some expr -> Const    (ty, [id, expr])
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
          where_am_I env, (p, n), mpOptional pExpr init env
      end
    )
let pClassElt_XHPClassAttributeDeclaration' : class_elt parser' =
  fun [ _kw; attrs; _semi ] env ->
    let attrl = couldMap ~f:begin mkP
      [ (K.XHPClassAttribute, fun [ ty; name; init; req ] env ->
          XhpAttrUse (pHint ty env)
        )
      ; (K.Token, fun [ tok ] -> pToken ~f:begin function
          | _ -> fun env ->
            XhpAttrUse (where_am_I env, cHapply (pos_name tok env, []))
          end tok
        )
      ] end attrs env
    in
    List.hd attrl
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
  fun [attr; mods; hdr; body; _semi] env ->
    let async, name, tparaml, paraml, ret, clsvl = pFunHdr hdr env in
    let member_init, member_def = List.split (List.map classvar_init clsvl) in
    let body = member_init @ match pCompoundStatement body env with
      (* Filthy, filthy hack to match inconsistend parser_hack behaviour *)
      | Block [] -> [Noop]
      | Block stmtl -> stmtl (* Unwrap statement blocks *)
      | stmt -> [stmt]
    in
    List.map (fun (k,h,v) -> ClassVars (k,h,v)) member_def @ [Method
    { m_kind            = couldMap ~f:(mkTP tKind) mods env
    ; m_tparams         = tparaml
    ; m_constrs         = []
    ; m_name            = name
    ; m_params          = paraml
    ; m_body            = body
    ; m_user_attributes = []
    ; m_ret             = ret
    ; m_ret_by_ref      = false
    ; m_fun_kind        = async
    ; m_span            = where_am_I env
    }]
let pClassElt_XHPCategoryDeclaration' : class_elt parser' =
  fun [ _kw; cats; _semi ] env ->
    XhpCategory (couldMap ~f:pos_name cats env)

let pClassElt : class_elt list parser = mkP
  [ (K.ConstDeclaration,             mpSingleton pClassElt_ConstDeclaration')
  ; (K.TypeConstDeclaration,         mpSingleton pClassElt_TypeConst')
  ; (K.TraitUse,                     mpSingleton pClassElt_TraitUse')
  ; (K.RequireClause,                mpSingleton pClassElt_RequireClause')
  ; (K.PropertyDeclaration,          mpSingleton pClassElt_PropertyDeclaration')
  ; (K.XHPClassAttributeDeclaration, mpSingleton pClassElt_XHPClassAttributeDeclaration')
  ; (K.MethodishDeclaration,         pClassElt_MethodishDeclaration')
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
    match mkTP tClassKind kind env with
    | Cnormal -> ifExists Cabstract Cnormal abs env
    | k -> k
  in
  fun [ attr; mods; kw; name; tparaml; _ext; exts; _impl; impls; body ] env ->
    Class
    { c_mode            = env.mode
    ; c_user_attributes = couldMap ~f:pUserAttribute attr env
    ; c_final           = List.mem Final @@  couldMap ~f:(mkTP tKind) mods env
    ; c_is_xhp          = begin
        let is_xhp = mkTP tIsXHP name env in
        (* pDbg "def" "name" name env;
        Printf.eprintf "IS_XHP: %B\n" is_xhp; *)
        is_xhp
      end
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
    ; t_user_attributes = couldMap ~f:pUserAttribute attr env
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

let pDef_ConstDeclaration' : def parser' = fun ps env ->
  let h, ds = pConstDeclaration' ps env in
  Constant
  { cst_mode      = env.mode
  ; cst_kind      = Cst_const
  ; cst_name      = Pos.none, ""
  ; cst_type      = h
  ; cst_value     = Pos.none, False
  ; cst_namespace = Namespace_env.empty_with_default_popt
  }

let pDef_NamespaceUseDeclaration' : def parser' =
  fun [ _kw; kind; clauses; _semi ] env ->
    NamespaceUse []

let pDef_NamespaceGroupUseDeclaration' : def parser' =
  fun [ _kw; kind; pfx; _lb; clauses; _rb; _semi ] env ->
    NamespaceUse []




let rec pDef_NamespaceDeclaration' : def parser' =
  fun [ _kw; name; body ] env ->
    let pBody = mkP
      [ (K.NamespaceBody, fun [ _lb; decls; _rb ] -> pProgram decls)
      ; (K.Token,         fun [ _ ] -> const [])
      ] in
    Namespace (pos_name name env, pBody body env)
and pDef : def parser = fun node -> mkP
  [ (K.FunctionDeclaration,          pDef_Fun')
  ; (K.ClassishDeclaration,          pDef_Class')
  ; (K.AliasDeclaration,             pDef_Typedef')
  ; (K.EnumDeclaration,              pDef_Enum')
  ; (K.InclusionDirective,           pDef_InclusionDirective')
  ; (K.ConstDeclaration,             pDef_ConstDeclaration')
  ; (K.NamespaceDeclaration,         pDef_NamespaceDeclaration')
  ; (K.NamespaceUseDeclaration,      pDef_NamespaceUseDeclaration')
  ; (K.NamespaceGroupUseDeclaration, pDef_NamespaceGroupUseDeclaration')
  (* The ugly duckling; Here, the FFP tree has one /fewer/ level of hierarchy,
   * so we have to "step back" and look at the node given to mkP.
   *)
  ; (K.ExpressionStatement,          fun _ env -> Stmt (pStmt node env))
  ] node
and pProgram : program parser = fun eta -> eta |>
  couldMap ~f:(oneOf [cStmt <$> pStmt; pDef])



let pScript : program parser =
  Printexc.record_backtrace true;
  single K.Script @@ fun [ _hdr; prog ] -> pProgram prog

let from_tree : Relative_path.t -> P.SyntaxTree.t -> Ast.program =
  fun file minimal_tree ->
    let module ST = Full_fidelity_syntax_tree in
    let env =
      { language = ST.language minimal_tree
      ; ancestry = []
      ; filePath = Relative_path.suffix file
      ; mode     = match ST.mode minimal_tree with
                     | "strict"  -> FileInfo.Mstrict
                     | "decl"    -> FileInfo.Mdecl
                     | "partial" -> FileInfo.Mpartial
                     | "" -> FileInfo.Mpartial
                     | s ->
                         Printf.eprintf "Unknown hh mode '%s'\n" s;
                         raise (Failure s)
      } in
    runP pScript (P.from_tree minimal_tree) env
