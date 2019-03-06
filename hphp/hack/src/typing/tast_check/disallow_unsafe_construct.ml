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

module UA = Naming_special_names.UserAttributes

let handler = object
  inherit Tast_visitor.handler_base

  method! at_user_attribute env { ua_name = (pos, name); _ } =
    if TypecheckerOptions.disallow_unsafe_construct (Tast_env.get_tcopt env) &&
      name = Naming_special_names.UserAttributes.uaUnsafeConstruct then
      Errors.experimental_feature pos ("The " ^ name ^ " attribute is not supported.");

end
