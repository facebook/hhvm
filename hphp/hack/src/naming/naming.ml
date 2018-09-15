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
open Ast
open Utils
open String_utils

module N = Nast
module ShapeMap = N.ShapeMap
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
type type_constraint = (Ast.constraint_kind * Ast.hint) list

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
  type_params: type_constraint SMap.t;

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

(* How to behave when we see an unbound name.  Either we raise an
 * error, or we call a function first and continue if it can resolve
 * the name.  This is used to nest environments when processing
 * closures. *)
type unbound_mode =
  | UBMErr
  | UBMFunc of ((Pos.t * string) -> positioned_ident)

(* The primitives to manipulate the naming environment *)
module Env : sig

  type all_locals
  type lenv

  val empty_local : unbound_mode -> lenv
  val make_class_genv :
    TypecheckerOptions.t ->
    type_constraint SMap.t ->
    FileInfo.mode ->
    Ast.id * Ast.class_kind -> Namespace_env.env -> bool -> genv
  val make_class_env :
    TypecheckerOptions.t ->
    type_constraint SMap.t -> Ast.class_ -> genv * lenv
  val make_typedef_env :
    TypecheckerOptions.t ->
    type_constraint SMap.t -> Ast.typedef -> genv * lenv
  val make_fun_genv :
    TypecheckerOptions.t ->
    type_constraint SMap.t ->
    FileInfo.mode -> string -> Namespace_env.env -> genv
  val make_fun_decl_genv :
    TypecheckerOptions.t ->
    type_constraint SMap.t -> Ast.fun_ -> genv
  val make_const_env : TypecheckerOptions.t -> Ast.gconst -> genv * lenv

  val has_unsafe : genv * lenv -> bool
  val set_unsafe : genv * lenv -> bool -> unit

  val in_ppl : genv * lenv -> bool
  val set_ppl : genv * lenv -> bool -> genv * lenv

  val add_lvar : genv * lenv -> Ast.id -> positioned_ident -> unit
  val add_param : genv * lenv -> N.fun_param -> genv * lenv
  val new_lvar : genv * lenv -> Ast.id -> positioned_ident
  val new_let_local : genv * lenv -> Ast.id -> positioned_ident
  val found_dollardollar : genv * lenv -> Pos.t -> positioned_ident
  val get_dollardollar : genv * lenv -> positioned_ident option
  val inside_pipe : genv * lenv -> bool
  val new_pending_lvar : genv * lenv -> Ast.id -> unit
  val promote_pending_lvar : genv * lenv -> string -> unit
  val lvar : genv * lenv -> Ast.id -> positioned_ident
  val let_local : genv * lenv -> Ast.id -> positioned_ident option
  val global_const : genv * lenv -> Ast.id -> Ast.id
  val type_name : genv * lenv -> Ast.id -> allow_typedef:bool -> Ast.id
  val fun_id : genv * lenv -> Ast.id -> Ast.id
  val bind_class_const : genv * lenv -> Ast.id -> unit
  val bind_prop : genv * lenv -> Ast.id -> unit
  val goto_label : genv * lenv -> string -> Pos.t option
  val new_goto_label : genv * lenv -> pstring -> unit
  val new_goto_target : genv * lenv -> pstring -> unit
  val check_goto_references : genv * lenv -> unit
  val copy_let_locals : genv * lenv -> genv * lenv -> unit

  val scope : genv * lenv -> (genv * lenv -> 'a) -> 'a
  val scope_all : genv * lenv -> (genv * lenv -> 'a) -> all_locals * 'a
  val scope_lexical : genv * lenv -> (genv * lenv -> 'a) -> 'a
  val extend_all_locals : genv * lenv -> all_locals -> unit
  val remove_locals : genv * lenv -> Ast.id list -> unit
  val pipe_scope : genv * lenv -> (genv * lenv -> N.expr) -> Local_id.t * N.expr
end = struct

  type map = positioned_ident SMap.t
  type all_locals = Pos.t SMap.t

  (* The local environment *)
  type lenv = {

    (* The set of locals *)
    locals: map ref;

    (* We keep all the locals, even if we are in a different scope
     * to provide better error messages.
     * if you write:
     * if(...) {
     *   $x = ...;
     * }
     * Technically, passed this point, $x is unbound.
     * But it is much better to keep it somewhere, so that you can
     * say it is bound, but in a different scope.
     *)
    all_locals: all_locals ref;

    (* Some statements can define new variables afterwards, e.g.,
     * if (...) {
     *    $x = ...;
     * } else {
     *    $x = ...;
     * }
     * We need to give $x the same name in both branches, but we don't want
     * $x to actually be a local until after the if block. So we stash it here,
     * to indicate a name has been pre-allocated, but that the variable isn't
     * actually defined yet.
     *)
    pending_locals: map ref;

    (* The set of lexically-scoped local `let` variables *)
    (* TODO: Currently these locals live in a separate namespace, it is
     * worthwhile considering unified namespace for all local variables T28712009 *)
    let_locals: map ref;

    (* Tag controlling what we do when we encounter an unbound name.
     * This is used when processing a lambda expression body that has
     * an automatic use list.
     *
     * See expr_lambda for details.
     *)
    unbound_mode: unbound_mode;

    (* The presence of an "UNSAFE" in the function body changes the
     * verifiability of the function's return type, since the unsafe
     * block could return. For the sanity of the typechecker, we flatten
     * this out, but need to track if we've seen an "UNSAFE" in order to
     * do so. *)
    has_unsafe: bool ref;

    (** Allows us to ban $$ appearances outside of pipe expressions and
     * equals expressions within pipes.  *)
    inside_pipe: bool ref;

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

  let empty_local unbound_mode = {
    locals     = ref SMap.empty;
    all_locals = ref SMap.empty;
    pending_locals = ref SMap.empty;
    let_locals = ref SMap.empty;
    unbound_mode;
    has_unsafe = ref false;
    inside_pipe = ref false;
    goto_labels = ref SMap.empty;
    goto_targets = ref SMap.empty;
  }

  let make_class_genv tcopt tparams mode (cid, ckind) namespace is_ppl =
    { in_mode       =
        (if !Autocomplete.auto_complete then FileInfo.Mpartial else mode);
      tcopt;
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

  let make_class_env tcopt tparams c =
    let is_ppl = List.exists
      c.c_user_attributes
      (fun { ua_name; _ } -> snd ua_name = SN.UserAttributes.uaProbabilisticModel) in
    let genv = make_class_genv tcopt tparams c.c_mode
      (c.c_name, c.c_kind) c.c_namespace is_ppl in
    let lenv = empty_local UBMErr in
    let env  = genv, lenv in
    env

  let make_typedef_genv tcopt cstrs tdef = {
    in_mode       = FileInfo.(if !Ide.is_ide_mode then Mpartial else Mstrict);
    tcopt;
    in_try        = false;
    in_finally    = false;
    in_ppl        = false;
    type_params   = cstrs;
    current_cls   = None;
    class_consts = Caml.Hashtbl.create 0;
    class_props = Caml.Hashtbl.create 0;
    droot         = Typing_deps.Dep.Class (snd tdef.t_id);
    namespace     = tdef.t_namespace;
  }

  let make_typedef_env genv cstrs tdef =
    let genv = make_typedef_genv genv cstrs tdef in
    let lenv = empty_local UBMErr in
    let env  = genv, lenv in
    env

  let make_fun_genv tcopt params f_mode f_name f_namespace = {
    in_mode       = f_mode;
    tcopt;
    in_try        = false;
    in_finally    = false;
    in_ppl        = false;
    type_params   = params;
    current_cls   = None;
    class_consts = Caml.Hashtbl.create 0;
    class_props = Caml.Hashtbl.create 0;
    droot         = Typing_deps.Dep.Fun f_name;
    namespace     = f_namespace;
  }

  let make_fun_decl_genv nenv params f =
    make_fun_genv nenv params f.f_mode (snd f.f_name) f.f_namespace

  let make_const_genv tcopt cst = {
    in_mode       = cst.cst_mode;
    tcopt;
    in_try        = false;
    in_finally    = false;
    in_ppl        = false;
    type_params   = SMap.empty;
    current_cls   = None;
    class_consts = Caml.Hashtbl.create 0;
    class_props = Caml.Hashtbl.create 0;
    droot         = Typing_deps.Dep.GConst (snd cst.cst_name);
    namespace     = cst.cst_namespace;
  }

  let make_const_env nenv cst =
    let genv = make_const_genv nenv cst in
    let lenv = empty_local UBMErr in
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
        | FileInfo.Mstrict | FileInfo.Mexperimental -> unbound_name_error genv p x `const
        | FileInfo.Mpartial | FileInfo.Mdecl when not
            (TypecheckerOptions.assume_php genv.tcopt) ->
          unbound_name_error genv p x `const
        | FileInfo.Mphp | FileInfo.Mdecl | FileInfo.Mpartial -> ()
      )
    | _ -> ()

  (* Check and see if the user might have been trying to use one of the
   * generics in scope as a runtime value *)
  let check_no_runtime_generic genv (p, name) =
    let tparaml = SMap.keys genv.type_params in
    if List.mem tparaml name ~equal:(=) then Errors.generic_at_runtime p;
    ()

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
              when TypecheckerOptions.assume_php genv.tcopt
              || name = SN.Classes.cUnknown -> ()
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

  let check_variable_scoping env (p, x) =
    match SMap.get x !(env.all_locals) with
    | Some p' -> Errors.different_scope p x p'
    | None -> ()

  (* Adds a local variable, without any check *)
  let add_lvar (_, lenv) (_, name) (p, x) =
    lenv.locals := SMap.add name (p, x) !(lenv.locals);
    ()

  let add_param env param =
    let p_name = param.N.param_name in
    let id = Local_id.get p_name in
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
          let ident = match SMap.get x !(lenv.pending_locals) with
            | Some (_, ident) -> ident
            | None -> Local_id.make x in
          lenv.all_locals := SMap.add x p !(lenv.all_locals);
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
    let ident = Local_id.make x in
    lenv.all_locals := SMap.add x p !(lenv.all_locals);
    lenv.let_locals := SMap.add x (p, ident) !(lenv.let_locals);
    p, ident

  (* Defines a new local variable for this dollardollar (or reuses
   * the exiting identifier). *)
  let found_dollardollar (genv, lenv) p =
    if not !(lenv.inside_pipe) then
      Errors.undefined p SN.SpecialIdents.dollardollar;
    new_lvar (genv, lenv) (p, SN.SpecialIdents.dollardollar)

  (* Check if dollardollar is defined in the current environment *)
  let get_dollardollar (_genv, lenv) =
    SMap.get SN.SpecialIdents.dollardollar !(lenv.locals)

  let inside_pipe (_, lenv) =
    !(lenv.inside_pipe)

  let new_pending_lvar (_, lenv) (p, x) =
    match SMap.get x !(lenv.locals), SMap.get x !(lenv.pending_locals) with
    | None, None ->
        let y = p, Local_id.make x in
        lenv.pending_locals := SMap.add x y !(lenv.pending_locals)
    | _ -> ()

  let promote_pending_lvar (_, lenv) x =
    match SMap.get x !(lenv.pending_locals) with
    | Some (p, ident) ->
      lenv.locals := SMap.add x (p, ident) !(lenv.locals);
      lenv.pending_locals := SMap.remove x !(lenv.pending_locals)
    | None -> ()

  let handle_undefined_variable (_genv, env) (p, x) =
    match env.unbound_mode with
    | UBMErr -> Errors.undefined p x; p, Local_id.make x
    | UBMFunc f -> f (p, x)

  (* Function used to name a local variable *)
  let lvar (genv, env) (p, x) =
    let p, ident =
      if SN.Superglobals.is_superglobal x && genv.in_mode = FileInfo.Mpartial
      then p, Local_id.make x
      else
        let lcl = SMap.get x !(env.locals) in
        match lcl with
        | Some lcl -> p, snd lcl
        | None when not !Autocomplete.auto_complete ->
            check_variable_scoping env (p, x);
            handle_undefined_variable (genv, env) (p, x)
        | None -> p, Local_id.tmp()
    in
    p, ident

  let let_local (_genv, env) (p, x) =
    let lcl = SMap.get x !(env.let_locals) in
    match lcl with
      | Some lcl -> Some (p, snd lcl)
      | None -> None

  let get_name genv get_pos x =
    lookup genv get_pos x; x

  (* For dealing with namespace fallback on constants *)
  let elaborate_and_get_name_with_fallback
    mk_dep
    genv
    (get_pos : string -> FileInfo.pos option) x =
    let get_name x = get_name genv get_pos x in
    let fq_x = NS.elaborate_id genv.namespace NS.ElaborateConst x in
    let need_fallback =
      genv.namespace.Namespace_env.ns_name <> None &&
      not (String.contains (snd x) '\\') in
    let use_fallback =
      need_fallback &&
      (* __FILE__, __LINE__ etc *)
      (string_starts_with (snd x) "__") && (string_ends_with (snd x) "__") in
    if use_fallback then begin
      let global_x = (fst x, "\\" ^ (snd x)) in
      (* Explicitly add dependencies on both of the consts we could be
       * referring to here. Normally naming doesn't have to deal with
       * deps at all -- they are added during typechecking just by the
       * nature of looking up a class or function name. However, we're
       * flattening namespaces here, and the fallback behavior of
       * consts means that we might suddenly be referring to a
       * different const without any change to the callsite at
       * all. Adding both dependencies explicitly captures this
       * action-at-a-distance. *)
      Typing_deps.add_idep genv.droot (mk_dep (snd fq_x));
      Typing_deps.add_idep genv.droot (mk_dep (snd global_x));
      let mem (_, s) = get_pos s in
      match mem fq_x, mem global_x with
      (* Found in the current namespace *)
      | Some _, _ -> get_name fq_x
      (* Found in the global namespace *)
      | _, Some _ -> get_name global_x
      (* Not found. Pick the more specific one to error on. *)
      | None, None -> get_name fq_x
    end else
      get_name fq_x

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

  let global_const (genv, _env) x  =
    elaborate_and_get_name_with_fallback
      (* Same idea as Dep.FunName, see below. *)
      (fun x -> Typing_deps.Dep.GConstName x)
      genv
      (Naming_heap.ConstPosHeap.get)
      x

  let type_name (genv, _) x ~allow_typedef =
    (* Generic names are not allowed to shadow class names *)
    check_no_runtime_generic genv x;
    let (pos, name) as x = NS.elaborate_id genv.namespace NS.ElaborateClass x in
    match Naming_heap.TypeIdHeap.get name with
    | Some (_def_pos, `Class) ->
      (* Don't let people use strictly internal classes
       * (except when they are being declared in .hhi files) *)
      if name = SN.Classes.cHH_BuiltinEnum &&
        not (string_ends_with (Relative_path.suffix (Pos.filename pos)) ".hhi")
      then Errors.using_internal_class pos (strip_ns name);
      pos, name
    | Some (def_pos, `Typedef) when not allow_typedef ->
      let full_pos, _ = GEnv.get_full_pos genv.tcopt (def_pos, name) in
      Errors.unexpected_typedef pos full_pos;
      pos, name
    | Some (_def_pos, `Typedef) -> pos, name
    | None ->
      handle_unbound_name genv
        (GEnv.type_pos genv.tcopt)
        GEnv.type_canon_name x `cls

  let fun_id (genv, _) x =
    elaborate_and_get_name_with_canonicalized_fallback
      genv
      (Naming_heap.FunPosHeap.get)
      (GEnv.fun_pos genv.tcopt)
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

  let bind_prop (genv, _env) x =
    bind_class_member genv.class_props x

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
    let lenv_pending_copy = !(lenv.pending_locals) in
    let lenv_scoped_copy = !(lenv.let_locals) in
    let res = f env in
    lenv.locals := lenv_copy;
    lenv.pending_locals := lenv_pending_copy;
    lenv.let_locals := lenv_scoped_copy;
    res

  let remove_locals env vars =
    let _genv, lenv = env in
    lenv.locals :=
      List.fold_left vars ~f:(fun l id -> SMap.remove (snd id) l) ~init:!(lenv.locals)

  let scope_all env f =
    let _genv, lenv = env in
    let lenv_all_locals_copy = !(lenv.all_locals) in
    let res = scope env f in
    let lenv_all_locals = !(lenv.all_locals) in
    lenv.all_locals := lenv_all_locals_copy;
    lenv_all_locals, res

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

  let extend_all_locals (_genv, lenv) more_locals =
    lenv.all_locals := SMap.union more_locals !(lenv.all_locals)

  (** Sets up the environment so that naming can be done on the RHS of a
   * pipe expression. It returns the identity of the $$ in the RHS and the
   * named RHS. The steps are as follows:
   *   - Removes the $$ from the local env
   *   - Name the RHS scope
   *   - Restore the binding of $$ in the local env (if it was bound).
   *
   * This will append an error if $$ was not used in the RHS.
   *
   * The inside_pipe flag is also set before the naming and restored afterwards.
   * *)
  let pipe_scope env name_e2 =
    let _, lenv = env in
    let outer_pipe_var_opt =
      SMap.get SN.SpecialIdents.dollardollar !(lenv.locals) in
    let inner_locals = SMap.remove SN.SpecialIdents.dollardollar
      !(lenv.locals) in
    lenv.locals := inner_locals;
    lenv.inside_pipe := true;
    (** Name the RHS of the pipe expression. During this naming, if the $$ from
     * this pipe is used, it will be added to the locals. *)
    let e2 = name_e2 env in
    let pipe_var_ident =
      match SMap.get SN.SpecialIdents.dollardollar !(lenv.locals) with
      | None ->
        Errors.dollardollar_unused (fst e2);
        (** The $$ lvar should be named when it is encountered inside e2,
         * but we've now discovered it wasn't used at all.
         * Create an ID here so we can keep going. *)
        Local_id.make SN.SpecialIdents.dollardollar
      | Some (_, x) -> x
    in
    let restored_locals = SMap.remove SN.SpecialIdents.dollardollar
      !(lenv.locals) in
    (match outer_pipe_var_opt with
    | None -> begin
      lenv.locals := restored_locals;
      lenv.inside_pipe := false;
      end
    | Some outer_pipe_var -> begin
      let restored_locals = SMap.add SN.SpecialIdents.dollardollar
        outer_pipe_var restored_locals in
      lenv.locals := restored_locals;
      end);
    pipe_var_ident, e2
end

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

(* Alok is constantly complaining that in partial mode,
 * he forgets to bind a type parameter, for example T,
 * and because partial assumes T is just a class that lives
 * in PHP land there is no error message.
 * So to help him, I am adding a rule that if
 * the class name starts with a T and is only 2 characters
 * it is considered a type variable. You will not be able to
 * define a class T in php land in this scheme ... But it is a bad
 * name for a class anyway.
*)
let is_alok_type_name (_, x) = String.length x <= 2 && x.[0] = 'T'

let check_constraint (_, (pos, name), _, _) =
  (* TODO refactor this in a separate module for errors *)
  if String.lowercase name = "this"
  then Errors.this_reserved pos
  else if name.[0] <> 'T' then Errors.start_with_T pos

let check_repetition s param =
  let x = snd param.param_id in
  if SSet.mem x s
  then Errors.already_bound (fst param.param_id) x;
  if x <> SN.SpecialIdents.placeholder then SSet.add x s else s

let convert_shape_name env = function
  | SFlit_int (pos, s) -> (pos, SFlit_int (pos, s))
  | SFlit_str (pos, s) -> (pos, SFlit_str (pos, s))
  | SFclass_const (x, (pos, y)) ->
    let class_name =
      if (snd x) = SN.Classes.cSelf then
        match (fst env).current_cls with
        | Some (cid, _) -> cid
        | None -> Errors.self_outside_class pos; (pos, SN.Classes.cUnknown)
      else Env.type_name env x ~allow_typedef:false in
    (pos, SFclass_const (class_name, (pos, y)))

let arg_unpack_unexpected = function
  | [] -> ()
  | (pos, _) :: _ -> Errors.naming_too_few_arguments pos; ()

module type GetLocals = sig
  val stmt : TypecheckerOptions.t -> Namespace_env.env * Pos.t SMap.t ->
    Ast.stmt -> Namespace_env.env * Pos.t SMap.t
  val lvalue : TypecheckerOptions.t -> Namespace_env.env * Pos.t SMap.t ->
    Ast.expr -> Namespace_env.env * Pos.t SMap.t
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
  let rec hint
      ?(forbid_this=false)
      ?(allow_retonly=false)
      ?(allow_typedef=true)
      ?(allow_wildcard=false)
      ?(in_where_clause=false)
      ?(tp_depth=0)
      env (p, h) =
    p, hint_
      ~forbid_this
      ~allow_retonly
      ~allow_typedef
      ~allow_wildcard
      ~in_where_clause
      ~tp_depth
      env h

  and shape_field_to_shape_field_info env { sf_optional; sf_name=_; sf_hint } =
    {
      N.sfi_optional = sf_optional;
      sfi_hint = hint env sf_hint;
    }

  and ast_shape_info_to_nast_shape_info
      env
      { si_allows_unknown_fields; si_shape_field_list } =
    let f fdm shape_field =
      let pos, name = convert_shape_name env shape_field.sf_name in
      if ShapeMap.mem name fdm
      then Errors.fd_name_already_bound pos;
      ShapeMap.add
        name (shape_field_to_shape_field_info env shape_field) fdm in
    let nsi_field_map =
      List.fold_left si_shape_field_list ~init:ShapeMap.empty ~f in
    N.{
      nsi_allows_unknown_fields=si_allows_unknown_fields;
      nsi_field_map
    }

  and hfun env reactivity is_coroutine hl kl variadic_hint h =
    let variadic_hint = match variadic_hint with
      | Hvariadic Some (h) -> N.Hvariadic (Some (hint env h))
      | Hvariadic None -> N.Hvariadic (None)
      | Hnon_variadic -> N.Hnon_variadic in
    N.Hfun (reactivity, is_coroutine, List.map hl (hint env), kl, variadic_hint,
            hint ~allow_retonly:true env h)

  and hint_ ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard
            ~in_where_clause ?(tp_depth=0)
        env x =
    let hint =
      hint ~forbid_this ~allow_typedef ~allow_wildcard in
    match x with
    | Htuple hl ->
      N.Htuple (List.map hl (hint ~allow_retonly env))
    | Hoption h ->
      (* void/noreturn are permitted for Typing.option_return_only_typehint *)
      N.Hoption (hint ~allow_retonly env h)
    | Hsoft h ->
      let h = hint ~allow_retonly env h
      in snd h
    | Hfun (is_coroutine, hl, kl, variadic_hint, h) ->
      hfun env N.FNonreactive is_coroutine hl kl variadic_hint h
    (* Special case for Rx<function> *)
    | Happly ((_, "Rx"), [(_, Hfun (is_coroutine, hl, kl, variadic_hint, h))]) ->
      hfun env N.FReactive is_coroutine hl kl variadic_hint h
    (* Special case for RxShallow<function> *)
    | Happly ((_, "RxShallow"), [(_, Hfun (is_coroutine, hl, kl, variadic_hint, h))]) ->
      hfun env N.FShallow is_coroutine hl kl variadic_hint h
    (* Special case for RxLocal<function> *)
    | Happly ((_, "RxLocal"), [(_, Hfun (is_coroutine, hl, kl, variadic_hint, h))]) ->
      hfun env N.FLocal is_coroutine hl kl variadic_hint h
    | Happly ((p, _x) as id, hl) ->
      let hint_id =
        hint_id ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard ~tp_depth
          env id
          hl in
      (match hint_id with
      | N.Hprim _ | N.Hmixed | N.Hnonnull ->
        if hl <> [] then Errors.unexpected_type_arguments p
      | _ -> ()
      );
      hint_id
    | Haccess ((pos, root_id) as root, id, ids) ->
      let root_ty =
        match root_id with
        | x when x = SN.Classes.cSelf ->
            (match (fst env).current_cls with
            | None ->
               Errors.self_outside_class pos;
               N.Hany
            | Some (cid, _) ->
               N.Happly (cid, [])
            )
        | x when x = SN.Classes.cStatic || x = SN.Classes.cParent ->
            Errors.invalid_type_access_root root; N.Hany
        | _ ->
          let tconst_on_generics_enabled =
            TypecheckerOptions.experimental_feature_enabled
              (fst env).tcopt
            TypecheckerOptions.experimental_tconst_on_generics in
          let h =
            hint_id ~forbid_this ~allow_retonly
              ~allow_typedef ~allow_wildcard:false ~tp_depth env root [] in
          (match h with
          | N.Hthis | N.Happly _ as h -> h
          | N.Habstr _ when in_where_clause && tconst_on_generics_enabled ->
            h
          | _ -> Errors.invalid_type_access_root root; N.Hany
          )
      in
      N.Haccess ((pos, root_ty), id :: ids)
    | Hshape ast_shape_info ->
      N.Hshape (ast_shape_info_to_nast_shape_info env ast_shape_info)

  and hint_id ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard ~tp_depth
    env (p, x as id) hl =
    let params = (fst env).type_params in
    if   is_alok_type_name id && not (SMap.mem x params)
    then Errors.typeparam_alok id;
    (* some common Xhp screw ups *)
    if   (x = "Xhp") || (x = ":Xhp") || (x = "XHP")
    then Errors.disallowed_xhp_type p x;
    match try_castable_hint ~forbid_this ~allow_wildcard ~tp_depth env p x hl with
    | Some h -> h
    | None -> begin
      match x with
        | x when x = SN.Typehints.wildcard && allow_wildcard && tp_depth = 1 ->
          if hl <> [] then
            (Errors.tparam_with_tparam p x;
            N.Hany)
          else
            N.Happly(id, [])
        | x when x = SN.Typehints.wildcard ->
          Errors.wildcard_disallowed p;
          N.Hany
        | x when x.[0] = '\\' &&
          ( x = ("\\"^SN.Typehints.void)
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
      | x when x = SN.Typehints.void && allow_retonly -> N.Hprim N.Tvoid
      | x when x = SN.Typehints.void ->
        if TypecheckerOptions.experimental_feature_enabled
             (fst env).tcopt
             TypecheckerOptions.experimental_void_is_type_of_null
        then N.Hprim N.Tvoid
        else (Errors.return_only_typehint p `void; N.Hany)
      | x when x = SN.Typehints.noreturn && allow_retonly -> N.Hprim N.Tnoreturn
      | x when x = SN.Typehints.noreturn ->
        Errors.return_only_typehint p `noreturn;
        N.Hany
      | x when x = SN.Typehints.num  -> N.Hprim N.Tnum
      | x when x = SN.Typehints.resource -> N.Hprim N.Tresource
      | x when x = SN.Typehints.arraykey -> N.Hprim N.Tarraykey
      | x when x = SN.Typehints.mixed -> N.Hmixed
      | x when x = SN.Typehints.nonnull -> N.Hnonnull
      | x when x = SN.Typehints.dynamic -> N.Hdynamic
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
        let name = Env.type_name env id ~allow_typedef in
        (* Note that we are intentionally setting allow_typedef to `true` here.
         * In general, generics arguments can be typedefs -- there is no
         * runtime restriction. *)
        N.Happly (name, hintl ~allow_wildcard ~forbid_this ~allow_typedef:true
          ~allow_retonly:true ~tp_depth:(tp_depth+1) env hl)
    end

  (* Hints that are valid both as casts and type annotations.  Neither
   * casts nor annotations are a strict subset of the other: For
   * instance, 'object' is not a valid annotation.  Thus callers will
   * have to handle the remaining cases. *)
  and try_castable_hint ?(forbid_this=false) ?(allow_wildcard=false) ~tp_depth env p x hl =
    let hint = hint ~forbid_this ~tp_depth:(tp_depth+1) ~allow_wildcard ~allow_retonly:false in
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
          | [val_] -> N.Harray (Some (hint env val_), None)
          | [key_; val_] ->
            N.Harray (Some (hint env key_), Some (hint env val_))
          | _ -> Errors.too_many_type_arguments p; N.Hany
        )
      | nm when nm = SN.Typehints.darray ->
        Some (match hl with
          | [] ->
              if (fst env).in_mode = FileInfo.Mstrict then
                Errors.too_few_type_arguments p;
              N.Hdarray ((p, N.Hany), (p, N.Hany))
          | [_] -> Errors.too_few_type_arguments p; N.Hany
          | [key_; val_] -> N.Hdarray (hint env key_, hint env val_)
          | _ -> Errors.too_many_type_arguments p; N.Hany)
      | nm when nm = SN.Typehints.varray ->
        Some (match hl with
          | [] ->
              if (fst env).in_mode = FileInfo.Mstrict then
                Errors.too_few_type_arguments p;
              N.Hvarray (p, N.Hany)
          | [val_] -> N.Hvarray (hint env val_)
          | _ -> Errors.too_many_type_arguments p; N.Hany)
      | nm when nm = SN.Typehints.varray_or_darray ->
        Some (match hl with
          | [] ->
              if (fst env).in_mode = FileInfo.Mstrict then
                Errors.too_few_type_arguments p;
              N.Hvarray_or_darray (p, N.Hany)
          | [val_] -> N.Hvarray_or_darray (hint env val_)
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

  and constraint_ ?(forbid_this=false) env (ck, h) = ck, hint ~forbid_this env h

  and hintl ~forbid_this ~allow_retonly
            ~allow_typedef ~allow_wildcard ~tp_depth env l =
    List.map l
      (hint ~forbid_this ~allow_retonly ~allow_typedef ~allow_wildcard ~tp_depth env)
  and hintl_funcall env p l =
    hintl
      ~allow_wildcard:true
      ~forbid_this:false
      ~allow_typedef:true
      ~allow_retonly:true
      ~tp_depth:1
      env (extract_hintl_from_type_args env p l)

  and extract_hintl_from_type_args env p hl =
    let hl, reifiedl = List.unzip hl in
    if not (TypecheckerOptions.experimental_feature_enabled
        (fst env).tcopt
      TypecheckerOptions.experimental_reified_generics)
      && List.exists reifiedl (fun i -> i)
    then
      Errors.experimental_feature p "reified generics";
    hl

  (**************************************************************************)
  (* All the methods and static methods of an interface are "implicitly"
   * declared as abstract
   *)
  (**************************************************************************)

  let add_abstract m = {m with N.m_abstract = true}

  let add_abstractl methods = List.map methods add_abstract

  let interface c constructor methods smethods =
    if c.c_kind <> Cinterface then constructor, methods, smethods else
    let constructor = Option.map constructor add_abstract in
    let methods  = add_abstractl methods in
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
    List.iter tparams begin fun (_, (p,x), _, _) ->
      List.iter class_tparam_names
        (fun (pc,xc) -> if (x = xc) then Errors.shadowed_type_param p pc x)
    end

  let check_tparams_constructor class_tparam_names constructor =
    match constructor with
    | None -> ()
    | Some constr -> check_method_tparams class_tparam_names constr

  let check_tparams_shadow class_tparam_names methods =
    List.iter methods (check_method_tparams class_tparam_names)

  let check_break_continue_level p level_opt =
    if Option.is_some level_opt
    then Errors.break_continue_n_not_supported p

  let ensure_name_not_dynamic env e err =
    match e with
    | (_, (Id _ | Lvar _)) -> ()
    | (p, _) ->
      if (fst env).in_mode = FileInfo.Mstrict
      then err p

  (* Naming of a class *)
  let rec class_ nenv c =
    let constraints = make_constraints c.c_tparams in
    let env      = Env.make_class_env nenv constraints c in
    (* Checking for a code smell *)
    List.iter c.c_tparams check_constraint;
    let name = Env.type_name env c.c_name ~allow_typedef:false in
    let smethods =
      List.fold_right c.c_body ~init:[] ~f:(class_static_method env) in
    let sprops = List.fold_right c.c_body ~init:[] ~f:(class_prop_static env) in
    let attrs = user_attributes env c.c_user_attributes in
    let const = (Attributes.find SN.UserAttributes.uaConst attrs) in
    let props = List.fold_right c.c_body ~init:[] ~f:(class_prop ~const env) in
    let parents =
      List.map c.c_extends
        (hint ~allow_retonly:false ~allow_typedef:false env) in
    let parents = match c.c_kind with
      (* Make enums implicitly extend the BuiltinEnum class in order to provide
       * utility methods. *)
      | Cenum ->
          let pos = fst name in
          let enum_type = pos, N.Happly (name, []) in
          let parent =
            pos, N.Happly ((pos, Naming_special_names.Classes.cHH_BuiltinEnum),
                           [enum_type]) in
          parent::parents
      | _ -> parents in
    let methods  = List.fold_right c.c_body ~init:[] ~f:(class_method env) in
    let uses     = List.fold_right c.c_body ~init:[] ~f:(class_use env) in
    let xhp_attr_uses =
      List.fold_right c.c_body ~init:[] ~f:(xhp_attr_use env) in
    let xhp_category =
      Option.value ~default:[] @@
        List.fold_right c.c_body ~init:None ~f:(xhp_category env) in
    let req_implements, req_extends = List.fold_right c.c_body
      ~init:([], []) ~f:(class_require env c.c_kind) in
    (* Setting a class type parameters constraint to the 'this' type is weird
     * so lets forbid it for now.
     *)
    let tparam_l  = type_paraml ~forbid_this:true env c.c_tparams in
    let consts   = List.fold_right ~f:(class_const env) c.c_body ~init:[] in
    let typeconsts =
      List.fold_right ~f:(class_typeconst env) c.c_body ~init:[] in
    let implements = List.map c.c_implements
      (hint ~allow_retonly:false ~allow_typedef:false env) in
    let constructor = List.fold_left ~f:(constructor env) ~init:None c.c_body in
    let constructor, methods, smethods =
      interface c constructor methods smethods in
    let class_tparam_names = List.map c.c_tparams (fun (_, x, _, _) -> x) in
    let enum = Option.map c.c_enum (enum_ env) in
    check_tparams_constructor class_tparam_names constructor;
    check_name_collision methods;
    check_tparams_shadow class_tparam_names methods;
    check_name_collision smethods;
    check_tparams_shadow class_tparam_names smethods;
    let named_class =
      { N.c_annotation     = ();
        N.c_mode           = c.c_mode;
        N.c_final          = c.c_final;
        N.c_is_xhp         = c.c_is_xhp;
        N.c_kind           = c.c_kind;
        N.c_name           = name;
        N.c_tparams        = (tparam_l, constraints);
        N.c_extends        = parents;
        N.c_uses           = uses;
        N.c_xhp_attr_uses  = xhp_attr_uses;
        N.c_xhp_category   = xhp_category;
        N.c_req_extends    = req_extends;
        N.c_req_implements = req_implements;
        N.c_implements     = implements;
        N.c_consts         = consts;
        N.c_typeconsts     = typeconsts;
        N.c_static_vars    = sprops;
        N.c_vars           = props;
        N.c_constructor    = constructor;
        N.c_static_methods = smethods;
        N.c_methods        = methods;
        N.c_user_attributes = attrs;
        N.c_enum           = enum
      }
    in
    named_class

  and user_attributes env attrl =
    let seen = Caml.Hashtbl.create 0 in
    let validate_seen = begin fun ua_name ->
      let pos, name = ua_name in
      let existing_attr_pos =
        try Some (Caml.Hashtbl.find seen name)
        with Caml.Not_found -> None
      in (match existing_attr_pos with
        | Some p -> Errors.duplicate_user_attribute ua_name p; false
        | None -> Caml.Hashtbl.add seen name pos; true
      )
    end in
    List.fold_left attrl ~init:[] ~f:begin fun acc {ua_name; ua_params} ->
      let ua_name =
        if String.is_prefix (snd ua_name) ~prefix:"__" ||
           TypecheckerOptions.allowed_attribute (fst env).tcopt (snd ua_name)
        then ua_name
        else Env.type_name env ua_name ~allow_typedef:false in
      if not (validate_seen ua_name) then acc
      else let attr = {
             N.ua_name = ua_name;
             N.ua_params = List.map ua_params (expr env)
           } in
           attr :: acc
    end

  and xhp_attribute_decl env h cv is_required maybe_enum =
    let p, (_, id), default = cv in
    if is_required && Option.is_some default then
      Errors.xhp_required_with_default p id;
    let h = (match maybe_enum with
      | Some (pos, _optional, items) ->
        let contains_int = List.exists items begin function
          | _, Int _ -> true
          | _ -> false
        end in
        let contains_str = List.exists items begin function
          | _, String _ | _, String2 _ -> true
          | _ -> false
        end in
        if contains_int && not contains_str then
          Some (pos, Happly ((pos, "int"), []))
        else if not contains_int && contains_str then
          Some (pos, Happly ((pos, "string"), []))
        else
          (* If the list was empty, or if there was a mix of
             ints and strings, then fallback to mixed *)
          Some (pos, Happly ((pos, "mixed"), []))
      | _ -> h) in
    let h = (match h with
      | Some (p, ((Hoption _) as x)) -> begin
          (* `null` has special meaning in XHP and a required attribute cannot
           * actually be nullable *)
          if is_required then Errors.xhp_optional_required_attr p id;
          Some (p, x)
      end
      | Some (p, ((Happly ((_, "mixed"), [])) as x)) -> Some (p, x)
      | Some (p, h) ->
        (* If a non-nullable attribute is not marked as "@required"
           AND it does not have a non-null default value, make the
           typehint nullable for now *)
        if (is_required ||
            (match default with
              | None ->            false
              | Some (_, Null) ->  false
              | Some _ ->          true))
          then Some (p, h)
          else Some (p, Hoption (p, h))
      | None -> None) in
    let h = Option.map h (hint env) in
    let cv = class_prop_ env cv in
    fill_prop [] h cv

  and enum_ env e =
    { N.e_base       = hint env e.e_base;
      N.e_constraint = Option.map e.e_constraint (hint env);
    }

  and type_paraml ?(forbid_this = false) env tparams =
    let _, ret = List.fold_left tparams ~init:(SMap.empty, [])
      ~f:(fun (seen, tparaml) ((_, (p, name), _, _) as tparam) ->
        match SMap.get name seen with
        | None ->
          SMap.add name p seen, (type_param ~forbid_this env tparam)::tparaml
        | Some pos ->
          Errors.shadowed_type_param p pos name;
          seen, tparaml
      )
    in
    List.rev ret

  and type_param ~forbid_this env (variance, param_name, cstr_list, reified) =
    if reified && not (TypecheckerOptions.experimental_feature_enabled
        (fst env).tcopt
      TypecheckerOptions.experimental_reified_generics)
    then
      Errors.experimental_feature (fst param_name) "reified generics";
    variance,
    param_name,
    List.map cstr_list (constraint_ ~forbid_this env),
    reified

  and type_where_constraints env locl_cstrl =
    List.map locl_cstrl (fun (h1, ck, h2) ->
          let ty1 = hint ~in_where_clause:true env h1 in
          let ty2 = hint ~in_where_clause:true env h2 in
          (ty1, ck, ty2))

  and class_use env x acc =
    match x with
    | Attributes _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassUse h ->
      hint ~allow_typedef:false env h :: acc
    | ClassUseAlias (_, (p, _), _, _) ->
      Errors.unsupported_feature p "Trait use aliasing";
      acc
    | ClassUsePrecedence (_, (p, _), _) ->
      Errors.unsupported_feature p "The insteadof keyword";
      acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method _ -> acc
    | TypeConst _ -> acc

  and xhp_attr_use env x acc =
    match x with
    | Attributes _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse h ->
      hint ~allow_typedef:false env h :: acc
    | ClassTraitRequire _ -> acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method _ -> acc
    | TypeConst _ -> acc

  and xhp_category _env x acc =
    match x with
    | Attributes _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory (_, cs) ->
      (match acc with
      | Some _ -> Errors.multiple_xhp_category (fst (List.hd_exn cs)); acc
      | None -> Some cs)
    | XhpChild _ -> acc
    | Method _ -> acc
    | TypeConst _ -> acc

  and class_require env c_kind x acc =
    match x with
    | Attributes _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire (MustExtend, h)
        when c_kind <> Ast.Ctrait && c_kind <> Ast.Cinterface ->
      let () = Errors.invalid_req_extends (fst h) in
      acc
    | ClassTraitRequire (MustExtend, h) ->
      let acc_impls, acc_exts = acc in
      (acc_impls, hint ~allow_typedef:false env h :: acc_exts)
    | ClassTraitRequire (MustImplement, h) when c_kind <> Ast.Ctrait ->
      let () = Errors.invalid_req_implements (fst h) in
      acc
    | ClassTraitRequire (MustImplement, h) ->
      let acc_impls, acc_exts = acc in
      (hint ~allow_typedef:false env h :: acc_impls, acc_exts)
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method _ -> acc
    | TypeConst _ -> acc

  and constructor env acc = function
    | Attributes _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method ({ m_name = (p, name); _ } as m)
        when name = SN.Members.__construct ->
      (match acc with
      | None ->
        let curr_class_kind =
          match (fst env).current_cls with
          | Some (_, kind) -> kind
          | None -> failwith "current class must be set for methods" in
        let params_have_visibility = List.exists m.m_params
        ~f:(fun p -> p.param_modifier <> None) in
        begin match curr_class_kind with
        | Cinterface
        | Ctrait when params_have_visibility ->
          Errors.trait_interface_constructor_promo p
        | _ -> ()
        end;
        Some (method_ (fst env) m)
      | Some _ -> Errors.method_name_already_bound p name; acc)
    | Method _ -> acc
    | TypeConst _ -> acc

  and class_const env x acc =
    match x with
    | Attributes _ -> acc
    | Const (h, l) -> const_defl h env l @ acc
    | AbsConst (h, x) -> abs_const_def env h x :: acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method _ -> acc
    | TypeConst _ -> acc

  and class_prop_static env x acc =
    match x with
    | Attributes _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassVars
      { cv_kinds = kl; cv_hint = h; cv_names = cvl; cv_user_attributes = ua; _ }
      when List.mem kl Static ~equal:(=) ->
      (* Static variables are shared for all classes in the hierarchy.
       * This makes the 'this' type completely unsafe as a type for a
       * static variable. See test/typecheck/this_tparam_static.php as
       * an example of what can occur.
       *)
      let h = Option.map h (hint ~forbid_this:true env) in
      let attrs = user_attributes env ua in
      let cvl = List.map cvl (fun cv ->
        let cv = class_prop_ env cv in
        let cv = fill_prop kl h cv in
        { cv with N.cv_user_attributes = attrs }
      ) in
      cvl @ acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method _ -> acc
    | TypeConst _ -> acc

  and class_prop env ?(const = None) x acc =
    match x with
    | Attributes _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassVars { cv_kinds; cv_hint; cv_names; cv_user_attributes; _ }
      when not (List.mem cv_kinds Static ~equal:(=)) ->
      let h = Option.map cv_hint (hint env) in
      let cvl = List.map cv_names (class_prop_ env) in
      let cvl = List.map cvl (fill_prop cv_kinds h) in
      let attrs = user_attributes env cv_user_attributes in
      (* if class is __Const, make all member fields __Const *)
      let attrs = match const with
      | Some c -> if not (Attributes.mem SN.UserAttributes.uaConst attrs)
        then c :: attrs else attrs
      | None -> attrs in
      let cvl = List.map cvl (fun cv -> { cv with N.cv_user_attributes = attrs}) in
      cvl @ acc
    | ClassVars _ -> acc
    | XhpAttr (h, cv, is_required, maybe_enum) ->
      (xhp_attribute_decl env h cv is_required maybe_enum) :: acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method _ -> acc
    | TypeConst _ -> acc

  and class_static_method env x acc =
    match x with
    | Attributes _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method m when snd m.m_name = SN.Members.__construct -> acc
    | Method m when List.mem m.m_kind Static ~equal:(=) -> method_ (fst env) m :: acc
    | Method _ -> acc
    | TypeConst _ -> acc

  and class_method env x acc =
    match x with
    | Attributes _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method m when snd m.m_name = SN.Members.__construct -> acc
    | Method m when not (List.mem m.m_kind Static ~equal:(=)) ->(
      match (m.m_name, m.m_params) with
        | ( (m_pos, m_name), _::_) when m_name = SN.Members.__clone ->
            Errors.clone_too_many_arguments m_pos; acc
        | _ -> let genv = fst env in method_ genv m :: acc
      )
    | Method _ -> acc
    | TypeConst _ -> acc

  and class_typeconst env x acc =
    match x with
    | Attributes _ -> acc
    | Const _ -> acc
    | AbsConst _ -> acc
    | ClassUse _ -> acc
    | ClassUseAlias _ -> acc
    | ClassUsePrecedence _ -> acc
    | XhpAttrUse _ -> acc
    | ClassTraitRequire _ -> acc
    | ClassVars _ -> acc
    | XhpAttr _ -> acc
    | XhpCategory _ -> acc
    | XhpChild _ -> acc
    | Method _ -> acc
    | TypeConst t -> typeconst env t :: acc

  and check_constant_expr env (pos, e) =
    match e with
    | Unsafeexpr _ | Id _ | Null | True | False | Int _
    | Float _ | String _ -> ()
    | Class_const ((_, cls), _)
      when (match cls with Id (_, "static") -> false | _ -> true) -> ()
    | Unop ((Uplus | Uminus | Utild | Unot), e) -> check_constant_expr env e
    | Binop (op, e1, e2) ->
      (* Only assignment is invalid *)
      (match op with
        | Eq _ -> Errors.illegal_constant pos
        | _ ->
          check_constant_expr env e1;
          check_constant_expr env e2)
    | Eif (e1, e2, e3) ->
      check_constant_expr env e1;
      Option.iter e2 (check_constant_expr env);
      check_constant_expr env e3
    | Array l -> List.iter l (check_afield_constant_expr env)
    | Darray l -> List.iter l (fun (e1, e2) ->
        check_constant_expr env e1;
        check_constant_expr env e2)
    | Varray l -> List.iter l (check_constant_expr env)
    | Shape fdl ->
        (* Only check the values because shape field names are always legal *)
        List.iter fdl (fun (_, e) -> check_constant_expr env e)
    | Call ((_, Id (_, cn)), _, el, uel) when cn = SN.SpecialFunctions.tuple ->
        (* Tuples are not really function calls, they are just parsed that way*)
        arg_unpack_unexpected uel;
        List.iter el (check_constant_expr env)
    | Collection (id, l) ->
      let p, cn = NS.elaborate_id ((fst env).namespace) NS.ElaborateClass id in
      (* Only vec/keyset/dict are allowed because they are value types *)
      (match cn with
        | _ when
             cn = SN.Collections.cVec
          || cn = SN.Collections.cKeyset
          || cn = SN.Collections.cDict ->
          List.iter l (check_afield_constant_expr env)
        | _ -> Errors.illegal_constant p)
    | _ -> Errors.illegal_constant pos

  and check_afield_constant_expr env = function
    | AFvalue e -> check_constant_expr env e
    | AFkvalue (e1, e2) ->
        check_constant_expr env e1;
        check_constant_expr env e2

  and constant_expr env e =
    let valid_constant_expression = Errors.try_with_error begin fun () ->
      check_constant_expr env e;
      true
    end (fun () -> false) in
    if valid_constant_expression then expr env e else fst e, N.Any

  and const_defl h env l = List.map l (const_def h env)
  and const_def h env (x, e) =
    Env.bind_class_const env x;
    let h = Option.map h (hint env) in
    h, x, Some (constant_expr env e)

  and abs_const_def env h x =
    Env.bind_class_const env x;
    let h = Option.map h (hint env) in
    h, x, None

  and class_prop_ env (_, x, e) =
    Env.bind_prop env x;
    let e = Option.map e (expr env) in
    (* If the user has not provided a value, we initialize the member variable
     * ourselves to a value of type Tany. Classes might inherit from our decl
     * mode class that are themselves not in decl, and there's no way to figure
     * out what variables are initialized in a decl class without typechecking
     * its initalizers and constructor, which we don't want to do, so just
     * assume we're covered. *)
    let e =
      if (fst env).in_mode = FileInfo.Mdecl && e = None
      then Some (fst x, N.Any)
      else e
    in
    let is_xhp = try ((String.sub (snd x) 0 1) = ":") with Invalid_argument _ -> false
    in
    { N.cv_final = false;
      N.cv_is_xhp = is_xhp;
      N.cv_visibility = N.Public;
      N.cv_type = None;
      N.cv_id = x;
      N.cv_expr = e;
      N.cv_user_attributes = [];
    }

  and fill_prop kl ty x =
    let x = { x with N.cv_type = ty } in
    List.fold_left kl ~init:x ~f:begin fun x k ->
      (* There is no field Static, they are dissociated earlier.
         An abstract class variable doesn't make sense.
       *)
      match k with
      | Final     -> { x with N.cv_final = true }
      | Static    -> x
      | Abstract  -> x
      | Private   -> { x with N.cv_visibility = N.Private }
      | Public    -> { x with N.cv_visibility = N.Public }
      | Protected -> { x with N.cv_visibility = N.Protected }
    end

  and typeconst env t =
    (* We use the same namespace as constants within the class so we cannot have
     * a const and type const with the same name
     *)
    Env.bind_class_const env t.tconst_name;
    let constr = Option.map t.tconst_constraint (hint env) in
    let hint_ =
      match t.tconst_type with
      | None when not t.tconst_abstract ->
          Errors.not_abstract_without_typeconst t.tconst_name;
          t.tconst_constraint
      | Some _h when t.tconst_abstract ->
          Errors.abstract_with_typeconst t.tconst_name;
          None
      | h -> h
    in
    let type_ = Option.map hint_ (hint env) in
    N.({ c_tconst_name = t.tconst_name;
         c_tconst_constraint = constr;
         c_tconst_type = type_;
       })

  and func_body_had_unsafe env = Env.has_unsafe env

  and method_ genv m =
    let genv = extend_params genv m.m_tparams in
    let env = genv, Env.empty_local UBMErr in
    (* Cannot use 'this' if it is a public instance method *)
    let variadicity, paraml = fun_paraml env m.m_params in
    let contains_visibility = List.exists m.m_kind ~f:(
        function
        | Private
        | Public
        | Protected -> true
        | _ -> false
      ) in
    if not contains_visibility then
      Errors.method_needs_visibility (fst m.m_name);
    let acc = false, false, N.Public in
    let final, abs, vis = List.fold_left ~f:kind ~init:acc m.m_kind in
    List.iter m.m_tparams check_constraint;
    let tparam_l = type_paraml env m.m_tparams in
    let where_constraints = type_where_constraints env m.m_constrs in
    let ret = Option.map m.m_ret (hint ~allow_retonly:true env) in
    let f_kind = m.m_fun_kind in
    let body = (match genv.in_mode with
      | FileInfo.Mdecl | FileInfo.Mphp ->
        N.NamedBody {
          N.fnb_nast = [];
          fnb_unsafe = true;
        }
      | FileInfo.Mstrict | FileInfo.Mpartial | FileInfo.Mexperimental ->
        N.UnnamedBody {
          N.fub_ast = m.m_body;
          fub_tparams = m.m_tparams;
          fub_namespace = genv.namespace;
        }
    ) in
    let attrs = user_attributes env m.m_user_attributes in
    { N.m_annotation      = ()          ;
      N.m_final           = final       ;
      N.m_visibility      = vis         ;
      N.m_abstract        = abs         ;
      N.m_name            = m.Ast.m_name;
      N.m_tparams         = tparam_l    ;
      N.m_where_constraints = where_constraints ;
      N.m_params          = paraml      ;
      N.m_body            = body        ;
      N.m_fun_kind        = f_kind      ;
      N.m_ret             = ret         ;
      N.m_variadic        = variadicity ;
      N.m_user_attributes = attrs;
      N.m_ret_by_ref      = m.m_ret_by_ref;
      N.m_external        = m.m_external;
    }

  and kind (final, abs, vis) = function
    | Final -> true, abs, vis
    | Static -> final, abs, vis
    | Abstract -> final, true, vis
    | Private -> final, abs, N.Private
    | Public -> final, abs, N.Public
    | Protected -> final, abs, N.Protected

  and fun_paraml env l =
    let _names = List.fold_left ~f:check_repetition ~init:SSet.empty l in
    let variadicity, l = determine_variadicity env l in
    variadicity, List.map l (fun_param env)

  and determine_variadicity env l =
    match l with
      | [] -> N.FVnonVariadic, []
      | [x] -> (
        match x.param_is_variadic, x.param_id with
          | false, _ -> N.FVnonVariadic, [x]
          (* NOTE: variadic params are removed from the list *)
          | true, (p, "...") -> N.FVellipsis p, []
          | true, _ -> N.FVvariadicArg (fun_param env x), []
      )
      | x :: rl ->
        let variadicity, rl = determine_variadicity env rl in
        variadicity, x :: rl

  and fun_param env param =
    let p, name = param.param_id in
    let ident = Local_id.get name in
    Env.add_lvar env param.param_id (p, ident);
    let ty = Option.map param.param_hint (hint env) in
    let eopt = Option.map param.param_expr (expr env) in
    let _ = match (fst env).in_mode with
    | FileInfo.Mstrict when param.param_is_reference ->
      Errors.reference_in_strict_mode p;
    | _ -> () in
    { N.param_annotation = p;
      param_hint = ty;
      param_is_reference = param.param_is_reference;
      param_is_variadic = param.param_is_variadic;
      param_pos = p;
      param_name = name;
      param_expr = eopt;
      param_callconv = param.param_callconv;
      param_user_attributes = user_attributes env param.param_user_attributes;
    }

  and make_constraints paraml =
    List.fold_right paraml ~init:SMap.empty
      ~f:begin fun (_, (_, x), cstr_list, _) acc ->
        SMap.add x cstr_list acc
      end

  and extend_params genv paraml =
    let params = List.fold_right paraml ~init:genv.type_params
      ~f:begin fun (_, (_, x), cstr_list, _) acc ->
        SMap.add x cstr_list acc
      end in
    { genv with type_params = params }

  and fun_ nenv f =
    let tparams = make_constraints f.f_tparams in
    let genv = Env.make_fun_decl_genv nenv tparams f in
    let lenv = Env.empty_local UBMErr in
    let env = genv, lenv in
    let where_constraints = type_where_constraints env f.f_constrs in
    let h = Option.map f.f_ret (hint ~allow_retonly:true env) in
    let variadicity, paraml = fun_paraml env f.f_params in
    let x = Env.fun_id env f.f_name in
    List.iter f.f_tparams check_constraint;
    let f_tparams = type_paraml env f.f_tparams in
    let f_kind = f.f_fun_kind in
    let body = match genv.in_mode with
      | FileInfo.Mdecl | FileInfo.Mphp ->
        N.NamedBody {
          N.fnb_nast = [];
          fnb_unsafe = true;
        }
      | FileInfo.Mstrict | FileInfo.Mpartial | FileInfo.Mexperimental ->
        N.UnnamedBody {
          N.fub_ast = f.f_body;
          fub_tparams = f.f_tparams;
          fub_namespace = f.f_namespace;
        }
    in
    let named_fun = {
      N.f_annotation = ();
      f_mode = f.f_mode;
      f_ret = h;
      f_name = x;
      f_tparams = f_tparams;
      f_where_constraints = where_constraints;
      f_params = paraml;
      f_body = body;
      f_fun_kind = f_kind;
      f_variadic = variadicity;
      f_user_attributes = user_attributes env f.f_user_attributes;
      f_ret_by_ref = f.f_ret_by_ref;
      f_external = f.f_external;
    } in
    named_fun

  and get_using_vars e =
    match snd e with
    | Expr_list using_clauses ->
      List.concat_map using_clauses get_using_vars
      (* Simple assignment to local of form `$lvar = e` *)
    | Binop (Ast.Eq None, (_, Lvar lvar), _) ->
      [lvar]
      (* Arbitrary expression. This will be assigned to a temporary *)
    | _ ->
      []

  and stmt env (p, st_ as st) =
    match st_ with
    | Let (x, h, e)        -> let_stmt env x h e
    | Block _              -> assert false
    | Unsafe               -> assert false
    | Fallthrough          -> N.Fallthrough
    | Noop                 -> N.Noop
    | Markup (_, None)     -> N.Noop (* ignore markup *)
    | Markup (_, Some e)   -> N.Expr (expr env e)
    | Break level_opt ->
      check_break_continue_level p level_opt;
      N.Break p
    | Continue level_opt ->
      check_break_continue_level p level_opt;
      N.Continue p
    | Throw e              -> let terminal = not (fst env).in_try in
                              N.Throw (terminal, expr env e)
    | Return e        -> N.Return (p, oexpr env e)
    | GotoLabel label      -> name_goto_label env label
    | Goto label           -> name_goto env label
    | Static_var el        -> N.Static_var (static_varl env el)
    | Global_var el        -> N.Global_var (global_varl env el)
    | If (e, b1, b2)       -> if_stmt env st e b1 b2
    | Do (b, e)            -> do_stmt env b e
    | While (e, b)         -> while_stmt env e b
    | Declare (is_block, e, b)  -> declare_stmt env is_block e b
    | Using s -> using_stmt env s.us_has_await s.us_expr s.us_block
    | For (st1, e, st2, b) -> for_stmt env st1 e st2 b
    | Switch (e, cl)       -> switch_stmt env st e cl
    | Foreach (e, aw, ae, b)-> foreach_stmt env e aw ae b
    | Try (b, cl, fb)      -> try_stmt env st b cl fb
    | Def_inline _ ->
      Errors.experimental_feature p "inlined definitions"; N.Expr (p, N.Any)
    | Expr (cp, Call ((p, Id (fp, fn)), hl, el, uel))
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
      (match el with
        | [] | [_]  ->
          Errors.naming_too_few_arguments p;
          N.Expr (cp, N.Any)
        | (cond_p, cond) :: el ->
          let violation = (cp, Call
            ((p, Id (fp, "\\"^SN.SpecialFunctions.invariant_violation)), hl, el,
             uel)) in
          if cond <> False then
            let b1, b2 = [p, Expr violation], [p, Noop] in
            let cond = cond_p, Unop (Unot, (cond_p, cond)) in
            if_stmt env st cond b1 b2
          else (* a false <condition> means unconditional invariant_violation *)
            N.Expr (expr env violation)
      )
    | Expr e               -> N.Expr (expr env e)

  and let_stmt env x h e =
    let e = expr env e in
    let h = Option.map h (hint env) in
    let x = Env.new_let_local env x in
    N.Let (x, h, e)

  and if_stmt env st e b1 b2 =
    let e = expr env e in
    let nsenv = (fst env).namespace in
    let _, vars = GetLocals.stmt (fst env).tcopt (nsenv, SMap.empty) st in
    SMap.iter (fun x p -> Env.new_pending_lvar env (p, x)) vars;
    let result = Env.scope env (
    fun env ->
      let all1, b1 = branch env b1 in
      let all2, b2 = branch env b2 in
      Env.extend_all_locals env all2;
      Env.extend_all_locals env all1;
      N.If (e, b1, b2)
   ) in
   SMap.iter (fun x _ -> Env.promote_pending_lvar env x) vars;
   result

  and do_stmt env b e =
    (* lexical block of `do` is extended to the expr of loop termination *)
    Env.scope_lexical env (fun env ->
      let b = block ~new_scope:false env b in
      let e = expr env e in
      N.Do (b, e)
    )

  and while_stmt env e b =
    let e = expr env e in
    N.While (e, block env b)

  and declare_stmt _env _is_block e _b =
    Errors.declare_statement_in_hack (fst e);
    N.Expr (fst e, N.Any)

  (* Scoping is essentially that of do: block is always executed *)
  and using_stmt env has_await e b =
    let vars = get_using_vars e in
    let e = expr env e in
    let b = block ~new_scope:false env b in
    Env.remove_locals env vars;
    N.Using (has_await, e, b)

  and for_stmt env e1 e2 e3 b =
    (* The initialization and condition expression should be in the outer scope,
     * as they are always executed. *)
    let e1 = expr env e1 in
    let e2 = expr env e2 in
    Env.scope env (
    fun env ->
      (* The third expression (iteration step) should have the same scope as the
       * block, as it is not always executed. *)
      let b = block ~new_scope:false env b in
      let e3 = expr env e3 in
      N.For (e1, e2, e3, b)
   )

  and switch_stmt env st e cl =
    let e = expr env e in
    let nsenv = (fst env).namespace in
    let _, vars = GetLocals.stmt (fst env).tcopt (nsenv, SMap.empty) st in
    SMap.iter (fun x p -> Env.new_pending_lvar env (p, x)) vars;
    let result = Env.scope env begin fun env ->
      let all_locals_l, cl = casel env cl in
      List.iter all_locals_l (Env.extend_all_locals env);
      N.Switch (e, cl)
    end in
    SMap.iter (fun x _ -> Env.promote_pending_lvar env x) vars;
    result

  and foreach_stmt env e aw ae b =
    let e = expr env e in
    Env.scope env begin fun env ->
      let ae = as_expr env aw ae in
      let b = block env b in
      N.Foreach (e, ae, b)
    end

  and as_expr env aw =
    let handle_v ev = match ev with
    | p, Id x when (fst env).in_mode = FileInfo.Mexperimental ->
      let x = Env.new_let_local env x in
      let ev = (p, N.ImmutableVar x) in
      ev
    | p, Id _ ->
      Errors.expected_variable p;
      p, N.Lvar (Env.new_lvar env (p, "__internal_placeholder"))
    | ev ->
      let nsenv = (fst env).namespace in
      let _, vars =
        GetLocals.lvalue (fst env).tcopt (nsenv, SMap.empty) ev in
      SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
      let ev = expr env ev in
      ev
    in
    let handle_k ek = match ek with
    | p, Lvar x ->
      p, N.Lvar (Env.new_lvar env x)
    | p, Id x when (fst env).in_mode = FileInfo.Mexperimental ->
        p, N.ImmutableVar (Env.new_let_local env x)
    | p, _ ->
      Errors.expected_variable p;
      p, N.Lvar (Env.new_lvar env (p, "__internal_placeholder"))
    in
    begin function
    | As_v ev ->
      let ev = handle_v ev in
      begin match aw with
      | None -> N.As_v ev
      | Some p -> N.Await_as_v (p, ev)
      end (* match *)
    | As_kv (k, ev) ->
      let k = handle_k k in
      let ev = handle_v ev in
      begin match aw with
        | None -> N.As_kv (k, ev)
        | Some p -> N.Await_as_kv (p, k, ev)
      end (* match *)
    end (* function *)

  and try_stmt env st b cl fb =
    let nsenv = (fst env).namespace in
    let _, vars =
      GetLocals.stmt (fst env).tcopt (nsenv, SMap.empty) st in
    SMap.iter (fun x p -> Env.new_pending_lvar env (p, x)) vars;
    let result = Env.scope env (
    fun env ->
      let genv, lenv = env in
      (* isolate finally from the rest of the try-catch: if the first
       * statement of the try is an uncaught exception, finally will
       * still be executed *)
      let _all_finally, fb = branch ({genv with in_finally = true}, lenv) fb in
      let all_locals_b, b = branch ({genv with in_try = true}, lenv) b in
      let all_locals_cl, cl = catchl env cl in
      List.iter all_locals_cl (Env.extend_all_locals env);
      Env.extend_all_locals env all_locals_b;
      N.Try (b, cl, fb)
    ) in
    SMap.iter (fun x _ -> Env.promote_pending_lvar env x) vars;
    result

  and stmt_list ?after_unsafe stl env =
    let stmt_list = stmt_list ?after_unsafe in
    match stl with
    | [] -> []
    | (_, Unsafe) :: rest ->
      Env.set_unsafe env true;
      let st = Errors.ignore_ (fun () -> N.Unsafe_block (stmt_list rest env)) in
      st :: Option.to_list after_unsafe
    | (_, Block b) :: rest ->
      (* Add lexical scope for block scoped let variables *)
      let b = Env.scope_lexical env (stmt_list b) in
      let rest = stmt_list rest env in
      b @ rest
    | x :: rest ->
      let x = stmt env x in
      let rest = stmt_list rest env in
      x :: rest

  and block ?(new_scope=true) env stl =
    if new_scope
    then Env.scope env (stmt_list stl)
    else stmt_list stl env

  and branch ?after_unsafe env stmt_l =
    Env.scope_all env (stmt_list ?after_unsafe stmt_l)

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

  and static_varl env l = List.map l (static_var env)
  and static_var env = function
    | p, Lvar _ as lv -> expr env (p, Binop(Eq None, lv, (p, Null)))
    | e -> expr env e

  and global_varl env l = List.map l (global_var env)
  and global_var env = function
    | p, Lvar _ as lv -> expr env (p, Binop(Eq None, lv, (p, Null)))
    | e -> expr env e

  and expr_obj_get_name env = function
    | p, Id x -> p, N.Id x
    | p, e ->
        expr env (p, e)

  and exprl env l = List.map l (expr env)
  and oexpr env e = Option.map e (expr env)
  and expr env (p, e) = p, expr_ env p e
  and expr_ env p = function
    | Array l ->
      let tcopt = (fst env).tcopt in
      let array_literals_disallowed =
        TypecheckerOptions.disallow_array_literal tcopt in
      if array_literals_disallowed
      then Errors.array_literals_disallowed p;
      N.Array (List.map l (afield env))
    | ParenthesizedExpr (p, e) -> expr_ env p e
    | Darray l ->
      N.Darray (List.map l (fun (e1, e2) -> expr env e1, expr env e2))
    | Varray l -> N.Varray (List.map l (expr env))
    | Collection (id, l) -> begin
      let p, cn = NS.elaborate_id ((fst env).namespace) NS.ElaborateClass id in
      match cn with
        | x when N.is_vc_kind x ->
          N.ValCollection ((N.get_vc_kind cn),
            (List.map l (afield_value env cn)))
        | x when N.is_kvc_kind x ->
          N.KeyValCollection ((N.get_kvc_kind cn),
            (List.map l (afield_kvalue env cn)))
        | x when x = SN.Collections.cPair ->
          (match l with
            | [] ->
                Errors.naming_too_few_arguments p;
                N.Any
            | e1::e2::[] ->
              let pn = SN.Collections.cPair in
              N.Pair (afield_value env pn e1, afield_value env pn e2)
            | _ ->
                Errors.naming_too_many_arguments p;
                N.Any
          )
        | _ ->
            Errors.expected_collection p cn;
            N.Any
      end
    | Clone e -> N.Clone (expr env e)
    | Null -> N.Null
    | True -> N.True
    | False -> N.False
    | Int s -> N.Int s
    | Float s -> N.Float s
    | String s -> N.String s
    | String2 idl
    (* treat execution operator similar to interpolated strings *)
    | Execution_operator idl -> N.String2 (string2 env idl)
    | PrefixedString (n, e) -> N.PrefixedString (n, (expr env e))
    | Id x ->
      (** TODO: Emit proper error messages T28473207. Currently the error message
        * emitted has reason Naming[2049] unbound name for global constant *)
      begin match Env.let_local env x with
        | Some x -> N.ImmutableVar x
        | None -> N.Id (Env.global_const env x)
      end (* match *)
    | Lvar (_, x) when x = SN.SpecialIdents.this -> N.This
    | Lvar (_, x) when x = SN.SpecialIdents.dollardollar ->
      N.Dollardollar (Env.found_dollardollar env p)
    | Lvar (pos, x) when x = SN.SpecialIdents.placeholder ->
      N.Lplaceholder pos
    | Lvar x ->
        N.Lvar (Env.lvar env x)
    | Obj_get (e1, e2, nullsafe) ->
        (* If we encounter Obj_get(_,_,true) by itself, then it means "?->"
           is being used for instance property access; see the case below for
           handling nullsafe instance method calls to see how this works *)
        let nullsafe = match nullsafe with
          | OG_nullsafe -> N.OG_nullsafe
          | OG_nullthrows -> N.OG_nullthrows
        in
        N.Obj_get (expr env e1, expr_obj_get_name env e2, nullsafe)
    | Array_get ((p, Lvar x), None) ->
        let id = p, N.Lvar (Env.lvar env x) in
        N.Array_get (id, None)
    | Array_get (e1, e2) -> N.Array_get (expr env e1, oexpr env e2)
    | Class_get ((_, (Id x1 | Lvar x1)), (_, (Id x2 | Lvar x2))) ->
      N.Class_get (make_class_id env x1 [], x2)
    | Class_get (x1, x2) ->
      ensure_name_not_dynamic env x1
        Errors.dynamic_class_name_in_strict_mode;
      ensure_name_not_dynamic env x2
        Errors.dynamic_class_property_name_in_strict_mode;
      N.Any
    | Class_const ((_, Id x1), x2)
    | Class_const ((_, Lvar x1), x2) ->
      let (genv, _) = env in
      let (_, name) = NS.elaborate_id genv.namespace NS.ElaborateClass x1 in
      begin match Naming_heap.TypeIdHeap.get name with
      | Some (_, `Typedef) when (snd x2) = "class" ->
        N.Typename (Env.type_name env x1 ~allow_typedef:true)
      | _ ->
        N.Class_const (make_class_id env x1 [], x2)
      end
    | Class_const _ ->
      (* TODO: report error in strict mode *)
      N.Any
    | Call ((_, Id (p, pseudo_func)), hl, el, uel)
        when pseudo_func = SN.SpecialFunctions.echo ->
        arg_unpack_unexpected uel ;
        N.Call (N.Cnormal, (p, N.Id (p, pseudo_func)), hintl_funcall env p hl, exprl env el, [])
    | Call ((p, Id (_, cn)), hl, el, uel)
      when cn = SN.SpecialFunctions.call_user_func ->
        arg_unpack_unexpected uel ;
        (match el with
        | [] -> Errors.naming_too_few_arguments p; N.Any
        | f :: el -> N.Call (N.Cuser_func, expr env f, hintl_funcall env p hl, exprl env el, [])
        )
    | Call ((p, Id (_, cn)), _, el, uel) when cn = SN.SpecialFunctions.fun_ ->
        arg_unpack_unexpected uel ;
        let (genv, _) = env in
        (match el with
        | [] -> Errors.naming_too_few_arguments p; N.Any
        | [_, String s] when String.contains s ':' ->
          Errors.illegal_meth_fun p; N.Any
        | [_, String s] when genv.in_ppl && SN.PPLFunctions.is_reserved s ->
          Errors.ppl_meth_pointer p ("fun("^s^")"); N.Any
        | [p, String x] ->
          (* Functions referenced by fun() are always fully-qualified. *)
          let x = if x <> "" && x.[0] <> '\\' then "\\" ^ x else x in
          N.Fun_id (Env.fun_id env (p, x))
        | [p, _] ->
            Errors.illegal_fun p;
            N.Any
        | _ -> Errors.naming_too_many_arguments p; N.Any
        )
    | Call ((p, Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.inst_meth ->
        arg_unpack_unexpected uel ;
        (match el with
        | [] -> Errors.naming_too_few_arguments p; N.Any
        | [_] -> Errors.naming_too_few_arguments p; N.Any
        | instance::(p, String meth)::[] ->
          N.Method_id (expr env instance, (p, meth))
        | (p, _)::(_)::[] ->
          Errors.illegal_inst_meth p;
          N.Any
        | _ -> Errors.naming_too_many_arguments p; N.Any
        )
    | Call ((p, Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.meth_caller ->
        arg_unpack_unexpected uel ;
        (match el with
        | [] -> Errors.naming_too_few_arguments p; N.Any
        | [_] -> Errors.naming_too_few_arguments p; N.Any
        | e1::e2::[] ->
            (match (expr env e1), (expr env e2) with
            | (pc, N.String cl), (pm, N.String meth) ->
              N.Method_caller (Env.type_name env (pc, cl) ~allow_typedef:false, (pm, meth))
            | (_, N.Class_const ((_, N.CI (cl, _)), (_, mem))), (pm, N.String meth)
              when mem = SN.Members.mClass ->
              N.Method_caller (Env.type_name env cl ~allow_typedef:false, (pm, meth))
            | (p, _), (_) ->
              Errors.illegal_meth_caller p;
              N.Any
            )
        | _ -> Errors.naming_too_many_arguments p; N.Any
        )
    | Call ((p, Id (_, cn)), _, el, uel)
      when cn = SN.SpecialFunctions.class_meth ->
        arg_unpack_unexpected uel ;
        (match el with
        | [] -> Errors.naming_too_few_arguments p; N.Any
        | [_] -> Errors.naming_too_few_arguments p; N.Any
        | e1::e2::[] ->
            (match (expr env e1), (expr env e2) with
            | (pc, N.String cl), (pm, N.String meth) ->
              N.Smethod_id (Env.type_name env (pc, cl) ~allow_typedef:false, (pm, meth))
            | (_, N.Id (_, const)), (pm, N.String meth)
              when const = SN.PseudoConsts.g__CLASS__  ->
              (* All of these that use current_cls aren't quite correct
               * inside a trait, as the class should be the using class.
               * It's sufficient for typechecking purposes (we require
               * subclass to be compatible with the trait member/method
               * declarations).
               *)
              (match (fst env).current_cls with
                | Some (cid, _) -> N.Smethod_id (cid, (pm, meth))
                | None -> Errors.illegal_class_meth p; N.Any)
            | (_, N.Class_const ((_, N.CI (cl, _)), (_, mem))), (pm, N.String meth)
              when mem = SN.Members.mClass ->
              N.Smethod_id (Env.type_name env cl ~allow_typedef:false, (pm, meth))
            | (p, N.Class_const ((_, (N.CIself|N.CIstatic)), (_, mem))),
                (pm, N.String meth) when mem = SN.Members.mClass ->
              (match (fst env).current_cls with
                | Some (cid, _) -> N.Smethod_id (cid, (pm, meth))
                | None -> Errors.illegal_class_meth p; N.Any)
            | (p, _), (_) -> Errors.illegal_class_meth p; N.Any
            )
        | _ -> Errors.naming_too_many_arguments p; N.Any
        )
    | Call ((p, Id (_, cn)), _, el, uel) when cn = SN.SpecialFunctions.assert_ ->
        arg_unpack_unexpected uel ;
        if List.length el <> 1
        then Errors.assert_arity p;
        N.Assert (N.AE_assert (
          Option.value_map (List.hd el) ~default:(p, N.Any) ~f:(expr env)
        ))
    | Call ((p, Id (_, cn)), _, el, uel) when cn = SN.SpecialFunctions.tuple ->
        arg_unpack_unexpected uel ;
        (match el with
        | [] -> Errors.naming_too_few_arguments p; N.Any
        | el -> N.List (exprl env el)
        )
    (* sample, factor, observe, condition *)
    | Call ((p1, Id (p2, cn)), hl, el, uel)
      when Env.in_ppl env && SN.PPLFunctions.is_reserved cn ->
        let n_expr = N.Id (p2, cn) in
        N.Call (N.Cnormal, (p1, n_expr),
                hintl_funcall env p hl, exprl env el, exprl env uel)
    | Call ((p, Id f), hl, el, uel) ->
      begin match Env.let_local env f with
      | Some x ->
        (* Translate into local id *)
        let f = (p, N.ImmutableVar x) in
        N.Call (N.Cnormal, f, hintl_funcall env p hl, exprl env el, exprl env uel)
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
        if cn = SN.FB.fgena then begin
          arg_unpack_unexpected uel ;
          (match el with
          | [e] -> N.Special_func (N.Gena (expr env e))
          | _ -> Errors.gena_arity p; N.Any
          )
        end else if (cn = SN.FB.fgenva)
                 || (cn = SN.HH.asio_va)
                 || (cn = SN.HH.lib_tuple_gen)
                 || (cn = SN.HH.lib_tuple_from_async) then begin
          arg_unpack_unexpected uel ;
          if List.length el < 1
          then (Errors.genva_arity p; N.Any)
          else N.Special_func (N.Genva (exprl env el))
        end else if cn = SN.FB.fgen_array_rec then begin
          arg_unpack_unexpected uel ;
          (match el with
          | [e] -> N.Special_func (N.Gen_array_rec (expr env e))
          | _ -> Errors.gen_array_rec_arity p; N.Any
          )
        end else
          N.Call (N.Cnormal, (p, N.Id qualified), hintl_funcall env p hl,
                  exprl env el, exprl env uel)
      end (* match *)
    (* Handle nullsafe instance method calls here. Because Obj_get is used
       for both instance property access and instance method calls, we need
       to match the entire "Call(Obj_get(..), ..)" pattern here so that we
       only match instance method calls *)
    | Call ((p, Obj_get (e1, e2, OG_nullsafe)), hl, el, uel) ->
        N.Call
          (N.Cnormal,
           (p, N.Obj_get (expr env e1,
              expr_obj_get_name env e2, N.OG_nullsafe)),
           hintl_funcall env p hl,
           exprl env el, exprl env uel)
    (* Handle all kinds of calls that weren't handled by any of
       the cases above *)
    | Call (e, hl, el, uel) ->
        N.Call (N.Cnormal, expr env e,
                hintl_funcall env p hl, exprl env el, exprl env uel)
    | Yield_break -> N.Yield_break
    | Yield e -> N.Yield (afield env e)
    | Await e -> N.Await (expr env e)
    | Suspend e -> N.Suspend (expr env e)
    | List el -> N.List (exprl env el)
    | Expr_list el -> N.Expr_list (exprl env el)
    | Cast (ty, e2) ->
        let (p, x), hl = match ty with
        | _, Happly (id, hl) -> (id, hl)
        | _                  -> assert false in
        let ty = match try_castable_hint ~tp_depth:1 env p x hl with
        | Some ty -> p, ty
        | None    -> begin
        match x with
        | x when x = SN.Typehints.object_cast ->
            (* (object) is a valid cast but not a valid type annotation *)
            (* FIXME we are not modeling the correct runtime behavior here --
             * the runtime result type is an stdClass if the original type is
             * primitive. But we should probably just disallow object casts
             * altogether. *)
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
            let h = hint ~allow_typedef:false env ty in
            Errors.object_cast p x;
            h
        end in
        N.Cast (ty, expr env e2)
    | Unop (uop, e) -> N.Unop (uop, expr env e)
    | Binop (Eq None as op, lv, e2) ->
        if Env.inside_pipe env then
          Errors.unimplemented_feature p "Assignment within pipe expressions";
        let e2 = expr env e2 in
        let nsenv = (fst env).namespace in
        let _, vars =
          GetLocals.lvalue (fst env).tcopt (nsenv, SMap.empty) lv in
        SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
        N.Binop (op, expr env lv, e2)
    | Binop (Eq _ as bop, e1, e2) ->
        if Env.inside_pipe env then
          Errors.unimplemented_feature p "Assignment within pipe expressions";
        let e1 = expr env e1 in
        N.Binop (bop, e1, expr env e2)
    | Binop (bop, e1, e2) ->
        let e1 = expr env e1 in
        N.Binop (bop, e1, expr env e2)
    | Pipe (e1, e2) ->
      let e1 = expr env e1 in
      let ident, e2 = Env.pipe_scope env
        begin fun env ->
          expr env e2
        end
      in
      N.Pipe ((p, ident), e1, e2)
    | Eif (e1, e2opt, e3) ->
        (* The order matters here, of course -- e1 can define vars that need to
         * be available in e2 and e3. *)
        let inject_dollardollar env (p, dd) =
          Env.add_lvar env (p, SN.SpecialIdents.dollardollar) (p, dd)
        in
        let e1 = expr env e1 in
        let nsenv = (fst env).namespace in
        let get_lvalues = function e ->
          snd @@ GetLocals.stmt (fst env).tcopt (nsenv, SMap.empty) (p, Expr e) in
        let e2_lvalues =
          Option.value (Option.map e2opt get_lvalues) ~default:SMap.empty
        in
        let e3_lvalues = get_lvalues e3 in
        let lvalues = smap_inter e2_lvalues e3_lvalues in
        SMap.iter (fun x p -> Env.new_pending_lvar env (p, x)) lvalues;
        let e2opt, e3, dollar_dollar = Env.scope env (fun env ->
          let all2, (e2opt, dollardollar_e2) = Env.scope_all env (fun env ->
            let e2opt = oexpr env e2opt in
            e2opt, Env.get_dollardollar env) in
          (* If $$ is found in e2, we inject the $$ in e3 to prevent multiple
           * local_id for the same $$ variable *)
          let all3, (e3, dollardollar_e3) = Env.scope_all env (fun env ->
            begin match dollardollar_e2 with
            | Some dollardollar -> inject_dollardollar env dollardollar
            | None -> ()
            end;
            let e3 = expr env e3 in
            e3, Env.get_dollardollar env) in
          Env.extend_all_locals env all2;
          Env.extend_all_locals env all3;
          e2opt, e3, Option.first_some dollardollar_e2 dollardollar_e3
        ) in
        SMap.iter (fun x _ -> Env.promote_pending_lvar env x) lvalues;
        if Env.inside_pipe env then
          (* Similarly, inject the $$ variable to the current environment *)
          Option.iter dollar_dollar (inject_dollardollar env);
        N.Eif (e1, e2opt, e3)
    | InstanceOf (e, (p, Id x)) ->
      let id = match x with
        | px, n when n = SN.Classes.cParent ->
          if (fst env).current_cls = None then
            let () = Errors.parent_outside_class p in
            N.CI ((px, SN.Classes.cUnknown), [])
          else N.CIparent
        | px, n when n = SN.Classes.cSelf ->
          if (fst env).current_cls = None then
            let () = Errors.self_outside_class p in
            N.CI ((px, SN.Classes.cUnknown), [])
          else N.CIself
        | px, n when n = SN.Classes.cStatic ->
          if (fst env).current_cls = None then
            let () = Errors.static_outside_class p in
            N.CI ((px, SN.Classes.cUnknown), [])
          else N.CIstatic
        | _ ->
          N.CI (Env.type_name env x ~allow_typedef:false, [])
      in
      N.InstanceOf (expr env e, (p, id))
    | InstanceOf (e1, (_,
        (Lvar _ | Obj_get _ | Class_get _ | Class_const _
        | Array_get _ | Call _) as e2)) ->
      N.InstanceOf (expr env e1, (fst e2, N.CIexpr (expr env e2)))
    | InstanceOf (_e1, (p, _)) ->
      Errors.invalid_instanceof p;
      N.Any
    | Is (e, h) ->
      let e1 = expr env e in
      let h1 = hint ~allow_wildcard:true env h in
      N.Is (e1, h1)
    | As (e, h, b) ->
      let e1 = expr env e in
      let h1 = hint ~allow_wildcard:true env h in
      N.As (e1, h1, b)
    | New ((_, Id x), hl, el, uel)
    | New ((_, Lvar x), hl, el, uel) ->
      let hl = extract_hintl_from_type_args env p hl in
      N.New (make_class_id env x hl,
        exprl env el,
        exprl env uel)
    | New ((p, _e), hl, el, uel) ->
      let hl = extract_hintl_from_type_args env p hl in
      if (fst env).in_mode = FileInfo.Mstrict
      then Errors.dynamic_new_in_strict_mode p;
      N.New (make_class_id env (p, SN.Classes.cUnknown) hl,
        exprl env el,
        exprl env uel)
    | NewAnonClass _ ->
      N.Null
    | Efun (f, idl) ->
        let _ = match (fst env).in_mode with
        | FileInfo.Mstrict -> List.iter idl ~f:(function
          | (p, _), is_ref when is_ref -> Errors.reference_in_strict_mode p;
          | _ -> ())
        | _ -> () in
        let idl =
          List.fold_right idl
            ~init:[]
            ~f:(fun ((p, x) as id, _) acc ->
               if x = SN.SpecialIdents.this
               then (Errors.this_as_lexical_variable p; acc)
               else id :: acc)
        in
        let idl' = List.map idl (Env.lvar env) in
        let env = (fst env, Env.empty_local UBMErr) in
        List.iter2_exn idl idl' (Env.add_lvar env);
        let f = expr_lambda env f in
        N.Efun (f, idl')
    | Lfun f ->
      (* We have to build the capture list while we're finding names in
         the closure body---accumulate it in to_capture. *)
      (* semantic duplication: The logic here is also used in `uselist_lambda`.
         The differences are enough that it does not make sense to refactor
         this out for now. *)
      let to_capture = ref [] in
      let handle_unbound (p, x) =
        let cap = Env.lvar env (p, x) in
        to_capture := cap :: !to_capture;
        cap
      in
      let lenv = Env.empty_local @@ UBMFunc handle_unbound in
      (* Extend the current let binding into the scope of lambda *)
      Env.copy_let_locals env (fst env, lenv);
      let env = (fst env, lenv) in
      let f = expr_lambda env f in
      (* TODO T28711692: Compute the correct capture list for let variables,
       * it does not seem to affect typechecking... *)
      N.Efun (f, !to_capture)
    | Xml (x, al, el) ->
      N.Xml (Env.type_name env x ~allow_typedef:false, attrl env al,
        exprl env el)
    | Shape fdl ->
      N.Shape begin List.fold_left fdl ~init:ShapeMap.empty
        ~f:begin fun fdm (pname, value) ->
          let pos, name = convert_shape_name env pname in
          if ShapeMap.mem name fdm
          then Errors.fd_name_already_bound pos;
          ShapeMap.add name (expr env value) fdm
        end
      end
    | Unsafeexpr e ->
      N.Unsafe_expr (Errors.ignore_ (fun () -> expr env e))
    | BracedExpr _ ->
      N.Any
    | Dollar _ ->
      Errors.variable_variables_disallowed p;
      N.Any
    | Yield_from e ->
      N.Yield_from (expr env e)
    | Import _ ->
      N.Any
    | Omitted ->
      N.Any
    | Callconv (kind, e) ->
      N.Callconv (kind, expr env e)

  and expr_lambda env f =
    let env = Env.set_ppl env false in
    let h = Option.map f.f_ret (hint ~allow_retonly:true env) in
    let previous_unsafe = Env.has_unsafe env in
    (* save unsafe and yield state *)
    Env.set_unsafe env false;
    let variadicity, paraml = fun_paraml env f.f_params in
    let f_kind = f.f_fun_kind in
    (* The bodies of lambdas go through naming in the containing local
     * environment *)
    let body_nast = block env f.f_body in
    let unsafe = func_body_had_unsafe env in
    (* restore unsafe state *)
    Env.set_unsafe env previous_unsafe;
    let body = N.NamedBody {
      N.fnb_unsafe = unsafe;
      fnb_nast = body_nast;
    } in {
      N.f_annotation = ();
      f_mode = (fst env).in_mode;
      f_ret = h;
      f_name = f.f_name;
      f_params = paraml;
      f_tparams = [];
      f_where_constraints = [];
      f_body = body;
      f_fun_kind = f_kind;
      f_variadic = variadicity;
      f_user_attributes = user_attributes env f.f_user_attributes;
      f_ret_by_ref = f.f_ret_by_ref;
      f_external = f.f_external;
    }

  and make_class_id env (p, x as cid) hl =
    p,
    match x with
      | x when x = SN.Classes.cParent ->
        if (fst env).current_cls = None then
          let () = Errors.parent_outside_class p in
          N.CI ((p, SN.Classes.cUnknown), [])
        else N.CIparent
      | x when x = SN.Classes.cSelf ->
        if (fst env).current_cls = None then
          let () = Errors.self_outside_class p in
          N.CI ((p, SN.Classes.cUnknown), [])
        else N.CIself
      | x when x = SN.Classes.cStatic -> if (fst env).current_cls = None then
          let () = Errors.static_outside_class p in
          N.CI ((p, SN.Classes.cUnknown), [])
        else N.CIstatic
      | x when x = SN.SpecialIdents.this -> N.CIexpr (p, N.This)
      | x when x = SN.SpecialIdents.dollardollar ->
          (* We won't reach here for "new $$" because the parser creates a
           * proper Ast.Dollardollar node, so make_class_id won't be called with
           * that node. In fact, the parser creates an Ast.Dollardollar for all
           * "$$" except in positions where a classname is expected, like in
           * static member access. So, we only reach here for things
           * like "$$::someMethod()". *)
          N.CIexpr(p, N.Lvar (Env.found_dollardollar env p))
      | x when x.[0] = '$' -> N.CIexpr (p, N.Lvar (Env.lvar env cid))
      | _ -> N.CI (Env.type_name env cid ~allow_typedef:false,
        hintl ~allow_wildcard:true ~forbid_this:false
          ~allow_typedef:true ~allow_retonly:true ~tp_depth:1 env hl
        )

  and casel env l =
    List.map_env [] l (case env)

  and case env acc = function
    | Default b ->
      let all_locals, b = branch ~after_unsafe:N.Fallthrough env b in
      all_locals :: acc, N.Default b
    | Case (e, b) ->
      let e = expr env e in
      let all_locals, b = branch ~after_unsafe:N.Fallthrough env b in
      all_locals :: acc, N.Case (e, b)

  and catchl env l = List.map_env [] l (catch env)
  and catch env acc (x1, x2, b) =
    Env.scope env (
    fun env ->
      (* If the variable does not begin with $, it is an immutable binding *)
      let x2 = if (snd x2) <> ""
        && (snd x2).[0] = '$' (* This is always true if not in experimental mode *)
        then Env.new_lvar env x2
        else Env.new_let_local env x2
      in
      let all_locals, b = branch env b in
      all_locals :: acc, (Env.type_name env x1 ~allow_typedef:true, x2, b)
    )

  and afield env = function
    | AFvalue e -> N.AFvalue (expr env e)
    | AFkvalue (e1, e2) -> N.AFkvalue (expr env e1, expr env e2)

  and afield_value env cname = function
    | AFvalue e -> expr env e
    | AFkvalue (e1, _e2) ->
      Errors.unexpected_arrow (fst e1) cname;
      expr env e1

  and afield_kvalue env cname = function
    | AFvalue e ->
      Errors.missing_arrow (fst e) cname;
      expr env e, expr env (fst e, Lvar (fst e, "__internal_placeholder"))
    | AFkvalue (e1, e2) -> expr env e1, expr env e2

  and attrl env l = List.map l (attr env)
  and attr env = function
    | Xhp_simple (x, e) -> N.Xhp_simple (x, expr env e)
    | Xhp_spread e -> N.Xhp_spread (expr env e)

  and string2 env idl =
    List.map idl (expr env)

  (**************************************************************************)
  (* Function/Method Body Naming: *)
  (* Ensure that, given a function / class, any UnnamedBody within is
   * transformed into a a named body *)
  (**************************************************************************)

  let func_body nenv f =
    match f.N.f_body with
      | N.NamedBody b -> b
      | N.UnnamedBody { N.fub_ast; N.fub_tparams; N.fub_namespace; _ } ->
        let genv = Env.make_fun_genv nenv
          SMap.empty f.N.f_mode (snd f.N.f_name) fub_namespace in
        let genv = extend_params genv fub_tparams in
        let lenv = Env.empty_local UBMErr in
        let env = genv, lenv in
        let env =
          List.fold_left ~f:Env.add_param f.N.f_params ~init:env in
        let env = match f.N.f_variadic with
          | N.FVellipsis _ | N.FVnonVariadic -> env
          | N.FVvariadicArg param -> Env.add_param env param
        in
        let body = block env fub_ast in
        let unsafe = func_body_had_unsafe env in
        Env.check_goto_references env;
        {
          N.fnb_nast = body;
          fnb_unsafe = unsafe;
        }

  let meth_body genv m =
    let named_body = (match m.N.m_body with
      | N.NamedBody _ as b -> b
      | N.UnnamedBody {N.fub_ast; N.fub_tparams; N.fub_namespace; _} ->
        let genv = {genv with namespace = fub_namespace} in
        let genv = extend_params genv fub_tparams in
        let env = genv, Env.empty_local UBMErr in
        let env =
          List.fold_left ~f:Env.add_param m.N.m_params ~init:env in
        let env = match m.N.m_variadic with
          | N.FVellipsis _ | N.FVnonVariadic -> env
          | N.FVvariadicArg param -> Env.add_param env param
        in
        let body = block env fub_ast in
        let unsafe = func_body_had_unsafe env in
        Env.check_goto_references env;
        N.NamedBody {
          N.fnb_nast = body;
          fnb_unsafe = unsafe;
        }
    ) in
    {m with N.m_body = named_body}

  let class_meth_bodies nenv nc =
    let _n_tparams, cstrs = nc.N.c_tparams in
    let genv  = Env.make_class_genv nenv cstrs
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

  let typedef genv tdef =
    let ty = match tdef.t_kind with Alias t | NewType t -> t in
    let cstrs = make_constraints tdef.t_tparams in
    let env = Env.make_typedef_env genv cstrs tdef in
    let tconstraint = Option.map tdef.t_constraint (hint env) in
    List.iter tdef.t_tparams check_constraint;
    let tparaml = type_paraml env tdef.t_tparams in
    let t_vis = match tdef.t_kind with
      | Ast.Alias _ -> N.Transparent
      | Ast.NewType _ -> N.Opaque
    in
    let attrs = user_attributes env tdef.t_user_attributes in
    {
      N.t_annotation = ();
      t_name = tdef.t_id;
      t_tparams = tparaml;
      t_constraint = tconstraint;
      t_kind = hint env ty;
      t_user_attributes = attrs;
      t_mode = tdef.t_mode;
      t_vis;
    }

  (**************************************************************************)
  (* Global constants *)
  (**************************************************************************)

  let check_constant_hint cst =
    match cst.cst_type with
    | None when cst.cst_mode = FileInfo.Mstrict ->
        Errors.add_a_typehint (fst cst.cst_name)
    | None
    | Some _ -> ()

  let global_const genv cst =
    let env = Env.make_const_env genv cst in
    let hint = Option.map cst.cst_type (hint env) in
    let e = match cst.cst_kind with
    | Ast.Cst_const ->
      check_constant_hint cst;
      Some (constant_expr env cst.cst_value)
    (* Define allows any expression, so don't call check_constant.
     * Furthermore it often appears at toplevel, which we don't track at
     * all, so don't type or even name that expression, it may refer to
     * "undefined" variables that actually exist, just untracked since
     * they're toplevel. *)
    | Ast.Cst_define -> None in
    { N.cst_annotation = ();
      cst_mode = cst.cst_mode;
      cst_name = cst.cst_name;
      cst_type = hint;
      cst_value = e;
      cst_is_define = (cst.cst_kind = Ast.Cst_define);
    }

  (* Uses a default empty environment to extract the use list
    of a lambda expression. This exists only for the sake of
    the dehackificator and is not meant for general use. *)
  let uselist_lambda f =
    (* semantic duplication: This is copied from the implementation of the
      `Lfun` variant of `expr_` defined earlier in this file. *)
    let to_capture = ref [] in
    let handle_unbound (p, x) =
      to_capture := x :: !to_capture;
      p, Local_id.tmp()
    in
    let tcopt = TypecheckerOptions.make_permissive TypecheckerOptions.default in
    let genv = Env.make_fun_decl_genv tcopt SMap.empty f in
    let lenv = Env.empty_local @@ UBMFunc handle_unbound in
    let env = genv, lenv in
    ignore (expr_lambda env f);
    List.dedup_and_sort !to_capture ~compare:String.compare

  (**************************************************************************)
  (* The entry point to CHECK the program, and transform the program *)
  (**************************************************************************)

  let program tcopt ast =
    let rec program ast =
    List.concat @@ List.map ast begin function
    | Ast.Fun f -> [N.Fun (fun_ tcopt f)]
    | Ast.Class c -> [N.Class (class_ tcopt c)]
    | Ast.Typedef t -> [N.Typedef (typedef tcopt t)]
    | Ast.Constant cst -> [N.Constant (global_const tcopt cst)]
    | Ast.Stmt _ -> []
    | Ast.Namespace (_ns, ast) -> program ast
    | Ast.NamespaceUse _ -> []
    | Ast.SetNamespaceEnv _ -> []
  end in program ast
end

include Make(struct
  let stmt _ acc _ = acc
  let lvalue _ acc _ = acc
end)
