(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let go : ServerEnv.env -> string -> string =
 fun env filename ->
  let _ = env in
  let _ = Hh_logger.info "Hello, world!!!!!" in
  String.concat " " ["Hello!!"; filename]
