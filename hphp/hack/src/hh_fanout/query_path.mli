(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type result

val go : source:Typing_deps.Dep.t -> dest:Typing_deps.Dep.t -> result

val result_to_json : result -> Hh_json.json
