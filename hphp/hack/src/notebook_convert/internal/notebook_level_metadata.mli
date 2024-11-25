(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = {
  notebook_number: string;  (** example: "N12345" *)
  kernelspec: Hh_json.json;
      (** corresponds to 'kernelspec' in ipynb format but we don't bother interpreting it *)
}

val of_comment : string -> t option

val to_comment : t -> string
