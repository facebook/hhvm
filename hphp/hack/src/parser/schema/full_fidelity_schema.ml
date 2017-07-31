(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* If you make changes to the schema that cause it to serialize / deserialize
differently, please update this version number *)
let full_fidelity_schema_version_number = "2017-07-24-0001"
(* TODO: Consider basing the version number on an auto-generated
hash of a file rather than relying on people remembering to update it. *)
(* TODO: It may be worthwhile to investigate how Thrift describes data types
and use that standard. *)

include Schema_definition
let schema_map =
  let add map ({ kind_name; _ } as schema_node) =
    SMap.add kind_name schema_node map
  in
  List.fold_left add SMap.empty @@
    { kind_name   = "Token"
    ; type_name   = "Token.t"
    ; func_name   = "token"
    ; description = "token"
    ; prefix      = "token"
    ; aggregates  = [ Expression ]
    ; fields      = []
    } ::
    { kind_name   = "error"
    ; type_name   = "error"
    ; func_name   = "error"
    ; description = "error"
    ; prefix      = "error"
    ; aggregates  = []
    ; fields      = []
    } ::
    schema

type trivia_node = {
  trivia_kind : string;
  trivia_text : string
}

type token_node = {
    token_kind : string;
    token_text : string
}

type transformation =
{
  pattern : string;
  func : schema_node -> string
}

type token_transformation =
{
  token_pattern : string;
  token_func : token_node list -> string
}

type trivia_transformation =
{
  trivia_pattern : string;
  trivia_func : trivia_node list -> string
}

type aggregate_transformation =
{
  aggregate_pattern : string;
  aggregate_func : aggregate_type -> string
}

type template_file =
{
  filename : string;
  template : string;
  transformations : transformation list;
  token_no_text_transformations : token_transformation list;
  token_variable_text_transformations : token_transformation list;
  token_given_text_transformations : token_transformation list;
  trivia_transformations : trivia_transformation list;
  aggregate_transformations : aggregate_transformation list;
}

let token_node_from_list l =
  match l with
  | [ token_kind; token_text ] ->
    { token_kind; token_text }
  | _ -> failwith "bad token schema"

let trivia_node_from_list l =
  match l with
  | [ trivia_kind; trivia_text ] ->
    { trivia_kind; trivia_text }
  | _ -> failwith "bad trivia schema"

let variable_text_tokens = List.map token_node_from_list [
  [ "ErrorToken"; "error_token" ];
  [ "Name"; "name" ];
  [ "QualifiedName"; "qualified_name" ];
  [ "Variable"; "variable" ];
  [ "NamespacePrefix"; "namespace_prefix" ];
  [ "DecimalLiteral"; "decimal_literal" ];
  [ "OctalLiteral"; "octal_literal" ];
  [ "HexadecimalLiteral"; "hexadecimal_literal" ];
  [ "BinaryLiteral"; "binary_literal" ];
  [ "FloatingLiteral"; "floating_literal" ];
  [ "ExecutionString"; "execution_string" ];
  [ "SingleQuotedStringLiteral"; "single_quoted_string_literal" ];
  [ "DoubleQuotedStringLiteral"; "double_quoted_string_literal" ];
  [ "DoubleQuotedStringLiteralHead"; "double_quoted_string_literal_head" ];
  [ "StringLiteralBody"; "string_literal_body" ];
  [ "DoubleQuotedStringLiteralTail"; "double_quoted_string_literal_tail" ];
  [ "HeredocStringLiteral"; "heredoc_string_literal" ];
  [ "HeredocStringLiteralHead"; "heredoc_string_literal_head" ];
  [ "HeredocStringLiteralTail"; "heredoc_string_literal_tail" ];
  [ "NowdocStringLiteral"; "nowdoc_string_literal" ];
  [ "BooleanLiteral"; "boolean_literal" ];
  [ "XHPCategoryName"; "XHP_category_name" ];
  [ "XHPElementName"; "XHP_element_name" ];
  [ "XHPClassName"; "XHP_class_name" ];
  [ "XHPStringLiteral"; "XHP_string_literal" ];
  [ "XHPBody"; "XHP_body" ];
  [ "XHPComment"; "XHP_comment" ];
  [ "Markup"; "markup" ]]

