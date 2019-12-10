(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
module SN = Naming_special_names

let handler =
  object
    inherit Nast_visitor.handler_base

    method! at_method_ _ m =
      let (pos, _) = m.m_name in
      let vis = m.m_visibility in
      let attr = m.m_user_attributes in
      match
        Naming_attributes.mem_pos SN.UserAttributes.uaDynamicallyCallable attr
      with
      | Some p when not (Aast.equal_visibility vis Public) ->
        Errors.illegal_use_of_dynamically_callable
          p
          pos
          (string_of_visibility vis)
      | _ -> ()
  end
