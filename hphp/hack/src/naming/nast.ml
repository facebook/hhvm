(* @generated from nast.src.ml by hphp/hack/tools/ppx/facebook:generate_ppx *)
(* Copyright (c) 2017, Facebook, Inc. All rights reserved. *)
(* SourceShasum<<5d77d0f06489b2e7f8677bf131d2ca5bd39bb5b3>> *)

(* DO NOT EDIT MANUALLY. *)
[@@@ocaml.text
  "\n * Copyright (c) 2015, Facebook, Inc.\n * All rights reserved.\n *\n * This source code is licensed under the BSD-style license found in the\n * LICENSE file in the \"hack\" directory of this source tree. An additional grant\n * of patent rights can be found in the PATENTS file in the same directory.\n *\n "]
module SN = Naming_special_names
include Aast
module PosAnnotation =
  struct
    type t = Pos.t[@@deriving show]
    let rec pp : Format.formatter -> t -> Ppx_deriving_runtime.unit =
      let __0 () = Pos.pp  in
      ((let open! Ppx_deriving_runtime in fun fmt  -> (__0 ()) fmt)
        [@ocaml.warning "-A"])
    
    and show : t -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp x
    
  end
module UnitAnnotation =
  struct
    type t = unit[@@deriving show]
    let rec (pp : Format.formatter -> t -> Ppx_deriving_runtime.unit) =
      ((let open! Ppx_deriving_runtime in
          fun fmt  -> fun ()  -> Format.pp_print_string fmt "()")
      [@ocaml.warning "-A"])
    
    and show : t -> Ppx_deriving_runtime.string =
      fun x  -> Format.asprintf "%a" pp x
    
  end
module Annotations =
  struct
    module ExprAnnotation = PosAnnotation
    module EnvAnnotation = UnitAnnotation
  end
module PosAnnotatedAST = (AnnotatedAST)(Annotations)
include PosAnnotatedAST
let assert_named_body =
  function
  | NamedBody b -> b
  | UnnamedBody _ -> failwith "Expecting a named function body" 
let get_instantiated_sid_name ((_,x),_) = x 
let class_id_to_str =
  function
  | CIparent  -> SN.Classes.cParent
  | CIself  -> SN.Classes.cSelf
  | CIstatic  -> SN.Classes.cStatic
  | CIexpr (_,This ) -> SN.SpecialIdents.this
  | CIexpr (_,Lvar (_,x)) -> "$" ^ (Local_id.to_string x)
  | CIexpr _ -> assert false
  | CI x -> get_instantiated_sid_name x 
let is_kvc_kind name =
  (name = SN.Collections.cMap) ||
    ((name = SN.Collections.cImmMap) ||
       ((name = SN.Collections.cStableMap) || (name = SN.Collections.cDict)))
  
let get_kvc_kind name =
  match name with
  | x when x = SN.Collections.cMap -> `Map
  | x when x = SN.Collections.cImmMap -> `ImmMap
  | x when x = SN.Collections.cDict -> `Dict
  | _ ->
      (Errors.internal_error Pos.none
         ("Invalid KeyValueCollection name: " ^ name);
       `Map)
  
let kvc_kind_to_name kind =
  match kind with
  | `Map -> SN.Collections.cMap
  | `ImmMap -> SN.Collections.cImmMap
  | `Dict -> SN.Collections.cDict 
let is_vc_kind name =
  (name = SN.Collections.cVector) ||
    ((name = SN.Collections.cImmVector) ||
       ((name = SN.Collections.cSet) ||
          ((name = SN.Collections.cImmSet) ||
             ((name = SN.Collections.cKeyset) || (name = SN.Collections.cVec)))))
  
let get_vc_kind name =
  match name with
  | x when x = SN.Collections.cVector -> `Vector
  | x when x = SN.Collections.cImmVector -> `ImmVector
  | x when x = SN.Collections.cVec -> `Vec
  | x when x = SN.Collections.cSet -> `Set
  | x when x = SN.Collections.cImmSet -> `ImmSet
  | x when x = SN.Collections.cKeyset -> `Keyset
  | _ ->
      (Errors.internal_error Pos.none
         ("Invalid ValueCollection name: " ^ name);
       `Set)
  
let vc_kind_to_name kind =
  match kind with
  | `Vector -> SN.Collections.cVector
  | `ImmVector -> SN.Collections.cImmVector
  | `Vec -> SN.Collections.cVec
  | `Set -> SN.Collections.cSet
  | `ImmSet -> SN.Collections.cImmSet
  | `Keyset -> SN.Collections.cKeyset
  | `Pair -> SN.Collections.cPair 
let map_xhp_attr (f : pstring -> pstring) (g : expr -> expr) =
  function
  | Xhp_simple (id,e) -> Xhp_simple ((f id), (g e))
  | Xhp_spread e -> Xhp_spread (g e) 
let get_xhp_attr_expr = function | Xhp_simple (_,e)|Xhp_spread e -> e 
let get_simple_xhp_attrs =
  Hh_core.List.filter_map
    ~f:(function | Xhp_simple (id,e) -> Some (id, e) | Xhp_spread _ -> None)
  
