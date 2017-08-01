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
module SourceText = Full_fidelity_source_text
open Prim_defs

open Core

let drop_pstr : int -> pstring -> pstring = fun cnt (pos, str) ->
  let len = String.length str in
  pos, if cnt >= len then "" else String.sub str cnt (len - cnt)

(* Context of the file being parsed, as global state. *)
type state_variables =
  { language  : string ref
  ; filePath  : Relative_path.t ref
  ; mode      : FileInfo.mode ref
  ; popt      : ParserOptions.t ref
  ; ignorePos : bool ref
  ; quickMode : bool ref
  ; suppress_output : bool ref
  }

let lowerer_state =
  { language  = ref "UNINITIALIZED"
  ; filePath  = ref Relative_path.default
  ; mode      = ref FileInfo.Mstrict
  ; popt      = ref ParserOptions.default
  ; ignorePos = ref false
  ; quickMode = ref false
  ; suppress_output = ref false
  }

let php_file () = !(lowerer_state.mode) == FileInfo.Mphp

(* "Local" context. *)
type env =
  { saw_yield : bool ref
  ; errors    : (Pos.t * string) list ref
  ; max_depth : int
  }

type +'a parser = node -> env -> 'a
type ('a, 'b) metaparser = 'a parser -> 'b parser

let get_pos : node -> Pos.t = fun node ->
  if !(lowerer_state.ignorePos)
  then Pos.none
  else begin
    let pos_file = !(lowerer_state.filePath) in
    let text = source_text node in
    (* TODO(8086635) Figure out where this off-by-one comes from *)
    let start_offset =
      (*should be removed after resolving TODO above *)
      let start_offset = start_offset node in
      if start_offset = 0 then 0 else start_offset - 1
    in
    let end_offset = end_offset node in
    let s_line, s_column = SourceText.offset_to_position text start_offset in
    let e_line, e_column = SourceText.offset_to_position text end_offset in
    let pos_start =
      File_pos.of_line_column_offset
        ~line:s_line
        ~column:s_column
        ~offset:start_offset
    in
    let pos_end =
      File_pos.of_line_column_offset
        ~line:e_line
        ~column:e_column
        ~offset:end_offset
    in
    Pos.make_from_file_pos ~pos_file ~pos_start ~pos_end
  end

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
  if not !(lowerer_state.suppress_output) then
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




let pos_name node =
  let name = text node in
  let local_ignore_pos = !(lowerer_state.ignorePos) in
  (* Special case for __LINE__; never ignore position for that special name *)
  if name = "__LINE__" then lowerer_state.ignorePos := false;
  let p = get_pos node in
  lowerer_state.ignorePos := local_ignore_pos;
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

let missing =
  let module V = Full_fidelity_positioned_syntax.PositionedSyntaxValue in
  let open Full_fidelity_source_text in
  { syntax = Missing
  ; value =
    { V.source_text    = { text = ""; offset_map = Line_break_map.make "" }
    ; V.offset         = 0
    ; V.leading_width  = 0
    ; V.width          = 0
    ; V.trailing_width = 0
    }
  }



let token_kind : node -> TK.t option = function
  | { syntax = Token t; _ } -> Some (PT.kind t)
  | _ -> None

let pBop : (expr -> expr -> expr_) parser = fun node env lhs rhs ->
  match token_kind node with
  | Some TK.Equal                       -> Binop (Eq None,           lhs, rhs)
  | Some TK.Bar                         -> Binop (Bar,               lhs, rhs)
  | Some TK.Ampersand                   -> Binop (Amp,               lhs, rhs)
  | Some TK.Plus                        -> Binop (Plus,              lhs, rhs)
  | Some TK.Minus                       -> Binop (Minus,             lhs, rhs)
  | Some TK.Star                        -> Binop (Star,              lhs, rhs)
  | Some TK.Or                          -> Binop (BArbar,            lhs, rhs)
  | Some TK.And                         -> Binop (AMpamp,            lhs, rhs)
  | Some TK.Xor                         -> Binop (LogXor,            lhs, rhs)
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
  (* The ugly ducklings; In the FFP, `|>` and '??' are parsed as
   * `BinaryOperator`s, whereas the typed AST has separate constructors for
   * NullCoalesce, Pipe and Binop. This is why we don't just project onto a
   * `bop`, but a `expr -> expr -> expr_`.
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
  | Some TK.Var       -> Public
  | _ -> missing_syntax "kind" node env
  )

let syntax_of_token : PositionedToken.t -> node = fun t ->
  { syntax = Token t
  ; value  = PositionedSyntaxValue.make t.PT.source_text
               t.PT.offset t.PT.leading_width t.PT.width t.PT.trailing_width
  }

(* TODO: Clean up string escaping *)
let prepString2 : node list -> node list =
  let trimLeft ~n t =
    PT.({ t with leading_width = t.leading_width + n; width = t.width - n })
  in
  let trimRight ~n t =
    PT.({ t with trailing_width = t.trailing_width + n; width = t.width - n })
  in
  function
  | ({ syntax = Token t; _ }::ss)
  when t.PT.width > 0 && (PT.text t).[0] = '"' ->
    let rec unwind = function
      | [{ syntax = Token t; _ }]
      when t.PT.width > 0 && (PT.text t).[t.PT.width - 1] = '"' ->
        let s = syntax_of_token (trimRight ~n:1 t) in
        if width s > 0 then [s] else []
      | x :: xs -> x :: unwind xs
      | _ -> raise (Invalid_argument "Malformed String2 SyntaxList")
    in
    let s = syntax_of_token (trimLeft ~n:1 t) in
    if width s > 0 then s :: unwind ss else unwind ss
  | ({ syntax = Token t; _ }::ss)
  when t.PT.width > 3 && String.sub (PT.text t) 0 3 = "<<<" ->
    let rec unwind = function
      | [{ syntax = Token t; _ }] when t.PT.width > 0 ->
        let content = PT.text t in
        let len = t.PT.width in
        let n = len - (String.rindex_from content (len - 2) '\n') in
        let s = syntax_of_token (trimRight ~n t) in
        if width s > 0 then [s] else []
      | x :: xs -> x :: unwind xs
      | _ -> raise (Invalid_argument "Malformed String2 SyntaxList")
    in
    let content = PT.text t in
    let n = (String.index content '\n') + 1 in
    let s = syntax_of_token (trimLeft ~n t) in
    if width s > 0 then s :: unwind ss else unwind ss
  | x -> x (* unchanged *)

