(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Decl_defs
open Decl_heap
open Reordered_argument_collections
open Pp_type
open Typing_defs

module CEKMap = struct
  include Reordered_argument_map (WrappedMap.Make (ClassEltKey))

  let pp ppd = make_pp (fun fmt (c, m) -> Format.fprintf fmt "(%S, %S)" c m) ppd
end

type saved_decls = {
  classes: decl_class_type SMap.t;
  props: decl_ty CEKMap.t;
  sprops: decl_ty CEKMap.t;
  meths: fun_elt CEKMap.t;
  smeths: fun_elt CEKMap.t;
  cstrs: fun_elt SMap.t;
  fixmes: Pos.t IMap.t IMap.t Relative_path.Map.t;
  decl_fixmes: Pos.t IMap.t IMap.t Relative_path.Map.t;
}
[@@deriving show]

let empty_decls =
  {
    classes = SMap.empty;
    props = CEKMap.empty;
    sprops = CEKMap.empty;
    meths = CEKMap.empty;
    smeths = CEKMap.empty;
    cstrs = SMap.empty;
    fixmes = Relative_path.Map.empty;
    decl_fixmes = Relative_path.Map.empty;
  }

let keys_to_sset smap =
  SMap.fold smap ~init:SSet.empty ~f:(fun k _ s -> SSet.add s k)

let rec collect_class
    ?(fail_if_missing = false)
    (ctx : Provider_context.t)
    (requested_classes : SSet.t)
    (cid : string)
    (decls : saved_decls) : saved_decls =
  if SMap.mem decls.classes cid then
    decls
  else
    let kind =
      if SSet.mem requested_classes cid then
        "requested"
      else
        "ancestor"
    in
    match Classes.get cid with
    | None ->
      if fail_if_missing then
        failwith @@ "Missing " ^ kind ^ " class " ^ cid ^ " after declaration"
      else (
        try
          match Naming_provider.get_class_path ctx cid with
          | None -> raise Exit
          | Some filename ->
            Hh_logger.log "Declaring %s class %s" kind cid;

            (* NOTE: the following relies on the fact that declaring a class puts
          the inheritance hierarchy into the shared memory heaps. When that
          invariant no longer holds, the following will no longer work. *)
            let (_ : Decl_defs.decl_class_type option) =
              Decl.declare_class_in_file ~sh:SharedMem.Uses ctx filename cid
            in
            collect_class ctx requested_classes cid decls ~fail_if_missing:true
        with
        | Exit
        | Decl_not_found _ ->
          if not @@ SSet.mem requested_classes cid then
            failwith @@ "Missing ancestor class " ^ cid
          else (
            Hh_logger.log "Missing requested class %s" cid;
            if Typedefs.mem cid then
              Hh_logger.log "(It may have been changed to a typedef)"
            else
              Hh_logger.log "(It may have been renamed or deleted)";
            decls
          )
      )
    | Some data ->
      let decls = { decls with classes = SMap.add decls.classes cid data } in
      let collect_elt add mid { elt_origin; _ } decls =
        if String.equal cid elt_origin then
          add decls cid mid
        else
          decls
      in
      let collect_elts elts init f = SMap.fold elts ~init ~f:(collect_elt f) in
      let decls =
        collect_elts data.dc_props decls @@ fun decls cid mid ->
        match Props.get (cid, mid) with
        | None -> failwith @@ "Missing property " ^ cid ^ "::" ^ mid
        | Some x -> { decls with props = CEKMap.add decls.props (cid, mid) x }
      in
      let decls =
        collect_elts data.dc_sprops decls @@ fun decls cid mid ->
        match StaticProps.get (cid, mid) with
        | None -> failwith @@ "Missing static property " ^ cid ^ "::" ^ mid
        | Some x -> { decls with sprops = CEKMap.add decls.sprops (cid, mid) x }
      in
      let decls =
        collect_elts data.dc_methods decls @@ fun decls cid mid ->
        match Methods.get (cid, mid) with
        | None -> failwith @@ "Missing method " ^ cid ^ "::" ^ mid
        | Some x -> { decls with meths = CEKMap.add decls.meths (cid, mid) x }
      in
      let decls =
        collect_elts data.dc_smethods decls @@ fun decls cid mid ->
        match StaticMethods.get (cid, mid) with
        | None -> failwith @@ "Missing static method " ^ cid ^ "::" ^ mid
        | Some x -> { decls with smeths = CEKMap.add decls.smeths (cid, mid) x }
      in
      let decls =
        fst data.dc_construct
        |> Option.value_map ~default:decls ~f:(fun { elt_origin; _ } ->
               if String.( <> ) cid elt_origin then
                 decls
               else
                 match Constructors.get cid with
                 | None -> failwith @@ "Missing constructor " ^ cid
                 | Some x -> { decls with cstrs = SMap.add decls.cstrs cid x })
      in
      let filename =
        match Naming_provider.get_class_path ctx cid with
        | None ->
          failwith @@ "Could not look up filename for " ^ kind ^ " class " ^ cid
        | Some f -> f
      in
      let decls =
        if
          Relative_path.Map.mem decls.fixmes filename
          || Relative_path.Map.mem decls.decl_fixmes filename
        then
          decls
        else
          match Fixme_provider.get_hh_fixmes filename with
          | Some fixmes ->
            {
              decls with
              fixmes = Relative_path.Map.add decls.fixmes filename fixmes;
            }
          | None ->
            (match Fixme_provider.get_decl_hh_fixmes filename with
            | Some fixmes ->
              {
                decls with
                decl_fixmes =
                  Relative_path.Map.add decls.decl_fixmes filename fixmes;
              }
            | None -> decls)
      in
      let ancestors =
        keys_to_sset data.dc_ancestors
        |> SSet.union data.dc_xhp_attr_deps
        |> SSet.union data.dc_req_ancestors_extends
      in
      collect_classes ctx requested_classes decls ancestors

and collect_classes ctx requested_classes decls =
  SSet.fold ~init:decls ~f:(collect_class ctx requested_classes)

let restore_decls decls =
  let { classes; props; sprops; meths; smeths; cstrs; fixmes; decl_fixmes } =
    decls
  in
  SMap.iter classes Classes.add;
  CEKMap.iter props Props.add;
  CEKMap.iter sprops StaticProps.add;
  CEKMap.iter meths Methods.add;
  CEKMap.iter smeths StaticMethods.add;
  SMap.iter cstrs Constructors.add;
  Relative_path.Map.iter fixmes Fixme_provider.provide_hh_fixmes;
  Relative_path.Map.iter decl_fixmes Fixme_provider.provide_decl_hh_fixmes

let export_class_decls ctx classes =
  collect_classes ctx classes empty_decls classes

let import_class_decls decls =
  restore_decls decls;
  keys_to_sset decls.classes
