(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

let () =
  List.iter
    Generate_full_fidelity_data.templates
    ~f:Full_fidelity_schema.generate_file
