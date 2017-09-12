(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Printf
open Full_fidelity_schema

let full_fidelity_path_prefix = "hphp/hack/src/parser/"

type comment_style =
  | CStyle
  | MLStyle
  | HHStyle

let make_header comment_style (header_comment : string) : string =
  let open_char, close_char = match comment_style with
  | CStyle -> "/", '/'
  | MLStyle -> "(", ')'
  | HHStyle -> "<?hh // strict\n/", '/'
  in
  sprintf
"%s**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the \"hack\" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 **
 *
 * THIS FILE IS @%s; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 * This module contains the type describing the structure of a syntax tree.
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
        sprintf "%s_%s = %s x.Syntax.%s_%s"
          x.prefix f
          (validator_of t)
          x.prefix f
      in
      let fields = List.rev_map mapper x.fields in

      let mapper (f,t) =
        sprintf "Syntax.%s_%s = %s x.%s_%s"
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
  | s -> validation_fail SyntaxKind.%s s
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

  exception Validation_failure of SyntaxKind.t * Syntax.t
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
    | _ -> validation_fail SyntaxKind.Token node
  let invalidate_token : Token.t invalidator = fun (value, token) ->
    { Syntax.syntax = Syntax.Token token; value }

  let validate_list_with : 'a . 'a validator -> 'a listesque validator =
    fun validate node ->
      let validate_item i =
        match Syntax.syntax i with
        | Syntax.ListItem { Syntax.list_item; list_separator } ->
          let item = validate list_item in
          let separator = validate_option_with validate_token list_separator in
          i.Syntax.value, (item, separator)
        | _ -> validation_fail SyntaxKind.ListItem i
      in
      let validate_list l =
        try Syntactic (List.map validate_item l) with
        | Validation_failure (SyntaxKind.ListItem, _) ->
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
            { Syntax.list_item = inode; list_separator = iseparator }
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
    let field_width = 50 - String.length x.prefix in
    let fmt = sprintf "%s_%%-%ds: t\n" x.prefix field_width in
    let mapper (f,_) = sprintf (Scanf.format_from_string fmt "%-1s") f in
    let fields = map_and_concat_separated "    ; " mapper x.fields in
    sprintf "  and %s =\n    { %s    }\n"
      x.type_name fields

  let to_syntax x =
    sprintf ("  | " ^^ kind_name_fmt ^^ " of %s\n")
      x.kind_name x.type_name

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
  type t
  val kind: t -> Full_fidelity_token_kind.t
  val to_json: t -> Hh_json.json
end

module type SyntaxValueType = sig
  type t
end

(* This functor describe the shape of a parse tree that has a particular kind of
 * token in the leaves, and a particular kind of value associated with each
 * node.
 *)
module MakeSyntaxType(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  type t = {
    syntax : syntax ;
    value : SyntaxValue.t
  }
PARSE_TREE  and syntax =
  | Token                             of Token.t
  | Missing
  | SyntaxList                        of t list
SYNTAX
end (* MakeSyntaxType *)

module MakeValidated(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  type 'a value = SyntaxValue.t * 'a
  (* TODO: Different styles of list seem to only happen in predetermined places,
   * so split this out again into specific variants
   *)
  type 'a listesque =
  | Syntactic of ('a value * Token.t option value) value list
  | NonSyntactic of 'a value list
  | MissingList
  | SingletonList of 'a value
AGGREGATE_TYPESVALIDATED_SYNTAX
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


module GenerateFFSyntax = struct

  let to_parse_tree x =
    let mapper (f,_) = sprintf "      %s_%s: t;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    sprintf "    and %s = {\n%s    }\n"
      x.type_name fields

  let to_syntax x =
    sprintf ("    | " ^^ kind_name_fmt ^^ " of %s\n")
      x.kind_name x.type_name

  let to_to_kind x =
    sprintf ("      | " ^^ kind_name_fmt ^^ " _ -> SyntaxKind.%s\n")
      x.kind_name x.kind_name

  let to_type_tests x =
    sprintf ("    let is_" ^^ type_name_fmt ^^ " = has_kind SyntaxKind.%s\n")
      x.type_name x.kind_name

  let to_child_list_from_type x =
    let mapper1 (f,_) = sprintf "      %s_%s;\n" x.prefix f in
    let mapper2 (f,_) = sprintf "      %s_%s" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let fields2 = String.concat ",\n" (List.map mapper2 x.fields) in
    sprintf "    let get_%s_children {\n%s    } = (\n%s\n    )\n\n"
      x.type_name fields1 fields2

  let to_children x =
    let mapper (f,_) = sprintf "        %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    sprintf "      | %s {\n%s      } -> [\n%s      ]\n"
      x.kind_name fields fields

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
    let mapper1 (f,_) = sprintf "      %s_%s\n" x.prefix f in
    let mapper2 (f,_) = sprintf "        %s_%s;\n" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let fields2 = map_and_concat mapper2 x.fields in
    sprintf "    let make_%s
%s    =
      from_children SyntaxKind.%s [
%s      ]

"
      x.type_name fields1 x.kind_name fields2

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
module Operator = Full_fidelity_operator

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
      | Token                             _ -> SyntaxKind.Token
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
        Full_fidelity_token_kind.(match Token.kind t with
        | PlusPlus | MinusMinus -> false
        | _ -> true) end
      | _ -> true

    let is_specific_token kind node =
      match syntax node with
      | Token t -> Token.kind t = kind
      | _ -> false


    let is_semicolon  = is_specific_token Full_fidelity_token_kind.Semicolon
    let is_name       = is_specific_token Full_fidelity_token_kind.Name
    let is_construct  = is_specific_token Full_fidelity_token_kind.Construct
    let is_destruct   = is_specific_token Full_fidelity_token_kind.Destruct
    let is_static     = is_specific_token Full_fidelity_token_kind.Static
    let is_private    = is_specific_token Full_fidelity_token_kind.Private
    let is_public     = is_specific_token Full_fidelity_token_kind.Public
    let is_protected  = is_specific_token Full_fidelity_token_kind.Protected
    let is_abstract   = is_specific_token Full_fidelity_token_kind.Abstract
    let is_final      = is_specific_token Full_fidelity_token_kind.Final
    let is_void       = is_specific_token Full_fidelity_token_kind.Void
    let is_left_brace = is_specific_token Full_fidelity_token_kind.LeftBrace
    let is_ellipsis   = is_specific_token Full_fidelity_token_kind.DotDotDot
    let is_comma      = is_specific_token Full_fidelity_token_kind.Comma
    let is_array      = is_specific_token Full_fidelity_token_kind.Array
    let is_var        = is_specific_token Full_fidelity_token_kind.Var

CHILD_LIST_FROM_TYPE

    (* The order that the children are returned in should match the order
       that they appear in the source text *)
    let children node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | SyntaxList x -> x
CHILDREN

    let children_names node =
      match node.syntax with
      | Missing -> []
      | Token _ -> []
      | SyntaxList _ -> []
CHILDREN_NAMES

    let rec to_json node =
      let open Hh_json in
      let ch = match node.syntax with
      | Token t -> [ \"token\", Token.to_json t ]
      | SyntaxList x -> [ (\"elements\", JSON_Array (List.map to_json x)) ]
      | _ ->
        let rec aux acc c n =
          match c, n with
          | ([], []) -> acc
          | ((hc :: tc), (hn :: tn)) ->
            aux ((hn, to_json hc) :: acc) tc tn
          | _ -> failwith \"mismatch between children and names\" in
        List.rev (aux [] (children node) (children_names node)) in
      let k = (\"kind\", JSON_String (SyntaxKind.to_string (kind node))) in
      JSON_Object (k :: ch)

    let binary_operator_kind b =
      match syntax b.binary_operator with
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
        Full_fidelity_syntax_kind.t -> t list -> SyntaxValue.t
      val value_from_token: Token.t -> SyntaxValue.t
    end

    module WithValueBuilder(ValueBuilder: ValueBuilderType) = struct
      let from_children kind ts =
        let syntax = syntax_from_children kind ts in
        let value = ValueBuilder.value_from_children kind ts in
        make syntax value

      let make_token token =
        let syntax = Token token in
        let value = ValueBuilder.value_from_token token in
        make syntax value

      let make_missing () =
        from_children SyntaxKind.Missing []

      (* An empty list is represented by Missing; everything else is a
        SyntaxList, even if the list has only one item. *)
      let make_list items =
        match items with
        | [] -> make_missing()
        | _ -> from_children SyntaxKind.SyntaxList items

CONSTRUCTOR_METHODS

      (* Takes a node and a function from token to token and returns a node with
      the function applied to the leading token, if there is one. *)
      let modify_leading_token node f =
        let rec aux nodes =
          match nodes with
          | [] -> (nodes, false)
          | h :: t ->
            begin
            match get_token h with
            | Some token ->
              let new_token = f token in
              let new_token = make_token new_token in
              ((new_token :: t), true)
            | None ->
              let (new_children, success) = aux (children h) in
              if success then
                let new_head = from_children (kind h) new_children in
                ((new_head :: t), true)
              else
                let (new_tail, success) = aux t in
                if success then
                  ((h :: new_tail), true)
                else
                  (nodes, false)
              end in
        let (results, _) = aux [node] in
        match results with
        | [] -> failwith
          \"how did we get a smaller list out than we started with?\"
        | h :: [] -> h
        | _ -> failwith
          \"how did we get a larger list out than we started with?\"

      (* Takes a node and a function from token to token and returns a node with
      the function applied to the trailing token, if there is one. *)
      let modify_trailing_token node f =
        (* We have a list of nodes, reversed, so the rightmost node is first. *)
        let rec aux nodes =
          match nodes with
          | [] -> (nodes, false)
          | h :: t ->
            begin
            match get_token h with
            | Some token ->
              let new_token = f token in
              let new_token = make_token new_token in
              ((new_token :: t), true)
            | None ->
              let (new_children, success) = aux (List.rev (children h)) in
              if success then
                let new_children = List.rev new_children in
                let new_head = from_children (kind h) new_children in
                ((new_head :: t), true)
              else
                let (new_tail, success) = aux t in
                if success then
                  ((h :: new_tail), true)
                else
                  (nodes, false)
              end in
        let (results, _) = aux [node] in
        match results with
        | [] -> failwith
          \"how did we get a smaller list out than we started with?\"
        | h :: [] -> h
        | _ -> failwith
          \"how did we get a larger list out than we started with?\"

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
      { pattern = "CHILD_LIST_FROM_TYPE"; func = to_child_list_from_type };
      { pattern = "CHILDREN"; func = to_children };
      { pattern = "CHILDREN_NAMES"; func = to_children_names };
      { pattern = "SYNTAX_FROM_CHILDREN"; func = to_syntax_from_children };
      { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods };
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }

end (* GenerateFFSyntax *)

module GenerateFFTriviaKind = struct

  let to_trivia { trivia_kind; trivia_text } =
    sprintf "  | %s\n" trivia_kind

  let to_to_string { trivia_kind; trivia_text } =
    sprintf ("  | " ^^ trivia_kind_fmt ^^ " -> \"%s\"\n")
      trivia_kind trivia_text

  let full_fidelity_trivia_kind_template = make_header MLStyle "" ^ "

type t =
TRIVIA
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
  | Token
  | Missing
  | SyntaxList
TOKENS

let to_string kind =
  match kind with
  | Missing                           -> \"missing\"
  | Token                             -> \"token\"
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

module GenerateFFHack = struct

  let to_from_json x =
    sprintf "    case '%s':
      return %s::from_json($json, $position, $source);
" x.description x.kind_name

  let to_from_json_trivia { trivia_kind; trivia_text } =
    sprintf "    case '%s':
      return %s::from_json($json, $position, $source);
" trivia_text trivia_kind

  let to_static_from_json_trivia { trivia_kind; trivia_text } =
    sprintf "      case '%s':
        return new %s($trivia_text);
" trivia_text trivia_kind

  let to_classes_trivia { trivia_kind; trivia_text } =
    sprintf "class %s extends EditableTrivia {
  public function __construct(string $text) {
    parent::__construct('%s', $text);
  }
  public function with_text(string $text): %s {
    return new %s($text);
  }
}

" trivia_kind trivia_text trivia_kind trivia_kind


  let to_editable_syntax x =
    let ctor_mapper (f,_) = sprintf "EditableSyntax $%s" f in
    let ctor = map_and_concat_separated ",\n    " ctor_mapper x.fields in
    let ctor2_mapper (f,_) = sprintf "$%s" f in
    let ctor2 = map_and_concat_separated ",\n        " ctor2_mapper x.fields in
    let props_mapper (f,_) =
      sprintf "private EditableSyntax $_%s;" f in
    let props = map_and_concat_separated "\n  " props_mapper x.fields in
    let getters_mapper (f,_) =
      sprintf "public function %s(): EditableSyntax {
    return $this->_%s;
  }" f f in
    let getters = map_and_concat_separated "\n  " getters_mapper x.fields in

    let assignments_mapper (f,_) =
      sprintf "$this->_%s = $%s;" f f in
    let assignments = map_and_concat_separated
      "\n    " assignments_mapper x.fields in
    let withs_mapper (f,_) =
      let inner_mapper (f_inner,_) =
        let prefix = if f_inner = f then "$" else "$this->_" in
        sprintf "%s%s" prefix f_inner in
      let inner = map_and_concat_separated ",\n      " inner_mapper x.fields in
      sprintf "public function with_%s(EditableSyntax $%s): %s {
    return new %s(
      %s);
  }"
        f f x.kind_name x.kind_name inner in
    let withs = map_and_concat_separated "\n  " withs_mapper x.fields in
    let rewriter_mapper (f,_) =
      sprintf "$%s = $this->%s()->rewrite($rewriter, $new_parents);" f f in
    let rewriter =
      map_and_concat_separated "\n    " rewriter_mapper x.fields in
    let condition_mapper (f,_) = sprintf "$%s === $this->%s()" f f in
    let condition =
      map_and_concat_separated " &&\n      " condition_mapper x.fields in
    let json_mapper (f,_) = sprintf
      "$%s = EditableSyntax::from_json(
      $json->%s_%s, $position, $source);
    $position += $%s->width();" f x.prefix f f in
    let json = map_and_concat_separated "\n    " json_mapper x.fields in
    let children_mapper (f,_) = sprintf "yield $this->_%s;" f in
    let children =
      map_and_concat_separated "\n    " children_mapper x.fields in
    sprintf
"final class %s extends EditableSyntax {
  %s
  public function __construct(
    %s) {
    parent::__construct('%s');
    %s
  }
  %s
  %s

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): ?EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    %s
    if (
      %s) {
      return $rewriter($this, $parents ?? []);
    } else {
      return $rewriter(new %s(
        %s), $parents ?? []);
    }
  }

  public static function from_json(mixed $json, int $position, string $source) {
    %s
    return new %s(
        %s);
  }
  public function children(): Generator<string, EditableSyntax, void> {
    %s
    yield break;
  }
}
" x.kind_name props ctor x.description assignments getters withs rewriter
  condition x.kind_name ctor2 json x.kind_name ctor2 children

  let to_editable_no_text x =
    sprintf "final class %sToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('%s', $leading, $trailing, '');
  }

  public function with_leading(EditableSyntax $leading): %sToken {
    return new %sToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): %sToken {
    return new %sToken($this->leading(), $trailing);
  }
}
" x.token_kind x.token_text x.token_kind x.token_kind x.token_kind x.token_kind

