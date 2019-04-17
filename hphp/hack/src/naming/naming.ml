(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Module "naming" a program.
 *
 * The naming phase consists in several things
 * 1- get all the global names
 * 2- transform all the local names into a unique identifier
 *)
open Core_kernel
open Common
open Utils
open String_utils

(* Alias for Nast used by functions on potentially unnamed Aast values *)
module Aast = Ast_to_nast.Aast
module N = Nast
module SN = Naming_special_names
module NS = Namespaces

module GEnv = NamingGlobal.GEnv

(*****************************************************************************)
(* The types *)
(*****************************************************************************)

(* We want to keep the positions of names that have been
 * replaced by identifiers.
 *)
type positioned_ident = (Pos.t * Local_id.t)

(* <T as A>, A is a type constraint *)
type type_constraint = N.reify_kind * (Ast.constraint_kind * Ast.hint) list
type aast_type_constraint = N.reify_kind * (Ast.constraint_kind * Aast.hint) list

let convert_type_constraints_to_aast =
  Tuple.T2.map_snd ~f:(List.map ~f:(Tuple.T2.map_snd ~f:Ast_to_nast.on_hint))

type genv = {

  (* strict? decl? partial? *)
  in_mode: FileInfo.mode;

  (* various options that control the strictness of the typechecker *)
  tcopt: TypecheckerOptions.t;

  (* are we in the body of a try statement? *)
  in_try: bool;

  (* are we in the body of a finally statement? *)
  in_finally: bool;

  (* are we in a __PPL attributed class *)
  in_ppl: bool;

  (* In function foo<T1, ..., Tn> or class<T1, ..., Tn>, the field
   * type_params knows T1 .. Tn. It is able to find out about the
   * constraint on these parameters. *)
  type_params: aast_type_constraint SMap.t;

  (* The current class, None if we are in a function *)
  current_cls: (Ast.id * Ast.class_kind) option;

  class_consts: (string, Pos.t) Caml.Hashtbl.t;

  class_props: (string, Pos.t) Caml.Hashtbl.t;

  (* Normally we don't need to add dependencies at this stage, but there
   * are edge cases when we do. *)
  droot: Typing_deps.Dep.variant;

  (* Namespace environment, e.g., what namespace we're in and what use
   * declarations are in play. *)
  namespace: Namespace_env.env;
}

(* Handler called when we see an unbound name. *)
type unbound_handler = (Pos.t * string) -> positioned_ident

(* The primitives to manipulate the naming environment *)
module Env : sig

  type lenv

  val empty_local : unbound_handler option -> lenv
  val make_class_genv :
    aast_type_constraint SMap.t ->
    FileInfo.mode ->
    Ast.id * Ast.class_kind -> Namespace_env.env -> bool -> genv
  val aast_make_class_env :
    aast_type_constraint SMap.t -> Aast.class_ -> genv * lenv
  val aast_make_typedef_env :
    aast_type_constraint SMap.t -> Aast.typedef -> genv * lenv
  val make_top_level_env :
    unit -> genv * lenv
  val make_fun_genv :
    type_constraint SMap.t ->
    FileInfo.mode -> string -> Namespace_env.env -> genv
  val aast_make_fun_decl_genv :
    aast_type_constraint SMap.t -> Aast.fun_ -> genv
  val make_file_attributes_env :
    FileInfo.mode -> Aast.nsenv -> genv * lenv
  val aast_make_const_env :
    Aast.gconst -> genv * lenv

  val has_unsafe : genv * lenv -> bool
  val set_unsafe : genv * lenv -> bool -> unit

  val in_ppl : genv * lenv -> bool
  val set_ppl : genv * lenv -> bool -> genv * lenv

  val add_lvar : genv * lenv -> Ast.id -> positioned_ident -> unit
  val aast_add_lvar : genv * lenv -> Aast.lid -> positioned_ident -> unit
  val add_param : genv * lenv -> N.fun_param -> genv * lenv
  val new_lvar : genv * lenv -> Ast.id -> positioned_ident
  val new_let_local : genv * lenv -> Ast.id -> positioned_ident
  val found_dollardollar : genv * lenv -> Pos.t -> Local_id.t
  val inside_pipe : genv * lenv -> bool
  val lvar : genv * lenv -> Ast.id -> positioned_ident
  val aast_lvar : genv * lenv -> Aast.lid -> positioned_ident
  val let_local : genv * lenv -> Ast.id -> positioned_ident option
  val global_const : genv * lenv -> Ast.id -> Ast.id
  val type_name : genv * lenv -> Ast.id -> allow_typedef:bool -> allow_generics:bool -> Ast.id
  val fun_id : genv * lenv -> Ast.id -> Ast.id
  val bind_class_const : genv * lenv -> Ast.id -> unit
  val goto_label : genv * lenv -> string -> Pos.t option
  val new_goto_label : genv * lenv -> Aast.pstring -> unit
  val new_goto_target : genv * lenv -> Aast.pstring -> unit
  val check_goto_references : genv * lenv -> unit
  val copy_let_locals : genv * lenv -> genv * lenv -> unit

  val scope : genv * lenv -> (genv * lenv -> 'a) -> 'a
  val scope_lexical : genv * lenv -> (genv * lenv -> 'a) -> 'a
  val remove_locals : genv * lenv -> Ast.id list -> unit
  val pipe_scope : genv * lenv -> (genv * lenv -> N.expr) -> Local_id.t * N.expr
end = struct

  type map = positioned_ident SMap.t

  type pipe_scope = {
    (* The identifier for the special pipe variable $$ for this pipe scope. *)
    dollardollar : Local_id.t;
    (* Whether the current pipe scope's $$ has been used. Used to raise error
     * if not. *)
    used_dollardollar : bool;
  }

  (* The local environment *)
  type lenv = {

    (* The set of locals *)
    locals: map ref;

    (* The set of lexically-scoped local `let` variables *)
    (* TODO: Currently these locals live in a separate namespace, it is
     * worthwhile considering unified namespace for all local variables T28712009 *)
    let_locals: map ref;

    (* stack of pipe scopes.
     * We use a stack (represented by a list) because pipe operators
     * can be nested. *)
    pipe_locals: pipe_scope list ref;

    (* Handler called when we see an unbound name.
     * This is used to compute an approximation of the list of captured
     * variables for closures: when we see an undefined variable, we add it
     * to the list of captured variables.
     *
     * See expr_lambda for details.
     *)
    unbound_handler: unbound_handler option;

    (* The presence of an "UNSAFE" in the function body changes the
     * verifiability of the function's return type, since the unsafe
     * block could return. For the sanity of the typechecker, we flatten
     * this out, but need to track if we've seen an "UNSAFE" in order to
     * do so. *)
    has_unsafe: bool ref;

    (**
     * A map from goto label strings to named labels.
     *)
    goto_labels: Pos.t SMap.t ref;

    (**
     * A map from goto label used in a goto statement to the position of that
     * goto label usage.
     *)
    goto_targets: Pos.t SMap.t ref;
  }

  let empty_local unbound_handler = {
    locals     = ref SMap.empty;
    let_locals = ref SMap.empty;
    pipe_locals = ref [];
    unbound_handler;
    has_unsafe = ref false;
    goto_labels = ref SMap.empty;
    goto_targets = ref SMap.empty;
  }

  let make_class_genv tparams mode (cid, ckind) namespace is_ppl =
    { in_mode       = mode;
      tcopt         = GlobalNamingOptions.get ();
      in_try        = false;
      in_finally    = false;
      in_ppl        = is_ppl;
      type_params   = tparams;
      current_cls   = Some (cid, ckind);
      class_consts  = Caml.Hashtbl.create 0;
      class_props   = Caml.Hashtbl.create 0;
      droot         = Typing_deps.Dep.Class (snd cid);
      namespace;
    }

  let unbound_name_error genv pos name kind =
    (* Naming pretends to be local and not dependent on other files, so it
     * doesn't bother with adding dependencies (even though it does look up
     * things in global state). This is mostly brushed aside because "they
     * will be added during typing". Unfortunately, there are multiple scenarios
     * when typechecker will name an expression, but gives up on typechecking
     * it. We are then left with a unrecorded dependency. This should be fixed
     * on some more basic level, but so far the only incorrectness that anyone
     * has observed due to this is that we fail to remove "unbound name" errors
     * sometimes. I add this dependency here for now to fix the annoyance it
     * causes developers. *)
    begin match kind with
      | `func -> Typing_deps.Dep.Fun name
      | `cls -> Typing_deps.Dep.Class name
      | `const -> Typing_deps.Dep.GConst name
    end |> Typing_deps.add_idep genv.droot;
    Errors.unbound_name pos name kind

  let aast_make_class_env tparams c =
    let is_ppl = List.exists
      c.Aast.c_user_attributes
      (fun { Aast.ua_name; _ } -> snd ua_name = SN.UserAttributes.uaProbabilisticModel) in
    let genv = make_class_genv tparams c.Aast.c_mode
      (c.Aast.c_name, c.Aast.c_kind) c.Aast.c_namespace is_ppl in
    let lenv = empty_local None in
    genv, lenv

  let make_typedef_genv cstrs tdef_name tdef_namespace = {
    in_mode       = FileInfo.Mstrict;
    tcopt         = GlobalNamingOptions.get ();
    in_try        = false;
    in_finally    = false;
    in_ppl        = false;
    type_params   = cstrs;
    current_cls   = None;
    class_consts = Caml.Hashtbl.create 0;
    class_props = Caml.Hashtbl.create 0;
    droot         = Typing_deps.Dep.Class tdef_name;
    namespace     = tdef_namespace;
  }

  let aast_make_typedef_env cstrs tdef =
    let genv = make_typedef_genv cstrs (snd tdef.Aast.t_name) tdef.Aast.t_namespace in
    let lenv = empty_local None in
    genv, lenv

  let make_fun_genv params f_mode f_name f_namespace = {
    in_mode       = f_mode;
    tcopt         = GlobalNamingOptions.get ();
    in_try        = false;
    in_finally    = false;
    in_ppl        = false;
    type_params   = SMap.map convert_type_constraints_to_aast params;
    current_cls   = None;
    class_consts = Caml.Hashtbl.create 0;
    class_props = Caml.Hashtbl.create 0;
    droot         = Typing_deps.Dep.Fun f_name;
    namespace     = f_namespace;
  }

  let aast_make_fun_genv params f_mode f_name f_namespace =
    { in_mode = f_mode
    ; tcopt = GlobalNamingOptions.get ()
    ; in_try = false
    ; in_finally = false
    ; in_ppl = false
    ; type_params = params
    ; current_cls = None
    ; class_consts = Caml.Hashtbl.create 0
    ; class_props = Caml.Hashtbl.create 0
    ; droot = Typing_deps.Dep.Fun f_name
    ; namespace = f_namespace
    }

  let aast_make_fun_decl_genv params f =
    aast_make_fun_genv params f.Aast.f_mode (snd f.Aast.f_name) f.Aast.f_namespace

  let aast_make_const_genv cst = {
    in_mode       = cst.Aast.cst_mode;
    tcopt         = GlobalNamingOptions.get ();
    in_try        = false;
    in_finally    = false;
    in_ppl        = false;
    type_params   = SMap.empty;
    current_cls   = None;
    class_consts = Caml.Hashtbl.create 0;
    class_props = Caml.Hashtbl.create 0;
    droot         = Typing_deps.Dep.GConst (snd cst.Aast.cst_name);
    namespace     = cst.Aast.cst_namespace;
  }

  let make_top_level_genv () = {
    in_mode       = FileInfo.Mpartial;
    tcopt         = GlobalNamingOptions.get ();
    in_try        = false;
    in_finally    = false;
    in_ppl        = false;
    type_params   = SMap.empty;
    current_cls   = None;
    class_consts = Caml.Hashtbl.create 0;
    class_props = Caml.Hashtbl.create 0;
    droot         = Typing_deps.Dep.Fun "";
    namespace     = Namespace_env.empty_with_default_popt;
  }

  let make_top_level_env () =
    let genv = make_top_level_genv () in
    let lenv = empty_local None in
    let env  = genv, lenv in
    env

  let make_file_attributes_genv mode namespace =
  {
    in_mode       = mode;
    tcopt         = GlobalNamingOptions.get ();
    in_try        = false;
    in_finally    = false;
    in_ppl        = false;
    type_params   = SMap.empty;
    current_cls   = None;
    class_consts  = Caml.Hashtbl.create 0;
    class_props   = Caml.Hashtbl.create 0;
    droot         = Typing_deps.Dep.Fun "";
    namespace     = namespace;
  }

  let make_file_attributes_env mode namespace =
    let genv = make_file_attributes_genv mode namespace in
    let lenv = empty_local None in
    let env  = genv, lenv in
    env

  let aast_make_const_env cst =
    let genv = aast_make_const_genv cst in
    let lenv = empty_local None in
    let env  = genv, lenv in
    env

  let has_unsafe (_genv, lenv) = !(lenv.has_unsafe)
  let set_unsafe (_genv, lenv) x =
    lenv.has_unsafe := x

  let in_ppl (genv, _lenv) = genv.in_ppl

  let set_ppl (genv, lenv) in_ppl =
    let genv = { genv with in_ppl } in
    (genv, lenv)

  let lookup genv (env : string -> FileInfo.pos option) (p, x) =
    let v = env x in
    match v with
    | None ->
      (match genv.in_mode with
        | FileInfo.Mstrict | FileInfo.Mexperimental
        | FileInfo.Mpartial | FileInfo.Mdecl ->
          unbound_name_error genv p x `const
        | FileInfo.Mphp -> ()
      )
    | _ -> ()

  let handle_unbound_name genv get_full_pos get_canon (p, name) kind =
    match get_canon name with
      | Some canonical ->
        canonical
        |> get_full_pos
        |> Option.iter ~f:(fun p_canon ->
          Errors.did_you_mean_naming p name p_canon canonical);
        (* Recovering from the capitalization error means
         * returning the name in its canonical form *)
        p, canonical
      | None ->
        (match genv.in_mode with
          | FileInfo.Mpartial | FileInfo.Mdecl
              when name = SN.Classes.cUnknown -> ()
          | FileInfo.Mphp -> ()
          | FileInfo.Mstrict | FileInfo.Mexperimental -> unbound_name_error genv p name kind
          | FileInfo.Mpartial | FileInfo.Mdecl ->
              unbound_name_error genv p name kind
        );
        p, name

  let canonicalize genv get_pos get_full_pos get_canon (p, name) kind =
    (* Get the canonical name to check if the name exists in the heap *)
    match get_pos name with
    | Some _ -> p, name
    | None -> handle_unbound_name genv get_full_pos get_canon (p, name) kind

  (* Adds a local variable, without any check *)
  let add_lvar (_, lenv) (_, name) (p, x) =
    lenv.locals := SMap.add name (p, x) !(lenv.locals);
    ()

  let aast_add_lvar (_, lenv) (_, lid) (p, x) =
    lenv.locals := SMap.add (Local_id.to_string lid) (p, x) !(lenv.locals);
    ()

  let add_param env param =
    let p_name = param.N.param_name in
    let id = Local_id.make_unscoped p_name in
    let p_pos = param.N.param_pos in
    let () = add_lvar env (p_pos, p_name) (p_pos, id) in
    env

  (* Defines a new local variable.
     Side effects:
     1) if the local is not in the local environment then it is added.
     Return value: the given position and deduced/created identifier. *)
  let new_lvar (_, lenv) (p, x) =
    let lcl = SMap.get x !(lenv.locals) in
    let ident =
      match lcl with
      | Some lcl -> snd lcl
      | None ->
          let ident = Local_id.make_unscoped x in
          lenv.locals := SMap.add x (p, ident) !(lenv.locals);
          ident
    in
    p, ident

  (* Defines a new scoped local variable
   * Side effects:
   * Always add a new variable in the local environment.
   * If the variable has been defined already, shadow the previously-defined
   * variable *)
   (* TODO: Emit warning if names are getting shadowed T28436131 *)
  let new_let_local (_, lenv) (p, x) =
    let ident = Local_id.make_scoped x in
    lenv.let_locals := SMap.add x (p, ident) !(lenv.let_locals);
    p, ident

  (* Check $$ is defined (i.e. we are indeed in a pipe) and if yes, mark it
   * as used and get the identifier for it. *)
  let found_dollardollar (_, lenv) p =
    match !(lenv.pipe_locals) with
    | [] ->
      Errors.undefined ~in_rx_scope:false p SN.SpecialIdents.dollardollar; (* TODO better error *)
      Local_id.make_scoped SN.SpecialIdents.dollardollar
    | pipe_scope :: scopes ->
      let pipe_scope = { pipe_scope with used_dollardollar = true } in
      lenv.pipe_locals := pipe_scope :: scopes;
      pipe_scope.dollardollar

  let inside_pipe (_, lenv) =
    match !(lenv.pipe_locals) with
    | [] -> false
    | _ -> true

  let handle_undefined_variable (_genv, env) (p, x) =
    match env.unbound_handler with
    | None -> p, Local_id.make_unscoped x
    | Some f -> f (p, x)

  (* Function used to name a local variable *)
  let lvar (genv, env) (p, x) =
    let p, ident =
      if SN.Superglobals.is_superglobal x && genv.in_mode = FileInfo.Mpartial
      then p, Local_id.make_unscoped x
      else
        let lcl = SMap.get x !(env.locals) in
        match lcl with
        | Some lcl -> p, snd lcl
        | None -> handle_undefined_variable (genv, env) (p, x)
    in
    p, ident

  let aast_lvar (genv, env) (p, x) =
    lvar (genv, env) (p, Local_id.to_string x)

  let let_local (_genv, env) (p, x) =
    let lcl = SMap.get x !(env.let_locals) in
    match lcl with
      | Some lcl -> Some (p, snd lcl)
      | None -> None

  let get_name genv get_pos x =
    lookup genv get_pos x; x

  (* For dealing with namespace resolution on functions *)
  let elaborate_and_get_name_with_canonicalized_fallback
      genv
      (get_pos : string -> FileInfo.pos option)
      (get_full_pos : string -> Pos.t option)
      get_canon x =
    let get_name x = get_name genv get_pos x in
    let canonicalize = canonicalize genv get_pos get_full_pos get_canon in
    let fq_x = NS.elaborate_id genv.namespace NS.ElaborateFun x in
    let fq_x = canonicalize fq_x `func in
    get_name fq_x

  let global_const (genv, _env) x =
    let fq_x = NS.elaborate_id genv.namespace NS.ElaborateConst x in
    get_name genv (Naming_table.Consts.get_pos) fq_x

  let type_name (genv, _) x ~allow_typedef ~allow_generics =
    let (p, name) = x in
    match SMap.find_opt name genv.type_params with
    | Some (reified, _) ->
      if not allow_generics then Errors.generics_not_allowed p;
      begin match reified with
      | N.Erased -> Errors.generic_at_runtime p "Erased"
      | N.SoftReified -> Errors.generic_at_runtime p "Soft reified"
      | N.Reified -> () end;
      x
    | None ->
      let (pos, name) as x = NS.elaborate_id genv.namespace NS.ElaborateClass x in
      match Naming_table.Types.get_pos name with
      | Some (_def_pos, Naming_table.TClass) ->
        (* Don't let people use strictly internal classes
         * (except when they are being declared in .hhi files) *)
        if name = SN.Classes.cHH_BuiltinEnum &&
          not (string_ends_with (Relative_path.suffix (Pos.filename pos)) ".hhi")
        then Errors.using_internal_class pos (strip_ns name);
        pos, name
      | Some (def_pos, Naming_table.TTypedef) when not allow_typedef ->
        let full_pos, _ = GEnv.get_full_pos (def_pos, name) in
        Errors.unexpected_typedef pos full_pos;
        pos, name
      | Some (_def_pos, Naming_table.TTypedef) -> pos, name
      | None ->
        handle_unbound_name genv
          GEnv.type_pos
          GEnv.type_canon_name x `cls

  let fun_id (genv, _) x =
    elaborate_and_get_name_with_canonicalized_fallback
      genv
      (Naming_table.Funs.get_pos)
      GEnv.fun_pos
      GEnv.fun_canon_name
      x

  let bind_class_member tbl (p, x) =
    try
      let p' = Caml.Hashtbl.find tbl x in
      Errors.error_name_already_bound x x p p'
    with Caml.Not_found ->
      Caml.Hashtbl.replace tbl x p

  let bind_class_const (genv, _env) (p, x) =
    if String.lowercase x = "class" then Errors.illegal_member_variable_class p;
    bind_class_member genv.class_consts (p, x)

  (**
   * Returns the position of the goto label declaration, if it exists.
   *)
  let goto_label (_, { goto_labels; _ }) label =
    SMap.get label !goto_labels

  (**
   * Adds a goto label and the position of its declaration to the known labels.
   *)
  let new_goto_label (_, { goto_labels; _ }) (pos, label) =
    goto_labels := SMap.add label pos !goto_labels

  (**
   * Adds a goto target and its reference position to the known targets.
   *)
  let new_goto_target (_, { goto_targets; _ }) (pos, label) =
    goto_targets := SMap.add label pos !goto_targets

  (**
   * Ensures that goto statements do not reference goto labels that are not
   * known within the current lenv.
   *)
  let check_goto_references (_, { goto_labels; goto_targets; _ }) =
    let check_label referenced_label referenced_label_pos =
      if not (SMap.mem referenced_label !goto_labels) then
        Errors.goto_label_undefined referenced_label_pos referenced_label in
    SMap.iter check_label !goto_targets

  (* Scope, keep the locals, go and name the body, and leave the
   * local environment intact
   *)
  let scope env f =
    let _genv, lenv = env in
    let lenv_copy = !(lenv.locals) in
    let lenv_scoped_copy = !(lenv.let_locals) in
    let res = f env in
    lenv.locals := lenv_copy;
    lenv.let_locals := lenv_scoped_copy;
    res

  let remove_locals env vars =
    let _genv, lenv = env in
    lenv.locals :=
      List.fold_left vars ~f:(fun l id -> SMap.remove (snd id) l) ~init:!(lenv.locals)

  (* Add a new lexical scope for block-scoped `let` variables.
     No other changes in the local environment *)
  let scope_lexical env f =
    let _genv, lenv = env in
    let lenv_scoped_copy = !(lenv.let_locals) in
    let res = f env in
    lenv.let_locals := lenv_scoped_copy;
    res

  (* Copy the let locals from lenv1 to lenv2 *)
  let copy_let_locals (_genv1, lenv1) (_genv2, lenv2) =
    let let_locals_1 = !(lenv1.let_locals) in
    lenv2.let_locals := let_locals_1

  (** Push a new pipe scope on the stack of pipe scopes in the environment
   * and create an identifier for the $$ variable associated to this pipe,
   * then perform the naming function [name_e2],
   * then finally pops the added pipe scope.
   * Append an error if $$ was not used in the RHS.
   *
   * Return the identifier for the $$ of this pipe and the names RHS.
   * *)
  let pipe_scope env name_e2 =
    let _, lenv = env in
    let pipe_var_ident = Local_id.make_scoped SN.SpecialIdents.dollardollar in
    let pipe_scope = {
      dollardollar = pipe_var_ident;
      used_dollardollar = false;
    } in
    lenv.pipe_locals := pipe_scope :: !(lenv.pipe_locals);
    (** Name the RHS of the pipe expression. *)
    let e2 = name_e2 env in
    let (pipe_scope, pipe_scopes) = match !(lenv.pipe_locals) with
      | [] -> assert false
      | pipe_scope :: pipe_scopes -> (pipe_scope, pipe_scopes) in
    if not pipe_scope.used_dollardollar then
      Errors.dollardollar_unused (fst e2);
    lenv.pipe_locals := pipe_scopes;
    pipe_var_ident, e2
