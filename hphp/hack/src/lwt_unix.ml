(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This file exists only to ensure that we don't accidentally link hh_server
against Lwt_unix. Something about the Lwt_unix module initialization messes with
signals in such a way that many of our raw Unix.XXX IO calls fail with EINTR, so
adding this fake module causes us to have a build failure if we ever link
against Lwt_unix from inside hh_server. EINTR is technically retryable, and we
could fix it by finding every single existing raw IO call and wrapping them, but
we'd have to make sure that no raw IO ever got checked in again without being
guarded. Given that, we felt it was more future-proof to just forbid Lwt_unix.
For more context, view T37072141. *)
