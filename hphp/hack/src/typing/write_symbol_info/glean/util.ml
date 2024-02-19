(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *)

(* @generated
   regenerate: buck2 run fbcode//glean/schema/gen:gen-schema  -- --ocaml fbcode/hphp/hack/src/typing/write_symbol_info/schema --dir DEST_DIR *)

open Hh_json


let key v = JSON_Object [("key", v)]

let id fact_id = JSON_Object [("id", Fact_id.to_json_number fact_id)]

let rem_opt (name, json_opt) =
  match json_opt with
  | Some json -> Some (name, json)
  | None -> None


