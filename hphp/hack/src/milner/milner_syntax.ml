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

let named_local name = "$" ^ name

let local_name local = String.drop_prefix local 1

type expr =
  | Unary of string * expr
  | Binary of string * expr * expr
  | As of expr * string
  | NullsafeMember of expr * string
  | AsyncLambda of parameter list * string list * string * stmt list
  | Await of expr
  | Xhp of string * (string * expr) list * expr list
  | Quote of string * expr
  | Splice of expr
  | Atom of string
  | Local of local
  | New of string * expr list
  | NewDynamic of local * expr list
  | DynamicStaticMember of local * string
  | Is of expr * string
  | Nameof of string
  | Member of expr * string
  | StaticMember of string * string
  | EnumLabel of string * string
  | StaticProperty of string * string
  | Call of expr * expr list
  | NamedArgument of string * expr
  | Index of expr * expr
  | Append of expr
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
  named: bool;
  default: expr option;
}

and stmt =
  | If of expr * stmt list * stmt list
  | While of expr * stmt list
  | Foreach of expr * local option * local * stmt list
  | Try of stmt list * (string * local * stmt list) list * stmt list
  | Concurrent of stmt list
  | Bind of local * expr
  | Assign of expr * expr
  | Eval of expr
  | Return of expr option
  | Throw of expr
  | Block of stmt list

let parameter ?(variadic = false) ?(named = false) ?default hint local =
  { hint; local; variadic; named; default }

let rec render_expr = function
  | Quote (visitor, body) -> visitor ^ "`" ^ render_expr body ^ "`"
  | Splice expression -> "${" ^ render_expr expression ^ "}"
  | Xhp (name, attributes, children) ->
    let attributes =
      List.map attributes ~f:(fun (name, value) ->
          " " ^ name ^ "={" ^ render_expr value ^ "}")
      |> String.concat ~sep:""
    in
    let children =
      List.map children ~f:(fun child -> "{" ^ render_expr child ^ "}")
      |> String.concat ~sep:""
    in
    "<" ^ name ^ attributes ^ ">" ^ children ^ "</" ^ name ^ ">"
  | Unary (operator, expression) ->
    "(" ^ operator ^ render_expr expression ^ ")"
  | Binary (operator, left, right) ->
    "(" ^ render_expr left ^ " " ^ operator ^ " " ^ render_expr right ^ ")"
  | As (expression, hint) -> "(" ^ render_expr expression ^ " as " ^ hint ^ ")"
  | NullsafeMember (receiver, name) ->
    "(" ^ render_expr receiver ^ ")?->" ^ name
  | Index (receiver, index) ->
    "(" ^ render_expr receiver ^ ")[" ^ render_expr index ^ "]"
  | Append receiver -> "(" ^ render_expr receiver ^ ")[]"
  | Inout expression -> "inout " ^ render_expr expression
  | Unpack expression -> "..." ^ render_expr expression
  | Atom source -> source
  | Local local -> local
  | New (class_name, arguments) ->
    Format.sprintf "new %s(%s)" class_name (render_arguments arguments)
  | NewDynamic (class_local, arguments) ->
    Format.sprintf "new %s(%s)" class_local (render_arguments arguments)
  | DynamicStaticMember (class_local, member) -> class_local ^ "::" ^ member
  | Is (value, hint) -> Format.sprintf "(%s is %s)" (render_expr value) hint
  | Nameof class_name -> "nameof " ^ class_name
  | Member (receiver, name) ->
    Format.sprintf "(%s)->%s" (render_expr receiver) name
  | StaticProperty (class_name, name) -> class_name ^ "::$" ^ name
  | EnumLabel (enum_name, member) -> enum_name ^ "#" ^ member
  | StaticMember (class_name, name) -> class_name ^ "::" ^ name
  | Call (callee, arguments) ->
    Format.sprintf "%s(%s)" (render_expr callee) (render_arguments arguments)
  | NamedArgument (name, expression) -> name ^ "=" ^ render_expr expression
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
    render_lambda "" parameters contexts return_hint body
  | AsyncLambda (parameters, contexts, return_hint, body) ->
    render_lambda "async " parameters contexts return_hint body
  | Await expression -> "(await " ^ render_expr expression ^ ")"

and render_parameters ?(canonical = true) parameters =
  (* T289079831: runtime type-structure checks require declarations to follow
     the sorted parameter slots. *)
  let parameters =
    if canonical then
      let (named, positional) =
        List.partition_tf parameters ~f:(fun parameter -> parameter.named)
      in
      List.sort named ~compare:(fun left right ->
          String.compare left.local right.local)
      @ positional
    else
      parameters
  in
  List.map parameters ~f:(fun { hint; local; variadic; named; default } ->
      (if named then
        "named "
      else
        "")
      ^ hint
      ^ " "
      ^ (if variadic then
          "..."
        else
          "")
      ^ local
      ^ Option.value_map default ~default:"" ~f:(fun expression ->
            " = " ^ render_expr expression))
  |> String.concat ~sep:", "

and render_lambda prefix parameters contexts return_hint body =
  Format.sprintf
    "(%s(%s)[%s]: %s ==> { %s })"
    prefix
    (render_parameters parameters)
    (String.concat ~sep:", " contexts)
    return_hint
    (render_body body)

and render_arguments arguments =
  List.map arguments ~f:render_expr |> String.concat ~sep:", "

and render_stmt = function
  | Concurrent statements -> "concurrent { " ^ render_body statements ^ " }"
  | If (condition, consequent, alternative) ->
    "if ("
    ^ render_expr condition
    ^ ") { "
    ^ render_body consequent
    ^ " }"
    ^
    if List.is_empty alternative then
      ""
    else
      " else { " ^ render_body alternative ^ " }"
  | While (condition, body) ->
    "while (" ^ render_expr condition ^ ") { " ^ render_body body ^ " }"
  | Foreach (collection, key, local, body) ->
    "foreach ("
    ^ render_expr collection
    ^ " as "
    ^ Option.value_map key ~default:"" ~f:(fun key -> key ^ " => ")
    ^ local
    ^ ") { "
    ^ render_body body
    ^ " }"
  | Try (body, catches, finally) ->
    let catches =
      List.map catches ~f:(fun (hint, local, body) ->
          " catch (" ^ hint ^ " " ^ local ^ ") { " ^ render_body body ^ " }")
      |> String.concat ~sep:""
    in
    "try { "
    ^ render_body body
    ^ " }"
    ^ catches
    ^
    if List.is_empty finally then
      ""
    else
      " finally { " ^ render_body finally ^ " }"
  | Bind (local, value) -> local ^ " = " ^ render_expr value ^ ";"
  | Assign (target, value) ->
    render_expr target ^ " = " ^ render_expr value ^ ";"
  | Eval (Unary ("++", target)) -> "++" ^ render_expr target ^ ";"
  | Eval expression -> render_expr expression ^ ";"
  | Throw expression -> "throw " ^ render_expr expression ^ ";"
  | Return None -> "return;"
  | Return (Some expression) -> "return " ^ render_expr expression ^ ";"
  | Block statements -> "{ " ^ render_body statements ^ " }"

and render_body statements =
  List.map statements ~f:render_stmt |> String.concat ~sep:" "
