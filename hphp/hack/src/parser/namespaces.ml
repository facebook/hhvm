(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Namespace_env
module SN = Naming_special_names

type elaborate_kind =
  | ElaborateFun
  | ElaborateClass
  | ElaborateRecord
  | ElaborateConst

let elaborate_into_ns ns_name id =
  match ns_name with
  | None -> "\\" ^ id
  | Some ns -> "\\" ^ ns ^ "\\" ^ id

let elaborate_into_current_ns nsenv id = elaborate_into_ns nsenv.ns_name id

(* If the given id is an xhp id, for example :foo:bar
 * we will pull it apart into foo and bar, then reassemble
 * into \foo\bar. This gives us the fully qualified name
 * in a way that the rest of elaborate_id_impl expects.
 *)
let elaborate_xhp_namespace id =
  let is_xhp s = s <> "" && String.contains s ':' in

  if is_xhp id then
    Str.global_replace (Str.regexp ":") "\\\\" id
  else
    id

(* Elaborate a defined identifier in a given namespace environment. For example,
 * a class might be defined inside a namespace.
 *)
let elaborate_defined_id nsenv (p, id) =
  if nsenv.ns_disable_xhp_element_mangling && String.contains id ':' && id <> ""
  then
    let id = elaborate_xhp_namespace id in
    if id.[0] = '\\' then
      (p, id)
    else
      (p, elaborate_into_current_ns nsenv id)
  else
    (p, elaborate_into_current_ns nsenv id)

(* Resolves an identifier in a given namespace environment. For example, if we
 * are in the namespace "N\O", the identifier "P\Q" is resolved to "\N\O\P\Q".
 *
 * All identifiers are fully-qualified by this function; the internal
 * representation of identifiers inside the typechecker after naming is a fully
 * qualified identifier.
 *
 * It's extremely important that this function is idempotent. We actually
 * normalize identifiers in two phases. Right after parsing, we need to have
 * the class hierarchy normalized so that we can recompute dependencies for
 * incremental mode properly. Other identifiers are normalized during naming.
 * However, we don't do any bookkeeping to determine which we've normalized or
 * not, just relying on the idempotence of this function to make sure everything
 * works out. (Fully qualifying identifiers is of course idempotent, but there
 * used to be other schemes here.)
 *)
let elaborate_raw_id nsenv kind id =
  (* in case we've found an xhp id let's do some preparation to get it into the \namespace\xhp format *)
  let id =
    if kind = ElaborateClass && nsenv.ns_disable_xhp_element_mangling then
      elaborate_xhp_namespace id
    else
      id
  in

  if id <> "" && id.[0] = '\\' then
    id
  else
    let fqid = Utils.add_ns id in
    match kind with
    | ElaborateConst when SN.PseudoConsts.is_pseudo_const fqid -> fqid
    | ElaborateFun when SN.PseudoFunctions.is_pseudo_function fqid -> fqid
    | ElaborateClass when SN.Typehints.is_reserved_global_name id -> fqid
    | ElaborateClass when SN.Typehints.is_reserved_hh_name id ->
      if nsenv.ns_is_codegen then
        elaborate_into_ns (Some "HH") id
      else
        fqid
    | _ ->
      let (prefix, has_bslash) =
        match String.index id '\\' with
        | Some i -> (String.sub id 0 i, true)
        | None -> (id, false)
      in
      if has_bslash && prefix = "namespace" then
        elaborate_into_current_ns nsenv (String_utils.lstrip id "namespace\\")
      else
        let uses =
          match kind with
          | _ when has_bslash -> nsenv.ns_ns_uses
          | ElaborateClass -> nsenv.ns_class_uses
          | ElaborateFun -> nsenv.ns_fun_uses
          | ElaborateConst -> nsenv.ns_const_uses
          | ElaborateRecord -> nsenv.ns_record_def_uses
        in
        (match SMap.find_opt prefix uses with
        | Some use -> Utils.add_ns (use ^ String_utils.lstrip id prefix)
        | None -> elaborate_into_current_ns nsenv id)

let elaborate_id nsenv kind (p, id) = (p, elaborate_raw_id nsenv kind id)

(* First pass of flattening namespaces, run super early in the pipeline, right
 * after parsing.
 *
 * Fully-qualifies the things we need for Parsing_service.AddDeps -- the classes
 * we extend, traits we use, interfaces we implement; along with classes we
 * define. So that we can also use them to figure out fallback behavior, we also
 * fully-qualify functions that we define, even though AddDeps doesn't need
 * them this early.
 *
 * Note that, since AddDeps doesn't need it, we don't recursively traverse
 * through Happly in hints -- we rely on the idempotence of elaborate_id to
 * allow us to fix those up during a second pass during naming.
 *)
module ElaborateDefs = struct
  open Aast

  let hint nsenv = function
    | (p, Happly (id, args)) ->
      (p, Happly (elaborate_id nsenv ElaborateClass id, args))
    | other -> other

  let rec def nsenv = function
    (*
      The default namespace in php is the global namespace specified by
      the empty string. In the case of an empty string, we model it as
      the global namespace.

      We remove namespace and use nodes and replace them with
      SetNamespaceEnv nodes that contain the namespace environment
    *)
    | Namespace ((_, nsname), prog) ->
      let parent_nsname =
        Option.value_map nsenv.ns_name ~default:"" ~f:(fun n -> n ^ "\\")
      in
      let nsname =
        match nsname with
        | "" -> None
        | _ -> Some (parent_nsname ^ nsname)
      in
      let new_nsenv = { nsenv with ns_name = nsname } in
      (nsenv, SetNamespaceEnv new_nsenv :: program new_nsenv prog)
    | NamespaceUse l ->
      let nsenv =
        List.fold_left l ~init:nsenv ~f:(fun nsenv (kind, id1, id2) ->
            match kind with
            | NSNamespace ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_ns_uses in
              { nsenv with ns_ns_uses = m }
            | NSClass ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_class_uses in
              { nsenv with ns_class_uses = m }
            | NSClassAndNamespace ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_class_uses in
              let n = SMap.add (snd id2) (snd id1) nsenv.ns_ns_uses in
              { nsenv with ns_class_uses = m; ns_ns_uses = n }
            | NSFun ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_fun_uses in
              { nsenv with ns_fun_uses = m }
            | NSConst ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_const_uses in
              { nsenv with ns_const_uses = m })
      in
      (nsenv, [SetNamespaceEnv nsenv])
    | Class c ->
      let name = elaborate_defined_id nsenv c.c_name in
      ( nsenv,
        [
          Class
            {
              c with
              c_name = name;
              c_extends = List.map c.c_extends (hint nsenv);
              c_reqs = List.map c.c_reqs (fun (h, e) -> (hint nsenv h, e));
              c_implements = List.map c.c_implements (hint nsenv);
              c_uses = List.map c.c_uses (hint nsenv);
              c_xhp_attr_uses = List.map c.c_xhp_attr_uses (hint nsenv);
              c_namespace = nsenv;
            };
        ] )
    | RecordDef rd ->
      let name = elaborate_defined_id nsenv rd.rd_name in
      (nsenv, [RecordDef { rd with rd_name = name; rd_namespace = nsenv }])
    | Fun f ->
      let name = elaborate_defined_id nsenv f.f_name in
      (nsenv, [Fun { f with f_name = name; f_namespace = nsenv }])
    | Typedef t ->
      let name = elaborate_defined_id nsenv t.t_name in
      (nsenv, [Typedef { t with t_name = name; t_namespace = nsenv }])
    | Constant cst ->
      let name = elaborate_defined_id nsenv cst.cst_name in
      (nsenv, [Constant { cst with cst_name = name; cst_namespace = nsenv }])
    | FileAttributes fa ->
      (nsenv, [FileAttributes { fa with fa_namespace = nsenv }])
    | other -> (nsenv, [other])

  and attach_file_attributes p =
    let file_attributes =
      List.filter_map p ~f:(function
          | FileAttributes fa -> Some fa
          | _ -> None)
    in
    List.map p ~f:(function
        | Class c -> Class { c with c_file_attributes = file_attributes }
        | Fun f -> Fun { f with f_file_attributes = file_attributes }
        | x -> x)

  and program nsenv p =
    let (_, p) =
      List.fold_left p ~init:(nsenv, []) ~f:(fun (nsenv, acc) item ->
          let (nsenv, item) = def nsenv item in
          (nsenv, item :: acc))
    in
    p |> List.rev |> List.concat |> attach_file_attributes
end

let elaborate_toplevel_defs popt ast =
  ElaborateDefs.program (Namespace_env.empty_from_popt popt) ast
