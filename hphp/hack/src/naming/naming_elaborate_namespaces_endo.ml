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
open Hh_prelude
module NS = Namespaces
module SN = Naming_special_names

type env = {
  namespace: Namespace_env.env;
  type_params: SSet.t;
}

let make_env namespace = { namespace; type_params = SSet.empty }

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
  fun name ->
    List.exists ~f:(fun ident -> String.equal ident name) special_identifiers

let is_reserved_type_hint name =
  let base_name = Utils.strip_ns name in
  SN.Typehints.is_reserved_type_hint base_name

let elaborate_type_name env ((_, name) as id) =
  if
    SSet.mem name env.type_params
    || is_special_identifier name
    || (String.length name <> 0 && Char.equal name.[0] '$')
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

(* `meth_caller` needs some fixup on the strings that are passed in *)
let handle_meth_caller env call =
  match call with
  | Call
      ({
         func = (_, _, Id (_, cn));
         args = [(pk, (ty, p1, String cl)); meth];
         _;
       } as call_expr)
    when String.equal cn SN.AutoimportedFunctions.meth_caller
         && (not @@ in_codegen env) ->
    let cl = Utils.add_ns cl in
    Call { call_expr with args = [(pk, (ty, p1, String cl)); meth] }
  | _ -> call

let contexts_ns =
  Namespace_env.
    {
      empty_with_default with
      ns_name = Some (Utils.strip_ns SN.Coeffects.contexts);
    }

let unsafe_contexts_ns =
  Namespace_env.
    {
      empty_with_default with
      ns_name = Some (Utils.strip_ns SN.Coeffects.unsafe_contexts);
    }

