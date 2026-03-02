(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let go ~ctx:_ ~genv:_ ~env:_ ~max_distance:_ ~max_test_files:_ _actions =
  Error "FindMyTests is not available in open source builds"
