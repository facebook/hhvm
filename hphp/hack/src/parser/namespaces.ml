(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Ast
open Namespace_env
module Nast = Aast
module SN = Naming_special_names

(* The typechecker has a different view of HH autoimporting than the compiler.
 * This type exists to track the unification of HH autoimporting between the
 * typechecker and the compiler, after which it will be deleted. *)
type autoimport_ns =
  | Global
  | HH

type elaborate_kind =
  | ElaborateFun
  | ElaborateClass
  | ElaborateRecord
  | ElaborateConst

(**
 * Convenience function for maps that have been completely unified to autoimport
 * into the HH namespace.
 *)
let autoimport_map_of_list ids =
  List.fold_left ids ~init:SMap.empty ~f:(fun map id -> SMap.add id HH map)

(* When dealing with an <?hh file, HHVM automatically imports a few
 * "core" classes into every namespace, mostly collections. Their
 * unqualified names always refer to this global version.
 *
 * Note that these are technically in the \HH namespace as far as the
 * runtime is concerned, but we treat them as in the global
 * namespace. This is a tiny bit weird, but since Facebook www all runs
 * in the global namespace relying on this autoimport, this makes the
 * most sense there.
 *)
let autoimport_types =
  SMap.of_list
    [
      ("AsyncFunctionWaitHandle", Global);
      ("AsyncGenerator", Global);
      ("AsyncGeneratorWaitHandle", Global);
      ("AsyncIterator", Global);
      ("AsyncKeyedIterator", Global);
      ("Awaitable", Global);
      ("AwaitAllWaitHandle", Global);
      ("classname", Global);
      ("Collection", Global);
      ("ConditionWaitHandle", Global);
      ("Container", Global);
      ("dict", Global);
      ("ExternalThreadEventWaitHandle", Global);
      ("IMemoizeParam", Global);
      ("ImmMap", Global);
      ("ImmSet", Global);
      ("ImmVector", Global);
      ("InvariantException", Global);
      ("Iterable", Global);
      ("Iterator", Global);
      ("KeyedContainer", Global);
      ("KeyedIterable", Global);
      ("KeyedIterator", Global);
      ("KeyedTraversable", Global);
      ("keyset", Global);
      ("Map", Global);
      ("ObjprofObjectStats", HH);
      ("ObjprofPathsStats", HH);
      ("ObjprofStringStats", HH);
      ("Pair", Global);
      ("RescheduleWaitHandle", Global);
      ("ResumableWaitHandle", Global);
      ("Set", Global);
      ("Shapes", Global);
      ("SleepWaitHandle", Global);
      ("StableMap", Global);
      ("StaticWaitHandle", Global);
      ("Traversable", Global);
      ("typename", Global);
      ("TypeStructure", HH);
      ("TypeStructureKind", HH);
      ("vec", Global);
      ("Vector", Global);
      ("WaitableWaitHandle", Global);
      ("XenonSample", HH);
    ]

let autoimport_funcs =
  autoimport_map_of_list
    [
      "asio_get_current_context_idx";
      "asio_get_running_in_context";
      "asio_get_running";
      "class_meth";
      "darray";
      "dict";
      "fun";
      "heapgraph_create";
      "heapgraph_dfs_edges";
      "heapgraph_dfs_nodes";
      "heapgraph_edge";
      "heapgraph_foreach_edge";
      "heapgraph_foreach_node";
      "heapgraph_foreach_root";
      "heapgraph_node_in_edges";
      "heapgraph_node_out_edges";
      "heapgraph_node";
      "heapgraph_stats";
      "idx";
      "inst_meth";
      "invariant_callback_register";
      "invariant_violation";
      "invariant";
      "is_darray";
      "is_dict";
      "is_keyset";
      "is_varray";
      "is_vec";
      "keyset";
      "meth_caller";
      "objprof_get_data";
      "objprof_get_paths";
      "objprof_get_strings";
      "server_warmup_status";
      "thread_mark_stack";
      "thread_memory_stats";
      "type_structure";
      "varray";
      "vec";
      "xenon_get_data";
    ]

let autoimport_consts = autoimport_map_of_list ["Rx\\IS_ENABLED"]

(**
 * Return the namespace into which id is auto imported for the typechecker and
 * compiler, respectively. Return None if it is not auto imported.
 *)
let get_autoimport_name_namespace id kind =
  let lookup_name map = Option.map (SMap.get id map) (fun ns -> (ns, HH)) in
  match kind with
  | ElaborateClass ->
    if SN.Typehints.is_reserved_global_name id then
      Some (Global, Global)
    else if SN.Typehints.is_reserved_hh_name id then
      Some (Global, HH)
    else
      lookup_name autoimport_types
  | ElaborateRecord -> lookup_name autoimport_types
  | ElaborateFun -> lookup_name autoimport_funcs
  | ElaborateConst -> lookup_name autoimport_consts

let is_autoimport_name id kind = get_autoimport_name_namespace id kind <> None

let elaborate_into_ns ns_name id =
  match ns_name with
  | None -> "\\" ^ id
  | Some ns -> "\\" ^ ns ^ "\\" ^ id

let elaborate_into_current_ns nsenv id = elaborate_into_ns nsenv.ns_name id

(* Walks over the namespace map and checks if any source
 * matches the given id.
 * If a match is found, then removes the match and
 * replaces it with the target
 * If no match is found, returns the id
 *
 * Regularly, translates from the long name to the short name.
 * If the reverse flag is give, then translation is done
 * otherway around.*)
let rec translate_id ~reverse ns_map id =
  match ns_map with
  | [] -> id
  | (short_name, long_name) :: rest ->
    let (target, source) =
      if reverse then
        (long_name, short_name)
      else
        (short_name, long_name)
    in
    (* Append backslash at the end so that it doesn't match partially *)
    if
      String_utils.string_starts_with id (source ^ "\\")
      (* Strip out the prefix and connect it to the next beginning *)
    then
      target ^ String_utils.lstrip id source
    else
      translate_id ~reverse rest id

let aliased_to_fully_qualified_id alias_map id =
  translate_id ~reverse:true alias_map id

(* Elaborate a defined identifier in a given namespace environment. For example,
 * a class might be defined inside a namespace. Return new environment if
 * ID is auto imported (e.g. Map) and so must be mapped when used.
 *)
let elaborate_defined_id nsenv kind (p, id) =
  let newid = elaborate_into_current_ns nsenv id in
  let update_nsenv = kind = ElaborateClass && is_autoimport_name id kind in
  let nsenv =
    if update_nsenv then
      { nsenv with ns_class_uses = SMap.add id newid nsenv.ns_class_uses }
    else
      nsenv
  in
  ((p, newid), nsenv, update_nsenv)

let rec elaborate_xhp_namespace_impl parts =
  match parts with
  | [] -> ""
  | h :: [] -> ":" ^ h
  | h::t -> h ^ "\\" ^ elaborate_xhp_namespace_impl t

(* If the given id is an xhp id, for example :foo:bar
 * we will pull it apart into foo and bar, then reassemble
 * into \foo\:bar. This gives us the fully qualified name
 * in a way that the rest of elaborate_id_impl expects.
 *)
let elaborate_xhp_namespace id =
  let is_xhp s = String.length s <> 0 && s.[0] = ':' in
  let strip_colon s = String_utils.lstrip s ":" in

  if is_xhp id = false then
    id
  else
    (* all xhp ids here have a leading colon here that we don't want to split on *)
    let parts = String.split (strip_colon id) ~on: ':' in
    if List.length parts > 1 then
      elaborate_xhp_namespace_impl parts
    else
      id

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
let elaborate_id_impl nsenv kind id =
  (* in case we've found an xhp id let's do some preparation to get it into the \namespace\:xhp format *)
  let id = elaborate_xhp_namespace id in

  if id <> "" && id.[0] = '\\' then
    (false, id)
  (* The name is already fully-qualified. *)
  else
    let global_id = Utils.add_ns id in
    if kind = ElaborateConst && SN.PseudoConsts.is_pseudo_const global_id then
      (false, global_id)
    (* Pseudo-constants are always global. *)
    else if
      kind = ElaborateFun && SN.PseudoFunctions.is_pseudo_function global_id
    then
      (false, global_id)
    else
      let (bslash_loc, has_bslash) =
        match String.index id '\\' with
        | Some i -> (i, true)
        | None -> (String.length id, false)
      in
      let prefix = String.sub id 0 bslash_loc in
      if prefix = "namespace" then
        ( false,
          elaborate_into_current_ns
            nsenv
            (String_utils.lstrip id "namespace\\") )
      else
        (* Expand "use" imports. "use function" and "use const" only apply if the id
         * is completely unqualified, otherwise the normal "use" imports apply. *)
        let uses =
          if has_bslash then
            nsenv.ns_ns_uses
          else
            match kind with
            | ElaborateClass -> nsenv.ns_class_uses
            | ElaborateFun -> nsenv.ns_fun_uses
            | ElaborateConst -> nsenv.ns_const_uses
            | ElaborateRecord -> nsenv.ns_record_def_uses
        in
        match SMap.get prefix uses with
        | Some use -> (true, use ^ String_utils.lstrip id prefix)
        | None ->
          let fq_id =
            let unaliased_id =
              aliased_to_fully_qualified_id nsenv.ns_auto_ns_map id
            in
            if unaliased_id <> id then
              "\\" ^ unaliased_id
            else
              match get_autoimport_name_namespace id kind with
              | None -> elaborate_into_current_ns nsenv id
              | Some (typechecker_ns, compiler_ns) ->
                let ns =
                  if nsenv.ns_is_codegen then
                    compiler_ns
                  else
                    typechecker_ns
                in
                begin
                  match ns with
                  | Global -> elaborate_into_ns None id
                  | HH -> elaborate_into_ns (Some "HH") id
                end
          in
          (false, fq_id)

let elaborate_id nsenv kind (p, id) =
  let (_, newid) = elaborate_id_impl nsenv kind id in
  (p, newid)

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
  let hint nsenv = function
    | (p, Happly (id, args)) ->
      (p, Happly (elaborate_id nsenv ElaborateClass id, args))
    | other -> other

  let class_def nsenv = function
    | ClassUse h -> ClassUse (hint nsenv h)
    | XhpAttrUse h -> XhpAttrUse (hint nsenv h)
    | other -> other

  let finish nsenv updated_nsenv stmt =
    if updated_nsenv then
      (nsenv, [stmt; SetNamespaceEnv nsenv])
    else
      (nsenv, [stmt])

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
      let (name, nsenv, updated_nsenv) =
        elaborate_defined_id nsenv ElaborateClass c.c_name
      in
      finish
        nsenv
        updated_nsenv
        (Class
           {
             c with
             c_name = name;
             c_extends = List.map c.c_extends (hint nsenv);
             c_implements = List.map c.c_implements (hint nsenv);
             c_body = List.map c.c_body (class_def nsenv);
             c_namespace = nsenv;
           })
    | RecordDef rd ->
      let (name, nsenv, updated_nsenv) =
        elaborate_defined_id nsenv ElaborateRecord rd.rd_name
      in
      finish
        nsenv
        updated_nsenv
        (RecordDef { rd with rd_name = name; rd_namespace = nsenv })
    | Fun f ->
      let (name, nsenv, updated_nsenv) =
        elaborate_defined_id nsenv ElaborateFun f.f_name
      in
      finish
        nsenv
        updated_nsenv
        (Fun { f with f_name = name; f_namespace = nsenv })
    | Typedef t ->
      let (name, nsenv, updated_nsenv) =
        elaborate_defined_id nsenv ElaborateClass t.t_id
      in
      finish
        nsenv
        updated_nsenv
        (Typedef { t with t_id = name; t_namespace = nsenv })
    | Constant cst ->
      ( nsenv,
        [
          Constant
            {
              cst with
              cst_name =
                (let (name, _, _) =
                   elaborate_defined_id nsenv ElaborateConst cst.cst_name
                 in
                 name);
              cst_namespace = nsenv;
            };
        ] )
    | FileAttributes fa ->
      finish nsenv false (FileAttributes { fa with fa_namespace = nsenv })
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

let elaborate_def nsenv def = ElaborateDefs.def nsenv def

module ElaborateDefsNast = struct
  let hint nsenv = function
    | (p, Nast.Happly (id, args)) ->
      (p, Nast.Happly (elaborate_id nsenv ElaborateClass id, args))
    | other -> other

  let finish nsenv updated_nsenv stmt =
    if updated_nsenv then
      (nsenv, [stmt; Nast.SetNamespaceEnv nsenv])
    else
      (nsenv, [stmt])

  let rec def nsenv = function
    (*
      The default namespace in php is the global namespace specified by
      the empty string. In the case of an empty string, we model it as
      the global namespace.

      We remove namespace and use nodes and replace them with
      SetNamespaceEnv nodes that contain the namespace environment
    *)
    | Nast.Namespace ((_, nsname), prog) ->
      let parent_nsname =
        Option.value_map nsenv.ns_name ~default:"" ~f:(fun n -> n ^ "\\")
      in
      let nsname =
        match nsname with
        | "" -> None
        | _ -> Some (parent_nsname ^ nsname)
      in
      let new_nsenv = { nsenv with ns_name = nsname } in
      (nsenv, Nast.SetNamespaceEnv new_nsenv :: program new_nsenv prog)
    | Nast.NamespaceUse l ->
      let nsenv =
        List.fold_left l ~init:nsenv ~f:(fun nsenv (kind, id1, id2) ->
            match kind with
            | Nast.NSNamespace ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_ns_uses in
              { nsenv with ns_ns_uses = m }
            | Nast.NSClass ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_class_uses in
              { nsenv with ns_class_uses = m }
            | Nast.NSClassAndNamespace ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_class_uses in
              let n = SMap.add (snd id2) (snd id1) nsenv.ns_ns_uses in
              { nsenv with ns_class_uses = m; ns_ns_uses = n }
            | Nast.NSFun ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_fun_uses in
              { nsenv with ns_fun_uses = m }
            | Nast.NSConst ->
              let m = SMap.add (snd id2) (snd id1) nsenv.ns_const_uses in
              { nsenv with ns_const_uses = m })
      in
      (nsenv, [Nast.SetNamespaceEnv nsenv])
    | Nast.Class c ->
      let (name, nsenv, updated_nsenv) =
        elaborate_defined_id nsenv ElaborateClass c.Nast.c_name
      in
      finish
        nsenv
        updated_nsenv
        (Nast.Class
           Nast.
             {
               c with
               c_name = name;
               c_extends = List.map c.Nast.c_extends (hint nsenv);
               c_implements = List.map c.Nast.c_implements (hint nsenv);
               c_uses = List.map c.Nast.c_uses (hint nsenv);
               c_xhp_attr_uses = List.map c.Nast.c_xhp_attr_uses (hint nsenv);
               c_namespace = nsenv;
             })
    | Nast.Fun f ->
      let (name, nsenv, updated_nsenv) =
        elaborate_defined_id nsenv ElaborateFun f.Nast.f_name
      in
      finish
        nsenv
        updated_nsenv
        (Nast.Fun Nast.{ f with f_name = name; f_namespace = nsenv })
    | Nast.Typedef t ->
      let (name, nsenv, updated_nsenv) =
        elaborate_defined_id nsenv ElaborateClass t.Nast.t_name
      in
      finish
        nsenv
        updated_nsenv
        (Nast.Typedef Nast.{ t with t_name = name; t_namespace = nsenv })
    | Nast.Constant cst ->
      ( nsenv,
        [
          Nast.Constant
            Nast.
              {
                cst with
                cst_name =
                  (let (name, _, _) =
                     elaborate_defined_id
                       nsenv
                       ElaborateConst
                       cst.Nast.cst_name
                   in
                   name);
                cst_namespace = nsenv;
              };
        ] )
    | Nast.FileAttributes fa ->
      finish
        nsenv
        false
        (Nast.FileAttributes Nast.{ fa with fa_namespace = nsenv })
    | other -> (nsenv, [other])

  and attach_file_attributes p =
    let file_attributes =
      List.filter_map p ~f:(function
          | Nast.FileAttributes fa -> Some fa
          | _ -> None)
    in
    List.map p ~f:(function
        | Nast.Class c ->
          Nast.Class Nast.{ c with c_file_attributes = file_attributes }
        | Nast.Fun f ->
          Nast.Fun Nast.{ f with f_file_attributes = file_attributes }
        | x -> x)

  and program nsenv p =
    let (_, p) =
      List.fold_left p ~init:(nsenv, []) ~f:(fun (nsenv, acc) item ->
          let (nsenv, item) = def nsenv item in
          (nsenv, item :: acc))
    in
    p |> List.rev |> List.concat |> attach_file_attributes
end

let elaborate_toplevel_defs_nast popt ast =
  ElaborateDefsNast.program (Namespace_env.empty_from_popt popt) ast