let to_editable_given_text x =
  sprintf "final class %sToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing) {
    parent::__construct('%s', $leading, $trailing, '%s');
  }

  public function with_leading(EditableSyntax $leading): %sToken {
    return new %sToken($leading, $this->trailing());
  }

  public function with_trailing(EditableSyntax $trailing): %sToken {
    return new %sToken($this->leading(), $trailing);
  }
}
" x.token_kind x.token_text x.token_text x.token_kind x.token_kind
  x.token_kind x.token_kind

  let to_editable_variable_text x =
    sprintf "final class %sToken extends EditableToken {
  public function __construct(
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('%s', $leading, $trailing, $text);
  }

  public function with_text(string $text): %sToken {
    return new %sToken($this->leading(), $this->trailing(), $text);
  }

  public function with_leading(EditableSyntax $leading): %sToken {
    return new %sToken($leading, $this->trailing(), $this->text());
  }

  public function with_trailing(EditableSyntax $trailing): %sToken {
    return new %sToken($this->leading(), $trailing, $this->text());
  }
}
" x.token_kind x.token_text
  x.token_kind x.token_kind
  x.token_kind x.token_kind
  x.token_kind x.token_kind

  let to_factory_no_text x =
    sprintf
"    case '%s':
       return new %sToken($leading, $trailing);
