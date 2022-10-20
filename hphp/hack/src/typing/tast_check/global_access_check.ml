(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * This global access checker raises an error when:
 * - a global variable is written:
 *   - a global variable is directly written (e.g. (Foo::$bar)->prop = 1;)
 *   - a global variable is written via references (e.g. $a = Foo::$bar; $a->prop = 1;)
 *   - a global variable is passed to a function call, in which it may be written.
 * - or a global variable is read.
 *
 * Notice that the return value of a memoized function (if it is mutable)
 * is treated as a global variable as well.
 *
 * By default, this checker is turned off.
 * To turn on this checker on certain files/functions:
 * - use the argument --enable-global-access-check-files
 *   to specify the prefixes of files to be checked (e.g. "/" for all files).
 * - use the argument --enable-global-access-check-functions
 *   to specify a JSON file of functions names to be checked.
 *
 * When the checker is turned on, both global writes and reads are checked by default.
 * To check only global writes or global reads:
 * - use the flag --disable-global-access-check-on-write;
 * - use the flag --disable-global-access-check-on-read.
 *
 * A trick to run this checker on a specific list of files (not simply by prefix) is to
 *   use "--config enable_type_check_filter_files=true" togehter with above arguments,
 *   which runs typechecker only on files listed in ~/.hack_type_check_files_filter.
 *)

open Hh_prelude
open Aast
module MakeType = Typing_make_type
module Reason = Typing_reason
module Cls = Decl_provider.Class
module Hashtbl = Stdlib.Hashtbl
module Option = Stdlib.Option
module GlobalAccessCheck = Error_codes.GlobalAccessCheck

(* Raise a global access error if the corresponding write/read mode is enabled. *)
let raise_global_access_error env pos fun_name data_type global_set error_code =
  let tcopt = Tast_env.get_tcopt env in
  let (error_message, error_enabled) =
    match error_code with
    | GlobalAccessCheck.DefiniteGlobalWrite ->
      ( "definitely written.",
        TypecheckerOptions.global_access_check_on_write tcopt )
    | GlobalAccessCheck.PossibleGlobalWriteViaReference ->
      ( "possibly written via reference.",
        TypecheckerOptions.global_access_check_on_write tcopt )
    | GlobalAccessCheck.PossibleGlobalWriteViaFunctionCall ->
      ( "possibly written via function call.",
        TypecheckerOptions.global_access_check_on_write tcopt )
    | GlobalAccessCheck.DefiniteGlobalRead ->
      ("definitely read.", TypecheckerOptions.global_access_check_on_read tcopt)
  in
  if error_enabled then
    let global_vars_str =
      SSet.fold
        (fun s cur_str ->
          cur_str
          ^ (if String.length cur_str > 0 then
              ","
            else
              "")
          ^ s)
        global_set
        ""
    in
    let message =
      "["
      ^ fun_name
      ^ "]{"
      ^ global_vars_str
      ^ "}("
      ^ data_type
      ^ ") A global variable is "
      ^ error_message
    in
    Errors.global_access_error error_code pos message

(* The context maintains a hash table from each global variable to the
  corresponding references of static variables or memoized functions.
  For example, consider the following program:
  "if (condition) { $a = Foo::$bar } else { $a = memoized_func() }"
  after the above conditional, $a is a global variable which is a reference to
  either Foo::$bar or memoized_func, thus the context gets a hash table
  {"a" => {"Foo::$bar", "\memoized_func"}}. *)
type ctx = { global_var_refs_tbl: (string, SSet.t) Hashtbl.t ref }

let current_ctx = { global_var_refs_tbl = ref (Hashtbl.create 0) }

(* Add the key (a variable name) and the value (a set of references) to the table. *)
let add_var_refs_to_tbl tbl var refs =
  let pre_ref_set =
    if Hashtbl.mem tbl var then
      Hashtbl.find tbl var
    else
      SSet.empty
  in
  Hashtbl.replace tbl var (SSet.union pre_ref_set refs)

