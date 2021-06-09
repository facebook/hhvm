(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module OcamlPrintf = Printf
open Hh_prelude
open Printf
open Full_fidelity_schema

let full_fidelity_path_prefix = "hphp/hack/src/parser/"

let rust_keywords =
  [ "as"; "break"; "const"; "continue"; "crate"; "else"; "enum"; "extern";
    "false"; "fn"; "for"; "if"; "impl"; "in"; "let"; "loop"; "match"; "mod";
    "move"; "mut"; "pub"; "ref"; "return"; "self"; "Self"; "static"; "struct";
    "super"; "trait"; "true"; "type"; "unsafe"; "use"; "where"; "while";
    "async"; "await"; "dyn" ]
  [@@ocamlformat "disable"]

let escape_rust_keyword field_name =
  if List.mem rust_keywords field_name ~equal:String.equal then
    sprintf "%s_" field_name
  else
    field_name

type comment_style =
  | CStyle
  | MLStyle
[@@deriving show]

let make_header comment_style (header_comment : string) : string =
  let (open_char, close_char) =
    match comment_style with
    | CStyle -> ("/*", '/')
    | MLStyle -> ("(", ')')
  in
  sprintf
    "%s*
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

let align_fmt : 'a. ('a -> string) -> 'a list -> valign =
 fun f xs ->
  let folder acc x = max acc (String.length (f x)) in
  let width = List.fold_left ~f:folder ~init:0 xs in
  Scanf.format_from_string (sprintf "%%-%ds" width) "%-1s"

let kind_name_fmt = align_fmt (fun x -> x.kind_name) schema

let type_name_fmt = align_fmt (fun x -> x.type_name) schema

let trivia_kind_fmt = align_fmt (fun x -> x.trivia_kind) trivia_kinds

let token_kind_fmt = align_fmt (fun x -> x.token_kind) all_tokens

let omit_syntax_record =
  let names =
    SSet.of_list
      [
        "anonymous_function";
        "closure_type_specifier";
        "function_declaration";
        "function_declaration_header";
        "lambda_expression";
        "lambda_signature";
        "methodish_declaration";
      ]
  in
  (fun x -> not (SSet.mem x.type_name names))

module GenerateFFValidatedSyntax = struct
  let to_validate_functions x =
    if
      String.equal x.kind_name "ErrorSyntax"
      || String.equal x.kind_name "ListItem"
    then
      ""
    else
      let get_type_string t =
        match SMap.find_opt t schema_map with
        | None -> failwith @@ sprintf "Unknown type: %s" t
        | Some t -> t.type_name
      in
      let rec validator_of ?(n = "validate") = function
        | Token -> sprintf "%s_token" n
        | Just t -> sprintf "%s_%s" n (get_type_string t)
        | Aggregate a -> sprintf "%s_%s" n (aggregate_type_name a)
        | ZeroOrMore s -> sprintf "%s_list_with (%s)" n (validator_of ~n s)
        | ZeroOrOne s -> sprintf "%s_option_with (%s)" n (validator_of ~n s)
      in
      let mapper (f, t) =
        sprintf "%s_%s = %s x.%s_%s" x.prefix f (validator_of t) x.prefix f
      in
      let fields = List.rev_map ~f:mapper x.fields in
      let mapper (f, t) =
        sprintf
          "%s_%s = %s x.%s_%s"
          x.prefix
          f
          (validator_of ~n:"invalidate" t)
          x.prefix
          f
      in
      let invalidations = List.map ~f:mapper x.fields in
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
        (String.concat ~sep:"\n    ; " fields)
        x.kind_name
        (* invalidator *)
        x.type_name
        x.type_name
        x.kind_name
        (String.concat ~sep:"\n      ; " invalidations)

  let to_aggregate_validation x =
    let aggregated_types = aggregation_of x in
    let (prefix, trim) = aggregate_type_pfx_trim x in
    let compact = Str.global_replace (Str.regexp trim) "" in
    let valign = align_fmt (fun x -> compact x.kind_name) aggregated_types in
    let type_name = aggregate_type_name x in
    let make_validation_clause ty =
      (* TODO: clean up *)
      sprintf
        "
    | Syntax.%s _ -> tag validate_%s (fun x -> %s%s x) x"
        ty.kind_name
        ty.func_name
        prefix
        (compact ty.kind_name)
    in
    let make_invalidation_clause ty =
      (* TODO: cleanup *)
      sprintf
        ("
    | %s" ^^ valign ^^ " thing -> invalidate_%-30s (value, thing)")
        prefix
        (compact ty.kind_name)
        ty.type_name
    in
    let invalidation_body =
      map_and_concat make_invalidation_clause aggregated_types
    in
    let validation_body =
      map_and_concat make_validation_clause aggregated_types
    in
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

  let full_fidelity_validated_syntax_template =
    make_header
      MLStyle
      "
 * This module contains the functions to (in)validate syntax trees."
    ^ "

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
end
"

  let full_fidelity_validated_syntax =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "VALIDATE_FUNCTIONS"; func = to_validate_functions }]
      ~aggregate_transformations:
        [
          {
            aggregate_pattern = "AGGREGATE_VALIDATORS";
            aggregate_func = to_aggregate_validation;
          };
        ]
      ~filename:(full_fidelity_path_prefix ^ "full_fidelity_validated_syntax.ml")
      ~template:full_fidelity_validated_syntax_template
      ()
end

module GenerateFFSyntaxType = struct
  let to_parse_tree x =
    if omit_syntax_record x then
      ""
    else
      let field_width = 50 - String.length x.prefix in
      let fmt = sprintf "%s_%%-%ds: t\n" x.prefix field_width in
      let mapper (f, _) = sprintf (Scanf.format_from_string fmt "%-1s") f in
      let fields = map_and_concat_separated "    ; " mapper x.fields in
      sprintf "  and %s =\n    { %s    }\n" x.type_name fields

  let to_syntax x =
    let field_width = 50 - String.length x.prefix in
    let fmt = sprintf "%s_%%-%ds: t\n" x.prefix field_width in
    let mapper (f, _) = sprintf (Scanf.format_from_string fmt "%-1s") f in
    let fields = map_and_concat_separated "    ; " mapper x.fields in
    sprintf
      ("  | " ^^ kind_name_fmt ^^ " of\n    { %s    }\n")
      x.kind_name
      fields

  let to_aggregate_type x =
    let aggregated_types = aggregation_of x in
    let (prefix, trim) = aggregate_type_pfx_trim x in
    let compact = Str.global_replace (Str.regexp trim) "" in
    let valign = align_fmt (fun x -> compact x.kind_name) aggregated_types in
    let type_name = aggregate_type_name x in
    let make_constructor ty =
      sprintf
        ("%s" ^^ valign ^^ " of %s")
        prefix
        (compact ty.kind_name)
        ty.type_name
    in
    let type_body = List.map ~f:make_constructor aggregated_types in
    sprintf
      "  and %s =\n  | %s\n"
      type_name
      (String.concat ~sep:"\n  | " type_body)

  let to_validated_syntax x =
    (* Not proud of this, but we have to exclude these things that don't occur
     * in validated syntax. Their absence being the point of the validated
     * syntax
     *)
    if
      String.equal x.kind_name "ErrorSyntax"
      || String.equal x.kind_name "ListItem"
    then
      ""
    else
      Printf.(
        let mapper (f, c) =
          let rec make_type_string : child_spec -> string = function
            | Aggregate t -> aggregate_type_name t
            | Token -> "Token.t"
            | Just t ->
              (match SMap.find_opt t schema_map with
              | None -> failwith @@ sprintf "Unknown type: %s" t
              | Some t -> t.type_name)
            | ZeroOrMore ((Just _ | Aggregate _ | Token) as c) ->
              sprintf "%s listesque" (make_type_string c)
            | ZeroOrOne ((Just _ | Aggregate _ | Token) as c) ->
              sprintf "%s option" (make_type_string c)
            | ZeroOrMore c -> sprintf "(%s) listesque" (make_type_string c)
            | ZeroOrOne c -> sprintf "(%s) option" (make_type_string c)
          in
          sprintf "%s_%s: %s value" x.prefix f (make_type_string c)
        in
        let fields = map_and_concat_separated "\n    ; " mapper x.fields in
        sprintf "  and %s =\n    { %s\n    }\n" x.type_name fields)

  let full_fidelity_syntax_template : string =
    make_header
      MLStyle
      "
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
 * associated with the node."
    ^ "

module type TokenType = sig
  module Trivia : Lexable_trivia_sig.LexableTrivia_S
  type t [@@deriving show, eq]
  val kind: t -> Full_fidelity_token_kind.t
  val to_json: t -> Hh_json.json
  val leading : t -> Trivia.t list
end

module type SyntaxValueType = sig
  type t [@@deriving show, eq]
  val to_json: t -> Hh_json.json
end

(* This functor describe the shape of a parse tree that has a particular kind of
 * token in the leaves, and a particular kind of value associated with each
 * node.
 *)
module MakeSyntaxType(Token : TokenType)(SyntaxValue : SyntaxValueType) = struct
  type value = SyntaxValue.t [@@deriving show, eq]
  type t = { syntax : syntax ; value : value } [@@deriving show, eq]
PARSE_TREE   and syntax =
  | Token                             of Token.t
  | Missing
  | SyntaxList                        of t list
SYNTAX
end

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
end
"

  let full_fidelity_syntax_type =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [
          { pattern = "PARSE_TREE"; func = to_parse_tree };
          { pattern = "SYNTAX"; func = to_syntax };
          { pattern = "VALIDATED_SYNTAX"; func = to_validated_syntax };
        ]
      ~aggregate_transformations:
        [
          {
            aggregate_pattern = "AGGREGATE_TYPES";
            aggregate_func = to_aggregate_type;
          };
        ]
      ~filename:(full_fidelity_path_prefix ^ "full_fidelity_syntax_type.ml")
      ~template:full_fidelity_syntax_template
      ()
end

module GenerateFFRustSyntaxImplByRef = struct
  let to_kind x =
    sprintf
      "            SyntaxVariant::%s {..} => SyntaxKind::%s,\n"
      x.kind_name
      x.kind_name

  let template =
    make_header CStyle ""
    ^ "
use crate::{syntax_kind::SyntaxKind, lexable_token::LexableToken};
use super::{syntax::Syntax, syntax_variant_generated::SyntaxVariant};

impl<T: LexableToken, V> Syntax<'_, T, V> {
    pub fn kind(&self) -> SyntaxKind {
        match &self.children {
            SyntaxVariant::Missing => SyntaxKind::Missing,
            SyntaxVariant::Token (t) => SyntaxKind::Token(t.kind()),
            SyntaxVariant::SyntaxList (_) => SyntaxKind::SyntaxList,
TO_KIND        }
    }
}
    "

  let full_fidelity_syntax =
    Full_fidelity_schema.make_template_file
      ~transformations:[{ pattern = "TO_KIND"; func = to_kind }]
      ~filename:
        (full_fidelity_path_prefix ^ "syntax_by_ref/syntax_impl_generated.rs")
      ~template
      ()
