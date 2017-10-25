(**
* Copyright (c) 2016, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the "hack" directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*
*)

module type Grammar = sig
  type term

  type nonterm

  type symbol =
  | Term of term
  | NonTerm of nonterm

  val start : nonterm

  val grammar : nonterm -> symbol list list

  val to_string : term -> string

  val nonterm_to_string : nonterm -> string

end

module HackGrammarTypeDef = struct
  module TokenKind = Full_fidelity_token_kind

  type term = TokenKind.t
  and nonterm = string * (unit -> symbol list list)
  and symbol =
  | Term of term
  | NonTerm of nonterm
end

module HackGrammarHelper = struct

  let is_keyword text =
    let text = String.lowercase text in
    match Full_fidelity_token_kind.from_string text with
    | Some _ -> true
    | _ -> false

  (* generate a random name *)
  let rec gen_name () =
    let var_length_max = 8 in
    let alpha_size = 26 in (* 26 English alphabet *)
    let postfix_size = Random.int var_length_max in
    let first_symbol = int_of_char 'a' + Random.int alpha_size in
    let second_symbol = int_of_char 'A' + Random.int alpha_size in
    let generate () =
      match Random.int (2 * alpha_size) with
      | n when n < 26 -> int_of_char 'a' + n
      | n -> int_of_char 'A' + n - 26
    in
    let init_fun i =
      char_of_int (if i = 0 then first_symbol
                   else if i = 1 then second_symbol
                   else generate ()) in
    let result = String.init (postfix_size + 2) init_fun in
    if is_keyword result then gen_name() else result

  (* create the powerset of the given set in lists. For our use case, the
   * original and the resultant sets are ordered and the function should
   * preserve the order *)
  let power_set lst =
    let folder acc el = [] :: List.map (fun lst -> el :: lst) acc in
    List.fold_left folder [[]] (List.rev lst)

  (* return the combinations of all pairs from each of list1 and list2
   * The pair is ordered so that the element in list 1 is the first argument to
   * the combinor *)
  let cross lst1 lst2 combinor =
    List.flatten
      (List.map (fun x -> List.map (fun y -> combinor x y) lst2 ) lst1)
end

