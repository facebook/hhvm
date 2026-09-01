(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type enforced_type = {
  ty_str: string;
  ty_json: Yojson.Safe.t;
}

type enforcement =
  | Enforced of enforced_type list
  | Unenforced

type result = enforcement option
