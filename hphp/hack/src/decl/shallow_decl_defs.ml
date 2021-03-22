(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

(* Is this bit set in the flags? *)
let is_set bit flags = not (Int.equal 0 (Int.bit_and bit flags))

(* Set a single bit to a boolean value *)
let set_bit bit value flags =
  if value then
    Int.bit_or bit flags
  else
    Int.bit_and (Int.bit_not bit) flags

module PropFlags = struct
  type t = int [@@deriving eq]

  let empty = 0

  let abstract_bit    = 1 lsl 0
  let const_bit       = 1 lsl 1
  let lateinit_bit    = 1 lsl 2
  let lsb_bit         = 1 lsl 3
  let needs_init_bit  = 1 lsl 4
  let php_std_lib_bit = 1 lsl 5
  let readonly_bit    = 1 lsl 6

  let get_abstract    = is_set abstract_bit
  let get_const       = is_set const_bit
  let get_lateinit    = is_set lateinit_bit
  let get_lsb         = is_set lsb_bit
  let get_needs_init  = is_set needs_init_bit
  let get_php_std_lib = is_set php_std_lib_bit
  let get_readonly = is_set readonly_bit

  let set_abstract    = set_bit abstract_bit
  let set_const       = set_bit const_bit
  let set_lateinit    = set_bit lateinit_bit
  let set_lsb         = set_bit lsb_bit
  let set_needs_init  = set_bit needs_init_bit
  let set_php_std_lib = set_bit php_std_lib_bit
  let set_readonly    = set_bit readonly_bit

  let make
      ~abstract
      ~const
      ~lateinit
      ~lsb
      ~needs_init
      ~php_std_lib
      ~readonly
      =
    empty
    |> set_abstract abstract
    |> set_const const
    |> set_lateinit lateinit
    |> set_lsb lsb
    |> set_needs_init needs_init
    |> set_php_std_lib php_std_lib
    |> set_readonly readonly

  let pp fmt t =
    if t = empty then
      Format.pp_print_string fmt "(empty)"
    else (
      Format.fprintf fmt "@[<2>";
      let sep = ref false in
      let print s =
        if !sep then Format.fprintf fmt " |@ ";
        Format.pp_print_string fmt s;
        sep := true
      in
      if get_abstract t then print "abstract";
      if get_const t then print "const";
      if get_lateinit t then print "lateinit";
      if get_lsb t then print "lsb";
      if get_needs_init t then print "needs_init";
      if get_php_std_lib t then print "php_std_lib";
      Format.fprintf fmt "@,@]"
    )

  let show = Format.asprintf "%a" pp
end
[@@ocamlformat "disable"]

module MethodFlags = struct
  type t = int [@@deriving eq]

  let empty = 0

  let abstract_bit               = 1 lsl 0
  let final_bit                  = 1 lsl 1
  let override_bit               = 1 lsl 2
  let dynamicallycallable_bit    = 1 lsl 3
  let php_std_lib_bit            = 1 lsl 4
  let sound_dynamic_callable_bit = 1 lsl 5

  let get_abstract               = is_set abstract_bit
  let get_final                  = is_set final_bit
  let get_override               = is_set override_bit
  let get_dynamicallycallable    = is_set dynamicallycallable_bit
  let get_php_std_lib            = is_set php_std_lib_bit
  let get_sound_dynamic_callable = is_set sound_dynamic_callable_bit

  let set_abstract               = set_bit abstract_bit
  let set_final                  = set_bit final_bit
  let set_override               = set_bit override_bit
  let set_dynamicallycallable    = set_bit dynamicallycallable_bit
  let set_php_std_lib            = set_bit php_std_lib_bit
  let set_sound_dynamic_callable = set_bit sound_dynamic_callable_bit

  let make
      ~abstract
      ~final
      ~override
      ~dynamicallycallable
      ~php_std_lib
      ~sound_dynamic_callable
      =
    empty
    |> set_abstract abstract
    |> set_final final
    |> set_override override
    |> set_dynamicallycallable dynamicallycallable
    |> set_php_std_lib php_std_lib
    |> set_sound_dynamic_callable sound_dynamic_callable

  let pp fmt t =
    if t = empty then
      Format.pp_print_string fmt "(empty)"
    else (
      Format.fprintf fmt "@[<2>";
      let sep = ref false in
      let print s =
        if !sep then Format.fprintf fmt " |@ ";
        Format.pp_print_string fmt s;
        sep := true
      in
      if get_abstract t then print "abstract";
      if get_final t then print "final";
      if get_override t then print "override";
      if get_dynamicallycallable t then print "dynamicallycallable";
      if get_php_std_lib t then print "php_std_lib";
      Format.fprintf fmt "@,@]"
    )

  let show = Format.asprintf "%a" pp
end
[@@ocamlformat "disable"]

type shallow_class_const = {
  scc_abstract: bool;
  scc_name: Typing_defs.pos_id;
  scc_type: decl_ty;
  scc_refs: Typing_defs.class_const_ref list;
}
[@@deriving eq, show]

type shallow_typeconst = {
  stc_abstract: typeconst_abstract_kind;
  stc_as_constraint: decl_ty option;
  stc_super_constraint: decl_ty option;
  stc_name: Typing_defs.pos_id;
  stc_type: decl_ty option;
  stc_enforceable: Pos_or_decl.t * bool;
  stc_reifiable: Pos_or_decl.t option;
}
[@@deriving eq, show]

type shallow_prop = {
  sp_name: Typing_defs.pos_id;
  sp_xhp_attr: xhp_attr option;
  sp_type: decl_ty option;
  sp_visibility: Ast_defs.visibility;
  sp_flags: PropFlags.t;
}
[@@deriving eq, show]

type shallow_method = {
  sm_name: Typing_defs.pos_id;
  sm_type: decl_ty;
  sm_visibility: Ast_defs.visibility;
  sm_deprecated: string option;
  sm_flags: MethodFlags.t;
}
[@@deriving eq, show]

type shallow_class = {
  sc_mode: FileInfo.mode;
  sc_final: bool;
  sc_is_xhp: bool;
  sc_has_xhp_keyword: bool;
  sc_kind: Ast_defs.class_kind;
  sc_name: Typing_defs.pos_id;
  sc_tparams: decl_tparam list;
  sc_where_constraints: decl_where_constraint list;
  sc_extends: decl_ty list;
  sc_uses: decl_ty list;
  sc_xhp_attr_uses: decl_ty list;
  sc_req_extends: decl_ty list;
  sc_req_implements: decl_ty list;
  sc_implements: decl_ty list;
  sc_implements_dynamic: bool;
  sc_consts: shallow_class_const list;
  sc_typeconsts: shallow_typeconst list;
  sc_props: shallow_prop list;
  sc_sprops: shallow_prop list;
  sc_constructor: shallow_method option;
  sc_static_methods: shallow_method list;
  sc_methods: shallow_method list;
  sc_user_attributes: user_attribute list;
  sc_enum_type: enum_type option;
}
[@@deriving eq, show]

let sp_abstract sp = PropFlags.get_abstract sp.sp_flags

let sp_const sp = PropFlags.get_const sp.sp_flags

let sp_lateinit sp = PropFlags.get_lateinit sp.sp_flags

let sp_lsb sp = PropFlags.get_lsb sp.sp_flags

let sp_needs_init sp = PropFlags.get_needs_init sp.sp_flags

let sp_php_std_lib sp = PropFlags.get_php_std_lib sp.sp_flags

let sp_readonly sp = PropFlags.get_readonly sp.sp_flags

let sm_abstract sm = MethodFlags.get_abstract sm.sm_flags

let sm_final sm = MethodFlags.get_final sm.sm_flags

let sm_override sm = MethodFlags.get_override sm.sm_flags

let sm_dynamicallycallable sm = MethodFlags.get_dynamicallycallable sm.sm_flags

let sm_php_std_lib sm = MethodFlags.get_php_std_lib sm.sm_flags

let sm_sound_dynamic_callable sm =
  MethodFlags.get_sound_dynamic_callable sm.sm_flags

type fun_decl = fun_elt [@@deriving show]

type class_decl = shallow_class [@@deriving show]

type record_decl = record_def_type [@@deriving show]

type typedef_decl = typedef_type [@@deriving show]

type const_decl = Typing_defs.const_decl [@@deriving show]

type decl =
  | Class of class_decl
  | Fun of fun_decl
  | Record of record_decl
  | Typedef of typedef_decl
  | Const of const_decl
[@@deriving show]
