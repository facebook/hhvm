(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * AAST visitor that elaborates the names in bodies
 *   call_some_function($args);
 *     ->
 *   \CurrentNamespace\CurrentSubnamespace\call_some_function($args);
 * Both typechecking and codegen code expect that the AAST has been elaborated.
 * Currently, this assumes that the AAST has been run through
 *   Namespaces.elaborate_toplevel_defs, which elaborates the names of toplevel
 *   classes, functions, without elaborating the names in their bodies.
 * Elaboration is currently separated into these two passes based on the idea
 *   that decl extraction does not need to do any work with function decls.
 *   However, it should be viable to combine the two passes into a single pass.
 * See also: parser/namespaces.ml, parser/hh_autoimport.ml,
 *   naming/naming_special_names.ml
 *)

open Aast
open Ast_defs
open Core_kernel
module NS = Namespaces
module SN = Naming_special_names

type env = {
  namespace: Namespace_env.env;
  type_params: SSet.t;
  in_ppl: bool;
}

let make_env namespace = { namespace; type_params = SSet.empty; in_ppl = false }

(* While elaboration for codegen and typing is similar, there are currently a
 *   couple differences between the two and are toggled by this flag (XHP).
 * It would be nice to eventually eliminate the discrepancies between the two.
 *)
let in_codegen env = env.namespace.Namespace_env.ns_is_codegen

let is_special_identifier =
  let special_identifiers =
    [
      SN.Members.mClass;
      SN.Classes.cParent;
      SN.Classes.cSelf;
      SN.Classes.cStatic;
      SN.SpecialIdents.this;
      SN.SpecialIdents.dollardollar;
      SN.Typehints.wildcard;
    ]
  in
  (fun name -> List.exists ~f:(fun ident -> ident = name) special_identifiers)

let is_reserved_type_hint name =
  let base_name = Utils.strip_ns name in
  SN.Typehints.is_reserved_type_hint base_name
  || SN.Rx.is_reactive_typehint name

let elaborate_type_name env ((_, name) as id) =
  if
    SSet.mem name env.type_params
    || is_special_identifier name
    || (String.length name <> 0 && name.[0] = '$')
  then
    id
  else
    NS.elaborate_id env.namespace NS.ElaborateClass id

let extend_tparams env tparaml =
  let type_params =
    List.fold
      tparaml
      ~f:(fun acc tparam -> SSet.add (snd tparam.tp_name) acc)
      ~init:env.type_params
  in
  { env with type_params }

(* Functions such as fun, class_meth, and meth_caller require additional
 * fixup on the strings that are passed in
 *)
let handle_special_calls env call =
  match call with
  | Call (ct, ((_, Id (_, cn)) as id), targs, [(p, String fn)], uargs)
    when cn = SN.AutoimportedFunctions.fun_ ->
    (* Functions referenced by fun() are always fully-qualified *)
    let fn = Utils.add_ns fn in
    Call (ct, id, targs, [(p, String fn)], uargs)
  | Call
      ( ct,
        ((_, Id (_, cn)) as id),
        targs,
        [(p1, String cl); meth],
        unpacked_element )
    when ( cn = SN.AutoimportedFunctions.meth_caller
         || cn = SN.AutoimportedFunctions.class_meth )
         && (not @@ in_codegen env) ->
    let cl = Utils.add_ns cl in
    Call (ct, id, targs, [(p1, String cl); meth], unpacked_element)
  | _ -> call

