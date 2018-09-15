open Printf
open Lexing

(* unused: module String_set = Set.Make (String) *)
(* unused: module String_map = Map.Make (String) *)

type loc = position * position

type bool_expr =
    [ `True
    | `False
    | `Defined of string
    | `Not of bool_expr (* not *)
    | `And of (bool_expr * bool_expr) (* && *)
    | `Or of (bool_expr * bool_expr) (* || *)
    | `Eq of (arith_expr * arith_expr) (* = *)
    | `Lt of (arith_expr * arith_expr) (* < *)
    | `Gt of (arith_expr * arith_expr) (* > *)
        (* syntax for additional operators: <>, <=, >= *)
    ]

and arith_expr = (* signed int64 *)
    [ `Int of int64
    | `Ident of (loc * string)
        (* must be bound to a valid int literal.
           Expansion of macro functions is not supported. *)

    | `Tuple of (loc * arith_expr list)
        (* tuple of 2 or more elements guaranteed by the syntax *)

    | `Neg of arith_expr (* - *)
    | `Add of (arith_expr * arith_expr) (* + *)
    | `Sub of (arith_expr * arith_expr) (* - *)
    | `Mul of (arith_expr * arith_expr) (* * *)
    | `Div of (loc * arith_expr * arith_expr) (* / *)
    | `Mod of (loc * arith_expr * arith_expr) (* mod *)

    (* Bitwise operations on 64 bits *)
    | `Lnot of arith_expr (* lnot *)
    | `Lsl of (arith_expr * arith_expr) (* lsl *)
    | `Lsr of (arith_expr * arith_expr) (* lsr *)
    | `Asr of (arith_expr * arith_expr) (* asr *)
    | `Land of (arith_expr * arith_expr) (* land *)
    | `Lor of (arith_expr * arith_expr) (* lor *)
    | `Lxor of (arith_expr * arith_expr) (* lxor *)
    ]

and node =
    [ `Ident of (loc * string * node list list option)
    | `Def of (loc * string * node list)
    | `Defun of (loc * string * string list * node list)
    | `Undef of (loc * string)
    | `Include of (loc * string)
    | `Ext of (loc * string * string)
    | `Cond of (loc * bool_expr * node list * node list)
    | `Error of (loc * string)
    | `Warning of (loc * string)
    | `Text of (loc * bool * string) (* bool is true for space tokens *)
    | `Seq of node list
    | `Stringify of node
    | `Capitalize of node
    | `Concat of (node * node)
    | `Line of (loc * string option * int)
    | `Current_line of loc
    | `Current_file of loc ]



let string_of_loc (pos1, pos2) =
  let line1 = pos1.pos_lnum
  and start1 = pos1.pos_bol in
  Printf.sprintf "File %S, line %i, characters %i-%i"
    pos1.pos_fname line1
    (pos1.pos_cnum - start1)
    (pos2.pos_cnum - start1)


exception Cppo_error of string

let error loc s =
  let msg =
    sprintf "%s\nError: %s" (string_of_loc loc) s in
  raise (Cppo_error msg)

let warning loc s =
  let msg =
    sprintf "%s\nWarning: %s" (string_of_loc loc) s in
  eprintf "%s\n%!" msg

let dummy_loc = (Lexing.dummy_pos, Lexing.dummy_pos)

let rec flatten_nodes (l: node list): node list =
  List.flatten (List.map flatten_node l)

and flatten_node (node: node): node list =
  match node with
  | `Seq l -> flatten_nodes l
  | x -> [x]