end

module GenerateSyntaxSerialize = struct
  let match_arm x =
    let get_field x = escape_rust_keyword (fst x) in
    let serialize_fields =
      map_and_concat_separated
        "\n"
        (fun y ->
          sprintf
            "ss.serialize_field(\"%s_%s\", &self.with(%s))?;"
            x.prefix
            (fst y)
            (get_field y))
        x.fields
    in
    let fields = map_and_concat_separated "," get_field x.fields in
    sprintf
      "SyntaxVariant::%s (%sChildren{%s} ) => {
      let mut ss = s.serialize_struct(\"\", %d)?;
      ss.serialize_field(\"kind\", \"%s\")?;
      %s
      ss.end()
} \n"
      x.kind_name
      x.kind_name
      fields
      (1 + List.length x.fields)
      x.description
      serialize_fields

  let template =
    make_header CStyle ""
    ^ "
use super::{serialize::WithContext, syntax::Syntax, syntax_variant_generated::*};
use serde::{ser::SerializeStruct, Serialize, Serializer};

impl<'a, T, V> Serialize for WithContext<'a, Syntax<'a, T, V>>
where
    T: 'a,
    WithContext<'a, T>: Serialize,
{
    fn serialize<S: Serializer>(&self, s: S) -> Result<S::Ok, S::Error> {
        match self.1.children {
            SyntaxVariant::Missing => {
                let mut ss = s.serialize_struct(\"\", 1)?;
                ss.serialize_field(\"kind\", \"missing\")?;
                ss.end()
            }
            SyntaxVariant::Token(ref t) => {
                let mut ss = s.serialize_struct(\"\", 2)?;
                ss.serialize_field(\"kind\", \"token\")?;
                ss.serialize_field(\"token\", &self.with(t))?;
                ss.end()
            }
            SyntaxVariant::SyntaxList(l) => {
                let mut ss = s.serialize_struct(\"\", 2)?;
                ss.serialize_field(\"kind\", \"list\")?;
                ss.serialize_field(\"elements\", &self.with(l))?;
                ss.end()
            }
            MATCH_ARMS
        }
    }
}
"

  let gen =
    Full_fidelity_schema.make_template_file
      ~transformations:[{ pattern = "MATCH_ARMS"; func = match_arm }]
      ~filename:
        ( full_fidelity_path_prefix
        ^ "syntax_by_ref/syntax_serialize_generated.rs" )
      ~template
      ()
end

module GenerateFFRustSyntaxVariantByRef = struct
  let to_syntax_variant_children x =
    let mapper (f, _) =
      sprintf "    pub %s: Syntax<'a, T, V>," (escape_rust_keyword f)
    in
    let fields = map_and_concat_separated "\n" mapper x.fields in
    sprintf
      "#[derive(Debug, Clone)]\npub struct %sChildren<'a, T, V> {\n%s\n}\n\n"
      x.kind_name
      fields

  let to_syntax_variant x =
    sprintf "    %s(&'a %sChildren<'a, T, V>),\n" x.kind_name x.kind_name

  let full_fidelity_syntax_template =
    make_header CStyle ""
    ^ "
use super::{
    syntax::Syntax,
    syntax_children_iterator::SyntaxChildrenIterator,
};

#[derive(Debug, Clone)]
pub enum SyntaxVariant<'a, T, V> {
    Token(T),
    Missing,
    SyntaxList(&'a [Syntax<'a, T, V>]),
SYNTAX_VARIANT}

SYNTAX_CHILDREN

impl<'a, T, V> SyntaxVariant<'a, T, V> {
    pub fn iter_children(&'a self) -> SyntaxChildrenIterator<'a, T, V> {
        SyntaxChildrenIterator {
            syntax: &self,
            index: 0,
            index_back: 0,
        }
    }
}
"

  let full_fidelity_syntax =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [
          { pattern = "SYNTAX_VARIANT"; func = to_syntax_variant };
          { pattern = "SYNTAX_CHILDREN"; func = to_syntax_variant_children };
        ]
      ~filename:
        (full_fidelity_path_prefix ^ "syntax_by_ref/syntax_variant_generated.rs")
      ~template:full_fidelity_syntax_template
      ()
end

module GenerateSyntaxChildrenIterator = struct
  let to_iter_children x =
    let index = ref 0 in
    let mapper (f, _) =
      let res = sprintf "%d => Some(&x.%s)," !index (escape_rust_keyword f) in
      let () = incr index in
      res
    in
    let fields =
      map_and_concat_separated "\n                    " mapper x.fields
    in
    sprintf
      "            %s(x) => {
                get_index(%d).and_then(|index| { match index {
                        %s
                        _ => None,
                    }
                })
            },\n"
      x.kind_name
      (List.length x.fields)
      fields

  let full_fidelity_syntax_template =
    make_header CStyle ""
    ^ "
use super::{
    syntax_children_iterator::*,
    syntax_variant_generated::*,
    syntax::*
};

impl<'a, T, V> SyntaxChildrenIterator<'a, T, V> {
    pub fn next_impl(&mut self, direction : bool) -> Option<&'a Syntax<'a, T, V>> {
        use SyntaxVariant::*;
        let get_index = |len| {
            let back_index_plus_1 = len - self.index_back;
            if back_index_plus_1 <= self.index {
                return None
            }
            if direction {
                Some (self.index)
            } else {
                Some (back_index_plus_1 - 1)
            }
        };
        let res = match self.syntax {
            Missing => None,
            Token (_) => None,
            SyntaxList(elems) => {
                get_index(elems.len()).and_then(|x| elems.get(x))
            },
ITER_CHILDREN
        };
        if res.is_some() {
            if direction {
                self.index = self.index + 1
            } else {
                self.index_back = self.index_back + 1
            }
        }
        res
    }
}
    "

  let full_fidelity_syntax =
    Full_fidelity_schema.make_template_file
      ~transformations:[{ pattern = "ITER_CHILDREN"; func = to_iter_children }]
      ~filename:
        ( full_fidelity_path_prefix
        ^ "syntax_by_ref/syntax_children_iterator_generated.rs" )
      ~template:full_fidelity_syntax_template
      ()
end

module GenerateSyntaxTypeImpl = struct
  let to_syntax_constructors x =
    let mapper (f, _) = sprintf "%s: Self" (escape_rust_keyword f) in
    let args = map_and_concat_separated ", " mapper x.fields in
    let mapper (f, _) = sprintf "%s" (escape_rust_keyword f) in
    let fields = map_and_concat_separated ",\n            " mapper x.fields in
    sprintf
      "    fn make_%s(ctx: &C, %s) -> Self {
        let syntax = SyntaxVariant::%s(ctx.get_arena().alloc(%sChildren {
            %s,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }\n\n"
      x.type_name
      args
      x.kind_name
      x.kind_name
      fields

  let full_fidelity_syntax_template =
    make_header CStyle ""
    ^ "
use super::{
    has_arena::HasArena,
    syntax::*, syntax_variant_generated::*,
};
use crate::{
    lexable_token::LexableToken,
    syntax::{SyntaxType, SyntaxValueType},
};

impl<'a, C, T, V> SyntaxType<C> for Syntax<'a, T, V>
where
    T: LexableToken + Copy,
    V: SyntaxValueType<T>,
    C: HasArena<'a>,
{
SYNTAX_CONSTRUCTORS }
"

  let full_fidelity_syntax =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "SYNTAX_CONSTRUCTORS"; func = to_syntax_constructors }]
      ~filename:
        ( full_fidelity_path_prefix
        ^ "syntax_by_ref/syntax_type_impl_generated.rs" )
      ~template:full_fidelity_syntax_template
      ()
end

module GenerateFFRustSyntax = struct
  let from_children x =
    let mapper prefix (f, _) =
      sprintf "%s_%s : (Box::new(ts.pop().unwrap()))" prefix f
    in
    let fields =
      map_and_concat_separated ",\n       " (mapper x.prefix) x.fields
    in
    sprintf
      "SyntaxKind::%s => SyntaxVariant::%s {
        %s
    },"
      x.kind_name
      x.kind_name
      fields

  let to_kind x =
    sprintf
      "            SyntaxVariant::%s {..} => SyntaxKind::%s,\n"
      x.kind_name
      x.kind_name

  let to_children x =
    let mapper prefix (f, _) = sprintf "&*%s_%s" prefix f in
    let fields2 =
      map_and_concat_separated ",\n       " (mapper x.prefix) x.fields
    in
    let mapper prefix (f, _) = sprintf "ref %s_%s" prefix f in
    let fields =
      map_and_concat_separated ",\n       " (mapper x.prefix) x.fields
    in
    sprintf
      "SyntaxVariant::%s {
        %s
    } => vec![%s],"
      x.kind_name
      fields
      fields2

  let to_iter_children x =
    let index = ref 0 in
    let mapper prefix (f, _) =
      let res = sprintf "%d => Some(&x.%s_%s)," !index prefix f in
      let () = incr index in
      res
    in
    let fields =
      map_and_concat_separated
        "\n                    "
        (mapper x.prefix)
        x.fields
    in
    sprintf
      "            %s(x) => {
                get_index(%d).and_then(|index| { match index {
                        %s
                        _ => None,
                    }
                })
            },\n"
      x.kind_name
      (List.length x.fields)
      fields

  let fold_over_owned x =
    let fold_mapper prefix (f, _) =
      sprintf "let acc = f(%s_%s, acc)" prefix f
    in
    let fold_fields =
      map_and_concat_separated
        ";\n                "
        (fold_mapper x.prefix)
        x.fields
    in
    let destructure_mapper prefix (f, _) = sprintf "%s_%s" prefix f in
    let destructure_fields =
      map_and_concat_separated ", " (destructure_mapper x.prefix) x.fields
    in
    sprintf
      "            SyntaxVariant::%s(x) => {
                let %sChildren { %s } = *x;
                %s;
                acc
            },\n"
      x.kind_name
      x.kind_name
      destructure_fields
      fold_fields

  let to_syntax_variant x =
    sprintf "    %s(Box<%sChildren<T, V>>),\n" x.kind_name x.kind_name

  let to_syntax_variant_children x =
    let mapper prefix (f, _) =
      sprintf "    pub %s_%s: Syntax<T, V>," prefix f
    in
    let fields = map_and_concat_separated "\n" (mapper x.prefix) x.fields in
    sprintf
      "#[derive(Debug, Clone)]\npub struct %sChildren<T, V> {\n%s\n}\n\n"
      x.kind_name
      fields

  let to_syntax_constructors x =
    let mapper prefix (f, _) = sprintf "%s_%s: Self" prefix f in
    let args = map_and_concat_separated ", " (mapper x.prefix) x.fields in
    let mapper prefix (f, _) = sprintf "%s_%s" prefix f in
    let fields =
      map_and_concat_separated ",\n            " (mapper x.prefix) x.fields
    in
    sprintf
      "    fn make_%s(_: &C, %s) -> Self {
        let syntax = SyntaxVariant::%s(Box::new(%sChildren {
            %s,
        }));
        let value = V::from_values(syntax.iter_children().map(|child| &child.value));
        Self::make(syntax, value)
    }\n\n"
      x.type_name
      args
      x.kind_name
      x.kind_name
      fields

  let to_syntax_from_children x =
    let mapper (f, _) =
      sprintf "%s_%s: ts.pop().unwrap(),\n                 " x.prefix f
    in
    let fields = map_and_concat mapper (List.rev x.fields) in
    sprintf
      "             (SyntaxKind::%s, %d) => SyntaxVariant::%s(Box::new(%sChildren {
                 %s
             })),\n"
      x.kind_name
      (List.length x.fields)
      x.kind_name
      x.kind_name
      fields

  let full_fidelity_syntax_template =
    make_header CStyle ""
    ^ "