"
      x.token_text x.token_kind

  let to_factory_given_text = to_factory_no_text

  let to_factory_variable_text x =
    sprintf
"    case '%s':
       return new %sToken($leading, $trailing, $token_text);
"
      x.token_text x.token_kind

  let full_fidelity_hack_template = make_header HHStyle "" ^ "

require_once 'full_fidelity_parser.php';

abstract class EditableSyntax implements ArrayAccess {
  private string $_syntax_kind;
  protected ?int $_width;
  public function __construct(string $syntax_kind) {
    $this->_syntax_kind = $syntax_kind;
  }

  public function offsetExists (mixed $offset): bool {
    return $offset === 0;
  }

  public function offsetGet (mixed $offset): mixed {
    return $this;
  }

  public function offsetSet (mixed $offset, mixed $value): void {
  }

  public function offsetUnset (mixed $offset): void {
  }

  public function syntax_kind(): string {
    return $this->_syntax_kind;
  }

  public abstract function children():
    Generator<string, EditableSyntax, void>;

  public function preorder(): Continuation<EditableSyntax> {
    yield $this;
    foreach($this->children() as $name => $child)
      foreach($child->preorder() as $descendant)
        yield $descendant;
  }

  private function _parented_preorder(array<EditableSyntax> $parents):
    Continuation<(EditableSyntax, array<EditableSyntax>)> {
    $new_parents = $parents;
    array_push($new_parents, $this);
    yield tuple($this, $parents);
    foreach($this->children() as $name => $child)
      foreach($child->_parented_preorder($new_parents) as $descendant)
        yield $descendant;
  }

