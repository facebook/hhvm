(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(** Module "naming" a program.
 *
 * The naming phase consists in several things
 * 1- get all the global names
 * 2- transform all the local names into a unique identifier
 *)
open Utils
open Ast

module N = Nast
module Error = Naming_error

(*****************************************************************************)
(* The types *)
(*****************************************************************************)

type fun_set = Utils.SSet.t
type class_set = Utils.SSet.t
type typedef_set = Utils.SSet.t
type const_set = Utils.SSet.t
type decl_set = fun_set * class_set * typedef_set * const_set

type class_cache = Nast.class_ option Utils.SMap.t ref

(* We want to keep the positions of names that have been
 * replaced by identifiers.
 *)
type positioned_ident = (Pos.t * Ident.t)
type map = positioned_ident SMap.t

(* <T as A>, A is a type constraint *)
type type_constraint = hint option

type genv = {
    (* strict? decl? partial? *)
    in_mode: Ast.mode;

    (* are we in the body of a try statement? *)
    in_try: bool;

    (* are we in the body of a non-static member function? *)
    in_member_fun: bool;

    (* In function foo<T1, ..., Tn> or class<T1, ..., Tn>
       The field type_params knows T1 .. Tn. It is able to
       find out about the constraint on these paramters.
     *)
    type_params: type_constraint SMap.t;

    (* The parameters is their original order
     * Necessary to type "this".
     *)
    type_paraml: Ast.id list;

    (* Set of class names defined, and their position *)
    classes: map ref;

    (* Set of function names defined, and their positions *)
    funs: map ref;

    (* Set of typedef names defined, and their position *)
    typedefs: map ref;

    (* Set of constant names defined, and their position *)
    gconsts: map ref;

    (* The name of the current class, None if we are in a function *)
    cclass: Ast.id option;

    (* Normally we don't need to add dependencies at this stage, but there
     * are edge cases when we do.  *)
    droot: Typing_deps.Dep.variant option;

    (* Namespace environment, e.g., what namespace we're in and what use
     * declarations are in play. *)
    namespace: Namespace_env.env;
  }

(* How to behave when we see an unbound name.  Either we raise an
   error, or we call a function first and continue if it can resolve
   the name.  This is used to nest environments when processing
   closures. *)
type unbound_mode =
  | UBMErr
  | UBMFunc of ((Pos.t * string) -> positioned_ident)

(* The local environment *)
type lenv = {

    (* The set of locals *)
    locals: map ref;

    (* The set of constants *)
    consts: map ref;

    (* A map of variable names to a list of previous references.
       Only used in find refs mode *)
    references: (Pos.t list) SMap.t ref;

    (* Variable name of the target we're finding references for,
       if we've found it *)
    find_refs_target_name: string option ref;

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
    all_locals: Pos.t SMap.t ref;

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

    (* Tag controlling what we do when we encounter an unbound name.
     * This is used when processing a lambda expression body that has
     * an automatic use list.
     *
     * See expr_lambda for details.
     *)
    unbound_mode: unbound_mode;
  }

(* The environment VISIBLE to the outside world. *)
type env = {
    iclasses: map;
    ifuns: map;
    itypedefs: map;
    iconsts: map;
  }

(**
 * Returns the list of classes which have been seen.
 * Useful for things like dumping json formatted information about the www
 * world.
 *)
let get_classes env =
  SMap.fold (fun key _ acc -> key :: acc) env.iclasses []

(*****************************************************************************)
(* Predefined names *)
(*****************************************************************************)

let predef_funs = ref SMap.empty
let predef_fun x =
  let var = Pos.none, Ident.make x in
  predef_funs := SMap.add x var !predef_funs;
  x

let anon      = predef_fun "?anon"
let is_int    = predef_fun "is_int"
let is_bool   = predef_fun "is_bool"
let is_array  = predef_fun "is_array"
let is_float  = predef_fun "is_float"
let is_string = predef_fun "is_string"
let is_null   = predef_fun "is_null"
let is_resource = predef_fun "is_resource"

let predef_tests_list =
  [is_int; is_bool; is_float; is_string; is_null; is_array; is_resource]
let predef_tests = List.fold_right SSet.add predef_tests_list SSet.empty

(*****************************************************************************)
(* Empty (initial) environments *)
(*****************************************************************************)

let empty = {
  iclasses  = SMap.empty;
  ifuns     = !predef_funs;
  itypedefs = SMap.empty;
  iconsts   = SMap.empty;
}

(* The primitives to manipulate the naming environment *)
module Env = struct

  let empty_local() = {
    locals     = ref SMap.empty;
    consts     = ref SMap.empty;
    all_locals = ref SMap.empty;
    references = ref SMap.empty;
    pending_locals = ref SMap.empty;
    find_refs_target_name = ref None;
    unbound_mode = UBMErr;
  }

  let empty_global env = {
    in_mode       = Ast.Mstrict;
    in_try        = false;
    in_member_fun = false;
    type_params   = SMap.empty;
    type_paraml   = [];
    classes       = ref env.iclasses;
    funs          = ref env.ifuns;
    typedefs      = ref env.itypedefs;
    gconsts       = ref env.iconsts;
    cclass        = None;
    droot         = None;
    namespace     = Namespace_env.empty;
  }

  let make_class_genv genv params c = {
    in_mode       =
      (if !Autocomplete.auto_complete then Ast.Mpartial else c.c_mode);
    in_try        = false;
    in_member_fun = false;
    type_params   = params;
    type_paraml   = List.map fst c.c_tparams;
    classes       = ref genv.iclasses;
    funs          = ref genv.ifuns;
    typedefs      = ref genv.itypedefs;
    gconsts       = ref genv.iconsts;
    cclass        = Some c.c_name;
    droot         = Some (Typing_deps.Dep.Class (snd c.c_name));
    namespace     = c.c_namespace;
  }

  let make_class_env genv params c =
    let genv = make_class_genv genv params c in
    let lenv = empty_local () in
    let env  = genv, lenv in
    env

  let make_typedef_genv genv cstrs tdef = {
    in_mode       = (if !Ide.is_ide_mode then Ast.Mpartial else Ast.Mstrict);
    in_try        = false;
    in_member_fun = false;
    type_params   = cstrs;
    type_paraml   = List.map fst tdef.t_tparams;
    classes       = ref genv.iclasses;
    funs          = ref genv.ifuns;
    typedefs      = ref genv.itypedefs;
    gconsts       = ref genv.iconsts;
    cclass        = None;
    droot         = None;
    namespace     = tdef.t_namespace;
  }

  let make_typedef_env genv cstrs tdef =
    let genv = make_typedef_genv genv cstrs tdef in
    let lenv = empty_local () in
    let env  = genv, lenv in
    env

  let make_fun_genv genv params f = {
    in_mode       = f.f_mode;
    in_try        = false;
    in_member_fun = false;
    type_params   = params;
    type_paraml   = [];
    classes       = ref genv.iclasses;
    funs          = ref genv.ifuns;
    typedefs      = ref genv.itypedefs;
    gconsts       = ref genv.iconsts;
    cclass        = None;
    droot         = Some (Typing_deps.Dep.Fun (snd f.f_name));
    namespace     = f.f_namespace;
  }

  let make_const_genv genv cst = {
    in_mode       = cst.cst_mode;
    in_try        = false;
    in_member_fun = false;
    type_params   = SMap.empty;
    type_paraml   = [];
    classes       = ref genv.iclasses;
    funs          = ref genv.ifuns;
    typedefs      = ref genv.itypedefs;
    gconsts       = ref genv.iconsts;
    cclass        = None;
    droot         = Some (Typing_deps.Dep.GConst (snd cst.cst_name));
    namespace     = cst.cst_namespace;
  }

  let make_const_env genv cst =
    let genv = make_const_genv genv cst in
    let lenv = empty_local () in
    let env  = genv, lenv in
    env

(* Helper used to write in different environments
 * consts, fun_names, class_names
 *)
  let new_var env (p, x) =
    if SMap.mem x !env && not !Silent.is_silent_mode
    then
      let p', y = SMap.find_unsafe x !env in
      error_l [p, "Name already bound: "^x;
               p', "Previous definition is here"]
    else
      let y = p, Ident.make x in
      env := SMap.add x y !env;
      y

  let var env (p, x) =
    let v = SMap.get x !env in
    match v with
    | None   -> error p ("Unbound name: "^x)
    | Some v -> p, snd v

(* Is called bad_style, but it is still an error ... Whatever *)
  let bad_style env (p, x) =
    let p' = SMap.get x !(env.all_locals) in
    match p' with None -> assert false | Some p' ->
      error_l [p, ("The variable "^ x ^" is defined");
               p', ("But in a different scope")]

  let is_superglobal =
    let l = [
      "$GLOBALS"; "$_SERVER"; "$_GET"; "$_POST"; "$_FILES";
      "$_COOKIE"; "$_SESSION"; "$_REQUEST"; "$_ENV"
    ] in
    let h = Hashtbl.create 23 in
    List.iter (fun x -> Hashtbl.add h x true) l;
    fun x -> Hashtbl.mem h x

(* Adds a local variable, without any check *)
  let add_lvar (_, lenv) (_, name) (p, x) =
    lenv.locals := SMap.add name (p, x) !(lenv.locals)

(* Saves the position of local variables if we're in find refs mode*)
  let save_ref x p lenv =
    Find_refs.process_var_ref p x;
    (* If we've already located the target and name of this var is
       the same, add it to the result list *)
    (match !(lenv.find_refs_target_name) with
     | Some target ->
       if target = x then
         Find_refs.find_refs_result := p :: !Find_refs.find_refs_result;
     | None -> ()
    );
    (* If we haven't found the target yet: *)
    match !Find_refs.find_refs_target with
    | None -> ()
    | Some (line, char_pos) ->
        (* store the location of this reference for later *)
        lenv.references := (match SMap.get x !(lenv.references) with
        | None -> SMap.add x (p :: []) !(lenv.references)
        | Some lst -> SMap.add x (p :: lst) !(lenv.references));

        let l, start, end_ = Pos.info_pos p in
        if l = line && start <= char_pos && char_pos <= end_
        then begin
          (* This is the target, so stop looking for it,
             save the target name, and copy the current references
             to this target to the result list *)
          Find_refs.find_refs_target := None;
          lenv.find_refs_target_name := Some x;
          Find_refs.find_refs_result :=
            (match SMap.get x !(lenv.references) with
            | None -> []
            | Some lst -> lst
            );
        end;
    ()

(* Defines a new local variable *)
  let new_lvar (_, lenv) (p, x) =
    let lcl = SMap.get x !(lenv.locals) in
    match lcl with
    | Some lcl -> p, snd lcl
    | None ->
        save_ref x p lenv;
        let ident = match SMap.get x !(lenv.pending_locals) with
          | Some (_, ident) -> ident
          | None -> Ident.make x in
        let y = p, ident in
        lenv.all_locals := SMap.add x p !(lenv.all_locals);
        lenv.locals := SMap.add x y !(lenv.locals);
        y

  let new_pending_lvar (_, lenv) (p, x) =
    match SMap.get x !(lenv.locals), SMap.get x !(lenv.pending_locals) with
    | None, None ->
        let y = p, Ident.make x in
        lenv.pending_locals := SMap.add x y !(lenv.pending_locals)
    | _ -> ()

  let promote_pending (_, lenv as env) =
    SMap.iter begin fun x (p, ident) ->
      add_lvar env (p, x) (p, ident)
    end !(lenv.pending_locals);
    lenv.pending_locals := SMap.empty

  let handle_undefined_variable (genv, env) (p, x) =
    match env.unbound_mode with
    | UBMErr -> error p ("Undefined variable: "^x)
    | UBMFunc f -> f (p, x)

(* Function used to name a local variable *)
  let lvar (genv, env) (p, x) =
    if is_superglobal x && genv.in_mode = Ast.Mpartial
    then p, Ident.tmp()
    else
      let lcl = SMap.get x !(env.locals) in
      match lcl with
      | Some lcl -> (if fst lcl != p then save_ref x p env); p, snd lcl
      | None when not !Autocomplete.auto_complete && not !Silent.is_silent_mode ->
          if SMap.mem x !(env.all_locals)
          then bad_style env (p, x)
          else begin
            handle_undefined_variable (genv, env) (p, x)
          end
      | None -> p, Ident.tmp()

  let resolve_namespace genv x =
    (* Resolve an id to a fully-qualified name. Things in the global namespace
     * aren't prefixed with a slash so that we don't gum up error message for
     * folks who don't use namespaces. So we have to manually remove the
     * leading slash from a fully-qualified name if it's referring to a name in
     * the global namespace. This logic is shared between classes and functions
     * and so does not deal with the function name fallback crap. *)
    let p, id = Namespaces.elaborate_id genv.namespace x in
    let fq_global = try String.rindex id '\\' = 0 with Not_found -> false in
    let id =
      if fq_global then String.sub id 1 (String.length id - 1) else id in
    p, id

  let get_name genv namespace x =
    try ignore (var namespace x); x with exn ->
      match genv.in_mode with
      | Ast.Mstrict -> raise exn
      | Ast.Mdecl | Ast.Mpartial -> x

  let const (genv, env) x  = get_name genv env.consts x

  let global_const (genv, env) x  =
    let x = resolve_namespace genv x in
    get_name genv genv.gconsts x

  let class_name (genv, _) x =
    let x = resolve_namespace genv x in
    get_name genv genv.classes x

  let fun_id (genv, _) x =
    let fq_x = resolve_namespace genv x in
    let need_fallback =
      (snd fq_x).[0] = '\\' &&
      not (String.contains (snd x) '\\') in
    if need_fallback then begin
      (* Explicitly add dependencies on both of the functions we could be
       * referring to here. Normally naming doesn't have to deal with deps at
       * all -- they are added during typechecking just by the nature of
       * looking up a class or function name. However, we're flattening
       * namespaces here, and the fallback behavior of functions means that we
       * might suddenly be referring to a different function without any
       * change to the callsite at all. Adding both dependencies explicitly
       * captures this action-at-a-distance. Furthermore note that we're adding
       * a special kind of dependency -- not just Dep.Fun, but Dep.FunName.
       * This forces an incremental full redeclaration of this class if either
       * of those names changes, not just a retypecheck -- the name that is
       * referred to here actually changes as a result of the other file, which
       * is stronger than just the need to retypecheck. *)
      Typing_deps.add_idep genv.droot (Typing_deps.Dep.FunName (snd fq_x));
      Typing_deps.add_idep genv.droot (Typing_deps.Dep.FunName (snd x));
      let mem x = SMap.mem (snd x) !(genv.funs) in
      match mem fq_x, mem x with
      (* Found in the current namespace *)
      | true, _ -> get_name genv genv.funs fq_x
      (* Found in the global namespace *)
      | _, true -> get_name genv genv.funs x
      (* Not found. Pick the more specific one to error on. *)
      | false, false -> get_name genv genv.funs fq_x
    end else
      get_name genv genv.funs fq_x

  let new_const (genv, env) x =
    try ignore (new_var env.consts x); x with exn ->
      match genv.in_mode with
      | Ast.Mstrict -> raise exn
      | Ast.Mpartial | Ast.Mdecl -> x

  let resilient_new_var env (p, x) =
    if SMap.mem x !env
    then
      let p', y = SMap.find_unsafe x !env in
      if Pos.compare p p' = 0 then (p, y)
      else if not !Silent.is_silent_mode
      then
        error_l [p, "Name already bound: "^x;
                 p', "Previous definition is here"]
      else
        let y = p, Ident.make x in
        env := SMap.add x y !env;
        y
    else
      let y = p, Ident.make x in
      env := SMap.add x y !env;
      y

  let new_fun_id genv x =
    if SMap.mem (snd x) !predef_funs then () else
    ignore (resilient_new_var genv.funs x)

  let new_class_id genv x =
    ignore (resilient_new_var genv.classes x)

  let new_typedef_id genv x =
    let v = resilient_new_var genv.classes x in
    genv.typedefs := SMap.add (snd x) v !(genv.typedefs);
    ()

  let new_global_const_id genv x =
    let v = resilient_new_var genv.gconsts x in
    genv.gconsts := SMap.add (snd x) v !(genv.gconsts);
    ()

(* Scope, keep the locals, go and name the body, and leave the
 * local environment intact
 *)
  let scope env f =
    let genv, lenv = env in
    let lenv_copy = !(lenv.locals) in
    let lenv_pending_copy = !(lenv.pending_locals) in
    let res = f env in
    lenv.locals := lenv_copy;
    lenv.pending_locals := lenv_pending_copy;
    res

end

(*****************************************************************************)
(* Updating the environment *)
(*****************************************************************************)

let remove_decls env (funs, classes, typedefs, consts) =
  let funs = SSet.diff funs predef_tests in
  let ifuns    = SSet.fold SMap.remove funs env.ifuns       in
  let iclasses = SSet.fold SMap.remove classes env.iclasses in
  let iclasses = SSet.fold SMap.remove typedefs iclasses in
  let itypedefs = SSet.fold SMap.remove typedefs env.itypedefs in
  let iconsts = SSet.fold SMap.remove consts env.iconsts in
  {
   ifuns     = ifuns;
   iclasses  = iclasses;
   itypedefs = itypedefs;
   iconsts   = iconsts;
 }

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

let check_constraint ((pos, name), _) =
  (* TODO refactor this in a seperate module for errors *)
  if String.lowercase name = "this"
  then error pos "The type parameter \"this\" is reserved"
  else if name.[0] <> 'T' then
    error pos
      "Please make your type parameter start with the letter T (capital)"


(* One of the rare cases where I don't work with a functional
 * environment ...
 * normally opt f env x returns env, Some (f x) when x is <> None
 * but in this case I want it to return x
*)
let opt f x = match x with None -> None | Some x -> Some (f x)

let check_repetition s param =
  let x = snd param.param_id in
  if SSet.mem x s && not !Silent.is_silent_mode
  then error (fst param.param_id) ("Argument already bound: "^x)
  else SSet.add x s

(* All classes extend Object (for JS generation) *)
let get_class_parent pos parent_type c =
  if !Ast.is_js && parent_type = [] && c.c_kind = Ast.Cnormal
  then [pos, N.Happly ((pos, "Object"), [])]
  else parent_type

(* Check that the implemented interface exists *)
let implement env x =
  match snd x with
  | Happly (x, _) -> ignore (Env.class_name env x)
  | _ -> ()

(* Check that a name is not a typedef *)
let no_typedef (genv, _) (pos, name) =
  if SMap.mem name !(genv.typedefs)
  then
    let def_pos, _ = SMap.find_unsafe name !(genv.typedefs) in
    error_l [
    pos, "Unexpected typedef";
    def_pos, "Definition is here";
  ]

let hint_no_typedef env = function
  | _, Happly (x, _) -> no_typedef env x
  | _ -> ()

(*****************************************************************************)
(* The entry point to build the naming environment *)
(*****************************************************************************)

let make_env old_env ~funs ~classes ~typedefs ~consts =
  let genv = Env.empty_global old_env in
  List.iter (Env.new_fun_id genv) funs;
  List.iter (Env.new_class_id genv) classes;
  List.iter (Env.new_typedef_id genv) typedefs;
  List.iter (Env.new_global_const_id genv) consts;
  let new_env = {
    iclasses = !(genv.classes);
    ifuns = !(genv.funs);
    itypedefs = !(genv.typedefs);
    iconsts = !(genv.gconsts);
  } in
  new_env

(*****************************************************************************)
(* Naming of type hints *)
(*****************************************************************************)

let rec hint ?(is_static_var=false) ?(allow_this=false) env (p, h) =
  p, hint_ ~allow_this is_static_var p env h

and hint_ ~allow_this is_static_var p env x =
  let hint = hint ~is_static_var ~allow_this in
  match x with
  | Htuple hl -> N.Htuple (List.map (hint env) hl)
  | Hoption h -> N.Hoption (hint env h)
  | Hfun (hl, opt, h) -> N.Hfun (List.map (hint env) hl, opt, hint env h)
  | Happly ((_, x) as id, hl) -> hint_id ~allow_this env is_static_var id hl
  | Hshape fdl -> N.Hshape begin
      List.fold_left begin fun fdm (pname, h) ->
        let pos, name = pname in
        if SMap.mem name fdm
        then error pos "Field name already bound";
        SMap.add name (hint env h) fdm
      end SMap.empty fdl
  end

and hint_id ~allow_this env is_static_var (p, x as id) hl =
  if Autocomplete.is_auto_complete x then (
    Autocomplete.auto_complete_for_global := x;
    Autocomplete.argument_global_type := Some Autocomplete.Actype);
  let hint = hint ~allow_this in
  let params = (fst env).type_params in
  if   is_alok_type_name id && not (SMap.mem x params)
  then Error.alok id;
  if   is_static_var && SMap.mem x params
  then Error.generic_class_var id;
  (* some common Xhp screw ups *)
  if   (x = "Xhp") || (x = ":Xhp") || (x = "XHP")
  then Error.disallowed_xhp_type (p, x);
  match x with
  | "\\void"
  | "\\int"
  | "\\bool"
  | "\\float"
  | "\\num"
  | "\\string"
  | "\\resource"
  | "\\mixed"
  | "\\array"
  | "\\integer"
  | "\\boolean"
  | "\\double"
  | "\\real" ->
      if !Silent.is_silent_mode
      then N.Hany
      else
        error p ("Primitive type annotations are always available and may no"^
                 "longer be referred to in the toplevel namespace.")
  | "void"             -> N.Hprim N.Tvoid
  | "int"              -> N.Hprim N.Tint
  | "bool"             -> N.Hprim N.Tbool
  | "float"            -> N.Hprim N.Tfloat
  | "num"              -> N.Hprim N.Tnum
  | "string"           -> N.Hprim N.Tstring
  | "resource"         -> N.Hprim N.Tresource
  | "mixed"            -> N.Hmixed
  | "array"            ->
      (match hl with
      | [] -> N.Harray (None, None)
      | [x] -> N.Harray (Some (hint env x), None)
      | [x; y] -> N.Harray (Some (hint env x), Some (hint env y))
      | _ -> Error.too_many_args p
      )
  | "integer" ->
      if !Silent.is_silent_mode
      then N.Hprim N.Tint
      else
        error p "Invalid Hack type. Using \"integer\" in Hack is considered \
                 an error. Use \"int\" instead, to keep the codebase \
                 consistent."
  | "boolean" ->
      if !Silent.is_silent_mode
      then N.Hprim N.Tbool
      else
        error p "Invalid Hack type. Using \"boolean\" in Hack is considered \
                 an error. Use \"bool\" instead, to keep the codebase \
                 consistent."
  | "double" ->
      if !Silent.is_silent_mode
      then N.Hprim N.Tfloat
      else
        error p "Invalid Hack type. Using \"double\" in Hack is considered \
         an error. Use \"float\" instead. They are equivalent data types \
         and the codebase remains consistent."
  | "real" ->
      if !Silent.is_silent_mode
      then N.Hprim N.Tfloat
      else error p "Invalid Hack type. Using \"real\" in Hack is considered \
        an error. Use \"float\" instead. They are equivalent data types and \
        the codebase remains consistent."
  | "this" when allow_this ->
      if hl != []
      then error p "\"this\" expects no arguments";
      (match (fst env).cclass with
      | None ->
        error p "Cannot use \"this\" outside of a class";
      | Some cid ->
        let tparaml = (fst env).type_paraml in
        let tparaml = List.map begin fun (param_pos, param_name) ->
          let _, cstr = get_constraint env param_name in
          let cstr =
              match cstr with
              | None -> None
              | Some h -> Some (hint env h)
            in
          param_pos, N.Habstr (param_name, cstr)
        end tparaml in
        N.Habstr ("this", Some (fst cid, N.Happly (cid, tparaml))))
  | "this" ->
      (match (fst env).cclass with
      | None ->
          error p "Cannot use \"this\" outside of a class";
      | Some _ ->
          error p "The type \"this\" can only be used as a return type, \
            to instantiate a covariant type variable, \
            or as a private non-static member variable")
  | _ when String.lowercase x = "this" ->
      error p ("Invalid Hack type \""^x^"\". Use \"this\" instead")
  | _ when SMap.mem x params ->
      if hl <> [] then
      error p (Printf.sprintf "%s is a type parameter. Type parameters cannot \
        themselves take type parameters (e.g. %s<int> doesn't make sense)" x x);
      let env, gen_constraint = get_constraint env x in
      N.Habstr (x, opt (hint env) gen_constraint)
  | _ ->
      (* In the future, when we have proper covariant support, we can
       * allow "this" to instantiate any covariant type variable. For
       * example, let us pretend that we have this defined:
       *
       *   interface IFoo<read Tread, write Twrite>
       *
       * IFoo<this, int> and IFoo<IFoo<this, int>, int> are ok
       * IFoo<int, this> and IFoo<int, IFoo<this>> are not ok
       *
       * For now, we're hardcoding the fact that all type variables for
       * Awaitable and WaitHandle are covariant (well, there's only one
       * type variable, but yeah...). We turn on allow_this in
       * Awaitable and WaitHandle cases to support members that look
       * like:
       *
       *   private ?WaitHandle<this> wh = ...; // e.g. generic preparables
       *)
    let cname = snd (Env.class_name env id) in
    let awaitable_covariance = (cname = "Awaitable" || cname = "WaitHandle") in
    let allow_this = allow_this && awaitable_covariance in
    N.Happly (Env.class_name env id, hintl ~allow_this env hl)

and get_constraint env tparam =
  let params = (fst env).type_params in
  let gen_constraint = SMap.find_unsafe tparam params in
  let genv, lenv = env in
  let genv = { genv with type_params = SMap.add tparam None params } in
  let env = genv, lenv in
  env, gen_constraint

and hintl ~allow_this env l = List.map (hint ~allow_this env) l

(*****************************************************************************)
(* All the methods and static methods of an interface are "implicitely"
 * declared as abstract
 *)
(*****************************************************************************)

let add_abstractl c methods =
  List.map (fun x -> { x with N.m_abstract = true }) methods

let interface c methods smethods =
  if c.c_kind <> Cinterface then methods, smethods else
  let methods  = add_abstractl c methods in
  let smethods = add_abstractl c smethods in
  methods, smethods

(*****************************************************************************)
(* Checking for collision on method names *)
(*****************************************************************************)

let check_method acc { N.m_name = (p, x) } =
  if SSet.mem x acc && not !Silent.is_silent_mode
  then error p "Name already bound"
  else SSet.add x acc

let check_name_collision methods =
  ignore (List.fold_left check_method SSet.empty methods)

(*****************************************************************************)
(* Checking for shadowing of method type parameters *)
(*****************************************************************************)

let shadowed_type_param p (pos, name) = 
  error_l [
    p, Printf.sprintf "You cannot re-bind the type parameter %s" name;
    pos, Printf.sprintf "%s is already bound here" name
  ]

let check_method_tparams class_tparam_names { N.m_tparams = tparams } =
  List.iter
    (fun ((p,x),_) -> List.iter
       (fun (pc,xc) -> if (x = xc) then shadowed_type_param p (pc, x))
       class_tparam_names)
    tparams

let check_tparams_shadow class_tparam_names methods =
  List.iter (check_method_tparams class_tparam_names) methods

(*****************************************************************************)
(* Check if the body of a method/function is UNSAFE *)
(*****************************************************************************)

let rec is_unsafe_body = function
  | [] -> false
  | Block x :: rl -> is_unsafe_body x || is_unsafe_body rl
  | Unsafe :: _ -> true
  | _ :: rl -> is_unsafe_body rl

(*****************************************************************************)
(* The entry point to CHECK the program, and transform the program *)
(*****************************************************************************)

let rec class_constraints genv tparams =
  let cstrs = make_constraints tparams in
  (* Checking there is no cycle in the type constraints *)
  List.iter (Naming_ast_helpers.HintCycle.check_constraint cstrs) tparams;
  cstrs

(* Naming of a class *)
and class_ genv c =
  let cstrs    = class_constraints genv c.c_tparams in
  let env      = Env.make_class_env genv cstrs c in
  class_check env c;
  let name     = Env.class_name env c.c_name in
  let smethods = List.fold_right (class_static_method env) c.c_body [] in
  let svars    = List.fold_right (class_var_static env) c.c_body [] in
  let vars     = List.fold_right (class_var env) c.c_body [] in
  let v_names  = List.map (fun x -> snd x.N.cv_id) vars in
  let v_names  = List.fold_right SSet.add v_names SSet.empty in
  let sm_names = List.map (fun x -> snd x.N.m_name) smethods in
  let sm_names = List.fold_right SSet.add sm_names SSet.empty in
  List.iter (hint_no_typedef env) c.c_extends;
  let parent   = List.map (hint ~allow_this:true env) c.c_extends in
  let parent   = get_class_parent (fst name) parent c in
  let fmethod  = class_method env sm_names v_names in
  let methods  = List.fold_right fmethod c.c_body [] in
  let methods, smethods = interface c methods smethods in
  let uses     = List.fold_right (class_use env) c.c_body [] in
  let req_implements, req_extends = List.fold_right
    (class_require env) c.c_body ([], []) in
  let tparam_l  = type_paraml env c.c_tparams in
  let consts   = List.fold_right (class_const env) c.c_body [] in
  List.iter (hint_no_typedef env) c.c_implements;
  let implements  = List.map (hint ~allow_this:true env) c.c_implements in
  let constructor = List.fold_left (constructor env) None c.c_body in
  let class_tparam_names = List.map (fun (x,_) -> x) c.c_tparams in
  check_name_collision methods;
  check_tparams_shadow class_tparam_names methods;
  check_name_collision smethods;
  check_tparams_shadow class_tparam_names smethods;
  { N.c_mode           = c.c_mode;
    N.c_final          = c.c_final;
    N.c_is_xhp         = c.c_is_xhp;
    N.c_kind           = c.c_kind;
    N.c_name           = name;
    N.c_tparams        = tparam_l;
    N.c_extends        = parent;
    N.c_uses           = uses;
    N.c_req_extends    = req_extends;
    N.c_req_implements = req_implements;
    N.c_implements     = implements;
    N.c_consts         = consts;
    N.c_static_vars    = svars;
    N.c_vars           = vars;
    N.c_constructor    = constructor;
    N.c_static_methods = smethods;
    N.c_methods        = methods;
    N.c_user_attributes = c.c_user_attributes;
    N.c_mtime          = c.c_mtime;
  }

and class_check env c =
  (* Checking all the parents exist *)
  List.iter (implement env) c.c_extends;
  List.iter (implement env) c.c_implements;
  (* Checking for a code smell *)
  List.iter check_constraint c.c_tparams;
  ()

and type_paraml env tparams = 
  let _, ret = List.fold_left 
    (fun (seen, tparaml) (((p, name), _) as tparam) ->
      match SMap.get name seen with
      | None -> (SMap.add name p seen, (type_param env tparam)::tparaml)
      | Some pos -> shadowed_type_param p (pos, name))
    (SMap.empty, [])
    tparams in
  List.rev ret

and type_param env (x, y) = x, opt (hint env) y

and class_use env x acc =
  match x with
  | Attributes _ -> acc
  | Const _ -> acc
  | ClassUse h ->
      hint_no_typedef env h;
      hint ~allow_this:true env h :: acc
  | ClassTraitRequire _ -> acc
  | ClassVars _ -> acc
  | Method _ -> acc

and class_require env x acc =
  match x with
  | Attributes _ -> acc
  | Const _ -> acc
  | ClassUse _ -> acc
  | ClassTraitRequire (MustExtend, h) ->
    hint_no_typedef env h;
    let acc_impls, acc_exts = acc in
    (acc_impls, hint ~allow_this:true env h :: acc_exts)
  | ClassTraitRequire (MustImplement, h) ->
    hint_no_typedef env h;
    let acc_impls, acc_exts = acc in
    (hint ~allow_this:true env h :: acc_impls, acc_exts)
  | ClassVars _ -> acc
  | Method _ -> acc

and constructor env acc = function
  | Attributes _ -> acc
  | Const _ -> acc
  | ClassUse _ -> acc
  | ClassTraitRequire _ -> acc
  | ClassVars _ -> acc
  | Method ({ m_name = (p, name); _ } as m) when name = "__construct" ->
      let genv, lenv = env in
      let env = ({ genv with in_member_fun = true}, lenv) in
      (match acc with
      | None -> Some (method_ env m)
      | Some _ when !Silent.is_silent_mode -> acc
      | Some _ -> error p ("Name already bound: "^name))
  | Method _ -> acc

and class_const env x acc =
  match x with
  | Attributes _ -> acc
  | Const (h, l) -> const_defl h env l @ acc
  | ClassUse _ -> acc
  | ClassTraitRequire _ -> acc
  | ClassVars _ -> acc
  | Method _ -> acc

and class_var_static env x acc =
  match x with
  | Attributes _ -> acc
  | ClassUse _ -> acc
  | ClassTraitRequire _ -> acc
  | Const _ -> acc
  | ClassVars (kl, h, cvl) when List.mem Static kl ->
    let h = opt (hint ~is_static_var:true env) h in
    let cvl = List.map (class_var_ env) cvl in
    let cvl = List.map (fill_cvar kl h) cvl in
    cvl @ acc
  | ClassVars _ -> acc
  | Method _ -> acc

and class_var env x acc =
  match x with
  | Attributes _ -> acc
  | ClassUse _ -> acc
  | ClassTraitRequire _ -> acc
  | Const _ -> acc
  | ClassVars (kl, h, cvl) when not (List.mem Static kl) ->
    (* there are no covariance issues with private members *)
    let allow_this = (List.mem Private kl) in
    let h = opt (hint ~allow_this:allow_this env) h in
    let cvl = List.map (class_var_ env) cvl in
    let cvl = List.map (fill_cvar kl h) cvl in
    cvl @ acc
  | ClassVars _ -> acc
  | Method _ -> acc

and class_static_method env x acc =
  match x with
  | Attributes _ -> acc
  | ClassUse _ -> acc
  | ClassTraitRequire _ -> acc
  | Const _ -> acc
  | ClassVars _ -> acc
  | Method m when snd m.m_name = "__construct" -> acc
  | Method m when List.mem Static m.m_kind -> method_ env m :: acc
  | Method _ -> acc

and class_method env sids cv_ids x acc =
  match x with
  | Attributes _ -> acc
  | ClassUse _ -> acc
  | ClassTraitRequire _ -> acc
  | Const _ -> acc
  | ClassVars _ -> acc
  | Method m when snd m.m_name = "__construct" -> acc
  | Method m when not (List.mem Static m.m_kind) ->
      let genv, lenv = env in
      let env = ({ genv with in_member_fun = true}, lenv) in
      method_ env m :: acc
  | Method _ -> acc

and const_defl h env l = List.map (const_def h env) l
and const_def h env (x, e) =
  match (fst env).in_mode with
  | Ast.Mstrict ->
      (* TODO THIS IS A BUG!!!! You should always try to guess the type of
       * constant, not only in strict mode. What an idiot!
       *)
      (match h with
      | None ->
          (* Whenever the type is "obvious", no need to add a type-hint
             if you add a case here, make sure you add it in the type-checker too
             cf class_const_decl in typing.ml
           *)
          (match snd e with
          | String _
          | String2 ([], _)
          | True
          | False
          | Int _
          | Float _
          | Array _ ->
              None, Env.new_const env x, expr env e
          | _ ->
            error (fst x) "Please add a type hint"
          )
      | Some h ->
          let h = Some (hint env h) in
          h, Env.new_const env x, expr env e)
  | Ast.Mpartial ->
      let h = opt (hint env) h in
      h, Env.new_const env x, expr env e
  | Ast.Mdecl ->
      let h = opt (hint env) h in
      h, Env.new_const env x, (fst e, N.Null)

and class_var_ env (x, e) =
  let id = Env.new_const env x in
  let e =
    match (fst env).in_mode with
    | Ast.Mstrict | Ast.Mpartial -> opt (expr env) e
    (* Consider every member variable defined in a class in decl mode to be
     * initalized by giving it a magic value of type Tany (you can't actually
     * write this cast in PHP). Classes might inherit from our decl mode class
     * that are themselves not in decl, and there's no way to figure out what
     * variables are initalized in a decl class without typechecking its
     * initalizers and constructor, which we don't want to do, so just assume
     * we're covered. *)
    | Ast.Mdecl ->
      let p = match e with
        | None -> fst id
        | Some (p, _) -> p in
      Some (p, N.Cast ((p, N.Hany), (p, N.Null)))
  in
  N.({ cv_final = false;
       cv_visibility = Public;
       cv_type = None;
       cv_id = id;
       cv_expr = e;
     })

and fill_cvar kl ty x =
  let x = { x with N.cv_type = ty } in
  List.fold_left (
  fun x k ->
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
 ) x kl

and method_ env m =
  let genv, lenv = env in
  let lenv = Env.empty_local() in
  let genv = extend_params genv m.m_tparams in
  let env = genv, lenv in
  let has_ddd, paraml = fun_paraml env m.m_params in
  let name = Env.new_const env m.m_name in
  let acc = false, false, N.Public in
  let final, abs, vis = List.fold_left kind acc m.m_kind in
  let unsafe = is_unsafe_body m.m_body in
  List.iter check_constraint m.m_tparams;
  let tparam_l = type_paraml env m.m_tparams in
  let body =
    match genv.in_mode with
    | Ast.Mpartial | Ast.Mstrict ->
        block env m.m_body
    | Ast.Mdecl -> [] in
  let attrs = m.m_user_attributes in
  let method_type = m.m_type in
  let ret = opt (hint ~allow_this:true env) m.m_ret in
  N.({ m_unsafe     = unsafe ;
       m_final      = final  ;
       m_visibility = vis    ;
       m_abstract   = abs    ;
       m_name       = name   ;
       m_tparams    = tparam_l;
       m_params     = paraml   ;
       m_body       = body   ;
       m_user_attributes = attrs;
       m_ret        = ret    ;
       m_ddd        = has_ddd;
       m_type       = method_type;
     })

and kind (final, abs, vis) = function
  | Final -> true, abs, vis
  | Static -> final, abs, vis
  | Abstract -> final, true, vis
  | Private -> final, abs, N.Private
  | Public -> final, abs, N.Public
  | Protected -> final, abs, N.Protected

and fun_paraml env l =
  let has_ddd, l = ddd l in
  let _ = List.fold_left check_repetition SSet.empty l in
  has_ddd, List.map (fun_param env) l

(* ddd stands for dot dot dot (...) *)
and ddd l =
  match l with
  | [] -> false, []
  | [x] -> (
      match (x.param_id) with
      | (_, "...") -> true, []
      | (_, _) -> false, [x]
     )
  | x :: rl ->
      let ddd, rl = ddd rl in
      ddd, x :: rl

and fun_param env param =
  let x = Env.new_lvar env param.param_id in
  let eopt = opt (expr env) param.param_expr in
  let ty = opt (hint env) param.param_hint in
  { N.param_hint = ty;
    param_is_reference = param.param_is_reference;
    param_id = x;
    param_name = snd param.param_id;
    param_expr = eopt;
  }

and make_constraints paraml =
  List.fold_right begin fun ((_, x), hl) acc ->
    SMap.add x hl acc
  end paraml SMap.empty

and extend_params genv paraml =
  let params = List.fold_right begin fun ((_, x), hopt) acc ->
    SMap.add x hopt acc
  end paraml genv.type_params in
  { genv with type_params = params }

and fun_ genv f =
  let tparams = make_constraints f.f_tparams in
  let genv = Env.make_fun_genv genv tparams f in
  let lenv = Env.empty_local () in
  let env = genv, lenv in
  let h = opt (hint ~allow_this:true env) f.f_ret in
  let has_ddd, paraml = fun_paraml env f.f_params in
  let x = Env.fun_id env f.f_name in
  let unsafe = is_unsafe_body f.f_body in
  List.iter check_constraint f.f_tparams;
  let f_tparams = type_paraml env f.f_tparams in
  let body =
    match genv.in_mode with
    | Ast.Mstrict | Ast.Mpartial -> block env f.f_body
    | Ast.Mdecl -> []
  in
  let fun_ =
    { N.f_unsafe = unsafe;
      f_mode = f.f_mode;
      f_ret = h;
      f_name = x;
      f_tparams = f_tparams;
      f_params = paraml;
      f_body = body;
      f_ddd = has_ddd;
      f_type = f.f_type;
    } in
  fun_

and flatten_blocks stl =
  List.fold_right begin fun st acc ->
    match st with
    | Block b -> flatten_blocks b @ acc
    | st -> st :: acc
  end stl []

and stmt env st =
  match st with
  | Block _              -> assert false
  | Unsafe               -> assert false
  | Fallthrough          -> N.Fallthrough
  | Noop                 -> N.Noop
  | Expr e               -> N.Expr (expr env e)
  | Break                -> N.Break
  | Continue             -> N.Continue
  | Throw e              -> let terminal = not (fst env).in_try in
                            N.Throw (terminal, expr env e)
  | Return (p, e)        -> N.Return (p, oexpr env e)
  | Static_var el        -> N.Static_var (static_varl env el)
  | If (e, b1, b2)       -> if_stmt env st e b1 b2
  | Do (b, e)            -> do_stmt env b e
  | While (e, b)         -> while_stmt env e b
  | For (st1, e, st2, b) -> for_stmt env st1 e st2 b
  | Switch (e, cl)       -> switch_stmt env st e cl
  | Foreach (e, ae, b)   -> foreach_stmt env e ae b
  | Try (b, cl, fb)      -> try_stmt env st b cl fb

and if_stmt env st e b1 b2 =
  let e = expr env e in
  let vars = Naming_ast_helpers.GetLocals.stmt SMap.empty st in
  SMap.iter (fun x p -> Env.new_pending_lvar env (p, x)) vars;
  let result = Env.scope env (
  fun env ->
    let _, lenv = env in
    let all_locals_copy = !(lenv.all_locals) in
    let all1, b1 = branch env b1 in
    let all2, b2 = branch env b2 in
    let all_locals = SMap.union all1 all2 in
    lenv.all_locals := SMap.union all_locals all_locals_copy;
    N.If (e, b1, b2)
 ) in
 Env.promote_pending env;
 result

and do_stmt env b e =
  let new_scope = false in
  let b = block ~new_scope env b in
  N.Do (b, expr env e)

and while_stmt env e b =
  let e = expr env e in
  N.While (e, block env b)

and for_stmt env e1 e2 e3 b =
  let e1 = expr env e1 in
  let e2 = expr env e2 in
  let e3 = expr env e3 in
  Env.scope env (
  fun env ->
    N.For (e1, e2, e3, block env b)
 )

and switch_stmt env st e cl =
  let e = expr env e in
  let vars = Naming_ast_helpers.GetLocals.stmt SMap.empty st in
  SMap.iter (fun x p -> Env.new_pending_lvar env (p, x)) vars;
  let result = Env.scope env (
  fun env ->
    let _, lenv = env in
    let all_locals_copy = !(lenv.all_locals) in
    let all_locals, cl = casel env cl in
    lenv.all_locals := SMap.union all_locals all_locals_copy;
    N.Switch (e, cl)
 ) in
 Env.promote_pending env;
 result

and foreach_stmt env e ae b =
  let e = expr env e in
  Env.scope env (
  fun env ->
    let _, lenv = env in
    let all_locals_copy = !(lenv.all_locals) in
    let ae = as_expr env ae in
    let all_locals, b = branch env b in
    lenv.all_locals := SMap.union all_locals all_locals_copy;
    N.Foreach (e, ae, b)
 )

and as_expr env = function
  | As_id (p, Lvar x) ->
      N.As_id (p, N.Lvar (Env.new_lvar env x))
  | As_id (p, _) when !Silent.is_silent_mode ->
      N.As_id (p, N.Lvar (Env.new_lvar env (p, "__dumb")))
  | As_id (p, _) -> error p "Was expecting a variable name"
  | As_kv ((p1, Lvar x1), (p2, Lvar x2)) ->
      let x1 = p1, N.Lvar (Env.new_lvar env x1) in
      let x2 = p2, N.Lvar (Env.new_lvar env x2) in
      N.As_kv (x1, x2)
  | As_kv ((p, _), _) when !Silent.is_silent_mode ->
      N.As_kv ((p, N.Lvar (Env.new_lvar env (p, "__dumb"))),
               (p, N.Lvar (Env.new_lvar env (p, "__dumb"))))
  | As_kv ((p, _), _) -> error p "Was expecting variable names"

and try_stmt env st b cl fb =
  let vars = Naming_ast_helpers.GetLocals.stmt SMap.empty st in
  SMap.iter (fun x p -> Env.new_pending_lvar env (p, x)) vars;
  let result = Env.scope env (
  fun env ->
    let genv, lenv = env in
    let all_locals_copy = !(lenv.all_locals) in
    (* isolate finally from the rest of the try-catch: if the first
     * statement of the try is an uncaught exception, finally will
     * still be executed *)
    let all_finally, fb = branch (genv, lenv) fb in
    lenv.all_locals := all_locals_copy;
    let all_locals_copy = !(lenv.all_locals) in
    let all1, b = branch ({ genv with in_try = true}, lenv) b in
    let all_locals, cl = catchl env cl in
    let all_locals = SMap.union all1 all_locals in
    lenv.all_locals := SMap.union all_locals all_locals_copy;
    N.Try (b, cl, fb)
  ) in
  Env.promote_pending env;
  result

and cut_unsafe ?(replacement=Noop) = function
  | [] -> []
  | Unsafe :: _ -> [replacement]
  | x :: rl -> x :: cut_unsafe ~replacement rl

and block ?(new_scope=true) env stl =
  let stl = flatten_blocks stl in
  let stl = cut_unsafe stl in
  if new_scope
  then
    Env.scope env (
      fun env -> List.map (stmt env) stl
    )
  else List.map (stmt env) stl

and branch env stmt_l =
  let stmt_l = flatten_blocks stmt_l in
  let stmt_l = cut_unsafe stmt_l in
  let genv, lenv = env in
  let lenv_copy = !(lenv.locals) in
  let lenv_all_locals_copy = !(lenv.all_locals) in
  let lenv_pending_copy = !(lenv.pending_locals) in
  let stmt_l = flatten_blocks stmt_l in
  let res = List.map (stmt env) stmt_l in
  lenv.locals := lenv_copy;
  let lenv_all_locals = !(lenv.all_locals) in
  lenv.all_locals := lenv_all_locals_copy;
  lenv.pending_locals := lenv_pending_copy;
  lenv_all_locals, res

and static_varl env l = List.map (static_var env) l
and static_var env = function
  | p, Lvar _ as lv -> expr env (p, Binop(Eq None, lv, (p, Null)))
  | e -> expr env e

and exprl env l = List.map (expr env) l
and oexpr env e = opt (expr env) e
and expr env (p, e) = p, expr_ env e
and expr_ env = function
  | Array l -> N.Array (List.map (afield env) l)
  | Collection (id, l) -> begin
    let p, cn = Env.resolve_namespace (fst env) id in
    match cn with
      | "Vector"
      | "ImmVector"
      | "Set"
      | "ImmSet" ->
        N.ValCollection (cn, (List.map (afield_value env cn) l))
      | "Map"
      | "ImmMap"
      | "StableMap" ->
        N.KeyValCollection (cn, (List.map (afield_kvalue env cn) l))
      | "Pair" ->
        (match l with
          | [] -> error p "Too few arguments"
          | e1::e2::[] ->
            let pn = "Pair" in
            N.Pair (afield_value env pn e1, afield_value env pn e2)
          | _ -> error p "Too many arguments"
        )
      | _ -> error p ("Unexpected collection type " ^ cn)
  end
  | Clone e -> N.Clone (expr env e)
  | Null -> N.Null
  | True -> N.True
  | False -> N.False
  | Int s -> N.Int s
  | Float s -> N.Float s
  | String s -> N.String s
  | String2 (idl, (_, s)) -> N.String2 (string2 env (List.rev idl), s)
  | Id x ->
    (match snd x with
      | "__LINE__" -> N.Int x
      | "__CLASS__" ->
        (match (fst env).cclass with
          | None -> error (fst x) "Using __CLASS__ outside a class"
          | Some c -> N.String c)
      | "__FILE__" | "__DIR__"
      (* could actually check that we are in a function, method, etc *)
      | "__FUNCTION__" | "__METHOD__" | "__TRAIT__"
      | "__NAMESPACE__"
        -> N.String x
      | _ -> N.Id (Env.global_const env x)
      )
  | Lvar (_, "$this") -> N.This
  | Lvar (_, x as var) when Autocomplete.is_auto_complete x ->
      Autocomplete.argument_global_type := Some Autocomplete.Acvar;
      Autocomplete.auto_complete_result :=
        SMap.fold begin fun x pos acc ->
          SMap.add x (Autocomplete.make_result_without_type x (fst pos)) acc
        end !((snd env).locals) SMap.empty;
      if (fst env).in_member_fun then begin
        let this_result = (match (fst env).cclass with
        | None -> Autocomplete.make_result_without_pos_or_type "$this"
        | Some cid -> Autocomplete.make_result_without_type "$this" (fst cid)) in
        Autocomplete.auto_complete_result :=
          SMap.add "$this" this_result !Autocomplete.auto_complete_result end;
      N.Lvar (Env.lvar env var)
  | Lvar x -> N.Lvar (Env.lvar env x)
  | Obj_get (e1, (p, Id x)) ->
      N.Obj_get (expr env e1, (p, N.Id x))
  | Obj_get (e1, (p, _ as e2)) ->
      (match (fst env).in_mode with
      | Ast.Mstrict ->
          error p "Dynamic method call"
      | Ast.Mpartial | Ast.Mdecl ->
          ()
      );
      N.Obj_get (expr env e1, expr env e2)
  | Array_get ((p, Lvar x), None) ->
      let id = p, N.Lvar (Env.lvar env x) in
      N.Array_get (id, None)
  | Array_get (e1, e2) -> N.Array_get (expr env e1, oexpr env e2)
  | Class_get (x1, x2) ->
      N.Class_get (make_class_id env x1, x2)
  | Class_const (x1, x2) ->
      N.Class_const (make_class_id env x1, x2)
  | Call ((_, Id (p, "echo")), el) ->
      N.Call (N.Cnormal, (p, N.Id (p, "echo")), List.map (expr env) el)
  | Call ((p, Id (_, "call_user_func")), el) ->
      (match el with
      | [] -> error p "Too few arguments"
      | f :: el ->
          N.Call (N.Cuser_func, expr env f, List.map (expr env) el)
      )
  | Call ((p, Id (_, "fun")), el) ->
      (match el with
      | [] -> error p "Too few arguments"
      | [_, String x] -> N.Fun_id (Env.fun_id env x)
      | [p, _] ->
          let msg = "The argument to fun() must be a single-quoted, constant "^
                    "literal string representing a valid function name." in
          error p msg
      | _ -> error p "Too many arguments"
      )
  | Call ((p, Id (_, "inst_meth")), el) ->
      (match el with
      | [] -> error p "Too few arguments"
      | [_] -> error p "Too few arguments"
      | instance::(_, String meth)::[] ->
        N.Method_id (expr env instance, meth)
      | (p, _)::(_)::[] ->
        let msg = "The argument to inst_meth() must be an expression and a "^
          "constant literal string representing a valid method name." in
        error p msg
      | _ -> error p "Too many arguments"
      )
  | Call ((p, Id (_, "meth_caller")), el) ->
      (match el with
      | [] -> error p "Too few arguments"
      | [_] -> error p "Too few arguments"
      | e1::e2::[] ->
          (match (expr env e1), (expr env e2) with
          | (_, N.String cl), (_, N.String meth)
          | (_, N.Class_const (N.CI cl, (_, "class"))), (_, N.String meth) ->
            N.Method_caller (cl, meth)
          | (p, _), (_) ->
            let msg =
              "The two arguments to meth_caller() must be:"
              ^"\n - first: ClassOrInterface::class"
              ^"\n - second: a single-quoted string literal containing the name"
              ^" of a non-static method of that class" in
            error p msg)
      | _ -> error p "Too many arguments"
      )
  | Call ((p, Id (_, "class_meth")), el) ->
      (match el with
      | [] -> error p "Too few arguments"
      | [_] -> error p "Too few arguments"
      | e1::e2::[] ->
          (match (expr env e1), (expr env e2) with
          | (_, N.String cl), (_, N.String meth)
          | (_, N.Class_const (N.CI cl, (_, "class"))), (_, N.String meth) ->
            N.Smethod_id (cl, meth)
          | (p, _), (_) ->
            let msg =
              "The two arguments to class_meth() must be:"
              ^"\n - first: ValidClassname::class"
              ^"\n - second: a single-quoted string literal containing the name"
              ^" of a static method of that class" in
            error p msg)
      | _ -> error p "Too many arguments"
      )
  | Call ((p, Id (_, "assert")), el) ->
      if List.length el <> 1
      then error p "assert expects exactly one argument";
      N.Assert (N.AE_assert (expr env (List.hd el)))
  | Call ((p, Id (_, "invariant")), el) ->
      let (st, format, el) = match el with
        | st :: format :: el -> (st, format, el)
        | _ -> error p "Too few arguments" in
      let el = List.map (expr env) el in
      N.Assert (N.AE_invariant (expr env st, expr env format, el))
  | Call ((p, Id (_, "invariant_violation")), el) ->
      let (format, el) = match el with
        | format :: el -> (format, el)
        | _ -> error p "Too few arguments" in
      let el = List.map (expr env) el in
      N.Assert (N.AE_invariant_violation (expr env format, el))
  | Call ((p, Id (_, "tuple")), el) ->
      (match el with
      | [] -> error p "Too few arguments"
      | el ->
          N.List (List.map (expr env) el)
      )
  | Call ((p, Id (_, "gena")), el) ->
      (match el with
      | [e] -> N.Special_func (N.Gena (expr env e))
      | _ -> error p "gena() expects exactly 1 argument")
  | Call ((p, Id (_, "genva")), el) ->
      if List.length el < 1
      then error p "genva() expects at least 1 argument"
      else N.Special_func (N.Genva (List.map (expr env) el))
  | Call ((p, Id (_, "gen_array_rec")), el) ->
      (match el with
      | [e] -> N.Special_func (N.Gen_array_rec (expr env e))
      | _ -> error p "gen_array_rec() expects exactly 1 argument")
  | Call ((p, Id (_, "gen_array_va_rec_DEPRECATED")), el) ->
      if List.length el < 1
      then error p "gen_array_va_rec_DEPRECATED() expects at least 1 argument"
      else N.Special_func (N.Gen_array_va_rec (List.map (expr env) el))
  | Call ((p, Id f), el) ->
      N.Call (N.Cnormal, (p, N.Id (Env.fun_id env f)),
        List.map (expr env) el)
  | Call (e, el) ->
      N.Call (N.Cnormal, expr env e, List.map (expr env) el)
  | Yield_break -> N.Yield_break
  | Yield e -> N.Yield (expr env e)
  | Await e -> N.Await (expr env e)
  | List el -> N.List (exprl env el)
  | Expr_list el -> N.Expr_list (exprl env el)
  | Cast (ty, e2) ->
      hint_no_typedef env ty;
      let ty = hint env ty in
      N.Cast (ty, expr env e2)
  | Unop (uop, e) -> N.Unop (uop, expr env e)
  | Binop (Eq None as op, lv, e2) ->
      let e2 = expr env e2 in
      let vars = Naming_ast_helpers.GetLocals.lvalue SMap.empty lv in
      SMap.iter (fun x p -> ignore (Env.new_lvar env (p, x))) vars;
      N.Binop (op, expr env lv, e2)
  | Binop (bop, e1, e2) ->
      let e1 = expr env e1 in
      N.Binop (bop, e1, expr env e2)
  | Eif (e1, e2opt, e3) ->
      (* The order matters here, of course -- e1 can define vars that need to
       * be available in e2 and e3. *)
      let e1 = expr env e1 in
      let e2opt = oexpr env e2opt in
      let e3 = expr env e3 in
      N.Eif (e1, e2opt, e3)
  | InstanceOf (e, (p, Id x)) ->
      N.InstanceOf (expr env e, (p, N.Id (Env.class_name env x)))
  | InstanceOf (e1, e2) ->
      N.InstanceOf (expr env e1, expr env e2)
  | New (x, el) ->
      if Autocomplete.is_auto_complete (snd x) then (
        Autocomplete.auto_complete_for_global := (snd x);
        Autocomplete.argument_global_type := Some Autocomplete.Acnew);
      N.New (make_class_id env x, exprl env el)
  | Efun (f, idl) ->
      let idl = List.filter (function (_, "$this") -> false | _ -> true) idl in
      let idl' = List.map (Env.lvar env) idl in
      let env = (fst env, Env.empty_local ()) in
      List.iter2 (Env.add_lvar env) idl idl';
      let f = expr_lambda env f in
      N.Efun (f, idl')
  | Lfun f ->
      (* We have to build the capture list while we're finding names in
         the closure body---accumulate it in to_capture. *)
      let to_capture = ref [] in
      let handle_unbound (p, x) =
        let cap = Env.lvar env (p, x) in
        to_capture := cap :: !to_capture;
        cap
      in
      let lenv = Env.empty_local () in
      let lenv = { lenv with unbound_mode = UBMFunc handle_unbound } in
      let env = (fst env, lenv) in
      let f = expr_lambda env f in
      N.Efun (f, !to_capture)
  | Xml (x, al, el) -> N.Xml (Env.class_name env x, attrl env al, List.map (expr env) el)
  | Shape fdl ->
      N.Shape begin List.fold_left begin fun fdm (pname, value) ->
        let pos, name = pname in
        if SMap.mem name fdm
        then error pos "Field already defined";
        SMap.add name (expr env value) fdm
      end SMap.empty fdl
      end

and expr_lambda env f =
  let h = opt (hint ~allow_this:true env) f.f_ret in
  let unsafe = List.mem Unsafe f.f_body in
  let has_ddd, paraml = fun_paraml env f.f_params in
  let body = block env f.f_body in
  {
    N.f_unsafe = unsafe;
    f_mode = (fst env).in_mode;
    f_ret = h;
    f_name = f.f_name;
    f_params = paraml;
    f_tparams = [];
    f_body = body;
    f_ddd = has_ddd;
    f_type = f.f_type;
  }

and make_class_id env cid =
  no_typedef env cid;
  match snd cid with
  | "parent" -> N.CIparent
  | "self" ->  N.CIself
  | "static" -> N.CIstatic
  | _ -> N.CI (Env.class_name env cid)

and casel env l =
  lfold (case env) SMap.empty l

and case env acc = function
  | Default b ->
      let b = cut_unsafe ~replacement:Fallthrough b in
      let all_locals, b = branch env b in
      let acc = SMap.union all_locals acc in
      acc, N.Default b
  | Case (e, b) ->
      let e = expr env e in
      let b = cut_unsafe ~replacement:Fallthrough b in
      let all_locals, b = branch env b in
      let acc = SMap.union all_locals acc in
      acc, N.Case (e, b)

and catchl env l = lfold (catch env) SMap.empty l
and catch env acc (x1, x2, b) =
  let x2 = Env.new_lvar env x2 in
  let all_locals, b = branch env b in
  let acc = SMap.union all_locals acc in
  acc, (Env.class_name env x1, x2, b)

and fieldl env l = List.map (field env) l
and field env (e1, e2) = (expr env e1, expr env e2)

and afield env = function
  | AFvalue e -> N.AFvalue (expr env e)
  | AFkvalue (e1, e2) -> N.AFkvalue (expr env e1, expr env e2)

and afield_value env cname = function
  | AFvalue e -> expr env e
  | AFkvalue (e1, e2) -> Error.unexpected_arrow ((fst e1), cname)

and afield_kvalue env cname = function
  | AFvalue e -> Error.missing_arrow ((fst e), cname)
  | AFkvalue (e1, e2) -> expr env e1, expr env e2

and attrl env l = List.map (attr env) l
and attr env (x, e) = x, expr env e

and string2 env idl =
  List.rev (List.rev_map (expr env) idl)

(*****************************************************************************)
(* Typedefs *)
(*****************************************************************************)

let typedef genv tdef =
  let ty = match tdef.t_kind with Alias t | NewType t -> t in
  let cstrs = class_constraints genv tdef.t_tparams in
  let env = Env.make_typedef_env genv cstrs tdef in
  let tconstraint =
    match tdef.t_constraint with
    | None -> None
    | Some h -> Some (hint env h)
  in
  List.iter check_constraint tdef.t_tparams;
  let tparaml = type_paraml env tdef.t_tparams in
  List.iter begin function
    | (_, Some (pos, _)) ->
        error pos "Constraints on typedefs are not supported"
    | _ -> ()
  end tparaml;
  let ty = hint env ty in
  tparaml, tconstraint, ty

(*****************************************************************************)
(* Global constants *)
(*****************************************************************************)

let check_constant cst =
  (match cst.cst_type with
  | None when cst.cst_mode = Ast.Mstrict ->
      error (fst cst.cst_name) "Please add a type hint"
  | None
  | Some _ -> ());
  match snd cst.cst_value with
  | Id _ | Null | True | False | Int _
  | Float _ | String _
  | Class_const _
  | Unop ((Uplus | Uminus), _)
  | String2 ([], _) -> ()
  | String2 ((var_pos, _) :: _, _) ->
      error var_pos "You cannot use a local variable in a constant definition"
  | _ -> error (fst cst.cst_value) "Illegal constant value"

let global_const genv cst =
  let env = Env.make_const_env genv cst in
  let hint = opt (hint env) cst.cst_type in
  let e = match cst.cst_kind with
  | Ast.Cst_const -> check_constant cst; Some (expr env cst.cst_value)
  (* Define allows any expression, so don't call check_constant. Furthermore it
   * often appears at toplevel, which we don't track at all, so don't type or
   * even name that expression, it may refer to "undefined" variables that
   * actually exist, just untracked since they're toplevel. *)
  | Ast.Cst_define -> None in
  { N.cst_mode = cst.cst_mode;
    cst_name = cst.cst_name;
    cst_type = hint;
    cst_value = e;
  }

(*****************************************************************************)
(* Declaring the names in a list of files *)
(*****************************************************************************)

let add_files_to_rename nenv failed defl defs_in_env =
  List.fold_left begin fun failed (_, def) ->
    match SMap.get def defs_in_env with
    | None -> failed
    | Some (previous_definition_position, _) ->
      let filename = Pos.filename previous_definition_position in
      SSet.add filename failed
  end failed defl

let ndecl_file fn
    {FileInfo.funs; classes; types; consts; consider_names_just_for_autoload}
    (errorl, failed, nenv) = try
  dn ("Naming decl: "^fn);
  let nenv =
    if consider_names_just_for_autoload
    then nenv
    else make_env nenv ~funs ~classes ~typedefs:types ~consts
  in
  errorl, failed, nenv
with Utils.Error l ->
  (* IMPORTANT:
   * If a file has name collisions, we MUST add the list of files that
   * were previously defining the type to the set of "failed" files.
   * If we fail to do so, we will be in a phony state, where a name could
   * be missing.
   *
   * Example:
   * A.php defines class A
   * B.php defines class B
   * Save the state, now let's introduce a new file (foo.php):
   * foo.php defines class A and class B.
   *
   * 2 things happen (cf serverTypeCheck.ml):
   * We remove the names A and B from the global environment.
   * We report the error.
   *
   * But this is clearly not enough. If the user removes the file foo.php,
   * both class A and class B are now missing from the naming environment.
   * If the user has a file using class A (in strict), he now gets the
   * error "Unbound name class A".
   *
   * The solution consist in adding all the files that were previously
   * defining the same things as foo.php to the set of files to recheck.
   *
   * This way, when the user removes foo.php, A.php and B.php are recomputed
   * and the naming environment is in a sane state.
   *)
  let failed = add_files_to_rename nenv failed funs nenv.ifuns in
  let failed = add_files_to_rename nenv failed classes nenv.iclasses in
  let failed = add_files_to_rename nenv failed types nenv.itypedefs in
  let failed = add_files_to_rename nenv failed consts nenv.iconsts in
 l :: errorl, SSet.add fn failed, nenv
