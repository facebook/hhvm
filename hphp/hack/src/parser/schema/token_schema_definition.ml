(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type token_node = {
  token_kind: string;
  token_text: string;
  (* Whether the token is allowed as identifier, i.e., in practice, whether
   * it is allowed as function name or class name.
   * For example, darray is allowed as identifier. The following is legit:
   *
   *   function darray() {}
   *
   * NB 1: This does not apply to names of constants, enum members, class
   * members including methods: for those, absolutely all keywords are
   * allowed.
   *
   * NB 2: for class names, in addition, a few other keywords are "reserved",
   * i.e. not allowed as class names. These include type names like int or
   * bool. Those "reserved" keywords are not defined here. See syntax error
   * `reserved_keyword_as_type_name`.
   *)
  allowed_as_identifier: bool;
}

let make_token_node token_kind token_text ?(allowed_as_identifier = false) () =
  { token_kind; token_text; allowed_as_identifier }

let variable_text_tokens =
  [
    make_token_node "ErrorToken" "error_token" ();
    make_token_node "Name" "name" ();
    make_token_node "Variable" "variable" ();
    make_token_node "DecimalLiteral" "decimal_literal" ();
    make_token_node "OctalLiteral" "octal_literal" ();
    make_token_node "HexadecimalLiteral" "hexadecimal_literal" ();
    make_token_node "BinaryLiteral" "binary_literal" ();
    make_token_node "FloatingLiteral" "floating_literal" ();
    make_token_node
      "SingleQuotedStringLiteral"
      "single_quoted_string_literal"
      ();
    make_token_node
      "DoubleQuotedStringLiteral"
      "double_quoted_string_literal"
      ();
    make_token_node
      "DoubleQuotedStringLiteralHead"
      "double_quoted_string_literal_head"
      ();
    make_token_node "StringLiteralBody" "string_literal_body" ();
    make_token_node
      "DoubleQuotedStringLiteralTail"
      "double_quoted_string_literal_tail"
      ();
    make_token_node "HeredocStringLiteral" "heredoc_string_literal" ();
    make_token_node "HeredocStringLiteralHead" "heredoc_string_literal_head" ();
    make_token_node "HeredocStringLiteralTail" "heredoc_string_literal_tail" ();
    make_token_node "NowdocStringLiteral" "nowdoc_string_literal" ();
    make_token_node "BooleanLiteral" "boolean_literal" ();
    make_token_node "XHPCategoryName" "XHP_category_name" ();
    make_token_node "XHPElementName" "XHP_element_name" ();
    make_token_node "XHPClassName" "XHP_class_name" ();
    make_token_node "XHPStringLiteral" "XHP_string_literal" ();
    make_token_node "XHPBody" "XHP_body" ();
    make_token_node "XHPComment" "XHP_comment" ();
    make_token_node "Hashbang" "hashbang" ();
  ]

let no_text_tokens = [make_token_node "EndOfFile" "end_of_file" ()]

let given_text_tokens =
  [
    make_token_node "Abstract" "abstract" ();
    make_token_node "Arraykey" "arraykey" ~allowed_as_identifier:true ();
    make_token_node "As" "as" ();
    make_token_node "Async" "async" ();
    make_token_node "Attribute" "attribute" ~allowed_as_identifier:true ();
    make_token_node "Await" "await" ();
    make_token_node "Backslash" "\\" ();
    make_token_node "Binary" "binary" ~allowed_as_identifier:true ();
    make_token_node "Bool" "bool" ~allowed_as_identifier:true ();
    make_token_node "Boolean" "boolean" ~allowed_as_identifier:true ();
    make_token_node "Break" "break" ();
    make_token_node "Case" "case" ();
    make_token_node "Catch" "catch" ();
    make_token_node "Category" "category" ~allowed_as_identifier:true ();
    make_token_node "Children" "children" ~allowed_as_identifier:true ();
    make_token_node "Class" "class" ();
    make_token_node "Classname" "classname" ~allowed_as_identifier:true ();
    make_token_node "Clone" "clone" ();
    make_token_node "Concurrent" "concurrent" ();
    make_token_node "Const" "const" ();
    make_token_node "Construct" "__construct" ();
    make_token_node "Continue" "continue" ();
    make_token_node "Ctx" "ctx" ();
    make_token_node "Darray" "darray" ~allowed_as_identifier:true ();
    make_token_node "Default" "default" ();
    make_token_node "Dict" "dict" ~allowed_as_identifier:true ();
    make_token_node "Do" "do" ();
    make_token_node "Double" "double" ~allowed_as_identifier:true ();
    make_token_node "Echo" "echo" ();
    make_token_node "Else" "else" ();
    make_token_node "Empty" "empty" ();
    make_token_node "Endif" "endif" ();
    make_token_node "Enum" "enum" ~allowed_as_identifier:true ();
    make_token_node "Eval" "eval" ();
    make_token_node "Exports" "exports" ~allowed_as_identifier:true ();
    make_token_node "Extends" "extends" ();
    make_token_node "Fallthrough" "fallthrough" ~allowed_as_identifier:true ();
    make_token_node "Float" "float" ~allowed_as_identifier:true ();
    make_token_node "File" "file" ~allowed_as_identifier:true ();
    make_token_node "Final" "final" ();
    make_token_node "Finally" "finally" ();
    make_token_node "For" "for" ();
    make_token_node "Foreach" "foreach" ();
    make_token_node "Function" "function" ();
    make_token_node "Global" "global" ();
    make_token_node "If" "if" ();
    make_token_node "Implements" "implements" ();
    make_token_node "Imports" "imports" ~allowed_as_identifier:true ();
    make_token_node "Include" "include" ();
    make_token_node "Include_once" "include_once" ();
    make_token_node "Inout" "inout" ();
    make_token_node "Instanceof" "instanceof" ();
    make_token_node "Insteadof" "insteadof" ();
    make_token_node "Int" "int" ~allowed_as_identifier:true ();
    make_token_node "Integer" "integer" ~allowed_as_identifier:true ();
    make_token_node "Interface" "interface" ();
    make_token_node "Is" "is" ~allowed_as_identifier:true ();
    make_token_node "Isset" "isset" ();
    make_token_node "Keyset" "keyset" ~allowed_as_identifier:true ();
    make_token_node "Lateinit" "lateinit" ();
    make_token_node "List" "list" ();
    make_token_node "Match" "match" ~allowed_as_identifier:true ();
    make_token_node "Mixed" "mixed" ~allowed_as_identifier:true ();
    make_token_node "Module" "module" ();
    make_token_node "Nameof" "nameof" ();
    make_token_node "Namespace" "namespace" ();
    make_token_node "New" "new" ();
    make_token_node "Newctx" "newctx" ~allowed_as_identifier:true ();
    make_token_node "Newtype" "newtype" ~allowed_as_identifier:true ();
    make_token_node "Noreturn" "noreturn" ~allowed_as_identifier:true ();
    make_token_node "Num" "num" ~allowed_as_identifier:true ();
    make_token_node "Parent" "parent" ~allowed_as_identifier:true ();
    make_token_node "Print" "print" ();
    make_token_node "Private" "private" ();
    make_token_node "Protected" "protected" ();
    make_token_node "Public" "public" ();
    make_token_node "Real" "real" ~allowed_as_identifier:true ();
    make_token_node "Reify" "reify" ~allowed_as_identifier:true ();
    make_token_node "Require" "require" ();
    make_token_node "Require_once" "require_once" ();
    make_token_node "Required" "required" ();
    make_token_node "Resource" "resource" ~allowed_as_identifier:true ();
    make_token_node "Return" "return" ();
    make_token_node "Self" "self" ~allowed_as_identifier:true ();
    make_token_node "Shape" "shape" ();
    make_token_node "Static" "static" ();
    make_token_node "String" "string" ~allowed_as_identifier:true ();
    make_token_node "Super" "super" ~allowed_as_identifier:true ();
    make_token_node "Switch" "switch" ();
    make_token_node "This" "this" ~allowed_as_identifier:true ();
    make_token_node "Throw" "throw" ();
    make_token_node "Trait" "trait" ();
    make_token_node "Try" "try" ();
    make_token_node "Tuple" "tuple" ();
    make_token_node "Type" "type" ~allowed_as_identifier:true ();
    make_token_node "Unset" "unset" ();
    make_token_node "Upcast" "upcast" ~allowed_as_identifier:true ();
    make_token_node "Use" "use" ();
    make_token_node "Using" "using" ();
    make_token_node "Var" "var" ();
    make_token_node "Varray" "varray" ~allowed_as_identifier:true ();
    make_token_node "Vec" "vec" ~allowed_as_identifier:true ();
    make_token_node "Void" "void" ~allowed_as_identifier:true ();
    make_token_node "With" "with" ~allowed_as_identifier:true ();
    make_token_node "Where" "where" ~allowed_as_identifier:true ();
    make_token_node "While" "while" ();
    make_token_node "Yield" "yield" ();
    make_token_node "NullLiteral" "null" ~allowed_as_identifier:true ();
    make_token_node "LeftBracket" "[" ();
    make_token_node "RightBracket" "]" ();
    make_token_node "LeftParen" "(" ();
    make_token_node "RightParen" ")" ();
    make_token_node "LeftBrace" "{" ();
    make_token_node "RightBrace" "}" ();
    make_token_node "Dot" "." ();
    make_token_node "MinusGreaterThan" "->" ();
    make_token_node "PlusPlus" "++" ();
    make_token_node "MinusMinus" "--" ();
    make_token_node "StarStar" "**" ();
    make_token_node "Star" "*" ();
    make_token_node "Plus" "+" ();
    make_token_node "Minus" "-" ();
    make_token_node "Tilde" "~" ();
    make_token_node "Exclamation" "!" ();
    make_token_node "Dollar" "$" ();
    make_token_node "Slash" "/" ();
    make_token_node "Percent" "%" ();
    make_token_node "LessThanEqualGreaterThan" "<=>" ();
    make_token_node "LessThanLessThan" "<<" ();
    make_token_node "GreaterThanGreaterThan" ">>" ();
    make_token_node "LessThan" "<" ();
    make_token_node "GreaterThan" ">" ();
    make_token_node "LessThanEqual" "<=" ();
    make_token_node "GreaterThanEqual" ">=" ();
    make_token_node "EqualEqual" "==" ();
    make_token_node "EqualEqualEqual" "===" ();
    make_token_node "ExclamationEqual" "!=" ();
    make_token_node "ExclamationEqualEqual" "!==" ();
    make_token_node "Carat" "^" ();
    make_token_node "Bar" "|" ();
    make_token_node "Ampersand" "&" ();
    make_token_node "AmpersandAmpersand" "&&" ();
    make_token_node "BarBar" "||" ();
    make_token_node "Question" "?" ();
    make_token_node "QuestionAs" "?as" ();
    make_token_node "QuestionColon" "?:" ();
    make_token_node "QuestionQuestion" "??" ();
    make_token_node "QuestionQuestionEqual" "??=" ();
    make_token_node "Colon" ":" ();
    make_token_node "Semicolon" ";" ();
    make_token_node "Equal" "=" ();
    make_token_node "StarStarEqual" "**=" ();
    make_token_node "StarEqual" "*=" ();
    make_token_node "SlashEqual" "/=" ();
    make_token_node "PercentEqual" "%=" ();
    make_token_node "PlusEqual" "+=" ();
    make_token_node "MinusEqual" "-=" ();
    make_token_node "DotEqual" ".=" ();
    make_token_node "LessThanLessThanEqual" "<<=" ();
    make_token_node "GreaterThanGreaterThanEqual" ">>=" ();
    make_token_node "AmpersandEqual" "&=" ();
    make_token_node "CaratEqual" "^=" ();
    make_token_node "BarEqual" "|=" ();
    make_token_node "Comma" "," ();
    make_token_node "At" "@" ();
    make_token_node "ColonColon" "::" ();
    make_token_node "EqualGreaterThan" "=>" ();
    make_token_node "EqualEqualGreaterThan" "==>" ();
    make_token_node "QuestionMinusGreaterThan" "?->" ();
    make_token_node "DotDotDot" "..." ();
    make_token_node "DollarDollar" "$$" ();
    make_token_node "BarGreaterThan" "|>" ();
    make_token_node "SlashGreaterThan" "/>" ();
    make_token_node "LessThanSlash" "</" ();
    make_token_node "LessThanQuestion" "<?" ();
    make_token_node "Backtick" "`" ();
    make_token_node "XHP" "xhp" ~allowed_as_identifier:true ();
    make_token_node "Hash" "#" ();
    make_token_node "Readonly" "readonly" ();
    make_token_node "Internal" "internal" ~allowed_as_identifier:true ();
    make_token_node "Package" "package" ();
    make_token_node "Let" "let" ~allowed_as_identifier:true ();
  ]

let tokens = variable_text_tokens @ no_text_tokens @ given_text_tokens
