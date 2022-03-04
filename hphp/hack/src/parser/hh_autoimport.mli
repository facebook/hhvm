(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** The names of autoimport types. *)
val types : string list

(** The names of autoimport functions. *)
val funcs : string list

(** The names of autoimport constants. *)
val consts : string list

(** The names of autoimport namespaces. *)
val namespaces : string list

(** Strip any `HH\\` prefix from the provided string if the string
  is the name of a autoimport type. *)
val strip_HH_namespace_if_autoimport : string -> string
