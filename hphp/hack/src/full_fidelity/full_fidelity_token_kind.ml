(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * grant of patent rights can be found in the PATENTS file in the same
 * directory.
 *
 *)
(* THIS FILE IS GENERATED; DO NOT EDIT IT *)
(* @generated *)
(**
  To regenerate this file build hphp/hack/src:generate_full_fidelity and run
  the binary.
  buck build hphp/hack/src:generate_full_fidelity
  buck-out/bin/hphp/hack/src/generate_full_fidelity/generate_full_fidelity.opt
*)

type t =
  | EndOfFile

  | Abstract
  | And
  | Array
  | Arraykey
  | As
  | Async
  | Attribute
  | Await
  | Bool
  | Break
  | Case
  | Catch
  | Category
  | Children
  | Class
  | Classname
  | Clone
  | Const
  | Construct
  | Continue
  | Default
  | Define
  | Destruct
  | Dict
  | Do
  | Double
  | Echo
  | Else
  | Elseif
  | Empty
  | Enum
  | Eval
  | Extends
  | Float
  | Final
  | Finally
  | For
  | Foreach
  | Function
  | Global
  | If
  | Implements
  | Include
  | Include_once
  | Instanceof
  | Insteadof
  | Int
  | Interface
  | Isset
  | Keyset
  | List
  | Mixed
  | Namespace
  | New
  | Newtype
  | Noreturn
  | Num
  | Object
  | Or
  | Parent
  | Print
  | Private
  | Protected
  | Public
  | Require
  | Require_once
  | Required
  | Resource
  | Return
  | Self
  | Shape
  | Static
  | String
  | Super
  | Switch
  | This
  | Throw
  | Trait
  | Try
  | Tuple
  | Type
  | Unset
  | Use
  | Var
  | Vec
  | Void
  | While
  | Xor
  | Yield
  | LeftBracket
  | RightBracket
  | LeftParen
  | RightParen
  | LeftBrace
  | RightBrace
  | Dot
  | QuestionGreaterThan
  | MinusGreaterThan
  | PlusPlus
  | MinusMinus
  | StarStar
  | Star
  | Plus
  | Minus
  | Tilde
  | Exclamation
  | Dollar
  | Slash
  | Percent
  | LessThanEqualGreaterThan
  | LessThanLessThan
  | GreaterThanGreaterThan
  | LessThan
  | GreaterThan
  | LessThanEqual
  | GreaterThanEqual
  | EqualEqual
  | EqualEqualEqual
  | ExclamationEqual
  | ExclamationEqualEqual
  | Carat
  | Bar
  | Ampersand
  | AmpersandAmpersand
  | BarBar
  | Question
  | QuestionQuestion
  | Colon
  | Semicolon
  | Equal
  | StarStarEqual
  | StarEqual
  | SlashEqual
  | PercentEqual
  | PlusEqual
  | MinusEqual
  | DotEqual
  | LessThanLessThanEqual
  | GreaterThanGreaterThanEqual
  | AmpersandEqual
  | CaratEqual
  | BarEqual
  | Comma
  | At
  | ColonColon
  | EqualGreaterThan
  | EqualEqualGreaterThan
  | QuestionMinusGreaterThan
  | DotDotDot
  | DollarDollar
  | BarGreaterThan
  | NullLiteral
  | SlashGreaterThan
  | LessThanSlash

  | ErrorToken
  | Name
  | QualifiedName
  | Variable
  | NamespacePrefix
  | DecimalLiteral
  | OctalLiteral
  | HexadecimalLiteral
  | BinaryLiteral
  | FloatingLiteral
  | SingleQuotedStringLiteral
  | DoubleQuotedStringLiteral
  | DoubleQuotedStringLiteralHead
  | StringLiteralBody
  | DoubleQuotedStringLiteralTail
  | HeredocStringLiteral
  | HeredocStringLiteralHead
  | HeredocStringLiteralTail
  | NowdocStringLiteral
  | BooleanLiteral
  | XHPCategoryName
  | XHPElementName
  | XHPClassName
  | XHPStringLiteral
  | XHPBody
  | XHPComment