class ['a, 'b, 'c, 'd] generic_elaborator =
  object (self)
    inherit [_] Aast.endo as super

    method on_'ex _ ex = ex

    method on_'en _ en = en

    (* Namespaces were already precomputed by ElaborateDefs
     * The following functions just set the namespace env correctly
     *)
    method! on_class_ env c =
      let env = { env with namespace = c.c_namespace } in
      let env = extend_tparams env c.c_tparams in
      super#on_class_ env c

    method! on_ctx_refinement env =
      function
      | CRexact h -> CRexact (self#on_ctx_hint_ns contexts_ns env h)
      | CRloose { cr_lower; cr_upper } ->
        CRloose
          {
            cr_lower =
              Option.map ~f:(self#on_ctx_hint_ns contexts_ns env) cr_lower;
            cr_upper =
              Option.map ~f:(self#on_ctx_hint_ns contexts_ns env) cr_upper;
          }

    method on_class_ctx_const env kind =
      match kind with
      | TCConcrete { c_tc_type } ->
        TCConcrete { c_tc_type = self#on_ctx_hint_ns contexts_ns env c_tc_type }
      | TCAbstract
          {
            c_atc_as_constraint = as_;
            c_atc_super_constraint = super;
            c_atc_default = default;
          } ->
        let as_ = Option.map ~f:(self#on_ctx_hint_ns contexts_ns env) as_ in
        let super = Option.map ~f:(self#on_ctx_hint_ns contexts_ns env) super in
        let default =
          Option.map ~f:(self#on_ctx_hint_ns contexts_ns env) default
        in
        TCAbstract
          {
            c_atc_as_constraint = as_;
            c_atc_super_constraint = super;
            c_atc_default = default;
          }

    method! on_class_typeconst_def env tc =
      if tc.c_tconst_is_ctx then
        let c_tconst_kind = self#on_class_ctx_const env tc.c_tconst_kind in
        super#on_class_typeconst_def env { tc with c_tconst_kind }
      else
        super#on_class_typeconst_def env tc

    method! on_typedef env td =
      let env = { env with namespace = td.t_namespace } in
      let env = extend_tparams env td.t_tparams in
      if td.t_is_ctx then
        let t_as_constraint =
          Option.map ~f:(self#on_ctx_hint_ns contexts_ns env) td.t_as_constraint
        in
        let t_super_constraint =
          Option.map
            ~f:(self#on_ctx_hint_ns contexts_ns env)
            td.t_super_constraint
        in
        let t_kind = self#on_ctx_hint_ns contexts_ns env td.t_kind in
        super#on_typedef
          env
          { td with t_as_constraint; t_super_constraint; t_kind }
      else
        super#on_typedef env td

    (* Difference between fun_def and fun_ is that fun_ is also lambdas *)
    method! on_fun_def env f =
      let env = { env with namespace = f.fd_namespace } in
      let env = extend_tparams env f.fd_tparams in
      super#on_fun_def env f

    method! on_fun_ env f =
      let f_ctxs =
        Option.map ~f:(self#on_contexts_ns contexts_ns env) f.f_ctxs
      in
      let f_unsafe_ctxs =
        Option.map
          ~f:(self#on_contexts_ns unsafe_contexts_ns env)
          f.f_unsafe_ctxs
      in
      { (super#on_fun_ env f) with f_ctxs; f_unsafe_ctxs }

    method! on_method_ env m =
      let env = extend_tparams env m.m_tparams in
      let m_ctxs =
        Option.map ~f:(self#on_contexts_ns contexts_ns env) m.m_ctxs
      in
      let m_unsafe_ctxs =
        Option.map
          ~f:(self#on_contexts_ns unsafe_contexts_ns env)
          m.m_unsafe_ctxs
      in
      { (super#on_method_ env m) with m_ctxs; m_unsafe_ctxs }

    method! on_tparam env tparam =
      (* Make sure that the nested tparams are in scope while traversing the rest
         of the tparam, in particular the constraints.
         See Naming.type_param for description of nested tparam scoping *)
      let env_with_nested = extend_tparams env tparam.tp_parameters in
      super#on_tparam env_with_nested tparam

    method! on_gconst env gc =
      let env = { env with namespace = gc.cst_namespace } in
      super#on_gconst env gc

    method! on_file_attribute env fa =
      let env = { env with namespace = fa.fa_namespace } in
      super#on_file_attribute env fa

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
        let on_expr_list env exprs = List.map exprs ~f:(self#on_expr env) in

        let e1 = on_expr_list env e1 in
        let e2 =
          match e2 with
          | Some e2 -> Some (self#on_expr env e2)
          | None -> None
        in
        let (env, b) = self#on_block_helper env b in
        let e3 = on_expr_list env e3 in
        For (e1, e2, e3, b)
      | Do (b, e) ->
        let (env, b) = self#on_block_helper env b in
        let e = self#on_expr env e in
        Do (b, e)
      | _ -> super#on_stmt_ env stmt

    (* The function that actually rewrites names *)
    method! on_expr_ env expr =
      let map_arg env_ (pk, e) =
        (self#on_param_kind env_ pk, self#on_expr env_ e)
      in
      match expr with
      | Collection (id, c_targ_opt, flds) ->
        let id = NS.elaborate_id env.namespace NS.ElaborateClass id
        and flds = super#on_list super#on_afield env flds
        and c_targ_opt =
          super#on_option super#on_collection_targ env c_targ_opt
        in
        Collection (id, c_targ_opt, flds)
      | Call { func = (ty, p, Id (p2, cn)); targs; args; unpacked_arg }
        when SN.SpecialFunctions.is_special_function cn ->
        Call
          {
            func = (ty, p, Id (p2, cn));
            targs = List.map targs ~f:(self#on_targ env);
            args = List.map args ~f:(map_arg env);
            unpacked_arg = Option.map unpacked_arg ~f:(self#on_expr env);
          }
      | Call { func = (ty, p, Aast.Id id); targs; args; unpacked_arg } ->
        let new_id = NS.elaborate_id env.namespace NS.ElaborateFun id in
        let renamed_call =
          Call
            {
              func = (ty, p, Id new_id);
              targs = List.map targs ~f:(self#on_targ env);
              args = List.map args ~f:(map_arg env);
              unpacked_arg = Option.map unpacked_arg ~f:(self#on_expr env);
            }
        in
        handle_meth_caller env renamed_call
      | FunctionPointer (FP_id fn, targs) ->
        let fn = NS.elaborate_id env.namespace NS.ElaborateFun fn in
        let targs = List.map targs ~f:(self#on_targ env) in
        FunctionPointer (FP_id fn, targs)
      | FunctionPointer
          (FP_class_const (((), p1, CIexpr ((), p2, Id x1)), meth_name), targs)
        ->
        let name = elaborate_type_name env x1 in
        let targs = List.map targs ~f:(self#on_targ env) in
        FunctionPointer
          (FP_class_const (((), p1, CIexpr ((), p2, Id name)), meth_name), targs)
      | Obj_get (e1, (ty, p, Id x), null_safe, in_parens) ->
        Obj_get (self#on_expr env e1, (ty, p, Id x), null_safe, in_parens)
      | Id ((_, name) as sid) ->
        if
          (String.equal name "NAN" || String.equal name "INF") && in_codegen env
        then
          expr
        else
          Id (NS.elaborate_id env.namespace NS.ElaborateConst sid)
      | New (((), p1, CIexpr (ty, p2, Id x)), tal, el, unpacked_element, ex) ->
        let x = elaborate_type_name env x in
        New
          ( ((), p1, CIexpr (ty, p2, Id x)),
            List.map tal ~f:(self#on_targ env),
            List.map el ~f:(self#on_expr env),
            Option.map unpacked_element ~f:(self#on_expr env),
            ex )
      | Class_const ((_, p1, CIexpr (ty, p2, Id x1)), pstr) ->
        let name = elaborate_type_name env x1 in
        Class_const (((), p1, CIexpr (ty, p2, Id name)), pstr)
      | Nameof ((), p1, CIexpr (ty, p2, Id x)) ->
        (* Identical to the previous case, prevents "static" from becoming "\\static"
         * TODO(vmladenov) make this better *)
        let x = elaborate_type_name env x in
        Nameof ((), p1, CIexpr (ty, p2, Id x))
      | Class_get ((_, p1, CIexpr (ty, p2, Id x1)), cge, in_parens) ->
        let x1 = elaborate_type_name env x1 in
        Class_get
          ( ((), p1, CIexpr (ty, p2, Id x1)),
            self#on_class_get_expr env cge,
            in_parens )
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
      | EnumClassLabel (Some sid, name) ->
        let sid = elaborate_type_name env sid in
        EnumClassLabel (Some sid, name)
      | _ -> super#on_expr_ env expr

    method! on_hint_ env h =
      let is_xhp_screwup name =
        String.equal name "Xhp"
        || String.equal name ":Xhp"
        || String.equal name "XHP"
      in
      match h with
      | Happly ((_, name), _) when is_xhp_screwup name -> super#on_hint_ env h
      | Happly ((_, name), _)
        when is_reserved_type_hint name && (not @@ in_codegen env) ->
        super#on_hint_ env h
      | Happly (x, hl) ->
        let x = elaborate_type_name env x in
        Happly (x, List.map hl ~f:(self#on_hint env))
      | _ -> super#on_hint_ env h

    method! on_hint_fun env hf =
      let hf_ctxs =
        Option.map ~f:(self#on_contexts_ns contexts_ns env) hf.hf_ctxs
      in
      { (super#on_hint_fun env hf) with hf_ctxs }

    (* For contexts like cipp_of<T>, the type argument needs to be elaborated
     * in the standard namespace *)
    method private on_contexts_ns ctx_ns env ctxs =
      let (p, cs) = ctxs in
      let cs = List.map ~f:(self#on_ctx_hint_ns ctx_ns env) cs in
      (p, cs)

    method private on_ctx_hint_ns ctx_ns env h =
      let ctx_env = { env with namespace = ctx_ns } in
      let is_ctx_user_defined ctx =
        let tokens = Str.split (Str.regexp "\\") ctx in
        match List.last tokens with
        | Some ctx_name ->
          Char.equal ctx_name.[0] (Char.uppercase ctx_name.[0])
          (* Char.is_uppercase does not work on unicode characters *)
        | None -> false
      in
      match h with
      | (p, Happly (((_, x) as ctx), hl)) when not (is_reserved_type_hint x) ->
        let ctx =
          if is_ctx_user_defined x then
            elaborate_type_name env ctx
          else
            elaborate_type_name ctx_env ctx
        in
        (p, Happly (ctx, List.map hl ~f:(self#on_hint env)))
      | (p, Hintersection ctxs) ->
        (p, Hintersection (List.map ctxs ~f:(self#on_ctx_hint_ns ctx_ns env)))
      | (p, Haccess (root, names)) -> (p, Haccess (self#on_hint env root, names))
      | _ -> self#on_hint ctx_env h

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

    method! on_program (env : env) (p : ('a, 'b) Aast.program) =
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

let elaborate_namespaces = new generic_elaborator

let elaborate_program program =
  elaborate_namespaces#on_program
    (make_env Namespace_env.empty_with_default)
    program

let elaborate_fun_def fd =
  elaborate_namespaces#on_fun_def (make_env fd.Aast.fd_namespace) fd

let elaborate_class_ c =
  elaborate_namespaces#on_class_ (make_env c.Aast.c_namespace) c

let elaborate_module_def m =
  elaborate_namespaces#on_module_def
    (make_env Namespace_env.empty_with_default)
    m

let elaborate_gconst cst =
  elaborate_namespaces#on_gconst (make_env cst.Aast.cst_namespace) cst

let elaborate_typedef td =
  elaborate_namespaces#on_typedef (make_env td.Aast.t_namespace) td

let elaborate_stmt stmt =
  elaborate_namespaces#on_stmt (make_env Namespace_env.empty_with_default) stmt