use crate::lexable_token::LexableToken;
use crate::syntax::*;
use crate::syntax_kind::SyntaxKind;

impl<T, V, C> SyntaxType<C> for Syntax<T, V>
where
    T: LexableToken,
    V: SyntaxValueType<T>,
{
SYNTAX_CONSTRUCTORS }

impl<T, V> Syntax<T, V>
where
    T: LexableToken,
{
    pub fn fold_over_children_owned<U>(
        f: &dyn Fn(Self, U) -> U,
        acc: U,
        syntax: SyntaxVariant<T, V>,
    ) -> U {
        match syntax {
            SyntaxVariant::Missing => acc,
            SyntaxVariant::Token (_) => acc,
            SyntaxVariant::SyntaxList(elems) => {
                let mut acc = acc;
                for item in elems {
                    acc = f(item, acc);
                }
                acc
            },
FOLD_OVER_CHILDREN_OWNED
        }
    }

    pub fn kind(&self) -> SyntaxKind {
        match &self.syntax {
            SyntaxVariant::Missing => SyntaxKind::Missing,
            SyntaxVariant::Token (t) => SyntaxKind::Token(t.kind()),
            SyntaxVariant::SyntaxList (_) => SyntaxKind::SyntaxList,
TO_KIND        }
    }

    pub fn from_children(kind : SyntaxKind, mut ts : Vec<Self>) -> SyntaxVariant<T, V> {
         match (kind, ts.len()) {
             (SyntaxKind::Missing, 0) => SyntaxVariant::Missing,
             (SyntaxKind::SyntaxList, _) => SyntaxVariant::SyntaxList(ts),
SYNTAX_FROM_CHILDREN             _ => panic!(\"from_children called with wrong number of children\"),
         }
    }
}

SYNTAX_CHILDREN
#[derive(Debug, Clone)]
pub enum SyntaxVariant<T, V> {
    Token(Box<T>),
    Missing,
    SyntaxList(Vec<Syntax<T, V>>),
SYNTAX_VARIANT}

impl<'a, T, V> SyntaxChildrenIterator<'a, T, V> {
    pub fn next_impl(&mut self, direction : bool) -> Option<&'a Syntax<T, V>> {
        use SyntaxVariant::*;
        let get_index = |len| {
            let back_index_plus_1 = len - self.index_back;
            if back_index_plus_1 <= self.index {
                return None
            }
            if direction {
                Some (self.index)
            } else {
                Some (back_index_plus_1 - 1)
            }
        };
        let res = match &self.syntax {
            Missing => None,
            Token (_) => None,
            SyntaxList(elems) => {
                get_index(elems.len()).and_then(|x| elems.get(x))
            },
ITER_CHILDREN
        };
        if res.is_some() {
            if direction {
                self.index = self.index + 1
            } else {
                self.index_back = self.index_back + 1
            }
        }
        res
    }
}
"

  let full_fidelity_syntax =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [
          { pattern = "SYNTAX_VARIANT"; func = to_syntax_variant };
          { pattern = "SYNTAX_CHILDREN"; func = to_syntax_variant_children };
          { pattern = "SYNTAX_CONSTRUCTORS"; func = to_syntax_constructors };
          { pattern = "ITER_CHILDREN"; func = to_iter_children };
          { pattern = "FOLD_OVER_CHILDREN_OWNED"; func = fold_over_owned };
          { pattern = "TO_KIND"; func = to_kind };
          { pattern = "SYNTAX_FROM_CHILDREN"; func = to_syntax_from_children };
        ]
      ~filename:(full_fidelity_path_prefix ^ "syntax_generated.rs")
      ~template:full_fidelity_syntax_template
      ()
end

(* GenerateFFRustSyntax *)

module GenerateFFRustSyntaxType = struct
  let to_kind x =
    sprintf
      "            SyntaxVariant::%s {..} => SyntaxKind::%s,\n"
      x.kind_name
      x.kind_name

  let to_children x =
    let mapper prefix (f, _) = sprintf "&*%s_%s" prefix f in
    let fields2 =
      map_and_concat_separated ",\n       " (mapper x.prefix) x.fields
    in
    let mapper prefix (f, _) = sprintf "ref %s_%s" prefix f in
    let fields =
      map_and_concat_separated ",\n       " (mapper x.prefix) x.fields
    in
    sprintf
      "SyntaxVariant::%s {
        %s
    } => vec![%s],"
      x.kind_name
      fields
      fields2

  let into_children x =
    let mapper prefix (f, _) = sprintf "x.%s_%s" prefix f in
    let fields =
      map_and_concat_separated ",\n                " (mapper x.prefix) x.fields
    in
    sprintf
      "            SyntaxVariant::%s (x) => { vec!(
                %s
            )},\n"
      x.kind_name
      fields

  let fold_over x =
    let mapper prefix (f, _) = sprintf "let acc = f(&x.%s_%s, acc)" prefix f in
    let fields =
      map_and_concat_separated ";\n                " (mapper x.prefix) x.fields
    in
    sprintf
      "            SyntaxVariant::%s(x) => {
                %s;
                acc
            },\n"
      x.kind_name
      fields

  let to_syntax_constructors x =
    let mapper prefix (f, _) = sprintf "%s_%s: Self" prefix f in
    let args = map_and_concat_separated ", " (mapper x.prefix) x.fields in
    sprintf "    fn make_%s(ctx: &C, %s) -> Self;\n" x.type_name args

  let full_fidelity_syntax_template =
    make_header CStyle ""
    ^ "
use crate::syntax::*;

pub trait SyntaxType<C>: SyntaxTypeBase<C>
{
SYNTAX_CONSTRUCTORS
}
"

  let full_fidelity_syntax =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "SYNTAX_CONSTRUCTORS"; func = to_syntax_constructors }]
      ~filename:(full_fidelity_path_prefix ^ "syntax_type.rs")
      ~template:full_fidelity_syntax_template
      ()
end

(* GenerateFFRustSyntaxType *)

module GenerateFFSyntaxSig = struct
  let to_constructor_methods x =
    let mapper1 (_f, _) = " t ->" in
    let fields1 = map_and_concat mapper1 x.fields in
    sprintf "  val make_%s :%s t\n" x.type_name fields1

  let to_type_tests x = sprintf "  val is_%s : t -> bool\n" x.type_name

  let to_syntax x =
    let field_width = 50 - String.length x.prefix in
    let fmt = sprintf "%s_%%-%ds: t\n" x.prefix field_width in
    let mapper (f, _) = sprintf (Scanf.format_from_string fmt "%-1s") f in
    let fields = map_and_concat_separated "    ; " mapper x.fields in
    sprintf
      ("  | " ^^ kind_name_fmt ^^ " of\n    { %s    }\n")
      x.kind_name
      fields

  let full_fidelity_syntax_template : string =
    make_header
      MLStyle
      "
* This module contains a signature which can be used to describe the public
* surface area of a constructable syntax tree.
"
    ^ "

module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind

module type Syntax_S = sig
  module Token : Lexable_token_sig.LexableToken_S
  type value [@@deriving show, eq]
  type t = { syntax : syntax ; value : value } [@@deriving show, eq]
  and syntax =
  | Token                             of Token.t
  | Missing
  | SyntaxList                        of t list
SYNTAX

  val rust_parse :
    Full_fidelity_source_text.t ->
    Full_fidelity_parser_env.t ->
    unit * t * Full_fidelity_syntax_error.t list * Rust_pointer.t option
  val rust_parse_with_verify_sc :
    Full_fidelity_source_text.t ->
    Full_fidelity_parser_env.t ->
    t list * t * Full_fidelity_syntax_error.t list * Rust_pointer.t option
  val rust_parser_errors :
    Full_fidelity_source_text.t ->
    Rust_pointer.t ->
    ParserOptions.ffi_t ->
    Full_fidelity_syntax_error.t list
  val has_leading_trivia : TriviaKind.t -> Token.t -> bool
  val to_json : ?with_value:bool -> ?ignore_missing:bool -> t -> Hh_json.json
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
  val syntax_list_fold : init:'a -> f:('a -> t -> 'a) -> t -> 'a
CONSTRUCTOR_METHODS

  val position : Relative_path.t -> t -> Pos.t option
  val offset : t -> int option
  val is_missing : t -> bool
  val is_list : t -> bool
TYPE_TESTS

  val is_specific_token : TokenKind.t -> t -> bool
  val is_loop_statement : t -> bool
  val is_external       : t -> bool
  val is_name           : t -> bool
  val is_construct      : t -> bool
  val is_static         : t -> bool
  val is_private        : t -> bool
  val is_public         : t -> bool
  val is_protected      : t -> bool
  val is_abstract       : t -> bool
  val is_final          : t -> bool
  val is_async          : t -> bool
  val is_void           : t -> bool
  val is_left_brace     : t -> bool
  val is_ellipsis       : t -> bool
  val is_comma          : t -> bool
  val is_ampersand      : t -> bool
  val is_inout          : t -> bool


end
"

  let full_fidelity_syntax_sig =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [
          { pattern = "SYNTAX"; func = to_syntax };
          { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods };
          { pattern = "TYPE_TESTS"; func = to_type_tests };
        ]
      ~filename:(full_fidelity_path_prefix ^ "syntax_sig.ml")
      ~template:full_fidelity_syntax_template
      ()
end

(* GenerateFFSyntaxSig *)

module GenerateFFSmartConstructors = struct
  let full_fidelity_smart_constructors_template : string =
    make_header
      MLStyle
      "
 * This module contains a signature which can be used to describe smart
 * constructors.
  "
    ^ "

module ParserEnv = Full_fidelity_parser_env

module type SmartConstructors_S = sig
  module Token : Lexable_token_sig.LexableToken_S

  type t (* state *) [@@deriving show]

  type r (* smart constructor return type *) [@@deriving show]

  val rust_parse :
    Full_fidelity_source_text.t ->
    ParserEnv.t ->
    t * r * Full_fidelity_syntax_error.t list * Rust_pointer.t option

  val initial_state : ParserEnv.t -> t
end
"

  let full_fidelity_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:[]
      ~filename:
        (full_fidelity_path_prefix ^ "smart_constructors/smartConstructors.ml")
      ~template:full_fidelity_smart_constructors_template
      ()
end

(* GenerateFFSmartConstructors *)

