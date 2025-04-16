(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

open Aast

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_user_attribute env { ua_name; ua_params } =
      if
        String.equal
          (snd ua_name)
          Naming_special_names.UserAttributes.uaSimpliHack
      then
        List.iter ua_params ~f:(fun e ->
            ignore @@ Simplihack_interpreter.eval env e)
  end
