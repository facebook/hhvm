(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type bucket =
  | Tuple
  | Vec
  | Pair
  | Union
  | Intersection
  | Dynamic
  | Generic
  | Misc
[@@deriving show { with_path = false }]

val yojson_of_bucket : bucket -> Yojson.Safe.t

val bucket_of_yojson : Yojson.Safe.t -> bucket

type lvalue_kind =
  | Array_append
  | Array_get
  | Obj_get
  | Class_get
  | Call
  | ReadonlyExpr
  | Other
[@@deriving show { with_path = false }]

val yojson_of_lvalue_kind : lvalue_kind -> Yojson.Safe.t

val lvalue_kind_of_yojson : Yojson.Safe.t -> lvalue_kind

type receiver_kind =
  | Self
  | Static
  | Parent
  | Explicit
  | Expr
[@@deriving show { with_path = false }]

val yojson_of_receiver_kind : receiver_kind -> Yojson.Safe.t

val receiver_kind_of_yojson : Yojson.Safe.t -> receiver_kind

type pos_info = {
  filename: string;
  line: int;
  char_start: int;
  char_end: int;
}
[@@deriving yojson]

type complex_lvalue_element = {
  lvalue_kind: lvalue_kind;
  lvalue_pos: pos_info;
  lvalue_code: string;
  lvalue_involves_call: bool;
  lvalue_receiver_kind: receiver_kind option; [@yojson.option]
}
[@@deriving yojson]

type t = {
  bucket: bucket;
  type_string: string;
  has_complex_lvalue: bool;
  pos: pos_info;
  lhs_code: string;
  total_lvalue_count: int;
  is_generated: bool;
  ty_node_kind: string option; [@yojson.option]
  complex_lvalue_elements: complex_lvalue_element list;
      [@default []] [@yojson_drop_if List.is_empty]
}
[@@deriving yojson]

val to_json_string : t -> string

val of_json_string : string -> t
