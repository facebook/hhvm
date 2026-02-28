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
  | Slash_opt of {
      prefix: file_path;
      segment: Patt_string.t;
    }
[@@deriving compare, eq, sexp, show]

let dot = Dot

let ( </> ) prefix segment = Slash { prefix; segment }

let ( </?> ) prefix segment = Slash_opt { prefix; segment }

type t =
  | As of {
      lbl: Patt_var.t;
      patt: t;
    }
  | Name of {
      allow_glob: bool;
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
