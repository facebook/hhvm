(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core

type t =
  | Member_name of { patt_string: Patt_string.t }
  | As of {
      lbl: Patt_var.t;
      patt: t;
    }
  | Wildcard
  | Invalid of {
      errs: Validation_err.t list;
      patt: t;
    }
[@@deriving compare, eq, sexp, show]
