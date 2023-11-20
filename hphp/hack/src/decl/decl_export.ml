(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Reordered_argument_collections
open Typing_defs

module CEKMap = struct
  include Reordered_argument_map (WrappedMap.Make (Decl_heap.ClassEltKey))

  let pp ppd = make_pp (fun fmt (c, m) -> Format.fprintf fmt "(%S, %S)" c m) ppd
end

type saved_legacy_decls = {
  classes: Decl_defs.decl_class_type SMap.t;
  props: decl_ty CEKMap.t;
  sprops: decl_ty CEKMap.t;
  meths: fun_elt CEKMap.t;
  smeths: fun_elt CEKMap.t;
  cstrs: fun_elt SMap.t;
  fixmes: Pos.t IMap.t IMap.t Relative_path.Map.t;
  decl_fixmes: Pos.t IMap.t IMap.t Relative_path.Map.t;
}
[@@deriving show]

let empty_legacy_decls =
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

let rec collect_legacy_class
    ?(fail_if_missing = false)
    (ctx : Provider_context.t)
    (requested_classes : SSet.t)
    (cid : string)
    (decls : saved_legacy_decls) : saved_legacy_decls =
  let open Decl_defs in
  if SMap.mem decls.classes cid then
    decls
  else
    let kind =
      if SSet.mem requested_classes cid then
        "requested"
      else
        "ancestor"
    in
    match Decl_heap.Classes.get cid with
    | None ->
      if fail_if_missing then
        failwith @@ "Missing " ^ kind ^ " class " ^ cid ^ " after declaration"
      else (
        try
          match Naming_provider.get_class_path ctx cid with
          | None -> raise Exit
          | Some _filename ->
            Hh_logger.log "Declaring %s class %s" kind cid;

            (* NOTE: the following relies on the fact that declaring a class puts
             * the inheritance hierarchy into the shared memory heaps. When that
             * invariant no longer holds, the following will no longer work. *)
            let (_ : _ option) =
              Errors.run_in_decl_mode (fun () ->
                  Decl_folded_class.class_decl_if_missing
                    ~sh:SharedMem.Uses
                    ctx
                    cid)
            in
            collect_legacy_class
              ctx
              requested_classes
              cid
              decls
              ~fail_if_missing:true
        with
        | Exit
        | Decl_defs.Decl_not_found _ ->
          if not @@ SSet.mem requested_classes cid then
            failwith @@ "Missing legacy ancestor class " ^ cid
          else (
            Hh_logger.log "Missing legacy requested class %s" cid;
            if Decl_heap.Typedefs.mem cid then
              Hh_logger.log "(It may have been changed to a typedef)"
            else
              Hh_logger.log "(It may have been renamed or deleted)";
            decls
          )
      )
    | Some data ->
      let decls =
        { decls with classes = SMap.add decls.classes ~key:cid ~data }
      in
      let collect_elt add mid { elt_origin; _ } decls =
        if String.equal cid elt_origin then
          add decls cid mid
        else
          decls
      in
      let collect_elts elts init f = SMap.fold elts ~init ~f:(collect_elt f) in
      let decls =
        collect_elts data.dc_props decls @@ fun decls cid mid ->
        match Decl_heap.Props.get (cid, mid) with
        | None -> failwith @@ "Missing property " ^ cid ^ "::" ^ mid
        | Some x ->
          { decls with props = CEKMap.add decls.props ~key:(cid, mid) ~data:x }
      in
      let decls =
        collect_elts data.dc_sprops decls @@ fun decls cid mid ->
        match Decl_heap.StaticProps.get (cid, mid) with
        | None -> failwith @@ "Missing static property " ^ cid ^ "::" ^ mid
        | Some x ->
          {
            decls with
            sprops = CEKMap.add decls.sprops ~key:(cid, mid) ~data:x;
          }
      in
      let decls =
        collect_elts data.dc_methods decls @@ fun decls cid mid ->
        match Decl_heap.Methods.get (cid, mid) with
        | None -> failwith @@ "Missing method " ^ cid ^ "::" ^ mid
        | Some x ->
          { decls with meths = CEKMap.add decls.meths ~key:(cid, mid) ~data:x }
      in
      let decls =
        collect_elts data.dc_smethods decls @@ fun decls cid mid ->
        match Decl_heap.StaticMethods.get (cid, mid) with
        | None -> failwith @@ "Missing static method " ^ cid ^ "::" ^ mid
        | Some x ->
          {
            decls with
            smeths = CEKMap.add decls.smeths ~key:(cid, mid) ~data:x;
          }
      in
      let decls =
        fst data.dc_construct
        |> Option.value_map ~default:decls ~f:(fun { elt_origin; _ } ->
               if String.( <> ) cid elt_origin then
                 decls
               else
                 match Decl_heap.Constructors.get cid with
                 | None -> failwith @@ "Missing constructor " ^ cid
                 | Some x ->
                   { decls with cstrs = SMap.add decls.cstrs ~key:cid ~data:x })
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
              fixmes =
                Relative_path.Map.add decls.fixmes ~key:filename ~data:fixmes;
            }
          | None ->
            (match Fixme_provider.get_decl_hh_fixmes filename with
            | Some fixmes ->
              {
                decls with
                decl_fixmes =
                  Relative_path.Map.add
                    decls.decl_fixmes
                    ~key:filename
                    ~data:fixmes;
              }
            | None -> decls)
      in
      let ancestors =
        keys_to_sset data.dc_ancestors
        |> SSet.union data.dc_xhp_attr_deps
        |> SSet.union data.dc_req_ancestors_extends
      in
      collect_legacy_classes ctx requested_classes decls ancestors

