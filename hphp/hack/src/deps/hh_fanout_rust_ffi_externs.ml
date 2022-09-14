(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type hh_fanout_rust_ffi

type edges_buffer

type edges_error = EeDecl of string

type edges_result = (edges_buffer, edges_error) result

type finish_edges_result = (unit, edges_error) result

external start_edges : hh_fanout_rust_ffi -> edges_buffer
  = "hh_fanout_start_edges"

external add_edge :
  hh_fanout_rust_ffi ->
  dependent:Int64.t ->
  dependency:Int64.t ->
  edges_buffer ->
  expected_checksum:Int64.t ->
  edges_result = "hh_fanout_add_edge"

external finish_edges :
  hh_fanout_rust_ffi -> edges_buffer -> finish_edges_result
  = "hh_fanout_finish_edges"
