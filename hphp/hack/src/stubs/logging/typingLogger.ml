(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module ProfileTypeCheck = struct
  let init
      ~init_id:_ ~threshold:_ ~owner:_ ~desc:_ ~max_times_to_defer:_ ~root:_ =
    ()

  let log
      ~recheck_id:_
      ~start_time:_
      ~second_start_time_opt:_
      ~times_checked:_
      ~files_to_declare:_
      ~absolute:_
      ~relative:_ =
    ()

  let print_path ~init_id:_ ~recheck_id:_ = ()
end

let log_lambda_counts _ = ()

module InferenceCnstr = struct
  let log _ ~pos:_ ~size:_ ~n_disj:_ ~n_conj:_ = ()
end

let flush_buffers _ = ()
