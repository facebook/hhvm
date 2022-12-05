(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* This validation pass will raise errors when it encounters use of internal,
   enum related class names:
   - \\HH\\BuiltinEnum
   - \\HH\\BuiltinEnumClass
   - \\HH\\BuiltinAbstractEnumClass

   Note that the validation _must_ be performed before enum elaboration when
   enum definitions are modified to extend one of these classes
*)
include Naming_phase_sigs.Validation
