(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast

(** Is this [def] a class or function that contains this position? *)
let cls_or_fun_at_pos (pos : Pos.t) (def : ('a, 'b) Aast.def) : bool =
  match def with
  | Fun fd -> Pos.contains fd.fd_fun.f_span pos
  | Class c -> Pos.contains c.c_span pos
  | _ -> false

(** Find the method that contains [pos], if any. *)
let method_at_pos (methods : ('a, 'b) Aast.method_ list) (pos : Pos.t) :
    ('a, 'b) Aast.method_ option =
  List.find methods ~f:(fun m -> Pos.contains m.m_span pos)

(** Find the type parameter named [name]. *)
let tparam_with_name (params : ('a, 'b) Aast.tparam list) (name : string) :
    ('a, 'b) Aast.tparam option =
  List.find params ~f:(fun tp -> String.equal (snd tp.tp_name) name)

let sym_def_of_tparam (tp : ('a, 'b) Aast.tparam) :
    Relative_path.t SymbolDefinition.t =
  let (pos, name) = tp.tp_name in
  let name_with_constraints =
    String.strip (Format.asprintf "%a" Hint_print.pp_tparam tp)
  in
  let docblock =
    if String.equal name name_with_constraints then
      None
    else
      Some name_with_constraints
  in
  SymbolDefinition.
    {
      kind = SymbolDefinition.TypeVar;
      name;
      full_name = name;
      class_name = None;
      id = Some name;
      pos;
      span = pos;
      modifiers = [];
      children = None;
      params = None;
      docblock;
    }

(** Find the type param definition corresponding to the type variable
   [name] at [pos]. *)
let find_tparam_def
    (program : ('a, 'b) Aast.def list) (pos : Pos.t) (name : string) :
    ('a, 'b) Aast.tparam option =
  let def_at_pos = List.find program ~f:(cls_or_fun_at_pos pos) in
  match def_at_pos with
  | Some (Fun fd) -> tparam_with_name fd.fd_tparams name
  | Some (Class c) ->
    let class_tparam = tparam_with_name c.c_tparams name in
    (match class_tparam with
    | Some tp -> Some tp
    | None ->
      (* This type parameter didn't occur on the class, try the
         method. Type parameters on methods cannot shadow type
         parameters on classes, so we don't need to worry about
         clashes. *)
      (match method_at_pos c.c_methods pos with
      | Some m -> tparam_with_name m.m_tparams name
      | None -> None))
  | _ -> None

let go (program : ('a, 'b) Aast.def list) (pos : Pos.t) (name : string) :
    Relative_path.t SymbolDefinition.t option =
  Option.map (find_tparam_def program pos name) ~f:sym_def_of_tparam
