(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type handle = unit

let initialize ~reponame:_ ~prev_init_time:_ = failwith "not implemented"

let query_autocomplete _ ~query_text:_ ~max_results:_ ~context:_ ~kind_filter:_
    =
  failwith "not implemented"

let query_refs _ ~action:_ ~max_results:_ = failwith "not implemented"
