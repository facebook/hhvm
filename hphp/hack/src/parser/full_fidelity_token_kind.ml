(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 *)

type t =
  (* No text tokens *)
  | EndOfFile
  (* Given text tokens *)
  | Abstract
  | Arraykey
  | As
  | Async
  | Attribute
  | Await
  | Backslash
  | Binary
  | Bool
  | Boolean
  | Break
  | Case
  | Catch
  | Category
  | Children
  | Class
  | Classname
  | Clone
  | Concurrent
  | Const
  | Construct
  | Continue
  | Ctx
  | Darray
  | Default
  | Dict
  | Do
  | Double
  | Echo
  | Else
  | Empty
  | Endif
  | Enum
  | Eval
  | Exports
  | Extends
  | Fallthrough
  | Float
  | File
  | Final
  | Finally
  | For
  | Foreach
  | Function
  | Global
  | If
  | Implements
  | Imports
  | Include
  | Include_once
  | Inout
  | Instanceof
  | Insteadof
  | Int
  | Integer
  | Interface
  | Is
  | Isset
  | Keyset
  | Lateinit
  | List
  | Match
  | Mixed
  | Module
  | Nameof
  | Namespace
  | New
  | Newctx
  | Newtype
  | Noreturn
  | Num
  | Parent
  | Print
  | Private
  | Protected
  | Public
  | Real
  | Reify
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
  | Upcast
  | Use
  | Using
  | Var
  | Varray
  | Vec
  | Void
  | With
  | Where
  | While
  | Yield
  | NullLiteral
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
  | QuestionAs
  | QuestionColon
  | QuestionQuestion
  | QuestionQuestionEqual
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
  | SlashGreaterThan
  | LessThanSlash
  | LessThanQuestion
  | Backtick
  | XHP
  | Hash
  | Readonly
  | Internal
  | Package
  | Let
  | Optional
  (* Variable text tokens *)
  | ErrorToken
  | Name
  | Variable
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
  | Hashbang
[@@deriving show, eq, sexp_of]

