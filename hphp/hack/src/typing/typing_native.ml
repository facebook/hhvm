(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SN = Naming_special_names

let is_native_fun ~env f =
  TypecheckerOptions.enable_systemlib_annotations
    (Provider_context.get_tcopt (Typing_env.get_ctx env))
  && Naming_attributes.mem SN.UserAttributes.uaNative f.Aast.f_user_attributes

let is_native_meth ~env m =
  TypecheckerOptions.enable_systemlib_annotations
    (Provider_context.get_tcopt (Typing_env.get_ctx env))
  && Naming_attributes.mem SN.UserAttributes.uaNative m.Aast.m_user_attributes
