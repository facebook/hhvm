(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t
val make : ?hhvm_compat_mode:bool -> Full_fidelity_source_text.t -> t
val errors : t -> Full_fidelity_syntax_error.t list
val hhvm_compat_mode : t -> bool
val parse_script : t -> t * Full_fidelity_minimal_syntax.t
