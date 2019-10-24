(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Aast
open Ast_defs
open Core_kernel
module NS = Namespaces
module SN = Naming_special_names

type env = {
  namespace: Namespace_env.env;
  let_locals: SSet.t;
  type_params: SSet.t;
  in_ppl: bool;
}

let is_special_identifier =
  let special_identifiers =
    [
      SN.Classes.cParent;
      SN.Classes.cSelf;
      SN.Classes.cStatic;
      SN.SpecialIdents.this;
      SN.SpecialIdents.dollardollar;
    ]
  in
  (fun name -> List.exists ~f:(fun ident -> ident = name) special_identifiers)

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

let namespace_elaborater =
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
        Attributes.mem
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
      super#on_method_redeclaration env mt

    method! on_gconst env gc =
      let env = { env with namespace = gc.cst_namespace } in
      super#on_gconst env gc

    method! on_file_attribute env fa =
      let env = { env with namespace = fa.fa_namespace } in
      super#on_file_attribute env fa

    method! on_func_body env fb =
      let env =
        match fb.fb_annotation with
        | Nast.Named
        | Nast.NamedWithUnsafeBlocks ->
          env
        | Nast.Unnamed nsenv -> { env with namespace = nsenv }
      in
      super#on_func_body env fb

    method! on_record_def env rd =
      let env = { env with namespace = rd.rd_namespace } in
      super#on_record_def env rd

    (* Sets let local env correctly *)
    method on_block_helper env b =
      let aux (env, stmts) stmt =
        match stmt with
        | (ann, Let (((_, lid) as x), h, e)) ->
          let new_env =
            {
              env with
              let_locals = SSet.add (Local_id.get_name lid) env.let_locals;
            }
          in
          let new_stmt_ =
            Let (x, Option.map h (super#on_hint env), super#on_expr env e)
          in
          (new_env, (ann, new_stmt_) :: stmts)
        | _ -> (env, super#on_stmt env stmt :: stmts)
      in
      let (env, rev_stmts) = List.fold b ~f:aux ~init:(env, []) in
      (env, List.rev rev_stmts)

    method! on_block env b =
      let (_, stmts) = self#on_block_helper env b in
      stmts

    method! on_catch env (x1, ((_, lid2) as x2), b) =
      let x1 = elaborate_type_name env x1 in
      (* If the variable does not begin with $, it is an immutable binding *)
      let name2 = Local_id.get_name lid2 in
      let let_locals =
        if name2 <> "" && name2.[0] <> '$' then
          SSet.add name2 env.let_locals
        else
          env.let_locals
      in
      let b = self#on_block { env with let_locals } b in
      (x1, x2, b)

    method! on_stmt_ env stmt =
      match stmt with
      | Foreach (e, ae, b) ->
        let add_local env e =
          match snd e with
          | Id x -> { env with let_locals = SSet.add (snd x) env.let_locals }
          | _ -> env
        in
        let new_env =
          match ae with
          | As_v ve
          | Await_as_v (_, ve) ->
            add_local env ve
          | As_kv (ke, ve)
          | Await_as_kv (_, ke, ve) ->
            let env = add_local env ke in
            add_local env ve
        in
        let e = self#on_expr env e in
        let b = self#on_block new_env b in
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
      (* invariant is handled differently in naming depending whether it is in
       * the statement position or expression position.
       *)
      | Expr (p, Call (p2, (p3, Id (p4, fn)), targs, el, uel))
        when fn = SN.SpecialFunctions.invariant ->
        Expr
          ( p,
            Call
              ( p2,
                (p3, Id (p4, fn)),
                List.map targs ~f:(self#on_targ env),
                List.map el ~f:(self#on_expr env),
                List.map uel ~f:(self#on_expr env) ) )
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
      | Call (ct, (p, Id (p2, cn)), targs, [(p3, String fn)], uargs)
        when cn = SN.SpecialFunctions.fun_ ->
        (* Functions referenced by fun() are always fully-qualified *)
        let fn = Utils.add_ns fn in
        Call (ct, (p, Id (p2, cn)), targs, [(p3, String fn)], uargs)
      | Call (ct, (p, Id ((_, cn) as id)), tal, el, uel)
        when cn = SN.SpecialFunctions.invariant ->
        (* invariant is handled differently in naming depending whether it is in
         * the statement position or expression position.
         *)
        let new_id = NS.elaborate_id env.namespace NS.ElaborateFun id in
        Call
          ( ct,
            (p, Id new_id),
            List.map tal ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            List.map uel ~f:(self#on_expr env) )
      | Call
          ( ct,
            (p1, Id (p2, cn)),
            targs,
            [(p3, String cl); (p4, String meth)],
            uel )
        when cn = SN.SpecialFunctions.meth_caller
             || cn = SN.SpecialFunctions.class_meth ->
        let cl = Utils.add_ns cl in
        Call
          ( self#on_call_type env ct,
            (p1, Id (p2, cn)),
            List.map targs ~f:(self#on_targ env),
            [(p3, String cl); (p4, String meth)],
            List.map uel ~f:(self#on_expr env) )
      | Call
          ( ct,
            (p1, Id (p2, cn)),
            targs,
            [(p3, Class_const ((p4, CI cl), (p5, mem))); (p6, String meth)],
            uel )
        when ( cn = SN.SpecialFunctions.meth_caller
             || cn = SN.SpecialFunctions.class_meth )
             && mem = SN.Members.mClass ->
        let cl = elaborate_type_name env cl in
        Call
          ( self#on_call_type env ct,
            (p1, Id (p2, cn)),
            List.map targs ~f:(self#on_targ env),
            [(p3, Class_const ((p4, CI cl), (p5, mem))); (p6, String meth)],
            List.map uel ~f:(self#on_expr env) )
      | Call (ct, (p, Id (p2, cn)), targs, el, uargs)
        when SN.SpecialFunctions.is_special_function cn
             || (SN.PPLFunctions.is_reserved cn && env.in_ppl) ->
        Call
          ( self#on_call_type env ct,
            (p, Id (p2, cn)),
            List.map targs ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            List.map uargs ~f:(self#on_expr env) )
      | Call (ct, (p, Aast.Id id), tal, el, uel) ->
        let new_id =
          if SSet.mem (snd id) env.let_locals then
            id
          else
            NS.elaborate_id env.namespace NS.ElaborateFun id
        in
        Call
          ( ct,
            (p, Id new_id),
            List.map tal ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            List.map uel ~f:(self#on_expr env) )
      | Obj_get (e1, (p, Id x), null_safe) ->
        Obj_get (self#on_expr env e1, (p, Id x), null_safe)
      | Id ((_, name) as sid) ->
        if SSet.mem name env.let_locals then
          expr
        else
          Id (NS.elaborate_id env.namespace NS.ElaborateConst sid)
      | PU_identifier ((p1, CIexpr (p2, Id x1)), s1, s2) ->
        let x1 = elaborate_type_name env x1 in
        PU_identifier ((p1, CIexpr (p2, Id x1)), s1, s2)
      | New ((p1, CIexpr (p2, Id x)), tal, el, uel, ex) ->
        let x = elaborate_type_name env x in
        New
          ( (p1, CIexpr (p2, Id x)),
            List.map tal ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            List.map uel ~f:(self#on_expr env),
            ex )
      | Record ((p1, CIexpr (p2, Id x)), is_array, l) ->
        let x = elaborate_type_name env x in
        let l =
          List.map l ~f:(fun (e1, e2) ->
              (self#on_expr env e1, self#on_expr env e2))
        in
        Record ((p1, CIexpr (p2, Id x)), is_array, l)
      | Class_const ((p1, CIexpr (p2, Id x1)), pstr) ->
        let name = elaborate_type_name env x1 in
        Class_const ((p1, CIexpr (p2, Id name)), pstr)
      | Class_get ((p1, CIexpr (p2, Id x1)), CGstring x2) ->
        let x1 = elaborate_type_name env x1 in
        Class_get ((p1, CIexpr (p2, Id x1)), CGstring x2)
      | Xml (id, al, el) ->
        let id = elaborate_type_name env id in
        Xml
          ( id,
            List.map al ~f:(self#on_xhp_attribute env),
            List.map el ~f:(self#on_expr env) )
      | _ -> super#on_expr_ env expr

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

    method! on_program (env : env) (p : Nast.program) =
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

(* Toplevel environment *)
let make_env namespace =
  {
    namespace;
    let_locals = SSet.empty;
    type_params = SSet.empty;
    in_ppl = false;
  }
