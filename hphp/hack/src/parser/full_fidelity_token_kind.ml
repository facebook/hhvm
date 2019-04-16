(**
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
  | And
  | Array
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
  | Const
  | Construct
  | Continue
  | Coroutine
  | Darray
  | Declare
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
  | Endfor
  | Endforeach
  | Enddeclare
  | Endif
  | Endswitch
  | Endwhile
  | Enum
  | Eval
  | Extends
  | Fallthrough
  | Float
  | File
  | Final
  | Finally
  | For
  | Foreach
  | From
  | Function
  | Global
  | Concurrent
  | Goto
  | HaltCompiler
  | If
  | Implements
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
  | Let
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
  | Real
  | Reify
  | Record
  | RecordDec
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
  | Suspend
  | Switch
  | This
  | Throw
  | Trait
  | Try
  | Tuple
  | Type
  | Unset
  | Use
  | Using
  | Var
  | Varray
  | Vec
  | Void
  | Where
  | While
  | Xor
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
  | LessThanGreaterThan
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
  | QuestionGreaterThan
  | ColonAt
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
  | Markup

  [@@deriving show]

let from_string keyword ~is_hack ~allow_xhp ~only_reserved =
  match keyword with
  | "true"                                        when not only_reserved -> Some BooleanLiteral
  | "false"                                       when not only_reserved -> Some BooleanLiteral
  | "abstract"                                                           -> Some Abstract
  | "and"                                                                -> Some And
  | "array"                                                              -> Some Array
  | "arraykey"        when is_hack                &&   not only_reserved -> Some Arraykey
  | "as"                                                                 -> Some As
  | "async"           when is_hack                                       -> Some Async
  | "attribute"       when (is_hack || allow_xhp) &&   not only_reserved -> Some Attribute
  | "await"           when is_hack                                       -> Some Await
  | "\\"                                                                -> Some Backslash
  | "binary"                                      when not only_reserved -> Some Binary
  | "bool"                                        when not only_reserved -> Some Bool
  | "boolean"                                     when not only_reserved -> Some Boolean
  | "break"                                                              -> Some Break
  | "case"                                                               -> Some Case
  | "catch"                                                              -> Some Catch
  | "category"        when (is_hack || allow_xhp) &&   not only_reserved -> Some Category
  | "children"        when (is_hack || allow_xhp) &&   not only_reserved -> Some Children
  | "class"                                                              -> Some Class
  | "classname"       when is_hack                &&   not only_reserved -> Some Classname
  | "clone"                                                              -> Some Clone
  | "const"                                                              -> Some Const
  | "__construct"                                                        -> Some Construct
  | "continue"                                                           -> Some Continue
  | "coroutine"       when is_hack                &&   not only_reserved -> Some Coroutine
  | "darray"          when is_hack                &&   not only_reserved -> Some Darray
  | "declare"                                                            -> Some Declare
  | "default"                                                            -> Some Default
  | "define"                                      when not only_reserved -> Some Define
  | "__destruct"                                                         -> Some Destruct
  | "dict"                                        when not only_reserved -> Some Dict
  | "do"                                                                 -> Some Do
  | "double"                                      when not only_reserved -> Some Double
  | "echo"                                                               -> Some Echo
  | "else"                                                               -> Some Else
  | "elseif"                                                             -> Some Elseif
  | "empty"                                                              -> Some Empty
  | "endfor"                                                             -> Some Endfor
  | "endforeach"                                                         -> Some Endforeach
  | "enddeclare"                                                         -> Some Enddeclare
  | "endif"                                                              -> Some Endif
  | "endswitch"                                                          -> Some Endswitch
  | "endwhile"                                                           -> Some Endwhile
  | "enum"            when (is_hack || allow_xhp) &&   not only_reserved -> Some Enum
  | "eval"                                                               -> Some Eval
  | "extends"                                                            -> Some Extends
  | "fallthrough"     when is_hack                &&   not only_reserved -> Some Fallthrough
  | "float"                                       when not only_reserved -> Some Float
  | "file"            when is_hack                &&   not only_reserved -> Some File
  | "final"                                                              -> Some Final
  | "finally"                                                            -> Some Finally
  | "for"                                                                -> Some For
  | "foreach"                                                            -> Some Foreach
  | "from"                                        when not only_reserved -> Some From
  | "function"                                                           -> Some Function
  | "global"                                                             -> Some Global
  | "concurrent"      when is_hack                                       -> Some Concurrent
  | "goto"                                                               -> Some Goto
  | "__halt_compiler"                                                    -> Some HaltCompiler
  | "if"                                                                 -> Some If
  | "implements"                                                         -> Some Implements
  | "include"                                                            -> Some Include
  | "include_once"                                                       -> Some Include_once
  | "inout"           when is_hack                                       -> Some Inout
  | "instanceof"                                                         -> Some Instanceof
  | "insteadof"                                                          -> Some Insteadof
  | "int"                                         when not only_reserved -> Some Int
  | "integer"                                     when not only_reserved -> Some Integer
  | "interface"                                                          -> Some Interface
  | "is"              when is_hack                &&   not only_reserved -> Some Is
  | "isset"                                                              -> Some Isset
  | "keyset"                                      when not only_reserved -> Some Keyset
  | "let"             when is_hack                &&   not only_reserved -> Some Let
  | "list"                                                               -> Some List
  | "mixed"           when is_hack                &&   not only_reserved -> Some Mixed
  | "namespace"                                   when not only_reserved -> Some Namespace
  | "new"                                                                -> Some New
  | "newtype"         when is_hack                &&   not only_reserved -> Some Newtype
  | "noreturn"        when is_hack                &&   not only_reserved -> Some Noreturn
  | "num"             when is_hack                &&   not only_reserved -> Some Num
  | "object"                                      when not only_reserved -> Some Object
  | "or"                                                                 -> Some Or
  | "parent"                                      when not only_reserved -> Some Parent
  | "print"                                                              -> Some Print
  | "private"                                                            -> Some Private
  | "protected"                                                          -> Some Protected
  | "public"                                                             -> Some Public
  | "real"                                        when not only_reserved -> Some Real
  | "reify"           when is_hack                &&   not only_reserved -> Some Reify
  | "recordname"      when is_hack                                       -> Some Record
  | "record"          when (is_hack || allow_xhp)                        -> Some RecordDec
  | "require"                                                            -> Some Require
  | "require_once"                                                       -> Some Require_once
  | "required"        when (is_hack || allow_xhp)                        -> Some Required
  | "resource"                                    when not only_reserved -> Some Resource
  | "return"                                                             -> Some Return
  | "self"                                        when not only_reserved -> Some Self
  | "shape"           when is_hack                                       -> Some Shape
  | "static"                                                             -> Some Static
  | "string"                                      when not only_reserved -> Some String
  | "super"                                       when not only_reserved -> Some Super
  | "suspend"         when is_hack                &&   not only_reserved -> Some Suspend
  | "switch"                                                             -> Some Switch
  | "this"            when is_hack                &&   not only_reserved -> Some This
  | "throw"                                                              -> Some Throw
  | "trait"                                                              -> Some Trait
  | "try"                                                                -> Some Try
  | "tuple"           when is_hack                                       -> Some Tuple
  | "type"            when is_hack                &&   not only_reserved -> Some Type
  | "unset"                                                              -> Some Unset
  | "use"                                                                -> Some Use
  | "using"           when is_hack                                       -> Some Using
  | "var"                                                                -> Some Var
  | "varray"          when is_hack                &&   not only_reserved -> Some Varray
  | "vec"                                         when not only_reserved -> Some Vec
  | "void"                                        when not only_reserved -> Some Void
  | "where"           when is_hack                &&   not only_reserved -> Some Where
  | "while"                                                              -> Some While
  | "xor"                                                                -> Some Xor
  | "yield"                                                              -> Some Yield
  | "null"                                        when not only_reserved -> Some NullLiteral
  | "["                                                                  -> Some LeftBracket
  | "]"                                                                  -> Some RightBracket
  | "("                                                                  -> Some LeftParen
  | ")"                                                                  -> Some RightParen
  | "{"                                                                  -> Some LeftBrace
  | "}"                                                                  -> Some RightBrace
  | "."                                                                  -> Some Dot
  | "->"                                                                 -> Some MinusGreaterThan
  | "++"                                                                 -> Some PlusPlus
  | "--"                                                                 -> Some MinusMinus
  | "**"                                                                 -> Some StarStar
  | "*"                                                                  -> Some Star
  | "+"                                                                  -> Some Plus
  | "-"                                                                  -> Some Minus
  | "~"                                                                  -> Some Tilde
  | "!"                                                                  -> Some Exclamation
  | "$"                                                                  -> Some Dollar
  | "/"                                                                  -> Some Slash
  | "%"                                                                  -> Some Percent
  | "<>"                                                                 -> Some LessThanGreaterThan
  | "<=>"                                                                -> Some LessThanEqualGreaterThan
  | "<<"                                                                 -> Some LessThanLessThan
  | ">>"                                                                 -> Some GreaterThanGreaterThan
  | "<"                                                                  -> Some LessThan
  | ">"                                                                  -> Some GreaterThan
  | "<="                                                                 -> Some LessThanEqual
  | ">="                                                                 -> Some GreaterThanEqual
  | "=="                                                                 -> Some EqualEqual
  | "==="                                                                -> Some EqualEqualEqual
  | "!="                                                                 -> Some ExclamationEqual
  | "!=="                                                                -> Some ExclamationEqualEqual
  | "^"                                                                  -> Some Carat
  | "|"                                                                  -> Some Bar
  | "&"                                                                  -> Some Ampersand
  | "&&"                                                                 -> Some AmpersandAmpersand
  | "||"                                                                 -> Some BarBar
  | "?"                                                                  -> Some Question
  | "?as"                                                                -> Some QuestionAs
  | "?:"                                                                 -> Some QuestionColon
  | "??"                                                                 -> Some QuestionQuestion
  | "??="                                                                -> Some QuestionQuestionEqual
  | ":"                                                                  -> Some Colon
  | ";"                                                                  -> Some Semicolon
  | "="                                                                  -> Some Equal
  | "**="                                                                -> Some StarStarEqual
  | "*="                                                                 -> Some StarEqual
  | "/="                                                                 -> Some SlashEqual
  | "%="                                                                 -> Some PercentEqual
  | "+="                                                                 -> Some PlusEqual
  | "-="                                                                 -> Some MinusEqual
  | ".="                                                                 -> Some DotEqual
  | "<<="                                                                -> Some LessThanLessThanEqual
  | ">>="                                                                -> Some GreaterThanGreaterThanEqual
  | "&="                                                                 -> Some AmpersandEqual
  | "^="                                                                 -> Some CaratEqual
  | "|="                                                                 -> Some BarEqual
  | ","                                                                  -> Some Comma
  | "@"                                                                  -> Some At
  | "::"                                                                 -> Some ColonColon
  | "=>"                                                                 -> Some EqualGreaterThan
  | "==>"                                                                -> Some EqualEqualGreaterThan
  | "?->"                                                                -> Some QuestionMinusGreaterThan
  | "..."                                                                -> Some DotDotDot
  | "$$"                                                                 -> Some DollarDollar
  | "|>"                                                                 -> Some BarGreaterThan
  | "/>"                                                                 -> Some SlashGreaterThan
  | "</"                                                                 -> Some LessThanSlash
  | "<?"                                                                 -> Some LessThanQuestion
  | "?>"                                                                 -> Some QuestionGreaterThan
  | ":@"              when is_hack                                       -> Some ColonAt
  | _              -> None

let to_string kind =
  match kind with
(* No text tokens *)
  | EndOfFile                     -> "end_of_file"
  (* Given text tokens *)
  | Abstract                      -> "abstract"
  | And                           -> "and"
  | Array                         -> "array"
  | Arraykey                      -> "arraykey"
  | As                            -> "as"
  | Async                         -> "async"
  | Attribute                     -> "attribute"
  | Await                         -> "await"
  | Backslash                     -> "\\"
  | Binary                        -> "binary"
  | Bool                          -> "bool"
  | Boolean                       -> "boolean"
  | Break                         -> "break"
  | Case                          -> "case"
  | Catch                         -> "catch"
  | Category                      -> "category"
  | Children                      -> "children"
  | Class                         -> "class"
  | Classname                     -> "classname"
  | Clone                         -> "clone"
  | Const                         -> "const"
  | Construct                     -> "__construct"
  | Continue                      -> "continue"
  | Coroutine                     -> "coroutine"
  | Darray                        -> "darray"
  | Declare                       -> "declare"
  | Default                       -> "default"
  | Define                        -> "define"
  | Destruct                      -> "__destruct"
  | Dict                          -> "dict"
  | Do                            -> "do"
  | Double                        -> "double"
  | Echo                          -> "echo"
  | Else                          -> "else"
  | Elseif                        -> "elseif"
  | Empty                         -> "empty"
  | Endfor                        -> "endfor"
  | Endforeach                    -> "endforeach"
  | Enddeclare                    -> "enddeclare"
  | Endif                         -> "endif"
  | Endswitch                     -> "endswitch"
  | Endwhile                      -> "endwhile"
  | Enum                          -> "enum"
  | Eval                          -> "eval"
  | Extends                       -> "extends"
  | Fallthrough                   -> "fallthrough"
  | Float                         -> "float"
  | File                          -> "file"
  | Final                         -> "final"
  | Finally                       -> "finally"
  | For                           -> "for"
  | Foreach                       -> "foreach"
  | From                          -> "from"
  | Function                      -> "function"
  | Global                        -> "global"
  | Concurrent                    -> "concurrent"
  | Goto                          -> "goto"
  | HaltCompiler                  -> "__halt_compiler"
  | If                            -> "if"
  | Implements                    -> "implements"
  | Include                       -> "include"
  | Include_once                  -> "include_once"
  | Inout                         -> "inout"
  | Instanceof                    -> "instanceof"
  | Insteadof                     -> "insteadof"
  | Int                           -> "int"
  | Integer                       -> "integer"
  | Interface                     -> "interface"
  | Is                            -> "is"
  | Isset                         -> "isset"
  | Keyset                        -> "keyset"
  | Let                           -> "let"
  | List                          -> "list"
  | Mixed                         -> "mixed"
  | Namespace                     -> "namespace"
  | New                           -> "new"
  | Newtype                       -> "newtype"
  | Noreturn                      -> "noreturn"
  | Num                           -> "num"
  | Object                        -> "object"
  | Or                            -> "or"
  | Parent                        -> "parent"
  | Print                         -> "print"
  | Private                       -> "private"
  | Protected                     -> "protected"
  | Public                        -> "public"
  | Real                          -> "real"
  | Reify                         -> "reify"
  | Record                        -> "recordname"
  | RecordDec                     -> "record"
  | Require                       -> "require"
  | Require_once                  -> "require_once"
  | Required                      -> "required"
  | Resource                      -> "resource"
  | Return                        -> "return"
  | Self                          -> "self"
  | Shape                         -> "shape"
  | Static                        -> "static"
  | String                        -> "string"
  | Super                         -> "super"
  | Suspend                       -> "suspend"
  | Switch                        -> "switch"
  | This                          -> "this"
  | Throw                         -> "throw"
  | Trait                         -> "trait"
  | Try                           -> "try"
  | Tuple                         -> "tuple"
  | Type                          -> "type"
  | Unset                         -> "unset"
  | Use                           -> "use"
  | Using                         -> "using"
  | Var                           -> "var"
  | Varray                        -> "varray"
  | Vec                           -> "vec"
  | Void                          -> "void"
  | Where                         -> "where"
  | While                         -> "while"
  | Xor                           -> "xor"
  | Yield                         -> "yield"
  | NullLiteral                   -> "null"
  | LeftBracket                   -> "["
  | RightBracket                  -> "]"
  | LeftParen                     -> "("
  | RightParen                    -> ")"
  | LeftBrace                     -> "{"
  | RightBrace                    -> "}"
  | Dot                           -> "."
  | MinusGreaterThan              -> "->"
  | PlusPlus                      -> "++"
  | MinusMinus                    -> "--"
  | StarStar                      -> "**"
  | Star                          -> "*"
  | Plus                          -> "+"
  | Minus                         -> "-"
  | Tilde                         -> "~"
  | Exclamation                   -> "!"
  | Dollar                        -> "$"
  | Slash                         -> "/"
  | Percent                       -> "%"
  | LessThanGreaterThan           -> "<>"
  | LessThanEqualGreaterThan      -> "<=>"
  | LessThanLessThan              -> "<<"
  | GreaterThanGreaterThan        -> ">>"
  | LessThan                      -> "<"
  | GreaterThan                   -> ">"
  | LessThanEqual                 -> "<="
  | GreaterThanEqual              -> ">="
  | EqualEqual                    -> "=="
  | EqualEqualEqual               -> "==="
  | ExclamationEqual              -> "!="
  | ExclamationEqualEqual         -> "!=="
  | Carat                         -> "^"
  | Bar                           -> "|"
  | Ampersand                     -> "&"
  | AmpersandAmpersand            -> "&&"
  | BarBar                        -> "||"
  | Question                      -> "?"
  | QuestionAs                    -> "?as"
  | QuestionColon                 -> "?:"
  | QuestionQuestion              -> "??"
  | QuestionQuestionEqual         -> "??="
  | Colon                         -> ":"
  | Semicolon                     -> ";"
  | Equal                         -> "="
  | StarStarEqual                 -> "**="
  | StarEqual                     -> "*="
  | SlashEqual                    -> "/="
  | PercentEqual                  -> "%="
  | PlusEqual                     -> "+="
  | MinusEqual                    -> "-="
  | DotEqual                      -> ".="
  | LessThanLessThanEqual         -> "<<="
  | GreaterThanGreaterThanEqual   -> ">>="
  | AmpersandEqual                -> "&="
  | CaratEqual                    -> "^="
  | BarEqual                      -> "|="
  | Comma                         -> ","
  | At                            -> "@"
  | ColonColon                    -> "::"
  | EqualGreaterThan              -> "=>"
  | EqualEqualGreaterThan         -> "==>"
  | QuestionMinusGreaterThan      -> "?->"
  | DotDotDot                     -> "..."
  | DollarDollar                  -> "$$"
  | BarGreaterThan                -> "|>"
  | SlashGreaterThan              -> "/>"
  | LessThanSlash                 -> "</"
  | LessThanQuestion              -> "<?"
  | QuestionGreaterThan           -> "?>"
  | ColonAt                       -> ":@"
  (* Variable text tokens *)
  | ErrorToken                    -> "error_token"
  | Name                          -> "name"
  | Variable                      -> "variable"
  | DecimalLiteral                -> "decimal_literal"
  | OctalLiteral                  -> "octal_literal"
  | HexadecimalLiteral            -> "hexadecimal_literal"
  | BinaryLiteral                 -> "binary_literal"
  | FloatingLiteral               -> "floating_literal"
  | SingleQuotedStringLiteral     -> "single_quoted_string_literal"
  | DoubleQuotedStringLiteral     -> "double_quoted_string_literal"
  | DoubleQuotedStringLiteralHead -> "double_quoted_string_literal_head"
  | StringLiteralBody             -> "string_literal_body"
  | DoubleQuotedStringLiteralTail -> "double_quoted_string_literal_tail"
  | HeredocStringLiteral          -> "heredoc_string_literal"
  | HeredocStringLiteralHead      -> "heredoc_string_literal_head"
  | HeredocStringLiteralTail      -> "heredoc_string_literal_tail"
  | NowdocStringLiteral           -> "nowdoc_string_literal"
  | BooleanLiteral                -> "boolean_literal"
  | XHPCategoryName               -> "XHP_category_name"
  | XHPElementName                -> "XHP_element_name"
  | XHPClassName                  -> "XHP_class_name"
  | XHPStringLiteral              -> "XHP_string_literal"
  | XHPBody                       -> "XHP_body"
  | XHPComment                    -> "XHP_comment"
  | Markup                        -> "markup"


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
  | Markup -> true
  | _ -> false
