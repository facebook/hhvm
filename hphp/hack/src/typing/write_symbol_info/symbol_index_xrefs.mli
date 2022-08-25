(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val process_xrefs_and_calls :
  Provider_context.t ->
  Symbol_predicate.Fact_acc.t ->
  Symbol_file_info.t ->
  Symbol_predicate.Fact_acc.t