end

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let aast_check_constraint { Aast.tp_name = (pos, name); _ } =
  if String.lowercase name = "this"
  then Errors.this_reserved pos
  else if name.[0] <> 'T' then Errors.start_with_T pos

let aast_check_repetition s param =
  let name = param.Aast.param_name in
  if SSet.mem name s
  then Errors.already_bound param.Aast.param_pos name;
  if name <> SN.SpecialIdents.placeholder
  then SSet.add name s
  else s

let convert_shape_name env = function
  | Ast.SFlit_int (pos, s) -> (pos, Ast.SFlit_int (pos, s))
  | Ast.SFlit_str (pos, s) -> (pos, Ast.SFlit_str (pos, s))
  | Ast.SFclass_const (x, (pos, y)) ->
    let class_name =
      if (snd x) = SN.Classes.cSelf then
        match (fst env).current_cls with
        | Some (cid, _) -> cid
        | None -> Errors.self_outside_class pos; (pos, SN.Classes.cUnknown)
      else Env.type_name env x ~allow_typedef:false ~allow_generics:false in
    (pos, Ast.SFclass_const (class_name, (pos, y)))

let arg_unpack_unexpected = function
  | [] -> ()
  | (pos, _) :: _ -> Errors.naming_too_few_arguments pos; ()

module type GetLocals = sig
  val stmt : Namespace_env.env * Pos.t SMap.t ->
    Ast.stmt -> Namespace_env.env * Pos.t SMap.t
  val lvalue : Namespace_env.env * Pos.t SMap.t ->
    Ast.expr -> Namespace_env.env * Pos.t SMap.t

  val aast_lvalue : Namespace_env.env * Pos.t SMap.t ->
    Aast.expr -> Namespace_env.env * Pos.t SMap.t
  val aast_stmt : Namespace_env.env * Pos.t SMap.t ->
    Aast.stmt -> Namespace_env.env * Pos.t SMap.t
end

(* This was made a functor due to the awkward nature of how our naming
 * code is structured.
 *
 * Naming is called both in the decl phase and type-check phase. In the
 * decl phase it's mostly used to construct things that do not belong in
 * function bodies; examples include classes, their member fields, and
 * global constants. This part of naming is entirely self-contained; it
 * only uses the data from the AST in the current file, and does not need
 * to cross-reference decl type data from other files.
 *
 * In the type-check phase, Naming is invoked again, this time to name the
 * bodies of functions. Now it requires decl type data in order to know
 * which function calls are marked as `noreturn`, because this affects
 * which local variables are considered to be defined at the end of a
 * statement.
 *
 * So decl depends on naming, but naming also depends on decl, creating
 * a circular dependency. The obvious solution would be to split it into
 * two, but this is nontrivial because decl-phase naming also does some
 * naming of expressions -- for example, constant initializers and default
 * parameter values have them. Of course, none of these expressions can
 * actually contain local variables, but our code is not written in a way
 * that the OCaml type system can understand that. So as a hacky solution,
 * I'm parameterizing GetLocals so that it is a no-op in the decl phase
 * but can be properly instantiated with Typing_get_locals in the typing
 * phase.
 *)