  public function parented_preorder():
    Continuation<(EditableSyntax, array<EditableSyntax>)> {
    return $this->_parented_preorder([]);
  }

  public function postorder(): Continuation<EditableSyntax> {
    foreach($this->children() as $name => $child)
      foreach($child->preorder() as $descendant)
        yield $descendant;
    yield $this;
  }

  public function is_token(): bool {
    return false;
  }

  public function is_trivia(): bool {
    return false;
  }

  public function is_list(): bool {
    return false;
  }

  public function is_missing(): bool {
    return false;
  }

  public function width(): int {
    if ($this->_width === null) {
      $width = 0;
      /* TODO: Make an accumulation sequence operator */
      foreach ($this->children() as $name => $node) {
        $width += $node->width();
      }
      $this->_width = $width;
      return $width;
    } else {
      return $this->_width;
    }
  }

  public function full_text(): string {
    /* TODO: Make an accumulation sequence operator */
    $s = '';
    foreach ($this->children() as $name => $node) {
      $s .= $node->full_text();
    }
    return $s;
  }

  public static function from_json(mixed $json, int $position, string $source) {
    switch($json->kind) {
    case 'token':
      return EditableToken::from_json($json->token, $position, $source);
    case 'list':
      return EditableList::from_json($json, $position, $source);
FROM_JSON_TRIVIA
    case 'missing':
      return Missing::missing();
FROM_JSON_SYNTAX
    default:
      throw new Exception('unexpected json kind: ' . $json->kind);
      // TODO: Better exception
    }
  }

  public function to_array(): array<EditableSyntax> {
    return [$this];
  }

  public function reduce<TAccumulator>(
    (function
      ( EditableSyntax,
        TAccumulator,
        array<EditableSyntax>): TAccumulator) $reducer,
    TAccumulator $accumulator,
    ?array<EditableSyntax> $parents = null): TAccumulator {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    foreach($this->children() as $child) {
      $accumulator = $child->reduce($reducer, $accumulator, $new_parents);
    }
    return $reducer($this, $accumulator, $parents ?? []);
  }

  // Returns all the parents (and the node itself) of the first node
  // that matches a predicate, or [] if there is no such node.
  public function find_with_parents(
    (function(EditableSyntax):bool) $predicate,
    ?array<EditableSyntax> $parents = null): array<EditableSyntax> {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    if ($predicate($this))
      return $new_parents;
    foreach($this->children() as $child) {
      $result = $child->find_with_parents($predicate, $new_parents);
      if (count($result) != 0)
        return $result;
    }
    return [];
  }

  // Returns a list of nodes that match a predicate.
  public function filter(
    (function(EditableSyntax, ?array<EditableSyntax>):bool) $predicate):
      array<EditableSyntax> {
    $reducer = ($node, $acc, $parents) ==> {
      if ($predicate($node, $parents))
        array_push($acc, $node);
      return $acc;
    };
    return $this->reduce($reducer, []);
  }

  public function of_syntax_kind(string $kind): Continuation<EditableSyntax> {
    foreach($this->preorder() as $child)
      if ($child->syntax_kind() === $kind)
        yield $child;
  }

  public function remove_where(
    (function(EditableSyntax, ?array<EditableSyntax>):bool) $predicate):
      array<EditableSyntax> {
    return $this->rewrite(
      ($node, $parents) ==>
        $predicate($node, $parents) ? Missing::missing() : $node);
  }

  public function without(EditableSyntax $target): EditableSyntax {
    return $this->remove_where(($node, $parents) ==> $node === $target);
  }