let no_text_tokens = List.map token_node_from_list [
  [ "EndOfFile"; "end_of_file" ]]

let given_text_tokens = List.map token_node_from_list [
  [ "Abstract"; "abstract" ];
  [ "And"; "and" ];
  [ "Array"; "array" ];
  [ "Arraykey"; "arraykey" ];
  [ "As"; "as" ];
  [ "Async"; "async" ];
  [ "Attribute"; "attribute" ];
  [ "Await"; "await" ];
  [ "Bool"; "bool" ];
  [ "Break"; "break" ];
  [ "Case"; "case" ];
  [ "Catch"; "catch" ];
  [ "Category"; "category" ];
  [ "Children"; "children" ];
  [ "Class"; "class" ];
  [ "Classname"; "classname" ];
  [ "Clone"; "clone" ];
  [ "Const"; "const" ];
  [ "Construct"; "__construct" ];
  [ "Continue"; "continue" ];
  [ "Coroutine"; "coroutine" ];
  [ "Darray"; "darray" ];
  [ "Default"; "default" ];
  [ "Define"; "define"];
  [ "Destruct"; "__destruct" ];
  [ "Dict"; "dict" ];
  [ "Do"; "do" ];
  [ "Double"; "double" ];
  [ "Echo"; "echo" ];
  [ "Else"; "else" ];
  [ "Elseif"; "elseif" ];
  [ "Empty"; "empty" ];
  [ "Enum"; "enum" ];
  [ "Eval"; "eval" ];
  [ "Extends"; "extends" ];
  [ "Fallthrough"; "fallthrough" ];
  [ "Float"; "float" ];
  [ "Final"; "final" ];
  [ "Finally"; "finally" ];
  [ "For"; "for" ];
  [ "Foreach"; "foreach" ];
  [ "From"; "from"];
  [ "Function"; "function" ];
  [ "Global"; "global" ];
  [ "Goto"; "goto" ];
  [ "If"; "if" ];
  [ "Implements"; "implements" ];
  [ "Include"; "include" ];
  [ "Include_once"; "include_once" ];
  [ "Instanceof"; "instanceof" ];
  [ "Insteadof"; "insteadof" ];
  [ "Int"; "int" ];
  [ "Interface"; "interface" ];
  [ "Isset"; "isset" ];
  [ "Keyset"; "keyset" ];
  [ "List"; "list" ];
  [ "Mixed"; "mixed" ];
  [ "Namespace"; "namespace" ];
  [ "New"; "new" ];
  [ "Newtype"; "newtype" ];
  [ "Noreturn"; "noreturn" ];
  [ "Num"; "num" ];
  [ "Object"; "object" ];
  [ "Or"; "or" ];
  [ "Parent"; "parent" ];
  [ "Print"; "print" ];
  [ "Private"; "private" ];
  [ "Protected"; "protected" ];
  [ "Public"; "public" ];
  [ "Require"; "require" ];
  [ "Require_once"; "require_once" ];
  [ "Required"; "required" ];
  [ "Resource"; "resource" ];
  [ "Return"; "return" ];
  [ "Self"; "self" ];
  [ "Shape"; "shape" ];
  [ "Static"; "static" ];
  [ "String"; "string" ];
  [ "Super"; "super" ];
  [ "Suspend"; "suspend" ];
  [ "Switch"; "switch" ];
  [ "This"; "this" ];
  [ "Throw"; "throw" ];
  [ "Trait"; "trait" ];
  [ "Try"; "try" ];
  [ "Tuple"; "tuple" ];
  [ "Type"; "type" ];
  [ "Unset"; "unset" ];
  [ "Use"; "use" ];
  [ "Var"; "var" ];
  [ "Varray"; "varray" ];
  [ "Vec"; "vec" ];
  [ "Void"; "void" ];
  [ "Where"; "where" ];
  [ "While"; "while" ];
  [ "Xor"; "xor" ];
  [ "Yield"; "yield" ];
  [ "LeftBracket"; "[" ];
  [ "RightBracket"; "]" ];
  [ "LeftParen"; "(" ];
  [ "RightParen"; ")" ];
  [ "LeftBrace"; "{" ];
  [ "RightBrace"; "}" ];
  [ "Dot"; "." ];
  [ "MinusGreaterThan"; "->" ];
  [ "PlusPlus"; "++" ];
  [ "MinusMinus"; "--" ];
  [ "StarStar"; "**" ];
  [ "Star"; "*" ];
  [ "Plus"; "+" ];
  [ "Minus"; "-" ];
  [ "Tilde"; "~" ];
  [ "Exclamation"; "!" ];
  [ "Dollar"; "$" ];
  [ "Slash"; "/" ];
  [ "Percent"; "%" ];
  [ "LessThanGreaterThan"; "<>"];
  [ "LessThanEqualGreaterThan"; "<=>"];
  [ "LessThanLessThan"; "<<" ];
  [ "GreaterThanGreaterThan"; ">>" ];
  [ "LessThan"; "<" ];
  [ "GreaterThan"; ">" ];
  [ "LessThanEqual"; "<=" ];
  [ "GreaterThanEqual"; ">=" ];
  [ "EqualEqual"; "==" ];
  [ "EqualEqualEqual"; "===" ];
  [ "ExclamationEqual"; "!=" ];
  [ "ExclamationEqualEqual"; "!==" ];
  [ "Carat"; "^" ];
  [ "Bar"; "|" ];
  [ "Ampersand"; "&" ];
  [ "AmpersandAmpersand"; "&&" ];
  [ "BarBar"; "||" ];
  [ "Question"; "?" ];
  [ "QuestionQuestion"; "??" ];
  [ "Colon"; ":" ];
  [ "Semicolon"; ";" ];
  [ "Equal"; "=" ];
  [ "StarStarEqual"; "**=" ];
  [ "StarEqual"; "*=" ];
  [ "SlashEqual"; "/=" ];
  [ "PercentEqual"; "%=" ];
  [ "PlusEqual"; "+=" ];
  [ "MinusEqual"; "-=" ];
  [ "DotEqual"; ".=" ];
  [ "LessThanLessThanEqual"; "<<=" ];
  [ "GreaterThanGreaterThanEqual"; ">>=" ];
  [ "AmpersandEqual"; "&=" ];
  [ "CaratEqual"; "^=" ];
  [ "BarEqual"; "|=" ];
  [ "Comma"; "," ];
  [ "At"; "@" ];
  [ "ColonColon"; "::" ];
  [ "EqualGreaterThan"; "=>" ];
  [ "EqualEqualGreaterThan"; "==>" ];
  [ "QuestionMinusGreaterThan"; "?->" ];
  [ "DotDotDot"; "..." ];
  [ "DollarDollar"; "$$" ];
  [ "BarGreaterThan"; "|>" ];
  [ "NullLiteral"; "null" ];
  [ "SlashGreaterThan"; "/>" ];
  [ "LessThanSlash"; "</" ];
  [ "LessThanQuestion";"<?" ];
  [ "QuestionGreaterThan"; "?>" ]]

