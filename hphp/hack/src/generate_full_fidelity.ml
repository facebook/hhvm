(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Printf
open Full_fidelity_schema

let full_fidelity_path_prefix = "hphp/hack/src/parser/"
let facts_path_prefix = "hphp/hack/src/facts/"

type comment_style =
  | CStyle
  | MLStyle
[@@deriving show]

let make_header comment_style (header_comment : string) : string =
  let open_char, close_char = match comment_style with
  | CStyle -> "/", '/'
  | MLStyle -> "(", ')'
  in
  sprintf
"%s**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the \"hack\" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @%s; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *%s
 *%c"
  open_char
  (* Any file containing '@' followed by the word 'generated' is considered a
   * generated file in Phabricator. Cheeky trick to avoid making this script
   * being seen as generated. *)
  "generated"
  header_comment
  close_char


type valign =
  (string -> string -> string, unit, string, string -> string) format4

let all_tokens = given_text_tokens @ variable_text_tokens @ no_text_tokens
let align_fmt : 'a . ('a -> string) -> 'a list -> valign = fun f xs ->
  let folder acc x = max acc (String.length (f x)) in
  let width = List.fold_left folder 0 xs in
  Scanf.format_from_string (sprintf "%%-%ds" width) "%-1s"
let kind_name_fmt   = align_fmt (fun x -> x.kind_name  ) schema
let type_name_fmt   = align_fmt (fun x -> x.type_name  ) schema
let trivia_kind_fmt = align_fmt (fun x -> x.trivia_kind) trivia_kinds
let token_kind_fmt  = align_fmt (fun x -> x.token_kind ) all_tokens

let omit_syntax_record =
  let names = SSet.of_list [
    "anonymous_function";
    "closure_type_specifier";
    "function_declaration";
    "function_declaration_header";
    "lambda_expression";
    "lambda_signature";
    "methodish_declaration"]
  in
  fun x -> not (SSet.mem x.type_name names)

module GenerateFFValidatedSyntax = struct

  let to_validate_functions x =
    if x.kind_name = "ErrorSyntax" || x.kind_name = "ListItem" then "" else
    begin
      let get_type_string t =
        match SMap.get t schema_map with
        | None -> failwith @@ sprintf "Unknown type: %s" t
        | Some t -> t.type_name
      in
      let rec validator_of ?(n="validate") = function
      | Token        -> sprintf "%s_token"            n
      | Just       t -> sprintf "%s_%s"               n (get_type_string t)
      | Aggregate  a -> sprintf "%s_%s"               n (aggregate_type_name a)
      | ZeroOrMore s -> sprintf "%s_list_with (%s)"   n (validator_of ~n s)
      | ZeroOrOne  s -> sprintf "%s_option_with (%s)" n (validator_of ~n s)
      in
      let mapper (f,t) =
        sprintf "%s_%s = %s x.%s_%s"
          x.prefix f
          (validator_of t)
          x.prefix f
      in
      let fields = List.rev_map mapper x.fields in

      let mapper (f,t) =
        sprintf "%s_%s = %s x.%s_%s"
          x.prefix f
          (validator_of ~n:"invalidate" t)
          x.prefix f
      in
      let invalidations = List.map mapper x.fields in
      sprintf
"  and validate_%s : %s validator = function
  | { Syntax.syntax = Syntax.%s x; value = v } -> v,
    { %s
    }
  | s -> validation_fail (Some SyntaxKind.%s) s
  and invalidate_%s : %s invalidator = fun (v, x) ->
    { Syntax.syntax =
      Syntax.%s
      { %s
      }
    ; Syntax.value = v
    }
