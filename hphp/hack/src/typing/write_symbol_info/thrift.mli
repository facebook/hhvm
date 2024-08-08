(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Defines a context used when parsing comments in a
    generated-thrift file. *)
type t

(** Should be created for each file *)
val empty : thrift_path:string -> t

(** Parse a hack container doc comment in hack file generated
    from [thrift_path] and generate corresponding thrift fact.

    As a side effect, remember container information needed
    to generate thrift declaration from members. *)
val get_thrift_from_container :
  t -> ('a, 'b) Aast_defs.class_ -> Fbthrift.Declaration.t option

(** Parse a hack member function doc comment and generate corresponding
   thrift fact. Assume [get_thrift_from_container] was called immediately
   before on the enclosing container. *)
val get_thrift_from_member : t -> doc:string -> Fbthrift.Declaration.t option

(** Generate thrift declaration fact from an enumerator.
   Assume [get_thrift_from_container] was called immediately before on the
   enclosing enum declaration. *)
val get_thrift_from_enum : t -> string -> Fbthrift.Declaration.t option
