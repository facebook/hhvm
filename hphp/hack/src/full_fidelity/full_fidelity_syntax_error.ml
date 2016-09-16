(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* TODO: Integrate these with the rest of the Hack error messages. *)

type t = {
  start_offset : int;
  end_offset : int;
  message : string
}

let make start_offset end_offset message =
  { start_offset; end_offset; message }

let to_string error =
  Printf.sprintf "(%d-%d) %s" error.start_offset error.end_offset error.message

let to_positioned_string error offset_to_position =
  let (sl, sc) = offset_to_position error.start_offset in
  let (el, ec) = offset_to_position error.end_offset in
  Printf.sprintf "(%d,%d)-(%d,%d) %s" sl sc el ec error.message

let compare err1 err2 =
  if err1.start_offset < err2.start_offset then -1
  else if err1.start_offset > err2.start_offset then 1
  else if err1.end_offset < err2.end_offset then -1
  else if err1.end_offset > err2.end_offset then 1
  else 0

(* Lexical errors *)
let error0001 = "A hexadecimal literal needs at least one digit."
let error0002 = "A binary literal needs at least one digit."
let error0003 = "A floating literal with an exponent needs at least " ^
                " one digit in the exponent."
let error0004 = "An octal literal must contain only digits 0 through 7."
let error0005 = "This string literal escape sequence is invalid."
let error0006 = "This character is invalid."
let error0007 = "This delimited comment is not terminated."
let error0008 = "A name is expected here."
let error0009 = "An unqualified name is expected here."
let error0010 = "A single quote is expected here."
let error0011 = "A newline is expected here."
let error0012 = "This string literal is not terminated."
let error0013 = "This XHP body is not terminated."
let error0014 = "This XHP comment is not terminated."

(* Syntactic errors *)
let error1001 = "A source file must begin with '<?hh'."
let error1002 = "An inclusion directive or type, function, " ^
                "namespace or use declaration is expected here."
let error1003 = "The 'function' keyword is expected here."
let error1004 = "A name is expected here."
let error1006 = "A right brace is expected here."
let error1007 = "A type specifier is expected here."
let error1008 = "A variable name is expected here."
let error1009 = "A comma or right parenthesis is expected here."
let error1010 = "A semicolon is expected here."
let error1011 = "A right parenthesis is expected here."
let error1012 = "A type is expected here."  (* TODO: Redundant to 1007. *)
let error1013 = "A closing angle bracket is expected here."
let error1014 = "A closing angle bracket or comma is expected here."
let error1015 = "An expression is expected here."
let error1016 = "An assignment is expected here."
let error1017 = "An XHP attribute value is expected here."
let error1018 = "The 'while' keyword is expected here"
let error1019 = "A left parenthesis is expected here."
let error1020 = "A colon is expected here."
let error1021 = "An opening angle bracket is expected here."
(* TODO: Remove this; redundant to 1009. *)
let error1022 = "A right parenthesis or comma is expected here."
let error1023 = "An 'as' keyword is expected here."
let error1024 = "A comma or semicolon is expected here."
let error1025 = "A shape field name is expected here."
let error1026 = "An opening square bracket is expected here."
let error1027 = "A class name, variable name, or 'static' is expected here."
let error1028 = "An arrow ('=>') is expected here."
let error1029 = "A closing double angle bracket is expected here."
let error1030 = "An attribute is expected here."
let error1031 = "A comma or a closing square bracket is expected here."
let error1032 = "A closing square bracket is expected here."
(* TODO: Break this up according to classish type *)
let error1033 = "A class member, method, type, trait usage, trait require, " ^
  "xhp attribute, xhp use, or xhp category is expected here."
let error1034 = "A left brace is expected here."
let error1035 = "The 'class' keyword is expected here."
let error1036 = "A '=' is expected here."
let error1037 = "A left brace is expected here."
let error1038 = "A namespace body is expected here."
let error1039 = "A closing XHP tag is expected here."
let error1040 = "A right brace or an enumerator is expected here."
let error1041 = "A function body or a semicolon is expected here."
let error1042 = "A visibility modifier, static, abstract, or final keyword is "^
                "expected here."
let error1043 = "A function header is expected here."
let error1044 = "A name, __construct, or __destruct keyword is expected here."
let error1045 = "An 'extends' or 'implements' keyword is expected here."
let error1046 = "A lambda arrow ('==>') is expected here."
let error1047 = "A scope resolution operator ('::') is expected here."
let error1048 = "A name, variable name or 'class' is expected here."
let error1050 = "A name or variable name is expected here."
let error1051 = "The 'required' keyword is expected here."
let error1052 = "An XHP category name beginning with a '%' is expected here."
let error1053 = "An XHP name or category name is expected here."

let error2001 = "A type annotation is required in strict mode."
let error2002 = "An XHP attribute name may not contain '-' or ':'."
let error2003 = "A case statement may only appear directly inside a switch."
let error2004 = "A default statement may only appear directly inside a switch."
let error2005 = "A break statement may only appear inside a switch or loop."
let error2006 = "A continue statement may only appear inside a loop."
let error2007 = "A try statement requires a catch or a finally clause."
let error2008 = "The first statement inside a switch statement must " ^
  "be a case or default label statement."
let error2009 = "A constructor cannot be static."
let error2010 = "Parameters cannot have visibility modifiers (except in " ^
  "parameter lists of constructors)."
let error2011 = "A destructor must have an empty parameter list."
let error2012 = "A destructor can only have visibility modifiers."
let error2013 = "A method declaration cannot have duplicate modifiers."
let error2014 = "An abstract method cannot have a method body."
let error2015 = "A non-abstract method must have a body."
let error2016 = "A method cannot be both abstract and private."
let error2017 =
  "A method declaration cannot have multiple visibility modifiers."
let error2018 =
  "A constructor or destructor cannot have a non-void type annotation."
let error2019 = "A method cannot be both abstract and final."
let error2020 = "Use of the '{}' subscript operator is deprecated; " ^
  " use '[]' instead."
let error2021 = "An ellipsis '...' may only appear at the end of a " ^
  "parameter list."
let error2022 = "An ellipsis '...' may not be followed by a comma ','."

let error2029 = "Only traits and interfaces may use 'require extends'."
let error2030 = "Only traits may use 'require implements'."
let error2031 =
  "A class, interface, or trait declaration cannot have duplicate modifiers."
let error2032 = "The array type is not allowed in strict mode."
let error2033 = "Variadic parameter or argument must be the last in a" ^
  " parameter or argument list."
