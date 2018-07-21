(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* If you make changes to the schema that cause it to serialize / deserialize
differently, please update this version number *)
let full_fidelity_schema_version_number = "2018-07-19-0001"
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
    ; aggregates  = [ Expression; Name ]
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
    token_text : string;
    hack_only : bool;
    is_xhp : bool;
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

module Language_flags = struct
  let php_and_hack = "php_and_hack"
  let hack_only = "hack_only"

  let is_hack_only : string -> bool = function
    | "php_and_hack" -> false
    | "hack_only" -> true
    | f -> failwith ("Unknown language flag " ^ f ^ " for token.")
end
module LF = Language_flags

module Optional_flags = struct
  let xhp = "xhp"

  let is_recognized : string -> bool = function
    | "xhp" -> true
    | _ -> false

  let is_xhp : string list -> bool = fun flags ->
    List.mem xhp flags
end
module OF = Optional_flags

let token_node_from_list l =
  match l with
  | token_kind :: token_text :: language :: optional_flags ->
    assert (List.for_all OF.is_recognized optional_flags);
    { token_kind;
      token_text;
      hack_only = LF.is_hack_only language;
      is_xhp = OF.is_xhp optional_flags;
    }
  | _ -> failwith "bad token schema"

let trivia_node_from_list l =
  match l with
  | [ trivia_kind; trivia_text ] ->
    { trivia_kind; trivia_text }
  | _ -> failwith "bad trivia schema"

let variable_text_tokens = List.map token_node_from_list [
  [ "ErrorToken"; "error_token"; LF.php_and_hack ];
  [ "Name"; "name"; LF.php_and_hack ];
  [ "Variable"; "variable"; LF.php_and_hack ];
  [ "DecimalLiteral"; "decimal_literal"; LF.php_and_hack ];
  [ "OctalLiteral"; "octal_literal"; LF.php_and_hack ];
  [ "HexadecimalLiteral"; "hexadecimal_literal"; LF.php_and_hack ];
  [ "BinaryLiteral"; "binary_literal"; LF.php_and_hack ];
  [ "FloatingLiteral"; "floating_literal"; LF.php_and_hack ];
  [ "ExecutionStringLiteral"; "execution_string_literal"; LF.php_and_hack ];
  [ "ExecutionStringLiteralHead"; "execution_string_literal_head"; LF.php_and_hack ];
  [ "ExecutionStringLiteralTail"; "execution_string_literal_tail"; LF.php_and_hack ];
  [ "SingleQuotedStringLiteral"; "single_quoted_string_literal"; LF.php_and_hack ];
  [ "DoubleQuotedStringLiteral"; "double_quoted_string_literal"; LF.php_and_hack ];
  [ "DoubleQuotedStringLiteralHead"; "double_quoted_string_literal_head"; LF.php_and_hack ];
  [ "StringLiteralBody"; "string_literal_body"; LF.php_and_hack ];
  [ "DoubleQuotedStringLiteralTail"; "double_quoted_string_literal_tail"; LF.php_and_hack ];
  [ "HeredocStringLiteral"; "heredoc_string_literal"; LF.php_and_hack ];
  [ "HeredocStringLiteralHead"; "heredoc_string_literal_head"; LF.php_and_hack ];
  [ "HeredocStringLiteralTail"; "heredoc_string_literal_tail"; LF.php_and_hack ];
  [ "NowdocStringLiteral"; "nowdoc_string_literal"; LF.php_and_hack ];
  [ "BooleanLiteral"; "boolean_literal"; LF.php_and_hack ];
  [ "XHPCategoryName"; "XHP_category_name"; LF.php_and_hack ];
  [ "XHPElementName"; "XHP_element_name"; LF.php_and_hack ];
  [ "XHPClassName"; "XHP_class_name"; LF.php_and_hack ];
  [ "XHPStringLiteral"; "XHP_string_literal"; LF.php_and_hack ];
  [ "XHPBody"; "XHP_body"; LF.php_and_hack ];
  [ "XHPComment"; "XHP_comment"; LF.php_and_hack ];
  [ "Markup"; "markup"; LF.php_and_hack ]]

let no_text_tokens = List.map token_node_from_list [
  [ "EndOfFile"; "end_of_file"; LF.php_and_hack ]]

