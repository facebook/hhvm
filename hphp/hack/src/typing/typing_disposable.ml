(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Typing code concerned with disposable types. *)

open Core_kernel
open Typing_defs

module Env = Typing_env

let is_disposable_visitor env =
  object(this)
  inherit [string option] Type_visitor.type_visitor
  (* Only bother looking at classish types. Other types can spuriously
   * claim to implement these interfaces. Ideally we should check
   * constrained generics, abstract types, etc.
   *)
  method! on_tclass acc _ (_, class_name) tyl =
    let default () =
      List.fold_left tyl ~f:this#on_type ~init:acc in
    begin match Env.get_class env class_name with
    | None -> default ()
    | Some c ->
      if c.tc_is_disposable
      then Some (Utils.strip_ns class_name)
      else default ()
    end
  end

(* Does ty (or a type embedded in ty) implement IDisposable
 * or IAsyncDisposable, directly or indirectly?
 * Return Some class_name if it does, None if it doesn't.
 *)
let is_disposable_type env ty =
  match Env.expand_type env ty with
  | _env, ety ->
    (is_disposable_visitor env)#on_type None ety

let enforce_is_disposable env hint =
  match hint with
    | (_, Nast.Happly ((p, c), _)) ->
      begin match Decl_env.get_class_dep env.Env.decl_env c with
      | None -> ()
      | Some c ->
        if not c.Decl_defs.dc_is_disposable
        then Errors.must_extend_disposable p
      end
    | _ -> ()

(* Ensure that `ty` is a subtype of IDisposable (for `using`) or
 * IAsyncDisposable (for `await using`)
 *)
let enforce_is_disposable_type env has_await pos ty =
  let class_name =
    if has_await
    then SN.Classes.cIAsyncDisposable
    else SN.Classes.cIDisposable in
  let disposable_ty = (Reason.Rusing pos, Tclass ((pos, class_name), [])) in
  Typing_ops.sub_type pos Reason.URusing env ty disposable_ty
