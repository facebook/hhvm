(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** A masking scheme indicating logging mode.

  For example, both the log levels 1 (0x1) and 3 (0x11) enable logging to a
  file. *)
type log_mask =
  | File [@value 1]
  | Scuba [@value 2]
[@@deriving enum]

type expression_info = {
  declaration_usage: bool;
      (** Notes that a bad type appears in the expression type as a direct
          consequence of a declaration having a bad type in it.

          This is less important than other bad types because the real culprit
          is the signature with the bad type in it. *)
  producer: string option;
      (** Marks a point where a bad type is introduced, e.g., as a result of a
          function call. When statically available, it indicates what produced
          the bad type, e.g., a function name.  *)
}

type parameter_info = {
  is_inout: bool;
  is_variadic: bool;
}

type position =
  | Return
  | Parameter of parameter_info

type declaration_info = {
  position: position;  (** Where in a declaration a bad type appears? *)
}

(** A bit vector to indicate if a type has a bad type in it and if so which
    one. *)
type bad_type_indicator = {
  has_tany: bool;
  has_terr: bool;
}

let has_bad_type { has_tany; has_terr } = has_tany || has_terr

(** A context is either a function name or a method name qualified by the
    enclosing class. *)
type context_id =
  | Function of string
  | Method of string * string

type common_info = {
  indicator: bad_type_indicator;  (** Does the type contain bad types? *)
  is_generated: bool;  (** Is it in a generated file (based on path)? *)
  is_test: bool;  (** Is it in a test file (based on path)? *)
  pos: Relative_path.t Pos.pos;  (** Position of a bad type *)
  context_id: context_id;
      (** Identifier for where bad types appear either of the form
          `function_name` or `class_name::method_name` *)
}

type context =
  | Expression of expression_info
  | Declaration of declaration_info

type info = {
  context: context;
      (** Information that is depended on being a decl or an expression *)
  common_info: common_info;
      (** Information common to different contexts bad types appear in. *)
}
