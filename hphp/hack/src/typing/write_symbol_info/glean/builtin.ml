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



module rec Unit: sig
  type t = unit
  [@@deriving ord]

  val to_json : t -> json
end = struct
  type t = unit
  [@@deriving ord]

  let to_json _ = JSON_Object []
end


