(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module L = At_lex
module T = At_ty

(* This is NOT a full-featured AT parser, and its error recovery is generally
 * poor. See comment at the top of at_ty.ml for why we can afford to play fast
 * and loose. *)

exception Parse_error

let composite = function
  | [] -> raise Parse_error
  | [t] -> t
  | l -> T.ATcomposite (List.rev l)

let rec ty buf acc =
  match L.token buf with
    | L.Tstar -> T.ATvariadic (ty buf acc)
    | L.Tident -> ty buf (ty_atomic buf::acc)
    | L.Tor -> ty buf acc
    | L.Teof -> composite acc
    | L.Tlp -> begin
      let t = ty buf [] in
      if L.token buf <> L.Trp then raise Parse_error;
      let t = ty_maybe_brackets buf t in
      match t with
        | T.ATcomposite l -> ty buf (List.rev_append l acc)
        | _ -> ty buf (t::acc)
    end
    | L.Trp -> L.back buf; composite acc
    | L.Tbrackets
    | L.Terror -> raise Parse_error

and ty_atomic buf =
  let atom = match Lexing.lexeme buf with
    | "string"
    | "varchar" -> T.ATstring
    | "int"
    | "integer" -> T.ATint
    | "float"
    | "double"
    | "real" -> T.ATfloat
    | "bool"
    | "boolean" -> T.ATbool
    | "uint"
    | "unsigned_integer" -> T.ATuint
    | "numeric" -> T.ATnumeric
    | "callable" -> T.ATcallable
    | "array" -> T.ATanyarray
    | "null" -> T.ATnull
    | "void" -> T.ATvoid
    | "object" -> T.ATobject
    | "resource" -> T.ATresource
    | "mixed" -> T.ATmixed
    | c -> T.ATclass c in
  ty_maybe_brackets buf atom

and ty_maybe_brackets buf t =
  match L.token buf with
    | L.Tbrackets -> ty_maybe_brackets buf (T.ATarray t)
    | _ -> L.back buf; t

let parse s =
  let buf = Lexing.from_string s in
  ty buf []
