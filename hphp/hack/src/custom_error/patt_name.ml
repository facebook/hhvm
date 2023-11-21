(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* TODO: investigate this warning 40 and how to fix it correctly *)
[@@@warning "-66-40"]

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
[@@deriving compare, eq, sexp, show]