let mkStr : (string -> string) -> string -> string = fun unescaper content ->
  let no_quotes = try
      if String.sub content 0 3 = "<<<" (* The heredoc case *)
      then
        (* These types of strings begin with an opening line containing <<<
         * followed by a string to use as a terminator (which is optionally
         * quoted) and end with a line containing only the terminator and a
         * semicolon followed by a blank line. We need to drop the opening
         * line as well as the blank line and preceding terminator line. *)
        let len = String.length content in
        let start = (String.index content '\n') + 1 in
        let end_ = (String.rindex_from content (len - 2) '\n') in
        let len = end_ - start in
          String.sub content start len
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

type suspension_kind =
  | SKSync
  | SKAsync
  | SKCoroutine

let mk_suspension_kind async_node coroutine_node =
  match is_missing async_node, is_missing coroutine_node with
  | true, true -> SKSync
  | false, true -> SKAsync
  | true, false -> SKCoroutine
  | false, false -> raise (Failure "Couroutine functions may not be async")

let mk_fun_kind suspension_kind yield =
  match suspension_kind, yield with
  | SKSync,  true  -> FGenerator
  | SKAsync, true  -> FAsyncGenerator
  | SKSync,  false -> FSync
  | SKAsync, false -> FAsync
  (* TODO(t17335630): Implement an FCoroutine fun_kind *)
  | SKCoroutine, false -> assert false
  | SKCoroutine, true -> raise (Failure "Couroutine functions may not yield")

