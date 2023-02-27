(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Sdt_analysis_types

val handler : Provider_context.t -> H.Write.t -> Tast_visitor.handler