(* Given two hash tables of type (string, SSet.t) Hashtbl.t, merge the second
  table into the first one. *)
let merge_var_refs_tbls tbl1 tbl2 = Hashtbl.iter (add_var_refs_to_tbl tbl1) tbl2

(* Remove a set of variables from the var_refs_tbl table. *)
let remove_vars_from_tbl tbl vars = SSet.iter (Hashtbl.remove tbl) vars

let rec grab_class_elts_from_ty ~static ?(seen = SSet.empty) env ty prop_id =
  let open Typing_defs in
  (* Given a list of types, find recurse on the first type that
     has the property and return the result *)
  let find_first_in_list ~seen tyl =
    List.find_map
      ~f:(fun ty ->
        match grab_class_elts_from_ty ~static ~seen env ty prop_id with
        | [] -> None
        | tyl -> Some tyl)
      tyl
  in
  match get_node ty with
  | Tclass (id, _exact, _args) ->
    let provider_ctx = Tast_env.get_ctx env in
    let class_decl = Decl_provider.get_class provider_ctx (snd id) in
    (match class_decl with
    | Some class_decl ->
      let prop =
        if static then
          Cls.get_sprop class_decl (snd prop_id)
        else
          Cls.get_prop class_decl (snd prop_id)
      in
      Option.to_list prop
    | None -> [])
  (* Accessing a property off of an intersection type
     should involve exactly one kind of readonlyness, since for
     the intersection type to exist, the property must be related
     by some subtyping relationship anyways, and property readonlyness
     is invariant. Thus we just grab the first one from the list where the prop exists. *)
  | Tintersection [] -> []
  | Tintersection tyl ->
    find_first_in_list ~seen tyl |> Option.value ~default:[]
  (* A union type is more interesting, where we must return all possible cases
     and be conservative in our use case. *)
  | Tunion tyl ->
    List.concat_map
      ~f:(fun ty -> grab_class_elts_from_ty ~static ~seen env ty prop_id)
      tyl
  (* Generic types can be treated similarly to an intersection type
     where we find the first prop that works from the upper bounds *)
  | Tgeneric (name, tyargs) ->
    (* Avoid circular generics with a set *)
    if SSet.mem name seen then
      []
    else
      let new_seen = SSet.add name seen in
      let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
      find_first_in_list ~seen:new_seen (Typing_set.elements upper_bounds)
      |> Option.value ~default:[]
  | Tdependent (_, ty) ->
    (* Dependent types have an upper bound that's a class or generic *)
    grab_class_elts_from_ty ~static ~seen env ty prop_id
  | Toption ty ->
    (* If it's nullable, take the *)
    grab_class_elts_from_ty ~static ~seen env ty prop_id
  | _ -> []

(* Return a list of possible static prop elts given a class_get expression *)
let get_static_prop_elts env class_id get =
  let (ty, _, _) = class_id in
  match get with
  | CGstring prop_id -> grab_class_elts_from_ty ~static:true env ty prop_id
  (* An expression is dynamic, so there's no way to tell the type generally *)
  | CGexpr _ -> []

(* Check if an expression is directly from a static variable or not,
  e.g. it returns true for Foo::$bar or (Foo::$bar)->prop. *)
let rec is_expr_static env (_, _, te) =
  match te with
  | Class_get (class_id, expr, Is_prop) ->
    (* Ignore static variables annotated with <<__SafeForGlobalAccessCheck>> *)
    let class_elts = get_static_prop_elts env class_id expr in
    not (List.exists class_elts ~f:Typing_defs.get_ce_safe_global_variable)
  | Obj_get (e, _, _, Is_prop) -> is_expr_static env e
  | Array_get (e, _) -> is_expr_static env e
  | _ -> false

(* Print out global variables, e.g. Foo::$bar => "Foo::$bar", self::$bar => "Foo::$bar",
  memoized_func => "\memoized_func", $baz->memoized_method => "Baz::memoized_method".
  Notice that this does not handle arbitrary expressions. *)
let rec print_global_expr env expr =
  match expr with
  | Call ((_, _, caller_expr), _, _, _) ->
    (* For function/method calls, we print the caller expression, which could be
       Id (e.g. memoized_func()) or Obj_get (e.g. $baz->memoized_method()). *)
    print_global_expr env caller_expr
  | Class_get ((c_ty, _, _), expr, _) ->
    (* For static properties, we concatenate the class type (instead of class_id_, which
       could be self, parent, static) and the property name. *)
    let class_ty_str = Tast_env.print_ty env c_ty in
    (match expr with
    | CGstring (_, expr_str) -> class_ty_str ^ "::" ^ expr_str
    | CGexpr _ -> class_ty_str ^ "::Unknown")
  | Class_const ((c_ty, _, _), (_, const_str)) ->
    (* For static method calls, we concatenate the class type and the method name. *)
    Tast_env.print_ty env c_ty ^ "::" ^ const_str
  | Id (_, name) -> name
  | Obj_get (obj, m, _, _) ->
    (* For Obj_get (e.g. $baz->memoized_method()), we concatenate the class type and the method id. *)
    let class_ty_str = Tast_env.print_ty env (Tast.get_type obj) in
    let (_, _, m_id) = m in
    (* For the case $obj?->method(), the question mark is removed from the class type,
       since we are not interested in the case where $obj is null. *)
    let remove_question_mark_prefix str =
      if String.is_prefix ~prefix:"?" str then
        String.sub str ~pos:1 ~len:(String.length str - 1)
      else
        str
    in
    remove_question_mark_prefix class_ty_str ^ "->" ^ print_global_expr env m_id
  | _ -> "Unknown"

(* Given the environment, the context and an expression, this function returns
  the set of global variables (i.e static variables / return of memoized functions)
  used in that expression. When track_refs is true, the references to global variables
  are also taken into account; otherwise, we get only the direct use of global variables.
  If there is no such global variable, return None. *)
let rec get_globals_from_expr env ctx (_, _, te) ~track_refs =
  let merge_opt_sets opt_s1 opt_s2 =
    match opt_s1 with
    | None -> opt_s2
    | Some s1 ->
      (match opt_s2 with
      | None -> opt_s1
      | Some s2 -> Some (SSet.union s1 s2))
  in
  match te with
  | Class_get (class_id, expr, Is_prop) ->
    (* Ignore static variables annotated with <<__SafeForGlobalAccessCheck>> *)
    let class_elts = get_static_prop_elts env class_id expr in
    if not (List.exists class_elts ~f:Typing_defs.get_ce_safe_global_variable)
    then
      Some (SSet.singleton (print_global_expr env te))
    else
      None
  | Lvar (_, id) ->
    if track_refs then
      Hashtbl.find_opt !(ctx.global_var_refs_tbl) (Local_id.to_string id)
    else
      None
  | Obj_get (e, _, _, Is_prop) -> get_globals_from_expr env ctx e ~track_refs
  | Darray (_, tpl) ->
    List.fold tpl ~init:None ~f:(fun cur_opt_set (_, e) ->
        merge_opt_sets cur_opt_set (get_globals_from_expr env ctx e ~track_refs))
  | Varray (_, el) ->
    List.fold el ~init:None ~f:(fun cur_opt_set e ->
        merge_opt_sets cur_opt_set (get_globals_from_expr env ctx e ~track_refs))
  | Shape tpl ->
    List.fold tpl ~init:None ~f:(fun cur_opt_set (_, e) ->
        merge_opt_sets cur_opt_set (get_globals_from_expr env ctx e ~track_refs))
  | ValCollection (_, _, el) ->
    List.fold el ~init:None ~f:(fun cur_opt_set e ->
        merge_opt_sets cur_opt_set (get_globals_from_expr env ctx e ~track_refs))
  | KeyValCollection (_, _, fl) ->
    List.fold fl ~init:None ~f:(fun cur_opt_set (_, e) ->
        merge_opt_sets cur_opt_set (get_globals_from_expr env ctx e ~track_refs))
  | Array_get (e, _) -> get_globals_from_expr env ctx e ~track_refs
  | Await e -> get_globals_from_expr env ctx e ~track_refs
  | ReadonlyExpr e -> get_globals_from_expr env ctx e ~track_refs
  | Tuple el
  | List el ->
    List.fold el ~init:None ~f:(fun cur_opt_set e ->
        merge_opt_sets cur_opt_set (get_globals_from_expr env ctx e ~track_refs))
  | Cast (_, e) -> get_globals_from_expr env ctx e ~track_refs
  | Eif (_, e1, e2) ->
    merge_opt_sets
      (match e1 with
      | Some e -> get_globals_from_expr env ctx e ~track_refs
      | None -> None)
      (get_globals_from_expr env ctx e2 ~track_refs)
  | As (e, _, _) -> get_globals_from_expr env ctx e ~track_refs
  | Upcast (e, _) -> get_globals_from_expr env ctx e ~track_refs
  | Pair (_, e1, e2) ->
    merge_opt_sets
      (get_globals_from_expr env ctx e1 ~track_refs)
      (get_globals_from_expr env ctx e2 ~track_refs)
  | Call (caller, _, _, _) ->
    let caller_ty = Tast.get_type caller in
    let open Typing_defs in
    (match get_node caller_ty with
    | Tfun fty when get_ft_is_memoized fty ->
      Some (SSet.singleton (print_global_expr env te))
    | _ -> None)
  | _ -> None

(* Check if type is a collection. *)
let is_value_collection_ty env ty =
  let mixed = MakeType.mixed Reason.none in
  let env = Tast_env.tast_env_as_typing_env env in
  let hackarray = MakeType.any_array Reason.none mixed mixed in
  (* Subtype against an empty open shape (shape(...)) *)
  let shape =
    MakeType.shape
      Reason.none
      Typing_defs.Open_shape
      Typing_defs.TShapeMap.empty
  in
  Typing_utils.is_sub_type env ty hackarray
  || Typing_utils.is_sub_type env ty shape

(* Check if the variable type does NOT has a reference to any object:
  if so, then it is OK to write to this variable.
  Copied from is_safe_mut_ty in readonly_check.ml.
  To do: check if any change is needed for the global write checker. *)
let rec has_no_object_ref_ty env (seen : SSet.t) ty =
  let open Typing_defs_core in
  let (env, ty) = Tast_env.expand_type env ty in
  match get_node ty with
  (* Allow all primitive types *)
  | Tprim _ -> true
  (* Open shapes can technically have objects in them, but as long as the current fields don't have objects in them
     we will allow you to call the function. Note that the function fails at runtime if any shape fields are objects. *)
  | Tshape (_, fields) ->
    TShapeMap.for_all
      (fun _k v -> has_no_object_ref_ty env seen v.sft_ty)
      fields
  (* If it's a Tclass it's an array type by is_value_collection *)
  | Tintersection tyl ->
    List.exists tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  (* Only error if there isn't a type that it could be that's primitive *)
  | Tunion tyl -> List.exists tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Ttuple tyl -> List.for_all tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Tdependent (_, upper) ->
    (* check upper bounds *)
    has_no_object_ref_ty env seen upper
  | Tclass ((_, id), _, tyl)
    when is_value_collection_ty env ty || String.equal id "\\HH\\Awaitable" ->
    List.for_all tyl ~f:(fun l -> has_no_object_ref_ty env seen l)
  | Tgeneric (name, tyargs) ->
    (* Avoid circular generics with a set *)
    if SSet.mem name seen then
      false
    else
      let new_seen = SSet.add name seen in
      let upper_bounds = Tast_env.get_upper_bounds env name tyargs in
      Typing_set.exists
        (fun l -> has_no_object_ref_ty env new_seen l)
        upper_bounds
  | _ ->
    (* Otherwise, check if there's any primitive type it could be *)
    let env = Tast_env.tast_env_as_typing_env env in
    let primitive_types =
      [
        MakeType.bool Reason.none;
        MakeType.int Reason.none;
        MakeType.arraykey Reason.none;
        MakeType.string Reason.none;
        MakeType.float Reason.none;
        MakeType.num Reason.none;
        (* Keysets only contain arraykeys so if they're readonly its safe to remove *)
        MakeType.keyset Reason.none (MakeType.arraykey Reason.none);
        (* We don't put null here because we want to exclude ?Foo.
           as_mut(null) itself is allowed by the Tprim above*)
      ]
    in
    (* Make sure that a primitive *could* be this type by intersecting all primitives and subtyping. *)
    let union = MakeType.union Reason.none primitive_types in
    not (Typing_subtype.is_type_disjoint env ty union)

let get_global_and_mutable_from_expr env ctx (tp, p, te) ~track_refs =
  if not (has_no_object_ref_ty env SSet.empty tp) then
    get_globals_from_expr env ctx (tp, p, te) ~track_refs
  else
    None

(* Given an expression that appears on LHS of an assignment,
  this method gets the set of variables whose value may be assigned. *)
let rec get_vars_in_expr vars (_, _, te) =
  match te with
  | Lvar (_, id) -> vars := SSet.add (Local_id.to_string id) !vars
  | Obj_get (e, _, _, Is_prop) -> get_vars_in_expr vars e
  | Array_get (e, _) -> get_vars_in_expr vars e
  | ReadonlyExpr e -> get_vars_in_expr vars e
  | List el -> List.iter el ~f:(get_vars_in_expr vars)
  | _ -> ()

(* Suppose te is on LHS of an assignment, check if we can write to global variables
  by accessing either directly static variables or an object's properties. *)
let rec has_global_write_access (_, _, te) =
  match te with
  | Class_get (_, _, Is_prop)
  | Obj_get (_, _, _, Is_prop) ->
    true
  | List el -> List.exists el ~f:has_global_write_access
  | Lvar _
  | ReadonlyExpr _
  | Array_get _
  | _ ->
    false

let visitor =
  object (self)
    inherit [_] Tast_visitor.iter_with_state as super

    method! on_method_ (env, (ctx, fun_name)) m =
      Hashtbl.clear !(ctx.global_var_refs_tbl);
      super#on_method_ (env, (ctx, fun_name)) m

    method! on_fun_def (env, (ctx, fun_name)) f =
      Hashtbl.clear !(ctx.global_var_refs_tbl);
      super#on_fun_def (env, (ctx, fun_name)) f

    method! on_fun_ (env, (ctx, fun_name)) f =
      let ctx_cpy = Hashtbl.copy !(ctx.global_var_refs_tbl) in
      super#on_fun_ (env, (ctx, fun_name)) f;
      ctx.global_var_refs_tbl := ctx_cpy

    method! on_stmt_ (env, (ctx, fun_name)) s =
      match s with
      | If (_, b1, b2) ->
        (* Union the contexts from two branches *)
        let ctx_cpy =
          {
            global_var_refs_tbl = ref (Hashtbl.copy !(ctx.global_var_refs_tbl));
          }
        in
        super#on_block (env, (ctx_cpy, fun_name)) b1;
        super#on_block (env, (ctx, fun_name)) b2;
        merge_var_refs_tbls
          !(ctx.global_var_refs_tbl)
          !(ctx_cpy.global_var_refs_tbl)
      | Do (b, _)
      | While (_, b)
      | For (_, _, _, b)
      | Foreach (_, _, b) ->
        (* Iterate the block and update the set of global varialbes until
           no new global variable is found *)
        let ctx_cpy =
          {
            global_var_refs_tbl = ref (Hashtbl.copy !(ctx.global_var_refs_tbl));
          }
        in
        let ctx_len = ref (Hashtbl.length !(ctx.global_var_refs_tbl)) in
        let has_context_change = ref true in
        while !has_context_change do
          super#on_block (env, (ctx_cpy, fun_name)) b;
          merge_var_refs_tbls
            !(ctx.global_var_refs_tbl)
            !(ctx_cpy.global_var_refs_tbl);
          if Hashtbl.length !(ctx.global_var_refs_tbl) <> !ctx_len then
            ctx_len := Hashtbl.length !(ctx.global_var_refs_tbl)
          else
            has_context_change := false
        done
      | Return r ->
        (match r with
        | Some ((ty, p, _) as e) ->
          (match
             get_global_and_mutable_from_expr env ctx e ~track_refs:true
           with
          | Some global_set ->
            raise_global_access_error
              env
              p
              fun_name
              (Tast_env.print_ty env ty)
              global_set
              GlobalAccessCheck.PossibleGlobalWriteViaFunctionCall
          | None -> ())
        | None -> ());
        super#on_stmt_ (env, (ctx, fun_name)) s
      | _ -> super#on_stmt_ (env, (ctx, fun_name)) s

    method! on_expr (env, (ctx, fun_name)) ((_, p, e) as te) =
      (match e with
      | Binop (Ast_defs.Eq _, le, re) ->
        let () = self#on_expr (env, (ctx, fun_name)) re in
        let re_ty = Tast_env.print_ty env (Tast.get_type re) in
        let le_global_opt = get_globals_from_expr env ctx le ~track_refs:true in
        let re_direct_global_opt =
          get_globals_from_expr env ctx re ~track_refs:false
        in
        (match re_direct_global_opt with
        | Some re_direct_global ->
          raise_global_access_error
            env
            p
            fun_name
            re_ty
            re_direct_global
            GlobalAccessCheck.DefiniteGlobalRead
        | None -> ());
        (* Distinguish directly writing to static variables from writing to a variable that has references to static variables. *)
        if is_expr_static env le && Option.is_some le_global_opt then
          raise_global_access_error
            env
            p
            fun_name
            re_ty
            (Option.get le_global_opt)
            GlobalAccessCheck.DefiniteGlobalWrite
        else
          let re_global_and_mutable_opt =
            get_global_and_mutable_from_expr env ctx re ~track_refs:true
          in
          let vars_in_le = ref SSet.empty in
          let () = get_vars_in_expr vars_in_le le in
          (match has_global_write_access le with
          | true ->
            if Option.is_some le_global_opt then
              raise_global_access_error
                env
                p
                fun_name
                re_ty
                (Option.get le_global_opt)
                GlobalAccessCheck.PossibleGlobalWriteViaReference
          | false ->
            if
              Option.is_some le_global_opt
              && Option.is_none re_global_and_mutable_opt
            then
              remove_vars_from_tbl !(ctx.global_var_refs_tbl) !vars_in_le);
          if Option.is_some re_global_and_mutable_opt then
            SSet.iter
              (fun v ->
                add_var_refs_to_tbl
                  !(ctx.global_var_refs_tbl)
                  v
                  (Option.get re_global_and_mutable_opt))
              !vars_in_le
        (* add_var_refs_to_tbl !(ctx.global_var_refs_tbl) !vars_in_le *)
      | Unop (op, e) ->
        let e_global_opt = get_globals_from_expr env ctx e ~track_refs:true in
        if Option.is_some e_global_opt then
          let e_global = Option.get e_global_opt in
          let e_ty = Tast_env.print_ty env (Tast.get_type e) in
          (match op with
          | Ast_defs.Uincr
          | Ast_defs.Udecr
          | Ast_defs.Upincr
          | Ast_defs.Updecr ->
            (* Distinguish directly writing to static variables from writing to a variable that has references to static variables. *)
            if is_expr_static env e then
              raise_global_access_error
                env
                p
                fun_name
                e_ty
                e_global
                GlobalAccessCheck.DefiniteGlobalWrite
            else if has_global_write_access e then
              raise_global_access_error
                env
                p
                fun_name
                e_ty
                e_global
                GlobalAccessCheck.PossibleGlobalWriteViaReference
          | _ -> ())
      | Call (_, _, tpl, _) ->
        (* Check if a global variable is used as the parameter. *)
        List.iter tpl ~f:(fun (pk, ((ty, pos, _) as expr)) ->
            let e_global_opt =
              match pk with
              | Ast_defs.Pinout _ ->
                get_globals_from_expr env ctx expr ~track_refs:true
              | Ast_defs.Pnormal ->
                get_global_and_mutable_from_expr env ctx expr ~track_refs:true
            in
            if Option.is_some e_global_opt then
              raise_global_access_error
                env
                pos
                fun_name
                (Tast_env.print_ty env ty)
                (Option.get e_global_opt)
                GlobalAccessCheck.PossibleGlobalWriteViaFunctionCall)
      | New (_, _, el, _, _) ->
        List.iter el ~f:(fun ((ty, pos, _) as expr) ->
            let e_global_opt =
              get_global_and_mutable_from_expr env ctx expr ~track_refs:true
            in
            if Option.is_some e_global_opt then
              raise_global_access_error
                env
                pos
                fun_name
                (Tast_env.print_ty env ty)
                (Option.get e_global_opt)
                GlobalAccessCheck.PossibleGlobalWriteViaFunctionCall)
      | _ -> ());
      super#on_expr (env, (ctx, fun_name)) te
  end

(* Determine if a file is enabled for global access check *)
let global_access_check_enabled_on_file tcopt file =
  let enabled_paths =
    TypecheckerOptions.global_access_check_files_enabled tcopt
  in
  let path = "/" ^ Relative_path.suffix file in
  List.exists enabled_paths ~f:(fun prefix ->
      String_utils.string_starts_with path prefix)

(* Determine if a function is enabled for global access check *)
let global_access_check_enabled_on_function tcopt function_name =
  let enabled_functions =
    TypecheckerOptions.global_access_check_functions_enabled tcopt
  in
  SSet.mem function_name enabled_functions

(* The global access check is turned on if:
 * the given file or function is enabled for global access check,
 * and at least one of (global_write_check, global_read_check) is turned on. *)
let global_access_check_enabled tcopt file function_name =
  (TypecheckerOptions.global_access_check_on_write tcopt
  || TypecheckerOptions.global_access_check_on_read tcopt)
  && (global_access_check_enabled_on_file tcopt file
     || global_access_check_enabled_on_function tcopt function_name)

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_method_ env m =
      let class_name = Tast_env.get_self_id env in
      let (_, method_name) = m.m_name in
      let full_name =
        match class_name with
        | Some s -> s ^ "::" ^ method_name
        | _ -> method_name
      in
      (* Class name starts with '\' or ';' *)
      let full_name =
        String.sub full_name ~pos:1 ~len:(String.length full_name - 1)
      in
      let tcopt = Tast_env.get_tcopt env in
      let file = Tast_env.get_file env in
      if global_access_check_enabled tcopt file full_name then
        visitor#on_method_ (env, (current_ctx, full_name)) m

    method! at_fun_def env f =
      let (_, function_name) = f.fd_fun.f_name in
      (* Function name starts with '\'*)
      let function_name =
        String.sub function_name ~pos:1 ~len:(String.length function_name - 1)
      in
      let tcopt = Tast_env.get_tcopt env in
      let file = Tast_env.get_file env in
      if global_access_check_enabled tcopt file function_name then
        visitor#on_fun_def (env, (current_ctx, function_name)) f
  end
