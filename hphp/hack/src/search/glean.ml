(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type handle = unit

let initialize ~reponame:_ = failwith "not implemented"

let fetch_namespaces _ = failwith "not implemented"

let query_autocomplete _ ~query_text:_ ~max_results:_ ~context:_ ~kind_filter:_
    =
  failwith "not implemented"

let query_filenames _ ~angle:_ ~max_results:_ = failwith "not implemented"