module GenerateFFRustSmartConstructors = struct
  let to_make_methods x =
    let fields =
      List.mapi x.fields ~f:(fun i _ -> "arg" ^ string_of_int i ^ " : Self::R")
    in
    let stack = String.concat ~sep:", " fields in
    sprintf "    fn make_%s(&mut self, %s) -> Self::R;\n" x.type_name stack

  let full_fidelity_smart_constructors_template : string =
    make_header CStyle ""
    ^ "
use parser_core_types::token_factory::TokenFactory;
use parser_core_types::lexable_token::LexableToken;

pub type Token<S> = <<S as SmartConstructors>::TF as TokenFactory>::Token;
pub type Trivia<S> = <Token<S> as LexableToken>::Trivia;

pub trait SmartConstructors: Clone {
    type TF: TokenFactory;
    type State;
    type R;

    fn state_mut(&mut self) -> &mut Self::State;
    fn into_state(self) -> Self::State;
    fn token_factory_mut(&mut self) -> &mut Self::TF;

    fn make_missing(&mut self, offset : usize) -> Self::R;
    fn make_token(&mut self, arg0: Token<Self>) -> Self::R;
    fn make_list(&mut self, arg0: Vec<Self::R>, offset: usize) -> Self::R;

    fn begin_enumerator(&mut self) {}
    fn begin_enum_class_enumerator(&mut self) {}
    fn begin_constant_declarator(&mut self) {}

MAKE_METHODS
}
"

  let full_fidelity_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:[{ pattern = "MAKE_METHODS"; func = to_make_methods }]
      ~filename:(full_fidelity_path_prefix ^ "smart_constructors_generated.rs")
      ~template:full_fidelity_smart_constructors_template
      ()
end

(* GenerateFFRustSmartConstructors *)

module GenerateFFRustPositionedSmartConstructors = struct
  let to_constructor_methods x =
    let args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d: Self::R" i) in
    let args = String.concat ~sep:", " args in
    let fwd_args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i) in
    let fwd_args = String.concat ~sep:", " fwd_args in
    sprintf
      "    fn make_%s(&mut self, %s) -> Self::R {
        <Self as SyntaxSmartConstructors<S, TF, State>>::make_%s(self, %s)
    }\n\n"
      x.type_name
      args
      x.type_name
      fwd_args

  let positioned_smart_constructors_template : string =
    make_header CStyle ""
    ^ "


use parser_core_types::{
    syntax::*,
    lexable_token::LexableToken,
    token_factory::TokenFactory,
};
use smart_constructors::SmartConstructors;
use syntax_smart_constructors::{SyntaxSmartConstructors, StateType};

#[derive(Clone)]
pub struct PositionedSmartConstructors<S, TF, State: StateType<S>> {
    pub state: State,
    token_factory: TF,
    phantom_s: std::marker::PhantomData<S>,
}

impl<S, TF, State: StateType<S>> PositionedSmartConstructors<S, TF, State> {
    pub fn new(state: State, token_factory: TF) -> Self {
        Self { state, token_factory, phantom_s: std::marker::PhantomData }
    }
}

impl<S, TF, State> SyntaxSmartConstructors<S, TF, State> for PositionedSmartConstructors<S, TF, State>
where
    TF: TokenFactory<Token = S::Token>,
    State: StateType<S>,
    S: SyntaxType<State> + Clone,
    S::Token: LexableToken,
{}

impl<S, TF, State> SmartConstructors for PositionedSmartConstructors<S, TF, State>
where
    TF: TokenFactory<Token = S::Token>,
    S::Token: LexableToken,
    S: SyntaxType<State> + Clone,
    State: StateType<S>,
{
    type TF = TF;
    type State = State;
    type R = S;

    fn state_mut(&mut self) -> &mut State {
       &mut self.state
    }

    fn into_state(self) -> State {
      self.state
    }

    fn token_factory_mut(&mut self) -> &mut Self::TF {
        &mut self.token_factory
    }

    fn make_missing(&mut self, offset: usize) -> Self::R {
        <Self as SyntaxSmartConstructors<S, TF, State>>::make_missing(self, offset)
    }

    fn make_token(&mut self, offset: <Self::TF as TokenFactory>::Token) -> Self::R {
        <Self as SyntaxSmartConstructors<S, TF, State>>::make_token(self, offset)
    }

    fn make_list(&mut self, lst: Vec<Self::R>, offset: usize) -> Self::R {
        <Self as SyntaxSmartConstructors<S, TF, State>>::make_list(self, lst, offset)
    }
CONSTRUCTOR_METHODS}
"

  let positioned_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }]
      ~filename:(full_fidelity_path_prefix ^ "positioned_smart_constructors.rs")
      ~template:positioned_smart_constructors_template
      ()
end

(* GenerateFFRustPositionedSmartConstructors *)

module GenerateFFRustVerifySmartConstructors = struct
  let to_constructor_methods x =
    let args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d: Self::R" i) in
    let args = String.concat ~sep:", " args in
    let fwd_args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i) in
    let fwd_args = String.concat ~sep:", " fwd_args in
    sprintf
      "    fn make_%s(&mut self, %s) -> Self::R {
        let args = arg_kinds!(%s);
        let r = <Self as SyntaxSmartConstructors<PositionedSyntax<'a>, TokenFactory<'a>, State<'a>>>::make_%s(self, %s);
        self.state_mut().verify(&args);
        self.state_mut().push(r.kind());
        r
    }\n\n"
      x.type_name
      args
      fwd_args
      x.type_name
      fwd_args

  let verify_smart_constructors_template : string =
    make_header CStyle ""
    ^ "
use crate::*;
use parser_core_types::syntax_by_ref::{
    positioned_syntax::PositionedSyntax,
    positioned_token::TokenFactory,
};
use smart_constructors::SmartConstructors;
use syntax_smart_constructors::SyntaxSmartConstructors;

macro_rules! arg_kinds {
    ($a0:ident) => (
        vec![$a0.kind()]
    );
    ($a0:ident, $($a1:ident),+) => (
        vec![$a0.kind(), $($a1.kind()),+]
    );
}

impl<'a> SmartConstructors for VerifySmartConstructors<'a>
{
    type State = State<'a>;
    type TF = TokenFactory<'a>;
    type R = PositionedSyntax<'a>;

    fn state_mut(&mut self) -> &mut State<'a> {
       &mut self.state
    }

    fn into_state(self) -> State<'a> {
      self.state
    }

    fn token_factory_mut(&mut self) -> &mut Self::TF {
        &mut self.token_factory
    }

    fn make_missing(&mut self, offset: usize) -> Self::R {
        let r = <Self as SyntaxSmartConstructors<PositionedSyntax<'a>, TokenFactory<'a>, State<'a>>>::make_missing(self, offset);
        self.state_mut().push(r.kind());
        r
    }

    fn make_token(&mut self, offset: PositionedToken<'a>) -> Self::R {
        let r = <Self as SyntaxSmartConstructors<PositionedSyntax<'a>, TokenFactory<'a>, State<'a>>>::make_token(self, offset);
        self.state_mut().push(r.kind());
        r
    }

    fn make_list(&mut self, lst: Vec<Self::R>, offset: usize) -> Self::R {
        if !lst.is_empty() {
            let args: Vec<_> = (&lst).iter().map(|s| s.kind()).collect();
            let r = <Self as SyntaxSmartConstructors<PositionedSyntax<'a>, TokenFactory<'a>, State<'a>>>::make_list(self, lst, offset);
            self.state_mut().verify(&args);
            self.state_mut().push(r.kind());
            r
        } else {
            <Self as SmartConstructors>::make_missing(self, offset)
        }
    }

CONSTRUCTOR_METHODS}
"

  let verify_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }]
      ~filename:
        (full_fidelity_path_prefix ^ "verify_smart_constructors_generated.rs")
      ~template:verify_smart_constructors_template
      ()
end

(* GenerateFFVerifySmartConstructors *)

module GenerateFFSyntaxSmartConstructors = struct
  let full_fidelity_syntax_smart_constructors_template : string =
    make_header
      MLStyle
      "
 * This module contains smart constructors implementation that can be used to
 * build AST.
 "
    ^ "

module type SC_S = SmartConstructors.SmartConstructors_S

module ParserEnv = Full_fidelity_parser_env

module type State_S = sig
  type r [@@deriving show]

  type t [@@deriving show]

  val initial : ParserEnv.t -> t

  val next : t -> r list -> t
end

module type RustParser_S = sig
  type t

  type r

  val rust_parse :
    Full_fidelity_source_text.t ->
    ParserEnv.t ->
    t * r * Full_fidelity_syntax_error.t list * Rust_pointer.t option
end

module WithSyntax (Syntax : Syntax_sig.Syntax_S) = struct
  module WithState (State : State_S with type r = Syntax.t) = struct
    module WithRustParser
        (RustParser : RustParser_S with type t = State.t with type r = Syntax.t) =
    struct
      module Token = Syntax.Token

      type t = State.t [@@deriving show]

      type r = Syntax.t [@@deriving show]

      let rust_parse = RustParser.rust_parse

      let initial_state = State.initial
    end
  end

  include WithState (struct
    type r = Syntax.t [@@deriving show]

    type t = unit [@@deriving show]

    let initial _ = ()

    let next () _ = ()
  end)

  include WithRustParser (struct
    type r = Syntax.t

    type t = unit

    let rust_parse = Syntax.rust_parse
  end)
end
"

  let full_fidelity_syntax_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:[]
      ~filename:
        ( full_fidelity_path_prefix
        ^ "smart_constructors/syntaxSmartConstructors.ml" )
      ~template:full_fidelity_syntax_smart_constructors_template
      ()
end

(* GenerateFFSyntaxSmartConstructors *)

module GenerateFFRustSyntaxSmartConstructors = struct
  let to_constructor_methods x =
    let params =
      List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d : Self::R" i)
    in
    let params = String.concat ~sep:", " params in
    let args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i) in
    let args = String.concat ~sep:", " args in
    let next_args = List.mapi x.fields ~f:(fun i _ -> sprintf "&arg%d" i) in
    let next_args = String.concat ~sep:", " next_args in
    sprintf
      "    fn make_%s(&mut self, %s) -> Self::R {
        self.state_mut().next(&[%s]);
        Self::R::make_%s(self.state_mut(), %s)
    }\n\n"
      x.type_name
      params
      next_args
      x.type_name
      args

  let full_fidelity_syntax_smart_constructors_template : string =
    make_header CStyle ""
    ^ "
use parser_core_types::{
    syntax::*,
    token_factory::TokenFactory,
};
use smart_constructors::{NoState, SmartConstructors};
use crate::StateType;

pub trait SyntaxSmartConstructors<S: SyntaxType<State>, TF: TokenFactory<Token = S::Token>, State = NoState>:
    SmartConstructors<State = State, R=S, TF = TF>
