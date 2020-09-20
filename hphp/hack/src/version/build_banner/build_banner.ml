(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external get_build_banner : unit -> string option = "hh_get_build_banner"

let banner = get_build_banner ()
