(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Decl_defs
open Decl_heap
open Reordered_argument_collections
open Pp_type
open Typing_defs

module CEKMap = struct
  include Reordered_argument_map(MyMap.Make(ClassEltKey))
  let pp ppd = make_pp (fun fmt (c,m) -> Format.fprintf fmt "(%S, %S)" c m) ppd
end

type saved_decls = {
  classes : decl_class_type SMap.t;
  props   : decl ty CEKMap.t;
  sprops  : decl ty CEKMap.t;
  meths   : decl fun_type CEKMap.t;
  smeths  : decl fun_type CEKMap.t;
  cstrs   : decl fun_type SMap.t;
  fixmes  : Pos.t IMap.t IMap.t Relative_path.Map.t;
  decl_fixmes : Pos.t IMap.t IMap.t Relative_path.Map.t;
} [@@deriving show]

let empty_decls = {
  classes = SMap.empty;
  props   = CEKMap.empty;
  sprops  = CEKMap.empty;
  meths   = CEKMap.empty;
  smeths  = CEKMap.empty;
  cstrs   = SMap.empty;
  fixmes  = Relative_path.Map.empty;
  decl_fixmes = Relative_path.Map.empty;
}

let keys_to_sset smap =
  SMap.fold smap ~init:SSet.empty ~f:(fun k _ s -> SSet.add s k)

let get_class_filename cid =
  match Naming_table.Types.get_pos cid with
  | None | Some (_, Naming_table.TTypedef) -> None
  | Some (pos, Naming_table.TClass) -> Some (FileInfo.get_pos_filename pos)

let rec collect_class
    ?(fail_if_missing=false)
    (requested_classes : SSet.t)
    (cid : string)
    (decls : saved_decls)
  : saved_decls =
  if SMap.mem decls.classes cid then decls else
  let kind =
    if SSet.mem requested_classes cid then "requested" else "ancestor" in
  match Classes.get cid with
  | None ->
    if fail_if_missing
    then failwith @@ "Missing "^kind^" class "^cid^" after declaration"
    else begin
      try
        match get_class_filename cid with
        | None -> raise Exit
        | Some filename ->
          Hh_logger.log "Declaring %s class %s" kind cid;
          Decl.declare_class_in_file filename cid;
          collect_class requested_classes cid decls ~fail_if_missing:true
      with Exit | Decl_not_found _ ->
        if not @@ SSet.mem requested_classes cid
        then failwith @@ "Missing ancestor class "^cid
        else begin
          Hh_logger.log "Missing requested class %s" cid;
          if Typedefs.mem cid
          then Hh_logger.log "(It may have been changed to a typedef)"
          else Hh_logger.log "(It may have been renamed or deleted)";
          decls
        end
    end
  | Some data ->
    let decls = {decls with classes = SMap.add decls.classes cid data} in
    let collect_elt add mid {elt_origin; _} decls =
      if cid = elt_origin then add decls cid mid else decls
    in
    let collect_elts elts init f = SMap.fold elts ~init ~f:(collect_elt f) in
    let decls = collect_elts data.dc_props decls @@ fun decls cid mid ->
      match Props.get (cid, mid) with
      | None -> failwith @@ "Missing property "^cid^"::"^mid
      | Some x -> {decls with props = CEKMap.add decls.props (cid, mid) x}
    in
    let decls = collect_elts data.dc_sprops decls @@ fun decls cid mid ->
      match StaticProps.get (cid, mid) with
      | None -> failwith @@ "Missing static property "^cid^"::"^mid
      | Some x -> {decls with sprops = CEKMap.add decls.sprops (cid, mid) x}
    in
    let decls = collect_elts data.dc_methods decls @@ fun decls cid mid ->
      match Methods.get (cid, mid) with
      | None -> failwith @@ "Missing method "^cid^"::"^mid
      | Some x -> {decls with meths = CEKMap.add decls.meths (cid, mid) x}
    in
    let decls = collect_elts data.dc_smethods decls @@ fun decls cid mid ->
      match StaticMethods.get (cid, mid) with
      | None -> failwith @@ "Missing static method "^cid^"::"^mid
      | Some x -> {decls with smeths = CEKMap.add decls.smeths (cid, mid) x}
    in
    let decls =
      fst data.dc_construct
      |> Option.value_map ~default:decls ~f:begin fun {elt_origin; _} ->
        if cid <> elt_origin then decls else
        match Constructors.get cid with
        | None -> failwith @@ "Missing constructor "^cid
        | Some x -> {decls with cstrs = SMap.add decls.cstrs cid x}
      end
    in
    let filename =
      match get_class_filename cid with
      | None -> failwith @@ "Could not look up filename for "^kind^" class "^cid
      | Some f -> f
    in
    let decls =
      if Relative_path.Map.mem decls.fixmes filename
      || Relative_path.Map.mem decls.decl_fixmes filename
      then decls
      else
        match Fixmes.HH_FIXMES.get filename with
        | Some fixmes ->
          {decls with fixmes =
            Relative_path.Map.add decls.fixmes filename fixmes}
        | None ->
          match Fixmes.DECL_HH_FIXMES.get filename with
          | Some fixmes ->
            {decls with decl_fixmes =
              Relative_path.Map.add decls.decl_fixmes filename fixmes}
          | None -> decls
    in
    let ancestors =
      keys_to_sset data.dc_ancestors
      |> SSet.union data.dc_xhp_attr_deps
      |> SSet.union data.dc_req_ancestors_extends
    in
    collect_classes requested_classes decls ancestors

and collect_classes requested_classes decls =
  SSet.fold ~init:decls ~f:(collect_class requested_classes)

let restore_decls decls =
  let {classes; props; sprops; meths; smeths; cstrs; fixmes; decl_fixmes} = decls in
  SMap.iter classes Classes.add;
  CEKMap.iter props Props.add;
  CEKMap.iter sprops StaticProps.add;
  CEKMap.iter meths Methods.add;
  CEKMap.iter smeths StaticMethods.add;
  SMap.iter cstrs Constructors.add;
  Relative_path.Map.iter fixmes Fixmes.HH_FIXMES.add;
  Relative_path.Map.iter decl_fixmes Fixmes.DECL_HH_FIXMES.add

let export_class_decls classes =
  collect_classes classes empty_decls classes

let import_class_decls decls =
  restore_decls decls;
  keys_to_sset decls.classes
