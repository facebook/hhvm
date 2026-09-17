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
  | EnumLabel of string * string
  | StaticProperty of string * string
  | Call of expr * expr list
  | Index of expr * expr
  | Inout of expr
  | Unpack of expr
  | Lambda of parameter list * string list * string * stmt list
  | Array of string * expr list
  | KeyValue of expr * expr
  | Tuple of expr list
  | Shape of (string * expr) list
  | Async of stmt list

and parameter = {
  hint: string;
  local: local;
  variadic: bool;
  default: expr option;
}

and stmt =
  | Bind of local * expr
  | Assign of expr * expr
  | Eval of expr
  | Return of expr option
  | Throw of expr
  | Block of stmt list

let parameter ?(variadic = false) ?default hint local =
  { hint; local; variadic; default }

let rec render_expr = function
  | Index (receiver, index) ->
    "(" ^ render_expr receiver ^ ")[" ^ render_expr index ^ "]"
  | Inout expression -> "inout " ^ render_expr expression
  | Unpack expression -> "..." ^ render_expr expression
  | Atom source -> source
  | Local local -> local
  | New (class_name, arguments) ->
    Format.sprintf "new %s(%s)" class_name (render_arguments arguments)
  | Member (receiver, name) ->
    Format.sprintf "(%s)->%s" (render_expr receiver) name
  | StaticProperty (class_name, name) -> class_name ^ "::$" ^ name
  | EnumLabel (enum_name, member) -> enum_name ^ "#" ^ member
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
      List.map parameters ~f:(fun { hint; local; variadic; default } ->
          hint
          ^ " "
          ^ (if variadic then
              "..."
            else
              "")
          ^ local
          ^ Option.value_map default ~default:"" ~f:(fun expression ->
                " = " ^ render_expr expression))
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
  | Throw expression -> "throw " ^ render_expr expression ^ ";"
  | Return None -> "return;"
  | Return (Some expression) -> "return " ^ render_expr expression ^ ";"
  | Block statements -> "{ " ^ render_body statements ^ " }"

and render_body statements =
  List.map statements ~f:render_stmt |> String.concat ~sep:" "
