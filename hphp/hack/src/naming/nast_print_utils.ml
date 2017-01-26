(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module N = Nast
module A = Ast

let tab_length = 2

let string_of_filemode mode =
  Sexp.to_string_hum @@ Sof.sexp_of_mode mode

let sexp_of_tprim = function
  | N.Tvoid     -> Sexp.Atom "Tvoid"
  | N.Tint      -> Sexp.Atom "Tint"
  | N.Tbool     -> Sexp.Atom "Tbool"
  | N.Tfloat    -> Sexp.Atom "Tfloat"
  | N.Tstring   -> Sexp.Atom "Tstring"
  | N.Tresource -> Sexp.Atom "Tresource"
  | N.Tnum      -> Sexp.Atom "Tnum"
  | N.Tarraykey -> Sexp.Atom "Tarraykey"
  | N.Tnoreturn -> Sexp.Atom "Tnoreturn"

let sexp_of_shapemap f s = Sexp.Atom "shapemap"

let sexp_of_sid : Nast.sid -> Sexp.t =
  fun (_, s) -> Sexp.List [ Sexp.Atom "Nast.sid"; Sexp.Atom s ]

let rec sexp_of_hint : N.hint -> Sexp.t =
  (**
   * TODO (hgo): sexp pos.t as well *)
  fun h -> sexp_of_hint_ @@ snd h

and sexp_of_hint_ : N.hint_ -> Sexp.t =
  fun h ->
    match h with
    | N.Hany      -> Sexp.Atom "Hany"
    | N.Hmixed    -> Sexp.Atom "Hmixed"
    | N.Htuple hs -> Sexp.List
      [ Sexp.Atom "Htuple"
      ; Conv.sexp_of_list sexp_of_hint hs
      ]
    | N.Habstr s  -> Sexp.List
      [ Sexp.Atom "Habstr"
      ; Sexp.Atom s
      ]
    | N.Harray (h1, h2) ->
      let default = Sexp.Atom "<no hint>" in
      Sexp.List
        [ Sexp.Atom "Harray"
        ; Option.value_map h1 ~default ~f:sexp_of_hint
        ; Option.value_map h2 ~default ~f:sexp_of_hint
        ]
    | N.Hprim p -> Sexp.List
      [ Sexp.Atom "Hprim"
      ; sexp_of_tprim p
      ]
    | N.Hoption h -> Sexp.List
      [ Sexp.Atom "Hoption"
      ; sexp_of_hint h
      ]
    | N.Hfun (hs, b, h) -> Sexp.List
      [ Sexp.Atom "Hfun"
      ; Conv.sexp_of_list sexp_of_hint hs
      ; Conv.sexp_of_bool b
      ; sexp_of_hint h
      ]
    | N.Happly (sid, hs) -> Sexp.List
      [ Sexp.Atom "Happly"
      ; sexp_of_sid sid
      ; Conv.sexp_of_list sexp_of_hint hs
      ]
    | N.Hshape hsm -> Sexp.List
      [ Sexp.Atom "Hshape"
      ; sexp_of_shapemap sexp_of_hint hsm
      ]
    | N.Hthis -> Sexp.Atom "Hthis"
    | N.Haccess (h, ss) -> Sexp.List
      [ Sexp.Atom "Haccess"
      ; sexp_of_hint h
      ; Conv.sexp_of_list sexp_of_sid ss
      ]

let sexp_of_fun_param _ = Sexp.Atom "<fun_param>"

let sexp_of_fun_variadicity : Nast.fun_variadicity -> Sexp.t =
  function
    | N.FVvariadicArg fp -> Sexp.List
      [ Sexp.Atom "FVvariadicArg"
      ; sexp_of_fun_param fp
      ]
    | N.FVellipsis -> Sexp.Atom "FVellipsis"
    | N.FVnonVariadic -> Sexp.Atom "FVnonVariadic"

let rec string_of_hint_list : int -> N.hint list -> string =
  fun sprefix hs ->
    let f_fold acc h = acc ^ "\n" ^ (string_of_hint h) in
    fold_hint_to_string sprefix f_fold hs

and string_of_hint : N.hint -> string =
  fun h -> string_of_hint_ @@ snd h

and fold_hint_to_string
  : int -> (string -> N.hint -> string) -> N.hint list -> string =
  fun p f h ->
    match h with
    | [] -> (String.make p ' ') ^ "<no hint>"
    | x::xs -> List.fold_left f (string_of_hint x) xs

and string_of_hint_ h_ =
  Sexp.to_string_hum @@ sexp_of_hint_ h_

let string_of_constraint_kind = function
  | A.Constraint_as     -> "Constraint_as"
  | A.Constraint_eq     -> "Constraint_eq"
  | A.Constraint_super  -> "Constraint_super"

let string_of_variance = function
  | A.Covariant     -> "Covariant"
  | A.Contravariant -> "Contravariant"
  | A.Invariant     -> "Invariant"

let string_of_tparam : int -> N.tparam -> string =
  fun sprefix tp ->
    let stringify (constraint_kind, hint): string =
      (string_of_constraint_kind constraint_kind) ^ " "
      ^ (string_of_hint hint) in
    let variance, sid, c_hints = tp in
    string_of_variance variance
    ^ snd sid
    ^ String.concat " " @@ List.map stringify c_hints

let string_of_tparam_list : int -> Nast.tparam list -> string =
  fun sprefix l ->
    let f_map x = string_of_tparam sprefix x in
    match l with
    | [] -> (String.make sprefix ' ') ^ "<no tparam>"
    | xs -> String.concat ", " @@ List.map f_map xs

let string_of_tprim tprim =
  Sexp.to_string_hum @@ sexp_of_tprim tprim

(**
 * TODO (hgo): implement it for real... *)
let string_of_hint_shapemap _ = "I'm a shapemap."
let string_of_expr _ _ = "I'm an expression."

let string_of_fun_variadicity fv =
  Sexp.to_string_hum @@ sexp_of_fun_variadicity fv

let string_of_fun_ : int -> Nast.fun_ -> string =
  fun sprefix f ->
    let p = String.make sprefix ' ' in
    let header_DEBUG = " -- string_of_fun_ -- \n" in
    let footer_DEBUG = " -------------------- \n" in
    let open Printf in
    let open Nast in
    let f_mode = "file mode: " ^ string_of_filemode f.f_mode  ^ "\n" in
    let f_ret =
      "file ret: "
      ^ (Option.value_map ~default:"<no f_ret>" ~f:string_of_hint f.f_ret)
      ^ "\n" in
    let f_name = "file name: " ^ (snd f.f_name) ^ "\n" in
    let f_tparams =
      "file parameters: "
      ^ (string_of_tparam_list (sprefix + tab_length) f.f_tparams) ^ "\n" in
    let f_variadic =
      "file variadicity: "
      ^ (string_of_fun_variadicity f.f_variadic)
      ^ "\n" in
    header_DEBUG
    ^ p ^ f_mode
    ^ p ^ f_name
    ^ p ^ f_ret
    ^ p ^ f_tparams
    ^ p ^ f_variadic
    ^ footer_DEBUG