let from_string keyword ~only_reserved =
  match keyword with
  | "true" when not only_reserved -> Some BooleanLiteral
  | "false" when not only_reserved -> Some BooleanLiteral
  | "abstract" -> Some Abstract
  | "arraykey" when not only_reserved -> Some Arraykey
  | "as" -> Some As
  | "async" -> Some Async
  | "attribute" when not only_reserved -> Some Attribute
  | "await" -> Some Await
  | "\\" -> Some Backslash
  | "binary" when not only_reserved -> Some Binary
  | "bool" when not only_reserved -> Some Bool
  | "boolean" when not only_reserved -> Some Boolean
  | "break" -> Some Break
  | "case" -> Some Case
  | "catch" -> Some Catch
  | "category" when not only_reserved -> Some Category
  | "children" when not only_reserved -> Some Children
  | "class" -> Some Class
  | "classname" when not only_reserved -> Some Classname
  | "clone" -> Some Clone
  | "concurrent" -> Some Concurrent
  | "const" -> Some Const
  | "__construct" -> Some Construct
  | "continue" -> Some Continue
  | "ctx" -> Some Ctx
  | "darray" when not only_reserved -> Some Darray
  | "default" -> Some Default
  | "dict" when not only_reserved -> Some Dict
  | "do" -> Some Do
  | "double" when not only_reserved -> Some Double
  | "echo" -> Some Echo
  | "else" -> Some Else
  | "empty" -> Some Empty
  | "endif" -> Some Endif
  | "enum" when not only_reserved -> Some Enum
  | "eval" -> Some Eval
  | "exports" when not only_reserved -> Some Exports
  | "extends" -> Some Extends
  | "fallthrough" when not only_reserved -> Some Fallthrough
  | "float" when not only_reserved -> Some Float
  | "file" when not only_reserved -> Some File
  | "final" -> Some Final
  | "finally" -> Some Finally
  | "for" -> Some For
  | "foreach" -> Some Foreach
  | "function" -> Some Function
  | "global" -> Some Global
  | "if" -> Some If
  | "implements" -> Some Implements
  | "imports" when not only_reserved -> Some Imports
  | "include" -> Some Include
  | "include_once" -> Some Include_once
  | "inout" -> Some Inout
  | "instanceof" -> Some Instanceof
  | "insteadof" -> Some Insteadof
  | "int" when not only_reserved -> Some Int
  | "integer" when not only_reserved -> Some Integer
  | "interface" -> Some Interface
  | "is" when not only_reserved -> Some Is
  | "isset" -> Some Isset
  | "keyset" when not only_reserved -> Some Keyset
  | "lateinit" -> Some Lateinit
  | "list" -> Some List
  | "match" when not only_reserved -> Some Match
  | "mixed" when not only_reserved -> Some Mixed
  | "module" -> Some Module
  | "nameof" -> Some Nameof
  | "namespace" -> Some Namespace
  | "new" -> Some New
  | "newctx" when not only_reserved -> Some Newctx
  | "newtype" when not only_reserved -> Some Newtype
  | "noreturn" when not only_reserved -> Some Noreturn
  | "num" when not only_reserved -> Some Num
  | "parent" when not only_reserved -> Some Parent
  | "print" -> Some Print
  | "private" -> Some Private
  | "protected" -> Some Protected
  | "public" -> Some Public
  | "real" when not only_reserved -> Some Real
  | "reify" when not only_reserved -> Some Reify
  | "require" -> Some Require
  | "require_once" -> Some Require_once
  | "required" -> Some Required
  | "resource" when not only_reserved -> Some Resource
  | "return" -> Some Return
  | "self" when not only_reserved -> Some Self
  | "shape" -> Some Shape
  | "static" -> Some Static
  | "string" when not only_reserved -> Some String
  | "super" when not only_reserved -> Some Super
  | "switch" -> Some Switch
  | "this" when not only_reserved -> Some This
  | "throw" -> Some Throw
  | "trait" -> Some Trait
  | "try" -> Some Try
  | "tuple" -> Some Tuple
  | "type" when not only_reserved -> Some Type
  | "unset" -> Some Unset
  | "upcast" when not only_reserved -> Some Upcast
  | "use" -> Some Use
  | "using" -> Some Using
  | "var" -> Some Var
  | "varray" when not only_reserved -> Some Varray
  | "vec" when not only_reserved -> Some Vec
  | "void" when not only_reserved -> Some Void
  | "with" when not only_reserved -> Some With
  | "where" when not only_reserved -> Some Where
  | "while" -> Some While
  | "yield" -> Some Yield
  | "null" when not only_reserved -> Some NullLiteral
  | "[" -> Some LeftBracket
  | "]" -> Some RightBracket
  | "(" -> Some LeftParen
  | ")" -> Some RightParen
  | "{" -> Some LeftBrace
  | "}" -> Some RightBrace
  | "." -> Some Dot
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
  | "?as" -> Some QuestionAs
  | "?:" -> Some QuestionColon
  | "??" -> Some QuestionQuestion
  | "??=" -> Some QuestionQuestionEqual
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
  | "/>" -> Some SlashGreaterThan
  | "</" -> Some LessThanSlash
  | "<?" -> Some LessThanQuestion
  | "`" -> Some Backtick
  | "xhp" when not only_reserved -> Some XHP
  | "#" -> Some Hash
  | "readonly" -> Some Readonly
  | "internal" when not only_reserved -> Some Internal
  | "package" -> Some Package
  | "let" when not only_reserved -> Some Let
  | "optional" when not only_reserved -> Some Optional
  | _ -> None