let from_string keyword =
  match keyword with
  | "true" -> Some BooleanLiteral
  | "false" -> Some BooleanLiteral
  | "abstract" -> Some Abstract
  | "and" -> Some And
  | "array" -> Some Array
  | "arraykey" -> Some Arraykey
  | "as" -> Some As
  | "async" -> Some Async
  | "attribute" -> Some Attribute
  | "await" -> Some Await
  | "bool" -> Some Bool
  | "break" -> Some Break
  | "case" -> Some Case
  | "catch" -> Some Catch
  | "category" -> Some Category
  | "children" -> Some Children
  | "class" -> Some Class
  | "classname" -> Some Classname
  | "clone" -> Some Clone
  | "const" -> Some Const
  | "__construct" -> Some Construct
  | "continue" -> Some Continue
  | "default" -> Some Default
  | "define" -> Some Define
  | "__destruct" -> Some Destruct
  | "dict" -> Some Dict
  | "do" -> Some Do
  | "double" -> Some Double
  | "echo" -> Some Echo
  | "else" -> Some Else
  | "elseif" -> Some Elseif
  | "empty" -> Some Empty
  | "enum" -> Some Enum
  | "eval" -> Some Eval
  | "extends" -> Some Extends
  | "float" -> Some Float
  | "final" -> Some Final
  | "finally" -> Some Finally
  | "for" -> Some For
  | "foreach" -> Some Foreach
  | "function" -> Some Function
  | "global" -> Some Global
  | "if" -> Some If
  | "implements" -> Some Implements
  | "include" -> Some Include
  | "include_once" -> Some Include_once
  | "instanceof" -> Some Instanceof
  | "insteadof" -> Some Insteadof
  | "int" -> Some Int
  | "interface" -> Some Interface
  | "isset" -> Some Isset
  | "keyset" -> Some Keyset
  | "list" -> Some List
  | "mixed" -> Some Mixed
  | "namespace" -> Some Namespace
  | "new" -> Some New
  | "newtype" -> Some Newtype
  | "noreturn" -> Some Noreturn
  | "num" -> Some Num
  | "object" -> Some Object
  | "or" -> Some Or
  | "parent" -> Some Parent
  | "print" -> Some Print
  | "private" -> Some Private
  | "protected" -> Some Protected
  | "public" -> Some Public
  | "require" -> Some Require
  | "require_once" -> Some Require_once
  | "required" -> Some Required
  | "resource" -> Some Resource
  | "return" -> Some Return
  | "self" -> Some Self
  | "shape" -> Some Shape
  | "static" -> Some Static
  | "string" -> Some String
  | "super" -> Some Super
  | "switch" -> Some Switch
  | "this" -> Some This
  | "throw" -> Some Throw
  | "trait" -> Some Trait
  | "try" -> Some Try
  | "tuple" -> Some Tuple
  | "type" -> Some Type
  | "unset" -> Some Unset
  | "use" -> Some Use
  | "var" -> Some Var
  | "vec" -> Some Vec
  | "void" -> Some Void
  | "while" -> Some While
  | "xor" -> Some Xor
  | "yield" -> Some Yield
  | "[" -> Some LeftBracket
  | "]" -> Some RightBracket
  | "(" -> Some LeftParen
  | ")" -> Some RightParen
  | "{" -> Some LeftBrace
  | "}" -> Some RightBrace
  | "." -> Some Dot
  | "?>" -> Some QuestionGreaterThan
  | "->" -> Some MinusGreaterThan
  | "++" -> Some PlusPlus
  | "--" -> Some MinusMinus
  | "**" -> Some StarStar
  | "*" -> Some Star
  | "+" -> Some Plus
  | "-" -> Some Minus
  | "~" -> Some Tilde
  | "!" -> Some Exclamation
  | "$" -> Some Dollar
  | "/" -> Some Slash
  | "%" -> Some Percent
  | "<=>" -> Some LessThanEqualGreaterThan
  | "<<" -> Some LessThanLessThan
  | ">>" -> Some GreaterThanGreaterThan
  | "<" -> Some LessThan
  | ">" -> Some GreaterThan
  | "<=" -> Some LessThanEqual
  | ">=" -> Some GreaterThanEqual
  | "==" -> Some EqualEqual
  | "===" -> Some EqualEqualEqual
  | "!=" -> Some ExclamationEqual
  | "!==" -> Some ExclamationEqualEqual
  | "^" -> Some Carat
  | "|" -> Some Bar
  | "&" -> Some Ampersand
  | "&&" -> Some AmpersandAmpersand
  | "||" -> Some BarBar
  | "?" -> Some Question
  | "??" -> Some QuestionQuestion
  | ":" -> Some Colon
  | ";" -> Some Semicolon
  | "=" -> Some Equal
  | "**=" -> Some StarStarEqual
  | "*=" -> Some StarEqual
  | "/=" -> Some SlashEqual
  | "%=" -> Some PercentEqual
  | "+=" -> Some PlusEqual
  | "-=" -> Some MinusEqual
  | ".=" -> Some DotEqual
  | "<<=" -> Some LessThanLessThanEqual
  | ">>=" -> Some GreaterThanGreaterThanEqual
  | "&=" -> Some AmpersandEqual
  | "^=" -> Some CaratEqual
  | "|=" -> Some BarEqual
  | "," -> Some Comma
  | "@" -> Some At
  | "::" -> Some ColonColon
  | "=>" -> Some EqualGreaterThan
  | "==>" -> Some EqualEqualGreaterThan
  | "?->" -> Some QuestionMinusGreaterThan
  | "..." -> Some DotDotDot
  | "$$" -> Some DollarDollar
  | "|>" -> Some BarGreaterThan
  | "null" -> Some NullLiteral
  | "/>" -> Some SlashGreaterThan
  | "</" -> Some LessThanSlash

  | _ -> None

