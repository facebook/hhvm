(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* This validation pass will raise errors when it encounters use of certain
   miscased or errneous XHP related class names.
*)
include Naming_phase_sigs.Validation
