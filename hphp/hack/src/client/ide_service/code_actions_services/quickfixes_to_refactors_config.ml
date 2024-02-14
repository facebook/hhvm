(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Make sure to write tests any time you add an error here,
 * as correct behavior is dependent on matching specific strings (which can change).
 *)
let mapping_from_error_message_to_refactors =
  SMap.of_list
    [
      ( "`await` cannot be used as an expression in this location because it's conditionally executed.",
        Extract_variable.find );
    ]
