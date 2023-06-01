(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t =
  | As of {
      lbl: Patt_var.t;
      patt: Patt_string.t;
    }
  | Name of Patt_string.t
  | Wildcard
  | Invalid of Validation_err.t list * t
[@@deriving compare, sexp, show, yojson]

include Can_validate.S with type t := t
