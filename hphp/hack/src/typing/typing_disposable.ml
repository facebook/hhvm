(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Typing code concerned with disposable types. *)

open Hh_prelude
module Env = Typing_env
module Cls = Decl_provider.Class

let is_disposable_class env cls =
  let name = Cls.name cls in
  String.equal name Naming_special_names.Classes.cIDisposable
  || String.equal name Naming_special_names.Classes.cIAsyncDisposable
  || Typing_utils.has_ancestor_including_req
       env
       cls
       Naming_special_names.Classes.cIDisposable
  || Typing_utils.has_ancestor_including_req
       env
       cls
       Naming_special_names.Classes.cIAsyncDisposable

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
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        default ()
      | Decl_entry.Found c ->
        if is_disposable_class env c then
          Some (Utils.strip_ns class_name)
        else
          default ()
  end

let is_disposable_type env ty =
  match Env.expand_type env ty with
  | (_env, ety) -> (is_disposable_visitor env)#on_type None ety

let enforce_is_disposable env hint =
  match hint with
  | (_, Aast.Happly ((p, c), _)) -> begin
    match Env.get_class env c with
    | Decl_entry.DoesNotExist
    | Decl_entry.NotYetAvailable ->
      ()
    | Decl_entry.Found c ->
      if not (is_disposable_class env c || Ast_defs.is_c_interface (Cls.kind c))
      then
        Typing_error_utils.add_typing_error
          ~env
          Typing_error.(primary @@ Primary.Must_extend_disposable p)
  end
  | _ -> ()
