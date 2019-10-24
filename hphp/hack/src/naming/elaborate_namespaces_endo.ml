(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast

type env = { namespace: Namespace_env.env }

let namespace_elaborater =
  object (_self)
    inherit [_] Aast.endo as super

    method on_'ex _ ex = ex

    method on_'fb _ fb = fb

    method on_'en _ en = en

    method on_'hi _ hi = hi

    method! on_expr_ env expr =
      match expr with
      | Call (ct, (p, Id (p2, cn)), targs, [(p3, String fn)], uargs)
        when cn = Naming_special_names.SpecialFunctions.fun_ ->
        (* Functions referenced by fun() are always fully-qualified *)
        let fn = Utils.add_ns fn in
        Call (ct, (p, Id (p2, cn)), targs, [(p3, String fn)], uargs)
      | _ -> super#on_expr_ env expr

    method! on_program (env : env) (p : Nast.program) = super#on_program env p
  end

let make_env namespace = { namespace }
