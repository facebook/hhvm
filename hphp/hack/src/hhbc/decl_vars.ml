(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open Core

module ULS = Unique_list_string

let add_local bareparam (needs_local_this, locals) (_, name) =
  if name = "$GLOBALS"
  then needs_local_this, locals
  else
  if name = "$this"
  then (bareparam || needs_local_this), ULS.add locals name
  else needs_local_this, ULS.add locals name

class declvar_visitor = object(this)
  inherit [bool * ULS.t] Ast_visitor.ast_visitor as _super

  method! on_global_var acc exprs =
    List.fold_left exprs ~init:acc
      ~f:(fun acc (_, e) ->
        match e with Ast.Id id -> add_local false acc id
                  | _ -> acc)
  method! on_foreach acc e pos iterator block =
    let acc =
      match snd e with
      | Ast.Lvar(_, "$this" as id) when Iterator.is_mutable_iterator iterator ->
        add_local true acc id
      | _ ->
        acc
    in
    _super#on_foreach acc e pos iterator block

  method! on_lvar acc id = add_local false acc id
  method! on_lvarvar acc _ id = add_local false acc id
  method! on_class_get acc id _ =
  (* Only add if it is a variable *)
  if String_utils.string_starts_with (snd id) "$"
  then add_local false acc id
  else acc

  method! on_efun acc _fn use_list =
    List.fold_left use_list ~init:acc
      ~f:(fun acc (x, _isref) -> add_local false acc x)
  method! on_call acc e el1 el2 =
    let call_isset =
      match e with (_, Ast.Id(_, "isset")) -> true | _ -> false in
    let on_arg acc e =
      match e with
      (* Only add $this to locals if it's bare *)
      | (_, Ast.Lvar(_, "$this" as id)) ->
       add_local (not call_isset) acc id
      | _ ->
        this#on_expr acc e
    in
    let acc = this#on_expr acc e in
    let acc = List.fold_left el1 ~f:on_arg ~init:acc in
    let acc = List.fold_left el2 ~f:on_arg ~init:acc in
    acc

  method! on_catch acc (_, x, b) =
    this#on_block (add_local false acc x) b
  method! on_class_ acc _ = acc
  method! on_fun_ acc _ = acc
end

(* See decl_vars.mli for details *)
let from_ast ~is_closure_body ~has_this ~params b =
  let visitor = new declvar_visitor in
  let needs_local_this, decl_vars = visitor#on_program (false, ULS.empty) b in
  let param_names =
    List.fold_left
      params
        ~init:ULS.empty
        ~f:(fun l p -> ULS.add l @@ Hhas_param.name p)
  in
  let decl_vars = ULS.diff decl_vars param_names in
  let decl_vars =
    if needs_local_this || is_closure_body || not has_this
    then decl_vars
    else ULS.remove "$this" decl_vars in
  needs_local_this && has_this, List.rev (ULS.items decl_vars)