let to_string kind =
  match kind with
  (* No text tokens *)
  | EndOfFile -> "end_of_file"
  (* Given text tokens *)
  | Abstract -> "abstract"
  | Arraykey -> "arraykey"
  | As -> "as"
  | Async -> "async"
  | Attribute -> "attribute"
  | Await -> "await"
  | Backslash -> "\\"
  | Binary -> "binary"
  | Bool -> "bool"
  | Boolean -> "boolean"
  | Break -> "break"
  | Case -> "case"
  | Catch -> "catch"
  | Category -> "category"
  | Children -> "children"
  | Class -> "class"
  | Classname -> "classname"
  | Clone -> "clone"
  | Concurrent -> "concurrent"
  | Const -> "const"
  | Construct -> "__construct"
  | Continue -> "continue"
  | Ctx -> "ctx"
  | Darray -> "darray"
  | Default -> "default"
  | Dict -> "dict"
  | Do -> "do"
  | Double -> "double"
  | Echo -> "echo"
  | Else -> "else"
  | Empty -> "empty"
  | Endif -> "endif"
  | Enum -> "enum"
  | Eval -> "eval"
  | Exports -> "exports"
  | Extends -> "extends"
  | Fallthrough -> "fallthrough"
  | Float -> "float"
  | File -> "file"
  | Final -> "final"
  | Finally -> "finally"
  | For -> "for"
  | Foreach -> "foreach"
  | Function -> "function"
  | Global -> "global"
  | If -> "if"
  | Implements -> "implements"
  | Imports -> "imports"
  | Include -> "include"
  | Include_once -> "include_once"
  | Inout -> "inout"
  | Instanceof -> "instanceof"
  | Insteadof -> "insteadof"
  | Int -> "int"
  | Integer -> "integer"
  | Interface -> "interface"
  | Is -> "is"
  | Isset -> "isset"
  | Keyset -> "keyset"
  | Lateinit -> "lateinit"
  | List -> "list"
  | Match -> "match"
  | Mixed -> "mixed"
  | Module -> "module"
  | Nameof -> "nameof"
  | Namespace -> "namespace"
  | New -> "new"
  | Newctx -> "newctx"
  | Newtype -> "newtype"
  | Noreturn -> "noreturn"
  | Num -> "num"
  | Parent -> "parent"
  | Print -> "print"
  | Private -> "private"
  | Protected -> "protected"
  | Public -> "public"
  | Real -> "real"
  | Reify -> "reify"
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
  | Upcast -> "upcast"
  | Use -> "use"
  | Using -> "using"
  | Var -> "var"
  | Varray -> "varray"
  | Vec -> "vec"
  | Void -> "void"
  | With -> "with"
  | Where -> "where"
  | While -> "while"
  | Yield -> "yield"
  | NullLiteral -> "null"
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
  | QuestionAs -> "?as"
  | QuestionColon -> "?:"
  | QuestionQuestion -> "??"
  | QuestionQuestionEqual -> "??="
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
  | SlashGreaterThan -> "/>"
  | LessThanSlash -> "</"
  | LessThanQuestion -> "<?"
  | Backtick -> "`"
  | XHP -> "xhp"
  | Hash -> "#"
  | Readonly -> "readonly"
  | Internal -> "internal"
  | Package -> "package"
  | Let -> "let"
  | Optional -> "optional"
  (* Variable text tokens *)
  | ErrorToken -> "error_token"
  | Name -> "name"
  | Variable -> "variable"
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
  | Hashbang -> "hashbang"

let is_variable_text kind =
  match kind with
  | ErrorToken -> true
  | Name -> true
  | Variable -> true
  | DecimalLiteral -> true
  | OctalLiteral -> true
  | HexadecimalLiteral -> true
  | BinaryLiteral -> true
  | FloatingLiteral -> true
  | SingleQuotedStringLiteral -> true
  | DoubleQuotedStringLiteral -> true
  | DoubleQuotedStringLiteralHead -> true
  | StringLiteralBody -> true
  | DoubleQuotedStringLiteralTail -> true
  | HeredocStringLiteral -> true
  | HeredocStringLiteralHead -> true
  | HeredocStringLiteralTail -> true
  | NowdocStringLiteral -> true
  | BooleanLiteral -> true
  | XHPCategoryName -> true
  | XHPElementName -> true
  | XHPClassName -> true
  | XHPStringLiteral -> true
  | XHPBody -> true
  | XHPComment -> true
  | Hashbang -> true
  | _ -> false
