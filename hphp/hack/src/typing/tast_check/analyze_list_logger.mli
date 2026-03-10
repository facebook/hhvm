(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** TAST logger that records list() assignment patterns.

    To run on WWW and analyze:
    {[
      buck run @//mode/opt //hphp/hack/src/analyze/list:run_list_analysis -- --root ~/www --pastry
    ]} *)

val create_handler : Provider_context.t -> Tast_visitor.handler