module Make (GetLocals : GetLocals) = struct
  (************************************************************************)
  (* Naming of type hints *)
  (************************************************************************)
  (**
   * The existing hint function goes from Ast.hint -> Nast.hint
   * This hint function goes from Aast.hint -> Nast.hint
   * Used with with Ast_to_nast to go from Ast.hint -> Nast.hint
   *)
  let rec aast_hint
      ?(forbid_this=false)
      ?(allow_retonly=false)
      ?(allow_typedef=true)
      ?(allow_wildcard=false)
      ?(in_where_clause=false)
      ?(tp_depth=0)
      env (hh : Aast.hint) =
    let mut, (p, h) = aast_unwrap_mutability hh in
    if Option.is_some mut
    then Errors.misplaced_mutability_hint p;
    p, aast_hint_
      ~forbid_this
      ~allow_retonly
      ~allow_typedef
      ~allow_wildcard
      ~in_where_clause
      ~tp_depth
      env (p, h)

  and aast_unwrap_mutability p =
    match p with
    | _, Aast.Happly ((_, "Mutable"), [t]) -> Some N.PMutable, t
    | _, Aast.Happly ((_, "MaybeMutable"), [t]) -> Some N.PMaybeMutable, t
    | _, Aast.Happly ((_, "OwnedMutable"), [t]) -> Some N.POwnedMutable, t
    | t -> None, t

  and aast_hfun env reactivity is_coroutine hl kl variadic_hint h =
    let variadic_hint =
      match variadic_hint with
      | Aast.Hnon_variadic
      | Aast.Hvariadic None -> variadic_hint
      | Aast.Hvariadic (Some h) -> N.Hvariadic (Some (aast_hint env h)) in
    let muts, hl =
      List.map ~f:(fun h ->
        let mut, h1 = aast_unwrap_mutability h in
        if Option.is_some mut && reactivity = N.FNonreactive
        then Errors.mutability_hint_in_non_rx_function (fst h);
        mut, aast_hint env h1)
        hl
      |> List.unzip in
    let ret_mut, rh = aast_unwrap_mutability h in
    let ret_mut =
      match ret_mut with
      | None -> false
      | Some N.POwnedMutable -> true
      | Some _ ->
        Errors.invalid_mutability_in_return_type_hint (fst h);
        true in
    N.Hfun (reactivity, is_coroutine, hl, kl, muts,
      variadic_hint, aast_hint ~allow_retonly:true env rh, ret_mut)

  and aast_hint_ ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard
            ~in_where_clause ?(tp_depth=0)
        env (p, x) =
    let tcopt = (fst env).tcopt in
    let pu_enabled = TypecheckerOptions.experimental_feature_enabled
      tcopt GlobalOptions.tco_experimental_pocket_universes in
    let like_types_enabled = TypecheckerOptions.(
      new_inference tcopt && experimental_feature_enabled tcopt experimental_like_types
    ) in
    let aast_hint =
      aast_hint ~forbid_this ~allow_typedef ~allow_wildcard in
    match x with
    | Aast.Htuple hl ->
      N.Htuple (List.map hl ~f:(aast_hint ~allow_retonly ~tp_depth:(tp_depth+1) env))
    | Aast.Hoption h ->
      (* void/noreturn are permitted for Typing.option_return_only_typehint *)
      N.Hoption (aast_hint ~allow_retonly env h)
    | Aast.Hlike h ->
      if not like_types_enabled then
        Errors.experimental_feature p "like-types";
      N.Hlike (aast_hint ~allow_retonly env h)
    | Aast.Hsoft h ->
      let h = aast_hint ~allow_retonly env h
      in snd h
    | Aast.Hfun (reactivity, coroutine, hl, kl, _, variadic_hint, h, _) ->
      aast_hfun env reactivity coroutine hl kl variadic_hint h
    (* Special case for Rx<function> *)
    | Aast.Happly
      ((_, "Rx"), [(_, Aast.Hfun (_, is_coroutine, hl, kl, _, variadic_hint, h, _))]) ->
        aast_hfun env N.FReactive is_coroutine hl kl variadic_hint h
    (* Special case for RxShallow<function> *)
    | Aast.Happly
      ((_, "RxShallow"),
      [(_, Aast.Hfun (_, is_coroutine, hl, kl, _, variadic_hint, h, _))]) ->
        aast_hfun env N.FShallow is_coroutine hl kl variadic_hint h
    (* Special case for RxLocal<function> *)
    | Aast.Happly
      ((_, "RxLocal"),
      [(_, Aast.Hfun (_, is_coroutine, hl, kl, _, variadic_hint, h, _))]) ->
        aast_hfun env N.FLocal is_coroutine hl kl variadic_hint h
    | Aast.Happly ((p, _x) as id, hl) ->
      let hint_id =
        aast_hint_id ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard ~tp_depth
          env id
          hl in
      (match hint_id with
      | N.Hprim _ | N.Hmixed | N.Hnonnull | N.Hnothing ->
        if hl <> [] then Errors.unexpected_type_arguments p
      | _ -> ()
      );
      hint_id
    | Aast.Haccess ((pos, root_id), ids) ->
      let root_ty =
        match root_id with
        | Aast.Happly ((pos, x), _) when x = SN.Classes.cSelf ->
          begin
            match (fst env).current_cls with
            | None -> Errors.self_outside_class pos; N.Hany
            | Some (cid, _) -> N.Happly (cid, [])
          end
        | Aast.Happly ((pos, x), _) when x = SN.Classes.cStatic || x = SN.Classes.cParent ->
          Errors.invalid_type_access_root (pos, x); N.Hany
        | Aast.Happly (root, _) ->
          let h = aast_hint_id ~forbid_this ~allow_retonly ~allow_typedef
            ~allow_wildcard:false ~tp_depth env root [] in
          begin
            match h with
            | N.Hthis
            | N.Happly _ -> h
            (* Pocket Universes: we want to be able to write `FieldName::Type`
               in various locations (class, pu enum definitions, function
               signatures), not just in `where` clauses.
               This syntax is not (yet) allowed in Hack so
               I guard its handling behind the experimental PU flag.
            *)
            | N.Habstr _ when in_where_clause && not pu_enabled -> h
            | N.Habstr _ when pu_enabled -> h
            | _ -> Errors.invalid_type_access_root root; N.Hany
          end
        | _ ->
          Errors.internal_error
            pos
            "Malformed hint: expected Haccess (Happly ...) from ast_to_nast";
          N.Hany
      in
      N.Haccess ((pos, root_ty), ids)
    | Aast.Hshape { Aast.nsi_allows_unknown_fields; nsi_field_map } ->
      let nsi_field_map =
        List.map
          ~f:(fun { Aast.sfi_optional; sfi_hint; sfi_name } ->
            let _pos, new_key = convert_shape_name env sfi_name in
            let new_field =
              { N.sfi_optional;
                sfi_hint = aast_hint ~allow_retonly ~tp_depth:(tp_depth+1) env sfi_hint;
                sfi_name = new_key
              } in
            new_field)
          nsi_field_map in
      N.Hshape { N.nsi_allows_unknown_fields; nsi_field_map }
    | Aast.Hany
    | Aast.Hmixed
    | Aast.Hnonnull
    | Aast.Habstr _
    | Aast.Harray _
    | Aast.Hdarray _
    | Aast.Hvarray _
    | Aast.Hvarray_or_darray _
    | Aast.Hprim _
    | Aast.Hthis
    | Aast.Hdynamic
    | Aast.Hnothing ->
      Errors.internal_error Pos.none "Unexpected hint not present on legacy AST";
      N.Hany

  and aast_hint_id ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard ~tp_depth
    env (p, x as id) hl =
    let params = (fst env).type_params in
    (* some common Xhp screw ups *)
    if   (x = "Xhp") || (x = ":Xhp") || (x = "XHP")
    then Errors.disallowed_xhp_type p x;
    match aast_try_castable_hint ~forbid_this ~allow_wildcard ~tp_depth env p x hl with
    | Some h -> h
    | None -> begin
      match x with
        | x when x = SN.Typehints.wildcard ->
          if allow_wildcard && tp_depth >= 1 (* prevents 3 as _ *)
          then
            if hl <> [] then
              (Errors.tparam_with_tparam p x;
              N.Hany)
            else
              N.Happly(id, [])
          else
            (Errors.wildcard_disallowed p;
            N.Hany)
        | x when
          (  x = ("\\"^SN.Typehints.void)
          || x = ("\\"^SN.Typehints.null)
          || x = ("\\"^SN.Typehints.noreturn)
          || x = ("\\"^SN.Typehints.int)
          || x = ("\\"^SN.Typehints.bool)
          || x = ("\\"^SN.Typehints.float)
          || x = ("\\"^SN.Typehints.num)
          || x = ("\\"^SN.Typehints.string)
          || x = ("\\"^SN.Typehints.resource)
          || x = ("\\"^SN.Typehints.mixed)
          || x = ("\\"^SN.Typehints.nonnull)
          || x = ("\\"^SN.Typehints.array)
          || x = ("\\"^SN.Typehints.arraykey)
          || x = ("\\"^SN.Typehints.integer)
          || x = ("\\"^SN.Typehints.boolean)
          || x = ("\\"^SN.Typehints.double)
          || x = ("\\"^SN.Typehints.real)
          ) ->
          Errors.primitive_toplevel p;
          N.Hany
      | x when x = "\\"^SN.Typehints.nothing &&
                 TypecheckerOptions.new_inference (fst env).tcopt ->
         Errors.primitive_toplevel p;
         N.Hany
      | x when x = SN.Typehints.void && allow_retonly -> N.Hprim N.Tvoid
      | x when x = SN.Typehints.void ->
         Errors.return_only_typehint p `void;
         N.Hany
      | x when x = SN.Typehints.noreturn && allow_retonly -> N.Hprim N.Tnoreturn
      | x when x = SN.Typehints.noreturn ->
        Errors.return_only_typehint p `noreturn;
        N.Hany
      | x when x = SN.Typehints.null -> N.Hprim N.Tnull
      | x when x = SN.Typehints.num  -> N.Hprim N.Tnum
      | x when x = SN.Typehints.resource -> N.Hprim N.Tresource
      | x when x = SN.Typehints.arraykey -> N.Hprim N.Tarraykey
      | x when x = SN.Typehints.mixed -> N.Hmixed
      | x when x = SN.Typehints.nonnull -> N.Hnonnull
      | x when x = SN.Typehints.dynamic -> N.Hdynamic
      | x when x = SN.Typehints.nothing &&
                 TypecheckerOptions.new_inference (fst env).tcopt ->
        N.Hnothing
      | x when x = SN.Typehints.this && not forbid_this ->
          if not (phys_equal hl [])
          then Errors.this_no_argument p;
          (match (fst env).current_cls with
          | None ->
            Errors.this_hint_outside_class p;
            N.Hany
          | Some _c ->
            N.Hthis
          )
      | x when x = SN.Typehints.this ->
          (match (fst env).current_cls with
          | None ->
              Errors.this_hint_outside_class p
          | Some _ ->
              Errors.this_type_forbidden p
          );
          N.Hany
      | x when x = SN.Classes.cClassname && (List.length hl) <> 1 ->
          Errors.classname_param p;
          N.Hprim N.Tstring
      | _ when String.lowercase x = SN.Typehints.this ->
          Errors.lowercase_this p x;
          N.Hany
      | _ when SMap.mem x params ->
          if hl <> [] then
          Errors.tparam_with_tparam p x;
          N.Habstr x
      | _ ->
        let name = Env.type_name env id ~allow_typedef ~allow_generics:false in
        (* Note that we are intentionally setting allow_typedef to `true` here.
         * In general, generics arguments can be typedefs -- there is no
         * runtime restriction. *)
        N.Happly (name, aast_hintl ~allow_wildcard ~forbid_this ~allow_typedef:true
          ~allow_retonly:true ~tp_depth:(tp_depth+1) env hl)
    end

  (* Hints that are valid both as casts and type annotations.  Neither
   * casts nor annotations are a strict subset of the other: For
   * instance, 'object' is not a valid annotation.  Thus callers will
   * have to handle the remaining cases. *)
  and aast_try_castable_hint ?(forbid_this=false) ?(allow_wildcard=false) ~tp_depth env p x hl =
    let aast_hint =
      aast_hint ~forbid_this ~tp_depth:(tp_depth+1) ~allow_wildcard ~allow_retonly:false in
    let canon = String.lowercase x in
    let opt_hint = match canon with
      | nm when nm = SN.Typehints.int    -> Some (N.Hprim N.Tint)
      | nm when nm = SN.Typehints.bool   -> Some (N.Hprim N.Tbool)
      | nm when nm = SN.Typehints.float  -> Some (N.Hprim N.Tfloat)
      | nm when nm = SN.Typehints.string -> Some (N.Hprim N.Tstring)
      | nm when nm = SN.Typehints.array  ->
        let tcopt = (fst env).tcopt in
        let array_typehints_disallowed =
          TypecheckerOptions.disallow_array_typehint tcopt in
        if array_typehints_disallowed
        then Errors.array_typehints_disallowed p;
        Some (match hl with
          | [] -> N.Harray (None, None)
          | [val_] -> N.Harray (Some (aast_hint env val_), None)
          | [key_; val_] ->
            N.Harray (Some (aast_hint env key_), Some (aast_hint env val_))
          | _ -> Errors.too_many_type_arguments p; N.Hany
        )
      | nm when nm = SN.Typehints.darray ->
        Some (match hl with
          | [] ->
              if FileInfo.is_strict (fst env).in_mode then
                Errors.too_few_type_arguments p;
              N.Hdarray ((p, N.Hany), (p, N.Hany))
          | [_] -> Errors.too_few_type_arguments p; N.Hany
          | [key_; val_] -> N.Hdarray (aast_hint env key_, aast_hint env val_)
          | _ -> Errors.too_many_type_arguments p; N.Hany)
      | nm when nm = SN.Typehints.varray ->
        Some (match hl with
          | [] ->
              if FileInfo.is_strict (fst env).in_mode then
                Errors.too_few_type_arguments p;
              N.Hvarray (p, N.Hany)
          | [val_] -> N.Hvarray (aast_hint env val_)
          | _ -> Errors.too_many_type_arguments p; N.Hany)
      | nm when nm = SN.Typehints.varray_or_darray ->
        Some (match hl with
          | [] ->
              if FileInfo.is_strict (fst env).in_mode then
                Errors.too_few_type_arguments p;
              N.Hvarray_or_darray (p, N.Hany)
          | [val_] -> N.Hvarray_or_darray (aast_hint env val_)
          | _ -> Errors.too_many_type_arguments p; N.Hany)
      | nm when nm = SN.Typehints.integer ->
        Errors.primitive_invalid_alias p nm SN.Typehints.int;
        Some (N.Hprim N.Tint)
      | nm when nm = SN.Typehints.boolean ->
        Errors.primitive_invalid_alias p nm SN.Typehints.bool;
        Some (N.Hprim N.Tbool)
      | nm when nm = SN.Typehints.double || nm = SN.Typehints.real ->
        Errors.primitive_invalid_alias p nm SN.Typehints.float;
        Some (N.Hprim N.Tfloat)
      | _ -> None
    in
    let () = match opt_hint with
      | Some _ when canon <> x -> Errors.primitive_invalid_alias p x canon
      | _ -> ()
    in opt_hint

  and aast_hintl ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard ~tp_depth env l =
      List.map
        ~f:(aast_hint ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard ~tp_depth env)
        l

  let aast_constraint_ ?(forbid_this=false) env (ck, h) = ck, aast_hint ~forbid_this env h

  let aast_targ env t =
      aast_hint
        ~allow_wildcard:true
        ~forbid_this:false
        ~allow_typedef:true
        ~allow_retonly:true
        ~tp_depth:1
        env t
  let aast_targl env _ tal =
    List.map tal ~f:(aast_targ env)


  (**************************************************************************)
  (* All the methods and static methods of an interface are "implicitly"
   * declared as abstract
   *)
  (**************************************************************************)

  let add_abstract m = {m with N.m_abstract = true}

  let add_abstractl methods = List.map methods add_abstract

  let aast_interface c constructor methods smethods =
    if c.Aast.c_kind <> Ast.Cinterface
    then constructor, methods, smethods
    else
      let constructor = Option.map constructor add_abstract in
      let methods = add_abstractl methods in
      let smethods = add_abstractl smethods in
      constructor, methods, smethods

  (**************************************************************************)
  (* Checking for collision on method names *)
  (**************************************************************************)

  let check_method acc { N.m_name = (p, x); _ } =
    if SSet.mem x acc
    then Errors.method_name_already_bound p x;
    SSet.add x acc

  let check_name_collision methods =
    ignore (List.fold_left methods ~init:SSet.empty ~f:check_method)

  (**************************************************************************)
  (* Checking for shadowing of method type parameters *)
  (**************************************************************************)

  let check_method_tparams class_tparam_names { N.m_tparams = tparams; _ } =
    List.iter tparams begin fun { N.tp_name = (p,x); _ } ->
      List.iter class_tparam_names
        (fun (pc,xc) -> if (x = xc) then Errors.shadowed_type_param p pc x)
    end

  let check_tparams_constructor class_tparam_names constructor =
    match constructor with
    | None -> ()
    | Some constr -> check_method_tparams class_tparam_names constr

  let check_tparams_shadow class_tparam_names methods =
    List.iter methods (check_method_tparams class_tparam_names)

  let aast_ensure_name_not_dynamic env e err =
    match e with
    | (_, (Aast.Id _ | Aast.Lvar _)) -> ()
    | (p, _) ->
      if FileInfo.is_strict (fst env).in_mode
      then err p

  (* Naming of a class *)
  let rec class_ c =
    let c = Ast_to_nast.on_class c in
    aast_class_ c

  (* Naming of a class *)
  and aast_class_ c =
    let constraints = aast_make_constraints c.Aast.c_tparams.Aast.c_tparam_list in
    let env = Env.aast_make_class_env constraints c in
    (* Checking for a code smell *)
    List.iter c.Aast.c_tparams.Aast.c_tparam_list aast_check_constraint;
    let name = Env.type_name env c.Aast.c_name ~allow_typedef:false ~allow_generics:false in
    let smethods = List.map ~f:(aast_method_ (fst env)) c.Aast.c_static_methods in
    let sprops = List.map ~f:(aast_class_prop_static env) c.Aast.c_static_vars in
    let attrs = aast_user_attributes env c.Aast.c_user_attributes in
    let const = (Attributes.find SN.UserAttributes.uaConst attrs) in
    let props = List.map ~f:(aast_class_prop_non_static ~const env) c.Aast.c_vars in
    let xhp_attrs = List.map ~f:(aast_xhp_attribute_decl env) c.Aast.c_xhp_attrs in
    (* These would be out of order with the old attributes, but that shouldn't matter? *)
    let props = props @ xhp_attrs in
    let parents =
      List.map c.Aast.c_extends
        (aast_hint ~allow_retonly:false ~allow_typedef:false env) in
    let parents =
      match c.Aast.c_kind with
      (* Make enums implicitly extend the BuiltinEnum class in order to provide
       * utility methods. *)
      | Ast.Cenum ->
        let pos = fst name in
        let enum_type = pos, N.Happly (name, []) in
        let parent =
          pos, N.Happly ((pos, Naming_special_names.Classes.cHH_BuiltinEnum),
                          [enum_type]) in
        parent::parents
      | _ -> parents in
    let methods = List.map ~f:(aast_class_method env) c.Aast.c_methods in
    let uses = List.map ~f:(aast_hint ~allow_typedef:false env) c.Aast.c_uses in
    let pu_enums = List.map ~f:(aast_class_pu_enum env) c.Aast.c_pu_enums in
    let redeclarations =
      List.map ~f:(aast_method_redeclaration env) c.Aast.c_method_redeclarations in
    let xhp_attr_uses =
      List.map ~f:(aast_hint ~allow_typedef:false env) c.Aast.c_xhp_attr_uses in
    if c.Aast.c_req_implements <> [] && c.Aast.c_kind <> Ast.Ctrait
    then Errors.invalid_req_implements (fst (List.hd_exn c.Aast.c_req_implements));
    let req_implements =
      List.map ~f:(aast_hint ~allow_typedef:false env) c.Aast.c_req_implements in
    if c.Aast.c_req_extends <> [] &&
       c.Aast.c_kind <> Ast.Ctrait &&
       c.Aast.c_kind <> Ast.Cinterface
    then Errors.invalid_req_extends (fst (List.hd_exn c.Aast.c_req_extends));
    let req_extends =
      List.map ~f:(aast_hint ~allow_typedef:false env) c.Aast.c_req_extends in
    (* Setting a class type parameters constraint to the 'this' type is weird
     * so lets forbid it for now.
     *)
    let tparam_l = aast_type_paraml ~forbid_this:true env c.Aast.c_tparams.Aast.c_tparam_list in
    let consts = List.map ~f:(aast_class_const env) c.Aast.c_consts in
    let typeconsts = List.map ~f:(aast_typeconst env) c.Aast.c_typeconsts in
    let implements =
      List.map
        ~f:(aast_hint ~allow_retonly:false ~allow_typedef:false env)
        c.Aast.c_implements in
    let constructor = Option.map c.Aast.c_constructor (aast_method_ (fst env)) in
    let constructor, methods, smethods =
      aast_interface c constructor methods smethods in
    let class_tparam_names =
      List.map ~f:(fun tp -> tp.Aast.tp_name) c.Aast.c_tparams.Aast.c_tparam_list in
    let enum = Option.map c.Aast.c_enum (aast_enum_ env) in
    let file_attributes =
      aast_file_attributes c.Aast.c_mode c.Aast.c_file_attributes in
    let c_tparams =
      { N.c_tparam_list = tparam_l
      ; N.c_tparam_constraints = constraints
      } in
    check_tparams_constructor class_tparam_names constructor;
    check_name_collision methods;
    check_tparams_shadow class_tparam_names methods;
    check_name_collision smethods;
    check_tparams_shadow class_tparam_names smethods;
    { N.c_annotation            = ();
      N.c_span                  = c.Aast.c_span;
      N.c_mode                  = c.Aast.c_mode;
      N.c_final                 = c.Aast.c_final;
      N.c_is_xhp                = c.Aast.c_is_xhp;
      N.c_kind                  = c.Aast.c_kind;
      N.c_name                  = name;
      N.c_tparams               = c_tparams;
      N.c_extends               = parents;
      N.c_uses                  = uses;
      N.c_method_redeclarations = redeclarations;
      N.c_xhp_attr_uses         = xhp_attr_uses;
      N.c_xhp_category          = c.Aast.c_xhp_category;
      N.c_req_extends           = req_extends;
      N.c_req_implements        = req_implements;
      N.c_implements            = implements;
      N.c_consts                = consts;
      N.c_typeconsts            = typeconsts;
      N.c_static_vars           = sprops;
      N.c_vars                  = props;
      N.c_constructor           = constructor;
      N.c_static_methods        = smethods;
      N.c_methods               = methods;
      N.c_user_attributes       = attrs;
      N.c_file_attributes       = file_attributes;
      N.c_namespace             = c.Aast.c_namespace;
      N.c_enum                  = enum;
      N.c_doc_comment           = c.Aast.c_doc_comment;
      N.c_pu_enums              = pu_enums;
      (* Naming and typechecking shouldn't use these fields *)
      N.c_attributes            = [];
      N.c_xhp_children          = [];
      N.c_xhp_attrs             = [];
    }

  and aast_user_attributes env attrl =
    let seen = Caml.Hashtbl.create 0 in
    let validate_seen ua_name =
      let pos, name = ua_name in
      let existing_attr_pos =
        try Some (Caml.Hashtbl.find seen name)
        with Caml.Not_found -> None in
      match existing_attr_pos with
      | Some p -> Errors.duplicate_user_attribute ua_name p; false
      | None -> Caml.Hashtbl.add seen name pos; true in
    let on_attr acc { Aast.ua_name; ua_params } =
      let name = snd ua_name in
      let ua_name =
        if String.is_prefix name ~prefix:"__"
        then ua_name
        else Env.type_name env ua_name ~allow_typedef:false ~allow_generics:false in
      if not (validate_seen ua_name)
      then acc
      else
        let attr =
          { N.ua_name = ua_name
          ; N.ua_params = List.map ~f:(aast_expr env) ua_params
          } in
        attr :: acc in
    List.fold_left ~init:[] ~f:on_attr attrl

  and aast_file_attributes mode fal =
    List.map ~f:(aast_file_attribute mode) fal
  and aast_file_attribute mode fa =
    let env = Env.make_file_attributes_env mode fa.Aast.fa_namespace in
    let ua = aast_user_attributes env fa.Aast.fa_user_attributes in
    N.
    { fa_user_attributes = ua
    ; fa_namespace = fa.Aast.fa_namespace
    }

  (* h cv is_required maybe_enum *)
  and aast_xhp_attribute_decl env (h, cv, is_required, maybe_enum) =
    let p, id = cv.Aast.cv_id in
    let default = cv.Aast.cv_expr in
    if is_required && Option.is_some default
    then Errors.xhp_required_with_default p id;
    let hint =
      match maybe_enum with
      | Some (pos, _optional, items) ->
        let is_int item =
          match item with
          | _, Aast.Int _ -> true
          | _ -> false in
        let contains_int = List.exists ~f:is_int items in
        let is_string item =
          match item with
          | _, Aast.String _
          | _, Aast.String2 _ -> true
          | _ -> false in
        let contains_str = List.exists ~f:is_string items in
        if contains_int && not contains_str
        then Some (pos, Aast.Happly ((pos, "int"), []))
        else if not contains_int && contains_str
        then Some (pos, Aast.Happly ((pos, "string"), []))
        else Some (pos, Aast.Happly ((pos, "mixed"), []))
      | _ -> h in
    let hint =
      match hint with
      | Some (p, Aast.Hoption _) ->
        if is_required
        then Errors.xhp_optional_required_attr p id;
        hint
      | Some (_, (Aast.Happly ((_, "mixed"), []))) -> hint
      | Some (p, h) ->
        let has_default =
          match default with
          | None
          | Some (_, Aast.Null) -> false
          | _ -> true in
        if is_required || has_default
        then hint
        else Some (p, Aast.Hoption (p, h))
      | None -> None in
    let hint = Option.map hint (aast_hint env) in
    let expr, is_xhp = aast_class_prop_expr_is_xhp env cv in
    { N.cv_final = cv.Aast.cv_final
    ; N.cv_is_xhp = is_xhp
    ; N.cv_visibility = cv.Aast.cv_visibility
    ; N.cv_type = hint
    ; N.cv_id = cv.Aast.cv_id
    ; N.cv_expr = expr
    ; N.cv_user_attributes = []
    ; N.cv_is_promoted_variadic = cv.Aast.cv_is_promoted_variadic
    ; N.cv_doc_comment = cv.Aast.cv_doc_comment (* Can make None to save space *)
    }

  and aast_enum_ env e =
    { N.e_base = aast_hint env e.Aast.e_base
    ; N.e_constraint = Option.map e.Aast.e_constraint (aast_hint env)
    }

  and aast_type_paraml ?(forbid_this = false) env tparams =
    let _, ret = List.fold_left tparams ~init:(SMap.empty, [])
      ~f:(fun (seen, tparaml) tparam ->
        let (p, name) = tparam.Aast.tp_name in
        match SMap.get name seen with
        | None ->
          SMap.add name p seen, (aast_type_param ~forbid_this env tparam) :: tparaml
        | Some pos ->
          Errors.shadowed_type_param p pos name;
          seen, tparaml
      )
    in
    List.rev ret

  and aast_type_param ~forbid_this env t =
    let (genv, _) = env in
    begin match t.Aast.tp_reified with
    | Aast.Erased -> ()
    | Aast.SoftReified
    | Aast.Reified ->
      if not (TypecheckerOptions.experimental_feature_enabled genv.tcopt
        TypecheckerOptions.experimental_reified_generics)
      then
        Errors.experimental_feature (fst t.Aast.tp_name) "reified generics" end;

    begin
      if (TypecheckerOptions.experimental_feature_enabled
        (fst env).tcopt
        TypecheckerOptions.experimental_type_param_shadowing)
      then
        (* Treat type params as inline class declarations that don't go into the naming heap *)
        let (pos, name) = NS.elaborate_id genv.namespace NS.ElaborateClass t.Aast.tp_name in
        match Naming_table.Types.get_pos name with
        | Some (def_pos, _) ->
          let def_pos, _ = GEnv.get_full_pos (def_pos, name) in
          Errors.error_name_already_bound name name pos def_pos
        | None ->
          match GEnv.type_canon_name name with
          | Some canonical ->
            let def_pos = Option.value ~default:Pos.none (GEnv.type_pos canonical) in
            Errors.error_name_already_bound name canonical pos def_pos
          | None -> ()
    end;

    {
      N.tp_variance = t.Aast.tp_variance;
      tp_name = t.Aast.tp_name;
      tp_constraints = List.map t.Aast.tp_constraints (aast_constraint_ ~forbid_this env);
      tp_reified = t.Aast.tp_reified;
      tp_user_attributes = aast_user_attributes env t.Aast.tp_user_attributes;
    }

  and aast_type_where_constraints env locl_cstrl =
    List.map
      ~f:(fun (h1, ck, h2) ->
        let ty1 = aast_hint ~in_where_clause:true env h1 in
        let ty2 = aast_hint ~in_where_clause:true env h2 in
        (ty1, ck, ty2))
      locl_cstrl

  and aast_class_prop_expr_is_xhp env cv =
    let expr = Option.map cv.Aast.cv_expr (aast_expr env) in
    let expr =
      if (fst env).in_mode = FileInfo.Mdecl && expr = None
      then Some (fst cv.Aast.cv_id, N.Any)
      else expr in
    let is_xhp =
      try ((String.sub (snd cv.Aast.cv_id) 0 1) = ":")
      with Invalid_argument _ -> false in
    expr, is_xhp

  and aast_class_prop_static env cv =
    let attrs = aast_user_attributes env cv.Aast.cv_user_attributes in
    let lsb = Attributes.mem SN.UserAttributes.uaLSB attrs in
    let forbid_this = not lsb in
    let h = Option.map cv.Aast.cv_type (aast_hint ~forbid_this env) in
    let expr, is_xhp = aast_class_prop_expr_is_xhp env cv in
    { N.cv_final = cv.Aast.cv_final
    ; N.cv_is_xhp = is_xhp
    ; N.cv_visibility = cv.Aast.cv_visibility
    ; N.cv_type = h
    ; N.cv_id = cv.Aast.cv_id
    ; N.cv_expr = expr
    ; N.cv_user_attributes = attrs
    ; N.cv_is_promoted_variadic = cv.Aast.cv_is_promoted_variadic
    ; N.cv_doc_comment = cv.Aast.cv_doc_comment (* Can make None to save space *)
    }

  and aast_class_prop_non_static env ?(const = None) cv =
    let h = Option.map cv.Aast.cv_type (aast_hint env) in
    let attrs = aast_user_attributes env cv.Aast.cv_user_attributes in
    let lsb_pos = Attributes.mem_pos SN.UserAttributes.uaLSB attrs in
    (* Non-static properties cannot have attribute __LSB *)
    let _ =
      match lsb_pos with
      | Some p -> Errors.nonstatic_property_with_lsb p
      | None -> () in
    (* if class is __Const, make all member fields __Const *)
    let attrs =
      match const with
      | Some c ->
        if not (Attributes.mem SN.UserAttributes.uaConst attrs)
        then c :: attrs
        else attrs
      | None -> attrs in
    let expr, is_xhp = aast_class_prop_expr_is_xhp env cv in
    { N.cv_final = cv.Aast.cv_final
    ; N.cv_is_xhp = is_xhp
    ; N.cv_visibility = cv.Aast.cv_visibility
    ; N.cv_type = h
    ; N.cv_id = cv.Aast.cv_id
    ; N.cv_expr = expr
    ; N.cv_user_attributes = attrs
    ; N.cv_is_promoted_variadic = cv.Aast.cv_is_promoted_variadic
    ; N.cv_doc_comment = cv.Aast.cv_doc_comment (* Can make None to save space *)
    }

  and aast_class_method env c_meth =
    match c_meth.Aast.m_name, c_meth.Aast.m_params with
    | ((m_pos, m_name), _ :: _) when m_name = SN.Members.__clone ->
      Errors.clone_too_many_arguments m_pos; c_meth
    | _ -> aast_method_ (fst env) c_meth

  and aast_check_constant_expr env (pos, e) =
    match e with
    | Aast.Unsafe_expr _
    | Aast.Id _
    | Aast.Null
    | Aast.True
    | Aast.False
    | Aast.Int _
    | Aast.Float _
    | Aast.String _ -> ()
    | Aast.Class_const ((_, Aast.CIexpr (_, cls)), _)
      when (match cls with Aast.Id (_, "static") -> false | _ -> true) -> ()
    | Aast.Unop ((Ast.Uplus| Ast.Uminus | Ast.Utild | Ast.Unot), e) ->
      aast_check_constant_expr env e
    | Aast.Binop (op, e1, e2) ->
      (* Only assignment is invalid *)
      begin
        match op with
        | Ast.Eq _ -> Errors.illegal_constant pos
        | _ ->
          aast_check_constant_expr env e1;
          aast_check_constant_expr env e2
      end
    | Aast.Eif (e1, e2, e3) ->
      aast_check_constant_expr env e1;
      Option.iter e2 (aast_check_constant_expr env);
      aast_check_constant_expr env e3
    | Aast.Array l -> List.iter l ~f:(aast_check_afield_constant_expr env)
    | Aast.Darray (_, l) ->
      List.iter l ~f:(fun (e1, e2) ->
        aast_check_constant_expr env e1;
        aast_check_constant_expr env e2)
    | Aast.Varray (_, l) -> List.iter l ~f:(aast_check_constant_expr env)
    | Aast.Shape fdl ->
        (* Only check the values because shape field names are always legal *)
        List.iter fdl ~f:(fun (_, e) -> aast_check_constant_expr env e)
    | Aast.Call (_, (_, Aast.Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.tuple ->
        (* Tuples are not really function calls, they are just parsed that way*)
        arg_unpack_unexpected uel;
        List.iter el ~f:(aast_check_constant_expr env)
    | Aast.Collection (id, _, l) ->
      let p, cn = NS.elaborate_id ((fst env).namespace) NS.ElaborateClass id in
      (* Only vec/keyset/dict are allowed because they are value types *)
      if cn = SN.Collections.cVec
      || cn = SN.Collections.cKeyset
      || cn = SN.Collections.cDict
      then List.iter l ~f:(aast_check_afield_constant_expr env)
      else Errors.illegal_constant p
    | _ -> Errors.illegal_constant pos

  and aast_check_afield_constant_expr env afield =
    match afield with
    | Aast.AFvalue e -> aast_check_constant_expr env e
    | Aast.AFkvalue (e1, e2) ->
      aast_check_constant_expr env e1;
      aast_check_constant_expr env e2

  and aast_constant_expr env e =
    let valid_constant_expression =
      Errors.try_with_error
        (fun () -> aast_check_constant_expr env e; true)
        (fun () -> false) in
    if valid_constant_expression
    then aast_expr env e
    else fst e, N.Any

  and aast_class_const env (h, x, eo) =
    Env.bind_class_const env x;
    let h = Option.map h (aast_hint env) in
    let eo = Option.map eo (aast_constant_expr env) in
    h, x, eo

  and aast_typeconst env t =
    (* We use the same namespace as constants within the class so we cannot have
     * a const and type const with the same name
     *)
    Env.bind_class_const env t.Aast.c_tconst_name;
    let constr = Option.map t.Aast.c_tconst_constraint (aast_hint env) in
    let type_ = Option.map t.Aast.c_tconst_type (aast_hint env) in
    let attrs = aast_user_attributes env t.Aast.c_tconst_user_attributes in
    if not (TypecheckerOptions.experimental_feature_enabled (fst env).tcopt
        TypecheckerOptions.experimental_type_const_attributes || List.is_empty attrs)
    then Errors.experimental_feature (fst t.Aast.c_tconst_name) "type constant attributes";
    begin match t.Aast.c_tconst_abstract with
    | Aast.TCAbstract (Some _) when not (TypecheckerOptions.experimental_feature_enabled (fst env).tcopt
      TypecheckerOptions.experimental_abstract_type_const_with_default) ->
      Errors.experimental_feature (fst t.Aast.c_tconst_name) "abstract type constant with default"
    | _ -> () end;
    N.
    { c_tconst_abstract = t.Aast.c_tconst_abstract
    ; c_tconst_name = t.Aast.c_tconst_name
    ; c_tconst_constraint = constr
    ; c_tconst_type = type_
    ; c_tconst_user_attributes = attrs
    }

  and func_body_had_unsafe env = Env.has_unsafe env

  and aast_method_ genv m =
    let genv = aast_extend_params genv m.Aast.m_tparams in
    let env = genv, Env.empty_local None in
    (* Cannot use 'this' if it is a public instance method *)
    let variadicity, paraml = aast_fun_paraml env m.Aast.m_params in
    let tparam_l = aast_type_paraml env m.Aast.m_tparams in
    List.iter tparam_l aast_check_constraint;
    let where_constraints = aast_type_where_constraints env m.Aast.m_where_constraints in
    let ret = Option.map m.Aast.m_ret (aast_hint ~allow_retonly:true env) in
    let body =
      match genv.in_mode with
      | FileInfo.Mdecl | FileInfo.Mphp ->
        { N.fb_ast = [];
          fb_annotation = N.BodyNamingAnnotation.NamedWithUnsafeBlocks;
        }
      | FileInfo.Mstrict | FileInfo.Mpartial | FileInfo.Mexperimental ->
        if Aast.is_body_named m.Aast.m_body
        then
          { N.fb_ast = m.Aast.m_body.Aast.fb_ast;
            fb_annotation = N.BodyNamingAnnotation.Unnamed genv.namespace;
          }
        else failwith "ast_to_nast error unnamedbody in method_"
    in
    let attrs = aast_user_attributes env m.Aast.m_user_attributes in
    { N.m_annotation = ();
      N.m_span = m.Aast.m_span;
      N.m_final = m.Aast.m_final;
      N.m_visibility = m.Aast.m_visibility;
      N.m_abstract = m.Aast.m_abstract;
      N.m_static = m.Aast.m_static;
      N.m_name = m.Aast.m_name;
      N.m_tparams = tparam_l;
      N.m_where_constraints = where_constraints;
      N.m_params = paraml;
      N.m_body = body;
      N.m_fun_kind = m.Aast.m_fun_kind;
      N.m_ret = ret;
      N.m_variadic = variadicity;
      N.m_user_attributes = attrs;
      N.m_external = m.Aast.m_external;
      N.m_doc_comment = m.Aast.m_doc_comment;
    }

  and aast_method_redeclaration env mt =
    if not
      (TypecheckerOptions.experimental_feature_enabled
        (fst env).tcopt
        TypecheckerOptions.experimental_trait_method_redeclarations)
    then Errors.experimental_feature (fst mt.Aast.mt_name) "trait method redeclarations";
    let genv = aast_extend_params (fst env) mt.Aast.mt_tparams in
    let env = genv, Env.empty_local None in
    let variadicity, paraml = aast_fun_paraml env mt.Aast.mt_params in
    let tparam_l = aast_type_paraml env mt.Aast.mt_tparams in
    let where_constraints = aast_type_where_constraints env mt.Aast.mt_where_constraints in
    let ret = Option.map mt.Aast.mt_ret (aast_hint ~allow_retonly:true env) in
    { N.mt_final = mt.Aast.mt_final
    ; N.mt_visibility = mt.Aast.mt_visibility
    ; N.mt_abstract = mt.Aast.mt_abstract
    ; N.mt_static = mt.Aast.mt_static
    ; N.mt_name = mt.Aast.mt_name
    ; N.mt_tparams = tparam_l
    ; N.mt_where_constraints = where_constraints
    ; N.mt_params = paraml
    ; N.mt_fun_kind = mt.Aast.mt_fun_kind
    ; N.mt_ret = ret
    ; N.mt_variadic = variadicity
    ; N.mt_trait = aast_hint ~allow_typedef:false env mt.Aast.mt_trait
    ; N.mt_method = mt.Aast.mt_method
    ; N.mt_user_attributes = []
    }

  and aast_fun_paraml env paraml =
    let _ = List.fold_left ~f:aast_check_repetition ~init:SSet.empty paraml in
    let variadicity, paraml = aast_determine_variadicity env paraml in
    variadicity, List.map ~f:(aast_fun_param env) paraml

  (* Variadic params are removed from the list *)
  and aast_determine_variadicity env paraml =
    match paraml with
    | [] -> N.FVnonVariadic, []
    | [x] ->
      begin
        match x.Aast.param_is_variadic, x.Aast.param_name with
        | false, _ -> N.FVnonVariadic, paraml
        | true, "..." -> N.FVellipsis x.Aast.param_pos, []
        | true, _ -> N.FVvariadicArg (aast_fun_param env x), []
      end
    | x :: rl ->
      let variadicity, rl = aast_determine_variadicity env rl in
      variadicity, x :: rl

  and aast_fun_param env (param : Aast.fun_param) =
    let p = param.Aast.param_pos in
    let name = param.Aast.param_name in
    let ident = Local_id.make_unscoped name in
    Env.add_lvar env (p, name) (p, ident);
    let ty = Option.map param.Aast.param_hint (aast_hint env) in
    let eopt = Option.map param.Aast.param_expr (aast_expr env) in
    if param.Aast.param_is_reference && FileInfo.is_strict (fst env).in_mode
    then Errors.reference_in_strict_mode p;
    { N.param_annotation = p;
      param_hint = ty;
      param_is_reference = param.Aast.param_is_reference;
      param_is_variadic = param.Aast.param_is_variadic;
      param_pos = p;
      param_name = name;
      param_expr = eopt;
      param_callconv = param.Aast.param_callconv;
      param_user_attributes = aast_user_attributes env param.Aast.param_user_attributes;
    }

  and aast_make_constraints paraml =
    List.fold_right
      ~init:SMap.empty
      ~f:(fun { Aast.tp_name = (_, x); tp_constraints; tp_reified; _ } acc ->
        SMap.add x (tp_reified, tp_constraints) acc)
      paraml

  and aast_extend_params genv paraml =
    let params = List.fold_right paraml ~init:genv.type_params
      ~f:begin fun {
        Aast.tp_name = (_, x);
        tp_constraints = cstr_list;
        tp_reified = r;
        _
      } acc ->
        SMap.add x (r, cstr_list) acc
      end in
    { genv with type_params = params }

  and fun_ f =
    let f = Ast_to_nast.on_fun f in
    aast_fun_ f

  and aast_fun_ f =
    let tparams = aast_make_constraints f.Aast.f_tparams in
    let genv = Env.aast_make_fun_decl_genv tparams f in
    let lenv = Env.empty_local None in
    let env = genv, lenv in
    let where_constraints = aast_type_where_constraints env f.Aast.f_where_constraints in
    let h = Option.map f.Aast.f_ret (aast_hint ~allow_retonly:true env) in
    let variadicity, paraml = aast_fun_paraml env f.Aast.f_params in
    let x = Env.fun_id env f.Aast.f_name in
    List.iter f.Aast.f_tparams aast_check_constraint;
    let f_tparams = aast_type_paraml env f.Aast.f_tparams in
    let f_kind = f.Aast.f_fun_kind in
    let body =
      match genv.in_mode with
      | FileInfo.Mdecl | FileInfo.Mphp ->
        { N.fb_ast = [];
          fb_annotation = N.BodyNamingAnnotation.NamedWithUnsafeBlocks;
        }
      | FileInfo.Mstrict | FileInfo.Mpartial | FileInfo.Mexperimental ->
        if Aast.is_body_named f.Aast.f_body
        then
          { N.fb_ast = f.Aast.f_body.Aast.fb_ast;
            fb_annotation = N.BodyNamingAnnotation.Unnamed genv.namespace;
          }
        else failwith "ast_to_nast error unnamedbody in fun_"
    in
    let named_fun = {
      N.f_annotation = ();
      f_span = f.Aast.f_span;
      f_mode = f.Aast.f_mode;
      f_ret = h;
      f_name = x;
      f_tparams = f_tparams;
      f_where_constraints = where_constraints;
      f_params = paraml;
      f_body = body;
      f_fun_kind = f_kind;
      f_variadic = variadicity;
      f_user_attributes = aast_user_attributes env f.Aast.f_user_attributes;
      (* Fix file attributes if they are important *)
      f_file_attributes = [];
      f_external = f.Aast.f_external;
      f_namespace = f.Aast.f_namespace;
      f_doc_comment = f.Aast.f_doc_comment;
      f_static = f.Aast.f_static;
    } in
    named_fun

  and aast_get_using_vars (_, e) =
    match e with
    | Aast.Expr_list using_clauses ->
      List.concat_map using_clauses aast_get_using_vars
      (* Simple assignment to local of form `$lvar = e` *)
    | Aast.Binop (Ast.Eq None, (_, Aast.Lvar (p, lid)), _) ->
      [ (p, Local_id.get_name lid) ]
      (* Arbitrary expression. This will be assigned to a temporary *)
    | _ -> []

  and aast_stmt env (pos, st) =
    let stmt = match st with
    | Aast.Let (x, h, e) -> aast_let_stmt env x h e
    | Aast.Block _ -> failwith "aast_stmt block error"
    | Aast.Unsafe_block _ -> failwith "aast_stmt unsafe_block error"
    | Aast.Fallthrough -> N.Fallthrough
    | Aast.Noop -> N.Noop
    | Aast.Markup (_, None) -> N.Noop
    | Aast.Markup (_m, Some e) -> N.Expr (aast_expr env e)
    | Aast.Break -> Aast.Break
    | Aast.Continue -> Aast.Continue
    | Aast.Throw (_, e) ->
      let terminal = not (fst env).in_try in
      N.Throw (terminal, aast_expr env e)
    | Aast.Return e -> N.Return (Option.map e (aast_expr env))
    | Aast.GotoLabel label -> name_goto_label env label
    | Aast.Goto label -> name_goto env label
    | Aast.Awaitall (el, b) -> aast_awaitall_stmt env el b
    | Aast.If (e, b1, b2) -> aast_if_stmt env e b1 b2
    | Aast.Do (b, e) -> aast_do_stmt env b e
    | Aast.While (e, b) ->
      N.While (aast_expr env e, aast_block env b)
    | Aast.Declare (_, (p, _), _) ->
      Errors.declare_statement_in_hack p;
      N.Expr (p, N.Any)
    | Aast.Using s -> aast_using_stmt env s.Aast.us_has_await s.Aast.us_expr s.Aast.us_block
    | Aast.For (st1, e, st2, b) -> aast_for_stmt env st1 e st2 b
    | Aast.Switch (e, cl) -> aast_switch_stmt env e cl
    | Aast.Foreach (e, ae, b) -> aast_foreach_stmt env e ae b
    | Aast.Try (b, cl, fb) -> aast_try_stmt env b cl fb
    | Aast.Def_inline _ -> (* No convenient pos information on Aast *)
      Errors.experimental_feature Pos.none "inlined definitions"; N.Expr (Pos.none, N.Any)
    | Aast.Expr (cp, Aast.Call (_, (p, Aast.Id (fp, fn)), hl, el, uel))
      when fn = SN.SpecialFunctions.invariant ->
        (* invariant is subject to a source-code transform in the HHVM
         * runtime: the arguments to invariant are lazily evaluated only in
         * the case in which the invariant condition does not hold. So:
         *
         *   invariant_violation(<condition>, <format>, <format_args...>)
         *
         * ... is rewritten as:
         *
         *   if (!<condition>) {
         *     invariant_violation(<format>, <format_args...>);
         *   }
         *)
        begin
          match el with
          | [] | [_] ->
            Errors.naming_too_few_arguments p;
            N.Expr (cp, N.Any)
          | (cond_p, cond) :: el ->
            let violation = (cp, Aast.Call (Aast.Cnormal, (p, Aast.Id
              (fp, "\\"^SN.SpecialFunctions.invariant_violation)), hl, el, uel)) in
            if cond <> Aast.False
            then
              let b1, b2 = [cp, Aast.Expr violation], [Pos.none, Aast.Noop] in
              let cond = (cond_p, Aast.Unop (Ast.Unot, (cond_p, cond))) in
              aast_if_stmt env cond b1 b2
            else (* a false <condition> means unconditional invariant_violation *)
              N.Expr (aast_expr env violation)
        end
    | Aast.Expr e -> N.Expr (aast_expr env e)
    in
    pos, stmt

  and aast_let_stmt env (p, lid) h e =
    let name = Local_id.get_name lid in
    let e = aast_expr env e in
    let h = Option.map h (aast_hint env) in
    let x = Env.new_let_local env (p, name) in
    N.Let (x, h, e)

  and aast_if_stmt env e b1 b2 =
    let e = aast_expr env e in
    Env.scope env
      (fun env ->
        let b1 = aast_branch env b1 in
        let b2 = aast_branch env b2 in
        N.If (e, b1, b2)
      )

  and aast_do_stmt env b e =
    (* lexical block of `do` is extended to the expr of loop termination *)
    Env.scope_lexical
      env
      (fun env ->
        let b = aast_block ~new_scope:false env b in
        let e = aast_expr env e in
        N.Do (b, e))

  (* Scoping is essentially that of do: block is always executed *)
  and aast_using_stmt env has_await e b =
    let vars = aast_get_using_vars e in
    let e = aast_expr env e in
    let b = aast_block ~new_scope:false env b in
    Env.remove_locals env vars;
    N.Using N.{
      us_is_block_scoped = false; (* This isn't used for naming so provide a default *)
      us_has_await = has_await;
      us_expr = e;
      us_block = b;
    }

  and aast_for_stmt env e1 e2 e3 b =
    (* The initialization and condition expression should be in the outer scope,
     * as they are always executed. *)
    let e1 = aast_expr env e1 in
    let e2 = aast_expr env e2 in
    Env.scope
      env
      (fun env ->
        (* The third expression (iteration step) should have the same scope as the
         * block, as it is not always executed. *)
        let b = aast_block ~new_scope:false env b in
        let e3 = aast_expr env e3 in
        N.For (e1, e2, e3, b))

  and aast_switch_stmt env e cl =
    let e = aast_expr env e in
    Env.scope env begin fun env ->
      let cl = aast_casel env cl in
      N.Switch (e, cl)
    end

  and aast_foreach_stmt env e ae b =
    let e = aast_expr env e in
    Env.scope
      env
      (fun env ->
        let ae = aast_as_expr env ae in
        let b = aast_block env b in
        N.Foreach (e, ae, b))

  and aast_as_expr env ae =
    let handle_v ev =
      match ev with
      | p, Aast.Id x when (fst env).in_mode = FileInfo.Mexperimental ->
        let x = Env.new_let_local env x in
        let ev = (p, N.ImmutableVar x) in
        ev
      | p, Aast.Id _ ->
        Errors.expected_variable p;
        p, N.Lvar (Env.new_lvar env (p, "__internal_placeholder"))
      | ev ->
        let nsenv = (fst env).namespace in
        let _, vars =
          GetLocals.aast_lvalue (nsenv, SMap.empty) ev in
        SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
        aast_expr env ev in
    let handle_k ek =
      match ek with
      | _, Aast.Lvar (p, lid) ->
        let x = (p, Local_id.get_name lid) in
        p, N.Lvar (Env.new_lvar env x)
      | p, Aast.Id x when (fst env).in_mode = FileInfo.Mexperimental ->
        p, N.ImmutableVar (Env.new_let_local env x)
      | p, _ ->
        Errors.expected_variable p;
        p, N.Lvar (Env.new_lvar env (p, "__internal_placeholder")) in
    match ae with
    | Aast.As_v ev ->
      let ev = handle_v ev in
      N.As_v ev
    | Aast.As_kv (k, ev) ->
      let k = handle_k k in
      let ev = handle_v ev in
      N.As_kv (k, ev)
    | N.Await_as_v (p, ev) ->
      let ev = handle_v ev in
      N.Await_as_v (p, ev)
    | N.Await_as_kv (p, k, ev) ->
      let k = handle_k k in
      let ev = handle_v ev in
      N.Await_as_kv (p, k, ev)

  and aast_try_stmt env b cl fb =
    Env.scope
      env
      (fun env ->
        let genv, lenv = env in
        (* isolate finally from the rest of the try-catch: if the first
         * statement of the try is an uncaught exception, finally will
         * still be executed *)
        let fb = aast_branch ({genv with in_finally = true}, lenv) fb in
        let b = aast_branch ({genv with in_try = true}, lenv) b in
        let cl = aast_catchl env cl in
        N.Try (b, cl, fb))

  and aast_stmt_list ?after_unsafe stl env =
    let aast_stmt_list = aast_stmt_list ?after_unsafe in
    match stl with
    | [] -> []
    | (p, Aast.Unsafe_block b) :: _ ->
      Env.set_unsafe env true;
      let st = Errors.ignore_ (fun () -> p, N.Unsafe_block (aast_stmt_list b env)) in
      st :: Option.to_list after_unsafe
    | (_, Aast.Block b) :: rest ->
      (* Add lexical scope for block scoped let variables *)
      let b = Env.scope_lexical env (aast_stmt_list b) in
      let rest = aast_stmt_list rest env in
      b @ rest
    | x :: rest ->
      let x = aast_stmt env x in
      let rest = aast_stmt_list rest env in
      x :: rest

  and aast_block ?(new_scope=true) env stl =
    if new_scope
    then Env.scope env (aast_stmt_list stl)
    else aast_stmt_list stl env

  and aast_branch ?after_unsafe env stmt_l =
    Env.scope env (aast_stmt_list ?after_unsafe stmt_l)

  (**
   * Names a goto label.
   *
   * The goto label is added to the local labels if it is not already there.
   * Otherwise, an error is produced.
   *
   * An error is produced if this is called within a finally block.
   *)
  and name_goto_label
      ({ in_finally; _ }, _ as env) (label_pos, label_name as label) =
    (match Env.goto_label env label_name with
      | Some original_declaration_pos ->
        Errors.goto_label_already_defined
          label_name
          label_pos
          original_declaration_pos
      | None -> Env.new_goto_label env label);
    if in_finally then
      Errors.goto_label_defined_in_finally label_pos;
    N.GotoLabel label

  (**
   * Names a goto target.
   *
   * The goto statement's target label is added to the local goto targets.
   *
   * An error is produced if this is called within a finally block.
   *)
  and name_goto
      ({ in_finally; _ }, _ as env) (label_pos, _ as label) =
    Env.new_goto_target env label;
    if in_finally then Errors.goto_invoked_in_finally label_pos;
    N.Goto label

  and aast_awaitall_stmt env el b =
    let el =
      List.map
        ~f:(fun (e1, e2) ->
          let e2 = aast_expr env e2 in
          let e1 =
            match e1 with
            | Some lid ->
              let e = Pos.none, Aast.Lvar lid in
              let nsenv = (fst env).namespace in
              let _, vars =
                GetLocals.aast_lvalue (nsenv, SMap.empty) e in
              SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
              e1
            | None -> None in
          (e1, e2))
      el in
    let s = aast_block env b in
    N.Awaitall (el, s)

  and aast_expr_obj_get_name env expr =
    match expr with
    | p, Aast.Id x -> p, N.Id x
    | p, e -> aast_expr env (p, e)

  and aast_exprl env l = List.map ~f:(aast_expr env) l

  and aast_oexpr env e = Option.map e (aast_expr env)

  and aast_expr env (p, e) = p, aast_expr_ env p e

  and aast_expr_ env p (e : Aast.expr_) =
    match e with
    | Aast.ParenthesizedExpr (p, e) -> aast_expr_ env p e
    | Aast.Array l ->
      let tcopt = (fst env).tcopt in
      if TypecheckerOptions.disallow_array_literal tcopt
      then Errors.array_literals_disallowed p;
      N.Array (List.map l (aast_afield env))
    | Aast.Varray (ta, l) ->
        N.Varray (Option.map ~f:(aast_targ env) ta, List.map l (aast_expr env))
    | Aast.Darray (tap, l) ->
      let nargs = Option.map ~f:(fun (t1, t2) -> aast_targ env t1, aast_targ env t2) tap in
      N.Darray (
        nargs,
        List.map l (fun (e1, e2) -> aast_expr env e1, aast_expr env e2))
    | Aast.Collection (id, tal, l) ->
      let p, cn = NS.elaborate_id ((fst env).namespace) NS.ElaborateClass id in
      begin
        match cn with
        | x when N.is_vc_kind x ->
          let ta = begin match tal with
          | Some Aast.CollectionTV tv -> Some (aast_targ env tv)
          | Some Aast.CollectionTKV _ -> Errors.naming_too_many_arguments p; None
          | None -> None
          end in
          N.ValCollection ((N.get_vc_kind cn), ta, (List.map l (aast_afield_value env cn)))
        | x when N.is_kvc_kind x ->
          let ta = begin match tal with
          | Some Aast.CollectionTV _ -> Errors.naming_too_few_arguments p; None
          | Some Aast.CollectionTKV (tk, tv) -> Some (aast_targ env tk, aast_targ env tv)
          | None -> None
          end in
          N.KeyValCollection ((N.get_kvc_kind cn), ta,
            (List.map l (aast_afield_kvalue env cn)))
        | x when x = SN.Collections.cPair ->
          begin
            match l with
            | [] ->
              Errors.naming_too_few_arguments p;
              N.Any
            | e1::e2::[] ->
              let pn = SN.Collections.cPair in
              N.Pair (aast_afield_value env pn e1, aast_afield_value env pn e2)
            | _ ->
              Errors.naming_too_many_arguments p;
              N.Any
          end
        | _ ->
            Errors.expected_collection p cn;
            N.Any
      end
    | Aast.Clone e -> N.Clone (aast_expr env e)
    | Aast.Null -> N.Null
    | Aast.True -> N.True
    | Aast.False -> N.False
    | Aast.Int s -> N.Int s
    | Aast.Float s -> N.Float s
    | Aast.String s -> N.String s
    | Aast.String2 idl -> N.String2 (aast_string2 env idl)
    | Aast.PrefixedString (n, e) -> N.PrefixedString (n, (aast_expr env e))
    | Aast.Id x ->
      (** TODO: Emit proper error messages T28473207. Currently the error message
        * emitted has reason Naming[2049] unbound name for global constant *)
      begin
        match Env.let_local env x with
        | Some x -> N.ImmutableVar x
        | None -> N.Id (Env.global_const env x)
      end (* match *)
    | Aast.Lvar (_, x) when Local_id.to_string x = SN.SpecialIdents.this -> N.This
    | Aast.Lvar (p, x) when Local_id.to_string x = SN.SpecialIdents.dollardollar ->
      N.Dollardollar (p, Env.found_dollardollar env p)
    | Aast.Lvar (p, x) when Local_id.to_string x = SN.SpecialIdents.placeholder ->
      N.Lplaceholder p
    | Aast.Lvar x ->
        N.Lvar (Env.aast_lvar env x)
    | Aast.PU_atom x -> N.PU_atom x
    | Aast.Obj_get (e1, e2, nullsafe) ->
      (* If we encounter Obj_get(_,_,true) by itself, then it means "?->"
         is being used for instance property access; see the case below for
         handling nullsafe instance method calls to see how this works *)
      N.Obj_get (aast_expr env e1, aast_expr_obj_get_name env e2, nullsafe)
    | Aast.Array_get ((p, Aast.Lvar x), None) ->
        let id = p, N.Lvar (Env.aast_lvar env x) in
        N.Array_get (id, None)
    | Aast.Array_get (e1, e2) -> N.Array_get (aast_expr env e1, aast_oexpr env e2)
    | Aast.Class_get ((_, Aast.CIexpr (_, Aast.Id x1)), Aast.CGstring x2) ->
      N.Class_get (make_class_id env x1, N.CGstring x2)
    | Aast.Class_get ((_, Aast.CIexpr (_, Aast.Lvar (p, lid))), Aast.CGstring x2) ->
      let x1 = (p, Local_id.to_string lid) in
      N.Class_get (make_class_id env x1, N.CGstring x2)
    | Aast.Class_get ((_, Aast.CIexpr x1), Aast.CGexpr x2) ->
      aast_ensure_name_not_dynamic env x1
        Errors.dynamic_class_name_in_strict_mode;
      aast_ensure_name_not_dynamic env x2
        Errors.dynamic_class_name_in_strict_mode;
      N.Any
    | Aast.Class_get _ -> failwith "Error in Ast_to_nast module for Class_get"
    | Aast.Class_const ((_, Aast.CIexpr (_, Aast.Id x1)), x2) ->
      let (genv, _) = env in
      let (_, name) = NS.elaborate_id genv.namespace NS.ElaborateClass x1 in
      begin
        match Naming_table.Types.get_pos name with
        | Some (_, Naming_table.TTypedef) when (snd x2) = "class" ->
          N.Typename (Env.type_name env x1 ~allow_typedef:true ~allow_generics:false)
        | _ ->
          N.Class_const (make_class_id env x1, x2)
      end
    | Aast.Class_const ((_, Aast.CIexpr (_, Aast.Lvar (p, lid))), x2) ->
      let x1 = (p, Local_id.to_string lid) in
      let (genv, _) = env in
      let (_, name) = NS.elaborate_id genv.namespace NS.ElaborateClass x1 in
      begin
        match Naming_table.Types.get_pos name with
        | Some (_, Naming_table.TTypedef) when (snd x2) = "class" ->
          N.Typename (Env.type_name env x1 ~allow_typedef:true ~allow_generics:false)
        | _ ->
          N.Class_const (make_class_id env x1, x2)
      end
    | Aast.Class_const _ -> (* TODO: report error in strict mode *) N.Any
    | Aast.PU_identifier ((_, c), s1, s2) -> begin
        match c with
        | Aast.CIexpr (_, Aast.Id x1) ->
          N.PU_identifier (make_class_id env x1, s1, s2)
        | _ ->
          failwith "TODO(T35357243): Error during parsing of PU_identifier"
      end
    | Aast.Call (_, (_, Aast.Id (p, pseudo_func)), tal, el, uel)
      when pseudo_func = SN.SpecialFunctions.echo ->
        arg_unpack_unexpected uel;
        N.Call (N.Cnormal, (p, N.Id (p, pseudo_func)), aast_targl env p tal, aast_exprl env el, [])
    | Aast.Call (_, (p, (Aast.Id (_, cn))), tal, el, uel)
      when cn = SN.SpecialFunctions.call_user_func ->
        arg_unpack_unexpected uel;
        begin
          match el with
          | [] -> Errors.naming_too_few_arguments p; N.Any
          | f :: el ->
            N.Call
              (N.Cuser_func, aast_expr env f, aast_targl env p tal, aast_exprl env el, [])
        end
    | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.fun_ ->
        arg_unpack_unexpected uel;
        let (genv, _) = env in
        begin
          match el with
          | [] -> Errors.naming_too_few_arguments p; N.Any
          | [_, Aast.String s] when String.contains s ':' ->
            Errors.illegal_meth_fun p; N.Any
          | [_, Aast.String s] when genv.in_ppl && SN.PPLFunctions.is_reserved s ->
            Errors.ppl_meth_pointer p ("fun("^s^")"); N.Any
          | [p, Aast.String x] ->
            (* Functions referenced by fun() are always fully-qualified *)
            let x = if x <> "" && x.[0] <> '\\' then "\\" ^ x else x in
            N.Fun_id (Env.fun_id env (p, x))
          | [p, _] -> Errors.illegal_fun p; N.Any
          | _ -> Errors.naming_too_many_arguments p; N.Any
        end
    | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.inst_meth ->
        arg_unpack_unexpected uel;
        begin
          match el with
          | []
          | [_] -> Errors.naming_too_few_arguments p; N.Any
          | instance :: (p, Aast.String meth) :: [] ->
            N.Method_id (aast_expr env instance, (p, meth))
          | (p, _) :: _ :: [] -> Errors.illegal_inst_meth p; N.Any
          | _ -> Errors.naming_too_many_arguments p; N.Any
        end
    | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.meth_caller ->
        arg_unpack_unexpected uel;
        begin
          match el with
          | []
          | [_] -> Errors.naming_too_few_arguments p; N.Any
          | e1 :: e2 :: [] ->
            begin
              match (aast_expr env e1), (aast_expr env e2) with
              | (pc, N.String cl), (pm, N.String meth) ->
                N.Method_caller (Env.type_name env (pc, cl) ~allow_typedef:false ~allow_generics:false, (pm, meth))
              | (_, N.Class_const ((_, N.CI cl), (_, mem))), (pm, N.String meth)
                when mem = SN.Members.mClass ->
                  N.Method_caller (Env.type_name env cl ~allow_typedef:false ~allow_generics:false, (pm, meth))
              | (p, _), _ -> Errors.illegal_meth_caller p; N.Any
            end
          | _ -> Errors.naming_too_many_arguments p; N.Any
        end
    | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.class_meth ->
        arg_unpack_unexpected uel;
        begin
          match el with
          | []
          | [_] -> Errors.naming_too_few_arguments p; N.Any
          | e1 :: e2 :: [] ->
            begin
              match (aast_expr env e1), (aast_expr env e2) with
              | (pc, N.String cl), (pm, N.String meth) ->
                N.Smethod_id (Env.type_name env (pc, cl) ~allow_typedef:false ~allow_generics:false, (pm, meth))
              | (_, N.Id (_, const)), (pm, N.String meth)
                when const = SN.PseudoConsts.g__CLASS__ ->
                  (* All of these that use current_cls aren't quite correct
                   * inside a trait, as the class should be the using class.
                   * It's sufficient for typechecking purposes (we require
                   * subclass to be compatible with the trait member/method
                   * declarations).
                   *)
                  (match (fst env).current_cls with
                  | Some (cid, _) -> N.Smethod_id (cid, (pm, meth))
                  | None -> Errors.illegal_class_meth p; N.Any)
              | (_, N.Class_const ((_, N.CI cl), (_, mem))), (pm, N.String meth)
                when mem = SN.Members.mClass ->
                N.Smethod_id (Env.type_name env cl ~allow_typedef:false ~allow_generics:false, (pm, meth))
              | (p, N.Class_const ((_, (N.CIself | N.CIstatic)), (_, mem))),
                (pm, N.String meth) when mem = SN.Members.mClass ->
                  (match (fst env).current_cls with
                  | Some (cid, _) -> N.Smethod_id (cid, (pm, meth))
                  | None -> Errors.illegal_class_meth p; N.Any)
              | (p, _), _ -> Errors.illegal_class_meth p; N.Any
            end
          | _ -> Errors.naming_too_many_arguments p; N.Any
        end
    | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.assert_ ->
        arg_unpack_unexpected uel;
        if List.length el <> 1
        then Errors.assert_arity p;
        N.Assert (N.AE_assert (
          Option.value_map (List.hd el) ~default:(p, N.Any) ~f:(aast_expr env)
        ))
    | Aast.Call (_, (p, Aast.Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.tuple ->
        arg_unpack_unexpected uel;
        (match el with
        | [] -> Errors.naming_too_few_arguments p; N.Any
        | el -> N.List (aast_exprl env el)
        )
    (* sample, factor, observe, condition *)
    | Aast.Call (_, (p1, Aast.Id (p2, cn)), tal, el, uel)
      when Env.in_ppl env && SN.PPLFunctions.is_reserved cn ->
        let n_expr = N.Id (p2, cn) in
        N.Call (N.Cnormal, (p1, n_expr),
          aast_targl env p tal, aast_exprl env el, aast_exprl env uel)
    | Aast.Call (_, (p, Aast.Id f), tal, el, uel) ->
      begin
        match Env.let_local env f with
        | Some x ->
          (* Translate into local id *)
          let f = (p, N.ImmutableVar x) in
          N.Call (N.Cnormal, f, aast_targl env p tal, aast_exprl env el, aast_exprl env uel)
        | None ->
          (* The name is not a local `let` binding *)
          let qualified = Env.fun_id env f in
          let cn = snd qualified in
          (* The above special cases (fun, inst_meth, meth_caller, class_meth,
           * and friends) are magical language constructs, which we should
           * check before calling fun_id and looking up the function and doing
           * namespace normalization. However, gena, genva, etc are actual
           * functions that actually exist, we just need to handle them
           * specially here, during naming. Note that most of the function
           * special cases, such as idx, are actually handled in typing, and
           * don't require naming magic. *)
          if cn = SN.FB.fgena
          then
            begin
              arg_unpack_unexpected uel;
              match el with
              | [e] -> N.Special_func (N.Gena (aast_expr env e))
              | _ -> Errors.gena_arity p; N.Any
            end
          else if (cn = SN.FB.fgenva)
                   || (cn = SN.HH.asio_va)
                   || (cn = SN.HH.lib_tuple_gen)
                   || (cn = SN.HH.lib_tuple_from_async)
          then
            begin
              arg_unpack_unexpected uel;
              if List.length el < 1
              then (Errors.genva_arity p; N.Any)
              else N.Special_func (N.Genva (aast_exprl env el))
            end
          else if cn = SN.FB.fgen_array_rec
          then
            begin
              arg_unpack_unexpected uel;
              match el with
              | [e] -> N.Special_func (N.Gen_array_rec (aast_expr env e))
              | _ -> Errors.gen_array_rec_arity p; N.Any
            end
          else
            N.Call (N.Cnormal, (p, N.Id qualified), aast_targl env p tal,
                    aast_exprl env el, aast_exprl env uel)
      end (* match *)
    (* Handle nullsafe instance method calls here. Because Obj_get is used
       for both instance property access and instance method calls, we need
       to match the entire "Call(Obj_get(..), ..)" pattern here so that we
       only match instance method calls *)
    | Aast.Call (_, (p, Aast.Obj_get (e1, e2, Aast.OG_nullsafe)), tal, el, uel) ->
      N.Call
        (N.Cnormal,
         (p, N.Obj_get (aast_expr env e1,
            aast_expr_obj_get_name env e2, N.OG_nullsafe)),
         aast_targl env p tal,
         aast_exprl env el, aast_exprl env uel)
    (* Handle all kinds of calls that weren't handled by any of the cases above *)
    | Aast.Call (_, e, tal, el, uel) ->
      N.Call (N.Cnormal, aast_expr env e,
             aast_targl env p tal, aast_exprl env el, aast_exprl env uel)
    | Aast.Yield_break -> N.Yield_break
    | Aast.Yield e -> N.Yield (aast_afield env e)
    | Aast.Await e -> N.Await (aast_expr env e)
    | Aast.Suspend e -> N.Suspend (aast_expr env e)
    | Aast.List el -> N.List (aast_exprl env el)
    | Aast.Expr_list el -> N.Expr_list (aast_exprl env el)
    | Aast.Cast (ty, e2) ->
      let (p, x), hl =
        match ty with
        | _, Aast.Happly (id, hl) -> (id, hl)
        | _                  -> assert false in
      let ty =
        match aast_try_castable_hint ~tp_depth:1 env p x hl with
        | Some ty -> p, ty
        | None    ->
          begin
            match x with
            | x when x = SN.Typehints.object_cast ->
              (* (object) is a valid cast but not a valid type annotation *)
              if FileInfo.is_strict (fst env).in_mode then Errors.object_cast p None;
              p, N.Hany
            | x when x = SN.Typehints.void ->
              Errors.void_cast p;
              p, N.Hany
            | x when x = SN.Typehints.unset_cast ->
              Errors.unset_cast p;
              p, N.Hany
            | _       ->
              (* Let's just assume that any other invalid cases are attempts to
              * cast to specific objects *)
              let h = aast_hint ~allow_typedef:false env ty in
              Errors.object_cast p (Some x);
              h
          end in
      N.Cast (ty, aast_expr env e2)
    | Aast.Unop (uop, e) -> N.Unop (uop, aast_expr env e)
    | Aast.Binop (Ast.Eq None as op, lv, e2) ->
      if Env.inside_pipe env then
        Errors.unimplemented_feature p "Assignment within pipe expressions";
      let e2 = aast_expr env e2 in
      let nsenv = (fst env).namespace in
      let _, vars =
        GetLocals.aast_lvalue (nsenv, SMap.empty) lv in
      SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
      N.Binop (op, aast_expr env lv, e2)
    | Aast.Binop (Ast.Eq _ as bop, e1, e2) ->
      if Env.inside_pipe env
      then Errors.unimplemented_feature p "Assignment within pipe expressions";
      N.Binop (bop, aast_expr env e1, aast_expr env e2)
    | Aast.Binop (bop, e1, e2) -> N.Binop (bop, aast_expr env e1, aast_expr env e2)
    | Aast.Pipe (_dollardollar, e1, e2) ->
      let e1 = aast_expr env e1 in
      let ident, e2 = Env.pipe_scope env (fun env -> aast_expr env e2) in
      N.Pipe ((p, ident), e1, e2)
    | Aast.Eif (e1, e2opt, e3) ->
      (* The order matters here, of course -- e1 can define vars that need to
       * be available in e2 and e3. *)
      let e1 = aast_expr env e1 in
      let e2opt, e3 = Env.scope env (fun env ->
        let e2opt = Env.scope env (fun env -> aast_oexpr env e2opt) in
        let e3 = Env.scope env (fun env -> aast_expr env e3) in
        e2opt, e3
      ) in
      N.Eif (e1, e2opt, e3)
    | Aast.InstanceOf (e, (_, Aast.CIexpr (p, Aast.Id x))) ->
      let id =
        match x with
        | px, n when n = SN.Classes.cParent ->
          if (fst env).current_cls = None then
            let () = Errors.parent_outside_class p in
            N.CI (px, SN.Classes.cUnknown)
          else N.CIparent
        | px, n when n = SN.Classes.cSelf ->
          if (fst env).current_cls = None then
            let () = Errors.self_outside_class p in
            N.CI (px, SN.Classes.cUnknown)
          else N.CIself
        | px, n when n = SN.Classes.cStatic ->
          if (fst env).current_cls = None then
            let () = Errors.static_outside_class p in
            N.CI (px, SN.Classes.cUnknown)
          else N.CIstatic
        | _ ->
          N.CI (Env.type_name env x ~allow_typedef:false ~allow_generics:false)
      in
      N.InstanceOf (aast_expr env e, (p, id))
    | Aast.InstanceOf (e1, (_, Aast.CIexpr (_,
        (Aast.Lvar _ | Aast.Obj_get _ | Aast.Class_get _ | Aast.Class_const _
        | Aast.Array_get _ | Aast.Call _) as e2))) ->
      N.InstanceOf (aast_expr env e1, (fst e2, N.CIexpr (aast_expr env e2)))
    | Aast.InstanceOf (_e1, (p, _)) ->
      Errors.invalid_instanceof p;
      N.Any
    | Aast.Is (e, h) ->
      N.Is (aast_expr env e, aast_hint ~allow_wildcard:true env h)
    | Aast.As (e, h, b) ->
      N.As (aast_expr env e, aast_hint ~allow_wildcard:true env h, b)
    | Aast.New ((_, Aast.CIexpr (p, Aast.Id x)), tal, el, uel, _) ->
      N.New (make_class_id env x,
        aast_targl env p tal,
        aast_exprl env el,
        aast_exprl env uel,
        p)
    | Aast.New ((_, Aast.CIexpr (_, Aast.Lvar (pos, x))), tal, el, uel, p) ->
      N.New (make_class_id env (pos, Local_id.to_string x),
        aast_targl env p tal,
        aast_exprl env el,
        aast_exprl env uel,
        p)
    | Aast.New ((_, Aast.CIexpr(p, _e)), tal, el, uel, _) ->
      if FileInfo.is_strict (fst env).in_mode
      then Errors.dynamic_new_in_strict_mode p;
      N.New (make_class_id env (p, SN.Classes.cUnknown),
        aast_targl env p tal,
        aast_exprl env el,
        aast_exprl env uel,
        p)
    | Aast.New _ -> failwith "ast_to_nast aast.new"
    | Aast.Record ((_,Aast.CIexpr(_, Aast.Id x)), l) ->
      let l = List.map l (fun (e1, e2) -> aast_expr env e1, aast_expr env e2) in
      N.Record (make_class_id env x, l)
    | Aast.Record ((_, Aast.CIexpr(_, Aast.Lvar (pos, x))), l) ->
      let l = List.map l (fun (e1, e2) -> aast_expr env e1, aast_expr env e2) in
      N.Record (make_class_id env (pos, Local_id.to_string x), l)
    | Aast.Record ((p, _e), l) ->
      let l = List.map l (fun (e1, e2) -> aast_expr env e1, aast_expr env e2) in
      if (fst env).in_mode = FileInfo.Mstrict
      then Errors.dynamic_new_in_strict_mode p;
      N.Record (make_class_id env (p, SN.Classes.cUnknown), l)
    | Aast.Efun (f, idl) ->
      let idl =
        List.fold_right idl
          ~init:[]
          ~f:(fun ((p, x) as id) acc ->
             if Local_id.to_string x = SN.SpecialIdents.this
             then (Errors.this_as_lexical_variable p; acc)
             else id :: acc)
      in
      let idl' = List.map idl (Env.aast_lvar env) in
      let env = (fst env, Env.empty_local None) in
      List.iter2_exn idl idl' (Env.aast_add_lvar env);
      let f = aast_expr_lambda env f in
      N.Efun (f, idl')
    | Aast.Lfun (_, _::_) -> assert false
    | Aast.Lfun (f, []) ->
      (* We have to build the capture list while we're finding names in
         the closure body---accumulate it in to_capture. *)
      let to_capture = ref [] in
      let handle_unbound (p, x) =
        let cap = Env.lvar env (p, x) in
        to_capture := cap :: !to_capture;
        cap
      in
      let lenv = Env.empty_local @@ Some handle_unbound in
      (* Extend the current let binding into the scope of lambda *)
      Env.copy_let_locals env (fst env, lenv);
      let env = (fst env, lenv) in
      let f = aast_expr_lambda env f in
      (* TODO T28711692: Compute the correct capture list for let variables,
       * it does not seem to affect typechecking... *)
      N.Lfun (f, !to_capture)
    | Aast.Xml (x, al, el) ->
      N.Xml (Env.type_name env x ~allow_typedef:false ~allow_generics:false, aast_attrl env al,
        aast_exprl env el)
    | Aast.Shape fdl ->
      let (shp, _) = begin List.fold_left fdl ~init:([], Ast.ShapeSet.empty)
        ~f:begin fun (fdm, set) (pname, value) ->
          let pos, name = convert_shape_name env pname in
          if Ast.ShapeSet.mem name set
          then Errors.fd_name_already_bound pos;
          (name, (aast_expr env value)) :: fdm, Ast.ShapeSet.add name set
        end
      end in
      N.Shape (List.rev shp)
    | Aast.Unsafe_expr e ->
      N.Unsafe_expr (Errors.ignore_ (fun () -> aast_expr env e))
    | Aast.BracedExpr _ ->
      N.Any
    | Aast.Yield_from e ->
      N.Yield_from (aast_expr env e)
    | Aast.Import _ ->
      N.Any
    | Aast.Omitted ->
      N.Any
    | Aast.Callconv (kind, e) ->
      N.Callconv (kind, aast_expr env e)
    (* The below were not found on the AST.ml so they are not implemented here *)
    | Aast.ValCollection _
    | Aast.KeyValCollection _
    | Aast.This
    | Aast.ImmutableVar _
    | Aast.Dollardollar _
    | Aast.Lplaceholder _
    | Aast.Fun_id _
    | Aast.Method_id _
    | Aast.Method_caller _
    | Aast.Smethod_id _
    | Aast.Special_func _
    | Aast.Pair _
    | Aast.Assert _
    | Aast.Typename _
    | Aast.Any ->
      Errors.internal_error p "Malformed expr: Expr not found on legacy AST: T39599317";
      Aast.Any

  and aast_expr_lambda env f =
    let env = Env.set_ppl env false in
    let h = Option.map f.Aast.f_ret (aast_hint ~allow_retonly:true env) in
    let previous_unsafe = Env.has_unsafe env in
    (* save unsafe and yield state *)
    Env.set_unsafe env false;
    let variadicity, paraml = aast_fun_paraml env f.Aast.f_params in
    (* The bodies of lambdas go through naming in the containing local
     * environment *)
    let body_nast = aast_f_body env f.Aast.f_body in
    let annotation =
      if func_body_had_unsafe env
      then N.BodyNamingAnnotation.NamedWithUnsafeBlocks
      else N.BodyNamingAnnotation.Named in
    (* restore unsafe state *)
    Env.set_unsafe env previous_unsafe;
    (* These could all be probably be replaced with a {... where ...} *)
    let body = {
      N.fb_ast = body_nast;
      fb_annotation = annotation;
    } in
    { N.f_annotation = ();
      f_span = f.Aast.f_span;
      f_mode = (fst env).in_mode;
      f_ret = h;
      f_name = f.Aast.f_name;
      f_params = paraml;
      f_tparams = [];
      f_where_constraints = [];
      f_body = body;
      f_fun_kind = f.Aast.f_fun_kind;
      f_variadic = variadicity;
      f_file_attributes = [];
      f_user_attributes = aast_user_attributes env f.Aast.f_user_attributes;
      f_external = f.Aast.f_external;
      f_namespace = f.Aast.f_namespace;
      f_doc_comment = f.Aast.f_doc_comment;
      f_static = f.Aast.f_static;
    }

  and aast_f_body env f_body =
    if Aast.is_body_named f_body
    then aast_block env f_body.Aast.fb_ast
    else failwith "Malformed f_body: unexpected UnnamedBody from ast_to_nast"

  and make_class_id env (p, x as cid) =
    p,
    match x with
      | x when x = SN.Classes.cParent ->
        if (fst env).current_cls = None then
          let () = Errors.parent_outside_class p in
          N.CI (p, SN.Classes.cUnknown)
        else N.CIparent
      | x when x = SN.Classes.cSelf ->
        if (fst env).current_cls = None then
          let () = Errors.self_outside_class p in
          N.CI (p, SN.Classes.cUnknown)
        else N.CIself
      | x when x = SN.Classes.cStatic -> if (fst env).current_cls = None then
          let () = Errors.static_outside_class p in
          N.CI (p, SN.Classes.cUnknown)
        else N.CIstatic
      | x when x = SN.SpecialIdents.this -> N.CIexpr (p, N.This)
      | x when x = SN.SpecialIdents.dollardollar ->
          (* We won't reach here for "new $$" because the parser creates a
           * proper Ast.Dollardollar node, so make_class_id won't be called with
           * that node. In fact, the parser creates an Ast.Dollardollar for all
           * "$$" except in positions where a classname is expected, like in
           * static member access. So, we only reach here for things
           * like "$$::someMethod()". *)
          N.CIexpr(p, N.Lvar (p, Env.found_dollardollar env p))
      | x when x.[0] = '$' -> N.CIexpr (p, N.Lvar (Env.lvar env cid))
      | _ -> N.CI (Env.type_name env cid ~allow_typedef:false ~allow_generics:true)

  and aast_casel env l =
    List.map l (aast_case env)

  and aast_case env c =
    match c with
    | Aast.Default b ->
      let b = aast_branch ~after_unsafe:(Pos.none, N.Fallthrough) env b in
      N.Default b
    | Aast.Case (e, b) ->
      let e = aast_expr env e in
      let b = aast_branch ~after_unsafe:(Pos.none, N.Fallthrough) env b in
      N.Case (e, b)

  and aast_catchl env l = List.map l (aast_catch env)
  and aast_catch env ((p1, lid1), (p2, lid2), b) =
    Env.scope
      env
      (fun env ->
        let name2 = Local_id.get_name lid2 in
        (* If the variable does not begin with $, it is an immutable binding *)
        let x2 =
          if name2 <> ""
            && name2.[0] = '$' (* This is always true if not in experimental mode *)
          then Env.new_lvar env (p2, name2)
          else Env.new_let_local env (p2, name2) in
        let b = aast_branch env b in
        Env.type_name env (p1, lid1) ~allow_typedef:true ~allow_generics:false, x2, b)

  and aast_afield env field =
    match field with
    | Aast.AFvalue e -> N.AFvalue (aast_expr env e)
    | Aast.AFkvalue (e1, e2) -> N.AFkvalue (aast_expr env e1, aast_expr env e2)

  and aast_afield_value env cname field =
    match field with
    | Aast.AFvalue e -> aast_expr env e
    | Aast.AFkvalue (e1, _e2) ->
      Errors.unexpected_arrow (fst e1) cname;
      aast_expr env e1

  and aast_afield_kvalue env cname field =
    match field with
    | Aast.AFvalue e ->
      Errors.missing_arrow (fst e) cname;
      aast_expr env e,
        aast_expr env (fst e, Aast.Lvar (fst e, Local_id.make_unscoped "__internal_placeholder"))
    | Aast.AFkvalue (e1, e2) -> aast_expr env e1, aast_expr env e2

  and aast_attrl env l = List.map ~f:(aast_attr env) l
  and aast_attr env at =
    match at with
    | Aast.Xhp_simple (x, e) -> N.Xhp_simple (x, aast_expr env e)
    | Aast.Xhp_spread e -> N.Xhp_spread (aast_expr env e)

  and aast_string2 env idl =
    List.map idl (aast_expr env)

  and aast_class_pu_enum env pu_enum =
    let aux (s, h) = (s, aast_hint ~forbid_this:true env h) in
    let aast_pu_member m =
      let pum_types = List.map ~f:aux m.Aast.pum_types in
      let pum_exprs = List.map ~f:(fun (s, e) -> (s, aast_expr env e))
          m.Aast.pum_exprs in
      { Aast.pum_atom = m.Aast.pum_atom
      ; pum_types
      ; pum_exprs
      } in
    let pu_case_values = List.map ~f:aux pu_enum.Aast.pu_case_values in
    let pu_members = List.map ~f:aast_pu_member pu_enum.Aast.pu_members in
    { Aast.pu_name = pu_enum.Aast.pu_name
    ; Aast.pu_is_final = pu_enum.Aast.pu_is_final
    ; Aast.pu_case_types = pu_enum.Aast.pu_case_types
    ; pu_case_values
    ; pu_members
    }

  (**************************************************************************)
  (* Function/Method Body Naming: *)
  (* Ensure that, given a function / class, any UnnamedBody within is
   * transformed into a a named body *)
  (**************************************************************************)

  let func_body f =
    match f.N.f_body.N.fb_annotation with
    | N.BodyNamingAnnotation.Named
    | N.BodyNamingAnnotation.NamedWithUnsafeBlocks -> f.N.f_body
    | N.BodyNamingAnnotation.Unnamed nsenv ->
      let genv = Env.make_fun_genv
        SMap.empty f.N.f_mode (snd f.N.f_name) nsenv in
      let genv = aast_extend_params genv f.N.f_tparams in
      let lenv = Env.empty_local None in
      let env = genv, lenv in
      let env =
        List.fold_left ~f:Env.add_param f.N.f_params ~init:env in
      let env = match f.N.f_variadic with
        | N.FVellipsis _ | N.FVnonVariadic -> env
        | N.FVvariadicArg param -> Env.add_param env param
      in
      let fub_ast = aast_block env f.N.f_body.N.fb_ast in
      let annotation =
        if func_body_had_unsafe env
        then N.BodyNamingAnnotation.NamedWithUnsafeBlocks
        else N.BodyNamingAnnotation.Named in
      Env.check_goto_references env;
      {
        N.fb_ast = fub_ast;
        fb_annotation = annotation;
      }

  let meth_body genv m =
    let named_body =
      match m.N.m_body.N.fb_annotation with
      | N.BodyNamingAnnotation.Named
      | N.BodyNamingAnnotation.NamedWithUnsafeBlocks -> m.N.m_body
      | N.BodyNamingAnnotation.Unnamed nsenv ->
        let genv = { genv with namespace = nsenv } in
        let genv = aast_extend_params genv m.N.m_tparams in
        let env = genv, Env.empty_local None in
        let env =
          List.fold_left ~f:Env.add_param m.N.m_params ~init:env in
        let env = match m.N.m_variadic with
          | N.FVellipsis _ | N.FVnonVariadic -> env
          | N.FVvariadicArg param -> Env.add_param env param
        in
        let fub_ast = aast_block env m.N.m_body.N.fb_ast in
        let annotation =
          if func_body_had_unsafe env
          then N.BodyNamingAnnotation.NamedWithUnsafeBlocks
          else N.BodyNamingAnnotation.Named in
        Env.check_goto_references env;
        { N.fb_ast = fub_ast;
          fb_annotation = annotation;
        } in
    {m with N.m_body = named_body}

  let class_meth_bodies nc =
    let { N.c_tparam_constraints = cstrs; _ } = nc.N.c_tparams in
    let genv  = Env.make_class_genv cstrs
      nc.N.c_mode (nc.N.c_name, nc.N.c_kind)
      Namespace_env.empty_with_default_popt
      (Attributes.mem SN.UserAttributes.uaProbabilisticModel nc.N.c_user_attributes)
    in
    let inst_meths = List.map nc.N.c_methods (meth_body genv) in
    let opt_constructor = match nc.N.c_constructor with
      | None -> None
      | Some c -> Some (meth_body genv c) in
    let static_meths = List.map nc.N.c_static_methods (meth_body genv) in
    { nc with
      N.c_methods        = inst_meths;
      N.c_static_methods = static_meths ;
      N.c_constructor    = opt_constructor ;
    }

  (**************************************************************************)
  (* Typedefs *)
  (**************************************************************************)

  let aast_typedef tdef =
    let cstrs = aast_make_constraints tdef.Aast.t_tparams in
    let env = Env.aast_make_typedef_env cstrs tdef in
    let tconstraint = Option.map tdef.Aast.t_constraint (aast_hint env) in
    List.iter tdef.Aast.t_tparams aast_check_constraint;
    let tparaml = aast_type_paraml env tdef.Aast.t_tparams in
    let attrs = aast_user_attributes env tdef.Aast.t_user_attributes in
    {
      N.t_annotation = ();
      t_name = tdef.Aast.t_name;
      t_tparams = tparaml;
      t_constraint = tconstraint;
      t_kind = aast_hint env tdef.Aast.t_kind;
      t_user_attributes = attrs;
      t_mode = tdef.Aast.t_mode;
      t_namespace = tdef.Aast.t_namespace;
      t_vis = tdef.Aast.t_vis;
    }

  let typedef tdef =
    let tdef = Ast_to_nast.on_typedef tdef in
    aast_typedef tdef

  (**************************************************************************)
  (* Global constants *)
  (**************************************************************************)

  let check_constant_hint cst =
    match cst.Aast.cst_type with
    | None when FileInfo.is_strict cst.Aast.cst_mode ->
        Errors.add_a_typehint (fst cst.Aast.cst_name)
    | None
    | Some _ -> ()

  let check_constant_name genv cst =
    if genv.namespace.Namespace_env.ns_name <> None then
      let pos, name = cst.Aast.cst_name in
      let name = Utils.strip_all_ns name in
      if SN.PseudoConsts.is_pseudo_const (Utils.add_ns name) then
        Errors.name_is_reserved name pos

  let aast_global_const cst =
    let env = Env.aast_make_const_env cst in
    let hint = Option.map cst.Aast.cst_type (aast_hint env) in
    let e =
        let _ = check_constant_name (fst env) cst in
        let _ = check_constant_hint cst in
        Option.map cst.Aast.cst_value (aast_constant_expr env) in
    { N.cst_annotation = ();
      cst_mode = cst.Aast.cst_mode;
      cst_name = cst.Aast.cst_name;
      cst_type = hint;
      cst_value = e;
      cst_namespace = cst.Aast.cst_namespace;
      cst_span = cst.Aast.cst_span;
    }

  let global_const cst =
    let cst = Ast_to_nast.on_constant cst in
    aast_global_const cst

  (**************************************************************************)
  (* The entry point to CHECK the program, and transform the program *)
  (**************************************************************************)

  let aast_program aast =
    let top_level_env = ref (Env.make_top_level_env ()) in
    let rec aux acc def =
      match def with
      | Aast.Fun f -> (N.Fun (aast_fun_ f)) :: acc
      | Aast.Class c -> (N.Class (aast_class_ c)) :: acc
      | Aast.Stmt (_, Aast.Noop)
      | Aast.Stmt (_, Aast.Markup _) -> acc
      | Aast.Stmt s -> (N.Stmt (aast_stmt !top_level_env s)) :: acc
      | Aast.Typedef t -> (N.Typedef (aast_typedef t)) :: acc
      | Aast.Constant cst -> (N.Constant (aast_global_const cst)) :: acc
      | Aast.Namespace (_ns, aast) -> List.fold_left ~f:aux ~init:[] aast @ acc
      | Aast.NamespaceUse _ -> acc
      | Aast.SetNamespaceEnv nsenv ->
        let (genv, lenv) = !top_level_env in
        let genv = { genv with namespace = nsenv } in
        top_level_env := (genv, lenv);
        acc
      | Aast.FileAttributes _ -> acc in
    let on_program aast =
      let nast = List.fold_left ~f:aux ~init:[] aast in
      List.rev nast in
    on_program aast

  let program ast =
    let aast = Ast_to_nast.on_program ast in
    aast_program aast

end

include Make(struct
  let stmt acc _ = acc
  let lvalue acc _ = acc

  let aast_stmt acc _ = acc
  let aast_lvalue acc _ = acc
end)
