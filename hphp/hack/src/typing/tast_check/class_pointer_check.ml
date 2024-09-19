(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module Env = Tast_env
module SN = Naming_special_names
module ClassPointer = Typing_class_pointers

(** A place to consolidate some static class pointer checks before inference
 * takes care of them all. *)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_const env cc =
      let enum_level =
        TypecheckerOptions.class_pointer_level (Env.get_tcopt env) "enum"
      in
      match cc.cc_kind with
      | CCConcrete (_ty, pos, Class_const ((_cty, _cpos, cid_), (_, name)))
        when enum_level > 0 && String.(name = SN.Members.mClass) ->
        (match Env.get_self_id env with
        | Some e when Env.is_enum env e ->
          ClassPointer.error
            (Env.tast_env_as_typing_env env)
            enum_level
            pos
            (ClassPointer.string_of_class_id_ cid_)
        | _ -> ())
      | _ -> ()

    method! at_expr env e =
      let arraykey_level =
        TypecheckerOptions.class_pointer_level (Env.get_tcopt env) "arraykey"
      in
      let error = function
        | (_ty, pos, Class_const ((_cty, _cpos, cid_), (_, name)))
          when String.(name = SN.Members.mClass) ->
          ClassPointer.error
            (Env.tast_env_as_typing_env env)
            arraykey_level
            pos
            (ClassPointer.string_of_class_id_ cid_)
        | _ -> ()
      in
      match e with
      (* $d[C::class] *)
      | (_, _, Array_get (_, Some expr)) when arraykey_level > 0 -> error expr
      (* dict[C::class => ...] *)
      | (_, _, KeyValCollection (_, _, fields)) when arraykey_level > 0 ->
        List.iter ~f:(fun (key, _val) -> error key) fields
      (* keyset[C::class] *)
      | (_, _, ValCollection ((_, (Set | ImmSet | Keyset)), _, exprs))
        when arraykey_level > 0 ->
        List.iter ~f:error exprs
      | _ -> ()
  end