let trivia_kinds = List.map trivia_node_from_list [
  [ "WhiteSpace"; "whitespace" ];
  [ "EndOfLine"; "end_of_line" ];
  [ "DelimitedComment"; "delimited_comment" ];
  [ "SingleLineComment"; "single_line_comment" ];
  [ "Unsafe"; "unsafe" ];
  [ "UnsafeExpression"; "unsafe_expression" ];
  [ "FixMe"; "fix_me" ];
  [ "IgnoreError"; "ignore_error" ];
  [ "FallThrough"; "fall_through" ];
  [ "ExtraTokenError"; "extra_token_error"]]

let map_and_concat_separated separator f items =
  String.concat separator (List.map f items)

let map_and_concat f items =
  map_and_concat_separated "" f items

let transform_schema f =
  map_and_concat f schema

let transform_tokens token_list f =
  map_and_concat f token_list

let transform_trivia trivia_list f =
  map_and_concat f trivia_list

let transform_aggregate f =
  map_and_concat f generated_aggregate_types

let replace pattern new_text source =
  Str.replace_first (Str.regexp pattern) new_text source

let generate_string template =
  let syntax_folder s x =
    replace x.pattern (transform_schema x.func) s in
  let tokens_folder token_list s x =
    replace x.token_pattern (x.token_func token_list) s in
  let trivia_folder trivia_list s x =
    replace x.trivia_pattern (x.trivia_func trivia_list) s in
  let aggregate_folder s x =
    replace x.aggregate_pattern (transform_aggregate x.aggregate_func) s in

  let result = List.fold_left
    syntax_folder template.template template.transformations in

  let result = List.fold_left (tokens_folder no_text_tokens)
    result template.token_no_text_transformations in
  let result = List.fold_left (tokens_folder variable_text_tokens)
    result template.token_variable_text_transformations in
  let result = List.fold_left (tokens_folder given_text_tokens)
    result template.token_given_text_transformations in
  let result = List.fold_left (trivia_folder trivia_kinds)
    result template.trivia_transformations in
  let result = List.fold_left
    aggregate_folder result template.aggregate_transformations in

  result