  public function replace(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {
    return $this->rewrite(
      ($node, $parents) ==> $node === $target ? $new_node : $node);
  }

  public function leftmost_token(): ?EditableSyntax {
    if ($this->is_token())
      return $this;
    foreach($this->children() as $child)
    {
      if (!$child->is_missing())
        return $child->leftmost_token();
    }
    return null;
  }

  public function rightmost_token(): ?EditableSyntax {
    if ($this->is_token())
      return $this;

    // TODO: Better way to reverse a sequence?
    foreach (array_reverse(iterator_to_array($this->children())) as $child) {
      if (!$child->is_missing())
        return $child->rightmost_token();
    }
    return null;
  }

  public function insert_before(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {
    // Inserting before missing is an error.
    if ($target->is_missing())
      throw new Exception('Target must not be missing in insert_before.');

    // Inserting missing is a no-op
    if ($new_node->is_missing())
      return $this;

    if ($new_node->is_trivia() && !$target->is_trivia()) {
      $token = $target->is_token() ? $target : $target->leftmost_token();
      if ($token === null)
        throw new Exception('Unable to find token to insert trivia.');

      // Inserting trivia before token is inserting to the right end of
      // the leading trivia.
      $new_leading = EditableList::concatenate_lists(
        $token->leading(), $new_node);
      $new_token = $token->with_leading($new_leading);
      return $this->replace($new_token, $token);
    }

    return $this->replace(
      EditableList::concatenate_lists($new_node, $target), $target);
  }

  public function insert_after(
    EditableSyntax $new_node,
    EditableSyntax $target): EditableSyntax {

    // Inserting after missing is an error.
    if ($target->is_missing())
      throw new Exception('Target must not be missing in insert_after.');

    // Inserting missing is a no-op
    if ($new_node->is_missing())
      return $this;

    if ($new_node->is_trivia() && !$target->is_trivia()) {
      $token = $target->is_token() ? $target : $target->rightmost_token();
      if ($token === null)
        throw new Exception('Unable to find token to insert trivia.');

      // Inserting trivia after token is inserting to the left end of
      // the trailing trivia.
      $new_trailing = EditableList::concatenate_lists(
        $new_node, $token->trailing());
      $new_token = $token->with_trailing($new_trailing);
      return $this->replace($new_token, $token);
    }

    return $this->replace(
      EditableSyntax::concatenate_lists($target, $new_node), $target);
  }
}

final class EditableList extends EditableSyntax implements ArrayAccess {
  private array<EditableSyntax> $_children;
  public function __construct(array<EditableSyntax> $children) {
    parent::__construct('list');
    $this->_children = $children;
  }

  public function offsetExists(mixed $offset): bool {
    return array_key_exists($offset, $this->_children);
  }

  public function offsetGet(mixed $offset): mixed {
    return $this->_children[$offset];
  }

  public function offsetSet(mixed $offset, mixed $value): void {
  }

  public function offsetUnset(mixed $offset): void {
  }

  public function is_list(): bool {
    return true;
  }

  public function to_array(): array<EditableSyntax> {
    return $this->_children;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    foreach($this->_children as $key => $node)
      yield $key => $node;
  }

  /* TODO: Getter by index? */

  public static function to_list(
    array<EditableSyntax> $syntax_list): EditableSyntax {
    if (count($syntax_list) === 0)
      return Missing::missing();
    else
      return new EditableList($syntax_list);
  }

  public static function concatenate_lists(
    EditableSyntax $left,
    EditableSyntax $right): EditableSyntax {
    if ($left->is_missing())
      return $right;
    if ($right->is_missing())
      return $left;
    return new EditableList(
      array_merge($left->to_array(), $right->to_array()));
  }

  public static function from_json(mixed $json, int $position, string $source) {
    // TODO Implement array map
    $children = [];
    $current_position = $position;
    foreach($json->elements as $element)
    {
      $child = EditableSyntax::from_json($element, $current_position, $source);
      array_push($children, $child);
      $current_position += $child->width();
    }
    return new EditableList($children);
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
    $dirty = false;
    $new_children = [];
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    foreach ($this->children() as $child)
    {
      $new_child = $child->rewrite($rewriter, $new_parents);
      if ($new_child != $child)
        $dirty = true;
      if ($new_child != null)
      {
        if ($new_child->is_list())
        {
          foreach($new_child->children() as $n)
            array_push($new_children, $n);
        }
        else
          array_push($new_children, $new_child);
      }
    }
    $result = $this;
    if ($dirty) {
      if (count($new_children) === 0)
        $result = Missing::missing();
      else if (count($new_children) === 1)
        $result = $new_children[0];
      else
        $result = new EditableList($new_children);
    }
    return $rewriter($result, $parents ?? []);
  }
}

abstract class EditableToken extends EditableSyntax {
  private string $_token_kind;
  private EditableSyntax $_leading;
  private EditableSyntax $_trailing;

  public function __construct(
    string $token_kind,
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $text) {
    parent::__construct('token');
    $this->_token_kind = $token_kind;
    $this->_text = $text;
    $this->_leading = $leading;
    $this->_trailing = $trailing;
    $this->_width = strlen($text) +
      $leading->width() + $trailing->width();
  }

  public function token_kind(): string {
    return $this->_token_kind;
  }

  public function text(): string {
    return $this->_text;
  }

  public function leading(): EditableSyntax {
    return $this->_leading;
  }

  public function trailing(): EditableSyntax {
    return $this->_trailing;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield 'leading' => $this->leading();
    yield 'trailing' => $this->trailing();
  }

  public function is_token(): bool {
    return true;
  }

  public function full_text(): string {
    return $this->leading()->full_text() .
      $this->text() .
      $this->trailing()->full_text();
  }

  public abstract function with_leading(
    EditableSyntax $leading): EditableToken;

  public abstract function with_trailing(
    EditableSyntax $trailing): EditableToken;

  private static function factory(
    string $token_kind,
    EditableSyntax $leading,
    EditableSyntax $trailing,
    string $token_text) {
    switch($token_kind) {
FACTORY_NO_TEXT_TOKENS
FACTORY_GIVEN_TEXT_TOKENS
FACTORY_VARIABLE_TEXT_TOKENS
      default:
        throw new Exception('unexpected token kind: ' . $token_kind);
        // TODO: Better error
    }
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
    $new_parents = $parents ?? [];
    array_push($new_parents, $this);
    $leading = $this->leading()->rewrite($rewriter, $new_parents);
    $trailing = $this->trailing()->rewrite($rewriter, $new_parents);
    if ($leading === $this->leading() && $trailing === $this->trailing())
      return $rewriter($this, $parents ?? []);
    else
      return $rewriter(EditableToken::factory(
        $this->token_kind(), $leading, $trailing,
        $this->text()), $parents ?? []);
  }

  public function reduce<TAccumulator>(
    (function
      ( EditableSyntax,
        TAccumulator,
        array<EditableSyntax>): TAccumulator) $reducer,
    TAccumulator $accumulator,
    ?array<EditableSyntax> $parents = null): TAccumulator {
    $accumulator = $this->leading()->reduce($reducer, $accumulator);
    $accumulator = $reducer($this, $accumulator, $parents ?? []);
    $accumulator = $this->trailing()->reduce($reducer, $accumulator);
    return $accumulator;
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source): EditableToken {
    $leading_list = fold_map(
      $json->leading,
      ($j, $p) ==> EditableSyntax::from_json($j, $p, $source),
      ($j, $p) ==> $j->width + $p,
      $position);

    $leading = EditableList::to_list($leading_list);
    $token_position = $position + $leading->width();
    $token_width = $json->width;
    $token_text = substr($source, $token_position, $token_width);
    $trailing_position = $token_position + $token_width;
    $trailing_list = fold_map(
      $json->trailing,
      ($j, $p) ==> EditableSyntax::from_json($j, $p, $source),
      ($j, $p) ==> $j->width + $p,
      $trailing_position);
    $trailing = EditableList::to_list($trailing_list);
    return EditableToken::factory(
      $json->kind, $leading, $trailing, $token_text);
  }
}

EDITABLE_NO_TEXT_TOKENS
EDITABLE_GIVEN_TEXT_TOKENS
EDITABLE_VARIABLE_TEXT_TOKENS

abstract class EditableTrivia extends EditableSyntax {
  private string $_text;
  public function __construct(string $trivia_kind , string $text) {
    parent::__construct($trivia_kind);
    $this->_text = $text;
  }

  public function text(): string {
    return $this->_text;
  }

  public function full_text() {
    return $this->_text;
  }

  public function width() {
    return strlen($this->_text);
  }

  public function is_trivia() {
    return true;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield break;
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source) {
    $trivia_text = substr($source, $position, $json->width);
    switch($json->kind) {
STATIC_FROM_JSON_TRIVIA
      default:
        throw new Exception('unexpected json kind: ' . $json->kind);
        // TODO: Better error
    }
  }

public function rewrite(
  ( function
    (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
  ?array<EditableSyntax> $parents = null): EditableSyntax {
    return $rewriter($this, $parents ?? []);
  }
}

TRIVIA_CLASSES

final class Missing extends EditableSyntax {
  private static ?Missing $_missing = null;

  public function __construct() {
    parent::__construct('missing');
  }

  public function is_missing(): bool {
    return true;
  }

  public function children(): Generator<string, EditableSyntax, void> {
    yield break;
  }

  public static function missing(): Missing {
    if (Missing::$_missing === null) {
      $m = new Missing();
      Missing::$_missing = $m;
      return $m;
    } else {
      return Missing::$_missing;
    }
  }

  public static function from_json(
    mixed $json,
    int $position,
    string $source) {
    return Missing::missing();
  }

  public function rewrite(
    ( function
      (EditableSyntax, ?array<EditableSyntax>): ?EditableSyntax ) $rewriter,
    ?array<EditableSyntax> $parents = null): EditableSyntax {
      return $rewriter($this, $parents ?? []);
  }

  public function to_array(): array<EditableSyntax> {
    return [];
  }
}

EDITABLE_SYNTAX

function from_json(mixed $json): EditableSyntax {
  return EditableSyntax::from_json($json->parse_tree, 0, $json->program_text);
}
/* End full_fidelity_editable.php */"

  let full_fidelity_hack =
  {
    filename = full_fidelity_path_prefix ^ "php/full_fidelity_editable.php";
    template = full_fidelity_hack_template;
    transformations = [
      { pattern = "FROM_JSON_SYNTAX"; func = to_from_json };
      { pattern = "EDITABLE_SYNTAX"; func = to_editable_syntax };
    ];
    token_no_text_transformations = [
      { token_pattern = "EDITABLE_NO_TEXT_TOKENS";
        token_func = map_and_concat to_editable_no_text };
      { token_pattern = "FACTORY_NO_TEXT_TOKENS";
        token_func = map_and_concat to_factory_no_text }];
    token_given_text_transformations = [
      { token_pattern = "EDITABLE_GIVEN_TEXT_TOKENS";
        token_func = map_and_concat to_editable_given_text };
      { token_pattern = "FACTORY_GIVEN_TEXT_TOKENS";
        token_func = map_and_concat to_factory_given_text }];
    token_variable_text_transformations = [
      { token_pattern = "EDITABLE_VARIABLE_TEXT_TOKENS";
        token_func = map_and_concat to_editable_variable_text };
      { token_pattern = "FACTORY_VARIABLE_TEXT_TOKENS";
        token_func = map_and_concat to_factory_variable_text }];
    trivia_transformations = [
      { trivia_pattern = "FROM_JSON_TRIVIA";
        trivia_func = map_and_concat to_from_json_trivia };
      { trivia_pattern = "STATIC_FROM_JSON_TRIVIA";
        trivia_func = map_and_concat to_static_from_json_trivia };
      { trivia_pattern = "TRIVIA_CLASSES";
        trivia_func = map_and_concat to_classes_trivia }];
    aggregate_transformations = [];
  }

end (* GenerateFFHack *)


module GenerateFFTokenKind = struct

  let given_text_width =
    let folder acc x = max acc (String.length x.token_text) in
    List.fold_left folder 0 given_text_tokens

  let to_kind_declaration x =
    sprintf "  | %s\n" x.token_kind

  let to_from_string x =
    let spacer_width = given_text_width - String.length x.token_text in
    let spacer = String.make spacer_width ' ' in
    sprintf "  | \"%s\"%s -> Some %s\n" x.token_text spacer x.token_kind

  let to_to_string x =
    sprintf ("  | " ^^ token_kind_fmt ^^ " -> \"%s\"\n")
      x.token_kind x.token_text

  let full_fidelity_token_kind_template = make_header MLStyle "" ^ "

type t =
  (* No text tokens *)
KIND_DECLARATIONS_NO_TEXT  (* Given text tokens *)
KIND_DECLARATIONS_GIVEN_TEXT  (* Variable text tokens *)
KIND_DECLARATIONS_VARIABLE_TEXT

let from_string keyword =
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
        token_func = map_and_concat to_to_string }];
    trivia_transformations = [];
    aggregate_transformations = [];
  }

end (* GenerateFFTokenKind *)

module GenerateFFPositionedSyntax = struct
  exception No_fields_in_schema_node of schema_node

