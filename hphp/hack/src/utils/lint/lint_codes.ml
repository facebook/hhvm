(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | LowercaseConstant [@value 5001]
  | UseCollectionLiteral
  | StaticString
  | ShapeIdxRequiredField [@value 5005]
  | OptClosedShapeIdxMissingField [@value 5006] (* EXTEND HERE WITH NEW VALUES IF NEEDED *)
[@@deriving enum]

let err_code = to_enum

(* Values 5501 - 5999 are reserved for FB-internal use *)
