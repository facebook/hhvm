(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  root: Path.t;
  from: string;
  rageid: string option;
  desc: string;
}

let main _ = Exit.exit Exit_status.No_error

let verify_typechecker_err_src () = ()
