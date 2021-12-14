(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Phase_error.S with module Error_code = Error_codes.Parsing

val fixme_format : Pos.t -> t

val parsing_error : Pos.t -> msg:string -> t

val xhp_parsing_error : Pos.t -> msg:string -> t
