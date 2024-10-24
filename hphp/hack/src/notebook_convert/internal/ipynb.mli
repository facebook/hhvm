(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type cell =
  | Non_hack of {
      cell_type: string;
      contents: string;
    }
  | Hack of string

type t = cell list

(** Expects JSON matching schema:
* https://github.com/jupyter/nbformat/blob/main/nbformat/v4/nbformat.v4.schema.json
*)
val ipynb_of_json : Hh_json.json -> (t, string) result
