(**
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

module SN = Naming_special_names

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
let autoimport_classes = [
  "Traversable";
  "KeyedTraversable";
  "Container";
  "KeyedContainer";
  "Iterator";
  "KeyedIterator";
  "Iterable";
  "KeyedIterable";
  "Collection";
  "Vector";
  "ImmVector";
  "vec";
  "dict";
  "keyset";
  "Map";
  "ImmMap";
  "StableMap";
  "Set";
  "ImmSet";
  "Pair";
  "Awaitable";
  "AsyncIterator";
  "IMemoizeParam";
  "AsyncKeyedIterator";
  "InvariantException";
  "AsyncGenerator";
  "StaticWaitHandle";
  "WaitableWaitHandle";
  "ResumableWaitHandle";
  "AsyncFunctionWaitHandle";
  "AsyncGeneratorWaitHandle";
  "AwaitAllWaitHandle";
  "ConditionWaitHandle";
  "RescheduleWaitHandle";
  "SleepWaitHandle";
  "ExternalThreadEventWaitHandle";
  "Shapes";
  "TypeStructureKind";
]
let autoimport_funcs =   [
  "fun";
  "meth_caller";
  "class_meth";
  "inst_meth";
  "invariant_callback_register";
  "invariant";
  "invariant_violation";
  "idx";
  "type_structure";
  "asio_get_current_context_idx";
  "asio_get_running_in_context";
  "asio_get_running";
  "xenon_get_data";
  "thread_memory_stats";
  "thread_mark_stack";
  "objprof_get_strings";
  "objprof_get_data";
  "objprof_get_paths";
  "heapgraph_create";
  "heapgraph_stats";
  "heapgraph_foreach_node";
  "heapgraph_foreach_edge";
  "heapgraph_foreach_root";
  "heapgraph_dfs_nodes";
  "heapgraph_dfs_edges";
  "heapgraph_node";
  "heapgraph_edge";
  "heapgraph_node_in_edges";
  "heapgraph_node_out_edges";
  "server_warmup_status";
  "dict";
  "vec";
  "keyset";
  "varray";
  "darray";
  "is_vec";
  "is_dict";
  "is_keyset";
  "is_varray";
  "is_darray";
]
let autoimport_types = [
  "typename";
  "classname";
  "TypeStructure";
]
let autoimport_consts = [
  "Rx\\IS_ENABLED";
]

let autoimport_set =
  let autoimport_list
    = autoimport_classes @ autoimport_funcs @ autoimport_types @ autoimport_consts in
  List.fold_left autoimport_list ~init:SSet.empty ~f:(fun s e -> SSet.add e s)

(* Return the namespace (or None if the global one) into which id is auto imported.
 * Return false as first value if it is not auto imported
 *)
let get_autoimport_name_namespace id =
  if SN.Typehints.is_reserved_global_name id
  then (true, None)
  else
  if SN.Typehints.is_reserved_hh_name id ||
     SSet.mem id autoimport_set
  then (true, Some "HH")
  else (false, None)

(* NOTE that the runtime is able to distinguish between class and
   function names when auto-importing *)
let is_autoimport_name id =
  fst (get_autoimport_name_namespace id)

let is_always_global_function =
  let h = HashSet.create 23 in
  let funcs = SN.PseudoFunctions.all_pseudo_functions @ [
    "\\assert";
    "\\echo";
    "\\exit";
    "\\die";
  ] in
  List.iter funcs (HashSet.add h);
  fun x -> HashSet.mem h x

let elaborate_into_ns ns_name id =
match ns_name with
  | None -> "\\" ^ id
  | Some ns -> "\\" ^ ns ^ "\\" ^ id

let elaborate_into_current_ns nsenv id =
  elaborate_into_ns nsenv.ns_name id

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
    | (short_name, long_name)::rest ->
      let (target, source) = if reverse
                             then (long_name, short_name)
                             else (short_name, long_name) in
      (* Append backslash at the end so that it doesn't match partially *)
      if String_utils.string_starts_with id (source ^ "\\")
      (* Strip out the prefix and connect it to the next beginning *)
      then target ^ (String_utils.lstrip id source)
      else translate_id ~reverse rest id

let aliased_to_fully_qualified_id alias_map id =
  translate_id ~reverse:true alias_map id

type elaborate_kind =
  | ElaborateFun
  | ElaborateClass
  | ElaborateConst

(* Elaborate a defined identifier in a given namespace environment. For example,
 * a class might be defined inside a namespace. Return new environment if
 * ID is auto imported (e.g. Map) and so must be mapped when used.
 *)
let elaborate_defined_id nsenv kind (p, id) =
  let newid = elaborate_into_current_ns nsenv id in
  let update_nsenv = kind = ElaborateClass && is_autoimport_name id in
  let nsenv =
    if update_nsenv
    then {nsenv with ns_class_uses = SMap.add id newid nsenv.ns_class_uses}
    else nsenv in
  (p, newid), nsenv, update_nsenv

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
let elaborate_id_impl ~autoimport nsenv kind id =
  if id <> "" && id.[0] = '\\' then
    false, id (* The name is already fully-qualified. *)
  else

  let global_id = Utils.add_ns id in
  if kind = ElaborateConst && SN.PseudoConsts.is_pseudo_const global_id then
    false, global_id (* Pseudo-constants are always global. *)
  else if kind = ElaborateFun && is_always_global_function global_id then
    false, global_id
  else

  let bslash_loc, has_bslash =
    match String.index id '\\' with
    | Some i -> i, true
    | None -> String.length id, false
  in
  let prefix = String.sub id 0 bslash_loc in
  if prefix = "namespace" then
    false, elaborate_into_current_ns nsenv (String_utils.lstrip id "namespace\\")
  else

  (* Expand "use" imports. "use function" and "use const" only apply if the id
   * is completely unqualified, otherwise the normal "use" imports apply. *)
  let uses = if has_bslash then nsenv.ns_ns_uses else
    match kind with
    | ElaborateClass -> nsenv.ns_class_uses
    | ElaborateFun -> nsenv.ns_fun_uses
    | ElaborateConst -> nsenv.ns_const_uses
  in
  match SMap.get prefix uses with
  | Some use ->
    true, use ^ (String_utils.lstrip id prefix)
  | None ->
    let fq_id =
      let unaliased_id = aliased_to_fully_qualified_id
        (ParserOptions.auto_namespace_map nsenv.ns_popt) id in
      if unaliased_id <> id then
        "\\" ^ unaliased_id
      else if not autoimport then
        elaborate_into_current_ns nsenv id
      else
        match get_autoimport_name_namespace id with
        | false, _ ->
          elaborate_into_current_ns nsenv id
        | true, ns_name ->
          if ParserOptions.enable_hh_syntax_for_hhvm nsenv.ns_popt
            && (kind = ElaborateClass || kind = ElaborateConst)
          then elaborate_into_ns ns_name id
          else global_id
    in
    false, fq_id

let elaborate_id ?(autoimport=true) nsenv kind (p, id) =
  let _, newid = elaborate_id_impl ~autoimport nsenv kind id in
  p, newid

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
  let hint ~autoimport nsenv = function
    | p, Happly (id, args) ->
        p, Happly (elaborate_id ~autoimport nsenv ElaborateClass id, args)
    | other -> other

  let class_def ~autoimport nsenv = function
    | ClassUse h -> ClassUse (hint ~autoimport nsenv h)
    | XhpAttrUse h -> XhpAttrUse (hint ~autoimport nsenv h)
    | other -> other

  let finish nsenv updated_nsenv stmt =
    if updated_nsenv
    then nsenv, [stmt; SetNamespaceEnv nsenv]
    else nsenv, [stmt]

  let rec def ~autoimport nsenv = function
    (*
      The default namespace in php is the global namespace specified by
      the empty string. In the case of an empty string, we model it as
      the global namespace.

      We remove namespace and use nodes and replace them with
      SetNamespaceEnv nodes that contain the namespace environment
    *)
    | Namespace ((_, nsname), prog) -> begin
        let parent_nsname =
          Option.value_map nsenv.ns_name
            ~default:""
            ~f:(fun n -> n ^ "\\") in
        let nsname = match nsname with
          | "" -> None
          | _ -> Some (parent_nsname ^ nsname) in
        let new_nsenv = {nsenv with ns_name = nsname} in
        nsenv, SetNamespaceEnv new_nsenv :: program ~autoimport new_nsenv prog
      end
    | NamespaceUse l -> begin
        let nsenv =
          List.fold_left l ~init:nsenv ~f:begin fun nsenv (kind, id1, id2) ->
            match kind with
              | NSNamespace -> begin
                let m = SMap.add (snd id2) (snd id1) nsenv.ns_ns_uses in
                {nsenv with ns_ns_uses = m}
              end
              | NSClass -> begin
                let m = SMap.add (snd id2) (snd id1) nsenv.ns_class_uses in
                {nsenv with ns_class_uses = m}
              end
              | NSClassAndNamespace -> begin
                let m = SMap.add (snd id2) (snd id1) nsenv.ns_class_uses in
                let n = SMap.add (snd id2) (snd id1) nsenv.ns_ns_uses in
                {nsenv with ns_class_uses = m; ns_ns_uses = n}
              end
              | NSFun -> begin
                let m = SMap.add (snd id2) (snd id1) nsenv.ns_fun_uses in
                {nsenv with ns_fun_uses = m}
              end
              | NSConst -> begin
                let m = SMap.add (snd id2) (snd id1) nsenv.ns_const_uses in
                {nsenv with ns_const_uses = m}
              end
          end in
        nsenv, [SetNamespaceEnv nsenv]
      end
    | Class c ->
      let name, nsenv, updated_nsenv =
        elaborate_defined_id nsenv ElaborateClass c.c_name in
      finish nsenv updated_nsenv (Class {c with
        c_name = name;
        c_extends = List.map c.c_extends (hint ~autoimport nsenv);
        c_implements = List.map c.c_implements (hint ~autoimport nsenv);
        c_body = List.map c.c_body (class_def ~autoimport nsenv);
        c_namespace = nsenv;
      })
    | Fun f ->
      let name, nsenv, updated_nsenv =
        elaborate_defined_id nsenv ElaborateFun f.f_name in
      finish nsenv updated_nsenv (Fun {f with
        f_name = name;
        f_namespace = nsenv;
      })
    | Typedef t ->
      let name, nsenv, updated_nsenv =
        elaborate_defined_id nsenv ElaborateClass t.t_id in
      finish nsenv updated_nsenv (Typedef {t with
        t_id = name;
        t_namespace = nsenv;
      })
    | Constant cst -> nsenv, [Constant {cst with
        cst_name =
          (let name, _, _ =
            elaborate_defined_id nsenv ElaborateConst cst.cst_name
          in name);
        cst_namespace = nsenv;
      }]
    | FileAttributes fa ->
      finish nsenv false (FileAttributes {fa with
        fa_namespace = nsenv;
      })
    | other -> nsenv, [other]

  and attach_file_attributes p =
    let file_attributes =
      List.filter_map p ~f:begin function
      | FileAttributes fa -> Some fa
      | _ -> None
    end in
    List.map p ~f:begin function
      | Class c -> Class { c with c_file_attributes = file_attributes }
      | Fun f -> Fun { f with f_file_attributes = file_attributes }
      | x -> x
    end

  and program ~autoimport nsenv p =
    let _, p =
      List.fold_left p ~init:(nsenv, []) ~f:begin fun (nsenv, acc) item ->
        let nsenv, item = def ~autoimport nsenv item in
        nsenv, item :: acc
      end in
    p |> List.rev |> List.concat |> attach_file_attributes
end

let elaborate_toplevel_defs ~autoimport popt ast  =
  ElaborateDefs.program ~autoimport (Namespace_env.empty popt) ast

let elaborate_def nsenv def =
  ElaborateDefs.def ~autoimport:true nsenv def
