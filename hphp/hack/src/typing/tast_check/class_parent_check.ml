(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Tast

module Env = Tast_env
module Cls = Typing_classes_heap

let check_is_class env (p, h) =
  match h with
  | Aast.Happly ((_, name), _) ->
    begin match Env.get_class env name with
    | None -> ()
    | Some cls ->
      let kind = Cls.kind cls in
      let name = Cls.name cls in
      match kind with
      | Ast.(Cabstract | Cnormal) ->
        if Cls.final cls then Errors.requires_final_class p name
      | _ ->
        Errors.requires_non_class p name (Ast.string_of_class_kind kind)
    end
  | Aast.Habstr name ->
    Errors.requires_non_class p name "a generic"
  | _ ->
    Errors.requires_non_class p "This" "an invalid type hint"

let check_is_interface (env, error_verb) (p, h) =
  match h with
  | Aast.Happly ((_, name), _) ->
    begin match Env.get_class env name with
    | None -> ()
    | Some cls when Cls.kind cls = Ast.Cinterface -> ()
    | Some cls ->
      Errors.non_interface p (Cls.name cls) error_verb
    end
  | Aast.Habstr _ ->
    Errors.non_interface p "generic" error_verb
  | _ ->
    Errors.non_interface p "invalid type hint" error_verb

let check_is_trait env (p, h) =
  match h with
  | Aast.Happly ((_, name), _) ->
    let type_info = Env.get_class env name in
    begin match type_info with
    | None -> ()
    | Some cls when Cls.kind cls = Ast.Ctrait -> ()
    | Some cls ->
      let name = Cls.name cls in
      let kind = Cls.kind cls in
      Errors.uses_non_trait p name (Ast.string_of_class_kind kind)
    end
  | _ -> failwith "assertion failure: trait isn't an Happly"

let handler = object
  inherit Tast_visitor.handler_base

  method! at_class_ env c =
    List.iter c.c_uses (check_is_trait env);
    List.iter c.c_req_extends (check_is_class env);
    List.iter c.c_implements (check_is_interface (env, "implement"));
    List.iter c.c_req_implements
      (check_is_interface (env, "require implementation of"));

end