let to_string kind =
match kind with
| EndOfFile -> "end of file"
  | Abstract -> "abstract"
  | And -> "and"
  | Array -> "array"
  | Arraykey -> "arraykey"
  | As -> "as"
  | Async -> "async"
  | Attribute -> "attribute"
  | Await -> "await"
  | Bool -> "bool"
  | Break -> "break"
  | Case -> "case"
  | Catch -> "catch"
  | Category -> "category"
  | Children -> "children"
  | Class -> "class"
  | Classname -> "classname"
  | Clone -> "clone"
  | Const -> "const"
  | Construct -> "__construct"
  | Continue -> "continue"
  | Default -> "default"
  | Define -> "define"
  | Destruct -> "__destruct"
  | Dict -> "dict"
  | Do -> "do"
  | Double -> "double"
  | Echo -> "echo"
  | Else -> "else"
  | Elseif -> "elseif"
  | Empty -> "empty"
  | Enum -> "enum"
  | Eval -> "eval"
  | Extends -> "extends"
  | Float -> "float"
  | Final -> "final"
  | Finally -> "finally"
  | For -> "for"
  | Foreach -> "foreach"
  | Function -> "function"
  | Global -> "global"
  | If -> "if"
  | Implements -> "implements"
  | Include -> "include"
  | Include_once -> "include_once"
  | Instanceof -> "instanceof"
  | Insteadof -> "insteadof"
  | Int -> "int"
  | Interface -> "interface"
  | Isset -> "isset"
  | Keyset -> "keyset"
  | List -> "list"
  | Mixed -> "mixed"
  | Namespace -> "namespace"
  | New -> "new"
  | Newtype -> "newtype"
  | Noreturn -> "noreturn"
  | Num -> "num"
  | Object -> "object"
  | Or -> "or"
  | Parent -> "parent"
  | Print -> "print"
  | Private -> "private"
  | Protected -> "protected"
  | Public -> "public"
  | Require -> "require"
  | Require_once -> "require_once"
  | Required -> "required"
  | Resource -> "resource"
  | Return -> "return"
  | Self -> "self"
  | Shape -> "shape"
  | Static -> "static"
  | String -> "string"
  | Super -> "super"
  | Switch -> "switch"
  | This -> "this"
  | Throw -> "throw"
  | Trait -> "trait"
  | Try -> "try"
  | Tuple -> "tuple"
  | Type -> "type"
  | Unset -> "unset"
  | Use -> "use"
  | Var -> "var"
  | Vec -> "vec"
  | Void -> "void"
  | While -> "while"
  | Xor -> "xor"
  | Yield -> "yield"
  | LeftBracket -> "["
  | RightBracket -> "]"
  | LeftParen -> "("
  | RightParen -> ")"
  | LeftBrace -> "{"
  | RightBrace -> "}"
  | Dot -> "."
  | QuestionGreaterThan -> "?>"
  | MinusGreaterThan -> "->"
  | PlusPlus -> "++"
  | MinusMinus -> "--"
  | StarStar -> "**"
  | Star -> "*"
  | Plus -> "+"
  | Minus -> "-"
  | Tilde -> "~"
  | Exclamation -> "!"
  | Dollar -> "$"
  | Slash -> "/"
  | Percent -> "%"
  | LessThanEqualGreaterThan -> "<=>"
  | LessThanLessThan -> "<<"
  | GreaterThanGreaterThan -> ">>"
  | LessThan -> "<"
  | GreaterThan -> ">"
  | LessThanEqual -> "<="
  | GreaterThanEqual -> ">="
  | EqualEqual -> "=="
  | EqualEqualEqual -> "==="
  | ExclamationEqual -> "!="
  | ExclamationEqualEqual -> "!=="
  | Carat -> "^"
  | Bar -> "|"
  | Ampersand -> "&"
  | AmpersandAmpersand -> "&&"
  | BarBar -> "||"
  | Question -> "?"
  | QuestionQuestion -> "??"
  | Colon -> ":"
  | Semicolon -> ";"
  | Equal -> "="
  | StarStarEqual -> "**="
  | StarEqual -> "*="
  | SlashEqual -> "/="
  | PercentEqual -> "%="
  | PlusEqual -> "+="
  | MinusEqual -> "-="
  | DotEqual -> ".="
  | LessThanLessThanEqual -> "<<="
  | GreaterThanGreaterThanEqual -> ">>="
  | AmpersandEqual -> "&="
  | CaratEqual -> "^="
  | BarEqual -> "|="
  | Comma -> ","
  | At -> "@"
  | ColonColon -> "::"
  | EqualGreaterThan -> "=>"
  | EqualEqualGreaterThan -> "==>"
  | QuestionMinusGreaterThan -> "?->"
  | DotDotDot -> "..."
  | DollarDollar -> "$$"
  | BarGreaterThan -> "|>"
  | NullLiteral -> "null"
  | SlashGreaterThan -> "/>"
  | LessThanSlash -> "</"

  | ErrorToken -> "error_token"
  | Name -> "name"
  | QualifiedName -> "qualified_name"
  | Variable -> "variable"
  | NamespacePrefix -> "namespace_prefix"
  | DecimalLiteral -> "decimal_literal"
  | OctalLiteral -> "octal_literal"
  | HexadecimalLiteral -> "hexadecimal_literal"
  | BinaryLiteral -> "binary_literal"
  | FloatingLiteral -> "floating_literal"
  | SingleQuotedStringLiteral -> "single_quoted_string_literal"
  | DoubleQuotedStringLiteral -> "double_quoted_string_literal"
  | DoubleQuotedStringLiteralHead -> "double_quoted_string_literal_head"
  | StringLiteralBody -> "string_literal_body"
  | DoubleQuotedStringLiteralTail -> "double_quoted_string_literal_tail"
  | HeredocStringLiteral -> "heredoc_string_literal"
  | HeredocStringLiteralHead -> "heredoc_string_literal_head"
  | HeredocStringLiteralTail -> "heredoc_string_literal_tail"
  | NowdocStringLiteral -> "nowdoc_string_literal"
  | BooleanLiteral -> "boolean_literal"
  | XHPCategoryName -> "XHP_category_name"
  | XHPElementName -> "XHP_element_name"
  | XHPClassName -> "XHP_class_name"
  | XHPStringLiteral -> "XHP_string_literal"
  | XHPBody -> "XHP_body"
  | XHPComment -> "XHP_comment"

