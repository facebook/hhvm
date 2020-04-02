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
  | Default
  | Define
  | Dict
  | Do
  | Double
  | Echo
  | Else
  | Elseif
  | Empty
  | Endfor
  | Endforeach
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
  | Real
  | Reify
  | Record
  | RecordDec
  | Require
  | Require_once
  | Required
  | Lateinit
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
  | ColonAt
  | XHP
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

let from_string keyword ~only_reserved =
  match keyword with
  | "true"            when not only_reserved -> Some BooleanLiteral
  | "false"           when not only_reserved -> Some BooleanLiteral
  | "abstract"                            -> Some Abstract
  | "array"                               -> Some Array
  | "arraykey"     when not only_reserved -> Some Arraykey
  | "as"                                  -> Some As
  | "async"                               -> Some Async
  | "attribute"    when not only_reserved -> Some Attribute
  | "await"                               -> Some Await
  | "\\"                                 -> Some Backslash
  | "binary"       when not only_reserved -> Some Binary
  | "bool"         when not only_reserved -> Some Bool
  | "boolean"      when not only_reserved -> Some Boolean
  | "break"                               -> Some Break
  | "case"                                -> Some Case
  | "catch"                               -> Some Catch
  | "category"     when not only_reserved -> Some Category
  | "children"     when not only_reserved -> Some Children
  | "class"                               -> Some Class
  | "classname"    when not only_reserved -> Some Classname
  | "clone"                               -> Some Clone
  | "const"                               -> Some Const
  | "__construct"                         -> Some Construct
  | "continue"                            -> Some Continue
  | "coroutine"    when not only_reserved -> Some Coroutine
  | "darray"       when not only_reserved -> Some Darray
  | "default"                             -> Some Default
  | "define"       when not only_reserved -> Some Define
  | "dict"         when not only_reserved -> Some Dict
  | "do"                                  -> Some Do
  | "double"       when not only_reserved -> Some Double
  | "echo"                                -> Some Echo
  | "else"                                -> Some Else
  | "elseif"                              -> Some Elseif
  | "empty"                               -> Some Empty
  | "endfor"                              -> Some Endfor
  | "endforeach"                          -> Some Endforeach
  | "endif"                               -> Some Endif
  | "endswitch"                           -> Some Endswitch
  | "endwhile"                            -> Some Endwhile
  | "enum"         when not only_reserved -> Some Enum
  | "eval"                                -> Some Eval
  | "extends"                             -> Some Extends
  | "fallthrough"  when not only_reserved -> Some Fallthrough
  | "float"        when not only_reserved -> Some Float
  | "file"         when not only_reserved -> Some File
  | "final"                               -> Some Final
  | "finally"                             -> Some Finally
  | "for"                                 -> Some For
  | "foreach"                             -> Some Foreach
  | "from"         when not only_reserved -> Some From
  | "function"                            -> Some Function
  | "global"                              -> Some Global
  | "concurrent"                          -> Some Concurrent
  | "goto"                                -> Some Goto
  | "if"                                  -> Some If
  | "implements"                          -> Some Implements
  | "include"                             -> Some Include
  | "include_once"                        -> Some Include_once
  | "inout"                               -> Some Inout
  | "instanceof"                          -> Some Instanceof
  | "insteadof"                           -> Some Insteadof
  | "int"          when not only_reserved -> Some Int
  | "integer"      when not only_reserved -> Some Integer
  | "interface"                           -> Some Interface
  | "is"           when not only_reserved -> Some Is
  | "isset"                               -> Some Isset
  | "keyset"       when not only_reserved -> Some Keyset
  | "list"                                -> Some List
  | "mixed"        when not only_reserved -> Some Mixed
  | "namespace"                           -> Some Namespace
  | "new"                                 -> Some New
  | "newtype"      when not only_reserved -> Some Newtype
  | "noreturn"     when not only_reserved -> Some Noreturn
  | "num"          when not only_reserved -> Some Num
  | "object"       when not only_reserved -> Some Object
  | "parent"       when not only_reserved -> Some Parent
  | "print"                               -> Some Print
  | "private"                             -> Some Private
  | "protected"                           -> Some Protected
  | "public"                              -> Some Public
  | "real"         when not only_reserved -> Some Real
  | "reify"        when not only_reserved -> Some Reify
  | "recordname"                          -> Some Record
  | "record"                              -> Some RecordDec
  | "require"                             -> Some Require
  | "require_once"                        -> Some Require_once
  | "required"                            -> Some Required
  | "lateinit"                            -> Some Lateinit
  | "resource"     when not only_reserved -> Some Resource
  | "return"                              -> Some Return
  | "self"         when not only_reserved -> Some Self
  | "shape"                               -> Some Shape
  | "static"                              -> Some Static
  | "string"       when not only_reserved -> Some String
  | "super"        when not only_reserved -> Some Super
  | "suspend"      when not only_reserved -> Some Suspend
  | "switch"                              -> Some Switch
  | "this"         when not only_reserved -> Some This
  | "throw"                               -> Some Throw
  | "trait"                               -> Some Trait
  | "try"                                 -> Some Try
  | "tuple"                               -> Some Tuple
  | "type"         when not only_reserved -> Some Type
  | "unset"                               -> Some Unset
  | "use"                                 -> Some Use
  | "using"                               -> Some Using
  | "var"                                 -> Some Var
  | "varray"       when not only_reserved -> Some Varray
  | "vec"          when not only_reserved -> Some Vec
  | "void"         when not only_reserved -> Some Void
  | "where"        when not only_reserved -> Some Where
  | "while"                               -> Some While
  | "yield"                               -> Some Yield
  | "null"         when not only_reserved -> Some NullLiteral
  | "["                                   -> Some LeftBracket
  | "]"                                   -> Some RightBracket
  | "("                                   -> Some LeftParen
  | ")"                                   -> Some RightParen
  | "{"                                   -> Some LeftBrace
  | "}"                                   -> Some RightBrace
  | "."                                   -> Some Dot
  | "->"                                  -> Some MinusGreaterThan
  | "++"                                  -> Some PlusPlus
  | "--"                                  -> Some MinusMinus
  | "**"                                  -> Some StarStar
  | "*"                                   -> Some Star
  | "+"                                   -> Some Plus
  | "-"                                   -> Some Minus
  | "~"                                   -> Some Tilde
  | "!"                                   -> Some Exclamation
  | "$"                                   -> Some Dollar
  | "/"                                   -> Some Slash
  | "%"                                   -> Some Percent
  | "<=>"                                 -> Some LessThanEqualGreaterThan
  | "<<"                                  -> Some LessThanLessThan
  | ">>"                                  -> Some GreaterThanGreaterThan
  | "<"                                   -> Some LessThan
  | ">"                                   -> Some GreaterThan
  | "<="                                  -> Some LessThanEqual
  | ">="                                  -> Some GreaterThanEqual
  | "=="                                  -> Some EqualEqual
  | "==="                                 -> Some EqualEqualEqual
  | "!="                                  -> Some ExclamationEqual
  | "!=="                                 -> Some ExclamationEqualEqual
  | "^"                                   -> Some Carat
  | "|"                                   -> Some Bar
  | "&"                                   -> Some Ampersand
  | "&&"                                  -> Some AmpersandAmpersand
  | "||"                                  -> Some BarBar
  | "?"                                   -> Some Question
  | "?as"                                 -> Some QuestionAs
  | "?:"                                  -> Some QuestionColon
  | "??"                                  -> Some QuestionQuestion
  | "??="                                 -> Some QuestionQuestionEqual
  | ":"                                   -> Some Colon
  | ";"                                   -> Some Semicolon
  | "="                                   -> Some Equal
  | "**="                                 -> Some StarStarEqual
  | "*="                                  -> Some StarEqual
  | "/="                                  -> Some SlashEqual
  | "%="                                  -> Some PercentEqual
  | "+="                                  -> Some PlusEqual
  | "-="                                  -> Some MinusEqual
  | ".="                                  -> Some DotEqual
  | "<<="                                 -> Some LessThanLessThanEqual
  | ">>="                                 -> Some GreaterThanGreaterThanEqual
  | "&="                                  -> Some AmpersandEqual
  | "^="                                  -> Some CaratEqual
  | "|="                                  -> Some BarEqual
  | ","                                   -> Some Comma
  | "@"                                   -> Some At
  | "::"                                  -> Some ColonColon
  | "=>"                                  -> Some EqualGreaterThan
  | "==>"                                 -> Some EqualEqualGreaterThan
  | "?->"                                 -> Some QuestionMinusGreaterThan
  | "..."                                 -> Some DotDotDot
  | "$$"                                  -> Some DollarDollar
  | "|>"                                  -> Some BarGreaterThan
  | "/>"                                  -> Some SlashGreaterThan
  | "</"                                  -> Some LessThanSlash
  | "<?"                                  -> Some LessThanQuestion
  | ":@"                                  -> Some ColonAt
  | "xhp"          when not only_reserved -> Some XHP
  | _              -> None

let to_string kind =
  match kind with
(* No text tokens *)
  | EndOfFile                     -> "end_of_file"
  (* Given text tokens *)
  | Abstract                      -> "abstract"
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
  | Default                       -> "default"
  | Define                        -> "define"
  | Dict                          -> "dict"
  | Do                            -> "do"
  | Double                        -> "double"
  | Echo                          -> "echo"
  | Else                          -> "else"
  | Elseif                        -> "elseif"
  | Empty                         -> "empty"
  | Endfor                        -> "endfor"
  | Endforeach                    -> "endforeach"
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
  | List                          -> "list"
  | Mixed                         -> "mixed"
  | Namespace                     -> "namespace"
  | New                           -> "new"
  | Newtype                       -> "newtype"
  | Noreturn                      -> "noreturn"
  | Num                           -> "num"
  | Object                        -> "object"
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
  | Lateinit                      -> "lateinit"
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
  | ColonAt                       -> ":@"
  | XHP                           -> "xhp"
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
