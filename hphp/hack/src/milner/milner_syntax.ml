(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type local = string

let next_local = ref 0

let fresh_local hint =
  let id = !next_local in
  incr next_local;
  Format.sprintf "$milner_%s_%d" hint id

type expr =
  | Atom of string
  | Local of local
  | New of string * expr list
  | Member of expr * string
  | StaticMember of string * string
  | Call of expr * expr list
  | Lambda of parameter list * string list * string * stmt list
  | Array of string * expr list
  | KeyValue of expr * expr
  | Tuple of expr list
  | Shape of (string * expr) list
  | Async of stmt list

and parameter = string * local

and stmt =
  | Bind of local * expr
  | Assign of expr * expr
  | Eval of expr
  | Return of expr option
  | Block of stmt list

let rec render_expr = function
  | Atom source -> source
  | Local local -> local
  | New (class_name, arguments) ->
    Format.sprintf "new %s(%s)" class_name (render_arguments arguments)
  | Member (receiver, name) ->
    Format.sprintf "(%s)->%s" (render_expr receiver) name
  | StaticMember (class_name, name) -> class_name ^ "::" ^ name
  | Call (callee, arguments) ->
    Format.sprintf "%s(%s)" (render_expr callee) (render_arguments arguments)
  | Array (kind, elements) -> kind ^ "[" ^ render_arguments elements ^ "]"
  | KeyValue (key, value) -> render_expr key ^ " => " ^ render_expr value
  | Tuple elements -> "tuple(" ^ render_arguments elements ^ ")"
  | Shape fields ->
    let fields =
      List.map fields ~f:(fun (key, value) -> key ^ " => " ^ render_expr value)
    in
    "shape(" ^ String.concat ~sep:", " fields ^ ")"
  | Async body -> "async { " ^ render_body body ^ " }"
  | Lambda (parameters, contexts, return_hint, body) ->
    let parameters =
      List.map parameters ~f:(fun (hint, local) -> hint ^ " " ^ local)
      |> String.concat ~sep:", "
    in
    Format.sprintf
      "((%s)[%s]: %s ==> { %s })"
      parameters
      (String.concat ~sep:", " contexts)
      return_hint
      (render_body body)

and render_arguments arguments =
  List.map arguments ~f:render_expr |> String.concat ~sep:", "

and render_stmt = function
  | Bind (local, value) -> local ^ " = " ^ render_expr value ^ ";"
  | Assign (target, value) ->
    render_expr target ^ " = " ^ render_expr value ^ ";"
  | Eval expression -> render_expr expression ^ ";"
  | Return None -> "return;"
  | Return (Some expression) -> "return " ^ render_expr expression ^ ";"
  | Block statements -> "{ " ^ render_body statements ^ " }"

and render_body statements =
  List.map statements ~f:render_stmt |> String.concat ~sep:" "
