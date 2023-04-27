(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Sdt_analysis_types

val of_summary : Summary.t -> string Sequence.t

(**
raises if given an invalid summary json line
returns `None` if valid summary json line that isn't a nadable list (such as `stats`)
otherwise
    `Some` lists of items that must have `__NoAutoDynamic` added atomically
    for every item in the list
*)
val nadables_of_line_exn : string -> Summary.nadable list option