and collect_legacy_classes ctx requested_classes decls =
  SSet.fold ~init:decls ~f:(collect_legacy_class ctx requested_classes)

let restore_legacy_decls decls =
  let { classes; props; sprops; meths; smeths; cstrs; fixmes; decl_fixmes } =
    decls
  in
  SMap.iter classes ~f:Decl_heap.Classes.add;
  CEKMap.iter props ~f:Decl_heap.Props.add;
  CEKMap.iter sprops ~f:Decl_heap.StaticProps.add;
  CEKMap.iter meths ~f:Decl_heap.Methods.add;
  CEKMap.iter smeths ~f:Decl_heap.StaticMethods.add;
  SMap.iter cstrs ~f:Decl_heap.Constructors.add;
  Relative_path.Map.iter fixmes ~f:Fixme_provider.provide_hh_fixmes;
  Relative_path.Map.iter decl_fixmes ~f:Fixme_provider.provide_decl_hh_fixmes;
  (* return the number of classes that we restored *)
  SMap.cardinal classes

let collect_legacy_decls ctx classes =
  collect_legacy_classes ctx classes empty_legacy_decls classes

type saved_shallow_decls = { classes: Shallow_decl_defs.shallow_class SMap.t }
[@@deriving show]

let collect_shallow_decls ctx workers classnames =
  let classnames = SSet.elements classnames in
  (* We're only going to fetch the shallow-decls that were explicitly listed;
     we won't look for ancestors. *)
  let job (init : 'a SMap.t) (classnames : string list) : 'a SMap.t =
    List.fold classnames ~init ~f:(fun acc cid ->
        match Decl_provider.get_shallow_class ctx cid with
        | None ->
          Hh_logger.log "Missing requested shallow class %s" cid;
          acc
        | Some data -> SMap.add acc ~key:cid ~data)
  in
  (* The 'classnames' came from a SSet, and therefore all elements are unique.
     So we can safely assume there will be no merge collisions. *)
  let classes =
    MultiWorker.call
      workers
      ~job
      ~neutral:SMap.empty
      ~merge:
        (SMap.merge ~f:(fun _key a b ->
             if Option.is_some a then
               a
             else
               b))
      ~next:(MultiWorker.next workers classnames)
  in
  { classes }

let restore_shallow_decls decls =
  SMap.iter decls.classes ~f:(fun name cls ->
      Shallow_classes_heap.Classes.add name cls);
  (* return the number of classes that we restored *)
  SMap.cardinal decls.classes