where
    State: StateType<S>,
{
    fn make_missing(&mut self, offset: usize) -> Self::R {
        let r = Self::R::make_missing(self.state_mut(), offset);
        self.state_mut().next(&[]);
        r
    }

    fn make_token(&mut self, arg: <Self::TF as TokenFactory>::Token) -> Self::R {
        let r = Self::R::make_token(self.state_mut(), arg);
        self.state_mut().next(&[]);
        r
    }

    fn make_list(&mut self, items: Vec<Self::R>, offset: usize) -> Self::R {
        if items.is_empty() {
            <Self as SyntaxSmartConstructors<S, TF, State>>::make_missing(self, offset)
        } else {
            let item_refs: Vec<_> = items.iter().collect();
            self.state_mut().next(&item_refs);
            Self::R::make_list(self.state_mut(), items, offset)
        }
    }

CONSTRUCTOR_METHODS}
"

  let full_fidelity_syntax_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }]
      ~filename:
        (full_fidelity_path_prefix ^ "syntax_smart_constructors_generated.rs")
      ~template:full_fidelity_syntax_smart_constructors_template
      ()
end

(* GenerateFFRustSyntaxSmartConstructors *)

module GenerateFFRustDeclModeSmartConstructors = struct
  let to_constructor_methods x =
    let args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d: Self::R" i) in
    let args = String.concat ~sep:", " args in
    let fwd_args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i) in
    let fwd_args = String.concat ~sep:", " fwd_args in
    sprintf
      "    fn make_%s(&mut self, %s) -> Self::R {
        <Self as SyntaxSmartConstructors<Self::R, Self::TF, State<Self::R>>>::make_%s(self, %s)
    }\n\n"
      x.type_name
      args
      x.type_name
      fwd_args

  let decl_mode_smart_constructors_template : string =
    make_header CStyle ""
    ^ "
use parser_core_types::{
    lexable_token::LexableToken, syntax::SyntaxValueType, syntax_by_ref::syntax::Syntax,
    token_factory::TokenFactory,
};
use smart_constructors::SmartConstructors;
use syntax_smart_constructors::SyntaxSmartConstructors;
use crate::*;

impl<'src, 'arena, Token, Value, TF> SmartConstructors
    for DeclModeSmartConstructors<'src, 'arena, Syntax<'arena, Token, Value>, Token, Value, TF>
where
    TF: TokenFactory<Token = SyntaxToken<'src, 'arena, Token, Value>>,
    Token: LexableToken + Copy,
    Value: SyntaxValueType<Token> + Clone,
{
    type State = State<'src, 'arena, Syntax<'arena, Token, Value>>;
    type TF = TF;
    type R = Syntax<'arena, Token, Value>;

    fn state_mut(&mut self) -> &mut State<'src, 'arena, Syntax<'arena, Token, Value>> {
        &mut self.state
    }

    fn into_state(self) -> State<'src, 'arena, Syntax<'arena, Token, Value>> {
        self.state
    }

    fn token_factory_mut(&mut self) -> &mut Self::TF {
        &mut self.token_factory
    }

    fn make_missing(&mut self, o: usize) -> Self::R {
        <Self as SyntaxSmartConstructors<Self::R, Self::TF, State<Self::R>>>::make_missing(self, o)
    }

    fn make_token(&mut self, token: <Self::TF as TokenFactory>::Token) -> Self::R {
        <Self as SyntaxSmartConstructors<Self::R, Self::TF, State<Self::R>>>::make_token(self, token)
    }

    fn make_list(&mut self, items: Vec<Self::R>, offset: usize) -> Self::R {
        <Self as SyntaxSmartConstructors<Self::R, Self::TF, State<Self::R>>>::make_list(self, items, offset)
    }

CONSTRUCTOR_METHODS}
"

  let decl_mode_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }]
      ~filename:
        (full_fidelity_path_prefix ^ "decl_mode_smart_constructors_generated.rs")
      ~template:decl_mode_smart_constructors_template
      ()
end

(* GenerateFFRustDeclModeSmartConstructors *)

module GenerateRustFlattenSmartConstructors = struct
  let to_constructor_methods x =
    let args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d: Self::R" i) in
    let args = String.concat ~sep:", " args in
    let if_cond =
      List.mapi x.fields ~f:(fun i _ -> sprintf "Self::is_zero(&arg%d)" i)
    in
    let if_cond = String.concat ~sep:" && " if_cond in
    let flatten_args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i) in
    let flatten_args = String.concat ~sep:", " flatten_args in
    sprintf
      "    fn make_%s(&mut self, %s) -> Self::R {
        if %s {
          Self::zero(SyntaxKind::%s)
        } else {
          self.flatten(SyntaxKind::%s, vec!(%s))
        }
    }\n\n"
      x.type_name
      args
      if_cond
      x.kind_name
      x.kind_name
      flatten_args

  let flatten_smart_constructors_template : string =
    make_header CStyle ""
    ^ "
use smart_constructors::SmartConstructors;
use parser_core_types::{
  lexable_token::LexableToken,
  syntax_kind::SyntaxKind,
  token_factory::TokenFactory,
};

pub trait FlattenOp {
    type S;
    fn is_zero(s: &Self::S) -> bool;
    fn zero(kind: SyntaxKind) -> Self::S;
    fn flatten(&self, kind: SyntaxKind, lst: Vec<Self::S>) -> Self::S;
}

pub trait FlattenSmartConstructors<'src, State>
: SmartConstructors<State = State> + FlattenOp<S=<Self as SmartConstructors>::R>
{
    fn make_missing(&mut self, _: usize) -> Self::R {
       Self::zero(SyntaxKind::Missing)
    }

    fn make_token(&mut self, token: <Self::TF as TokenFactory>::Token) -> Self::R {
        Self::zero(SyntaxKind::Token(token.kind()))
    }

    fn make_list(&mut self, _: Vec<Self::R>, _: usize) -> Self::R {
        Self::zero(SyntaxKind::SyntaxList)
    }

    fn begin_enumerator(&mut self) {}

    fn begin_enum_class_enumerator(&mut self) {}

    fn begin_constant_declarator(&mut self) {}

CONSTRUCTOR_METHODS}
"

  let flatten_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }]
      ~filename:(full_fidelity_path_prefix ^ "flatten_smart_constructors.rs")
      ~template:flatten_smart_constructors_template
      ()
end

(* GenerateRustFlattenSmartConstructors *)

module GenerateRustFactsSmartConstructors = struct
  let to_constructor_methods x =
    let args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d: Self::R" i) in
    let args = String.concat ~sep:", " args in
    let fwd_args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i) in
    let fwd_args = String.concat ~sep:", " fwd_args in
    sprintf
      "    fn make_%s(&mut self, %s) -> Self::R {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_%s(self, %s)
    }\n\n"
      x.type_name
      args
      x.type_name
      fwd_args

  let facts_smart_constructors_template : string =
    make_header CStyle ""
    ^ "
use flatten_smart_constructors::*;
use smart_constructors::SmartConstructors;
use parser_core_types::positioned_token::PositionedToken;
use parser_core_types::token_factory::SimpleTokenFactoryImpl;

use crate::*;

#[derive(Clone)]
pub struct FactsSmartConstructors<'src> {
    pub state: HasScriptContent<'src>,
    pub token_factory: SimpleTokenFactoryImpl<PositionedToken>,
}
impl<'src> SmartConstructors for FactsSmartConstructors<'src> {
    type State = HasScriptContent<'src>;
    type TF = SimpleTokenFactoryImpl<PositionedToken>;
    type R = Node;

    fn state_mut(&mut self) -> &mut HasScriptContent<'src> {
        &mut self.state
    }

    fn into_state(self) -> HasScriptContent<'src> {
      self.state
    }

    fn token_factory_mut(&mut self) -> &mut Self::TF {
        &mut self.token_factory
    }

    fn make_missing(&mut self, offset: usize) -> Self::R {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_missing(self, offset)
    }

    fn make_token(&mut self, token: PositionedToken) -> Self::R {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_token(self, token)
    }

    fn make_list(&mut self, items: Vec<Self::R>, offset: usize) -> Self::R {
        <Self as FlattenSmartConstructors<'src, HasScriptContent<'src>>>::make_list(self, items, offset)
    }

CONSTRUCTOR_METHODS}
"

  let facts_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }]
      ~filename:
        ( full_fidelity_path_prefix
        ^ "../facts/facts_smart_constructors_generated.rs" )
      ~template:facts_smart_constructors_template
      ()
end

(* GenerateRustFactsSmartConstructors *)

module GenerateRustDirectDeclSmartConstructors = struct
  let to_constructor_methods x =
    let args =
      List.map x.fields ~f:(fun (name, _) ->
          sprintf "%s: Self::R" (escape_rust_keyword name))
    in
    let args = String.concat ~sep:", " args in
    let fwd_args =
      List.map x.fields ~f:(fun (name, _) -> escape_rust_keyword name)
    in
    let fwd_args = String.concat ~sep:", " fwd_args in
    sprintf
      "    fn make_%s(&mut self, %s) -> Self::R {
        <Self as FlattenSmartConstructors<'src, Self>>::make_%s(self, %s)
    }\n\n"
      x.type_name
      args
      x.type_name
      fwd_args

  let direct_decl_smart_constructors_template : string =
    make_header CStyle ""
    ^ "
use flatten_smart_constructors::*;
use parser_core_types::compact_token::CompactToken;
use parser_core_types::token_factory::SimpleTokenFactoryImpl;
use smart_constructors::SmartConstructors;

use crate::{DirectDeclSmartConstructors, Node, SourceTextAllocator};

impl<'src, 'text, S: SourceTextAllocator<'text, 'src>> SmartConstructors for DirectDeclSmartConstructors<'src, 'text, S> {
    type State = Self;
    type TF = SimpleTokenFactoryImpl<CompactToken>;
    type R = Node<'src>;

    fn state_mut(&mut self) -> &mut Self {
        self
    }

    fn into_state(self) -> Self {
        self
    }

    fn token_factory_mut(&mut self) -> &mut Self::TF {
        &mut self.token_factory
    }

    fn make_missing(&mut self, offset: usize) -> Self::R {
        <Self as FlattenSmartConstructors<'src, Self>>::make_missing(self, offset)
    }

    fn make_token(&mut self, token: CompactToken) -> Self::R {
        <Self as FlattenSmartConstructors<'src, Self>>::make_token(self, token)
    }

    fn make_list(&mut self, items: Vec<Self::R>, offset: usize) -> Self::R {
        <Self as FlattenSmartConstructors<'src, Self>>::make_list(self, items, offset)
    }

    fn begin_enumerator(&mut self) {
        <Self as FlattenSmartConstructors<'src, Self>>::begin_enumerator(self)
    }

    fn begin_enum_class_enumerator(&mut self) {
        <Self as FlattenSmartConstructors<'src, Self>>::begin_enum_class_enumerator(self)
    }

    fn begin_constant_declarator(&mut self) {
        <Self as FlattenSmartConstructors<'src, Self>>::begin_constant_declarator(self)
    }



CONSTRUCTOR_METHODS}
"

  let direct_decl_smart_constructors =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }]
      ~filename:
        ( full_fidelity_path_prefix
        ^ "../decl/direct_decl_smart_constructors_generated.rs" )
      ~template:direct_decl_smart_constructors_template
      ()
