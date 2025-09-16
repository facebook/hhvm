(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Config settings which apply to the conversion of all modules. The global
    configuration is set once at startup and not changed after. *)

type t = {
  extern_types: string SMap.t;
      (** The extern_types setting allows for the importing of types defined
          outside the set of modules to be oxidized. If our extern_types map has
          an entry mapping ["bar::Bar"] to ["foo::bar::Bar"], then instances of
          [Bar.t] in the OCaml source will be converted to [foo::bar::Bar]
          rather than [bar::Bar]. All extern_types are assumed to take no
          lifetime parameter. *)
  copy_types: SSet.t option;
      (** The owned_types setting allows specifying a set of types which
          implement Copy, and should not be put behind a reference (so that
          hh_oxidize need not use global knowledge of all types being converted
          to track which do and do not implement Copy). *)
  safe_ints_types: SSet.t;
      (** Types for which any ocaml int will be converted to ocamlrep::OCamlInt rather than isize *)
}

val default : t

(** Set the global config. To be invoked at startup. Raises an exception if
    invoked more than once. *)
val set : t -> unit

(** If the given type name was set to be imported from an extern types file,
    return its fully-qualified name, else None. Raises an exception if invoked
    before [set]. *)
val extern_type : string -> string option

(** If the given type name implements (or should implement) Copy, return
    [`Known true]. If no list of copy types was provided, return [`Unknown].
    Raises an exception if invoked before [set]. *)
val copy_type : string -> [ `Known of bool | `Unknown ]

(** Test if the given value is the right kind of `Known *)
val is_known : [> `Known of bool ] -> bool -> bool

(** Whether OCaml ints should be converted to ocamlrep::OCamlInt instead of
  isize for this type declarartion *)
val safe_ints : mod_name:string -> name:string -> bool