class ['a, 'b, 'c, 'd] generic_elaborator =
  object (self)
    inherit [_] Aast.endo as super

    method on_'ex _ ex = ex

    method on_'fb _ fb = fb

    method on_'en _ en = en

    method on_'hi _ hi = hi

    (* Namespaces were already precomputed by ElaborateDefs
     * The following functions just set the namespace env correctly
     *)
    method! on_class_ env c =
      let in_ppl =
        Naming_attributes.mem
          SN.UserAttributes.uaProbabilisticModel
          c.c_user_attributes
      in
      let env = { env with namespace = c.c_namespace; in_ppl } in
      let env = extend_tparams env c.c_tparams.c_tparam_list in
      super#on_class_ env c

    method! on_typedef env td =
      let env = { env with namespace = td.t_namespace } in
      let env = extend_tparams env td.t_tparams in
      super#on_typedef env td

    (* Difference between fun_def and fun_ is that fun_ is also lambdas *)
    method! on_fun_def env f =
      let env = { env with namespace = f.f_namespace } in
      let env = extend_tparams env f.f_tparams in
      super#on_fun_def env f

    method! on_method_ env m =
      let env = extend_tparams env m.m_tparams in
      super#on_method_ env m

    method! on_method_redeclaration env mt =
      let env = extend_tparams env mt.mt_tparams in
      (* Codegen does not elaborate traits in the trait redeclaration node.
       * TODO: This should be changed if this feature is to be shipped.
       * Also change: class_method_trait_resolution in emit_class.ml
       * T56629465
       *)
      if in_codegen env then
        let mr_new = super#on_method_redeclaration env mt in
        { mr_new with mt_trait = mt.mt_trait }
      else
        super#on_method_redeclaration env mt

    method! on_pu_enum env pue =
      let type_params =
        List.fold
          pue.pu_case_types
          ~f:(fun acc (sid, _) -> SSet.add (snd sid) acc)
          ~init:env.type_params
      in
      let env = { env with type_params } in
      super#on_pu_enum env pue

    method! on_gconst env gc =
      let env = { env with namespace = gc.cst_namespace } in
      super#on_gconst env gc

    method! on_file_attribute env fa =
      let env = { env with namespace = fa.fa_namespace } in
      super#on_file_attribute env fa

    method! on_record_def env rd =
      let env = { env with namespace = rd.rd_namespace } in
      let rd_name =
        NS.elaborate_id env.namespace NS.ElaborateRecord rd.rd_name
      in
      let rd_extends =
        match rd.rd_extends with
        | Some (p, Aast.Happly (name, hl)) ->
          let name = NS.elaborate_id env.namespace NS.ElaborateRecord name in
          let hl = List.map ~f:(self#on_hint env) hl in
          Some (p, Aast.Happly (name, hl))
        | _ -> Option.map ~f:(self#on_hint env) rd.rd_extends
      in
      let rd = super#on_record_def env rd in
      { rd with rd_name; rd_extends }

    (* Sets let local env correctly *)
    method on_block_helper env b =
      let aux (env, stmts) stmt = (env, super#on_stmt env stmt :: stmts) in
      let (env, rev_stmts) = List.fold b ~f:aux ~init:(env, []) in
      (env, List.rev rev_stmts)

    method! on_block env b =
      let (_, stmts) = self#on_block_helper env b in
      stmts

    method! on_catch env (x1, x2, b) =
      let x1 = elaborate_type_name env x1 in
      let b = self#on_block env b in
      (x1, x2, b)

    method! on_stmt_ env stmt =
      match stmt with
      | Foreach (e, ae, b) ->
        let e = self#on_expr env e in
        let ae = self#on_as_expr env ae in
        let b = self#on_block env b in
        Foreach (e, ae, b)
      | For (e1, e2, e3, b) ->
        let e1 = self#on_expr env e1 in
        let e2 = self#on_expr env e2 in
        let (env, b) = self#on_block_helper env b in
        let e3 = self#on_expr env e3 in
        For (e1, e2, e3, b)
      | Do (b, e) ->
        let (env, b) = self#on_block_helper env b in
        let e = self#on_expr env e in
        Do (b, e)
      | _ -> super#on_stmt_ env stmt

    (* Lambda environments *)
    method! on_Lfun env e =
      let env = { env with in_ppl = false } in
      super#on_Lfun env e

    method! on_Efun env e =
      let env = { env with in_ppl = false } in
      super#on_Efun env e

    (* The function that actually rewrites names *)
    method! on_expr_ env expr =
      match expr with
      | Call (ct, (p, Id (p2, cn)), targs, el, uarg)
        when SN.SpecialFunctions.is_special_function cn
             || (SN.PPLFunctions.is_reserved cn && env.in_ppl) ->
        Call
          ( self#on_call_type env ct,
            (p, Id (p2, cn)),
            List.map targs ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            Option.map uarg ~f:(self#on_expr env) )
      | Call (ct, (p, Aast.Id id), tal, el, unpacked_element) ->
        let new_id = NS.elaborate_id env.namespace NS.ElaborateFun id in
        let renamed_call =
          Call
            ( ct,
              (p, Id new_id),
              List.map tal ~f:(self#on_targ env),
              List.map el ~f:(self#on_expr env),
              Option.map unpacked_element ~f:(self#on_expr env) )
        in
        handle_special_calls env renamed_call
      | FunctionPointer ((p, Id fn), targs) ->
        let fn = NS.elaborate_id env.namespace NS.ElaborateFun fn in
        let targs = List.map targs ~f:(self#on_targ env) in
        FunctionPointer ((p, Id fn), targs)
      | Obj_get (e1, (p, Id x), null_safe) ->
        Obj_get (self#on_expr env e1, (p, Id x), null_safe)
      | Id ((_, name) as sid) ->
        if (name = "NAN" || name = "INF") && in_codegen env then
          expr
        else
          Id (NS.elaborate_id env.namespace NS.ElaborateConst sid)
      | PU_identifier ((p1, CIexpr (p2, Id x1)), s1, s2) ->
        let x1 = elaborate_type_name env x1 in
        PU_identifier ((p1, CIexpr (p2, Id x1)), s1, s2)
      | New ((p1, CIexpr (p2, Id x)), tal, el, unpacked_element, ex) ->
        let x = elaborate_type_name env x in
        New
          ( (p1, CIexpr (p2, Id x)),
            List.map tal ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            Option.map unpacked_element ~f:(self#on_expr env),
            ex )
      | Record (id, is_array, l) ->
        let id = elaborate_type_name env id in
        let l =
          List.map l ~f:(fun (e1, e2) ->
              (self#on_expr env e1, self#on_expr env e2))
        in
        Record (id, is_array, l)
      | Class_const ((p1, CIexpr (p2, Id x1)), pstr) ->
        let name = elaborate_type_name env x1 in
        Class_const ((p1, CIexpr (p2, Id name)), pstr)
      | Class_get ((p1, CIexpr (p2, Id x1)), cge) ->
        let x1 = elaborate_type_name env x1 in
        Class_get ((p1, CIexpr (p2, Id x1)), self#on_class_get_expr env cge)
      | Xml (id, al, el) ->
        let id =
          (* if XHP element mangling is disabled, namespaces are supported *)
          if
            in_codegen env
            && not env.namespace.Namespace_env.ns_disable_xhp_element_mangling
          then
            id
          else
            elaborate_type_name env id
        in
        Xml
          ( id,
            List.map al ~f:(self#on_xhp_attribute env),
            List.map el ~f:(self#on_expr env) )
      | _ -> super#on_expr_ env expr

    method! on_hint_ env h =
      let is_rx name =
        name = SN.Rx.hRx || name = SN.Rx.hRxLocal || name = SN.Rx.hRxShallow
      in
      let is_xhp_screwup name = name = "Xhp" || name = ":Xhp" || name = "XHP" in
      match h with
      | Happly ((_, name), _) when is_xhp_screwup name -> super#on_hint_ env h
      | Happly ((_, name), [(_, Hfun _)]) when is_rx name ->
        super#on_hint_ env h
      | Happly (((_, name) as x), hl) when is_rx name ->
        let x = elaborate_type_name env x in
        Happly (x, List.map hl ~f:(self#on_hint env))
      | Happly ((_, name), _)
        when is_reserved_type_hint name && (not @@ in_codegen env) ->
        super#on_hint_ env h
      | Happly (x, hl) ->
        let x = elaborate_type_name env x in
        Happly (x, List.map hl ~f:(self#on_hint env))
      | _ -> super#on_hint_ env h

    method! on_shape_field_name env sfn =
      match sfn with
      | SFclass_const (x, (pos, y)) ->
        let x = elaborate_type_name env x in
        SFclass_const (x, (pos, y))
      | _ -> sfn

    method! on_user_attribute env ua =
      let ua_name =
        if SN.UserAttributes.is_reserved (snd ua.ua_name) then
          ua.ua_name
        else
          elaborate_type_name env ua.ua_name
      in
      { ua_name; ua_params = List.map ~f:(self#on_expr env) ua.ua_params }

    method! on_insteadof_alias env alias =
      let (sid1, pstr, sid2) = alias in
      ( elaborate_type_name env sid1,
        pstr,
        List.map ~f:(elaborate_type_name env) sid2 )

    method! on_use_as_alias env alias =
      let (sido1, pstr, sido2, visl) = alias in
      let sido1 = Option.map ~f:(elaborate_type_name env) sido1 in
      (sido1, pstr, sido2, visl)

    method! on_xhp_child env child =
      if in_codegen env then
        super#on_xhp_child env child
      else
        match child with
        | ChildName ((_, name) as sid)
          when (not @@ Naming_special_names.XHP.is_reserved name)
               && (not @@ Naming_special_names.XHP.is_xhp_category name) ->
          ChildName (elaborate_type_name env sid)
        | _ -> super#on_xhp_child env child

    method! on_program (env : env) (p : ('a, 'b, 'c, 'd) Aast.program) =
      let aux (env, defs) def =
        match def with
        | SetNamespaceEnv nsenv ->
          let env = { env with namespace = nsenv } in
          (env, def :: defs)
        | _ -> (env, super#on_def env def :: defs)
      in
      let (_, rev_defs) = List.fold p ~f:aux ~init:(env, []) in
      List.rev rev_defs
  end
