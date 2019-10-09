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
module SN = Naming_special_names

type elaborate_kind =
  | ElaborateFun
  | ElaborateClass
  | ElaborateRecord
  | ElaborateConst

(**
 * Return the namespace into which id is auto imported for the typechecker and
 * compiler, respectively. Return None if it is not auto imported.
 *)
let get_autoimport_name_namespace id kind =
  match kind with
  | ElaborateClass -> Hh_autoimport.lookup_type id
  | ElaborateRecord -> Hh_autoimport.lookup_type id
  | ElaborateFun -> Hh_autoimport.lookup_func id
  | ElaborateConst -> Hh_autoimport.lookup_const id

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
  if id <> "" && id.[0] = '\\' then
    (* The name is already fully-qualified. *)
    id
  else
    let global_id = Utils.add_ns id in
    if kind = ElaborateConst && SN.PseudoConsts.is_pseudo_const global_id then
      (* Pseudo-constants are always global. *)
      global_id
    else if
      kind = ElaborateFun && SN.PseudoFunctions.is_pseudo_function global_id
    then
      global_id
    else if kind = ElaborateClass && SN.Typehints.is_reserved_global_name id
    then
      global_id
    else
      let (bslash_loc, has_bslash) =
        match String.index id '\\' with
        | Some i -> (i, true)
        | None -> (String.length id, false)
      in
      let prefix = String.sub id 0 bslash_loc in
      if has_bslash && prefix = "namespace" then
        elaborate_into_current_ns nsenv (String_utils.lstrip id "namespace\\")
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
        | Some use -> use ^ String_utils.lstrip id prefix
        | None ->
          let unaliased_id =
            aliased_to_fully_qualified_id nsenv.ns_auto_ns_map id
          in
          if unaliased_id <> id then
            Utils.add_ns unaliased_id
          else (
            match get_autoimport_name_namespace id kind with
            | None -> elaborate_into_current_ns nsenv id
            | Some (typechecker_ns, compiler_ns) ->
              let ns =
                if nsenv.ns_is_codegen then
                  compiler_ns
                else
                  typechecker_ns
              in
              elaborate_into_ns (Hh_autoimport.string_of_ns ns) id
          )

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
