(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
module T = Aast
module ULS = Unique_list_string
module SN = Naming_special_names

type bare_this_usage =
  (* $this appear as expression *)
  | Bare_this
  (* bare $this appear as possible targetfor assignment
  - $this as function argument
  - $this as target of & operator
  - $this as reference in catch clause *)
  | Bare_this_as_ref

type decl_vars_state = {
  (* set of locals used inside the functions *)
  dvs_locals: ULS.t;
  (* does function uses bare form of $this *)
  dvs_bare_this: bare_this_usage option;
}

let with_local name s = { s with dvs_locals = ULS.add s.dvs_locals name }

let with_this barethis s =
  let new_bare_this =
    match (s.dvs_bare_this, barethis) with
    | (_, Bare_this_as_ref) -> Some Bare_this_as_ref
    | (None, Bare_this) -> Some Bare_this
    | (u, _) -> u
  in
  if s.dvs_bare_this = new_bare_this then
    s
  else
    {
      dvs_bare_this = new_bare_this;
      dvs_locals = ULS.add s.dvs_locals SN.SpecialIdents.this;
    }

let dvs_empty = { dvs_locals = ULS.empty; dvs_bare_this = None }

(* Add a local to the accumulated list. Don't add if it's $GLOBALS or
 * the pipe variable $$. If it's $this, add it, and if this variable appears
 * "bare" (because bareparam=true), remember for needs_local_this *)
let add_local ~barethis s (_, name) =
  if
    name = SN.Superglobals.globals
    || name = SN.SpecialIdents.dollardollar
    || SN.SpecialIdents.is_tmp_var name
  then
    s
  else if name = SN.SpecialIdents.this then
    with_this barethis s
  else
    with_local name s

let on_class_get visitor (_, cid_) cge ~is_call_target =
  match cid_ with
  | T.CIparent
  | T.CIself
  | T.CIstatic ->
    failwith "Expects CIexpr as class_id on aast where expr was on ast"
  | T.CI _sid ->
    failwith "Expects CIexpr as class_id on aast where expr was on ast"
  | T.CIexpr e ->
    let _ = visitor#on_expr () e in
    begin
      match cge with
      | T.CGstring pstr ->
        (* TODO: Thomas: For this to match correctly, we need to adjust ast_to_nast
         * because it does not make a distinction between ID and Lvar, which is needed here
         *)
        if is_call_target then
          visitor#set_state (add_local ~barethis:Bare_this visitor#state pstr)
        else
          ()
      | T.CGexpr e2 -> visitor#on_expr () e2
    end

let declvar_visitor explicit_use_set_opt is_in_static_method is_closure_body =
  let state = ref dvs_empty in
  object (self)
    inherit [_] Aast.iter as super

    method! on_stmt_ _ s =
      match s with
      | Aast.Try (body, catch_list, finally) ->
        self#on_block () body;
        List.iter catch_list (fun (_exn_name, (pos, id), catch_body) ->
            state :=
              add_local
                ~barethis:Bare_this_as_ref
                !state
                (pos, Local_id.get_name id);
            self#on_block () catch_body);
        self#on_block () finally
      | _ -> super#on_stmt_ () s

    method! on_expr_ _ e =
      match e with
      | Aast.Obj_get (receiver_e, prop_e, _) ->
        (match snd receiver_e with
        | Aast.Lvar (_, id) when Local_id.get_name id = "$this" ->
          if is_in_static_method && not is_closure_body then
            ()
          else
            state := with_local (Local_id.get_name id) !state
        | _ -> super#on_expr () receiver_e);
        (match snd prop_e with
        (* Only add if it is a variable *)
        | Aast.Lvar (pos, id) ->
          state :=
            add_local ~barethis:Bare_this !state (pos, Local_id.get_name id)
        | _ -> super#on_expr () prop_e)
      | Aast.Binop (binop, e1, e2) ->
        (match (binop, e2) with
        | (Ast_defs.Eq _, (_, Aast.Await _))
        | (Ast_defs.Eq _, (_, Aast.Yield _))
        | (Ast_defs.Eq _, (_, Aast.Yield_from _)) ->
          (* Visit e2 before e1. The ordering of declvars in async
                expressions matters to HHVM. See D5674623. *)
          self#on_expr () e2;
          self#on_expr () e1
        | _ -> super#on_expr_ () e)
      | Aast.Lvar (pos, id) ->
        state := add_local ~barethis:Bare_this !state (pos, Local_id.get_name id)
      | Aast.Class_get (cid, expr) ->
        on_class_get self cid expr ~is_call_target:false
      | Aast.Lfun _ ->
        (* For an Lfun, we don't want to recurse, because it's a separate scope. *)
        ()
      | Aast.Efun (fun_, use_list) ->
        (* at this point AST is already rewritten so use lists on EFun nodes
            contain list of captured variables. However if use list was initially absent
            it is not correct to traverse such nodes to collect locals because it will impact
            the order of locals in generated .declvars section:
            // .declvars $a, $c, $b
            $a = () => { $b = 1 };
            $c = 1;
            $b = 2;
            // .declvars $a, $b, $c
            $a = function () use ($b) => { $b = 1 };
            $c = 1;
            $b = 2;

            'explicit_use_set' is used to in order to avoid synthesized use list *)
        let fn_name = snd fun_.Aast.f_name in
        let has_use_list =
          Option.value_map explicit_use_set_opt ~default:false ~f:(fun s ->
              SSet.mem fn_name s)
        in
        if has_use_list then
          List.iter use_list (fun (pos, id) ->
              state :=
                add_local ~barethis:Bare_this !state (pos, Local_id.get_name id))
        else
          ()
      | Aast.Call (_, func_e, _, pos_args, unpacked_arg) ->
        (match func_e with
        | (_, Aast.Id (pos, call_name))
          when call_name = SN.EmitterSpecialFunctions.set_frame_metadata ->
          state := add_local ~barethis:Bare_this !state (pos, "$86metadata")
        | _ -> ());
        let barethis =
          match func_e with
          | (_, Aast.Id (_, name))
            when name = SN.PseudoFunctions.isset
                 || "\\" ^ name = SN.PseudoFunctions.echo ->
            Bare_this
          | _ -> Bare_this_as_ref
        in
        let on_arg e =
          match e with
          (* Only add $this to locals if it's bare *)
          | (_, Aast.Lvar (_, id)) when Local_id.get_name id = "$this" ->
            state := with_this barethis !state
          | _ -> self#on_expr () e
        in
        (match snd func_e with
        | Aast.Class_get (id, prop) ->
          on_class_get self id prop ~is_call_target:true
        | _ -> self#on_expr () func_e);
        List.iter pos_args on_arg;
        Option.iter unpacked_arg on_arg
      | Aast.New (_, _, exprs1, expr2, _) ->
        let add_bare_expr expr =
          match expr with
          | (_, Aast.Lvar (_, id)) when Local_id.get_name id = "$this" ->
            state := with_this Bare_this_as_ref !state
          | _ -> self#on_expr () expr
        in
        List.iter exprs1 add_bare_expr;
        Option.iter expr2 add_bare_expr
      | _ -> super#on_expr_ () e

    method! on_class_ _ _ = ()

    method! on_fun_def _ _ = ()

    method state = !state

    method set_state new_state = state := new_state
  end

let uls_from_ast
    ~is_closure_body
    ~has_this
    ~params
    ~is_toplevel
    ~is_in_static_method
    ~get_param_name
    ~get_param_default_value
    ~explicit_use_set_opt
    (b : Tast.program) =
  let visitor =
    declvar_visitor explicit_use_set_opt is_in_static_method is_closure_body
  in
  (* pull variables used in default values *)
  List.iter params (fun p ->
      match get_param_default_value p with
      | Some e -> visitor#on_expr () e
      | _ -> ());
  visitor#on_program () b;
  let state = visitor#state in
  let needs_local_this =
    state.dvs_bare_this = Some Bare_this_as_ref || is_in_static_method
  in
  let param_names =
    List.fold_left params ~init:ULS.empty ~f:(fun l p ->
        ULS.add l @@ get_param_name p)
  in
  let decl_vars = ULS.diff state.dvs_locals param_names in
  let decl_vars =
    if needs_local_this || is_closure_body || (not has_this) || is_toplevel then
      decl_vars
    else
      ULS.remove "$this" decl_vars
  in
  (needs_local_this && has_this, decl_vars)

(* See decl_vars.mli for details *)
let from_ast
    ~is_closure_body
    ~has_this
    ~params
    ~is_toplevel
    ~is_in_static_method
    ~explicit_use_set
    (b : Tast.program) =
  let (needs_local_this, decl_vars) =
    uls_from_ast
      ~is_closure_body
      ~has_this
      ~params
      ~is_toplevel
      ~is_in_static_method
      ~get_param_name:Hhas_param.name
      ~get_param_default_value:(fun p ->
        Option.map (Hhas_param.default_value p) ~f:snd)
      ~explicit_use_set_opt:(Some explicit_use_set)
      b
  in
  (needs_local_this, ULS.items decl_vars)

let vars_from_ast
    ~is_closure_body
    ~has_this
    ~(params : Tast.fun_param list)
    ~is_toplevel
    ~is_in_static_method
    (b : Tast.program) =
  let (_, decl_vars) =
    uls_from_ast
      ~is_closure_body
      ~has_this
      ~params
      ~is_toplevel
      ~is_in_static_method
      ~get_param_name:(fun p -> p.Aast.param_name)
      ~get_param_default_value:(fun p -> p.Aast.param_expr)
      ~explicit_use_set_opt:None
      b
  in
  ULS.items_set decl_vars
