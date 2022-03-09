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
      quickfixes: Quickfix.t list;
    }
  | Xhp_parsing_error of {
      pos: Pos.t;
      msg: string;
    }

let to_user_error = function
  | Fixme_format pos ->
    User_error.make
      Error_code.(to_enum FixmeFormat)
      (pos, "`HH_FIXME` wrong format, expected `/* HH_FIXME[ERROR_NUMBER] */`")
      []
  | Parsing_error { pos; msg; quickfixes } ->
    User_error.make Error_code.(to_enum ParsingError) ~quickfixes (pos, msg) []
  | Xhp_parsing_error { pos; msg } ->
    User_error.make Error_code.(to_enum XhpParsingError) (pos, msg) []
