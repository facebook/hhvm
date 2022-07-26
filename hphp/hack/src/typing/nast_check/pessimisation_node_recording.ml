(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Nast_check_env

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_fun_ env fun_ =
      let ctx = env.ctx in
      let deps_mode = Provider_context.get_deps_mode ctx in
      let name = fun_.Aast.f_name |> Ast_defs.get_id in
      let node = Typing_deps.Dep.Fun name in
      Typing_pessimisation_deps.add_node deps_mode node None

    method! at_method_ env method_ =
      let class_ = env.class_name |> Option.value_exn in
      let name = method_.Aast.m_name |> Ast_defs.get_id in

      let static = method_.Aast.m_static in
      let abstract = method_.Aast.m_abstract in
      let external_ = method_.Aast.m_external in
      if
        (not abstract)
        && (not external_)
        && not String.(name = Naming_special_names.Members.__construct)
      then
        let ctx = env.ctx in
        let deps_mode = Provider_context.get_deps_mode ctx in
        let class_dep = Typing_deps.Dep.Type class_ in
        let member =
          if static then
            Typing_pessimisation_deps.SMethod name
          else
            Typing_pessimisation_deps.Method name
        in
        Typing_pessimisation_deps.add_node deps_mode class_dep (Some member)
  end