  let to_kind_declaration x =
    sprintf "  | %s\n" x.token_kind

  let to_from_string x =
    sprintf "  | \"%s\" -> Some %s\n" x.token_text x.token_kind

  let to_to_string x =
    sprintf "  | %s -> \"%s\"\n" x.token_kind x.token_text

  let full_fidelity_positioned_syntax_template = make_header MLStyle "
 * Positioned syntax tree
 *
 * A positioned syntax tree stores the original source text,
 * the offset of the leading trivia, the width of the leading trivia,
 * node proper, and trailing trivia. From all this information we can
 * rapidly compute the absolute position of any portion of the node,
 * or the text." ^ "

module SyntaxTree = Full_fidelity_syntax_tree
module SourceText = Full_fidelity_source_text
module PositionedToken = Full_fidelity_positioned_token

module SyntaxWithPositionedToken =
  Full_fidelity_syntax.WithToken(PositionedToken)

module PositionedSyntaxValue = struct
  type t = {
    source_text: SourceText.t;
    offset: int; (* Beginning of first trivia *)
    leading_width: int;
    width: int; (* Width of node, not counting trivia *)
    trailing_width: int;
  }

  let make source_text offset leading_width width trailing_width =
    { source_text; offset; leading_width; width; trailing_width }

  let source_text value =
    value.source_text

