(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env ((p, ty), _) =
      if
        TypecheckerOptions.disallow_unresolved_type_variables
          (Tast_env.get_tcopt env)
      then
        ignore (Tast_expand.expand_ty ~pos:p env ty)
  end
