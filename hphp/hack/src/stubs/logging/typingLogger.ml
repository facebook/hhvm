(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let log_lambda_counts _ = ()

module InferenceCnstr = struct
  let log _ ~pos:_ ~size:_ ~n_disj:_ ~n_conj:_ = ()
end

let flush_buffers _ = ()
