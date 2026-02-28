(*
 * Copyright (c) Meta, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This module is used to generate fbthrift.declaration facts from
    hack entities generated from Thrift.

    For generated hack classes or field/function members,
    a thrift declaration can be constructed from one or more
    of the following:
    - the doc comment of the generated entity
    - the entity name (for union, and enum fields).
    - the context in which it appears (enclosing class,
        thrift source of generated file...)

    The entity comment isn't always sufficient and some context [t] must
    be maintain at the file level *)

(** Defines a mutable context used when processing generated entities. *)
type t

(** Empty context, [thrift_path] is the path relative to repo root
    of the source thrift file *)
val empty : thrift_path:string -> t

(** Parses a hack container doc comment and generate thrift decl fact.

    Side effect: store in context Thrift container declaration *)
val get_thrift_from_container :
  t -> ('a, 'b) Aast_defs.class_ -> Fbthrift.Declaration.t option

(** Parses a hack member function or field doc comment and generates
   the corresponding declaration thrift fact. Assumes
   [get_thrift_from_container] was called immediately before on
   the enclosing container.

   Side effect, for hack fields generated from Thrift union field, remember
   Thrift field name in context [t] *)
val get_thrift_from_comment : t -> doc:string -> Fbthrift.Declaration.t option

(** Generate thrift declaration fact from an enumerator.
   Assume [get_thrift_from_container] was called immediately before on the
   enclosing enum declaration. *)
val get_thrift_from_enum : t -> string -> Fbthrift.Declaration.t option

(** Generate thrift declaration fact for class members generated from union fields.
   Assume [get_thrift_from_container] was called on the enclosing container,
   and [get_thrift_from_comment] on the first union field. *)
val get_thrift_from_union_member : t -> string -> Fbthrift.Declaration.t option
