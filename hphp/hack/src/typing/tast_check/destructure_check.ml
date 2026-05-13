(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* TODO(T266467978): stub — real structural validation in next commit *)

let handler : Tast_visitor.handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr (_env : Tast_env.env) ((_ty, _pos, expr_) : Tast.expr)
        : unit =
      match expr_ with
      | DestructureShape _ -> ()
      | DestructureTuple _ -> ()
      | _ -> ()
  end