end

(* GenerateRustDirectDeclSmartConstructors *)

module GenerateFFSmartConstructorsWrappers = struct
  let full_fidelity_smart_constructors_wrappers_template : string =
    make_header
      MLStyle
      "
 * This module contains smart constructors implementation that can be used to
 * build AST.
 "
    ^ "

module type SC_S = SmartConstructors.SmartConstructors_S

module SK = Full_fidelity_syntax_kind

module type SyntaxKind_S = sig
  include SC_S

  type original_sc_r [@@deriving show]
end

module SyntaxKind (SC : SC_S) :
  SyntaxKind_S
    with module Token = SC.Token
     and type original_sc_r = SC.r
     and type t = SC.t = struct
  module Token = SC.Token

  type original_sc_r = SC.r [@@deriving show]

  type t = SC.t [@@deriving show]

  type r = SK.t * SC.r [@@deriving show]

  let compose : SK.t -> t * SC.r -> t * r =
   (fun kind (state, res) -> (state, (kind, res)))

  let rust_parse text env =
    let (state, res, errors, pointer) = SC.rust_parse text env in
    let (state, res) = compose SK.Script (state, res) in
    (state, res, errors, pointer)

  let initial_state = SC.initial_state
end
"

  let full_fidelity_smart_constructors_wrappers =
    Full_fidelity_schema.make_template_file
      ~transformations:[]
      ~filename:
        ( full_fidelity_path_prefix
        ^ "smart_constructors/smartConstructorsWrappers.ml" )
      ~template:full_fidelity_smart_constructors_wrappers_template
      ()
end

(* GenerateFFSmartConstructorsWrappers *)

module GenerateFFRustSmartConstructorsWrappers = struct
  let to_constructor_methods x =
    let params =
      List.mapi x.fields ~f:(fun i _ -> "arg" ^ string_of_int i ^ " : Self::R")
    in
    let params = String.concat ~sep:", " params in
    let args = List.mapi x.fields ~f:(fun i _ -> sprintf "arg%d" i) in
    let raw_args = map_and_concat_separated ", " (fun x -> x ^ ".1") args in
    sprintf
      "    fn make_%s(&mut self, %s) -> Self::R {
        compose(SyntaxKind::%s, self.s.make_%s(%s))
    }\n"
      x.type_name
      params
      x.kind_name
      x.type_name
      raw_args

  let full_fidelity_smart_constructors_wrappers_template : string =
    make_header CStyle ""
    ^ "
 // This module contains smart constructors implementation that can be used to
 // build AST.
"
    ^ "
use parser_core_types::{
  lexable_token::LexableToken,
  syntax_kind::SyntaxKind,
  token_factory::TokenFactory,
};
use crate::SmartConstructors;

#[derive(Clone)]
pub struct WithKind<S> {
    s: S,
}

impl<S> WithKind<S> {
    pub fn new(s: S) -> Self {
        Self { s }
    }
}

impl<S, State> SmartConstructors for WithKind<S>
where S: SmartConstructors<State = State>,
{
    type TF = S::TF;
    type State = State;
    type R = (SyntaxKind, S::R);

    fn state_mut(&mut self) -> &mut State {
        self.s.state_mut()
    }

    fn into_state(self) -> State {
      self.s.into_state()
    }

    fn token_factory_mut(&mut self) -> &mut Self::TF {
        self.s.token_factory_mut()
    }


    fn make_token(&mut self, token: <Self::TF as TokenFactory>::Token) -> Self::R {
        compose(SyntaxKind::Token(token.kind()), self.s.make_token(token))
    }

    fn make_missing(&mut self, p: usize) -> Self::R {
        compose(SyntaxKind::Missing, self.s.make_missing(p))
    }

    fn make_list(&mut self, items: Vec<Self::R>, p: usize) -> Self::R {
        let kind = if items.is_empty() {
            SyntaxKind::Missing
        } else {
            SyntaxKind::SyntaxList
        };
        compose(kind, self.s.make_list(items.into_iter().map(|x| x.1).collect(), p))
    }

CONSTRUCTOR_METHODS
}

#[inline(always)]
fn compose<R>(kind: SyntaxKind, r: R) -> (SyntaxKind, R) {
    (kind, r)
}
"

  let full_fidelity_smart_constructors_wrappers =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [{ pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods }]
      ~filename:(full_fidelity_path_prefix ^ "smart_constructors_wrappers.rs")
      ~template:full_fidelity_smart_constructors_wrappers_template
      ()
end

(* GenerateFFRustSmartConstructorsWrappers *)

module GenerateFFSyntax = struct
  let to_to_kind x =
    sprintf
      ("      | " ^^ kind_name_fmt ^^ " _ -> SyntaxKind.%s\n")
      x.kind_name
      x.kind_name

  let to_type_tests x =
    sprintf
      ("    let is_" ^^ type_name_fmt ^^ " = has_kind SyntaxKind.%s\n")
      x.type_name
      x.kind_name

  let to_children x =
    let mapper (f, _) = sprintf "        %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    sprintf
      "      | %s {\n%s      } -> [\n%s      ]\n"
      x.kind_name
      fields
      fields

  let to_fold_from_syntax x =
    let mapper (f, _) = sprintf "        %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    let mapper2 (f, _) =
      sprintf "         let acc = f acc %s_%s in\n" x.prefix f
    in
    let fields2 = map_and_concat mapper2 x.fields in
    sprintf
      "      | %s {\n%s      } ->\n%s         acc\n"
      x.kind_name
      fields
      fields2

  let to_children_names x =
    let mapper1 (f, _) = sprintf "        %s_%s;\n" x.prefix f in
    let mapper2 (f, _) = sprintf "        \"%s_%s\";\n" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let fields2 = map_and_concat mapper2 x.fields in
    sprintf
      "      | %s {\n%s      } -> [\n%s      ]\n"
      x.kind_name
      fields1
      fields2

  let to_syntax_from_children x =
    let mapper (f, _) = sprintf "          %s_%s;\n" x.prefix f in
    let fields = map_and_concat mapper x.fields in
    sprintf
      "      | (SyntaxKind.%s, [
%s        ]) ->
        %s {
%s        }
"
      x.kind_name
      fields
      x.kind_name
      fields

  let to_constructor_methods x =
    let mapper1 (f, _) = sprintf "        %s_%s\n" x.prefix f in
    let fields1 = map_and_concat mapper1 x.fields in
    let mapper2 (f, _) = sprintf "          %s_%s;\n" x.prefix f in
    let fields2 = map_and_concat mapper2 x.fields in
    sprintf
      "      let make_%s
%s      =
        let syntax = %s {
%s        } in
        let value = ValueBuilder.value_from_syntax syntax in
        make syntax value

"
      x.type_name
      fields1
      x.kind_name
      fields2

  let to_from_methods x =
    if omit_syntax_record x then
      ""
    else
      let mapper (f, _) = sprintf "          %s_%s;\n" x.prefix f in
      let fields = map_and_concat mapper x.fields in
      sprintf
        "     let from_%s {
%s       } = %s {
%s       }
"
        x.type_name
        fields
        x.kind_name
        fields

  let to_get_methods x =
    if omit_syntax_record x then
      ""
    else
      let mapper (f, _) = sprintf "          %s_%s;\n" x.prefix f in
      let fields = map_and_concat mapper x.fields in
      sprintf
        "     let get_%s x =
        match x with
        | %s {\n%s            } -> {\n%s           }
        | _ -> failwith \"get_%s: not a %s\"
"
        x.type_name
        x.kind_name
        fields
        fields
        x.type_name
        x.kind_name

  let full_fidelity_syntax_template =
    make_header
      MLStyle
      "
 * With these factory methods, nodes can be built up from their child nodes. A
 * factory method must not just know all the children and the kind of node it is
 * constructing; it also must know how to construct the value that this node is
 * going to be tagged with. For that reason, an optional functor is provided.
 * This functor requires that methods be provided to construct the values
 * associated with a token or with any arbitrary node, given its children. If
 * this functor is used then the resulting module contains factory methods.
 *
 * This module also provides some useful helper functions, like an iterator,
 * a rewriting visitor, and so on."
    ^ "

