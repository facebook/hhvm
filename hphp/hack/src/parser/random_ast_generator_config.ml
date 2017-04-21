(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This file specifies special configuration where the generation of random
* program is biased to create the programs of certain properties
*
* Each config is a dictionary mapping a nonterminal to the distribution of the
* probabilities of the different production rules from this terminal.
*
* Note: the weight in the distribution should sum up to 1 *)

type t = (string * float list) list

let exists name config = List.mem_assoc name config
let get name config = List.assoc name config
let default = []

let long_assignment_config = [
  ("SimpleAssignmentExpression", [
    0.6; 0.2; 0.2;
  ]);
  ("CompoundAssignmentExpression", [
    0.6; 0.2; 0.2;
  ]);
]

let mapping = [
  ("Default", default);
  ("LongAssignmentConfig", long_assignment_config);
]

let find_config name = List.assoc name mapping