  let start_offset value =
    value.offset

  let leading_width value =
    value.leading_width

  let width value =
    value.width

  let trailing_width value =
    value.trailing_width
end

open Core
include SyntaxWithPositionedToken.WithSyntaxValue(PositionedSyntaxValue)
module Validated =
  Full_fidelity_validated_syntax.Make(PositionedToken)(PositionedSyntaxValue)

let source_text node =
  PositionedSyntaxValue.source_text (value node)

let leading_width node =
  PositionedSyntaxValue.leading_width (value node)

let width node =
  PositionedSyntaxValue.width (value node)

let trailing_width node =
  PositionedSyntaxValue.trailing_width (value node)

let full_width node =
  (leading_width node) + (width node) + (trailing_width node)

let leading_start_offset node =
  PositionedSyntaxValue.start_offset (value node)

let leading_end_offset node =
  let w = (leading_width node) - 1 in
  let w = if w < 0 then 0 else w in
  (leading_start_offset node) + w

let start_offset node =
  (leading_start_offset node) + (leading_width node)

let end_offset node =
  let w = (width node) - 1 in
  let w = if w < 0 then 0 else w in
  (start_offset node) + w

let trailing_start_offset node =
  (leading_start_offset node) + (leading_width node) + (width node)

let trailing_end_offset node =
  let w = (full_width node) - 1 in
  let w = if w < 0 then 0 else w in
  (leading_start_offset node) + w

let leading_start_position node =
  SourceText.offset_to_position (source_text node) (leading_start_offset node)

let leading_end_position node =
  SourceText.offset_to_position (source_text node) (leading_end_offset node)

let start_position node =
  SourceText.offset_to_position (source_text node) (start_offset node)

let end_position node =
  SourceText.offset_to_position (source_text node) (end_offset node)

let trailing_start_position node =
  SourceText.offset_to_position (source_text node) (trailing_start_offset node)

let trailing_end_position node =
  SourceText.offset_to_position (source_text node) (trailing_end_offset node)

let leading_span node =
  ((leading_start_position node), (leading_end_position node))

let span node =
  ((start_position node), (end_position node))

let trailing_span node =
  ((trailing_start_position node), (trailing_end_position node))

let full_span node =
  ((leading_start_position node), (trailing_end_position node))

let full_text node =
  SourceText.sub
    (source_text node) (leading_start_offset node) (full_width node)

let leading_text node =
  SourceText.sub
    (source_text node)
    (leading_start_offset node)
    (leading_width node)

let trailing_text node =
  SourceText.sub
    (source_text node) ((end_offset node) + 1) (trailing_width node)

let text node =
  SourceText.sub (source_text node) (start_offset node) (width node)

(* Takes a node and an offset; produces the descent through the parse tree
   to that position. *)
let parentage node position =
  let rec aux nodes position acc =
    match nodes with
    | [] -> acc
    | h :: t ->
      let width = full_width h in
      if position < width then
        aux (children h) position (h :: acc)
      else
        aux t (position - width) acc in
  aux [node] position []

module FromMinimal = struct
  module SyntaxKind = Full_fidelity_syntax_kind
  module M = Full_fidelity_minimal_syntax