let given_text_tokens = List.map token_node_from_list [
  [ "Abstract"; "abstract"; LF.php_and_hack ];
  [ "And"; "and"; LF.php_and_hack ];
  [ "Array"; "array"; LF.php_and_hack ];
  [ "Arraykey"; "arraykey"; LF.hack_only ];
  [ "As"; "as"; LF.php_and_hack ];
  [ "Async"; "async"; LF.hack_only ];
  [ "Attribute"; "attribute"; LF.hack_only; OF.xhp ];
  [ "Await"; "await"; LF.hack_only ];
  [ "Backslash"; "\\"; LF.php_and_hack ];
  [ "Binary"; "binary"; LF.php_and_hack ];
  [ "Bool"; "bool"; LF.php_and_hack ];
  [ "Boolean"; "boolean"; LF.php_and_hack ];
  [ "Break"; "break"; LF.php_and_hack ];
  [ "Case"; "case"; LF.php_and_hack ];
  [ "Catch"; "catch"; LF.php_and_hack ];
  [ "Category"; "category"; LF.hack_only; OF.xhp ];
  [ "Children"; "children"; LF.hack_only; OF.xhp ];
  [ "Class"; "class"; LF.php_and_hack ];
  [ "Classname"; "classname"; LF.hack_only ];
  [ "Clone"; "clone"; LF.php_and_hack ];
  [ "Const"; "const"; LF.php_and_hack ];
  [ "Construct"; "__construct"; LF.php_and_hack ];
  [ "Continue"; "continue"; LF.php_and_hack ];
  [ "Coroutine"; "coroutine"; LF.hack_only ];
  [ "Darray"; "darray"; LF.hack_only ];
  [ "Declare"; "declare"; LF.php_and_hack ];
  [ "Default"; "default"; LF.php_and_hack ];
  [ "Define"; "define"; LF.php_and_hack ];
  [ "Destruct"; "__destruct"; LF.php_and_hack ];
  [ "Dict"; "dict"; LF.php_and_hack ];
  [ "Do"; "do"; LF.php_and_hack ];
  [ "Double"; "double"; LF.php_and_hack ];
  [ "Echo"; "echo"; LF.php_and_hack ];
  [ "Else"; "else"; LF.php_and_hack ];
  [ "Elseif"; "elseif"; LF.php_and_hack ];
  [ "Empty"; "empty"; LF.php_and_hack ];
  [ "Endfor"; "endfor"; LF.php_and_hack ];
  [ "Endforeach"; "endforeach"; LF.php_and_hack ];
  [ "Enddeclare"; "enddeclare"; LF.php_and_hack ];
  [ "Endif"; "endif"; LF.php_and_hack ];
  [ "Endswitch" ; "endswitch"; LF.php_and_hack ];
  [ "Endwhile"; "endwhile"; LF.php_and_hack ];
  [ "Enum"; "enum"; LF.hack_only; OF.xhp ];
  [ "Eval"; "eval"; LF.php_and_hack ];
  [ "Extends"; "extends"; LF.php_and_hack ];
  [ "Fallthrough"; "fallthrough"; LF.hack_only ];
  [ "Float"; "float"; LF.php_and_hack ];
  [ "Final"; "final"; LF.php_and_hack ];
  [ "Finally"; "finally"; LF.php_and_hack ];
  [ "For"; "for"; LF.php_and_hack ];
  [ "Foreach"; "foreach"; LF.php_and_hack ];
  [ "From"; "from"; LF.php_and_hack ];
  [ "Function"; "function"; LF.php_and_hack ];
  [ "Global"; "global"; LF.php_and_hack ];
  [ "Goto"; "goto"; LF.php_and_hack ];
  [ "If"; "if"; LF.php_and_hack ];
  [ "Implements"; "implements"; LF.php_and_hack ];
  [ "Include"; "include"; LF.php_and_hack ];
  [ "Include_once"; "include_once"; LF.php_and_hack ];
  [ "Inout"; "inout"; LF.hack_only ];
  [ "Instanceof"; "instanceof"; LF.php_and_hack ];
  [ "Insteadof"; "insteadof"; LF.php_and_hack ];
  [ "Int"; "int"; LF.php_and_hack ];
  [ "Integer"; "integer"; LF.php_and_hack ];
  [ "Interface"; "interface"; LF.php_and_hack ];
  [ "Is"; "is"; LF.hack_only ];
  [ "Isset"; "isset"; LF.php_and_hack ];
  [ "Keyset"; "keyset"; LF.php_and_hack ];
  [ "Let"; "let"; LF.hack_only ];
  [ "List"; "list"; LF.php_and_hack ];
  [ "Mixed"; "mixed"; LF.hack_only ];
  [ "Namespace"; "namespace"; LF.php_and_hack ];
  [ "New"; "new"; LF.php_and_hack ];
  [ "Newtype"; "newtype"; LF.hack_only ];
  [ "Noreturn"; "noreturn"; LF.hack_only ];
  [ "Num"; "num"; LF.hack_only ];
  [ "Object"; "object"; LF.php_and_hack ];
  [ "Or"; "or"; LF.php_and_hack ];
  [ "Parent"; "parent"; LF.php_and_hack ];
  [ "Print"; "print"; LF.php_and_hack ];
  [ "Private"; "private"; LF.php_and_hack ];
  [ "Protected"; "protected"; LF.php_and_hack ];
  [ "Public"; "public"; LF.php_and_hack ];
  [ "Real"; "real"; LF.php_and_hack ];
  [ "Reified"; "reified"; LF.hack_only ];
  [ "Require"; "require"; LF.php_and_hack ];
  [ "Require_once"; "require_once"; LF.php_and_hack ];
  [ "Required"; "required"; LF.hack_only; OF.xhp ];
  [ "Resource"; "resource"; LF.php_and_hack ];
  [ "Return"; "return"; LF.php_and_hack ];
  [ "Self"; "self"; LF.php_and_hack ];
  [ "Shape"; "shape"; LF.hack_only ];
  [ "Static"; "static"; LF.php_and_hack ];
  [ "String"; "string"; LF.php_and_hack ];
  [ "Super"; "super"; LF.php_and_hack ];
  [ "Suspend"; "suspend"; LF.hack_only ];
  [ "Switch"; "switch"; LF.php_and_hack ];
  [ "This"; "this"; LF.hack_only ];
  [ "Throw"; "throw"; LF.php_and_hack ];
  [ "Trait"; "trait"; LF.php_and_hack ];
  [ "Try"; "try"; LF.php_and_hack ];
  [ "Tuple"; "tuple"; LF.hack_only ];
  [ "Type"; "type"; LF.hack_only ];
  [ "Unset"; "unset"; LF.php_and_hack ];
  [ "Use"; "use"; LF.php_and_hack ];
  [ "Using"; "using"; LF.hack_only ];
  [ "Var"; "var"; LF.php_and_hack ];
  [ "Varray"; "varray"; LF.hack_only ];
  [ "Vec"; "vec"; LF.php_and_hack ];
  [ "Void"; "void"; LF.php_and_hack ];
  [ "Where"; "where"; LF.hack_only ];
  [ "While"; "while"; LF.php_and_hack ];
  [ "Xor"; "xor"; LF.php_and_hack ];
  [ "Yield"; "yield"; LF.php_and_hack ];
  [ "LeftBracket"; "["; LF.php_and_hack ];
  [ "RightBracket"; "]"; LF.php_and_hack ];
  [ "LeftParen"; "("; LF.php_and_hack ];
  [ "RightParen"; ")"; LF.php_and_hack ];
  [ "LeftBrace"; "{"; LF.php_and_hack ];
  [ "RightBrace"; "}"; LF.php_and_hack ];
  [ "Dot"; "."; LF.php_and_hack ];
  [ "MinusGreaterThan"; "->"; LF.php_and_hack ];
  [ "PlusPlus"; "++"; LF.php_and_hack ];
  [ "MinusMinus"; "--"; LF.php_and_hack ];
  [ "StarStar"; "**"; LF.php_and_hack ];
  [ "Star"; "*"; LF.php_and_hack ];
  [ "Plus"; "+"; LF.php_and_hack ];
  [ "Minus"; "-"; LF.php_and_hack ];
  [ "Tilde"; "~"; LF.php_and_hack ];
  [ "Exclamation"; "!"; LF.php_and_hack ];
  [ "Dollar"; "$"; LF.php_and_hack ];
  [ "Slash"; "/"; LF.php_and_hack ];
  [ "Percent"; "%"; LF.php_and_hack ];
  [ "LessThanGreaterThan"; "<>"; LF.php_and_hack ];
  [ "LessThanEqualGreaterThan"; "<=>"; LF.php_and_hack ];
  [ "LessThanLessThan"; "<<"; LF.php_and_hack ];
  [ "GreaterThanGreaterThan"; ">>"; LF.php_and_hack ];
  [ "LessThan"; "<"; LF.php_and_hack ];
  [ "GreaterThan"; ">"; LF.php_and_hack ];
  [ "LessThanEqual"; "<="; LF.php_and_hack ];
  [ "GreaterThanEqual"; ">="; LF.php_and_hack ];
  [ "EqualEqual"; "=="; LF.php_and_hack ];
  [ "EqualEqualEqual"; "==="; LF.php_and_hack ];
  [ "ExclamationEqual"; "!="; LF.php_and_hack ];
  [ "ExclamationEqualEqual"; "!=="; LF.php_and_hack ];
  [ "Carat"; "^"; LF.php_and_hack ];
  [ "Bar"; "|"; LF.php_and_hack ];
  [ "Ampersand"; "&"; LF.php_and_hack ];
  [ "AmpersandAmpersand"; "&&"; LF.php_and_hack ];
  [ "BarBar"; "||"; LF.php_and_hack ];
  [ "Question"; "?"; LF.php_and_hack ];
  [ "QuestionAs"; "?as"; LF.php_and_hack ];
  [ "QuestionColon"; "?:"; LF.php_and_hack ];
  [ "QuestionQuestion"; "??"; LF.php_and_hack ];
  [ "QuestionQuestionEqual"; "??="; LF.php_and_hack ];
  [ "Colon"; ":"; LF.php_and_hack ];
  [ "Semicolon"; ";"; LF.php_and_hack ];
  [ "Equal"; "="; LF.php_and_hack ];
  [ "StarStarEqual"; "**="; LF.php_and_hack ];
  [ "StarEqual"; "*="; LF.php_and_hack ];
  [ "SlashEqual"; "/="; LF.php_and_hack ];
  [ "PercentEqual"; "%="; LF.php_and_hack ];
  [ "PlusEqual"; "+="; LF.php_and_hack ];
  [ "MinusEqual"; "-="; LF.php_and_hack ];
  [ "DotEqual"; ".="; LF.php_and_hack ];
  [ "LessThanLessThanEqual"; "<<="; LF.php_and_hack ];
  [ "GreaterThanGreaterThanEqual"; ">>="; LF.php_and_hack ];
  [ "AmpersandEqual"; "&="; LF.php_and_hack ];
  [ "CaratEqual"; "^="; LF.php_and_hack ];
  [ "BarEqual"; "|="; LF.php_and_hack ];
  [ "Comma"; ","; LF.php_and_hack ];
  [ "At"; "@"; LF.php_and_hack ];
  [ "ColonColon"; "::"; LF.php_and_hack ];
  [ "EqualGreaterThan"; "=>"; LF.php_and_hack ];
  [ "EqualEqualGreaterThan"; "==>"; LF.php_and_hack ];
  [ "QuestionMinusGreaterThan"; "?->"; LF.php_and_hack ];
  [ "DotDotDot"; "..."; LF.php_and_hack ];
  [ "DollarDollar"; "$$"; LF.php_and_hack ];
  [ "BarGreaterThan"; "|>"; LF.php_and_hack ];
  [ "NullLiteral"; "null"; LF.php_and_hack ];
  [ "SlashGreaterThan"; "/>"; LF.php_and_hack ];
  [ "LessThanSlash"; "</"; LF.php_and_hack ];
  [ "LessThanQuestion";"<?"; LF.php_and_hack ];
  [ "QuestionGreaterThan"; "?>"; LF.php_and_hack ];
  [ "HaltCompiler"; "__halt_compiler"; LF.php_and_hack ]]

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
  [ "ExtraTokenError"; "extra_token_error"];
  [ "AfterHaltCompiler"; "after_halt_compiler" ]]

let escape_token_text t =
  (* add one extra backslash because
     it is removed by Str.replace_first downstream *)
  if t = "\\" then "\\\\\\" else t

let map_and_concat_separated separator f items =
  String.concat separator (List.map f items)

let map_and_concat f items =
  map_and_concat_separated "" f items

let transform_schema f =
  map_and_concat f schema

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
    x.token_kind (escape_token_text x.token_text)

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
  \"@"^"generated JSON schema of the Hack Full Fidelity Parser AST\",
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
