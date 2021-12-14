(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module Error_code = Error_codes.Parsing

type t =
  | Fixme_format of Pos.t
  | Parsing_error of {
      pos: Pos.t;
      msg: string;
    }
  | Xhp_parsing_error of {
      pos: Pos.t;
      msg: string;
    }

let fixme_format pos = Fixme_format pos

let parsing_error pos ~msg = Parsing_error { pos; msg }

let xhp_parsing_error pos ~msg = Xhp_parsing_error { pos; msg }

(* -- Phase error implementation -------------------------------------------- *)
let error_code = function
  | Fixme_format _ -> Error_code.FixmeFormat
  | Parsing_error _ -> Error_code.ParsingError
  | Xhp_parsing_error _ -> Error_code.XhpParsingError

let claim err =
  match err with
  | Fixme_format pos ->
    (pos, "`HH_FIXME` wrong format, expected `/* HH_FIXME[ERROR_NUMBER] */`")
  | Parsing_error { pos; msg }
  | Xhp_parsing_error { pos; msg } ->
    (pos, msg)

let reasons = function
  | Fixme_format _
  | Parsing_error _
  | Xhp_parsing_error _ ->
    []

let quickfixes = function
  | Fixme_format _
  | Parsing_error _
  | Xhp_parsing_error _ ->
    []
