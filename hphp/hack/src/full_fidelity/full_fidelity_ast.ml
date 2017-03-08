(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

 (* What we're lowering from *)
 open Full_fidelity_positioned_syntax
 type node = Full_fidelity_positioned_syntax.t (* Let's be more explicit *)
 (* What we're lowering to *)
 open Ast

module SyntaxKind = Full_fidelity_syntax_kind
module TK = Full_fidelity_token_kind
module PT = Full_fidelity_positioned_token



let drop_pstr : int -> pstring -> pstring = fun cnt (pos, str) ->
  let len = String.length str in
  pos, if cnt >= len then "" else String.sub str cnt (len - cnt)

(* Context of the file being parsed, as global state. *)
type state_variables =
  { language : string ref
  ; filePath : string ref
  ; mode     : FileInfo.mode ref
  }

let lowerer_state =
  { language = ref "UNINITIALIZED"
  ; filePath = ref "UNINITIALIZED"
  ; mode     = ref FileInfo.Mstrict
  }

(* "Local" context. *)
type env =
  { saw_yield : bool ref
  ; errors    : (Pos.t * string) list ref
  }

type +'a parser = node -> env -> 'a
type ('a, 'b) metaparser = 'a parser -> 'b parser

let get_pos : node -> Pos.t = fun node ->
  let text = source_text node in
  let mk_pos offset =
    let line, column =
      Full_fidelity_source_text.offset_to_position text offset
    in
    File_pos.of_line_column_offset ~line ~column ~offset
  in
  Pos.make_from_file_pos
    ~pos_file:Relative_path.(create Dummy !(lowerer_state.filePath))
    ~pos_start:(start_offset node |> mk_pos)
    ~pos_end:(end_offset node |> mk_pos)

exception Lowerer_invariant_failure of string * string
let invariant_failure where what =
  raise (Lowerer_invariant_failure (where, what))

exception API_Missing_syntax of string * env * node
let missing_syntax : string -> node -> env -> 'a = fun s n env ->
  raise (API_Missing_syntax (s, env, n))
let handle_missing_syntax : string -> node -> env -> 'a = fun s n _ ->
  let pos = Pos.string (Pos.to_absolute (get_pos n)) in
  let msg = Printf.sprintf
    "Missing case in %s.
 - Pos: %s
 - Unexpected: '%s'
 - Kind: %s
 "
  s
  pos
  (text n)
  (SyntaxKind.to_string (kind n))
  in
  Printf.eprintf "EXCEPTION\n---------\n%s\n" msg;
  raise (Failure msg)

let runP : 'a parser -> node -> env -> 'a = fun pThing thing env ->
  try pThing thing env with
  | API_Missing_syntax (ex, env, n) -> handle_missing_syntax ex n env
  | e -> raise e


(* TODO: Cleanup this hopeless Noop mess *)
let mk_noop : stmt list -> stmt list = function
  | [] -> [Noop]
  | s -> s
let mpStripNoop pThing node env = match pThing node env with
  | [Noop] -> []
  | stmtl -> stmtl

let mpOptional : ('a, 'a option) metaparser = fun p -> fun node env ->
  Option.try_with (fun () -> p node env)
let mpYielding : ('a, ('a * bool)) metaparser = fun p node env ->
  let local_ptr = ref false in
  let result = p node { env with saw_yield = local_ptr } in
  result, !local_ptr




let pos_name node = get_pos node, text node

let couldMap : 'a . f:'a parser -> 'a list parser = fun ~f -> fun node env ->
  let rec synmap : 'a . 'a parser -> 'a list parser = fun f node env ->
    match syntax node with
    | SyntaxList        l -> List.flatten @@
        List.map (fun n -> go ~f n env) l
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
    List.map strip_list_item synl
  | { syntax = SyntaxList synl; _ } -> synl
  | { syntax = Missing; _ } -> []
  | syn -> [syn]

let missing =
  let module V = Full_fidelity_positioned_syntax.PositionedSyntaxValue in
  let open Full_fidelity_source_text in
  { syntax = Missing
  ; value =
    { V.source_text    = { text = ""; offset_map = [] }
    ; V.offset         = 0
    ; V.leading_width  = 0
    ; V.width          = 0
    ; V.trailing_width = 0
    }
  }



let token_kind : node -> TK.t option = function
  | { syntax = Token t; _ } -> Some (PT.kind t)
  | _ -> None

(**
 * FFP does not destinguish between ++$i and $i++ on the level of token kind
 * annotation. Prevent duplication by switching on `postfix` for the two
 * operatores for which AST /does/ differentiate between fixities.
 *)
let pUop : bool -> (expr -> expr_) parser = fun postfix node env expr ->
  match token_kind node with
  | Some TK.PlusPlus   when postfix -> Unop (Upincr, expr)
  | Some TK.MinusMinus when postfix -> Unop (Updecr, expr)
  | Some TK.PlusPlus                -> Unop (Uincr,  expr)
  | Some TK.MinusMinus              -> Unop (Udecr,  expr)
  | Some TK.Exclamation             -> Unop (Unot,   expr)
  | Some TK.Tilde                   -> Unop (Utild,  expr)
  | Some TK.Plus                    -> Unop (Uplus,  expr)
  | Some TK.Minus                   -> Unop (Uminus, expr)
  | Some TK.Ampersand               -> Unop (Uref,   expr)
  (* The ugly ducklings; In the FFP, `await` and `clone` are parsed as
   * `UnaryOperator`s, whereas the typed AST has separate constructors for
   * `Await`, `Clone` and `Uop`. Also, `@` is thrown out by the old parser.
   * This is why we don't just project onto a `uop`, but rather a
   * `expr -> expr_`.
   *)
  | Some TK.Await                   -> Await expr
  | Some TK.Clone                   -> Clone expr
  | Some TK.At                      -> snd expr
  | _ -> missing_syntax "unary operator" node env

let pBop : (expr -> expr -> expr_) parser = fun node env lhs rhs ->
  match token_kind node with
  | Some TK.Equal                       -> Binop (Eq None,           lhs, rhs)
  | Some TK.Bar                         -> Binop (Bar,               lhs, rhs)
  | Some TK.Ampersand                   -> Binop (Amp,               lhs, rhs)
  | Some TK.Plus                        -> Binop (Plus,              lhs, rhs)
  | Some TK.Minus                       -> Binop (Minus,             lhs, rhs)
  | Some TK.Star                        -> Binop (Star,              lhs, rhs)
  | Some TK.Or                          -> Binop (BArbar,            lhs, rhs)
  | Some TK.Xor                         -> Binop (Xor,               lhs, rhs)
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
  | Some TK.ExclamationEqualEqual       -> Binop (Diff2,             lhs, rhs)
  (* The spaceship operator `<=>` isn't implemented in AST, but has the same
   * typing properties as `<=`
   *)
  | Some TK.LessThanEqualGreaterThan    -> Binop (Lte,               lhs, rhs)
  (* The ugly ducklings; In the FFP, `|>` and '??' are parsed as
   * `BinaryOperator`s, whereas the typed AST has separate constructors for
   * NullCoalesce, Pipe and Binop. This is why we don't just project onto a
   * `bop`, but - analagous to pUop - a `expr -> expr -> expr_`.
   *)
  | Some TK.BarGreaterThan              -> Pipe         (lhs, rhs)
  | Some TK.QuestionQuestion            -> NullCoalesce (lhs, rhs)
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

let pKinds : kind list parser = couldMap ~f:(fun node env ->
  match token_kind node with
  | Some TK.Final     -> Final
  | Some TK.Static    -> Static
  | Some TK.Abstract  -> Abstract
  | Some TK.Private   -> Private
  | Some TK.Public    -> Public
  | Some TK.Protected -> Protected
  | _ -> missing_syntax "kind" node env
  )

let syntax_of_token : PositionedToken.t -> node = fun t ->
  { syntax = Token t
  ; value  = PositionedSyntaxValue.make t.PT.source_text
               t.PT.offset t.PT.leading_width t.PT.width t.PT.trailing_width
  }

(* TODO: Clean up string escaping *)
let prepString2 : node list -> node list =
  let trimLeft t =
    PT.({ t with leading_width = t.leading_width + 1; width = t.width - 1 })
  in
  let trimRight t =
    PT.({ t with trailing_width = t.trailing_width + 1; width = t.width - 1 })
  in
  function
  | ({ syntax = Token t; _ }::ss)
  when t.PT.width > 0 && (PT.text t).[0] = '"' ->
    let rec unwind = function
      | [{ syntax = Token t; _ }]
      when t.PT.width > 0 && (PT.text t).[t.PT.width - 1] = '"' ->
        let s = syntax_of_token (trimRight t) in
        if width s > 0 then [s] else []
      | x :: xs -> x :: unwind xs
      | _ -> raise (Invalid_argument "Malformed String2 SyntaxList")
    in
    let s = syntax_of_token (trimLeft t) in
    if width s > 0 then s :: unwind ss else unwind ss
  | x -> x (* unchanged *)

let mkStr : (string -> string) -> string -> string = fun unescaper content ->
  let no_quotes = try
      if String.sub content 0 3 = "<<<" (* The heredoc case *)
      then String.sub content 3 (String.length content - 4)
      else String.sub content 1 (String.length content - 2)
    with
    | Invalid_argument _ -> content
  in
  try unescaper no_quotes with
  | Php_escaping.Invalid_string _ -> raise @@
      Failure (Printf.sprintf "Malformed string literal <<%s>>" no_quotes)
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
let unesc_xhp_attr s =
  let open Str in
  unesc_dbl @@
    if string_match (regexp "[ \t\n\r\012]*\"\\(\\(.\\|\n\\)*\\)\"") s 0
    then matched_group 1 s
    else s

let mk_fun_kind sync yield =
  match sync, yield with
  | true,  true  -> FGenerator
  | false, true  -> FAsyncGenerator
  | true,  false -> FSync
  | false, false -> FAsync

let fun_template yielding node is_sync =
  let p = get_pos node in
  { f_mode            = !(lowerer_state.mode)
  ; f_tparams         = []
  ; f_ret             = None
  ; f_ret_by_ref      = false
  ; f_name            = p, ";anonymous"
  ; f_params          = []
  ; f_body            = []
  ; f_user_attributes = []
  ; f_fun_kind        = mk_fun_kind is_sync yielding
  ; f_namespace       = Namespace_env.empty_with_default_popt
  ; f_span            = p
  }

let param_template node =
  { param_hint            = None
  ; param_is_reference    = false
  ; param_is_variadic     = false
  ; param_id              = pos_name node
  ; param_expr            = None
  ; param_modifier        = None
  ; param_user_attributes = []
  }

let mpShapeField : ('a, (shape_field_name * 'a)) metaparser = fun pThing ->
  fun node env ->
    match syntax node with
    | FieldSpecifier { field_name = name; field_type = thing; _ }
    | FieldInitializer
      { field_initializer_name = name; field_initializer_value = thing; _ }
      ->
        ( (match syntax name with
          | ScopeResolutionExpression
            { scope_resolution_qualifier; scope_resolution_name; _ } ->
              SFclass_const
              ( pos_name scope_resolution_qualifier
              , pos_name scope_resolution_name
              )
          | _ -> let p, n = pos_name name in SFlit (p, mkStr unesc_dbl n)
          )
        , pThing thing env
        )
    | _ -> missing_syntax "shape field" node env

let rec pHint : hint parser = fun node env ->
  let rec pHint_ : hint_ parser = fun node env ->
    match syntax node with
    (* Dirty hack; CastExpression can have type represented by token *)
    | Token _
    | SimpleTypeSpecifier _
      -> Happly (pos_name node, [])
    | ShapeTypeSpecifier { shape_type_fields; _ } ->
      let pShapeField node env =
        let sf_name, sf_hint = mpShapeField pHint node env in
        { sf_optional = false; sf_name; sf_hint }
      in
      Hshape (couldMap ~f:pShapeField shape_type_fields env)
    | TupleTypeSpecifier { tuple_types; _ } ->
      Htuple (couldMap ~f:pHint tuple_types env)

    | KeysetTypeSpecifier { keyset_type_keyword = kw; keyset_type_type = ty; _ }
    | VectorTypeSpecifier { vector_type_keyword = kw; vector_type_type = ty; _ }
    | ClassnameTypeSpecifier {classname_keyword = kw; classname_type   = ty; _ }
    | VectorArrayTypeSpecifier
      { vector_array_keyword = kw
      ; vector_array_type    = ty
      ; _ }
      -> Happly (pos_name kw, couldMap ~f:pHint ty env)

    | MapArrayTypeSpecifier
      { map_array_keyword = kw
      ; map_array_key     = key
      ; map_array_value   = value
      ; _ } ->
        Happly
        ( pos_name kw
        , List.map (fun x -> pHint x env) [ key; value ]
        )
    | DictionaryTypeSpecifier
      { dictionary_type_keyword = kw
      ; dictionary_type_members = members
      ; _ } -> Happly (pos_name kw, couldMap ~f:pHint members env)
    | GenericTypeSpecifier { generic_class_type; generic_argument_list } ->
      Happly
      ( pos_name generic_class_type
      , match syntax generic_argument_list with
        | TypeArguments { type_arguments_types; _ }
          -> couldMap ~f:pHint type_arguments_types env
        | _ -> missing_syntax "generic type arguments" generic_argument_list env
      )
    | NullableTypeSpecifier { nullable_type; _ } ->
      Hoption (pHint nullable_type env)
    | SoftTypeSpecifier { soft_type; _ } -> pHint_ soft_type env
    | ClosureTypeSpecifier { closure_parameter_types; closure_return_type; _ }
      -> Hfun
      ( couldMap ~f:pHint closure_parameter_types env
      , false
      , pHint closure_return_type env
      )
    | TypeConstant { type_constant_left_type; type_constant_right_type; _ } ->
      let child = pos_name type_constant_right_type in
      (match pHint_ type_constant_left_type env with
      | Haccess (b, c, cs) -> Haccess (b, c, cs @ [child])
      | Happly (b, []) -> Haccess (b, child, [])
      | _ -> missing_syntax "type constant base" node env
      )
    | _ -> missing_syntax "type hint" node env
  in
  get_pos node, pHint_ node env

let rec pSimpleInitializer node env =
  match syntax node with
  | SimpleInitializer { simple_initializer_value; _ } ->
    pExpr simple_initializer_value env
  | _ -> missing_syntax "simple initializer" node env
and pFunParam : fun_param parser = fun node env ->
  match syntax node with
  | ParameterDeclaration
    { parameter_attribute
    ; parameter_visibility
    ; parameter_type
    ; parameter_name
    ; parameter_default_value
    } ->
    let decoration, name =
      match syntax parameter_name with
      | DecoratedExpression
        { decorated_expression_decorator; decorated_expression_expression } ->
          text decorated_expression_decorator, decorated_expression_expression
      | _ -> "", parameter_name
    in
    { param_hint            = mpOptional pHint parameter_type env
    ; param_is_reference    = decoration = "&"
    ; param_is_variadic     = decoration = "..."
    ; param_id              = pos_name name
    ; param_expr            =
      mpOptional pSimpleInitializer parameter_default_value env
    ; param_user_attributes = List.flatten @@
      couldMap ~f:pUserAttribute parameter_attribute env
    (* implicit field via constructor parameter.
     * This is always None except for constructors and the modifier
     * can be only Public or Protected or Private.
     *)
    ; param_modifier =
      let rec go = function
      | [] -> None
      | x :: _ when List.mem x [Private; Public; Protected] -> Some x
      | _ :: xs -> go xs
      in
      go (pKinds parameter_visibility env)
    }
  | VariadicParameter _
  | Token _ when text node = "..."
    -> { (param_template node) with param_is_variadic = true }
  | _ -> missing_syntax "function parameter" node env
and pUserAttribute : user_attribute list parser = fun node env ->
  match syntax node with
  | AttributeSpecification { attribute_specification_attributes; _ } ->
    couldMap attribute_specification_attributes env ~f:begin function
      | { syntax = Attribute { attribute_name; attribute_values; _}; _ } ->
        fun env ->
          { ua_name   = pos_name attribute_name
          ; ua_params = couldMap ~f:pExpr attribute_values env
          }
      | node -> missing_syntax "attribute" node
    end
  | _ -> missing_syntax "attribute specification" node env
and pAField : afield parser = fun node env ->
  match syntax node with
  | ElementInitializer { element_key; element_value; _ } ->
    AFkvalue (pExpr element_key env, pExpr element_value env)
  | _ -> AFvalue (pExpr node env)
and pExpr ?top_level:(top_level=true) : expr parser = fun node env ->
  let rec pExpr_ : expr_ parser = fun node env ->
    let pos = get_pos node in
    match syntax node with
    | LambdaExpression { lambda_async; lambda_signature; lambda_body; _ } ->
      let is_sync = is_missing lambda_async in
      let f_params, f_ret =
        match syntax lambda_signature with
        | LambdaSignature { lambda_parameters; lambda_type; _ } ->
          ( couldMap ~f:pFunParam lambda_parameters env
          , mpOptional pHint lambda_type env
          )
        | Token _ -> ([param_template lambda_signature], None)
        | _ -> missing_syntax "lambda signature" lambda_signature env
      in
      let pBody node env =
        try mk_noop (pBlock node env) with
        | _ ->
          let (p,r) = pExpr node env in
          [ Return (p, Some (p, r)) ]
      in
      let f_body, yield = mpYielding pBody lambda_body env in
      Lfun
      { (fun_template yield node is_sync) with f_ret; f_params; f_body }

    | BracedExpression        { braced_expression_expression        = expr; _ }
    | ParenthesizedExpression { parenthesized_expression_expression = expr; _ }
    | DecoratedExpression     { decorated_expression_expression     = expr; _ }
      -> pExpr_ expr env

    | DictionaryIntrinsicExpression
      { dictionary_intrinsic_keyword = kw
      ; dictionary_intrinsic_members = members
      ; _ }
    | KeysetIntrinsicExpression
      { keyset_intrinsic_keyword = kw
      ; keyset_intrinsic_members = members
      ; _ }
    | VectorIntrinsicExpression
      { vector_intrinsic_keyword = kw
      ; vector_intrinsic_members = members
      ; _ }
    | CollectionLiteralExpression
      { collection_literal_name         = kw
      ; collection_literal_initializers = members
      ; _ }
      -> Collection (pos_name kw, couldMap ~f:pAField members env)

    | ArrayIntrinsicExpression { array_intrinsic_members = members; _ }
    | ArrayCreationExpression  { array_creation_members  = members; _ }
    ->
      (* TODO: Or tie in with other intrinsics and post-process to Array *)
      Array (couldMap ~f:pAField members env)

    | ListExpression { list_members = members; _ } ->
      (* TODO: Or tie in with other intrinsics and post-process to List *)
      List (couldMap ~f:pExpr members env)

    | EvalExpression  { eval_keyword  = recv; eval_argument       = args; _ }
    | EmptyExpression { empty_keyword = recv; empty_argument      = args; _ }
    | IssetExpression { isset_keyword = recv; isset_argument_list = args; _ }
    | TupleExpression
      { tuple_expression_keyword = recv
      ; tuple_expression_items   = args
      ; _ }
    | FunctionCallExpression
      { function_call_receiver      = recv
      ; function_call_argument_list = args
      ; _ }
      -> Call (pExpr recv env, couldMap ~f:pExpr args  env, [])

    | PrintExpression { print_keyword = _recv; print_expression = args } ->
      (* TODO: Or tie in with FunctionCallExpression et al and post-process *)
      Call ((pos, Id (pos, "echo")), couldMap ~f:pExpr args env, [])

    | QualifiedNameExpression { qualified_name_expression } ->
      Id (pos_name qualified_name_expression)
    | VariableExpression { variable_expression } ->
      Lvar (pos_name variable_expression)

    | PipeVariableExpression _ -> Dollardollar

    | InclusionExpression { inclusion_require; inclusion_filename } ->
      Import
      ( pImportFlavor inclusion_require env
      , pExpr inclusion_filename env
      )

    | MemberSelectionExpression { member_object; member_operator; member_name }
      (* TODO: Surely member selection and safe member selection can be one *)
      -> Obj_get
      ( pExpr member_object env
      , (let p, n as name = pos_name member_name in
          (p, if n.[0] = '$' then Lvar name else Id name)
        )
      , pNullFlavor member_operator env
      )
    | SafeMemberSelectionExpression
      { safe_member_object; safe_member_operator; safe_member_name } ->
      Obj_get
      ( pExpr safe_member_object env
      , pExpr safe_member_name   env
      , pNullFlavor safe_member_operator env
      )

    | PrefixUnaryExpression
      { prefix_unary_operator = operator
      ; prefix_unary_operand  = operand
      }
    | PostfixUnaryExpression
      { postfix_unary_operand  = operand
      ; postfix_unary_operator = operator
      }
      -> pUop (kind node = SyntaxKind.PostfixUnaryExpression) operator env @@
        pExpr operand env

    | BinaryExpression
      { binary_left_operand; binary_operator; binary_right_operand }
      ->
        pBop binary_operator env
          (pExpr binary_left_operand  env)
          (pExpr binary_right_operand env)

    | Token _ when top_level -> Id (pos_name node)
    | Token _ -> String (pos, unesc_dbl (text node))

    | YieldExpression { yield_operand; _ } when text yield_operand = "break" ->
      env.saw_yield := true;
      Yield_break
    | YieldExpression { yield_operand; _ } ->
      env.saw_yield := true;
      Yield (pAField yield_operand env)


    | ScopeResolutionExpression
      { scope_resolution_qualifier; scope_resolution_name; _ } ->
      let (_, n) as name = pos_name scope_resolution_name in
      let qual = pos_name scope_resolution_qualifier in
      if String.length n > 0 && n.[0] = '$'
      then Class_get   (qual, name)
      else Class_const (qual, name)

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
    | ShapeExpression { shape_expression_fields; _ } ->
      Shape (couldMap ~f:(mpShapeField pExpr) shape_expression_fields env)
    | ObjectCreationExpression
      { object_creation_type; object_creation_argument_list; _ } ->
      New
      ( (match syntax object_creation_type with
        | QualifiedNameExpression _
        | SimpleTypeSpecifier _
        | Token _
          -> let name = pos_name object_creation_type in fst name, Id name
        | _ -> pExpr object_creation_type env
        )
      , couldMap ~f:pExpr object_creation_argument_list env
      , []
      )
    | LiteralExpression { literal_expression = expr } ->
      (match syntax expr with
      | Token _ ->
        let s = text expr in
        (match token_kind expr with
        | Some TK.DecimalLiteral
        | Some TK.OctalLiteral
        | Some TK.HexadecimalLiteral
        | Some TK.BinaryLiteral             -> Int    (pos, s)
        | Some TK.FloatingLiteral           -> Float  (pos, s)
        | Some TK.SingleQuotedStringLiteral -> String (pos, mkStr unesc_sgl s)
        | Some TK.DoubleQuotedStringLiteral
        | Some TK.HeredocStringLiteral
        | Some TK.NowdocStringLiteral       -> String (pos, mkStr unesc_dbl s)
        | Some TK.NullLiteral               -> Null
        | Some TK.BooleanLiteral            ->
          (match String.lowercase_ascii s with
          | "false" -> False
          | "true"  -> True
          | _       -> missing_syntax ("boolean (not: " ^ s ^ ")") expr env
          )
        | _ -> missing_syntax "literal" expr env
        )

      | SyntaxList ts -> String2
          (List.map (fun x -> pExpr ~top_level:false x env) (prepString2 ts))
      | _ -> missing_syntax "literal expression" expr env
      )

    | InstanceofExpression
      { instanceof_left_operand; instanceof_right_operand; _ } ->
      let ty =
        match pExpr instanceof_right_operand env with
        | p, Class_const (pid, (_, "")) -> p, Id pid
        | ty -> ty
      in
      InstanceOf (pExpr instanceof_left_operand env, ty)
      (* TODO: Priority fix? *)
      (*match pExpr instanceof_left_operand env with
      | p, Unop (o,e) -> Unop (0, (p, InstanceOf (e, ty)))
      | e -> InstanceOf (e, ty)
      *)
    | AnonymousFunction
      { anonymous_async_keyword
      ; anonymous_parameters
      ; anonymous_type
      ; anonymous_use
      ; anonymous_body
      ; _ } ->
        let pArg node env =
          match syntax node with
          | PrefixUnaryExpression
            { prefix_unary_operator = op; prefix_unary_operand = v } ->
              pos_name v, token_kind op = Some TK.Ampersand
          | Token _ -> pos_name node, false
          | _ -> missing_syntax "use variable" node env
        in
        let pUse node =
          match syntax node with
          | AnonymousFunctionUseClause { anonymous_use_variables; _ } ->
            couldMap ~f:pArg anonymous_use_variables
          | _ -> fun _env -> []
        in
        let is_sync = is_missing anonymous_async_keyword in
        let body, yield = mpYielding pBlock anonymous_body env in
        Efun
        ( { (fun_template yield node is_sync) with
            f_ret    = mpOptional pHint anonymous_type env
          ; f_params = couldMap ~f:pFunParam anonymous_parameters env
          ; f_body   = mk_noop body
          }
        , try pUse anonymous_use env with _ -> []
        )

    | AwaitableCreationExpression
      { awaitable_async; awaitable_compound_statement } ->
      let is_sync = is_missing awaitable_async in
      let blk, yld = mpYielding pBlock awaitable_compound_statement env in
      let body =
        { (fun_template yld node is_sync) with f_body = mk_noop blk }
      in
      Call ((get_pos node, Lfun body), [], [])
    | XHPExpression
      { xhp_open =
        { syntax = XHPOpen { xhp_open_name; xhp_open_attributes; _ }; _ }
      ; xhp_body = body
      ; _ } ->
      let name =
        let pos, name = drop_pstr 1 (pos_name xhp_open_name) in
        (pos, ":" ^ name)
      in
      let combine b e =
        syntax_of_token PT.(
          make (kind b) (source_text b) (leading_start_offset b)
            (start_offset e - start_offset b + width e) (leading b) (trailing e)
        )
      in
      let aggregate_tokens node =
        let rec search = function (* scroll through non-token things *)
        | [] -> []
        | { syntax = Token b; _ } as t :: xs -> track t b None xs
        | x :: xs -> x :: search xs
        and track t b oe = function (* keep going through consecutive tokens *)
        | { syntax = Token e; _ } :: xs -> track t b (Some e) xs
        | xs -> Option.value_map oe ~default:t ~f:(combine b) :: search xs
        in
        search (as_list node)
      in
      let pEmbedded escaper node env =
        match syntax node with
        | Token _ ->
          let p = get_pos node in
          p, String (p, escaper (full_text node))
        | _ -> pExpr node env
      in
      let pAttr = fun node env ->
        match syntax node with
        | XHPAttribute { xhp_attribute_name; xhp_attribute_expression; _ } ->
          ( pos_name xhp_attribute_name
          , pEmbedded unesc_xhp_attr xhp_attribute_expression env
          )
        | _ -> missing_syntax "XHP attribute" node env
      in
      Xml
      ( name
      , couldMap ~f:pAttr xhp_open_attributes env
      , List.map (fun x -> pEmbedded unesc_xhp x env) (aggregate_tokens body)
      )
    (* FIXME; should this include Missing? ; "| Missing -> Null" *)
    | _ -> missing_syntax "expression" node env
  in
  get_pos node, pExpr_ node env
and pBlock : block parser = fun node env ->
   match pStmt node env with
   | Block block -> List.filter (fun x -> x <> Noop) block
   | stmt -> [stmt]
and pStmt : stmt parser = fun node env ->
  match syntax node with
  | SwitchStatement { switch_expression; switch_sections; _ } ->
    let pSwitchLabel : (block -> case) parser = fun node env cont ->
      match syntax node with
      | CaseLabel { case_expression; _ } ->
        Case (pExpr case_expression env, cont)
      | DefaultLabel _ -> Default cont
      | _ -> missing_syntax "pSwitchLabel" node env
    in
    let pSwitchSection : case list parser = fun node env ->
      match syntax node with
      | SwitchSection { switch_section_labels; switch_section_statements; _ } ->
        let rec null_out cont = function
          | [x] -> [x cont]
          | (x::xs) -> x [] :: null_out cont xs
          | _ -> raise (Failure "Malformed block result")
        in
        let blk = couldMap ~f:pStmt switch_section_statements env in
        null_out blk (couldMap ~f:pSwitchLabel switch_section_labels env)
      | _ -> missing_syntax "switch section" node env
    in
    Switch
    ( pExpr switch_expression env
    , List.flatten @@ couldMap ~f:pSwitchSection switch_sections env
    )
  | IfStatement
    { if_condition; if_statement; if_elseif_clauses; if_else_clause; _ } ->
    let pElseIf : (block -> block) parser = fun node env ->
      match syntax node with
      | ElseifClause { elseif_condition; elseif_statement; _ } ->
        fun next_clause ->
          [ If
            ( pExpr elseif_condition env
            , [ pStmt elseif_statement env ]
            , next_clause
            )
          ]
      | _ -> missing_syntax "elseif clause" node env
    in
    If
    ( pExpr if_condition env
    , [ pStmt if_statement env ]
    , List.fold_right (@@)
        (couldMap ~f:pElseIf if_elseif_clauses env)
        [ match syntax if_else_clause with
          | ElseClause { else_statement; _ } -> pStmt else_statement env
          | Missing -> Noop
          | _ -> missing_syntax "else clause" if_else_clause env
        ]
    )
  | ExpressionStatement { expression_statement_expression; _ } ->
    if is_missing expression_statement_expression
    then Noop
    else Expr (pExpr expression_statement_expression env)
  | CompoundStatement { compound_statements; _ } ->
    Block (List.filter (fun x -> x <> Noop) @@
      couldMap ~f:pStmt compound_statements env)
  | ThrowStatement { throw_expression; _ } -> Throw (pExpr throw_expression env)
  | DoStatement { do_body; do_condition; _ } ->
    Do ([Block (pBlock do_body env)], pExpr do_condition env)
  | WhileStatement { while_condition; while_body; _ } ->
    While (pExpr while_condition env, [ pStmt while_body env ])
  | ForStatement
    { for_initializer; for_control; for_end_of_loop; for_body; _ } ->
    let pExprL node env =
      (get_pos node, Expr_list (couldMap ~f:pExpr node env))
    in
    For
    ( pExprL for_initializer env
    , pExprL for_control env
    , pExprL for_end_of_loop env
    , [Block (pBlock for_body env)]
    )
  | ForeachStatement
    { foreach_collection
    ; foreach_await_keyword
    ; foreach_key
    ; foreach_value
    ; foreach_body
    ; _ } ->
      Foreach
      ( pExpr foreach_collection env
      , ( if token_kind foreach_await_keyword = Some TK.Await
          then Some (get_pos foreach_await_keyword)
          else None
        )
      , ( let value = pExpr foreach_value env in
          Option.value_map (mpOptional pExpr foreach_key env)
            ~default:(As_v value)
            ~f:(fun key -> As_kv (key, value))
        )
      , [ pStmt foreach_body env ]
      )
  | TryStatement
    { try_compound_statement; try_catch_clauses; try_finally_clause; _ } ->
    Try
    ( [ Block (pBlock try_compound_statement env) ]
    , couldMap try_catch_clauses env ~f:begin fun node env ->
      match syntax node with
      | CatchClause { catch_type; catch_variable; catch_body; _ } ->
        ( pos_name catch_type
        , pos_name catch_variable
        , [ Block (mpStripNoop pBlock catch_body env) ]
        )
      | _ -> missing_syntax "catch clause" node env
      end
    , match syntax try_finally_clause with
      | FinallyClause { finally_body; _ } -> [ Block (pBlock finally_body env) ]
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
          ( get_pos static_initializer
          , Binop (Eq None, lhs, pExpr simple_initializer_value env)
          )
        | _ -> lhs
        )
      | _ -> missing_syntax "static declarator" node env
    in
    Static_var (couldMap ~f:pStaticDeclarator static_declarations env)
  | ReturnStatement { return_expression; _ } -> Return
      ( get_pos return_expression
      , mpOptional pExpr return_expression env
      )
  | EchoStatement  { echo_keyword  = kw; echo_expressions = exprs; _ }
  | UnsetStatement { unset_keyword = kw; unset_variables  = exprs; _ }
    -> Expr
      ( get_pos node
      , Call
        ( (match syntax kw with
          | QualifiedNameExpression _
          | SimpleTypeSpecifier _
          | Token _
            -> let name = pos_name kw in fst name, Id name
          | _ -> missing_syntax "id" kw env
          )
        , couldMap ~f:pExpr exprs env
        , []
      ))
  | BreakStatement _ -> Break (get_pos node)
  | ContinueStatement _ -> Continue (get_pos node)
  | _ -> missing_syntax "statement" node env

let pTConstraintTy : hint parser = fun node ->
  match syntax node with
  | TypeConstraint { constraint_type; _ } -> pHint constraint_type
  | _ -> missing_syntax "type constraint" node

let pTConstraint : (constraint_kind * hint) parser = fun node env ->
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

let pTParaml : tparam list parser = fun node env ->
  let pTParam : tparam parser = fun node env ->
    match syntax node with
    | TypeParameter { type_variance; type_name; type_constraints } ->
      ( (match token_kind type_variance with
        | Some TK.Plus  -> Covariant
        | Some TK.Minus -> Contravariant
        | _ -> Invariant
        )
      , pos_name type_name
      , couldMap ~f:pTConstraint type_constraints env
      )
    | _ -> missing_syntax "type parameter" node env
  in
  match syntax node with
  | Missing -> []
  | TypeParameters { type_parameters_parameters; _ } ->
    couldMap ~f:pTParam type_parameters_parameters env
  | _ -> missing_syntax "type parameter list" node env

type fun_hdr =
  { fh_is_sync         : bool
  ; fh_name            : pstring
  ; fh_type_parameters : tparam list
  ; fh_parameters      : fun_param list
  ; fh_return_type     : hint option
  ; fh_param_modifiers : fun_param list
  ; fh_keep_noop       : bool
  }

let empty_fun_hdr =
  { fh_is_sync         = false
  ; fh_name            = Pos.none, "<ANONYMOUS>"
  ; fh_type_parameters = []
  ; fh_parameters      = []
  ; fh_return_type     = None
  ; fh_param_modifiers = []
  ; fh_keep_noop       = false
  }

(* TODO: Translate the where clause *)
let pFunHdr : fun_hdr parser = fun node env ->
  match syntax node with
  | FunctionDeclarationHeader
    { function_async
    ; function_name
    ; function_type_parameter_list
    ; function_parameter_list
    ; function_type
    ; _ } ->
      let fh_parameters = couldMap ~f:pFunParam function_parameter_list env in
      let fh_return_type, fh_keep_noop =
        match mpOptional pHint function_type env with
        | Some x -> Some x, false
        | None ->
            let pos = get_pos function_type in
            match text function_name with
            | "__construct"
            | "__destruct" -> Some (pos, Happly ((pos, "void"), [])), true
            | _            -> None, false
      in
      { fh_is_sync         = is_missing function_async
      ; fh_name            = pos_name function_name
      ; fh_type_parameters = pTParaml function_type_parameter_list env
      ; fh_parameters
      ; fh_return_type
      ; fh_param_modifiers =
        List.filter (fun p -> Option.is_some p.param_modifier) fh_parameters
      ; fh_keep_noop
      }
  | LambdaSignature { lambda_parameters; lambda_type; _ } ->
    { empty_fun_hdr with
      fh_parameters  = couldMap ~f:pFunParam lambda_parameters env
    ; fh_return_type = mpOptional pHint lambda_type env
    }
  | Token _ -> empty_fun_hdr
  | _ -> missing_syntax "function header" node env

let pClassElt : class_elt list parser = fun node env ->
  match syntax node with
  | ConstDeclaration
    { const_abstract; const_type_specifier; const_declarators; _ } ->
      let ty = mpOptional pHint const_type_specifier env in
      let res =
        couldMap const_declarators env ~f:begin function
          | { syntax = ConstantDeclarator
              { constant_declarator_name; constant_declarator_initializer }
            ; _ } -> fun env ->
              ( pos_name constant_declarator_name
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
    ; type_const_type_constraint
    ; type_const_type_specifier
    ; _ } ->
      [ TypeConst
        { tconst_abstract   = not (is_missing type_const_abstract)
        ; tconst_name       = pos_name type_const_name
        ; tconst_constraint = mpOptional pTConstraintTy type_const_type_constraint env
        ; tconst_type       = mpOptional pHint type_const_type_specifier env
        ; tconst_span       = get_pos node
        }
      ]
  | PropertyDeclaration
    { property_modifiers; property_type; property_declarators; _ } ->
      [ ClassVars
        ( pKinds property_modifiers env
        , mpOptional pHint property_type env
        , couldMap property_declarators env ~f:begin fun node env ->
          match syntax node with
          | PropertyDeclarator { property_name; property_initializer } ->
            ( let _, n as name = pos_name property_name in
              ( get_pos node
              , ( if n.[0] = '$'
                  then drop_pstr 1 name
                  else name
                )
              , mpOptional pSimpleInitializer property_initializer env
              )
            )
          | _ -> missing_syntax "property declarator" node env
          end
        )
      ]
  | MethodishDeclaration
    { methodish_attribute
    ; methodish_modifiers
    ; methodish_function_decl_header
    ; methodish_function_body
    ; _ } ->
      let classvar_init : fun_param -> stmt * class_elt = fun param ->
        let p, _ as cvname = drop_pstr 1 param.param_id in (* Drop the '$' *)
        let span =
          match param.param_expr with
          | Some (pos_end, _) -> Pos.btw p pos_end
          | None -> p
        in
        ( Expr (p, Binop (Eq None,
            (p, Obj_get((p, Lvar (p, "$this")), (p, Id cvname), OG_nullthrows)),
            (p, Lvar param.param_id)
          ))
        , ClassVars
          ( Option.to_list param.param_modifier
          , param.param_hint
          , [span, cvname, None]
          )
        )
      in
      let hdr = pFunHdr methodish_function_decl_header env in
      let member_init, member_def =
        List.split @@
          List.map classvar_init @@
            List.filter (fun p -> Option.is_some p.param_modifier) @@
              hdr.fh_parameters
      in
      let pBody = fun node env ->
        if is_missing node then [] else
          (* TODO: Give parse error when not abstract, but body is missing *)
          member_init @
          match pStmt node env with
          | Block []    -> [Noop]
          | Block stmtl -> stmtl
          | stmt        -> [stmt]
      in
      let body, body_has_yield = mpYielding pBody methodish_function_body env in
      let kind = pKinds methodish_modifiers env in
      member_def @ [Method
      { m_kind            = kind
      ; m_tparams         = hdr.fh_type_parameters
      ; m_constrs         = []
      ; m_name            = hdr.fh_name
      ; m_params          = hdr.fh_parameters
      ; m_body            = body
      ; m_user_attributes = List.flatten @@
        couldMap ~f:pUserAttribute methodish_attribute env
      ; m_ret             = hdr.fh_return_type
      ; m_ret_by_ref      = false
      ; m_span            = get_pos node
      ; m_fun_kind        = mk_fun_kind hdr.fh_is_sync body_has_yield
      }]
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
          let (p, name) = pos_name name in
          XhpAttr
          ( mpOptional pHint ty env
          , (Pos.none, (p, ":" ^ name), mpOptional pSimpleInitializer init env)
          , not (is_missing req)
          , match syntax ty with
            | XHPEnumType { xhp_enum_values; _ } ->
              Some (get_pos ty, couldMap ~f:pExpr xhp_enum_values env)
            | _ -> None
          )
      | XHPSimpleClassAttribute { xhp_simple_class_attribute_type = attr } ->
        XhpAttrUse (get_pos attr, Happly (pos_name attr, []))
      | Token _ ->
        XhpAttrUse (get_pos node, Happly (pos_name node, []))
      | _ -> missing_syntax "XHP attribute" node env
    in
    couldMap ~f:pXHPAttr xhp_attribute_attributes env
  | XHPChildrenDeclaration _ -> []
  | XHPCategoryDeclaration { xhp_category_categories = cats; _ } ->
    let pNameSansPercent node _env = drop_pstr 1 (pos_name node) in
    [ XhpCategory (couldMap ~f:pNameSansPercent cats env) ]
  | _ -> missing_syntax "expression" node env

(*****************************************************************************(
 * Parsing definitions (AST's `def`)
)*****************************************************************************)
let rec pDefStmt node env = try Stmt (pStmt node env) with _ -> pDef node env
and pDef : def parser = fun node env ->
  match syntax node with
  | FunctionDeclaration
    { function_attribute_spec; function_declaration_header; function_body } ->
      let containsUNSAFE =
        let re = Str.regexp_string "UNSAFE" in
        try Str.search_forward re (full_text function_body) 0 >= 0 with
        | Not_found -> false
      in
      let hdr = pFunHdr function_declaration_header env in
      let block, yield = mpYielding (mpOptional pBlock) function_body env in
      Fun
      { (fun_template yield node hdr.fh_is_sync) with
        f_tparams         = hdr.fh_type_parameters
      ; f_ret             = hdr.fh_return_type
      ; f_name            = hdr.fh_name
      ; f_params          = hdr.fh_parameters
      ; f_body            = begin
          (* FIXME: Filthy hack to catch UNSAFE *)
          match block with
          | Some [Noop] when containsUNSAFE -> [Unsafe]
          | Some [] -> [Noop]
          | None -> []
          | Some b -> b
        end
      ; f_user_attributes =
        List.flatten @@ couldMap ~f:pUserAttribute function_attribute_spec env
      }
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
      Class
      { c_mode            = !(lowerer_state.mode)
      ; c_user_attributes = List.flatten @@ couldMap ~f:pUserAttribute attr env
      ; c_final           = List.mem Final @@ pKinds mods env
      ; c_is_xhp          =
        (match token_kind name with
        | Some TK.XHPElementName | Some TK.XHPClassName -> true
        | _ -> false
        )
      ; c_name            = pos_name name
      ; c_tparams         = pTParaml tparaml env
      ; c_extends         = couldMap ~f:pHint exts env
      ; c_implements      = couldMap ~f:pHint impls env
      ; c_body            = List.concat (couldMap ~f:pClassElt elts env)
      ; c_namespace       = Namespace_env.empty_with_default_popt
      ; c_enum            = None
      ; c_span            = get_pos node
      ; c_kind            =
        let is_abs = Str.(string_match (regexp ".*abstract.*") (text mods) 0) in
        match token_kind kw with
        | Some TK.Class when is_abs -> Cabstract
        | Some TK.Class             -> Cnormal
        | Some TK.Interface         -> Cinterface
        | Some TK.Trait             -> Ctrait
        | Some TK.Enum              -> Cenum
        | _ -> missing_syntax "class kind" kw env
      }
  | ConstDeclaration
    { const_type_specifier = ty
    ; const_declarators    = decls
    ; _ } ->
      (match List.map syntax (as_list decls) with
      | [ ConstantDeclarator
          { constant_declarator_name        = name
          ; constant_declarator_initializer = init
          }
        ] -> Constant
          { cst_mode      = !(lowerer_state.mode)
          ; cst_kind      = Cst_const
          ; cst_name      = pos_name name
          ; cst_type      = mpOptional pHint ty env
          ; cst_value     = pSimpleInitializer init env
          ; cst_namespace = Namespace_env.empty_with_default_popt
          }
      | _ -> missing_syntax "constant declaration" decls env
      )
  | AliasDeclaration
    { alias_attribute_spec    = attr
    ; alias_keyword           = kw
    ; alias_name              = name
    ; alias_generic_parameter = tparams
    ; alias_constraint        = constr
    ; alias_type              = hint
    ; _ } -> Typedef
      { t_id              = pos_name name
      ; t_tparams         = pTParaml tparams env
      ; t_constraint      = Option.map ~f:snd @@
          mpOptional pTConstraint constr env
      ; t_user_attributes = List.flatten @@
          List.map (fun x -> pUserAttribute x env) (as_list attr)
      ; t_namespace       = Namespace_env.empty_with_default_popt
      ; t_mode            = !(lowerer_state.mode)
      ; t_kind            =
        match token_kind kw with
        | Some TK.Newtype -> NewType (pHint hint env)
        | Some TK.Type    -> Alias   (pHint hint env)
        | _ -> missing_syntax "kind" kw env
      }
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
          fun env -> Const (None, [pos_name name, pExpr value env])
        | _ -> missing_syntax "enumerator" node
      in
      Class
      { c_mode            = !(lowerer_state.mode)
      ; c_user_attributes = List.flatten @@ couldMap ~f:pUserAttribute attrs env
      ; c_final           = false
      ; c_kind            = Cenum
      ; c_is_xhp          = false
      ; c_name            = pos_name name
      ; c_tparams         = []
      ; c_extends         = []
      ; c_implements      = []
      ; c_body            = couldMap enums env ~f:pEnumerator
      ; c_namespace       = Namespace_env.empty_with_default_popt
      ; c_span            = get_pos node
      ; c_enum            = Some
        { e_base       = pHint base env
        ; e_constraint = mpOptional pTConstraintTy constr env
        }
      }
  | InclusionDirective
    { inclusion_expression =
      { syntax = InclusionExpression
        { inclusion_require  = req
        ; inclusion_filename = file
        }
      ; _ }
    ; inclusion_semicolon  = _
    } ->
      let flavor = pImportFlavor req env in
      Stmt (Expr (get_pos node, Import (flavor, pExpr file env)))
  | NamespaceDeclaration
    { namespace_name = name
    ; namespace_body =
      { syntax = NamespaceBody { namespace_declarations = decls; _ }; _ }
    ; _ } -> Namespace
      ( pos_name name
      , List.map (fun x -> pDefStmt x env) (as_list decls)
      )
  | NamespaceUseDeclaration
    { namespace_use_kind    = kind
    ; namespace_use_clauses = clauses
    ; _ } ->
      let f node = match syntax node with
      | NamespaceUseClause
        { namespace_use_name  = name
        ; namespace_use_alias = alias
        ; _ } ->
          let (p, n) as name = pos_name name in
          let x = Str.search_forward (Str.regexp "[^\\\\]*$") n 0 in
          let key = drop_pstr x name in
          let kind =
            match syntax kind with
            | Missing                            -> NSClass
            | Token { PT.kind = TK.Function; _ } -> NSFun
            | Token { PT.kind = TK.Const   ; _ } -> NSConst
            | _ -> missing_syntax "namespace use kind" kind env
          in
          ( kind
          , (p, if n.[0] = '\\' then n else "\\" ^ n)
          , if is_missing alias
            then key
            else pos_name alias
          )
      | _ -> missing_syntax "namespace use clause" node env
      in
      NamespaceUse (List.map f (as_list clauses))
  | NamespaceGroupUseDeclaration _ -> NamespaceUse []
  (* The ugly duckling; Here, the FFP tree has one /fewer/ level of hierarchy,
   * so we have to "step back" and look at node.
   *)
  | ExpressionStatement _ -> Stmt (pStmt node env)
  | _ -> missing_syntax "definition" node env
let pProgram : program parser = fun node env ->
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
    | (Namespace (n, [])::el) ->
      let body, remainder = span not_namespace el in
      Namespace (n, body) :: post_process remainder
    | (Namespace (n, il)::el) ->
      Namespace (n, post_process il) :: post_process el
    | (Stmt Noop::el) -> post_process el
    | ((Stmt (Expr (_, (Call
        ( (_, (Id (_, "define")))
        , [ (_, (String name))
          ; value
          ]
        , []
        )
      )))) :: el) -> Constant
        { cst_mode      = !(lowerer_state.mode)
        ; cst_kind      = Cst_define
        ; cst_name      = name
        ; cst_type      = None
        ; cst_value     = value
        ; cst_namespace = Namespace_env.empty_with_default_popt
        } :: post_process el
    | (e::el) -> e :: post_process el
  in

  (* The list of top-level things in a file is somewhat special. *)
  let rec aux env = function
  | [] -> []
  (* EOF happens only as the last token in the list. *)
  | [{ syntax = EndOfFile _; _ }] -> []
  (* There's an incompatibility between the Full-Fidelity (FF) and the AST view
   * of the world; `define` is an *expression* in FF, but a *definition* in AST.
   * Luckily, `define` only happens at the level of definitions.
   *)
  | { syntax = ExpressionStatement
      { expression_statement_expression =
        { syntax = DefineExpression { define_argument_list = args; _ } ; _ }
      ; _ }
    ; _ } :: nodel ->
      ( match List.map (fun x -> pExpr x env) (as_list args) with
      | [ _, String name; e ] -> Constant
        { cst_mode      = !(lowerer_state.mode)
        ; cst_kind      = Cst_define
        ; cst_name      = name
        ; cst_type      = None
        ; cst_value     = e
        ; cst_namespace = Namespace_env.empty_with_default_popt
        }
      | _ -> missing_syntax "DefineExpression:inner" args env
      ) :: aux env nodel
  | node :: nodel -> pDefStmt node env :: aux env nodel
  in
  post_process @@ aux env (as_list node)

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
let rec scour_line_comments (node : node)
  : (Pos.t * string) list * Pos.t IMap.t IMap.t =
    let comments, fixmes =
      List.split @@ List.map scour_line_comments (children node)
    in
    (* TODO: Actually add comments for 'this' node. *)
    ( List.concat comments
    , IMap.empty (* TODO: Correctly combine fixmes-s *)
    )

(*****************************************************************************(
 * Front-end matter
)*****************************************************************************)

exception Unknown_hh_mode of string
let unknown_hh_mode s = raise (Unknown_hh_mode s)

type result =
  { fi_mode  : FileInfo.mode
  ; ast      : Ast.program
  ; content  : string
  ; file     : Relative_path.t
  ; comments : (Pos.t * string) list
  }

let from_text
  ?(elaborate_namespaces  = true)
  ?(include_line_comments = false)
  ?(keep_errors           = true)
  ?(parser_options        = ParserOptions.default)
  (file        : Relative_path.t)
  (source_text : Full_fidelity_source_text.t)
  : result =
    let open Full_fidelity_syntax_tree in
    let tree   = make source_text in
    let script = Full_fidelity_positioned_syntax.from_tree tree in
    let fi_mode =
      (match mode tree with
      | _ when is_php tree -> FileInfo.Mdecl
      | "decl"             -> FileInfo.Mdecl
      | "strict"           -> FileInfo.Mstrict
      | "partial" | ""     -> FileInfo.Mpartial
      | s                  -> unknown_hh_mode s
      )
    in
    lowerer_state.language := language tree;
    lowerer_state.filePath := Relative_path.suffix file;
    lowerer_state.mode     := fi_mode;
    let errors = ref [] in (* The top-level error list. *)
    let ast = runP pScript script { saw_yield = ref false; errors } in
    let ast =
      if elaborate_namespaces
      then Namespaces.elaborate_defs parser_options ast
      else ast
    in
    let content = Full_fidelity_source_text.text source_text in
    let comments, fixmes = scour_line_comments script in
    let comments = if include_line_comments then comments else [] in
    if keep_errors then begin
      Fixmes.HH_FIXMES.add file fixmes;
      Option.iter (Core.List.last !errors) Errors.parsing_error
    end;
    { fi_mode; ast; content; comments; file }

let from_file
  ?(elaborate_namespaces  = true)
  ?(include_line_comments = false)
  ?(keep_errors           = true)
  ?(parser_options        = ParserOptions.default)
  (path : Relative_path.t)
  : result =
    from_text
      ~elaborate_namespaces
      ~include_line_comments
      ~keep_errors
      ~parser_options
      path
      (Full_fidelity_source_text.from_file path)

(*****************************************************************************(
 * Backward compatibility matter (should be short-lived)
)*****************************************************************************)

let legacy (x : result) : Parser_hack.parser_return =
  { Parser_hack.file_mode = Some x.fi_mode
  ; Parser_hack.comments  = x.comments
  ; Parser_hack.ast       = x.ast
  ; Parser_hack.content   = x.content
  }

let from_text_with_legacy
  ?(elaborate_namespaces  = true)
  ?(include_line_comments = false)
  ?(keep_errors           = true)
  ?(parser_options        = ParserOptions.default)
  (file    : Relative_path.t)
  (content : string)
  : Parser_hack.parser_return =
    legacy @@ from_text
      ~elaborate_namespaces
      ~include_line_comments
      ~keep_errors
      ~parser_options
      file
      (Full_fidelity_source_text.make content)

let from_file_with_legacy
  ?(elaborate_namespaces  = true)
  ?(include_line_comments = false)
  ?(keep_errors           = true)
  ?(parser_options        = ParserOptions.default)
  (file : Relative_path.t)
  : Parser_hack.parser_return =
    legacy @@ from_file
      ~elaborate_namespaces
      ~include_line_comments
      ~keep_errors
      ~parser_options
      file
