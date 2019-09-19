(*
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
module MakeType = Typing_make_type
module Cls = Decl_provider.Class

let is_disposable_visitor env =
  object (this)
    inherit [string option] Type_visitor.locl_type_visitor

    (* Only bother looking at classish types. Other types can spuriously
     * claim to implement these interfaces. Ideally we should check
     * constrained generics, abstract types, etc.
     *)
    method! on_tclass acc _ (_, class_name) _ tyl =
      let default () = List.fold_left tyl ~f:this#on_type ~init:acc in
      match Env.get_class env class_name with
      | None -> default ()
      | Some c ->
        if Cls.is_disposable c then
          Some (Utils.strip_ns class_name)
        else
          default ()
  end

(* Does ty (or a type embedded in ty) implement IDisposable
 * or IAsyncDisposable, directly or indirectly?
 * Return Some class_name if it does, None if it doesn't.
 *)
let is_disposable_type env ty =
  match Env.expand_type env ty with
  | (_env, ety) -> (is_disposable_visitor env)#on_type None ety

let enforce_is_disposable env hint =
  match hint with
  | (_, Aast.Happly ((p, c), _)) ->
    begin
      match Env.get_class_dep env c with
      | None -> ()
      | Some c ->
        if not (Cls.is_disposable c) then Errors.must_extend_disposable p
    end
  | _ -> ()

(* Ensure that `ty` is a subtype of IDisposable (for `using`) or
 * IAsyncDisposable (for `await using`)
 *)
let enforce_is_disposable_type env has_await pos ty =
  let class_name =
    if has_await then
      SN.Classes.cIAsyncDisposable
    else
      SN.Classes.cIDisposable
  in
  let disposable_ty = MakeType.class_type (Reason.Rusing pos) class_name [] in
  Typing_ops.sub_type
    pos
    Reason.URusing
    env
    ty
    disposable_ty
    Errors.unify_error
