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
open Type_validator
module Env = Tast_env
module SN = Naming_special_names
module UA = SN.UserAttributes
module Cls = Decl_provider.Class
module Nast = Aast

let validator =
  object (this)
    inherit type_validator as super

    method! on_tapply acc r (p, h) tyl =
      if String.equal h SN.Classes.cTypename then
        this#invalid acc r "a typename"
      else if String.equal h SN.Classes.cClassname then
        this#invalid acc r "a classname"
      else if
        String.equal h SN.Typehints.wildcard
        && not (Env.get_allow_wildcards acc.env)
      then
        this#invalid acc r "a wildcard"
      else
        super#on_tapply acc r (p, h) tyl

    method! on_tgeneric acc r t _tyargs =
      (* Ignoring type aguments: If there were any, then this generic variable isn't allowed to be
        reified anyway *)
      (* TODO(T70069116) actually implement that check *)
      match Env.get_reified acc.env t with
      | Nast.Erased -> this#invalid acc r "not reified"
      | Nast.SoftReified -> this#invalid acc r "soft reified"
      | Nast.Reified -> acc

    method! on_tvarray acc r _ = this#invalid acc r "an array type"

    method! on_tdarray acc r _ _ = this#invalid acc r "an array type"

    method! on_tvarray_or_darray acc r _ _ = this#invalid acc r "an array type"

    method! on_tfun acc r _ = this#invalid acc r "a function type"

    method! on_typeconst acc class_ is_concrete typeconst =
      match typeconst.ttc_abstract with
      | _ when Option.is_some typeconst.ttc_reifiable || is_concrete ->
        super#on_typeconst acc class_ is_concrete typeconst
      | _ ->
        let r = Reason.Rwitness_from_decl (fst typeconst.ttc_name) in
        let kind =
          "an abstract type constant without the __Reifiable attribute"
        in
        this#invalid acc r kind

    method! on_taccess acc r (root, ids) =
      let acc =
        match acc.reification with
        | Unresolved -> this#on_type acc root
        | Resolved -> acc
      in
      super#on_taccess acc r (root, ids)

    method! on_tthis acc r =
      this#invalid acc r "the late static bound this type"
  end
