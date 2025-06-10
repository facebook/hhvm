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
      match cc.cc_kind with
      | CCConcrete (_ty, pos, Class_const ((_cty, _cpos, cid_), (_, name)))
        when String.(name = SN.Members.mClass) ->
        (match Env.get_self_id env with
        | Some e when Env.is_enum env e ->
          ClassPointer.error_at_cls_const_expr
            (Env.tast_env_as_typing_env env)
            pos
            (ClassPointer.string_of_class_id_ cid_)
        | _ -> ())
      | _ -> ()

    method! at_expr env e =
      let env = Env.tast_env_as_typing_env env in
      let error = function
        | (_ty, pos, Class_const ((_cty, _cpos, cid_), (_, name)))
          when String.(name = SN.Members.mClass) ->
          ClassPointer.error_at_cls_const_expr
            env
            pos
            (ClassPointer.string_of_class_id_ cid_)
        | (ty, pos, _) ->
          let open Typing_defs in
          let ty_str = Typing_print.error env ty in
          (match get_node ty with
          | Tclass_ptr _
            when TypecheckerOptions.class_pointer_ban_class_array_key
                   env.genv.tcopt ->
            ClassPointer.error_at_cls_ptr_type env pos ty_str
          | _ -> ())
      in
      match e with
      (* $d[C::class] *)
      | (_, _, Array_get (_, Some expr)) -> error expr
      (* dict[C::class => ...] *)
      | (_, _, KeyValCollection (_, _, fields)) ->
        List.iter ~f:(fun (key, _val) -> error key) fields
      (* keyset[C::class] *)
      | (_, _, ValCollection ((_, (Set | ImmSet | Keyset)), _, exprs)) ->
        List.iter ~f:error exprs
      | _ -> ()
  end