let fun_template yielding node suspension_kind =
  let p = get_pos node in
  { f_mode            = !(lowerer_state.mode)
  ; f_tparams         = []
  ; f_constrs         = []
  ; f_ret             = None
  ; f_ret_by_ref      = false
  ; f_name            = p, ";anonymous"
  ; f_params          = []
  ; f_body            = []
  ; f_user_attributes = []
  ; f_fun_kind        = mk_fun_kind suspension_kind yielding
  ; f_namespace       = Namespace_env.empty !(lowerer_state.popt)
  ; f_span            = p
  ; f_doc_comment     = None
  ; f_static          = false
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

let pShapeFieldName : shape_field_name parser = fun name _env ->
  match syntax name with
  | ScopeResolutionExpression
    { scope_resolution_qualifier; scope_resolution_name; _ } ->
      SFclass_const
      ( pos_name scope_resolution_qualifier
      , pos_name scope_resolution_name
      )
  | _ -> let p, n = pos_name name in SFlit (p, mkStr unesc_dbl n)

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

let rec pHint : hint parser = fun node env ->
  let rec pHint_ : hint_ parser = fun node env ->
    match syntax node with
    (* Dirty hack; CastExpression can have type represented by token *)
    | Token _
    | SimpleTypeSpecifier _
      -> Happly (pos_name node, [])
    | ShapeTypeSpecifier { shape_type_fields; shape_type_ellipsis; _ } ->
      let si_allows_unknown_fields = not (is_missing shape_type_ellipsis) in
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
      -> Happly (pos_name kw, couldMap ~f:pHint ty env)

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
        ( pos_name kw
        , List.map ~f:(fun x -> pHint x env) [ key; value ]
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
    | SoftTypeSpecifier { soft_type; _ } ->
      Hsoft (pHint soft_type env)
    | ClosureTypeSpecifier { closure_parameter_types; closure_return_type; _} ->
      let param_types =
        List.map ~f:(fun x -> pHint x env)
          (as_list closure_parameter_types)
      in
      let is_variadic_param x = snd x = Htuple [] in
      Hfun
      ( List.filter ~f:(fun x -> not (is_variadic_param x)) param_types
      , List.exists ~f:is_variadic_param param_types
      , pHint closure_return_type env
      )
    | TypeConstant { type_constant_left_type; type_constant_right_type; _ } ->
      let child = pos_name type_constant_right_type in
      (match pHint_ type_constant_left_type env with
      | Haccess (b, c, cs) -> Haccess (b, c, cs @ [child])
      | Happly (b, []) -> Haccess (b, child, [])
      | _ -> missing_syntax "type constant base" node env
      )
    | VariadicParameter _ ->
      (* Clever trick warning: empty tuple types indicating variadic params *)
      Htuple []
    | _ -> missing_syntax "type hint" node env
  in
  get_pos node, pHint_ node env

type fun_hdr =
  { fh_suspension_kind : suspension_kind
  ; fh_name            : pstring
  ; fh_type_parameters : tparam list
  ; fh_parameters      : fun_param list
  ; fh_return_type     : hint option
  ; fh_param_modifiers : fun_param list
  ; fh_ret_by_ref      : bool
  }

let empty_fun_hdr =
  { fh_suspension_kind = SKSync
  ; fh_name            = Pos.none, "<ANONYMOUS>"
  ; fh_type_parameters = []
  ; fh_parameters      = []
  ; fh_return_type     = None
  ; fh_param_modifiers = []
  ; fh_ret_by_ref      = false
  }

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
    ; param_id              = pos_name name
    ; param_expr            =
      mpOptional pSimpleInitializer parameter_default_value env
    ; param_user_attributes = List.concat @@
      couldMap ~f:pUserAttribute parameter_attribute env
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
and pString2: node list -> env -> expr list =
  let rec aux l env acc =
    (* in PHP "${x}" in strings is treated as if it was written "$x",
       here we recognize pattern: Dollar; EmbeddedBracedExpression { QName (Token.Name) }
       produced by FFP and lower it into Lvar.
    *)
    match l with
    | [] -> List.rev acc
    | ({ syntax = Token { PT.kind = TK.Dollar; _ }; _ } as l)::
      ({ syntax = EmbeddedBracedExpression {
          embedded_braced_expression_expression = {
            syntax = QualifiedNameExpression {
              qualified_name_expression = {
                syntax = Token { PT.kind = TK.Name; _ }
                ; _ } as name
              ; _ }
            ; _ }
          ; _ }
       ; _ } as r)
      ::tl ->
        let pos, name = pos_name name in
        let id = Lvar (pos, "$" ^ name) in
        let left_pos = get_pos l in
        let right_pos = get_pos r in
        let pos =
          if left_pos = Pos.none || right_pos = Pos.none then Pos.none
          else
            (* build final position as:
            start_pos = start position of Dollar token
            end_pos = end position pof embedded brace expression *)
            let pos_file = Pos.filename left_pos in
            let pos_start = Pos.pos_start left_pos in
            let pos_end = Pos.pos_end right_pos in
            Pos.make_from_file_pos ~pos_file ~pos_start ~pos_end
        in
        aux tl env ((pos, id)::acc)
    | x::xs -> aux xs env ((pExpr ~top_level:false x env)::acc)
  in
  fun l env -> aux l env []
and pExpr ?top_level:(top_level=true) : expr parser = fun node env ->
  let rec pExpr_ : expr_ parser = fun node env ->
    let pos = get_pos node in
    match syntax node with
    | LambdaExpression {
        lambda_async; lambda_coroutine; lambda_signature; lambda_body; _ } ->
      let suspension_kind =
        mk_suspension_kind lambda_async lambda_coroutine in
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
      { (fun_template yield node suspension_kind) with f_ret; f_params; f_body }

    | BracedExpression        { braced_expression_expression        = expr; _ }
    | EmbeddedBracedExpression
      { embedded_braced_expression_expression = expr; _ }
    | ParenthesizedExpression { parenthesized_expression_expression = expr; _ }
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

    | VarrayIntrinsicExpression { varray_intrinsic_members = members; _ } ->
      Varray (couldMap ~f:pExpr members env)
    | DarrayIntrinsicExpression { darray_intrinsic_members = members; _ } ->
      let pMember node env =
        match syntax node with
        | ElementInitializer { element_key; element_value; _ } ->
          (pExpr element_key env, pExpr element_value env)
        | _ -> missing_syntax "darray intrinsic expression element" node env
      in
      Darray (couldMap ~f:pMember members env)
    | ArrayIntrinsicExpression { array_intrinsic_members = members; _ }
    | ArrayCreationExpression  { array_creation_members  = members; _ }
    ->
      (* TODO: Or tie in with other intrinsics and post-process to Array *)
      Array (couldMap ~f:pAField members env)

    | ListExpression { list_members = members; _ } ->
      (* TODO: Or tie in with other intrinsics and post-process to List *)
      let pBinderOrIgnore node env =
        Option.value ~default:(Pos.none, Omitted) @@ mpOptional pExpr node env
      in
      List (couldMap ~f:pBinderOrIgnore members env)

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
      -> Call (pExpr recv env, [], couldMap ~f:pExpr args  env, [])

    | QualifiedNameExpression { qualified_name_expression } ->
      Id (pos_name qualified_name_expression)
    | VariableExpression { variable_expression } ->
      Lvar (pos_name variable_expression)

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
      -> Obj_get (pExpr recv env, pExpr name env, pNullFlavor op env)

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
        (match token_kind operator with
        | Some TK.PlusPlus   when postfix -> Unop (Upincr, expr)
        | Some TK.MinusMinus when postfix -> Unop (Updecr, expr)
        | Some TK.PlusPlus                -> Unop (Uincr,  expr)
        | Some TK.MinusMinus              -> Unop (Udecr,  expr)
        | Some TK.Exclamation             -> Unop (Unot,   expr)
        | Some TK.Tilde                   -> Unop (Utild,  expr)
        | Some TK.Plus                    -> Unop (Uplus,  expr)
        | Some TK.Minus                   -> Unop (Uminus, expr)
        | Some TK.Ampersand               -> Unop (Uref,   expr)
        | Some TK.DotDotDot               -> Unop (Usplat, expr)
        | Some TK.At                      -> Unop (Usilence, expr)
        | Some TK.Await                   -> Await expr
        | Some TK.Clone                   -> Clone expr
        | Some TK.Print                   ->
          Call ((pos, Id (pos, "echo")), [], [expr], [])
        | Some TK.Dollar                  ->
          (match snd expr with
          | Lvarvar (n, id) -> Lvarvar (n + 1, id)
          | Lvar id         -> Lvarvar (1, id)
          | _ -> BracedExpr expr
          )
        | _ -> missing_syntax "unary operator" node env
        )
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
    | YieldExpression { yield_operand; _ } when is_missing yield_operand ->
      env.saw_yield := true;
      Yield (AFvalue (pos, Null))
    | YieldExpression { yield_operand; _ } ->
      env.saw_yield := true;
      Yield (pAField yield_operand env)
    | YieldFromExpression { yield_from_operand; _ } ->
      env.saw_yield := true;
      Yield_from (pExpr yield_from_operand env)

    | DefineExpression { define_keyword; define_argument_list; _ } -> Call
      ( (let name = pos_name define_keyword in fst name, Id name)
      , []
      , List.map ~f:(fun x -> pExpr x env) (as_list define_argument_list)
      , []
      )

    | ScopeResolutionExpression
      { scope_resolution_qualifier; scope_resolution_name; _ } ->
      let qual = pos_name scope_resolution_qualifier in
      begin match syntax scope_resolution_name with
      | Token { PositionedToken.kind = TK.Variable; _ } ->
        let name =
          get_pos scope_resolution_name, Lvar (pos_name scope_resolution_name)
        in
        Class_get (qual, name)
      | _ ->
        let name = pExpr scope_resolution_name env in
        begin match snd name with
        | Id id -> Class_const (qual, id)
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
      , mpOptional pExpr embedded_subscript_index env
      )
    | ShapeExpression { shape_expression_fields; _ } ->
      Shape (
        couldMap ~f:(mpShapeExpressionField pExpr) shape_expression_fields env
      )
    | ObjectCreationExpression
      { object_creation_type; object_creation_argument_list; _ } ->
      New
      ( (match syntax object_creation_type with
        | GenericTypeSpecifier { generic_class_type; generic_argument_list } ->
          let name = pos_name generic_class_type in
          let hints =
            match syntax generic_argument_list with
            | TypeArguments { type_arguments_types; _ }
              -> couldMap ~f:pHint type_arguments_types env
            | _ ->
              missing_syntax "generic type arguments" generic_argument_list env
          in
          fst name, Id_type_arguments (name, hints)
        | QualifiedNameExpression _
        | SimpleTypeSpecifier _
        | Token _
          -> let name = pos_name object_creation_type in fst name, Id name
        | _ -> pExpr object_creation_type env
        )
      , couldMap ~f:pExpr object_creation_argument_list env
      , []
      )
    | GenericTypeSpecifier
      { generic_class_type
      ; generic_argument_list
      } ->
        let name = pos_name generic_class_type in
        let hints =
          match syntax generic_argument_list with
          | TypeArguments { type_arguments_types; _ }
            -> couldMap ~f:pHint type_arguments_types env
          | _ ->
            missing_syntax "generic type arguments" generic_argument_list env
        in
        Id_type_arguments (name, hints)
    | LiteralExpression { literal_expression = expr } ->
      (match syntax expr with
      | Token _ ->
        let s = text expr in
        (* We allow underscores while lexing the integer literals. This function gets
         * rid of them before the literal is created. *)
        let eliminate_underscores s = s
                                      |> Str.split (Str.regexp "_")
                                      |> String.concat "" in
        (* TODO(17796330): Get rid of linter functionality in the lowerer *)
        if s <> String.lowercase s then Lint.lowercase_constant pos s;
        (match token_kind expr with
        | Some TK.DecimalLiteral
        | Some TK.OctalLiteral
        | Some TK.HexadecimalLiteral
        | Some TK.BinaryLiteral             -> Int    (pos, eliminate_underscores s)
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

      | SyntaxList ts -> String2 (pString2 (prepString2 ts) env)
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
      { anonymous_static_keyword
      ; anonymous_async_keyword
      ; anonymous_coroutine_keyword
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
        let suspension_kind =
          mk_suspension_kind
            anonymous_async_keyword
            anonymous_coroutine_keyword in
        let body, yield = mpYielding pBlock anonymous_body env in
        Efun
        ( { (fun_template yield node suspension_kind) with
            f_ret    = mpOptional pHint anonymous_type env
          ; f_params = couldMap ~f:pFunParam anonymous_parameters env
          ; f_body   = mk_noop body
          ; f_static = not (is_missing anonymous_static_keyword)
          }
        , try pUse anonymous_use env with _ -> []
        )

    | AwaitableCreationExpression
      { awaitable_async; awaitable_coroutine; awaitable_compound_statement } ->
      let suspension_kind =
        mk_suspension_kind awaitable_async awaitable_coroutine in
      let blk, yld = mpYielding pBlock awaitable_compound_statement env in
      let body =
        { (fun_template yld node suspension_kind) with f_body = mk_noop blk }
      in
      Call ((get_pos node, Lfun body), [], [], [])
    | XHPExpression
      { xhp_open =
        { syntax = XHPOpen { xhp_open_name; xhp_open_attributes; _ }; _ }
      ; xhp_body = body
      ; _ } ->
      lowerer_state.ignorePos := false;
      let name =
        let pos, name = pos_name xhp_open_name in
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
        | t :: xs when token_kind t = Some TK.XHPComment -> search xs
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
      , List.map ~f:(fun x -> pEmbedded unesc_xhp x env)
          (aggregate_tokens body)
      )
    (* FIXME; should this include Missing? ; "| Missing -> Null" *)
    | _ -> missing_syntax "expression" node env
  in
  (* Since we need positions in XHP, regardless of the ignorePos flag, we
   * parenthesise the call to pExpr_ so that the XHP expression case can flip
   * the switch. The key part is that `get_pos node` happens before the old
   * setting is restored.
   *
   * Evaluation order matters here!
   *)
  let local_ignore_pos = !(lowerer_state.ignorePos) in
  let expr_ = pExpr_ node env in
  let p = get_pos node in
  lowerer_state.ignorePos := local_ignore_pos;
  p, expr_
and pBlock : block parser = fun node env ->
   match pStmt node env with
   | Block block -> List.filter ~f:(fun x -> x <> Noop) block
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
    , List.concat @@ couldMap ~f:pSwitchSection switch_sections env
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
    , List.fold_right ~f:(@@)
        (couldMap ~f:pElseIf if_elseif_clauses env)
        ~init:[ match syntax if_else_clause with
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
    Block (List.filter ~f:(fun x -> x <> Noop) @@
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
  | Full_fidelity_positioned_syntax.GotoLabel { goto_label_name; _ } ->
    let pos = get_pos goto_label_name in
    let label_name = text goto_label_name in
    Ast.GotoLabel (pos, label_name)
  | GotoStatement { goto_statement_label_name; _ } ->
    Goto  (pos_name goto_statement_label_name)
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
        , []
        , couldMap ~f:pExpr exprs env
        , []
      ))
  | BreakStatement { break_level=level; _ } ->
    Break (get_pos node, pBreak_or_continue_level env level)
  | ContinueStatement { continue_level=level; _ } ->
    Continue (get_pos node, pBreak_or_continue_level env level)
  | GlobalStatement { global_variables; _ } ->
    Global_var (couldMap ~f:pExpr global_variables env)
  | MarkupSection _ -> pMarkup node env
  | _ when env.max_depth > 0 ->
    (* OCaml optimisers; Forgive them, for they know not what they do!
     *
     * The max_depth is only there to stop the *optimised* version from an
     * unbounded recursion. Sad times.
     *)
    Def_inline (pDef node { env with max_depth = env.max_depth - 1 })
  | _ -> missing_syntax "statement" node env

and pMarkup node env =
  match syntax node with
  | MarkupSection { markup_text; markup_expression; _ } ->
    let expr =
      match syntax markup_expression with
      | Missing -> None
      | ExpressionStatement {
          expression_statement_expression = e
        ; _} -> Some (pExpr e env)
      | _ -> failwith "expression expected"
    in
    Markup ((get_pos node, text markup_text), expr)
  | _ -> failwith "invalid node"

and pBreak_or_continue_level env level =
  match mpOptional pExpr level env with
  | Some (_, Int(_, s)) -> Some (int_of_string s)
  | _ -> None

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

(* TODO: Translate the where clause *)
and pFunHdr : fun_hdr parser = fun node env ->
  match syntax node with
  | FunctionDeclarationHeader
    { function_async
    ; function_coroutine
    ; function_ampersand
    ; function_name
    ; function_type_parameter_list
    ; function_parameter_list
    ; function_type
    ; _ } ->
      let fh_parameters = couldMap ~f:pFunParam function_parameter_list env in
      let fh_return_type = mpOptional pHint function_type env in
      let fh_suspension_kind =
        mk_suspension_kind function_async function_coroutine in
      let fh_name = pos_name function_name in
      let fh_type_parameters = pTParaml function_type_parameter_list env in
      let fh_param_modifiers =
        List.filter ~f:(fun p -> Option.is_some p.param_modifier) fh_parameters
      in
      let fh_ret_by_ref = is_ret_by_ref function_ampersand in
      { fh_suspension_kind
      ; fh_name
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
          | `MaybeDoc2, _               -> go start `DocComment  next
          | `DocComment, '*'            -> go start `EndDoc      next
          | `DocComment, _              -> go start `DocComment  next
          | `EndDoc, _                  -> go start `DocComment  next
          (* A * without a / does not end an embedded comment *)
          | `EmbeddedCmt, '*'           -> go start `EndEmbedded next
          | `EndEmbedded, '*'           -> go start `EndEmbedded next
          | `EndEmbedded,  _            -> go start `EmbeddedCmt next
          (* Whitespace skips everywhere else *)
          | _, (' ' | '\t' | '\n')      -> go start state        next
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

and pClassElt : class_elt list parser = fun node env ->
  let opt_doc_comment = extract_docblock node in
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
        List.unzip @@
          List.filter_map hdr.fh_parameters ~f:(fun p ->
            Option.map ~f: (fun _ -> classvar_init p) p.param_modifier
          )
      in
      let pBody = fun node env ->
        if is_missing node then [] else
          (* TODO: Give parse error when not abstract, but body is missing *)
          List.rev member_init @
          match pStmt node env with
          | Block []    -> [Noop]
          | Block stmtl -> stmtl
          | stmt        -> [stmt]
      in
      let body, body_has_yield = mpYielding pBody methodish_function_body env in
      let body =
        (* Drop it on the floor in quickMode; we still need to process the body
         * to know, e.g. whether it contains a yield.
         *)
        if !(lowerer_state.quickMode)
        then [Noop]
        else body
      in
      let kind = pKinds methodish_modifiers env in
      member_def @ [Method
      { m_kind            = kind
      ; m_tparams         = hdr.fh_type_parameters
      ; m_constrs         = []
      ; m_name            = hdr.fh_name
      ; m_params          = hdr.fh_parameters
      ; m_body            = body
      ; m_user_attributes = List.concat @@
        couldMap ~f:pUserAttribute methodish_attribute env
      ; m_ret             = hdr.fh_return_type
      ; m_ret_by_ref      = hdr.fh_ret_by_ref
      ; m_span            = get_pos node
      ; m_fun_kind        = mk_fun_kind hdr.fh_suspension_kind body_has_yield
      ; m_doc_comment     = opt_doc_comment
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
            pos_name scope_resolution_qualifier,
            pos_name scope_resolution_name
          | _ -> missing_syntax "trait use precedence item" node env
        in
        let removed_names =
          couldMap ~f:(fun n _e -> pos_name n) removed_names env
        in
        ClassUsePrecedence (qualifier, name, removed_names)
      | TraitUseAliasItem
        { trait_use_alias_item_aliasing_name = aliasing_name
        ; trait_use_alias_item_visibility = visibility
        ; trait_use_alias_item_aliased_name = aliased_name
        ; _
        } ->
        let qualifier, name =
          match syntax aliasing_name with
          | ScopeResolutionExpression
            { scope_resolution_qualifier; scope_resolution_name; _ } ->
            Some (pos_name scope_resolution_qualifier),
            pos_name scope_resolution_name
          | _ -> None, pos_name aliasing_name
        in
        let visibility = Option.map (token_kind visibility)
          ~f:begin function
          | TK.Private   -> Private
          | TK.Public    -> Public
          | TK.Protected -> Protected
          | _ -> missing_syntax "trait use alias item" node env end
        in
        let aliased_name =
          if is_missing aliased_name then None else Some (pos_name aliased_name)
        in
        ClassUseAlias (qualifier, name, aliased_name, visibility)
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
  | XHPChildrenDeclaration { xhp_children_expression; _; } ->
    [ XhpChild (pXhpChild xhp_children_expression env) ]
  | XHPCategoryDeclaration { xhp_category_categories = cats; _ } ->
    let pNameSansPercent node _env = drop_pstr 1 (pos_name node) in
    [ XhpCategory (couldMap ~f:pNameSansPercent cats env) ]
  | _ -> missing_syntax "expression" node env

and pXhpChild : xhp_child parser = fun node env ->
  match syntax node with
  | Token t -> ChildName (pos_name node)
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
    ; _ } ->
    let p, n as name =
      match prefix, pos_name name with
      | None, (p, n) -> (p, n)
      | Some prefix, (p, n) -> p, (snd @@ pos_name prefix) ^ n
    in
    let x = Str.search_forward (Str.regexp "[^\\\\]*$") n 0 in
    let key = drop_pstr x name in
    let kind =
      match syntax kind with
      | Token { PT.kind = TK.Namespace; _ } -> NSNamespace
      | Token { PT.kind = TK.Type     ; _ } -> NSClass
      | Token { PT.kind = TK.Function ; _ } -> NSFun
      | Token { PT.kind = TK.Const    ; _ } -> NSConst
      | Missing                             -> NSClassAndNamespace
      | _ -> missing_syntax "namespace use kind" kind env
    in
    ( kind
    , (p, if n.[0] = '\\' then n else "\\" ^ n)
    , if is_missing alias
      then key
      else pos_name alias
    )
  | _ -> missing_syntax "namespace use clause" node env

and pDef : def parser = fun node env ->
  let opt_doc_comment = extract_docblock node in
  match syntax node with
  | FunctionDeclaration
    { function_attribute_spec; function_declaration_header; function_body } ->
      let hdr = pFunHdr function_declaration_header env in
      let block, yield = mpYielding (mpOptional pBlock) function_body env in
      Fun
      { (fun_template yield node hdr.fh_suspension_kind) with
        f_tparams         = hdr.fh_type_parameters
      ; f_ret             = hdr.fh_return_type
      ; f_name            = hdr.fh_name
      ; f_params          = hdr.fh_parameters
      ; f_ret_by_ref      = hdr.fh_ret_by_ref
      ; f_body            =
        if !(lowerer_state.quickMode)
        then [Noop]
        else begin
          (* FIXME: Filthy hack to catch UNSAFE *)
          let containsUNSAFE =
            let re = Str.regexp_string "UNSAFE" in
            try Str.search_forward re (full_text function_body) 0 >= 0 with
            | Not_found -> false
          in
          match block with
          | Some [Noop] when containsUNSAFE -> [Unsafe]
          | Some [] -> [Noop]
          | None -> []
          | Some b -> b
        end
      ; f_user_attributes =
        List.concat @@ couldMap ~f:pUserAttribute function_attribute_spec env
      ; f_doc_comment = opt_doc_comment
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
      ; c_user_attributes = List.concat @@ couldMap ~f:pUserAttribute attr env
      ; c_final           = List.mem (pKinds mods env) Final
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
      ; c_namespace       = Namespace_env.empty !(lowerer_state.popt)
      ; c_enum            = None
      ; c_span            = get_pos node
      ; c_kind            = begin
        let is_abs = Str.(string_match (regexp ".*abstract.*") (text mods) 0) in
        match token_kind kw with
        | Some TK.Class when is_abs -> Cabstract
        | Some TK.Class             -> Cnormal
        | Some TK.Interface         -> Cinterface
        | Some TK.Trait             -> Ctrait
        | Some TK.Enum              -> Cenum
        | _ -> missing_syntax "class kind" kw env
        end
      ; c_doc_comment = opt_doc_comment
      }
  | ConstDeclaration
    { const_type_specifier = ty
    ; const_declarators    = decls
    ; _ } ->
      (match List.map ~f:syntax (as_list decls) with
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
          ; cst_namespace = Namespace_env.empty !(lowerer_state.popt)
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
      ; t_user_attributes = List.concat @@
          List.map ~f:(fun x -> pUserAttribute x env) (as_list attr)
      ; t_namespace       = Namespace_env.empty !(lowerer_state.popt)
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
      ; c_user_attributes = List.concat @@ couldMap ~f:pUserAttribute attrs env
      ; c_final           = false
      ; c_kind            = Cenum
      ; c_is_xhp          = false
      ; c_name            = pos_name name
      ; c_tparams         = []
      ; c_extends         = []
      ; c_implements      = []
      ; c_body            = couldMap enums env ~f:pEnumerator
      ; c_namespace       = Namespace_env.empty !(lowerer_state.popt)
      ; c_span            = get_pos node
      ; c_enum            = Some
        { e_base       = pHint base env
        ; e_constraint = mpOptional pTConstraintTy constr env
        }
      ; c_doc_comment = opt_doc_comment
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
      , List.map ~f:(fun x -> pDef x env) (as_list decls)
      )
  | NamespaceDeclaration { namespace_name = name; _ } ->
    Namespace (pos_name name, [])
  | NamespaceGroupUseDeclaration
    { namespace_group_use_kind = kind
    ; namespace_group_use_prefix = prefix
    ; namespace_group_use_clauses = clauses
    ; _ } ->
      let f = pNamespaceUseClause env kind ~prefix:(Some prefix) in
      NamespaceUse (List.map ~f (as_list clauses))
  | NamespaceUseDeclaration
    { namespace_use_kind    = kind
    ; namespace_use_clauses = clauses
    ; _ } ->
      let f = pNamespaceUseClause env kind ~prefix:None in
      NamespaceUse (List.map ~f (as_list clauses))
  (* Fail open, assume top-level statement. Not too nice when reporting bugs,
   * but if this turns out prohibitive, just `try` this and catch-and-correct
   * the raised exception.
   *)
  | _ -> Stmt (pStmt node env)
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
        , []
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
        ; cst_namespace = Namespace_env.empty !(lowerer_state.popt)
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
        { syntax = DefineExpression
          { define_keyword; define_argument_list = args; _ }
        ; _ }
      ; _ }
    ; _ } :: nodel ->
      ( match List.map ~f:(fun x -> pExpr x env) (as_list args) with
      | [ _, String name; e ] -> Constant
        { cst_mode      = !(lowerer_state.mode)
        ; cst_kind      = Cst_define
        ; cst_name      = name
        ; cst_type      = None
        ; cst_value     = e
        ; cst_namespace = Namespace_env.empty !(lowerer_state.popt)
        }
      | args ->
        let name = pos_name define_keyword in
        Stmt (Expr (fst name, Call ((fst name, Id name), [], args, [])))
      ) :: aux env nodel
  | node :: nodel -> pDef node env :: aux env nodel
  in
  let nodes = as_list node in
  let nodes = aux env nodes in
  post_process nodes

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

exception Malformed_trivia of int

type scoured_comment = Pos.t * comment
type scoured_comments = scoured_comment list

let scour_comments
  (path        : Relative_path.t)
  (source_text : Full_fidelity_source_text.t)
  (tree        : node)
  : scoured_comments =
    let pos_of_offset =
      Full_fidelity_source_text.relative_pos path source_text
    in
    let parse
      (acc : scoured_comments)
      (offset : int)
      (str : string)
      : scoured_comments =
        let fail state n =
          let state =
            match state with
            | `Free        -> "Free"
            | `LineCmt     -> "LineCmt"
            | `SawSlash    -> "SawSlash"
            | `EmbeddedCmt -> "EmbeddedCmt"
            | `EndEmbedded -> "EndEmbedded"
          in
          if not !(lowerer_state.suppress_output) then
            Printf.eprintf "Error parsing trivia in state %s: '%s'\n" state str;
          raise (Malformed_trivia n)
        in
        let length = String.length str in
        let mk tag (start : int) (end_plus_one : int) acc : scoured_comments =
          match tag with
          | `Line ->
            (* Correct for the offset of the comment in the file *)
            let start = offset + start in
            let end_ = offset + end_plus_one in
            let (p, c) as result =
              Full_fidelity_source_text.
              ( pos_of_offset start end_
              , CmtLine (sub source_text start (end_ - start))
              )
            in
            result :: acc
          | `Block ->
            (* Correct for the offset of the comment in the file *)
            let start = offset + start in
            let end_ = offset + end_plus_one - 1 in
            let (p, c) as result =
              Full_fidelity_source_text.
              (* Should be 'start end_', but keeping broken for fidelity. *)
              ( pos_of_offset (end_) (end_ + 1)
              , CmtBlock (sub source_text start (end_ - start))
              )
            in
            result :: acc
        in
        let rec go start state idx : scoured_comments =
          if idx = length (* finished? *)
          then begin
            match state with
            | `Free -> acc
            | `LineCmt -> mk `Line start length acc
            | _        -> fail state start
          end else begin
            let next = idx + 1 in
            match state, str.[idx] with
            (* Ending comments produces the comment just scanned *)
            | `LineCmt,     '\n' -> mk `Line  start idx @@ go next `Free next
            | `EndEmbedded, '/'  -> mk `Block start idx @@ go next `Free next
            (* PHP has line comments delimited by a # *)
            | `Free,     '#'              -> go next `LineCmt      next
            (* All other comment delimiters start with a / *)
            | `Free,     '/'              -> go start `SawSlash    next
            (* After a / in trivia, we must see either another / or a * *)
            | `SawSlash, '/'              -> go next  `LineCmt     next
            | `SawSlash, '*'              -> go next  `EmbeddedCmt next
            (* A * without a / does not end an embedded comment *)
            | `EmbeddedCmt, '*'           -> go start `EndEmbedded next
            | `EndEmbedded, '*'           -> go start `EndEmbedded next
            | `EndEmbedded,  _            -> go start `EmbeddedCmt next
            (* Whitespace skips everywhere else *)
            | _, (' ' | '\t' | '\n')      -> go start state        next
            (* When scanning comments, anything else is accepted *)
            | `LineCmt,     _             -> go start state        next
            | `EmbeddedCmt, _             -> go start state        next
            (* Anything else; bail *)
            | _ -> fail state start
          end
        in
        go 0 `Free 0
    in (* Now that we have a parser *)
    let rec aux (acc : scoured_comments) node : scoured_comments =
      match syntax node with
      | Token _ ->
        let acc = parse acc (leading_start_offset node) (leading_text node) in
        parse acc (trailing_start_offset node) (trailing_text node)
      | _ -> List.fold_left ~f:aux ~init:acc (children node)
    in
    aux [] tree

(*****************************************************************************(
 * Front-end matter
)*****************************************************************************)

type result =
  { fi_mode  : FileInfo.mode
  ; ast      : Ast.program
  ; content  : string
  ; file     : Relative_path.t
  ; comments : (Pos.t * comment) list
  }

let lower
  ?(elaborate_namespaces  = true)
  ?(include_line_comments = false)
  ?(keep_errors           = true)
  ?(ignore_pos            = false)
  ?(quick                 = false)
  ?(suppress_output       = false)
  ?(parser_options        = ParserOptions.default)
  ~content
  ~language
  ~file
  ~fi_mode
  ~source_text
  ~script
  : result =
    lowerer_state.language  := language;
    lowerer_state.filePath  := file;
    lowerer_state.mode      := fi_mode;
    lowerer_state.popt      := parser_options;
    lowerer_state.ignorePos := ignore_pos;
    lowerer_state.quickMode := quick;
    lowerer_state.suppress_output := suppress_output;
    let saw_yield = ref false in
    let errors = ref [] in (* The top-level error list. *)
    let max_depth = 42 in (* Filthy hack around OCaml bug *)
    let ast = runP pScript script { saw_yield; errors; max_depth } in
    let ast =
      if elaborate_namespaces
      then Namespaces.elaborate_defs parser_options ast
      else ast
    in
    let comments, fixmes =
      if not include_line_comments
      then [], IMap.empty
      else
        let comments = scour_comments file source_text script in
        let fixmes = IMap.empty (*TODO*) in
        comments, fixmes
    in
    if keep_errors then begin
      Fixmes.HH_FIXMES.add file fixmes;
      Option.iter (Core.List.last !errors) Errors.parsing_error
    end;
    { fi_mode; ast; content; comments; file }

let from_text
  ?(elaborate_namespaces  = true)
  ?(include_line_comments = false)
  ?(keep_errors           = true)
  ?(ignore_pos            = false)
  ?(quick                 = false)
  ?(suppress_output       = false)
  ?(lower_coroutines      = true)
  ?(parser_options        = ParserOptions.default)
  (file        : Relative_path.t)
  (source_text : Full_fidelity_source_text.t)
  : result =
    let open Full_fidelity_syntax_tree in
    let tree   = make source_text in
    let tree =
      if lower_coroutines then
        Coroutine_lowerer.lower_coroutines tree
      else
        tree in
    let script = Full_fidelity_positioned_syntax.from_tree tree in
    let fi_mode = if is_php tree then FileInfo.Mphp else
      let mode_string = String.trim (mode tree) in
      let mode_word =
        try List.hd (Str.split (Str.regexp " +") mode_string) with
        | _ -> None
      in
      Option.value_map mode_word ~default:FileInfo.Mpartial ~f:(function
        | "decl"           -> FileInfo.Mdecl
        | "strict"         -> FileInfo.Mstrict
        | ("partial" | "") -> FileInfo.Mpartial
        (* TODO: Come up with better mode detection *)
        | _                -> FileInfo.Mpartial
      )
    in
    lower
      ~elaborate_namespaces
      ~include_line_comments
      ~keep_errors
      ~ignore_pos
      ~quick
      ~suppress_output
      ~parser_options
      ~content:(Full_fidelity_source_text.text source_text)
      ~language:(language tree)
      ~file
      ~fi_mode
      ~source_text
      ~script

let from_file
  ?(elaborate_namespaces  = true)
  ?(include_line_comments = false)
  ?(keep_errors           = true)
  ?(ignore_pos            = false)
  ?(quick                 = false)
  ?(suppress_output       = false)
  ?lower_coroutines
  ?(parser_options        = ParserOptions.default)
  (path : Relative_path.t)
  : result =
    from_text
      ~elaborate_namespaces
      ~include_line_comments
      ~keep_errors
      ~ignore_pos
      ~quick
      ~suppress_output
      ?lower_coroutines
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
  ?(ignore_pos            = false)
  ?(quick                 = false)
  ?(suppress_output       = false)
  ?lower_coroutines
  ?(parser_options        = ParserOptions.default)
  (file    : Relative_path.t)
  (content : string)
  : Parser_hack.parser_return =
    legacy @@ from_text
      ~elaborate_namespaces
      ~include_line_comments
      ~keep_errors
      ~ignore_pos
      ~quick
      ~suppress_output
      ?lower_coroutines
      ~parser_options
      file
      (Full_fidelity_source_text.make content)

let from_file_with_legacy
  ?(elaborate_namespaces  = true)
  ?(include_line_comments = false)
  ?(keep_errors           = true)
  ?(ignore_pos            = false)
  ?(quick                 = false)
  ?(suppress_output       = false)
  ?lower_coroutines
  ?(parser_options        = ParserOptions.default)
  (file : Relative_path.t)
  : Parser_hack.parser_return =
    legacy @@ from_file
      ~elaborate_namespaces
      ~include_line_comments
      ~keep_errors
      ~ignore_pos
      ~quick
      ~suppress_output
      ?lower_coroutines
      ~parser_options
      file
