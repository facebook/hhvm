(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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

include
  Phase_error.S with type t := t and module Error_code = Error_codes.Parsing
