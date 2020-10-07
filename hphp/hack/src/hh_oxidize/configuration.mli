(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Config settings which apply to the conversion of all modules. The global
    configuration is set once at startup and not changed after. *)

type mode =
  | ByBox  (** Emit recursive definitions using Boxes. Use Vecs and Strings. *)
  | ByRef
      (** Emit type definitions containing references, slices, and
          &strs rather than Boxes, Vecs, and Strings. The emitted type
          definitions are intended to be suitable for arena-allocation
          (i.e., all types have a no-op implementation of Drop). *)

type t = {
  mode: mode;
      (** The mode to when emitting recursive types, vectors and strings. *)
  extern_types: string SMap.t;
      (** The extern_types setting allows for the importing of types defined
          outside the set of modules to be oxidized. If our extern_types map has
          an entry mapping ["bar::Bar"] to ["foo::bar::Bar"], then instances of
          [Bar.t] in the OCaml source will be converted to [foo::bar::Bar]
          rather than [bar::Bar]. All extern_types are assumed to take no
          lifetime parameter. *)
  owned_types: SSet.t;
      (** The owned_types setting allows specifying a set of types which do not
          need a lifetime parameter (so that hh_oxidize need not use global
          knowledge of all types being converted to track which do and do not
          need lifetime parameters). *)
  copy_types: SSet.t option;
      (** The owned_types setting allows specifying a set of types which
          implement Copy, and should not be put behind a reference (so that
          hh_oxidize need not use global knowledge of all types being converted
          to track which do and do not implement Copy). *)
}

val default : t

(** Set the global config. To be invoked at startup. Raises an exception if
    invoked more than once. *)
val set : t -> unit

(** Return the emitter's [mode]. Raises an exception if invoked before [set]. *)
val mode : unit -> mode

(** If the given type name was set to be imported from an extern types file,
    return its fully-qualified name, else None. Raises an exception if invoked
    before [set]. *)
val extern_type : string -> string option

(** If the given type name does not need a lifetime parameter, return true.
    Raises an exception if invoked before [set]. *)
val owned_type : string -> bool

(** If the given type name implements (or should implement) Copy, return
    [`Known true]. If no list of copy types was provided, return [`Unknown].
    Raises an exception if invoked before [set]. *)
val copy_type : string -> [ `Known of bool | `Unknown ]