  exception Multiplicitous_conversion_result of int

  type todo =
  | Build of (M.t * int * todo)
  | Convert of (M.t * todo)
  | Done

  let from_minimal source_text (node : M.t) : t =
    let make_syntax minimal_node positioned_syntax offset =
      let leading_width = M.leading_width minimal_node in
      let width = M.width minimal_node in
      let trailing_width = M.trailing_width minimal_node in
      let value = PositionedSyntaxValue.make
        source_text offset leading_width width trailing_width in
      make positioned_syntax value
    in
    let build minimal_t (results : t list) : syntax * t list =
      match M.kind minimal_t, results with
      | SyntaxKind.SyntaxList, _ ->
        let len =
          match M.syntax minimal_t with
          | M.SyntaxList l -> List.length l
          | _ -> failwith \"SyntaxKind out of sync with Syntax (impossible).\"
        in
        let rec aux ls n rs =
          match n, rs with
          | 0, _ -> ls, rs
          | _, [] -> failwith \"Rebuilding list with insufficient elements.\"
          | n, (r::rs) -> aux (r :: ls) (n - 1) rs
        in
        let ls, rs = aux [] len results in
        SyntaxList ls, rs
BUILD_CASES
      | _ ->
        failwith @@ Printf.sprintf
          \"BUILD: Failed to build %s with %d results.\"
          (SyntaxKind.to_string @@ M.kind minimal_t)
          (List.length results)
    in

    let rec dispatch (offset : int) (todo : todo) (results : t list) : t =
      match todo with
      | Build (node, node_offset, todo) ->
        let syntax, results = build node results in
        let results = make_syntax node syntax node_offset :: results in
        dispatch offset todo results
      | Convert (n, todo) -> convert offset todo results n
      | Done ->
        (match results with
        | [result] -> result
        | _  -> raise @@ Multiplicitous_conversion_result (List.length results)
        )
    and convert (offset : int) (todo : todo) (results : t list) : M.t -> t = function
    | { M.syntax = M.Token token; _ } as minimal_t ->
      let token = PositionedToken.from_minimal source_text token offset in
      let syntax = Token token in
      let node = make_syntax minimal_t syntax offset in
      let offset = offset + M.full_width minimal_t in
      dispatch offset todo (node :: results)
    | { M.syntax = M.Missing; _ } as minimal_t ->
      let node = make_syntax minimal_t Missing offset in
      dispatch offset todo (node :: results)
    | { M.syntax = M.SyntaxList l; _ } as minimal_t ->
      let todo = Build (minimal_t, offset, todo) in
      let todo = List.fold_right ~f:(fun n t -> Convert (n,t)) l ~init:todo in
      dispatch offset todo results
CONVERT_CASES    in
    convert 0 Done [] node
end

let from_minimal = FromMinimal.from_minimal

let from_tree tree =
  from_minimal (SyntaxTree.text tree) (SyntaxTree.root tree)

"

  let to_convert_cases x =
    let open Printf in
    let fields, first, other =
      match List.map (fun (f,_) -> sprintf "%s_%s" x.prefix f) x.fields with
      | (first :: other) as fields -> fields, first, other
      | _ -> raise (No_fields_in_schema_node x)
    in
    let fields =
      String.concat "\n        ; " @@
        List.map (fun (f,_) -> sprintf "M.%s_%s" x.prefix f) x.fields
    in
    let todos =
      String.concat "" @@
        List.rev_map (sprintf "let todo = Convert (%s, todo) in\n        ") other
    in
    sprintf
"    | { M.syntax = M.%s
        { %s
        }
      ; _ } as minimal_t ->
        let todo = Build (minimal_t, offset, todo) in
        %sconvert offset todo results %s
"
      x.kind_name
      fields
      todos
      first

  let to_build_cases x =
    let open Printf in
    let fields = List.map (fun (f,_) -> sprintf "%s_%s" x.prefix f) x.fields in
    sprintf
"      | SyntaxKind.%s
      , (  %s
        :: results
        ) ->
          %s
          { %s
          }, results
"
      x.kind_name
      (String.concat "\n        :: " @@ List.rev fields)
      x.kind_name
      (String.concat "\n          ; " fields)

  let full_fidelity_positioned_syntax =
  {
    filename = full_fidelity_path_prefix ^ "full_fidelity_positioned_syntax.ml";
    template = full_fidelity_positioned_syntax_template;
    transformations = [
      { pattern = "CONVERT_CASES"; func = to_convert_cases };
      { pattern = "BUILD_CASES";   func = to_build_cases   };
    ];
    token_no_text_transformations = [];
    token_given_text_transformations = [];
    token_variable_text_transformations = [];
    trivia_transformations = [];
    aggregate_transformations = [];
  }

end (* GenerateFFPositionedSyntax *)

let () =
  generate_file GenerateFFSyntaxType.full_fidelity_syntax_type;
  generate_file GenerateFFValidatedSyntax.full_fidelity_validated_syntax;
  generate_file GenerateFFTriviaKind.full_fidelity_trivia_kind;
  generate_file GenerateFFSyntax.full_fidelity_syntax;
  generate_file GenerateFFSyntaxKind.full_fidelity_syntax_kind;
  generate_file GenerateFFJavaScript.full_fidelity_javascript;
  generate_file GenerateFFHack.full_fidelity_hack;
  generate_file GenerateFFTokenKind.full_fidelity_token_kind;
  generate_file GenerateFFJSONSchema.full_fidelity_json_schema;
  generate_file GenerateFFPositionedSyntax.full_fidelity_positioned_syntax;
