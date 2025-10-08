(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
[@@@warning "-40"]

open Core

type file_path =
  | Dot
  | Slash of {
      prefix: file_path;
      segment: Patt_string.t;
    }
[@@deriving compare, eq, sexp, show]

type t =
  | As of {
      lbl: Patt_var.t;
      patt: t;
    }
  | Name of {
      patt_file_path: file_path option;
      patt_file_name: Patt_string.t;
      patt_file_extension: Patt_string.t;
    }
  | Wildcard
  | Invalid of {
      errs: Validation_err.t list;
      patt: t;
    }
[@@deriving compare, eq, sexp, show]
