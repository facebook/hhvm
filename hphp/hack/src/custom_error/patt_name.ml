(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-66"]

module V = Validated
open Core

type t =
  | As of {
      lbl: Patt_var.t;
      patt: t;
    }
  | Name of {
      patt_namespace: namespace;
      patt_name: Patt_string.t;
    }
  | Wildcard
  | Invalid of {
      errs: Validation_err.t list;
      patt: t;
    }

and namespace =
  | Root
  | Slash of {
      prefix: namespace;
      elt: Patt_string.t;
    }
[@@deriving compare, eq, sexp, show, yojson]

let validate ?(env = Validation_env.empty) t =
  match t with
  | As { lbl; _ } ->
    (match Validation_env.add ~key:lbl ~data:Patt_binding_ty.Name env with
    | Error env ->
      ( V.invalid (Invalid { errs = [Validation_err.Shadowed lbl]; patt = t }),
        env )
    | Ok env -> (V.valid t, env))
  | _ -> (V.valid t, env)
