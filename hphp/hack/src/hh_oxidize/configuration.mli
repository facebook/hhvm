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
  | ByRc  (** Emit recursive definitions using Rcs. *)

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
