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
  | Hh_ignore_comment of Pos.t
  | Parsing_error of {
      pos: Pos.t;
      msg: string;
      quickfixes: Pos.t Quickfix.t list;
    }
  | Xhp_parsing_error of {
      pos: Pos.t;
      msg: string;
    }
  | Package_config_error of {
      pos: Pos.t;
      msg: string;
    }

let to_user_error = function
  | Fixme_format pos ->
    User_error.make
      Error_code.(to_enum FixmeFormat)
      (pos, "`HH_FIXME` wrong format, expected `/* HH_FIXME[ERROR_NUMBER] */`")
      []
  | Hh_ignore_comment pos ->
    User_error.make
      Error_code.(to_enum HhIgnoreComment)
      ( pos,
        "`HH_IGNORE_ERROR` comments are disabled by configuration and will soon be treated like normal comments, so you cannot use them to suppress errors"
      )
      []
  | Parsing_error { pos; msg; quickfixes } ->
    User_error.make Error_code.(to_enum ParsingError) ~quickfixes (pos, msg) []
  | Xhp_parsing_error { pos; msg } ->
    User_error.make Error_code.(to_enum XhpParsingError) (pos, msg) []
  | Package_config_error { pos; msg } ->
    User_error.make Error_code.(to_enum PackageConfigError) (pos, msg) []
