(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = {
  props: Decl_heap.Props.KeySet.t;
  sprops: Decl_heap.StaticProps.KeySet.t;
  meths: Decl_heap.Methods.KeySet.t;
  smeths: Decl_heap.StaticMethods.KeySet.t;
}

val get_for_classes : old:bool -> string list -> t SMap.t

val oldify_all : t SMap.t -> unit

val remove_old_all : t SMap.t -> unit

val remove_all : t SMap.t -> unit