let generate_file template =
  let file = open_out template.filename in
  let s = generate_string template in
  Printf.fprintf file "%s" s;
  close_out file


module GenerateFFJSONSchema = struct
  let to_json_trivia { trivia_kind; trivia_text } =
    Printf.sprintf
"    { \"trivia_kind_name\" : \"%s\",
      \"trivia_type_name\" : \"%s\" }"
    trivia_kind trivia_text

  let to_json_given_text x =
    Printf.sprintf
"    { \"token_kind\" : \"%s\",
      \"token_text\" : \"%s\" },
"
    x.token_kind x.token_text

  let to_json_variable_text x =
    Printf.sprintf
"    { \"token_kind\" : \"%s\",
      \"token_text\" : null },
"
    x.token_kind

  let to_json_ast_nodes x =
    let mapper (f,_) = Printf.sprintf
"{ \"field_name\" : \"%s\" }" f in
    let fields = String.concat ",\n        " (List.map mapper x.fields) in
    Printf.sprintf
"    { \"kind_name\" : \"%s\",
      \"type_name\" : \"%s\",
      \"description\" : \"%s\",
      \"prefix\" : \"%s\",
      \"fields\" : [
        %s
      ] },
"  x.kind_name x.type_name x.description x.prefix fields

  let full_fidelity_json_schema_template =
"{ \"description\" :
  \"Auto-generated JSON schema of the Hack Full Fidelity Parser AST\",
  \"version\" : \"" ^ full_fidelity_schema_version_number ^ "\",
  \"trivia\" : [
TRIVIA_KINDS ],
  \"tokens\" : [
GIVEN_TEXT_TOKENS
VARIABLE_TEXT_TOKENS
    { \"token_kind\" : \"EndOfFile\",
      \"token_text\" : null } ],
  \"AST\" : [
AST_NODES
    { \"kind_name\" : \"Token\",
      \"type_name\" : \"token\",
      \"description\" : \"token\",
      \"prefix\" : \"\",
      \"fields\" : [
        { \"field_name\" : \"leading\" },
        { \"field_name\" : \"trailing\" } ] },
    { \"kind_name\" : \"Missing\",
      \"type_name\" : \"missing\",
      \"description\" : \"missing\",
      \"prefix\" : \"\",
      \"fields\" : [ ] },
    { \"kind_name\" : \"SyntaxList\",
      \"type_name\" : \"syntax_list\",
      \"description\" : \"syntax_list\",
      \"prefix\" : \"\",
      \"fields\" : [ ] } ] }"

  let full_fidelity_json_schema =
  {
    filename = "hphp/hack/src/parser/js/full_fidelity_schema.json";
    template = full_fidelity_json_schema_template;
    transformations = [
      { pattern = "AST_NODES"; func = to_json_ast_nodes }
    ];
    token_no_text_transformations = [ ];
    token_given_text_transformations = [
      { token_pattern = "GIVEN_TEXT_TOKENS";
        token_func = map_and_concat to_json_given_text } ];
    token_variable_text_transformations = [
      { token_pattern = "VARIABLE_TEXT_TOKENS";
        token_func = map_and_concat to_json_variable_text }];
    trivia_transformations = [
      { trivia_pattern = "TRIVIA_KINDS";
        trivia_func = map_and_concat_separated ",\n" to_json_trivia }
    ];
    aggregate_transformations = [];
  }

end (* GenerateFFJSONSchema *)

let schema_as_json () =
  generate_string GenerateFFJSONSchema.full_fidelity_json_schema