open Hh_prelude
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
      SyntaxKind.equal (kind node) syntax_kind

    let is_missing node =
      match kind node with
      | SyntaxKind.Missing -> true
      | _ -> false

    let is_list node =
      match kind node with
      | SyntaxKind.SyntaxList -> true
      | _ -> false

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
      | Token t -> TokenKind.equal (Token.kind t) kind
      | _ -> false

    let is_namespace_prefix node =
      match syntax node with
      | QualifiedName e ->
        begin match List.last (syntax_node_to_list e.qualified_name_parts) with
        | None -> false
        | Some p ->
          begin match syntax p with
          | ListItem p -> not (is_missing p.list_separator)
          | _ -> false
          end
        end
      | _ -> false

    let has_leading_trivia kind token =
      List.exists (Token.leading token)
        ~f:(fun trivia ->
            Full_fidelity_trivia_kind.equal (Token.Trivia.kind trivia) kind)

    let is_external e =
      is_specific_token TokenKind.Semicolon e || is_missing e

    let is_name       = is_specific_token TokenKind.Name
    let is_construct  = is_specific_token TokenKind.Construct
    let is_static     = is_specific_token TokenKind.Static
    let is_private    = is_specific_token TokenKind.Private
    let is_public     = is_specific_token TokenKind.Public
    let is_protected  = is_specific_token TokenKind.Protected
    let is_abstract   = is_specific_token TokenKind.Abstract
    let is_final      = is_specific_token TokenKind.Final
    let is_async      = is_specific_token TokenKind.Async
    let is_void       = is_specific_token TokenKind.Void
    let is_left_brace = is_specific_token TokenKind.LeftBrace
    let is_ellipsis   = is_specific_token TokenKind.DotDotDot
    let is_comma      = is_specific_token TokenKind.Comma
    let is_ampersand  = is_specific_token TokenKind.Ampersand
    let is_inout      = is_specific_token TokenKind.Inout

    let syntax_list_fold ~init ~f node =
      match syntax node with
      | SyntaxList sl ->
        List.fold_left
          ~init
          ~f:(fun init li -> match syntax li with
              | ListItem { list_item; _; }-> f init list_item
              | Missing -> init
              | _ -> f init li)
          sl
      | Missing -> init
      | _ -> f init node

    let fold_over_children f acc syntax =
      match syntax with
      | Missing -> acc
      | Token _ -> acc
      | SyntaxList items ->
        List.fold_left ~f ~init:acc items
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

    let rec to_json_ ?(with_value = false) ?(ignore_missing = false) node =
      let open Hh_json in
      let ch = match node.syntax with
      | Token t -> [ \"token\", Token.to_json t ]
      | SyntaxList x -> [ (\"elements\",
        JSON_Array (List.filter_map ~f:(to_json_ ~with_value ~ignore_missing) x)) ]
      | _ ->
        let rec aux acc c n =
          match c, n with
          | ([], []) -> acc
          | ((hc :: tc), (hn :: tn)) ->
            let result = (to_json_ ~with_value ~ignore_missing) hc in
            (match result with
            | Some r -> aux ((hn, r):: acc) tc tn
            | None -> aux acc tc tn)
          | _ -> failwith \"mismatch between children and names\" in
        List.rev (aux [] (children node) (children_names node)) in
      let k = (\"kind\", JSON_String (SyntaxKind.to_string (kind node))) in
      let v = if with_value then
        (\"value\", SyntaxValue.to_json node.value) :: ch
        else ch in
      if ignore_missing && (List.is_empty ch) then None else Some(JSON_Object (k :: v))

    let to_json ?(with_value = false) ?(ignore_missing = false) node =
      match to_json_ ~with_value ~ignore_missing node with
      | Some x -> x
      | None -> Hh_json.JSON_Object([])

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
          if Option.is_none token then
            let result = aux (children h) in
            if Option.is_none result then aux t else result
          else
            token in
      aux [node]

    let trailing_token node =
      let rec aux nodes =
        match nodes with
        | [] -> None
        | h :: t ->
          let token = get_token h in
          if Option.is_none token then
            let result = aux (List.rev (children h)) in
            if Option.is_none result then aux t else result
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

    end
  end
end
"

  let full_fidelity_syntax =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [
          { pattern = "TO_KIND"; func = to_to_kind };
          { pattern = "TYPE_TESTS"; func = to_type_tests };
          { pattern = "CHILDREN"; func = to_children };
          { pattern = "FOLD_FROM_SYNTAX"; func = to_fold_from_syntax };
          { pattern = "CHILDREN_NAMES"; func = to_children_names };
          { pattern = "SYNTAX_FROM_CHILDREN"; func = to_syntax_from_children };
          { pattern = "CONSTRUCTOR_METHODS"; func = to_constructor_methods };
          { pattern = "FROM_METHODS"; func = to_from_methods };
          { pattern = "GET_METHODS"; func = to_get_methods };
        ]
      ~filename:(full_fidelity_path_prefix ^ "full_fidelity_syntax.ml")
      ~template:full_fidelity_syntax_template
      ()
end

module GenerateFFTriviaKind = struct
  let to_trivia { trivia_kind; trivia_text = _ } =
    sprintf "  | %s\n" trivia_kind

  let to_to_string { trivia_kind; trivia_text } =
    sprintf
      ("  | " ^^ trivia_kind_fmt ^^ " -> \"%s\"\n")
      trivia_kind
      trivia_text

  let full_fidelity_trivia_kind_template =
    make_header MLStyle ""
    ^ "

type t =
TRIVIA
  [@@deriving show, enum, eq]

let to_string kind =
  match kind with
TO_STRING"

  let full_fidelity_trivia_kind =
    Full_fidelity_schema.make_template_file
      ~trivia_transformations:
        [
          { trivia_pattern = "TRIVIA"; trivia_func = map_and_concat to_trivia };
          {
            trivia_pattern = "TO_STRING";
            trivia_func = map_and_concat to_to_string;
          };
        ]
      ~filename:(full_fidelity_path_prefix ^ "/full_fidelity_trivia_kind.ml")
      ~template:full_fidelity_trivia_kind_template
      ()
end

(* GenerateFFSyntaxKind *)

module GenerateFFRustTriviaKind = struct
  let ocaml_tag = ref (-1)

  let to_trivia { trivia_kind; trivia_text = _ } =
    incr ocaml_tag;
    sprintf "    %s = %d,\n" trivia_kind !ocaml_tag

  let to_to_string { trivia_kind; trivia_text } =
    sprintf "            TriviaKind::%s => \"%s\",\n" trivia_kind trivia_text

  let full_fidelity_trivia_kind_template =
    make_header CStyle ""
    ^ "

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

#[derive(Debug, Copy, Clone, FromOcamlRep, ToOcamlRep, PartialEq)]
#[repr(u8)]
pub enum TriviaKind {
TRIVIA}

impl TriviaKind {
    pub fn to_string(&self) -> &str {
        match self {
TO_STRING        }
    }

    pub const fn ocaml_tag(self) -> u8 {
        self as u8
    }
}
"

  let full_fidelity_trivia_kind =
    Full_fidelity_schema.make_template_file
      ~trivia_transformations:
        [
          { trivia_pattern = "TRIVIA"; trivia_func = map_and_concat to_trivia };
          {
            trivia_pattern = "TO_STRING";
            trivia_func = map_and_concat to_to_string;
          };
        ]
      ~filename:(full_fidelity_path_prefix ^ "trivia_kind.rs")
      ~template:full_fidelity_trivia_kind_template
      ()
end

(* GenerateFFRustTriviaKind *)

module GenerateFFSyntaxKind = struct
  let to_tokens x = sprintf "  | %s\n" x.kind_name

  let to_to_string x =
    sprintf
      ("  | " ^^ kind_name_fmt ^^ " -> \"%s\"\n")
      x.kind_name
      x.description

  let full_fidelity_syntax_kind_template =
    make_header MLStyle ""
    ^ "

type t =
  | Token of Full_fidelity_token_kind.t
  | Missing
  | SyntaxList
TOKENS
  [@@deriving show, eq]

let to_string kind =
  match kind with
  | Token _                           -> \"token\"
  | Missing                           -> \"missing\"
  | SyntaxList                        -> \"list\"
TO_STRING"

  let full_fidelity_syntax_kind =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [
          { pattern = "TOKENS"; func = to_tokens };
          { pattern = "TO_STRING"; func = to_to_string };
        ]
      ~filename:(full_fidelity_path_prefix ^ "full_fidelity_syntax_kind.ml")
      ~template:full_fidelity_syntax_kind_template
      ()
end

(* GenerateFFTriviaKind *)

module GenerateFFRustSyntaxKind = struct
  let to_tokens x = sprintf "    %s,\n" x.kind_name

  let to_to_string x =
    sprintf
      ("            SyntaxKind::" ^^ kind_name_fmt ^^ " => \"%s\",\n")
      x.kind_name
      x.description

  let tag = ref 1

  let to_ocaml_tag x =
    incr tag;
    sprintf "            SyntaxKind::%s => %d,\n" x.kind_name !tag

  let full_fidelity_syntax_kind_template =
    make_header CStyle ""
    ^ "

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

use crate::token_kind::TokenKind;

#[derive(Debug, Copy, Clone, FromOcamlRep, ToOcamlRep, PartialEq)]
pub enum SyntaxKind {
    Missing,
    Token(TokenKind),
    SyntaxList,
TOKENS
}

impl SyntaxKind {
    pub fn to_string(&self) -> &str {
        match self {
            SyntaxKind::SyntaxList => \"list\",
            SyntaxKind::Missing => \"missing\",
            SyntaxKind::Token(_) => \"token\",
TO_STRING        }
    }

    pub fn ocaml_tag(self) -> u8 {
        match self {
            SyntaxKind::Missing => 0,
            SyntaxKind::Token(_) => 0,
            SyntaxKind::SyntaxList => 1,
OCAML_TAG        }
    }
}
"

  let full_fidelity_syntax_kind =
    Full_fidelity_schema.make_template_file
      ~transformations:
        [
          { pattern = "TOKENS"; func = to_tokens };
          { pattern = "TO_STRING"; func = to_to_string };
          { pattern = "OCAML_TAG"; func = to_ocaml_tag };
        ]
      ~filename:(full_fidelity_path_prefix ^ "syntax_kind.rs")
      ~template:full_fidelity_syntax_kind_template
      ()
end

(* GenerateFFRustSyntaxKind *)

module GenerateFFTokenKind = struct
  let given_text_width =
    let folder acc x = max acc (String.length x.token_text) in
    List.fold_left ~f:folder ~init:0 given_text_tokens

  let to_kind_declaration x = sprintf "  | %s\n" x.token_kind

  let add_guard_or_pad :
      cond:bool * string -> ?else_cond:bool * string -> string -> string =
   fun ~cond:(cond, guard) ?else_cond guards ->
    let pad str = String.make (String.length str) ' ' in
    let is_only_spaces str = String.equal str (pad str) in
    let make_same_length str1 str2 =
      let blanks n = (try String.make n ' ' with Invalid_argument _ -> "") in
      let (len1, len2) = (String.length str1, String.length str2) in
      let str1 = str1 ^ blanks (len2 - len1) in
      let str2 = str2 ^ blanks (len1 - len2) in
      (str1, str2)
    in
    let (else_cond, else_guard) =
      match else_cond with
      | Some (cond, guard) -> (cond, guard)
      | None -> (false, "")
    in
    let prefix =
      if cond || else_cond then
        if is_only_spaces guards then
          "when "
        else
          "&&   "
      else
        "     "
    in
    let (guard, else_guard) = make_same_length guard else_guard in
    let guard =
      if cond then
        guard
      else if else_cond then
        else_guard
      else
        pad guard
    in
    guards ^ prefix ^ guard ^ " "

  let to_from_string x =
    let token_text = escape_token_text x.token_text in
    let spacer_width = given_text_width - String.length token_text in
    let spacer = String.make spacer_width ' ' in
    let guards =
      add_guard_or_pad "" ~cond:(x.allowed_as_identifier, "not only_reserved")
    in
    sprintf "  | \"%s\"%s %s-> Some %s\n" token_text spacer guards x.token_kind

  let to_to_string x =
    let token_text = escape_token_text x.token_text in
    sprintf ("  | " ^^ token_kind_fmt ^^ " -> \"%s\"\n") x.token_kind token_text

  let to_is_variable_text x = sprintf "  | %s -> true\n" x.token_kind

  let full_fidelity_token_kind_template =
    make_header MLStyle ""
    ^ "

type t =
  (* No text tokens *)
KIND_DECLARATIONS_NO_TEXT  (* Given text tokens *)
KIND_DECLARATIONS_GIVEN_TEXT  (* Variable text tokens *)
KIND_DECLARATIONS_VARIABLE_TEXT
  [@@deriving show, eq]

let from_string keyword ~only_reserved =
  match keyword with
  | \"true\"            when not only_reserved -> Some BooleanLiteral
  | \"false\"           when not only_reserved -> Some BooleanLiteral
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
    Full_fidelity_schema.make_template_file
      ~token_no_text_transformations:
        [
          {
            token_pattern = "KIND_DECLARATIONS_NO_TEXT";
            token_func = map_and_concat to_kind_declaration;
          };
          {
            token_pattern = "TO_STRING_NO_TEXT";
            token_func = map_and_concat to_to_string;
          };
        ]
      ~token_given_text_transformations:
        [
          {
            token_pattern = "KIND_DECLARATIONS_GIVEN_TEXT";
            token_func = map_and_concat to_kind_declaration;
          };
          {
            token_pattern = "FROM_STRING_GIVEN_TEXT";
            token_func = map_and_concat to_from_string;
          };
          {
            token_pattern = "TO_STRING_GIVEN_TEXT";
            token_func = map_and_concat to_to_string;
          };
        ]
      ~token_variable_text_transformations:
        [
          {
            token_pattern = "KIND_DECLARATIONS_VARIABLE_TEXT";
            token_func = map_and_concat to_kind_declaration;
          };
          {
            token_pattern = "TO_STRING_VARIABLE_TEXT";
            token_func = map_and_concat to_to_string;
          };
          {
            token_pattern = "IS_VARIABLE_TEXT_VARIABLE_TEXT";
            token_func = map_and_concat to_is_variable_text;
          };
        ]
      ~filename:(full_fidelity_path_prefix ^ "full_fidelity_token_kind.ml")
      ~template:full_fidelity_token_kind_template
      ()
end

(* GenerateFFTokenKind *)

module GenerateFFRustTokenKind = struct
  let token_kind x =
    match x.token_kind with
    | "Self" -> "SelfToken"
    | x -> x

  let to_from_string x =
    let token_text = escape_token_text x.token_text in
    let guard =
      if x.allowed_as_identifier then
        "!only_reserved"
      else
        ""
    in
    let guard =
      if String.equal guard "" then
        ""
      else
        " if " ^ guard
    in
    sprintf
      "            \"%s\"%s => Some(TokenKind::%s),\n"
      token_text
      guard
      (token_kind x)

  let rust_tag = ref (-1)

  let to_kind_declaration x =
    incr rust_tag;
    sprintf "    %s = %d,\n" (token_kind x) !rust_tag

  let token_text x = escape_token_text x.token_text

  let to_to_string x =
    sprintf
      "            TokenKind::%s => \"%s\",\n"
      (token_kind x)
      (token_text x)

  let ocaml_tag = ref (-1)

  let to_ocaml_tag x =
    incr ocaml_tag;
    sprintf "            TokenKind::%s => %d,\n" (token_kind x) !ocaml_tag

  let from_u8_tag = ref (-1)

  let to_try_from_u8 x =
    incr from_u8_tag;
    sprintf
      "            %d => Some(TokenKind::%s),\n"
      !from_u8_tag
      (token_kind x)

  let to_width x =
    let len =
      if String.equal (token_kind x) "Backslash" then
        1
      else
        String.length (token_text x)
    in
    assert (len > 0);
    sprintf
      "            TokenKind::%s => Some(unsafe { NonZeroUsize::new_unchecked(%d) }),\n"
      (token_kind x)
      len

  let full_fidelity_rust_token_kind_template =
    make_header CStyle ""
    ^ "

use std::num::NonZeroUsize;

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

#[allow(non_camel_case_types)] // allow Include_once and Require_once
#[derive(Debug, Copy, Clone, PartialEq, Ord, Eq, PartialOrd, FromOcamlRep, ToOcamlRep)]
#[repr(u8)]
pub enum TokenKind {
    // No text tokens
KIND_DECLARATIONS_NO_TEXT    // Given text tokens
KIND_DECLARATIONS_GIVEN_TEXT    // Variable text tokens
KIND_DECLARATIONS_VARIABLE_TEXT}

impl TokenKind {
    pub fn to_string(self) -> &'static str {
        match self {
            // No text tokens
TO_STRING_NO_TEXT            // Given text tokens
TO_STRING_GIVEN_TEXT            // Variable text tokes
TO_STRING_VARIABLE_TEXT        }
    }

    pub fn from_string(
        keyword: &[u8],
        only_reserved: bool,
    ) -> Option<Self> {
        let keyword = unsafe { std::str::from_utf8_unchecked(keyword) };
        match keyword {
            \"true\" if !only_reserved => Some(TokenKind::BooleanLiteral),
            \"false\" if !only_reserved => Some(TokenKind::BooleanLiteral),
FROM_STRING_GIVEN_TEXT            _ => None,
        }
    }

    pub fn ocaml_tag(self) -> u8 {
        match self {
OCAML_TAG_NO_TEXTOCAML_TAG_GIVEN_TEXTOCAML_TAG_VARIABLE_TEXT        }
    }

    pub fn try_from_u8(tag: u8) -> Option<Self> {
        match tag {
FROM_U8_NO_TEXTFROM_U8_GIVEN_TEXTFROM_U8_VARIABLE_TEXT            _ => None,
        }
    }

    pub fn fixed_width(self) -> Option<NonZeroUsize> {
        match self {
WIDTH_GIVEN_TEXT            _ => None,
        }
    }
}
"

  let full_fidelity_token_kind =
    Full_fidelity_schema.make_template_file
      ~token_no_text_transformations:
        [
          {
            token_pattern = "KIND_DECLARATIONS_NO_TEXT";
            token_func = map_and_concat to_kind_declaration;
          };
          {
            token_pattern = "TO_STRING_NO_TEXT";
            token_func = map_and_concat to_to_string;
          };
          {
            token_pattern = "OCAML_TAG_NO_TEXT";
            token_func = map_and_concat to_ocaml_tag;
          };
          {
            token_pattern = "FROM_U8_NO_TEXT";
            token_func = map_and_concat to_try_from_u8;
          };
        ]
      ~token_given_text_transformations:
        [
          {
            token_pattern = "KIND_DECLARATIONS_GIVEN_TEXT";
            token_func = map_and_concat to_kind_declaration;
          };
          {
            token_pattern = "FROM_STRING_GIVEN_TEXT";
            token_func = map_and_concat to_from_string;
          };
          {
            token_pattern = "TO_STRING_GIVEN_TEXT";
            token_func = map_and_concat to_to_string;
          };
          {
            token_pattern = "OCAML_TAG_GIVEN_TEXT";
            token_func = map_and_concat to_ocaml_tag;
          };
          {
            token_pattern = "FROM_U8_GIVEN_TEXT";
            token_func = map_and_concat to_try_from_u8;
          };
          {
            token_pattern = "WIDTH_GIVEN_TEXT";
            token_func = map_and_concat to_width;
          };
        ]
      ~token_variable_text_transformations:
        [
          {
            token_pattern = "KIND_DECLARATIONS_VARIABLE_TEXT";
            token_func = map_and_concat to_kind_declaration;
          };
          {
            token_pattern = "TO_STRING_VARIABLE_TEXT";
            token_func = map_and_concat to_to_string;
          };
          {
            token_pattern = "OCAML_TAG_VARIABLE_TEXT";
            token_func = map_and_concat to_ocaml_tag;
          };
          {
            token_pattern = "FROM_U8_VARIABLE_TEXT";
            token_func = map_and_concat to_try_from_u8;
          };
        ]
      ~filename:(full_fidelity_path_prefix ^ "token_kind.rs")
      ~template:full_fidelity_rust_token_kind_template
      ()
end

(* GenerateFFTRustTokenKind *)

module GenerateFFOperatorRust = struct
  let template =
    make_header CStyle ""
    ^ "

use ocamlrep_derive::{FromOcamlRep, ToOcamlRep};

#[derive(FromOcamlRep, ToOcamlRep)]
pub enum Operator {
OPERATORS}
"

  let full_fidelity_operators =
    Full_fidelity_schema.make_template_file
      ~operator_transformations:
        [
          {
            operator_pattern = "OPERATORS";
            operator_func =
              map_and_concat (fun op -> sprintf "    %sOperator,\n" op.name);
          };
        ]
      ~filename:(full_fidelity_path_prefix ^ "operator_generated.rs")
      ~template
      ()
end

(* GenerateFFOperatorRust *)

module GenerateFFOperator = struct
  let template =
    make_header MLStyle ""
    ^ "
module type Sig = sig
  type t =
OPERATOR_DECL_SIGend

module Impl : Sig = struct
  type t =
OPERATOR_DECL_IMPLend
"

  let op_pattern prefix op = sprintf "%s| %sOperator\n" prefix op.name

  let transform pattern =
    {
      operator_pattern = pattern;
      operator_func = map_and_concat (op_pattern "  ");
    }

  let full_fidelity_operator =
    Full_fidelity_schema.make_template_file
      ~operator_transformations:
        [transform "OPERATOR_DECL_SIG"; transform "OPERATOR_DECL_IMPL"]
      ~filename:
        (full_fidelity_path_prefix ^ "full_fidelity_operator_generated.ml")
      ~template
      ()
end

module GenerateSchemaVersion = struct
  let template =
    make_header CStyle ""
    ^ sprintf
        "
pub const VERSION: &'static str = \"%s\";
"
        Full_fidelity_schema.full_fidelity_schema_version_number

  let gen =
    Full_fidelity_schema.make_template_file
      ~filename:
        "hphp/hack/src/parser/schema/full_fidelity_schema_version_number.rs"
      ~template
      ()
end

let templates =
  [
    GenerateFFOperatorRust.full_fidelity_operators;
    GenerateFFOperator.full_fidelity_operator;
    GenerateFFSyntaxType.full_fidelity_syntax_type;
    GenerateFFSyntaxSig.full_fidelity_syntax_sig;
    GenerateFFValidatedSyntax.full_fidelity_validated_syntax;
    GenerateFFTriviaKind.full_fidelity_trivia_kind;
    GenerateFFRustTriviaKind.full_fidelity_trivia_kind;
    GenerateFFSyntax.full_fidelity_syntax;
    GenerateFFRustSyntax.full_fidelity_syntax;
    GenerateFFRustSyntaxType.full_fidelity_syntax;
    GenerateFFSyntaxKind.full_fidelity_syntax_kind;
    GenerateFFRustSyntaxKind.full_fidelity_syntax_kind;
    GenerateFFTokenKind.full_fidelity_token_kind;
    GenerateFFRustTokenKind.full_fidelity_token_kind;
    GenerateFFJSONSchema.full_fidelity_json_schema;
    GenerateFFSmartConstructors.full_fidelity_smart_constructors;
    GenerateFFRustSmartConstructors.full_fidelity_smart_constructors;
    GenerateFFRustPositionedSmartConstructors.positioned_smart_constructors;
    GenerateFFRustVerifySmartConstructors.verify_smart_constructors;
    GenerateFFSyntaxSmartConstructors.full_fidelity_syntax_smart_constructors;
    GenerateFFRustSyntaxSmartConstructors
    .full_fidelity_syntax_smart_constructors;
    GenerateFFRustDeclModeSmartConstructors.decl_mode_smart_constructors;
    GenerateRustFlattenSmartConstructors.flatten_smart_constructors;
    GenerateRustFactsSmartConstructors.facts_smart_constructors;
    GenerateRustDirectDeclSmartConstructors.direct_decl_smart_constructors;
    GenerateFFSmartConstructorsWrappers
    .full_fidelity_smart_constructors_wrappers;
    GenerateFFRustSmartConstructorsWrappers
    .full_fidelity_smart_constructors_wrappers;
    GenerateFFRustSyntaxVariantByRef.full_fidelity_syntax;
    GenerateSyntaxTypeImpl.full_fidelity_syntax;
    GenerateSyntaxChildrenIterator.full_fidelity_syntax;
    GenerateFFRustSyntaxImplByRef.full_fidelity_syntax;
    GenerateSyntaxSerialize.gen;
    GenerateSchemaVersion.gen;
  ]
