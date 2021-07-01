(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Compute_tast : sig
  type t = {
    tast: Tast.program;
    telemetry: Telemetry.t;
  }
end

module Compute_tast_and_errors : sig
  type t = {
    tast: Tast.program;
    errors: Errors.t;
    telemetry: Telemetry.t;
  }
end

(** Computes TAST and error-list (other than "name already
bound" errors) by taking the AST in a context entry,
and typechecking it, and memoizing the result (caching the results in the
context entry). CAUTION: this function doesn't use a quarantine, and so
is inappropriate for IDE scenarios. *)
val compute_tast_and_errors_unquarantined :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  Compute_tast_and_errors.t

(** Same as [compute_tast_and_errors_unquarantined], but skips computing the
full error list. If the errors are needed at a later time, you'll have to incur
the full cost of recomputing the entire TAST and errors. *)
val compute_tast_unquarantined :
  ctx:Provider_context.t -> entry:Provider_context.entry -> Compute_tast.t

(** This function computes TAST and error-list. At the moment,
the suffix "quarantined" means that this function enforces a quarantine
in case one isn't yet in force. In future, it might mean that we assert
that a quarantine is already in force. CAUTION: this function is only
appropriate for IDE scenarios. *)
val compute_tast_and_errors_quarantined :
  ctx:Provider_context.t ->
  entry:Provider_context.entry ->
  Compute_tast_and_errors.t

(** Same as [compute_tast_and_errors_quarantined], but skips computing the full
error list. If the errors are needed at a later time, you'll have to incur the
full cost of recomputing the entire TAST and errors. *)
val compute_tast_quarantined :
  ctx:Provider_context.t -> entry:Provider_context.entry -> Compute_tast.t