module HackGrammarTermSpec = struct
  include HackGrammarTypeDef
  include HackGrammarHelper

  open TokenKind

  (************************ Definition of terms ****************************)
  let term_array = Term Array
  let term_list = Term List
  let name = Term Name
  let qualified_name = Term QualifiedName
  let equal = Term Equal
  let equal_equal = Term EqualEqual
  let equal_equal_equal = Term EqualEqualEqual
  let equal_greater_than = Term EqualGreaterThan
  let equal_equal_greater_than = Term EqualEqualGreaterThan
  let dollar = Term Dollar
  let dollar_dollar = Term DollarDollar
  let star_star_equal = Term StarStarEqual
  let star_equal = Term StarEqual
  let slash_equal = Term SlashEqual
  let percent_equal = Term PercentEqual
  let plus_equal = Term PlusEqual
  let plus_plus = Term PlusPlus
  let minus_minus = Term MinusMinus
  let at = Term At
  let plus = Term Plus
  let minus = Term Minus
  let star = Term Star
  let slash = Term Slash
  let percent = Term Percent
  let dot = Term Dot
  let ampersand = Term Ampersand
  let ampersand_ampersand = Term AmpersandAmpersand
  let carat = Term Carat
  let bar = Term Bar
  let bar_bar = Term BarBar
  let exclamation = Term Exclamation
  let exclamation_equal = Term ExclamationEqual
  let exclamation_equal_equal = Term ExclamationEqualEqual
  let tilde = Term Tilde
  let minus_equal = Term MinusEqual
  let dot_equal = Term DotEqual
  let dot_dot_dot = Term DotDotDot
  let less_than = Term LessThan
  let less_than_equal = Term LessThanEqual
  let less_than_less_than = Term LessThanLessThan
  let less_less_equal = Term LessThanLessThanEqual
  let greater_than = Term GreaterThan
  let greater_than_equal = Term GreaterThanEqual
  let greater_than_greater_than = Term GreaterThanGreaterThan
  let greater_greater_equal = Term GreaterThanGreaterThanEqual
  let ampersand_equal = Term AmpersandEqual
  let carat_equal = Term CaratEqual
  let bar_equal = Term BarEqual
  let bar_greater_than = Term BarGreaterThan
  let minus_greater_than = Term MinusGreaterThan
  let colon_colon = Term ColonColon
  let star_star = Term StarStar
  let question = Term Question
  let question_question = Term QuestionQuestion
  let question_minus_greater_than = Term QuestionMinusGreaterThan
  let clone = Term Clone
  let require = Term Require
  let require_once = Term Require_once
  let term_function = Term Function
  let abstract = Term Abstract
  let final = Term Final
  let enum = Term Enum
  let interface = Term Interface
  let trait = Term Trait
  let term_class = Term Class
  let namespace = Term Namespace
  let implements = Term Implements
  let extends = Term Extends
  let use = Term Use
  let const = Term Const
  let destruct = Term Destruct
  let construct = Term Construct
  let term_public = Term Public
  let term_private = Term Private
  let term_protected = Term Protected
  let term_new = Term New
  let instanceof = Term Instanceof
  let async = Term Async
  let case = Term Case
  let default = Term Default
  let colon = Term Colon
  let comma = Term Comma
  let left_paren = Term LeftParen
  let right_paren = Term RightParen
  let left_brace = Term LeftBrace
  let right_brace = Term RightBrace
  let left_bracket = Term LeftBracket
  let right_bracket = Term RightBracket
  let static = Term Static
  let semicolon = Term Semicolon
  let continue = Term Continue
  let break = Term Break
  let return = Term Return
  let throw = Term Throw
  let term_try = Term Try
  let finally = Term Finally
  let yield = Term Yield
  let switch = Term Switch
  let term_if = Term If
  let term_else = Term Else
  let elseif = Term Elseif
  let term_do = Term Do
  let term_while = Term While
  let term_for = Term For
  let foreach = Term Foreach
  let await = Term Await
  let term_as = Term As
  let catch = Term Catch
  let this = Term This
  let self = Term Self
  let parent = Term Parent
  let term_type = Term Type
  let newtype = Term Newtype
  let void = Term Void
  let term_bool = Term Bool
  let term_int = Term Int
  let term_float = Term Float
  let term_string = Term String
  let noreturn = Term Noreturn
  let arraykey = Term Arraykey
  let num = Term Num
  let resource = Term Resource
  let classname = Term Classname
  let mixed = Term Mixed
  let boolean_literal = Term BooleanLiteral
  let floating_literal = Term FloatingLiteral
  let null_literal = Term NullLiteral
  let decimal_literal = Term DecimalLiteral
  let octal_literal = Term OctalLiteral
  let hexadecimal_literal = Term HexadecimalLiteral
  let binary_literal = Term BinaryLiteral
  let single_quoted_string_literal = Term SingleQuotedStringLiteral
  let double_quoted_string_literal = Term DoubleQuotedStringLiteral
  let heredoc_string_literal = Term HeredocStringLiteral
  let nowdoc_string_literal = Term NowdocStringLiteral
  let echo = Term Echo
  (* TODO should be exit but doesnt exits, use echo for now *)
  let term_exit = Term Echo
  let tuple = Term Tuple
  let shape = Term Shape
  let xhp_element_name = Term XHPElementName
  let xhp_string_literal = Term XHPStringLiteral
  let xhp_body_text = Term XHPBody
  let xhp_comment = Term XHPComment
  let slash_greater_than = Term SlashGreaterThan
  let less_than_slash = Term LessThanSlash
  let inout = Term Inout

  (* For keywords, use TokenKind to_string. For names, use gen_name to generate
   * name *)
  let to_string = function
  | Dollar -> "$" ^ (gen_name ())
  | QualifiedName
  | Name -> gen_name()
  (* TODO more general approach *)
  (* | QualifiedName -> (gen_name ()) ^ "/" ^ (gen_name ()) *)
  | BooleanLiteral -> "true"
  | FloatingLiteral -> "42.42"
  | NullLiteral -> "null"
  | HexadecimalLiteral -> "0x16"
  | DecimalLiteral -> "12"
  | OctalLiteral -> "0123"
  | BinaryLiteral -> "0b101010"
  (* TODO *)
  (* | HeredocStringLiteral -> Printf.sprintf "<<<HSL\nsome string\nHSL;"
  | NowdocStringLiteral -> Printf.sprintf "<<<'NSL'\nsome string\nNSL;" *)
  | HeredocStringLiteral -> "\"heredoc\""
  | NowdocStringLiteral -> "\"nowdoc\""
  | DoubleQuotedStringLiteral -> "\"double string\""
  | SingleQuotedStringLiteral -> "'single string'"
  | XHPElementName -> "<" ^ (gen_name())
  | XHPStringLiteral -> "\"xhp string literal\""
  | XHPBody -> "just some body text" (*TODO better test coverage *)
  | XHPComment -> "<!-- some comments -->"
  | x -> TokenKind.to_string x
end
