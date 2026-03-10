(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open! Ppx_yojson_conv_lib.Yojson_conv.Primitives

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

let yojson_of_bucket = function
  | Tuple -> `String "Tuple"
  | Vec -> `String "Vec"
  | Pair -> `String "Pair"
  | Union -> `String "Union"
  | Intersection -> `String "Intersection"
  | Dynamic -> `String "Dynamic"
  | Generic -> `String "Generic"
  | Misc -> `String "Misc"

let bucket_of_yojson = function
  | `String "Tuple" -> Tuple
  | `String "Vec" -> Vec
  | `String "Pair" -> Pair
  | `String "Union" -> Union
  | `String "Intersection" -> Intersection
  | `String "Dynamic" -> Dynamic
  | `String "Generic" -> Generic
  | `String "Misc" -> Misc
  | yojson ->
    Ppx_yojson_conv_lib.Yojson_conv.of_yojson_error
      "bucket_of_yojson: unexpected value"
      yojson

type lvalue_kind =
  | Array_append
  | Array_get
  | Obj_get
  | Class_get
  | Call
  | ReadonlyExpr
  | Other
[@@deriving show { with_path = false }]

let yojson_of_lvalue_kind = function
  | Array_append -> `String "Array_append"
  | Array_get -> `String "Array_get"
  | Obj_get -> `String "Obj_get"
  | Class_get -> `String "Class_get"
  | Call -> `String "Call"
  | ReadonlyExpr -> `String "ReadonlyExpr"
  | Other -> `String "Other"

let lvalue_kind_of_yojson = function
  | `String "Array_append" -> Array_append
  | `String "Array_get" -> Array_get
  | `String "Obj_get" -> Obj_get
  | `String "Class_get" -> Class_get
  | `String "Call" -> Call
  | `String "ReadonlyExpr" -> ReadonlyExpr
  | `String "Other" -> Other
  | yojson ->
    Ppx_yojson_conv_lib.Yojson_conv.of_yojson_error
      "lvalue_kind_of_yojson: unexpected value"
      yojson

type receiver_kind =
  | Self
  | Static
  | Parent
  | Explicit
  | Expr
[@@deriving show { with_path = false }]

let yojson_of_receiver_kind = function
  | Self -> `String "Self"
  | Static -> `String "Static"
  | Parent -> `String "Parent"
  | Explicit -> `String "Explicit"
  | Expr -> `String "Expr"

let receiver_kind_of_yojson = function
  | `String "Self" -> Self
  | `String "Static" -> Static
  | `String "Parent" -> Parent
  | `String "Explicit" -> Explicit
  | `String "Expr" -> Expr
  | yojson ->
    Ppx_yojson_conv_lib.Yojson_conv.of_yojson_error
      "receiver_kind_of_yojson: unexpected value"
      yojson

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

let to_json_string entry = Yojson.Safe.to_string (yojson_of_t entry)

let of_json_string s = t_of_yojson (Yojson.Safe.from_string s)
