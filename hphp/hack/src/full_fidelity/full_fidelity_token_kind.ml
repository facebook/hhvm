(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t =
| Error
| EndOfFile
| Name
| QualifiedName
| Variable
(* Keywords *)
| Abstract
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
| Destruct
| Do
| Double
| Echo
| Else
| Elseif
| Empty
| Enum
| Extends
| Float
| Final
| Finally
| For
| Foreach
| Function
| If
| Implements
| Include
| Include_once
| Instanceof
| Insteadof
| Int
| Interface
| List
| Mixed
| Namespace
| New
| Newtype
| Noreturn
| Num
| Object
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
| Void
| While
| Yield
(* Punctuation *)
| LeftBracket
| RightBracket
| LeftParen
| RightParen
| LeftBrace
| RightBrace
| Dot
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
| NamespacePrefix
| DotDotDot
| DollarDollar
| BarGreaterThan
(* Literals *)
| DecimalLiteral
| OctalLiteral
| HexadecimalLiteral
| BinaryLiteral
| FloatingLiteral
| SingleQuotedStringLiteral
| DoubleQuotedStringLiteral
| HeredocStringLiteral
| NowdocStringLiteral
| BooleanLiteral
| NullLiteral
(* XHP *)
| XHPCategoryName
| XHPElementName
| XHPClassName
| XHPStringLiteral
| XHPBody
| XHPComment
| SlashGreaterThan
| LessThanSlash


let from_string keyword =
  match keyword with
  | "abstract" -> Some Abstract
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
  | "__destruct" -> Some Destruct
  | "do" -> Some Do
  | "double" -> Some Double
  | "echo" -> Some Echo
  | "else" -> Some Else
  | "elseif" -> Some Elseif
  | "empty" -> Some Empty
  | "enum" -> Some Enum
  | "extends" -> Some Extends
  | "false" -> Some BooleanLiteral
  | "float" -> Some Float
  | "final" -> Some Final
  | "finally" -> Some Finally
  | "for" -> Some For
  | "foreach" -> Some Foreach
  | "function" -> Some Function
  | "if" -> Some If
  | "implements" -> Some Implements
  | "include" -> Some Include
  | "include_once" -> Some Include_once
  | "instanceof" -> Some Instanceof
  | "insteadof" -> Some Insteadof
  | "int" -> Some Int
  | "interface" -> Some Interface
  | "list" -> Some List
  | "mixed" -> Some Mixed
  | "namespace" -> Some Namespace
  | "new" -> Some New
  | "newtype" -> Some Newtype
  | "noreturn" -> Some Noreturn
  | "null" -> Some NullLiteral
  | "num" -> Some Num
  | "object" -> Some Object
  | "parent" -> Some Parent
  | "print" -> Some Print
  | "private" -> Some Private
  | "protected" -> Some Protected
  | "public" -> Some Public
  | "require" -> Some Require
  | "required" -> Some Required
  | "require_once" -> Some Require_once
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
  | "true" -> Some BooleanLiteral
  | "try" -> Some Try
  | "tuple" -> Some Tuple
  | "type" -> Some Type
  | "unset" -> Some Unset
  | "use" -> Some Use
  | "var" -> Some Var
  | "void" -> Some Void
  | "while" -> Some While
  | "yield" -> Some Yield
  | _ -> None

let to_string kind =
  match kind with
  | Abstract -> "abstract"
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
  | Destruct -> "__destruct"
  | Do -> "do"
  | Double -> "double"
  | Echo -> "echo"
  | Else -> "else"
  | Elseif -> "elseif"
  | Empty -> "empty"
  | Enum -> "enum"
  | Extends -> "extends"
  | Float -> "float"
  | Final -> "final"
  | Finally -> "finally"
  | For -> "for"
  | Foreach -> "foreach"
  | Function -> "function"
  | If -> "if"
  | Implements -> "implements"
  | Include -> "include"
  | Include_once -> "include_once"
  | Instanceof -> "instanceof"
  | Insteadof -> "insteadof"
  | Int -> "int"
  | Interface -> "interface"
  | List -> "list"
  | Mixed -> "mixed"
  | Namespace -> "namespace"
  | NamespacePrefix -> "namespace_prefix"
  | New -> "new"
  | Newtype -> "newtype"
  | Noreturn -> "noreturn"
  | Num -> "num"
  | Object -> "object"
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
  | Void -> "void"
  | While -> "while"
  | Yield -> "yield"
  | LeftBracket -> "["
  | RightBracket -> "]"
  | LeftParen -> "("
  | RightParen -> ")"
  | LeftBrace -> "{"
  | RightBrace -> "}"
  | Dot -> "."
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
  | Error -> "error"
  | EndOfFile -> "end of file"
  | Name -> "name"
  | QualifiedName -> "qualified_name"
  | Variable -> "variable"
  | DecimalLiteral -> "decimal_literal"
  | OctalLiteral -> "octal_literal"
  | HexadecimalLiteral -> "hexadecimal_literal"
  | BinaryLiteral -> "binary_literal"
  | FloatingLiteral -> "floating_literal"
  | SingleQuotedStringLiteral -> "single_quoted_string_literal"
  | DoubleQuotedStringLiteral -> "double_quoted_string_literal"
  | HeredocStringLiteral -> "heredoc_string_literal"
  | NowdocStringLiteral -> "nowdoc_string_literal"
  | BooleanLiteral -> "boolean_literal"
  | NullLiteral -> "null_literal"
  | XHPCategoryName -> "XHP_category_name"
  | XHPElementName -> "XHP_element_name"
  | XHPClassName -> "XHP_class_name"
  | XHPStringLiteral -> "XHP_string_literal"
  | XHPBody -> "XHP_body"
  | XHPComment -> "XHP_comment"
  | SlashGreaterThan -> "/>"
  | LessThanSlash -> "</"
