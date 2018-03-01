(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Syntax     = Full_fidelity_positioned_syntax
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax(Syntax)
module TokenKind  = Full_fidelity_token_kind
module J = Hh_json

open Syntax

type type_kind =
  | TKClass
  | TKInterface
  | TKEnum
  | TKTrait
  | TKUnknown
  | TKMixed

let type_kind_to_string = function
  | TKClass -> "class"
  | TKInterface -> "interface"
  | TKEnum -> "enum"
  | TKTrait -> "trait"
  | TKUnknown -> "unknown"
  | TKMixed -> "mixed"

type type_facts =
{ base_types: SSet.t
; kind: type_kind
; flags: int
; require_extends: SSet.t
; require_implements: SSet.t }

type facts =
{ types: type_facts SMap.t
; functions: string list
; constants: string list
; type_aliases: string list }

let flags_default = 0
let flags_abstract = 1
let flags_final = 2
let flags_multiple_declarations = 4

let has_multiple_declarations_flag tf =
  (tf.flags land flags_multiple_declarations) <> 0

let combine_flags tf flags =
  { tf with flags = tf.flags lor flags }

let set_multiple_declarations_flag tf =
  combine_flags tf flags_multiple_declarations

let strip_list_item node =
  match node with
  | { syntax = ListItem { list_item = n; _ }; _} -> n
  | n -> n

let qualified_name_part_to_string p =
  match syntax p with
  | ListItem li -> (text li.list_item) ^ (text li.list_separator)
  | _ -> text p

let qualified_name namespace name_node =
  let name_text =
    match syntax name_node with
    | QualifiedName {
        qualified_name_parts = { syntax = SyntaxList parts; _}; _
      } ->
      String.concat "" @@ Core_list.map parts ~f:qualified_name_part_to_string
    | _ -> text name_node in
  if String.length namespace = 0
  then name_text
  else if String.get name_text 0 = '\\'
  then String.sub name_text 1 (String.length name_text - 1)
  else namespace ^ "\\" ^ name_text

let add_or_update_classish_declaration facts name kind flags base_types
  require_extends require_implements =
  match SMap.get name facts.types with
  | Some old_tf ->
    let tf =
      if old_tf.kind <> kind then { old_tf with kind = TKMixed }
      else old_tf in
    let tf =
      if not @@ has_multiple_declarations_flag tf
      then set_multiple_declarations_flag tf
      else tf in
    let tf =
      if tf.flags = flags then tf
      else combine_flags tf flags in
    let tf =
      if SSet.is_empty base_types
      then tf
      else {
        tf with base_types =
          SSet.union base_types tf.base_types } in
    let tf =
      if SSet.is_empty require_extends
      then tf
      else {
        tf with require_extends =
          SSet.union require_extends tf.require_extends } in
    let tf =
      if SSet.is_empty require_implements
      then tf
      else {
        tf with require_implements =
          SSet.union require_implements tf.require_implements } in

    if tf == old_tf then facts
    else { facts with types = SMap.add name tf facts.types }

  | None ->
    let base_types =
      if kind = TKEnum
      then SSet.add "HH\\BuiltinEnum" base_types
      else base_types in
    let tf =
      { base_types; flags; kind; require_extends; require_implements } in
    { facts with types = SMap.add name tf facts.types }

let facts_from_define_argument facts node =
  let node = strip_list_item node in
  match syntax node with
  | LiteralExpression {
      literal_expression = {
        syntax = Token {
          Token.kind = TokenKind.DoubleQuotedStringLiteral
                       | TokenKind.SingleQuotedStringLiteral; _
        }; _
      }; _
    } ->
    let constant_name = text node in
    (* strip quotes *)
    let constant_name =
      String.sub constant_name 1 (String.length constant_name - 2) in
    { facts with constants = constant_name :: facts.constants }
  | _ -> facts

let type_names_from_list ns init list =
  let aux acc n =
    match syntax @@ strip_list_item n with
    | SimpleTypeSpecifier { simple_type_specifier = t; _ }
    | GenericTypeSpecifier { generic_class_type = t; _ }
      when not @@ is_missing t ->
      let name = qualified_name ns t in
      SSet.add name acc
    | _ -> acc in
  match syntax list with
  | SyntaxList l -> Core_list.fold_left ~init ~f:aux l
  | _ -> init

let flags_from_modifiers modifiers =
  let aux acc n =
    match syntax n with
    | Token { Token.kind = TokenKind.Abstract; _ } ->
      acc lor flags_abstract
    | Token { Token.kind = TokenKind.Final; _ } ->
      acc lor flags_final
    | Token { Token.kind = TokenKind.Static; _ } ->
      acc lor flags_abstract lor flags_final
    | _ -> acc in
  fold_over_children aux flags_default (syntax modifiers)

let require_sections_from_classish_body ns body =
  match syntax body with
  | ClassishBody { classish_body_elements = e; _ } ->
    let aux ((extends, implements) as acc) n =
      begin match syntax n with
      | RequireClause { require_kind; require_name; _ } ->
        let name = qualified_name ns require_name in
        begin match syntax require_kind with
        | Token { Token.kind = TokenKind.Extends; _ } ->
          SSet.add name extends, implements
        | Token { Token.kind = TokenKind.Implements; _ } ->
          extends, SSet.add name implements
        | _ -> acc
        end
      | _ -> acc
      end in
    fold_over_children aux (SSet.empty, SSet.empty) (syntax e)
  | _ -> SSet.empty, SSet.empty

let facts_from_classish_declaration ns facts
  modifiers keyword name extends_list implements_list body =
  let name = qualified_name ns name in
  let kind, flags =
    match syntax keyword with
    | Token { Token.kind = TokenKind.Class; _ } ->
      TKClass, flags_from_modifiers modifiers
    | Token { Token.kind = TokenKind.Interface; _ } ->
      TKInterface, flags_abstract
    | Token { Token.kind = TokenKind.Trait; _ } ->
      TKTrait, flags_abstract
    | Token { Token.kind = TokenKind.Enum; _ } ->
      TKEnum, flags_final
    | _ -> TKUnknown, flags_default in
  let base_types = type_names_from_list ns SSet.empty extends_list in
  let base_types = type_names_from_list ns base_types implements_list in
  let require_extends, require_implements =
    begin match kind with
    | TKTrait | TKInterface -> require_sections_from_classish_body ns body
    | _ -> SSet.empty, SSet.empty
    end in
  add_or_update_classish_declaration facts name kind flags
    base_types require_extends require_implements

let rec collect_facts (ns, facts as acc) node =
  match syntax node with
  (* top level - collect facts for all top level declarations *)
  | Script { script_declarations; _ } ->
    fold_over_children collect_facts acc (syntax script_declarations)
  (* namespaces *)
  | NamespaceDeclaration { namespace_name; namespace_body; _ } ->
    (* if namespace body is empty - treat the rest of the file as if it is
       nested in current namespace  *)
    if is_namespace_empty_body namespace_body
    then
      (qualified_name "" namespace_name), facts
    else begin
    (* if namespace name is not specified - keep current *)
      let namespace_name =
        if is_missing namespace_name then ns
        else qualified_name ns namespace_name in
      match syntax namespace_body with
      | NamespaceBody { namespace_declarations = decls; _ } ->
        (* dive into namespace body, collect facts and restore namespace name *)
        let _, facts =
          fold_over_children collect_facts (namespace_name, facts) (syntax decls) in
        ns, facts
      | _ -> acc
    end
  (* class-like declarations *)
  | ClassishDeclaration {
      classish_modifiers = modifiers;
      classish_name = name;
      classish_keyword = keyword;
      classish_extends_list = extends_list;
      classish_implements_list = implements_list;
      classish_body = body; _ } when not @@ is_missing name ->
    let facts =
      facts_from_classish_declaration ns facts
        modifiers keyword name extends_list implements_list body in
    ns, facts

  (* type/newtype *)
  | AliasDeclaration { alias_name; _ } ->
    let alias_name = qualified_name ns alias_name in
    ns, { facts with type_aliases = alias_name :: facts.type_aliases }

  (* enums *)
  | EnumDeclaration { enum_name; _ } ->
    let enum_name = qualified_name ns enum_name in
    let facts =
      add_or_update_classish_declaration facts
        enum_name TKEnum flags_final SSet.empty SSet.empty SSet.empty in
    ns, facts

  (* top level define call *)
  | ExpressionStatement {
      expression_statement_expression = {
        syntax = DefineExpression {
        define_argument_list = {
        syntax = SyntaxList [ arg1; _ ];
      _ }; _ }; _ }; _
    } when String.length ns = 0 ->
    ns, facts_from_define_argument facts arg1

  (*  constants *)
  | ConstDeclaration { const_declarators = { syntax = SyntaxList l; _}; _ } ->
    let aux facts n =
      begin match syntax @@ strip_list_item n with
      | ConstantDeclarator { constant_declarator_name = n; _ }
        when not @@ is_missing n ->
        let constant_name = qualified_name ns n in
        { facts with constants = constant_name :: facts.constants }
      | _ -> facts
      end in
    let facts = Core_list.fold_left ~init:facts ~f:aux l in
    ns, facts

  (* function declarations *)
  | FunctionDeclaration {
      function_declaration_header = {
        syntax = FunctionDeclarationHeader { function_name = n; _ }; _
      }; _
    } when not @@ is_missing n ->
    let function_name = qualified_name ns n in
    ns, { facts with functions = function_name :: facts.functions }
  | _ -> acc

let hex_number_to_json s =
  let number =
    "0x" ^ s
    |> Int64.of_string
    |> Int64.to_string in
  J.JSON_Number number

let add_member ~include_empty name values members =
  if SSet.is_empty values && not include_empty
  then members
  else
  let elements = SSet.fold (fun el acc -> J.JSON_String el :: acc ) values [] in
  (name, J.JSON_Array elements) :: members

let list_to_json_array l =
  let elements =
    Core_list.fold_left l ~init:[] ~f:(fun acc el -> J.JSON_String el :: acc) in
  J.JSON_Array elements

let type_facts_to_json name tf =
  let members =
    add_member ~include_empty:(tf.kind = TKTrait)
      "requireImplements" tf.require_implements []
    |> add_member ~include_empty:(tf.kind = TKInterface || tf.kind = TKTrait)
      "requireExtends" tf.require_extends
    |> add_member ~include_empty:true "baseTypes" tf.base_types in
  let members =
    ("name", J.JSON_String name)::
    ("kindOf", J.JSON_String (type_kind_to_string tf.kind))::
    ("flags", J.JSON_Number (string_of_int tf.flags)) ::
    members in
  J.JSON_Object members

let facts_to_json md5 facts =
  let md5sum0 =
    "md5sum0", hex_number_to_json (String.sub md5 0 16) in
  let md5sum1 =
    "md5sum1", hex_number_to_json (String.sub md5 16 16) in
  let type_facts_json =
    let elements =
      SMap.fold (fun name v acc -> (type_facts_to_json name v) :: acc)
        facts.types [] in
    "types", J.JSON_Array elements in
  let functions_json =
    "functions", list_to_json_array facts.functions in
  let constants_json =
    "constants", list_to_json_array facts.constants in
  let type_aliases_json =
    "typeAliases", list_to_json_array facts.type_aliases in
  J.JSON_Object [
    md5sum0;
    md5sum1;
    type_facts_json;
    functions_json;
    constants_json;
    type_aliases_json; ]

let from_text php5_compat_mode s =
  let env = Full_fidelity_parser_env.make ~php5_compat_mode () in
  let root =
    Full_fidelity_source_text.make Relative_path.default s
    |> SyntaxTree.make ~env
    |> SyntaxTree.root in
  let initial_facts = {
    types = SMap.empty;
    functions = [];
    constants = [];
    type_aliases = []
  } in
  let _, facts = collect_facts ("", initial_facts) root in
  facts

let extract_as_json ~php5_compat_mode text =
  let md5 = Digest.to_hex @@ Digest.string text in
  let facts = from_text php5_compat_mode text in
  facts_to_json md5 facts
