(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
module Env = Typing_env
module SN = Naming_special_names
module UA = SN.UserAttributes
module Cls = Decl_provider.Class
module Nast = Aast

let validator =
  object (this)
    inherit Type_validator.type_validator as super

    method! on_tapply acc r (p, h) tyl =
      if String.equal h SN.Classes.cTypename then
        this#invalid acc r "a typename"
      else if String.equal h SN.Classes.cClassname then
        this#invalid acc r "a classname"
      else
        super#on_tapply acc r (p, h) tyl

    method! on_twildcard acc r =
      if acc.Type_validator.env.Typing_env_types.allow_wildcards then
        acc
      else
        this#invalid acc r "a wildcard"

    method! on_tgeneric acc r t _tyargs =
      (* Ignoring type aguments: If there were any, then this generic variable isn't allowed to be
         reified anyway *)
      (* TODO(T70069116) actually implement that check *)
      match Env.get_reified acc.Type_validator.env t with
      | Nast.Erased -> this#invalid acc r "not reified"
      | Nast.SoftReified -> this#invalid acc r "soft reified"
      | Nast.Reified -> acc

    method! on_tfun acc r _ = this#invalid acc r "a function type"

    method! on_typeconst acc class_ typeconst =
      match typeconst.ttc_kind with
      | TCConcrete _ -> super#on_typeconst acc class_ typeconst
      | TCAbstract _ when Option.is_some typeconst.ttc_reifiable ->
        super#on_typeconst acc class_ typeconst
      | TCAbstract _ ->
        let r = Reason.Rwitness_from_decl (fst typeconst.ttc_name) in
        let kind =
          "an abstract type constant without the __Reifiable attribute"
        in
        this#invalid acc r kind

    method! on_taccess acc r (root, ids) =
      let acc =
        match acc.Type_validator.reification with
        | Type_validator.Unresolved ->
          (match get_node root with
          | Tthis ->
            { acc with Type_validator.reification = Type_validator.Resolved }
          | _ -> this#on_type acc root)
        | Type_validator.Resolved -> acc
      in
      super#on_taccess acc r (root, ids)

    method! on_tthis acc r =
      this#invalid acc r "the late static bound this type"

    method! on_trefinement acc r _ty _ = this#invalid acc r "type refinement"
  end