"
        (* validator *)
        x.type_name
        x.type_name
        x.kind_name
        (String.concat "\n    ; " fields)
        x.kind_name
        (* invalidator *)
        x.type_name
        x.type_name
        x.kind_name
        (String.concat "\n      ; " invalidations)
    end

  let to_aggregate_validation x =
    let aggregated_types = aggregation_of x in
    let prefix, trim = aggregate_type_pfx_trim x in
    let compact = Str.global_replace (Str.regexp trim) "" in
    let valign = align_fmt (fun x -> compact x.kind_name) aggregated_types in
    let type_name = aggregate_type_name x in
    let make_validation_clause ty = (* TODO: clean up *)
      sprintf ("
    | Syntax.%s _ -> tag validate_%s (fun x -> %s%s x) x")
        ty.kind_name
        ty.func_name
        prefix (compact ty.kind_name)
    in
    let make_invalidation_clause ty = (* TODO: cleanup *)
      sprintf ("
    | %s" ^^ valign ^^ " thing -> invalidate_%-30s (value, thing)")
        prefix (compact ty.kind_name)
        ty.type_name
    in
    let invalidation_body = map_and_concat make_invalidation_clause aggregated_types in
    let validation_body = map_and_concat make_validation_clause aggregated_types in
    let invalidation =
      sprintf
"  and invalidate_%s : %s invalidator = fun (value, thing) ->
    match thing with%s
"
        type_name
        type_name
        invalidation_body
    in
    let validation =
      sprintf
"  and validate_%s : %s validator = fun x ->
    match Syntax.syntax x with%s
    | s -> aggregation_fail Def.%s s
"
        type_name
        type_name
        validation_body
        (string_of_aggregate_type x)
    in
    validation ^ invalidation



  let full_fidelity_validated_syntax_template = make_header MLStyle "
 * This module contains the functions to (in)validate syntax trees." ^ "

open Full_fidelity_syntax_type (* module signatures of the functor *)
module SyntaxKind = Full_fidelity_syntax_kind
module Def = Schema_definition

module Make(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  module SyntaxBase = Full_fidelity_syntax.WithToken(Token)
  module Syntax = SyntaxBase.WithSyntaxValue(SyntaxValue)
  module Validated = MakeValidated(Token)(SyntaxValue)
  open Validated

  type 'a validator = Syntax.t -> 'a value
  type 'a invalidator = 'a value -> Syntax.t

  exception Validation_failure of SyntaxKind.t option * Syntax.t
  let validation_fail k t = raise (Validation_failure (k, t))

  exception Aggregation_failure of Def.aggregate_type * Syntax.syntax
  let aggregation_fail a s =
    Printf.eprintf \"Aggregation failure: For %s not expecting %s\\n\"
      (Schema_definition.string_of_aggregate_type a)
      (SyntaxKind.to_string @@ Syntax.to_kind s);
    raise (Aggregation_failure (a, s))

  let validate_option_with : 'a . 'a validator -> 'a option validator =
    fun validate node ->
      match Syntax.syntax node with
      | Syntax.Missing -> Syntax.value node, None
      | _ -> let value, result = validate node in value, Some result
  let invalidate_option_with : 'a . 'a invalidator -> 'a option invalidator =
    fun invalidate (value, thing) -> match thing with
    | Some real_thing -> invalidate (value, real_thing)
    | None -> { Syntax.syntax = Syntax.Missing; value }

  let validate_token : Token.t validator = fun node ->
    match Syntax.syntax node with
    | Syntax.Token t -> Syntax.value node, t
    | _ -> validation_fail None node
  let invalidate_token : Token.t invalidator = fun (value, token) ->
    { Syntax.syntax = Syntax.Token token; value }

  let validate_list_with : 'a . 'a validator -> 'a listesque validator =
    fun validate node ->
      let validate_item i =
        match Syntax.syntax i with
        | Syntax.ListItem { list_item ; list_separator } ->
          let item = validate list_item in
          let separator = validate_option_with validate_token list_separator in
          i.Syntax.value, (item, separator)
        | _ -> validation_fail (Some SyntaxKind.ListItem) i
      in
      let validate_list l =
        try Syntactic (List.map validate_item l) with
        | Validation_failure (Some SyntaxKind.ListItem, _) ->
          NonSyntactic (List.map validate l)
      in
      let result =
        match Syntax.syntax node with
        | Syntax.SyntaxList l -> validate_list l
        | Syntax.Missing -> MissingList
        | _ -> SingletonList (validate node)
      in
      node.Syntax.value, result

  let invalidate_list_with : 'a . 'a invalidator -> 'a listesque invalidator =
    fun invalidate (value, listesque) ->
      match listesque with
      | SingletonList node -> invalidate node
      | MissingList -> { Syntax.syntax = Syntax.Missing; value }
      | NonSyntactic nodes ->
        { Syntax.syntax = Syntax.SyntaxList (List.map invalidate nodes); value }
      | Syntactic nodes ->
        let mapper (value, (node, separator)) =
          let inode = invalidate node in
          let iseparator = invalidate_option_with invalidate_token separator in
          { Syntax.syntax = Syntax.ListItem
            { list_item = inode; list_separator = iseparator }
          ; value
          }
        in
        { Syntax.syntax = Syntax.SyntaxList (List.map mapper nodes); value }

  let rec tag : 'a 'b . 'a validator -> ('a -> 'b) -> 'b validator =
    (* Validating aggregate types means picking the right validator for the
     * expected/valid variants and then tagging the result with the constructor
     * corresponding to the variant. This is a repetative pattern. Explicit
     * polymorphism saves us this trouble.
     *)
    fun validator projection node ->
      let value, node = validator node in
      value, projection node
AGGREGATE_VALIDATORS
VALIDATE_FUNCTIONS
end (* Make *)
"
  let full_fidelity_validated_syntax =
  {
    filename = full_fidelity_path_prefix ^ "full_fidelity_validated_syntax.ml";
    template = full_fidelity_validated_syntax_template;
    transformations = [
      { pattern = "VALIDATE_FUNCTIONS"; func = to_validate_functions };
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [
      { aggregate_pattern = "AGGREGATE_VALIDATORS"; aggregate_func = to_aggregate_validation };
    ];
  }

end (* ValidatedSyntax *)

module GenerateFFSyntaxType = struct
  let to_parse_tree x =
    if omit_syntax_record x then "" else
    begin
      let field_width = 50 - String.length x.prefix in
      let fmt = sprintf "%s_%%-%ds: t\n" x.prefix field_width in
      let mapper (f,_) = sprintf (Scanf.format_from_string fmt "%-1s") f in
      let fields = map_and_concat_separated "    ; " mapper x.fields in
      sprintf "  and %s =\n    { %s    }\n"
        x.type_name fields
    end

  let to_syntax x =
    let field_width = 50 - String.length x.prefix in
    let fmt = sprintf "%s_%%-%ds: t\n" x.prefix field_width in
    let mapper (f,_) = sprintf (Scanf.format_from_string fmt "%-1s") f in
    let fields = map_and_concat_separated "    ; " mapper x.fields in
    sprintf ("  | " ^^ kind_name_fmt ^^ " of\n    { %s    }\n")
      x.kind_name fields

  let to_aggregate_type x =
    let aggregated_types = aggregation_of x in
    let prefix, trim = aggregate_type_pfx_trim x in
    let compact = Str.global_replace (Str.regexp trim) "" in
    let valign = align_fmt (fun x -> compact x.kind_name) aggregated_types in
    let type_name = aggregate_type_name x in
    let make_constructor ty =
      sprintf ("%s" ^^ valign ^^ " of %s")
        prefix (compact ty.kind_name)
        ty.type_name
    in
    let type_body = List.map make_constructor aggregated_types in
    sprintf "  and %s =\n  | %s\n" type_name (String.concat "\n  | " type_body)

  let to_validated_syntax x =
    (* Not proud of this, but we have to exclude these things that don't occur
     * in validated syntax. Their absence being the point of the validated
     * syntax
     *)
    if x.kind_name = "ErrorSyntax" || x.kind_name = "ListItem" then "" else
    begin
      let open Printf in
      let mapper (f,c) =
        let rec make_type_string : child_spec -> string = function
        | Aggregate t -> aggregate_type_name t
        | Token -> "Token.t"
        | Just t ->
          (match SMap.get t schema_map with
          | None   -> failwith @@ sprintf "Unknown type: %s" t
          | Some t -> t.type_name
          )
        | ZeroOrMore ((Just _ | Aggregate _ | Token) as c) ->
          sprintf "%s listesque" (make_type_string c)
        | ZeroOrOne  ((Just _ | Aggregate _ | Token) as c) ->
          sprintf "%s option" (make_type_string c)
        | ZeroOrMore c -> sprintf "(%s) listesque" (make_type_string c)
        | ZeroOrOne  c -> sprintf "(%s) option"    (make_type_string c)
        in
        sprintf "%s_%s: %s value" x.prefix f (make_type_string c)
      in
      let fields = map_and_concat_separated "\n    ; " mapper x.fields in
      sprintf "  and %s =\n    { %s\n    }\n" x.type_name fields
    end

  let full_fidelity_syntax_template : string = make_header MLStyle "
 * This module contains the type describing the structure of a syntax tree.
 *
 * The structure of the syntax tree is described by the collection of recursive
 * types that makes up the bulk of this file. The type `t` is the type of a node
 * in the syntax tree; each node has associated with it an arbitrary value of
 * type `SyntaxValue.t`, and syntax node proper, which has structure given by
 * the `syntax` type.
 *
 * Note that every child in the syntax tree is of type `t`, except for the
 * `Token.t` type. This should be the *only* child of a type other than `t`.
 * We are explicitly NOT attempting to impose a type structure on the parse
 * tree beyond what is already implied by the types here. For example,
 * we are not attempting to put into the type system here the restriction that
 * the children of a binary operator must be expressions. The reason for this
 * is because we are potentially parsing code as it is being typed, and we
 * do not want to restrict our ability to make good error recovery by imposing
 * a restriction that will only be valid in correct program text.
 *
 * That said, it would of course be ideal if the only children of a compound
 * statement were statements, and so on. But those invariants should be
 * imposed by the design of the parser, not by the type system of the syntax
 * tree code.
 *
 * We want to be able to use different kinds of tokens, with different
 * performance characteristics. Moreover, we want to associate arbitrary values
 * with the syntax nodes, so that we can construct syntax trees with various
 * properties -- trees that only know their widths and are thereby cheap to
 * serialize, trees that have full position data for each node, trees where the
 * tokens know their text and can therefore be edited, trees that have name
 * annotations or type annotations, and so on.
 *
 * We wish to associate arbitrary values with the syntax nodes so that we can
 * construct syntax trees with various properties -- trees that only know
 * their widths and are thereby cheap to serialize, trees that have full
 * position data for each node, trees where the tokens know their text and
 * can therefore be edited, trees that have name annotations or type
 * annotations, and so on.
 *
 * Therefore this module is functorized by the types for token and value to be
 * associated with the node." ^ "

module type TokenType = sig
  module Trivia : Lexable_trivia_sig.LexableTrivia_S
  type t [@@deriving show]
  val kind: t -> Full_fidelity_token_kind.t
  val to_json: t -> Hh_json.json
  val leading : t -> Trivia.t list
end

module type SyntaxValueType = sig
  type t [@@deriving show]
  val to_json: t -> Hh_json.json
end

(* This functor describe the shape of a parse tree that has a particular kind of
 * token in the leaves, and a particular kind of value associated with each
 * node.
 *)
module MakeSyntaxType(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  type value = SyntaxValue.t [@@deriving show]
  type t = { syntax : syntax ; value : value } [@@deriving show]
PARSE_TREE   and syntax =
  | Token                             of Token.t
  | Missing
  | SyntaxList                        of t list
SYNTAX
end (* MakeSyntaxType *)

module MakeValidated(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  type 'a value = SyntaxValue.t * 'a [@@deriving show]
  (* TODO: Different styles of list seem to only happen in predetermined places,
   * so split this out again into specific variants
   *)
  type 'a listesque =
  | Syntactic of ('a value * Token.t option value) value list
  | NonSyntactic of 'a value list
  | MissingList
  | SingletonList of 'a value
AGGREGATE_TYPESVALIDATED_SYNTAX
[@@deriving show]
end (* MakeValidated *)
"

  let full_fidelity_syntax_type =
  {
    filename = full_fidelity_path_prefix ^ "full_fidelity_syntax_type.ml";
    template = full_fidelity_syntax_template;
    transformations = [
      { pattern = "PARSE_TREE"; func = to_parse_tree };
      { pattern = "SYNTAX"; func = to_syntax };
      { pattern = "VALIDATED_SYNTAX"; func = to_validated_syntax };
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [
      { aggregate_pattern = "AGGREGATE_TYPES"; aggregate_func = to_aggregate_type };
    ];
  }
end (* GenerateFFSyntaxType *)

module GenerateFFSyntaxSig = struct

  let to_constructor_methods x =
    let mapper1 (_f,_) = " t ->" in
    let fields1 = map_and_concat mapper1 x.fields in
    sprintf "  val make_%s :%s t\n"
      x.type_name fields1

  let to_type_tests x =
    sprintf ("  val is_%s : t -> bool\n") x.type_name

  let to_syntax x =
    let field_width = 50 - String.length x.prefix in
    let fmt = sprintf "%s_%%-%ds: t\n" x.prefix field_width in
    let mapper (f,_) = sprintf (Scanf.format_from_string fmt "%-1s") f in
    let fields = map_and_concat_separated "    ; " mapper x.fields in
    sprintf ("  | " ^^ kind_name_fmt ^^ " of\n    { %s    }\n")
      x.kind_name fields

  let full_fidelity_syntax_template : string = (make_header MLStyle "
* This module contains a signature which can be used to describe the public
* surface area of a constructable syntax tree.
  ") ^ "

module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind

module type Syntax_S = sig
  module Token : Lexable_token_sig.LexableToken_S
  type value [@@deriving show]
  type t = { syntax : syntax ; value : value } [@@deriving show]
  and syntax =
  | Token                             of Token.t
  | Missing
  | SyntaxList                        of t list
SYNTAX

  val has_leading_trivia : TriviaKind.t -> Token.t -> bool
  val to_json : ?with_value:bool -> t -> Hh_json.json
  val extract_text : t -> string option
  val is_in_body : t -> int -> bool
  val syntax_node_to_list : t -> t list
  val width : t -> int
  val full_width : t -> int
  val trailing_width : t -> int
  val leading_width : t -> int
  val leading_token : t -> Token.t option
  val children : t -> t list
  val syntax : t -> syntax
  val kind : t -> Full_fidelity_syntax_kind.t
  val value : t -> value
  val make_token : Token.t -> t
  val get_token : t -> Token.t option
  val all_tokens : t -> Token.t list
  val make_missing : Full_fidelity_source_text.t -> int -> t
  val make_list : Full_fidelity_source_text.t -> int -> t list -> t
  val is_namespace_prefix : t -> bool
CONSTRUCTOR_METHODS

  val position : Relative_path.t -> t -> Pos.t option
  val offset : t -> int option
  val is_missing : t -> bool
  val is_list : t -> bool
TYPE_TESTS

  val is_specific_token : TokenKind.t -> t -> bool
  val is_loop_statement : t -> bool
  val is_semicolon      : t -> bool
  val is_name           : t -> bool
  val is_construct      : t -> bool
  val is_destruct       : t -> bool
  val is_static         : t -> bool
  val is_private        : t -> bool
  val is_public         : t -> bool
  val is_protected      : t -> bool
  val is_abstract       : t -> bool
  val is_final          : t -> bool
  val is_async          : t -> bool
  val is_coroutine      : t -> bool
  val is_void           : t -> bool
  val is_left_brace     : t -> bool
  val is_ellipsis       : t -> bool
  val is_comma          : t -> bool
  val is_array          : t -> bool
  val is_var            : t -> bool
  val is_ampersand      : t -> bool
  val is_inout          : t -> bool


end (* Syntax_S *)
"

  let full_fidelity_syntax_sig =
  {
    filename = full_fidelity_path_prefix ^ "syntax_sig.ml";
    template = full_fidelity_syntax_template;
    transformations = [
      { pattern = "SYNTAX"; func = to_syntax };
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods };
      { pattern = "TYPE_TESTS"; func = to_type_tests };
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }
end (* GenerateFFSyntaxSig *)

module GenerateFFSmartConstructors = struct
  let to_constructor_methods x =
    let args = map_and_concat_separated " -> " (fun _ -> "r") x.fields in
    (* We put state as the last argument to use currying *)
    sprintf "  val make_%s : %s -> t -> t * r\n" x.type_name args

  let to_make_methods x =
    let fields = Core_list.mapi x.fields ~f:(fun i _ -> "arg" ^ string_of_int i)
    in
    let stack = String.concat " " fields in
    sprintf "    let %s parser %s = call parser (SCI.make_%s %s)\n"
      x.type_name stack x.type_name stack

  let full_fidelity_smart_constructors_template: string =
  (make_header MLStyle "
 * This module contains a signature which can be used to describe smart
 * constructors.
  ") ^ "

module ParserEnv = Full_fidelity_parser_env

module type SmartConstructors_S = sig
  module Token : Lexable_token_sig.LexableToken_S
  type t (* state *) [@@deriving show]
  type r (* smart constructor return type *) [@@deriving show]

  val initial_state : ParserEnv.t -> t
  val make_token : Token.t -> t -> t * r
  val make_missing : Full_fidelity_source_text.pos -> t -> t * r
  val make_list : Full_fidelity_source_text.pos -> r list -> t -> t * r
CONSTRUCTOR_METHODS
end (* SmartConstructors_S *)

module ParserWrapper (Parser : sig
  type parser_type [@@deriving show]
  module SCI : SmartConstructors_S
  val call : parser_type -> (SCI.t -> SCI.t * SCI.r) -> parser_type * SCI.r
end) = struct

  module Make = struct
    open Parser

    let token parser token = call parser (SCI.make_token token)
    let missing parser p = call parser (SCI.make_missing p)
    let list parser p items = call parser (SCI.make_list p items)
MAKE_METHODS
  end (* Make *)
end (* ParserWrapper *)
"

  let full_fidelity_smart_constructors =
  {
    filename = full_fidelity_path_prefix ^
      "smart_constructors/smartConstructors.ml";
    template = full_fidelity_smart_constructors_template;
    transformations = [
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods };
      { pattern = "MAKE_METHODS"; func = to_make_methods }
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }
end (* GenerateFFSmartConstructors *)

module GenerateFFParserSig = struct
  let to_make_methods x =
    let args = map_and_concat_separated " -> " (fun _ -> "SC.r") x.fields in
    (* We put state as the last argument to use currying *)
    sprintf "        val %s : t -> %s -> t * SC.r\n" x.type_name args

  let full_fidelity_parser_sig_template: string =
  (make_header MLStyle "
 * This module contains a signature which can be used to describe smart
 * constructors.
  ") ^ "

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module Token = Syntax.Token
  module TokenKind = Full_fidelity_token_kind
  module SyntaxError = Full_fidelity_syntax_error
  module Env = Full_fidelity_parser_env
  module type SCWithKind_S = SmartConstructorsWrappers.SyntaxKind_S
  module type Lexer_S = Full_fidelity_lexer_sig.WithToken(Token).Lexer_S

  module WithLexer(Lexer : Lexer_S) = struct
    module type Parser_S = sig
      module SC : SCWithKind_S with module Token = Token
      type t [@@deriving show]
      val pos : t -> Full_fidelity_source_text.pos
      val sc_call : t -> (SC.t -> SC.t * SC.r) -> t * SC.r
      val lexer : t -> Lexer.t
      val errors : t -> Full_fidelity_syntax_error.t list
      val env : t -> Full_fidelity_parser_env.t
      val sc_state : t -> SC.t
      val with_errors : t -> SyntaxError.t list -> t
      val with_lexer : t -> Lexer.t -> t
      val expect : t -> TokenKind.t list -> t
      val skipped_tokens : t -> Token.t list
      val with_skipped_tokens : t -> Token.t list -> t
      val clear_skipped_tokens : t -> t

      module Make : sig
        val token : t -> Token.t -> t * SC.r
        val missing : t -> Full_fidelity_source_text.pos -> t * SC.r
        val list : t -> Full_fidelity_source_text.pos -> SC.r list -> t * SC.r
MAKE_METHODS
      end (* Make *)
    end (* Parser_S *)
  end (* WithLexer *)
end (* WithSyntax *)
"

  let full_fidelity_parser_sig =
  {
    filename = full_fidelity_path_prefix ^ "parserSig.ml";
    template = full_fidelity_parser_sig_template;
    transformations = [
      { pattern = "MAKE_METHODS"; func = to_make_methods }
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }
end (* GenerateFFParserSig *)

module GenerateFFVerifySmartConstructors = struct
  let to_constructor_methods x =
    let params = Core_list.mapi x.fields ~f:(fun i _ -> sprintf "p%d" i) in
    let args = Core_list.mapi x.fields ~f:(fun i _ -> sprintf "a%d" i) in
    sprintf "
  let make_%s %s stack =
    match stack with
    | %s :: rem ->
      let () = verify ~stack [%s] [%s] \"%s\" in
      let node = Syntax.make_%s %s in
      node :: rem, node
    | _ -> failwith \"Unexpected stack state\"
    "
    x.type_name
    (String.concat " " params)
    (String.concat " :: " (List.rev args))
    (String.concat "; " params)
    (String.concat "; " args)
    x.type_name
    x.type_name
    (String.concat " " params)

  let full_fidelity_verify_smart_constructors_template: string =
    (make_header MLStyle "
 * This module contains smart constructors implementation that can be used to
 * build AST.
 ") ^ "

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module Token = Syntax.Token
  type t = Syntax.t list [@@deriving show]
  type r = Syntax.t [@@deriving show]

  exception NotEquals of
    string * Syntax.t list * Syntax.t list * Syntax.t list
  exception NotPhysicallyEquals of
    string * Syntax.t list * Syntax.t list * Syntax.t list

  let verify ~stack params args cons_name =
    let equals e1 e2 =
      if e1 != e2 then
        if e1 = e2
        then
          raise @@ NotPhysicallyEquals
            (cons_name
            , List.rev stack
            , params
            , args
            )
        else
          raise @@ NotEquals
            (cons_name
            , List.rev stack
            , params
            , args
            )
    in
    Core_list.iter2_exn ~f:equals params args

  let initial_state _ = []

  let make_token token stack =
    let token = Syntax.make_token token in
    token :: stack, token

  let make_missing (s, o) stack =
    let missing = Syntax.make_missing s o in
    missing :: stack, missing

  let make_list (s, o) items stack =
    if items <> [] then
      let (h, t) = Core_list.split_n stack (List.length items) in
      let () = verify ~stack items (List.rev h) \"list\" in
      let lst = Syntax.make_list s o items in
      lst :: t, lst
    else make_missing (s, o) stack
CONSTRUCTOR_METHODS
end (* WithSyntax *)
"

  let full_fidelity_verify_smart_constructors =
  {
    filename = full_fidelity_path_prefix ^
      "smart_constructors/verifySmartConstructors.ml";
    template = full_fidelity_verify_smart_constructors_template;
    transformations = [
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }
end (* GenerateFFVerifySmartConstructors *)

module GenerateFFSyntaxSmartConstructors = struct
  let to_constructor_methods x =
    let fields = Core_list.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i)
    in
    let stack = String.concat " " fields in
    let arr = String.concat "; " fields in
    sprintf "    let make_%s %s state = State.next state [%s], Syntax.make_%s %s\n"
      x.type_name stack arr x.type_name stack

  let full_fidelity_syntax_smart_constructors_template: string =
    (make_header MLStyle "
 * This module contains smart constructors implementation that can be used to
 * build AST.
 ") ^ "

module type SC_S = SmartConstructors.SmartConstructors_S
module ParserEnv = Full_fidelity_parser_env

module type State_S = sig
  type r [@@deriving show]
  type t [@@deriving show]
  val initial : ParserEnv.t -> t
  val next : t -> r list -> t
end

module WithSyntax(Syntax : Syntax_sig.Syntax_S) = struct
  module WithState(State : State_S with type r = Syntax.t) = struct
    module Token = Syntax.Token
    type t = State.t [@@deriving show]
    type r = Syntax.t [@@deriving show]

    let initial_state = State.initial
    let make_token token state = State.next state [], Syntax.make_token token
    let make_missing (s, o) state = State.next state [], Syntax.make_missing s o
    let make_list (s, o) items state =
      if items <> []
      then State.next state items, Syntax.make_list s o items
      else make_missing (s, o) state
CONSTRUCTOR_METHODS
  end (* WithState *)

  include WithState(
    struct
      type r = Syntax.t [@@deriving show]
      type t = unit [@@deriving show]
      let initial _ = ()
      let next () _ = ()
    end
  )
end (* WithSyntax *)
"

  let full_fidelity_syntax_smart_constructors =
  {
    filename = full_fidelity_path_prefix ^
      "smart_constructors/syntaxSmartConstructors.ml";
    template = full_fidelity_syntax_smart_constructors_template;
    transformations = [
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }
end (* GenerateFFSyntaxSmartConstructors *)

module GenerateFlattenSmartConstructors = struct
  let to_constructor_methods x =
    let fields = Core_list.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i)
    in
    let stack = String.concat " " fields in
    let arr = String.concat "; " fields in
    let is_zero =
      Core_list.map fields (sprintf "Op.is_zero %s")
      |> String.concat " && " in
    sprintf "  let make_%s %s state =\n    \
        if %s then state, Op.zero\n    \
        else state, Op.flatten [%s]\n"
      x.type_name stack is_zero arr

  let flatten_smart_constructors_template: string =
    (make_header MLStyle "
 * This module contains smart constructors implementation that does nothing
 * and can be used as initial stubs.
 ") ^ "

module type Op_S = sig
  type r [@@deriving show]
  val is_zero: r -> bool
  val flatten: r list -> r
  val zero: r
end

module WithOp(Op : Op_S) = struct
  type r = Op.r [@@deriving show]

  let make_token _token state = state, Op.zero
  let make_missing _ state = state, Op.zero
  let make_list _  _ state = state, Op.zero
CONSTRUCTOR_METHODS

end (* WithSyntax *)
"

  let flatten_smart_constructors =
  {
    filename = facts_path_prefix ^ "flatten_smart_constructors.ml";
    template = flatten_smart_constructors_template;
    transformations = [
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }
end (* GenerateFFSyntaxSmartConstructors *)

module GenerateFFSmartConstructorsWrappers = struct
  let to_constructor_methods x =
    let fields = Core_list.mapi x.fields ~f:(fun i _ -> "arg" ^ string_of_int i)
    in
    let stack = String.concat " " fields in
    let raw_stack =
      map_and_concat_separated " " (fun x -> "(snd " ^ x ^ ")") fields
    in
    sprintf
      "  let make_%s %s state = compose SK.%s (SC.make_%s %s state)\n"
      x.type_name stack x.kind_name x.type_name raw_stack

  let to_type_tests_sig x =
    sprintf ("  val is_%s : r -> bool\n")
      x.type_name

  let to_type_tests x =
    sprintf ("  let is_" ^^ type_name_fmt ^^ " = has_kind SK.%s\n")
      x.type_name x.kind_name

  let full_fidelity_smart_constructors_wrappers_template: string =
    (make_header MLStyle "
 * This module contains smart constructors implementation that can be used to
 * build AST.
 ") ^ "

module type SC_S = SmartConstructors.SmartConstructors_S
module SK = Full_fidelity_syntax_kind

module type SyntaxKind_S = sig
  include SC_S
  type original_sc_r [@@deriving show]
  val extract : r -> original_sc_r
  val is_name : r -> bool
  val is_abstract : r -> bool
  val is_missing : r -> bool
  val is_list : r -> bool
TYPE_TESTS_SIG
end

module SyntaxKind(SC : SC_S)
  : (SyntaxKind_S
    with module Token = SC.Token
    and type original_sc_r = SC.r
    and type t = SC.t
  ) = struct
  module Token = SC.Token
  type original_sc_r = SC.r [@@deriving show]
  type t = SC.t [@@deriving show]
  type r = SK.t * SC.r [@@deriving show]

  let extract (_, r) = r
  let kind_of (kind, _) = kind
  let compose : SK.t -> t * SC.r -> t * r = fun kind (state, res) ->
    state, (kind, res)
  let initial_state = SC.initial_state

  let make_token token state = compose (SK.Token (SC.Token.kind token)) (SC.make_token token state)
  let make_missing p state = compose SK.Missing (SC.make_missing p state)
  let make_list p items state =
    let kind = if items <> [] then SK.SyntaxList else SK.Missing in
    compose kind (SC.make_list p (Core_list.map ~f:snd items) state)
CONSTRUCTOR_METHODS

  let has_kind kind node = kind_of node = kind
  let is_name = has_kind (SK.Token Full_fidelity_token_kind.Name)
  let is_abstract = has_kind (SK.Token Full_fidelity_token_kind.Abstract)
  let is_missing = has_kind SK.Missing
  let is_list = has_kind SK.Missing
TYPE_TESTS

end (* SyntaxKind *)
"

  let full_fidelity_smart_constructors_wrappers =
  {
    filename = full_fidelity_path_prefix ^
      "smart_constructors/smartConstructorsWrappers.ml";
    template = full_fidelity_smart_constructors_wrappers_template;
    transformations = [
      { pattern = "TYPE_TESTS_SIG"; func = to_type_tests_sig };
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods };
      { pattern = "TYPE_TESTS"; func = to_type_tests }
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }
end (* GenerateFFSmartConstructorsWrappers *)

module GenerateFFSyntax = struct
  let to_to_kind x =
    sprintf ("      | " ^^ kind_name_fmt ^^ " _ -> SyntaxKind.%s\n")
      x.kind_name x.kind_name

  let to_type_tests x =
    sprintf ("    let is_" ^^ type_name_fmt ^^ " = has_kind SyntaxKind.%s\n")
      x.type_name x.kind_name
  let to_children x =
    let mapper (f,_) = sprintf "        %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    sprintf "      | %s {\n%s      } -> [\n%s      ]\n"
      x.kind_name fields fields

  let to_fold_from_syntax x =
    let mapper (f,_) = sprintf "        %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    let mapper2 (f, _) = sprintf "         let acc = f acc %s_%s in\n" x.prefix f in
    let fields2 = map_and_concat mapper2 x.fields in
    sprintf "      | %s {\n%s      } ->\n%s         acc\n"
      x.kind_name fields fields2

  let to_children_names x =
    let mapper1 (f,_) = sprintf "        %s_%s;\n" x.prefix f in
    let mapper2 (f,_) = sprintf "        \"%s_%s\";\n" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let fields2 = map_and_concat mapper2 x.fields in
    sprintf "      | %s {\n%s      } -> [\n%s      ]\n"
      x.kind_name fields1 fields2

  let to_syntax_from_children x =
    let mapper (f,_) = sprintf "          %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    sprintf "      | (SyntaxKind.%s, [
%s        ]) ->
        %s {
%s        }
"
      x.kind_name fields x.kind_name fields

  let to_constructor_methods x =
    let mapper1 (f,_) = sprintf "        %s_%s\n" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let mapper2 (f,_) = sprintf "          %s_%s;\n" x.prefix f in
    let fields2 = map_and_concat mapper2 x.fields in
    sprintf "      let make_%s
%s      =
        let syntax = %s {
%s        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

"
    x.type_name fields1 x.kind_name fields2

  let to_from_methods x =
    if omit_syntax_record x then "" else
    begin
      let mapper (f,_) = sprintf "          %s_%s;\n" x.prefix f in
      let fields = map_and_concat mapper x.fields in
      sprintf "     let from_%s {
%s       } = %s {
%s       }
"
        x.type_name fields x.kind_name fields
    end

  let to_get_methods x =
    if omit_syntax_record x then "" else
    begin
      let mapper (f,_) = sprintf "          %s_%s;\n" x.prefix f in
      let fields = map_and_concat mapper x.fields in
      sprintf "     let get_%s x =
        match x with
        | %s {\n%s            } -> {\n%s           }
        | _ -> failwith \"get_%s: not a %s\"
"
        x.type_name x.kind_name fields fields x.type_name x.kind_name
    end

  let full_fidelity_syntax_template = make_header MLStyle "
 * With these factory methods, nodes can be built up from their child nodes. A
 * factory method must not just know all the children and the kind of node it is
 * constructing; it also must know how to construct the value that this node is
 * going to be tagged with. For that reason, an optional functor is provided.
 * This functor requires that methods be provided to construct the values
 * associated with a token or with any arbitrary node, given its children. If
 * this functor is used then the resulting module contains factory methods.
 *
 * This module also provides some useful helper functions, like an iterator,
 * a rewriting visitor, and so on." ^ "

open Full_fidelity_syntax_type
module SyntaxKind = Full_fidelity_syntax_kind
module TokenKind = Full_fidelity_token_kind
module Operator = Full_fidelity_operator
[@@@warning \"-27\"] (* unused variable *)

module WithToken(Token: TokenType) = struct
  module WithSyntaxValue(SyntaxValue: SyntaxValueType) = struct

    include MakeSyntaxType(Token)(SyntaxValue)

    let make syntax value =
      { syntax; value }

    let syntax node =
      node.syntax

    let value node =
      node.value

    let syntax_node_to_list node =
      match syntax node with
      | SyntaxList x -> x
      | Missing -> []
      | _ -> [node]

    let to_kind syntax =
      match syntax with
      | Missing                             -> SyntaxKind.Missing
      | Token                             t -> SyntaxKind.Token (Token.kind t)
      | SyntaxList                        _ -> SyntaxKind.SyntaxList
TO_KIND

    let kind node =
      to_kind (syntax node)

    let has_kind syntax_kind node =
      kind node = syntax_kind

    let is_missing node =
      kind node = SyntaxKind.Missing

    let is_list node =
      kind node = SyntaxKind.SyntaxList

TYPE_TESTS

    let is_loop_statement node =
      is_for_statement node ||
      is_foreach_statement node ||
      is_while_statement node ||
      is_do_statement node

    let is_separable_prefix node =
      match syntax node with
      | Token t -> begin
        TokenKind.(match Token.kind t with
        | PlusPlus | MinusMinus -> false
        | _ -> true) end
      | _ -> true

    let is_specific_token kind node =
      match syntax node with
      | Token t -> Token.kind t = kind
      | _ -> false

    let is_namespace_prefix node =
      match syntax node with
      | QualifiedName e ->
        begin match Core_list.last (syntax_node_to_list e.qualified_name_parts) with
        | None -> false
        | Some p ->
          begin match syntax p with
          | ListItem p -> not (is_missing p.list_separator)
          | _ -> false
          end
        end
      | _ -> false

    let has_leading_trivia kind token =
      Hh_core.List.exists (Token.leading token)
        ~f:(fun trivia ->  Token.Trivia.kind trivia = kind)

    let is_semicolon  = is_specific_token TokenKind.Semicolon
    let is_name       = is_specific_token TokenKind.Name
    let is_construct  = is_specific_token TokenKind.Construct
    let is_destruct   = is_specific_token TokenKind.Destruct
    let is_static     = is_specific_token TokenKind.Static
    let is_private    = is_specific_token TokenKind.Private
    let is_public     = is_specific_token TokenKind.Public
    let is_protected  = is_specific_token TokenKind.Protected
    let is_abstract   = is_specific_token TokenKind.Abstract
    let is_final      = is_specific_token TokenKind.Final
    let is_async      = is_specific_token TokenKind.Async
    let is_coroutine  = is_specific_token TokenKind.Coroutine
    let is_void       = is_specific_token TokenKind.Void
    let is_left_brace = is_specific_token TokenKind.LeftBrace
    let is_ellipsis   = is_specific_token TokenKind.DotDotDot
    let is_comma      = is_specific_token TokenKind.Comma
    let is_array      = is_specific_token TokenKind.Array
    let is_var        = is_specific_token TokenKind.Var
    let is_ampersand  = is_specific_token TokenKind.Ampersand
    let is_inout      = is_specific_token TokenKind.Inout

    let fold_over_children f acc syntax =
      match syntax with
      | Missing -> acc
      | Token _ -> acc
      | SyntaxList items ->
        List.fold_left f acc items
FOLD_FROM_SYNTAX

    (* The order that the children are returned in should match the order
       that they appear in the source text *)
    let children_from_syntax s =
      match s with
      | Missing -> []
      | Token _ -> []
      | SyntaxList x -> x
CHILDREN

    let children node =
      children_from_syntax node.syntax

    let children_names node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | SyntaxList _ -> []
CHILDREN_NAMES

    let rec to_json ?(with_value = false) node =
      let open Hh_json in
      let ch = match node.syntax with
      | Token t -> [ \"token\", Token.to_json t ]
      | SyntaxList x -> [ (\"elements\",
        JSON_Array (List.map (to_json ~with_value) x)) ]
      | _ ->
        let rec aux acc c n =
          match c, n with
          | ([], []) -> acc
          | ((hc :: tc), (hn :: tn)) ->
            aux ((hn, (to_json ~with_value) hc) :: acc) tc tn
          | _ -> failwith \"mismatch between children and names\" in
        List.rev (aux [] (children node) (children_names node)) in
      let k = (\"kind\", JSON_String (SyntaxKind.to_string (kind node))) in
      let v = if with_value then
        (\"value\", SyntaxValue.to_json node.value) :: ch
        else ch in
      JSON_Object (k :: v)

    let binary_operator_kind b =
      match syntax b with
      | Token token ->
        let kind = Token.kind token in
        if Operator.is_trailing_operator_token kind then
          Some (Operator.trailing_from_token kind)
        else
          None
      | _ -> None

    let get_token node =
      match (syntax node) with
      | Token token -> Some token
      | _ -> None

    let leading_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if token = None then
            let result = aux (children h) in
            if result = None then aux t else result
          else
            token in
      aux [node]

    let trailing_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if token = None then
            let result = aux (List.rev (children h)) in
            if result = None then aux t else result
          else
            token in
      aux [node]

    let syntax_from_children kind ts =
      match kind, ts with
SYNTAX_FROM_CHILDREN      | (SyntaxKind.Missing, []) -> Missing
      | (SyntaxKind.SyntaxList, items) -> SyntaxList items
      | _ -> failwith
        \"syntax_from_children called with wrong number of children\"

    let all_tokens node =
      let rec aux acc nodes =
        match nodes with
        | [] -> acc
        | h :: t ->
          begin
            match syntax h with
            | Token token -> aux (token :: acc) t
            | _ -> aux (aux acc (children h)) t
          end in
      List.rev (aux [] [node])

    module type ValueBuilderType = sig
      val value_from_children:
        Full_fidelity_source_text.t ->
        int -> (* offset *)
        Full_fidelity_syntax_kind.t ->
        t list ->
        SyntaxValue.t
      val value_from_token: Token.t -> SyntaxValue.t
      val value_from_syntax: syntax -> SyntaxValue.t
    end

    module WithValueBuilder(ValueBuilder: ValueBuilderType) = struct
      let from_children text offset kind ts =
        let syntax = syntax_from_children kind ts in
        let value = ValueBuilder.value_from_children text offset kind ts in
        make syntax value

      let make_token token =
        let syntax = Token token in
        let value = ValueBuilder.value_from_token token in
        make syntax value

      let make_missing text offset =
        from_children text offset SyntaxKind.Missing []

      (* An empty list is represented by Missing; everything else is a
        SyntaxList, even if the list has only one item. *)
      let make_list text offset items =
        match items with
        | [] -> make_missing text offset
        | _ -> from_children text offset SyntaxKind.SyntaxList items

CONSTRUCTOR_METHODS

FROM_METHODS

GET_METHODS

    end (* WithValueBuilder *)
  end (* WithSyntaxValue *)
end (* WithToken *)
"

  let full_fidelity_syntax =
  {
    filename = full_fidelity_path_prefix ^ "full_fidelity_syntax.ml";
    template = full_fidelity_syntax_template;
    transformations = [
      { pattern = "TO_KIND"; func = to_to_kind };
      { pattern = "TYPE_TESTS"; func = to_type_tests };
      { pattern = "CHILDREN"; func = to_children };
      { pattern = "FOLD_FROM_SYNTAX"; func = to_fold_from_syntax };
      { pattern = "CHILDREN_NAMES"; func = to_children_names };
      { pattern = "SYNTAX_FROM_CHILDREN"; func = to_syntax_from_children };
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods };
      { pattern = "FROM_METHODS"; func = to_from_methods };
      { pattern = "GET_METHODS"; func = to_get_methods };
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }

end (* GenerateFFSyntax *)

module GenerateFFTriviaKind = struct

  let to_trivia { trivia_kind; trivia_text = _; } =
    sprintf "  | %s\n" trivia_kind

  let to_to_string { trivia_kind; trivia_text } =
    sprintf ("  | " ^^ trivia_kind_fmt ^^ " -> \"%s\"\n")
      trivia_kind trivia_text

  let full_fidelity_trivia_kind_template = make_header MLStyle "" ^ "

type t =
TRIVIA
  [@@deriving show, enum]

let to_string kind =
  match kind with
TO_STRING"

let full_fidelity_trivia_kind =
{
  filename = full_fidelity_path_prefix ^ "/full_fidelity_trivia_kind.ml";
  template = full_fidelity_trivia_kind_template;
  transformations = [];
  token_no_text_transformations = [];
  token_given_text_transformations = [];
  token_variable_text_transformations = [];
  trivia_transformations = [
    { trivia_pattern = "TRIVIA";
      trivia_func = map_and_concat to_trivia };
    { trivia_pattern = "TO_STRING";
      trivia_func = map_and_concat to_to_string }];
  aggregate_transformations = [];
}

end (* GenerateFFSyntaxKind *)

module GenerateFFSyntaxKind = struct

  let to_tokens x =
    sprintf "  | %s\n" x.kind_name

  let to_to_string x =
    sprintf ("  | " ^^ kind_name_fmt ^^ " -> \"%s\"\n")
      x.kind_name x.description

  let full_fidelity_syntax_kind_template = make_header MLStyle "" ^ "

type t =
  | Token of Full_fidelity_token_kind.t
  | Missing
  | SyntaxList
TOKENS
  [@@deriving show]

let to_string kind =
  match kind with
  | Token _                           -> \"token\"
  | Missing                           -> \"missing\"
  | SyntaxList                        -> \"list\"
TO_STRING"

let full_fidelity_syntax_kind =
{
  filename = full_fidelity_path_prefix ^ "full_fidelity_syntax_kind.ml";
  template = full_fidelity_syntax_kind_template;
  transformations = [
    { pattern = "TOKENS"; func = to_tokens };
    { pattern = "TO_STRING"; func = to_to_string };
  ];
  token_no_text_transformations = [];
  token_given_text_transformations = [];
  token_variable_text_transformations = [];
  trivia_transformations = [];
  aggregate_transformations = [];
}

end (* GenerateFFTriviaKind *)

module GenerateFFJavaScript = struct

  let to_from_json x =
    sprintf "    case '%s':
      return %s.from_json(json, position, source);
" x.description x.kind_name

  let trivia_from_json { trivia_kind; trivia_text } =
    sprintf "    case '%s':
      return %s.from_json(json, position, source);
" trivia_text trivia_kind

  let trivia_static_from_json { trivia_kind; trivia_text } =
    sprintf "      case '%s':
        return new %s(trivia_text);
" trivia_text trivia_kind

  let trivia_classes { trivia_kind; trivia_text } =
    sprintf "class %s extends EditableTrivia
{
  constructor(text) { super('%s', text); }
  with_text(text)
  {
    return new %s(text);
  }
}

" trivia_kind trivia_text trivia_kind

  let to_editable_syntax x =
    let ctor_mapper (f,_) = f in
    let ctor = map_and_concat_separated ",\n    " ctor_mapper x.fields in
    let ctor2 = map_and_concat_separated ",\n        " ctor_mapper x.fields in
    let children_mapper (f,_) = sprintf "%s: %s" f f in
    let children =
      map_and_concat_separated ",\n      " children_mapper x.fields in
    let props_mapper (f,_) =
      sprintf "get %s() { return this.children.%s; }" f f in
    let props = map_and_concat_separated "\n  " props_mapper x.fields in
    let withs_mapper (f,_) =
      let inner_mapper (f_inner,_) =
        let prefix = if f_inner = f then "" else "this." in
        sprintf "%s%s" prefix f_inner in
      let inner = map_and_concat_separated ",\n      " inner_mapper x.fields in
      sprintf "with_%s(%s){\n    return new %s(\n      %s);\n  }"
        f f x.kind_name inner in
    let withs = map_and_concat_separated "\n  " withs_mapper x.fields in
    let rewriter_mapper (f,_) =
      sprintf "var %s = this.%s.rewrite(rewriter, new_parents);" f f in
    let rewriter =
      map_and_concat_separated "\n    " rewriter_mapper x.fields in
    let condition_mapper (f,_) = sprintf "%s === this.%s" f f in
    let condition =
      map_and_concat_separated " &&\n      " condition_mapper x.fields in
    let json_mapper (f,_) = sprintf
      "let %s = EditableSyntax.from_json(
      json.%s_%s, position, source);
    position += %s.width;" f x.prefix f f in
    let json = map_and_concat_separated "\n    " json_mapper x.fields in
    let keys_mapper (f,_) = sprintf "'%s'" f in
    let keys = map_and_concat_separated ",\n        " keys_mapper x.fields in
    sprintf
"class %s extends EditableSyntax
{
  constructor(
    %s)
  {
    super('%s', {
      %s });
  }
  %s
  %s
  rewrite(rewriter, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    %s
    if (
      %s)
    {
      return rewriter(this, parents);
    }
    else
    {
      return rewriter(new %s(
        %s), parents);
    }
  }
  static from_json(json, position, source)
  {
    %s
    return new %s(
        %s);
  }
  get children_keys()
  {
    if (%s._children_keys == null)
      %s._children_keys = [
        %s];
    return %s._children_keys;
  }
}
" x.kind_name ctor x.description children props withs rewriter condition
  x.kind_name ctor2 json x.kind_name ctor2 x.kind_name x.kind_name keys
  x.kind_name

  let to_exports_syntax x =
    sprintf "exports.%s = %s;\n" x.kind_name x.kind_name

  let to_editable_no_text x =
    sprintf "class %sToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('%s', leading, trailing, '');
  }
}
" x.token_kind x.token_text

let to_editable_given_text x =
  sprintf "class %sToken extends EditableToken
{
  constructor(leading, trailing)
  {
    super('%s', leading, trailing, '%s');
  }
}
" x.token_kind x.token_text x.token_text

  let to_editable_variable_text x =
    sprintf "class %sToken extends EditableToken
{
  constructor(leading, trailing, text)
  {
    super('%s', leading, trailing, text);
  }
  with_text(text)
  {
    return new %sToken(this.leading, this.trailing, text);
  }

}
" x.token_kind x.token_text x.token_kind

  let to_factory_no_text x =
    sprintf
"    case '%s':
       return new %sToken(leading, trailing);
"
      x.token_text x.token_kind

  let to_factory_given_text = to_factory_no_text

  let to_factory_variable_text x =
    sprintf
"    case '%s':
       return new %sToken(leading, trailing, token_text);
"
      x.token_text x.token_kind

  let to_export_token x =
    sprintf "exports.%sToken = %sToken;\n" x.token_kind x.token_kind

  let to_export_trivia x =
    sprintf "exports.%s = %s;\n" x.trivia_kind x.trivia_kind

  let full_fidelity_javascript_template = make_header CStyle "" ^ "

\"use strict\";

let utils = require('./full_fidelity_utils.js');
let array_map_reduce = utils.array_map_reduce;
let array_sum = utils.array_sum;

class EditableSyntax
{
  constructor(syntax_kind, children)
  {
    this._syntax_kind = syntax_kind;
    this._children = children;
    let width = 0;
    for(let child in children)
      width += children[child].width;
    this._children_width = width;
  }
  get syntax_kind() { return this._syntax_kind; }
  get children() { return this._children; }
  get is_token() { return false; }
  get is_trivia() { return false; }
  get is_list() { return false; }
  get is_missing() { return false; }

  get width() { return this._children_width; }

  get full_text()
  {
    let s = '';
    for(let key of this.children_keys)
      s += this.children[key].full_text;
    return s;
  }

  static from_json(json, position, source)
  {
    switch(json.kind)
    {
    case 'token':
      return EditableToken.from_json(json.token, position, source);
    case 'list':
      return EditableList.from_json(json, position, source);
FROM_JSON_TRIVIA
    case 'missing':
      return Missing.missing;
FROM_JSON_SYNTAX
    default:
      throw 'unexpected json kind: ' + json.kind; // TODO: Better exception
    }
  }

  reduce(reducer, accumulator, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    for(let key of this.children_keys)
    {
      accumulator = this.children[key].reduce(
        reducer, accumulator, new_parents);
    }
    return reducer(this, accumulator, parents);
  }

  // Returns all the parents (and the node itself) of the first node
  // that matches a predicate, or [] if there is no such node.
  find(predicate, parents)
  {
    if (parents == undefined)
      parents = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    if (predicate(this))
      return new_parents;
    for(let key of this.children_keys)
    {
      let result = this.children[key].find(predicate, new_parents);
      if (result.length != 0)
        return result;
    }
    return [];
  }

  // Returns a list of nodes that match a predicate.
  filter(predicate)
  {
    let reducer = (node, acc, parents) => {
      if (predicate(node))
        acc.push(node);
      return acc;
    };
    return this.reduce(reducer, []);
  }

  of_syntax_kind(kind)
  {
    return this.filter((node) => node.syntax_kind == kind);
  }

  remove_where(predicate)
  {
    return this.rewrite(
      (node, parents) => predicate(node) ? Missing.missing : node);
  }

  without(target)
  {
    return this.remove_where((node) => node === target);
  }

  replace(new_node, target)
  {
    return this.rewrite((node, parents) => node === target ? new_node : node);
  }

  pre_order(action, parents)
  {
    if (parents == undefined)
      parents = [];
    action(this, parents);
    let new_parents = parents.slice();
    new_parents.push(this);
    for(let key of this.children_keys)
      this.children[key].pre_order(action, new_parents);
  }

  get leftmost_token()
  {
    if (this.is_token)
      return this;

    for(let key of this.children_keys)
    {
      if (!this.children[key].is_missing)
        return this.children[key].leftmost_token;
    }
    return null;
  }

  get rightmost_token()
  {
    if (this.is_token)
      return this;

    for (let i = this.children_keys.length - 1; i >= 0; i--)
    {
      if (!this.children[this.children_keys[i]].is_missing)
        return this.children[key].rightmost_token;
    }
    return null;
  }

  insert_before(new_node, target)
  {
    // Inserting before missing is an error.
    if (target.is_missing)
      throw 'Target must not be missing in insert_before.';

    // Inserting missing is a no-op
    if (new_node.is_missing)
      return this;

    if (new_node.is_trivia && !target.is_trivia)
    {
      let token = target.is_token ? target : target.leftmost_token;
      if (token == null)
        throw 'Unable to find token to insert trivia.';

      // Inserting trivia before token is inserting to the right end of
      // the leading trivia.
      let new_leading = EditableSyntax.concatenate_lists(
        token.leading, new_node);
      let new_token = token.with_leading(new_leading);
      return this.replace(new_token, token);
    }

    return this.replace(
      EditableSyntax.concatenate_lists(new_node, target), target);
  }

  insert_after(new_node, target)
  {
    // Inserting after missing is an error.
    if (target.is_missing)
      throw 'Target must not be missing in insert_after.';

    // Inserting missing is a no-op
    if (new_node.is_missing)
      return this;

    if (new_node.is_trivia && !target.is_trivia)
    {
      let token = target.is_token ? target : target.rightmost_token;
      if (token == null)
        throw 'Unable to find token to insert trivia.';

      // Inserting trivia after token is inserting to the left end of
      // the trailing trivia.
      let new_trailing = EditableSyntax.concatenate_lists(
        new_node, token.trailing);
      let new_token = token.with_trailing(new_trailing);
      return this.replace(new_token, token);
    }

    return this.replace(
      EditableSyntax.concatenate_lists(target, new_node), target);
  }

  static to_list(syntax_list)
  {
    if (syntax_list.length == 0)
      return Missing.missing;
    else
      return new EditableList(syntax_list);
  }

  static concatenate_lists(left, right)
  {
    if (left.is_missing)
      return right;
    if (right.is_missing)
      return left;
    if (left.is_list && right.is_list)
      return new EditableList(left.children.concat(right.children));
    if (left.is_list)
      return new EditableList(left.children.splice().push(right));
    if (right.is_list)
      return new EditableList([right].concat(left.children));
    return new EditableList([left, right]);
  }
}

class EditableList extends EditableSyntax
{
  constructor(children)
  {
    super('list', children);
  }
  get is_list() { return true; }

  static from_json(json, position, source)
  {
    let children = [];
    let current_position = position;
    for(let element of json.elements)
    {
      let child = EditableSyntax.from_json(element, current_position, source);
      children.push(child);
      current_position += child.width;
    }
    return new EditableList(children);
  }

  rewrite(rewriter, parents)
  {
    let dirty = false;
    let new_children = [];
    let new_parents = parents.slice();
    new_parents.push(this);
    for (let key of this.children_keys)
    {
      let child = this.children[key];
      let new_child = child.rewrite(rewriter, new_parents);
      if (new_child != child)
        dirty = true;
      if (new_child != null)
      {
        if (new_child.is_list)
        {
          for(let n of new_child.children)
            new_children.push(n);
        }
        else
          new_children.push(new_child);
      }
    }
    let result = this;
    if (dirty)
    {
      if (new_children.length === 0)
        result = Missing.missing;
      else if (new_children.length === 1)
        result = new_children[0];
      else
        result = new EditableList(new_children);
    }
    return rewriter(result, parents);
  }
  get children_keys()
  {
    return Object.keys(this.children);
  }
}

class EditableToken extends EditableSyntax
{
  constructor(token_kind, leading, trailing, text)
  {
    super('token', { leading : leading, trailing : trailing });
    this._token_kind = token_kind;
    this._text = text;
  }

  get token_kind() { return this._token_kind; }
  get text() { return this._text; }
  get leading() { return this.children.leading; }
  get trailing() { return this.children.trailing; }
  get width()
  {
    return this.text.length + this.leading.width + this.trailing.width;
  }
  get is_token() { return true; }
  get full_text()
  {
    return this.leading.full_text + this.text + this.trailing.full_text;
  }
  with_leading(leading)
  {
    return EditableToken.factory(
      this.token_kind, leading, this.trailing, this.text);
  }
  with_trailing(trailing)
  {
    return EditableToken.factory(
      this.token_kind, this.leading, trailing, this.text);
  }
  static factory(token_kind, leading, trailing, token_text)
  {
    switch(token_kind)
    {
FACTORY_NO_TEXT_TOKENS
FACTORY_GIVEN_TEXT_TOKENS
FACTORY_VARIABLE_TEXT_TOKENS
      default: throw 'unexpected token kind; ' + token_kind;
      // TODO: Better error
    }
  }

  rewrite(rewriter, parents)
  {
    let new_parents = parents.slice();
    new_parents.push(this);
    let leading = this.leading.rewrite(rewriter, new_parents);
    let trailing = this.trailing.rewrite(rewriter, new_parents);
    if (leading === this.leading && trailing === this.trailing)
      return rewriter(this, parents);
    else
      return rewriter(EditableToken.factory(
        this.token_kind, leading, trailing, this.text), parents);
  }

  reduce(reducer, accumulator)
  {
    accumulator = this.leading.reduce(reducer, accumulator);
    accumulator = reducer(this, accumulator);
    accumulator = this.trailing.reduce(reducer, accumulator);
    return accumulator;
  }

  static from_json(json, position, source)
  {
    let leading_list = array_map_reduce(
      json.leading,
      (json, position) => EditableSyntax.from_json(json, position, source),
      (json, position) => json.width + position,
      position);
    let leading = EditableSyntax.to_list(leading_list);
    let token_position = position + leading.width;
    let token_text = source.substring(
      token_position, token_position + json.width);
    let trailing_position = token_position + json.width;
    let trailing_list = array_map_reduce(
      json.trailing,
      (json, position) => EditableSyntax.from_json(json, position, source),
      (json, position) => json.width + position,
      trailing_position);
    let trailing = EditableSyntax.to_list(trailing_list);
    return EditableToken.factory(json.kind, leading, trailing, token_text);
  }

  get children_keys()
  {
    if (EditableToken._children_keys == null)
      EditableToken._children_keys = ['leading', 'trailing'];
    return EditableToken._children_keys;
  }
}

EDITABLE_NO_TEXT_TOKENS
EDITABLE_GIVEN_TEXT_TOKENS
EDITABLE_VARIABLE_TEXT_TOKENS

class EditableTrivia extends EditableSyntax
{
  constructor(trivia_kind, text)
  {
    super(trivia_kind, {});
    this._text = text;
  }
  get text() { return this._text; }
  get full_text() { return this.text; }
  get width() { return this.text.length; }
  get is_trivia() { return true; }

  static from_json(json, position, source)
  {
    let trivia_text = source.substring(position, position + json.width);
    switch(json.kind)
    {
STATIC_FROM_JSON_TRIVIA
      default: throw 'unexpected json kind: ' + json.kind; // TODO: Better error
    }
  }

  rewrite(rewriter, parents)
  {
    return rewriter(this, parents);
  }
  get children_keys()
  {
    return [];
  }
}

TRIVIA_CLASSES

class Missing extends EditableSyntax
{
  constructor()
  {
    super('missing', {});
  }
  get is_missing() { return true; }
  static get missing() { return Missing._missing; }
  static from_json(json, position, source)
  {
    return Missing._missing;
  }
  rewrite(rewriter, parents)
  {
    return rewriter(this, parents);
  }
  get children_keys()
  {
    return [];
  }
}
Missing._missing = new Missing();

EDITABLE_SYNTAX

function from_json(json)
{
  return EditableSyntax.from_json(json.parse_tree, 0, json.program_text);
}

exports.from_json = from_json;
exports.EditableSyntax = EditableSyntax;
exports.EditableList = EditableList;
exports.EditableToken = EditableToken;
EXPORTS_NO_TEXT_TOKENS
EXPORTS_GIVEN_TEXT_TOKENS
EXPORTS_VARIABLE_TEXT_TOKENS
exports.EditableTrivia = EditableTrivia;
EXPORTS_TRIVIA
EXPORTS_SYNTAX"

  let full_fidelity_javascript =
  {
    filename = full_fidelity_path_prefix ^ "js/full_fidelity_editable.js";
    template = full_fidelity_javascript_template;
    transformations = [
      { pattern = "FROM_JSON_SYNTAX"; func = to_from_json };
      { pattern = "EDITABLE_SYNTAX"; func = to_editable_syntax };
      { pattern = "EXPORTS_SYNTAX"; func = to_exports_syntax };
    ];
    token_no_text_transformations = [
      { token_pattern = "EDITABLE_NO_TEXT_TOKENS";
        token_func = map_and_concat to_editable_no_text };
      { token_pattern = "FACTORY_NO_TEXT_TOKENS";
        token_func = map_and_concat to_factory_no_text };
      { token_pattern = "EXPORTS_NO_TEXT_TOKENS";
        token_func = map_and_concat to_export_token }];
    token_given_text_transformations = [
      { token_pattern = "EDITABLE_GIVEN_TEXT_TOKENS";
        token_func = map_and_concat to_editable_given_text };
      { token_pattern = "FACTORY_GIVEN_TEXT_TOKENS";
        token_func = map_and_concat to_factory_given_text };
      { token_pattern = "EXPORTS_GIVEN_TEXT_TOKENS";
        token_func = map_and_concat to_export_token }];
    token_variable_text_transformations = [
      { token_pattern = "EDITABLE_VARIABLE_TEXT_TOKENS";
        token_func = map_and_concat to_editable_variable_text };
      { token_pattern = "FACTORY_VARIABLE_TEXT_TOKENS";
        token_func = map_and_concat to_factory_variable_text };
      { token_pattern = "EXPORTS_VARIABLE_TEXT_TOKENS";
        token_func = map_and_concat to_export_token }];
    trivia_transformations = [
      { trivia_pattern = "FROM_JSON_TRIVIA";
        trivia_func = map_and_concat trivia_from_json };
      { trivia_pattern = "STATIC_FROM_JSON_TRIVIA";
        trivia_func = map_and_concat trivia_static_from_json };
      { trivia_pattern = "TRIVIA_CLASSES";
        trivia_func = map_and_concat trivia_classes };
      { trivia_pattern = "EXPORTS_TRIVIA";
        trivia_func = map_and_concat to_export_trivia }
    ];
    aggregate_transformations = [];
  }

end (* GenerateFFJavaScript *)


module GenerateFFTokenKind = struct

  let given_text_width =
    let folder acc x = max acc (String.length x.token_text) in
    List.fold_left folder 0 given_text_tokens

  let to_kind_declaration x =
    sprintf "  | %s\n" x.token_kind

  let add_guard_or_pad :
  cond:(bool * string) -> ?else_cond:(bool * string) -> string -> string =
  fun ~cond:(cond, guard) ?else_cond guards ->
    let pad str = String.make (String.length str) ' ' in
    let is_only_spaces str = (str = (pad str)) in
    let make_same_length str1 str2 =
      let blanks n = try String.make n ' ' with Invalid_argument _ -> "" in
      let (len1, len2) = (String.length str1, String.length str2) in
      let str1 = str1 ^ (blanks (len2 - len1)) in
      let str2 = str2 ^ (blanks (len1 - len2)) in
      (str1, str2) in
    let (else_cond, else_guard) = match else_cond with
      | Some (cond, guard) -> cond, guard
      | None -> false, "" in
    let prefix = if cond || else_cond
      then if is_only_spaces guards then "when " else "&&   "
      else "     " in
    let (guard, else_guard) = make_same_length guard else_guard in
    let guard = if cond then guard
      else if else_cond then else_guard
      else pad guard in
    guards ^ prefix ^ guard ^ " "

  let to_from_string x =
    let token_text = escape_token_text x.token_text in
    let spacer_width = given_text_width - String.length token_text in
    let spacer = String.make spacer_width ' ' in
    let guards = add_guard_or_pad ""
      ~cond:(x.is_xhp, "(is_hack || allow_xhp)")
      ~else_cond:(x.hack_only, "is_hack") in
    sprintf "  | \"%s\"%s %s-> Some %s\n" token_text spacer guards x.token_kind

  let to_to_string x =
    let token_text = escape_token_text x.token_text in
    sprintf ("  | " ^^ token_kind_fmt ^^ " -> \"%s\"\n")
      x.token_kind token_text

  let to_is_variable_text x =
    sprintf "  | %s -> true\n" x.token_kind

  let full_fidelity_token_kind_template = make_header MLStyle "" ^ "

type t =
  (* No text tokens *)
KIND_DECLARATIONS_NO_TEXT  (* Given text tokens *)
KIND_DECLARATIONS_GIVEN_TEXT  (* Variable text tokens *)
KIND_DECLARATIONS_VARIABLE_TEXT
  [@@deriving show]

let from_string keyword ~is_hack ~allow_xhp =
  match keyword with
  | \"true\"         -> Some BooleanLiteral
  | \"false\"        -> Some BooleanLiteral
FROM_STRING_GIVEN_TEXT  | _              -> None

let to_string kind =
  match kind with
(* No text tokens *)
TO_STRING_NO_TEXT  (* Given text tokens *)
TO_STRING_GIVEN_TEXT  (* Variable text tokens *)
TO_STRING_VARIABLE_TEXT

let is_variable_text kind =
  match kind with
IS_VARIABLE_TEXT_VARIABLE_TEXT  | _ -> false
"
  let full_fidelity_token_kind =
  {
    filename = full_fidelity_path_prefix ^ "full_fidelity_token_kind.ml";
    template = full_fidelity_token_kind_template;
    transformations = [];
    token_no_text_transformations = [
      { token_pattern = "KIND_DECLARATIONS_NO_TEXT";
        token_func = map_and_concat to_kind_declaration };
      { token_pattern = "TO_STRING_NO_TEXT";
        token_func = map_and_concat to_to_string }];
    token_given_text_transformations = [
      { token_pattern = "KIND_DECLARATIONS_GIVEN_TEXT";
        token_func = map_and_concat to_kind_declaration };
      { token_pattern = "FROM_STRING_GIVEN_TEXT";
        token_func = map_and_concat to_from_string };
      { token_pattern = "TO_STRING_GIVEN_TEXT";
        token_func = map_and_concat to_to_string }];
    token_variable_text_transformations = [
      { token_pattern = "KIND_DECLARATIONS_VARIABLE_TEXT";
        token_func = map_and_concat to_kind_declaration };
      { token_pattern = "TO_STRING_VARIABLE_TEXT";
        token_func = map_and_concat to_to_string };
      { token_pattern = "IS_VARIABLE_TEXT_VARIABLE_TEXT";
        token_func = map_and_concat to_is_variable_text }];
    trivia_transformations = [];
    aggregate_transformations = [];
  }

end (* GenerateFFTokenKind *)

let () =
  generate_file GenerateFFSyntaxType.full_fidelity_syntax_type;
  generate_file GenerateFFSyntaxSig.full_fidelity_syntax_sig;
  generate_file GenerateFFValidatedSyntax.full_fidelity_validated_syntax;
  generate_file GenerateFFTriviaKind.full_fidelity_trivia_kind;
  generate_file GenerateFFSyntax.full_fidelity_syntax;
  generate_file GenerateFFSyntaxKind.full_fidelity_syntax_kind;
  generate_file GenerateFFJavaScript.full_fidelity_javascript;
  generate_file GenerateFFTokenKind.full_fidelity_token_kind;
  generate_file GenerateFFJSONSchema.full_fidelity_json_schema;
  generate_file
    GenerateFFSmartConstructors.full_fidelity_smart_constructors;
  generate_file
    GenerateFFVerifySmartConstructors.full_fidelity_verify_smart_constructors;
  generate_file
    GenerateFFSyntaxSmartConstructors.full_fidelity_syntax_smart_constructors;
  generate_file
    GenerateFlattenSmartConstructors.flatten_smart_constructors;
  generate_file
    GenerateFFParserSig.full_fidelity_parser_sig;
  generate_file
    GenerateFFSmartConstructorsWrappers
      .full_fidelity_smart_constructors_wrappers
