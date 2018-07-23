(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module SN = Naming_special_names

include Aast

(* The NAST definitions, which we just include into this file *)
module PosAnnotation = struct type t = Pos.t [@@deriving show] end
module UnitAnnotation = struct type t = unit [@@deriving show] end

module Annotations = struct
  module ExprAnnotation = PosAnnotation
  module EnvAnnotation = UnitAnnotation
  module ClassIdAnnotation = PosAnnotation
end

module PosAnnotatedAST = AnnotatedAST(Annotations)
include PosAnnotatedAST

(* Expecting that Naming.func_body / Naming.class_meth_bodies has been
 * allowed at the AST. Ideally this would be enforced by the compiler,
 * a la the typechecking decl vs local phases *)
let assert_named_body = function
  | NamedBody b -> b
  | UnnamedBody _ -> failwith "Expecting a named function body"

let get_instantiated_sid_name ((_, x), _) = x

let class_id_to_str = function
  | CIparent -> SN.Classes.cParent
  | CIself -> SN.Classes.cSelf
  | CIstatic -> SN.Classes.cStatic
  | CIexpr (_, This) -> SN.SpecialIdents.this
  | CIexpr (_, Lvar (_, x)) -> "$"^Local_id.to_string x
  | CIexpr _ -> assert false
  | CI x -> get_instantiated_sid_name x

let is_kvc_kind name =
  name = SN.Collections.cMap ||
  name = SN.Collections.cImmMap ||
  name = SN.Collections.cStableMap ||
  name = SN.Collections.cDict

let get_kvc_kind name = match name with
  | x when x = SN.Collections.cMap -> `Map
  | x when x = SN.Collections.cImmMap -> `ImmMap
  | x when x = SN.Collections.cDict -> `Dict
  | _ -> begin
    Errors.internal_error Pos.none ("Invalid KeyValueCollection name: "^name);
    `Map
  end

let kvc_kind_to_name kind = match kind with
  | `Map -> SN.Collections.cMap
  | `ImmMap -> SN.Collections.cImmMap
  | `Dict -> SN.Collections.cDict

let is_vc_kind name =
  name = SN.Collections.cVector ||
  name = SN.Collections.cImmVector ||
  name = SN.Collections.cSet ||
  name = SN.Collections.cImmSet ||
  name = SN.Collections.cKeyset ||
  name = SN.Collections.cVec

let get_vc_kind name = match name with
  | x when x = SN.Collections.cVector -> `Vector
  | x when x = SN.Collections.cImmVector -> `ImmVector
  | x when x = SN.Collections.cVec -> `Vec
  | x when x = SN.Collections.cSet -> `Set
  | x when x = SN.Collections.cImmSet -> `ImmSet
  | x when x = SN.Collections.cKeyset -> `Keyset
  | _ -> begin
    Errors.internal_error Pos.none ("Invalid ValueCollection name: "^name);
    `Set
  end

let vc_kind_to_name kind = match kind with
  | `Vector -> SN.Collections.cVector
  | `ImmVector -> SN.Collections.cImmVector
  | `Vec -> SN.Collections.cVec
  | `Set -> SN.Collections.cSet
  | `ImmSet -> SN.Collections.cImmSet
  | `Keyset -> SN.Collections.cKeyset
  | `Pair -> SN.Collections.cPair

(* XHP attribute helpers *)
let map_xhp_attr (f: pstring -> pstring) (g: expr -> expr) = function
  | Xhp_simple (id, e) -> Xhp_simple (f id, g e)
  | Xhp_spread e -> Xhp_spread (g e)

let get_xhp_attr_expr = function
  | Xhp_simple (_, e)
  | Xhp_spread e -> e

let get_simple_xhp_attrs =
  Hh_core.List.filter_map ~f:(function Xhp_simple (id, e) -> Some (id, e) | Xhp_spread _ -> None)
