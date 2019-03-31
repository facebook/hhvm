(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_defs
module Env = Typing_env
module Pr = Typing_print
module TySet = Typing_set
open Env
open Tty

(*****************************************************************************)
(* Logging type inference environment                                        *)
(*****************************************************************************)

(* Eventual input to Tty.cprint *)
let out_channel = ref stdout
let logBuffer = ref []
let indentLevel = ref 0
let accumulatedLength = ref 0
let lnewline () =
  match !logBuffer with
  | [] -> ()
  | _ ->
    begin
      cprint ~out_channel:!out_channel ~color_mode:Color_Auto
        ((Normal White, String.make (2 * !indentLevel) ' ') ::
           List.rev ((Normal White, "\n") :: !logBuffer));
      Out_channel.flush !out_channel;
      accumulatedLength := 0;
      logBuffer := []
    end

let lprintf c =
  Printf.ksprintf (fun s ->
    let len = String.length s in
    logBuffer := (c,s) :: !logBuffer;
    if !accumulatedLength + len > 80
    then lnewline ()
    else accumulatedLength := !accumulatedLength + len)

let lnewline_open () =
  lnewline ();
  indentLevel := !indentLevel + 1

let lnewline_close () =
  lnewline ();
  indentLevel := !indentLevel - 1

let indentEnv ?(color=Normal Yellow) message f =
  lnewline ();
  lprintf color "%s" message;
  lnewline_open ();
  f ();
  lnewline_close ()

(* Most recent environment. We only display diffs *)
let lastenv =ref (Env.empty TypecheckerOptions.default
  Relative_path.default None)
let iterations: int Pos.Map.t ref = ref Pos.Map.empty

(* Universal representation of an environment, for pretty-printing and delta computation
 *)
type value =
| Bool of bool
| Atom of string
| List of value list
| Set of SSet.t
| Map of value SMap.t

let make_map l =
  Map (SMap.of_list l)

let filter_missing l =
  List.filter_map l
    (fun (k,v) -> match v with None -> None | Some v -> Some(k,v))

let bool_as_value v = Bool v
let string_as_value s = Atom s
let smap_as_value f m = Map (SMap.map f m)

let pos_as_value p = string_as_value (Pos.string (Pos.to_absolute p))

(* Universal representation of a delta between values
 *)
type delta =
| Updated of value (* For bools, atoms, lists, replaced sets, replaced maps *)
| Unchanged
(* Set has had some elements removed, some added *)
| Set_delta of { added: SSet.t; removed: SSet.t }
(* Map has some new keys, some removed keys, and deltas to existing keys.
 * All other keys assumed to be unchanged.
 *)
| Map_delta of { added: value SMap.t; removed: SSet.t; changed: delta SMap.t }

let rec compute_value_delta (oldval: value) (newval: value): delta =
  match oldval, newval with
  | Bool b1, Bool b2 -> if b1=b2 then Unchanged else Updated newval
  | Atom s1, Atom s2 -> if s1=s2 then Unchanged else Updated newval
  | Set s1, Set s2 ->
    let added = SSet.diff s2 s1 in
    let removed = SSet.diff s1 s2 in
    if SSet.is_empty added && SSet.is_empty removed
    then Unchanged
    else Set_delta {added; removed }
  | List l1, List l2 ->
    if l1 = l2 then Unchanged else Updated newval
  | Map m1, Map m2 ->
    let removed =
      SMap.fold (fun i _ s ->
        match SMap.get i m2 with
        | None -> SSet.add i s
        | Some _ -> s) m1 SSet.empty in
    let added =
      SMap.fold (fun i v m ->
        match SMap.get i m1 with
        | None -> SMap.add i v m
        | _ -> m) m2 SMap.empty in
    let changed =
      SMap.fold (fun i oldx m ->
        match SMap.get i m2 with
        | None -> m
        | Some newx ->
          match compute_value_delta oldx newx with
          | Unchanged -> m
          | d -> SMap.add i d m)
        m1 SMap.empty in
    if SSet.is_empty removed && SMap.is_empty added && SMap.is_empty changed
    then Unchanged
    else Map_delta { removed; added; changed; }
    (* Type has changed! *)
  | _, _ ->
    Updated newval

let is_leaf_value v =
  match v with
  | Atom _ | Bool _ | List _ -> true
  | Map m when SMap.is_empty m -> true
  | Set _ -> true
  | _ -> false

let log_key key =
  lprintf (Normal Yellow) "%s" key

let log_sset s =
  match SSet.elements s with
  | [] -> lprintf (Normal Green) "{}"
  | [s] -> lprintf (Normal Green) "{%s}" s
  | s::ss ->
    lprintf (Normal Green) "{";
    lprintf (Normal Green) "%s" s;
    List.iter ss (fun s -> lprintf (Normal Green) ",%s" s);
    lprintf (Normal Green) "}"

let rec log_value value =
  match value with
  | Atom s -> lprintf (Normal Green) "%s" s
  | Bool b -> lprintf (Normal Green) "%s" (if b then "true" else "false")
  | List [] -> lprintf (Normal Green) "[]"
  | List (v::vs) ->
    lprintf (Normal Green) "[";
    log_value v;
    List.iter vs (fun v -> lprintf (Normal Green) ","; log_value v);
    lprintf (Normal Green) "]"
  | Map m ->
    if SMap.is_empty m then lprintf (Normal Green) "{}"
    else SMap.iter (log_key_value "") m
  | Set s ->
    log_sset s

and log_key_value prefix k v =
  if is_leaf_value v
  then begin
    lprintf (Normal Green) "%s" prefix;
    log_key k;
    lprintf (Normal Yellow) " ";
    log_value v;
    lnewline ()
  end
  else begin
    lnewline ();
    lprintf (Normal Green) "%s" prefix;
    log_key k;
    lnewline_open ();
    log_value v;
    lnewline_close ()
  end

let is_leaf_delta d =
  match d with
  | Updated v -> is_leaf_value v
  (* | Added d | Removed d | Updated d -> is_leaf_delta d*)
  | _ -> false

let rec log_delta delta =
  match delta with
  | Updated v -> log_value v
  | Unchanged -> ()
  | Set_delta { added; removed } ->
    if not (SSet.is_empty added)
    then begin lprintf (Bold Green) " += "; log_sset added end;
    if not (SSet.is_empty removed)
    then begin lprintf (Bold Red) " -= "; log_sset removed end
  | Map_delta { added; removed; changed } ->
    SSet.iter (fun k -> lprintf (Bold Red) "-"; log_key k; lnewline ()) removed;
    SMap.iter (log_key_value "+") added;
    SMap.iter log_key_delta changed

and log_key_delta k d =
  if is_leaf_delta d
  then begin
    log_key k;
    lprintf (Normal Yellow) " ";
    log_delta d;
    lnewline ()
  end
  else begin
    lnewline ();
    log_key k;
    lprintf (Normal Yellow) " ";
    lnewline_open ();
    log_delta d;
    lnewline_close ()
  end

let type_as_value env ty = Atom (Typing_print.debug env ty)

let return_info_as_value env return_info =
    let Typing_env_return_info.
      {return_type; return_disposable; return_mutable; return_explicit;
       return_void_to_rx; } = return_info in
    make_map [
      "return_type", type_as_value env return_type;
      "return_disposable", Bool return_disposable;
      "return_mutable", Bool return_mutable;
      "return_explicit", Bool return_explicit;
      "return_void_to_rx", Bool return_void_to_rx
    ]

let local_id_as_string id =
  Printf.sprintf "%s[#%d]" (Local_id.get_name id) (Local_id.to_int id)

let local_id_map_as_value f m =
  Map (Local_id.Map.fold (fun id x m ->
    SMap.add (local_id_as_string id) (f x) m) m SMap.empty)

let continuations_map_as_value f m =
  Map (Typing_continuations.Map.fold (fun k x m ->
    SMap.add (Typing_continuations.to_string k) (f x) m) m SMap.empty)

let local_as_value env (ty, _expr_id) =
  type_as_value env ty

let local_types_as_value env (local_types: local_types) =
  continuations_map_as_value (local_id_map_as_value (local_as_value env)) local_types

let log_position p ?function_name f =
  let n =
    match Pos.Map.get p !iterations with
    | None -> iterations := Pos.Map.add p 1 !iterations; 1
    | Some n -> iterations := Pos.Map.add p (n+1) !iterations; n+1 in
  (* If we've hit this many iterations then something must have gone wrong
   * so let's not bother spewing to the log *)
  if n > 10000 then ()
  else
    indentEnv ~color:(Bold Yellow) (Pos.string (Pos.to_absolute p)
      ^ (if n = 1 then "" else "[" ^ string_of_int n ^ "]")
      ^ (match function_name with None -> "" | Some n -> " {" ^ n ^ "}")) f

let log_subtype_prop ?(do_normalize = false) env message prop =
  lprintf (Tty.Bold Tty.Green) "%s: " message;
  lprintf (Tty.Normal Tty.Green) "%s"
    (Typing_print.subtype_prop ~do_normalize env prop);
  lnewline ()

let tenv_as_value env tenv =
  Map (IMap.fold (fun i x m ->
    SMap.add (Printf.sprintf "#%d" i) (Atom (Typing_print.full env x)) m) tenv SMap.empty)

let subst_as_value subst =
  Map (IMap.fold (fun i x m ->
    SMap.add (Printf.sprintf "#%d" i) (Atom (Printf.sprintf "#%d" x)) m) subst SMap.empty)

let tyset_as_value env tys =
  Set (TySet.fold (fun t s -> SSet.add (Typing_print.full env t) s) tys SSet.empty)
let tyvar_info_as_value env tvinfo =
  let {
    tyvar_pos;
    eager_solve_fail;
    appears_covariantly;
    appears_contravariantly;
    type_constants;
    lower_bounds;
    upper_bounds } = tvinfo in
  make_map [
    "tyvar_pos", pos_as_value tyvar_pos;
    "eager_solve_fail", bool_as_value eager_solve_fail;
    "appears_covariantly", bool_as_value appears_covariantly;
    "appears_contravariantly", bool_as_value appears_contravariantly;
    "lower_bounds", tyset_as_value env lower_bounds;
    "upper_bounds", tyset_as_value env upper_bounds;
    "type_constants", smap_as_value (fun (_, ty) -> type_as_value env ty) type_constants;
  ]
let tvenv_as_value env tvenv =
  Map (IMap.fold (fun i x m ->
    SMap.add (Printf.sprintf "#%d" i) (tyvar_info_as_value env x) m) tvenv SMap.empty)
let tyvars_stack_as_value tyvars_stack =
  List (List.map tyvars_stack (fun (_, l) ->
  List (List.map l (fun i -> Atom (Printf.sprintf "#%d" i)))))

let fake_members_as_value fake_members =
  let {
    last_call;
    invalid;
    valid
    } = fake_members in
  make_map ([
    "invalid", Set invalid;
    "valid", Set valid;
  ] @ filter_missing [
    "last_call", Option.map last_call pos_as_value
  ])

let reify_kind_as_value k =
  string_as_value (
    match k with
    | Nast.Erased -> "erased"
    | Nast.SoftReified -> "soft_reified"
    | Nast.Reified -> "reified")

let tparam_info_as_value env tpinfo =
  let Type_parameter_env.{
    lower_bounds;
    upper_bounds;
    reified;
    enforceable;
    newable;
  } = tpinfo in
  make_map [
    "lower_bounds", tyset_as_value env lower_bounds;
    "upper_bounds", tyset_as_value env upper_bounds;
    "reified", reify_kind_as_value reified;
    "enforceable", bool_as_value enforceable;
    "newable", bool_as_value newable;
  ]

let tpenv_as_value env tpenv =
  Map (SMap.fold (fun name tpinfo m ->
    SMap.add name (tparam_info_as_value env tpinfo) m) tpenv SMap.empty)

let local_mutability_as_value local_mutability =
  local_id_map_as_value
    (fun m -> Atom (Typing_mutability_env.to_string m)) local_mutability

let local_id_set_as_value s =
  Set (Local_id.Set.fold (fun id s -> SSet.add (local_id_as_string id) s) s SSet.empty)

let fun_kind_to_string k =
  match k with
  | Ast.FSync -> "normal"
  | Ast.FAsync -> "async"
  | Ast.FGenerator -> "generator"
  | Ast.FAsyncGenerator -> "async generator"
  | Ast.FCoroutine -> "coroutine"

let rec reactivity_to_string env r =
  match r with
  | Nonreactive -> "nonreactive"
  | Local opt_ty ->
    "local" ^ (match opt_ty with None -> "" | Some ty -> " " ^ Typing_print.debug env ty)
  | Shallow opt_ty ->
    "shallow" ^ (match opt_ty with None -> "" | Some ty -> " " ^ Typing_print.debug env ty)
  | Reactive opt_ty ->
    "reactive" ^ (match opt_ty with None -> "" | Some ty -> " " ^ Typing_print.debug env ty)
  | MaybeReactive r ->
    "maybereactive " ^ reactivity_to_string env r
  | RxVar opt_r ->
    "rxvar" ^ (match opt_r with None -> "" | Some r -> " " ^ reactivity_to_string env r)
let lenv_as_value env lenv =
  let {
    fake_members;
    local_types;
    tpenv;
    local_using_vars;
    local_reactive;
    local_mutability
    } = lenv in
  make_map [
    "fake_members", fake_members_as_value fake_members;
    "local_types", local_types_as_value env local_types;
    "tpenv", tpenv_as_value env tpenv;
    "local_mutability", local_mutability_as_value local_mutability;
    "local_using_vars", local_id_set_as_value local_using_vars;
    "local_reactive", string_as_value (reactivity_to_string env local_reactive);
  ]

let subtype_prop_as_value env prop =
  Atom (Typing_print.subtype_prop ~do_normalize:false env prop)

let todo_as_value todo =
  Atom (Printf.sprintf "%d" (List.length todo))

let param_as_value env (ty, mode) =
  let ty_str = Typing_print.debug env ty in
  match mode with
  | FPnormal -> Atom ty_str
  | FPinout -> Atom (Printf.sprintf "[inout]%s" ty_str)
  | FPref -> Atom (Printf.sprintf "[ref]%s" ty_str)

let genv_as_value env genv =
  let {
    tcopt = _;
    return;
    params;
    condition_types;
    parent_id;
    parent;
    self_id;
    self;
    static;
    fun_kind;
    fun_mutable;
    anons = _;
    file = _;
  } = genv in
  make_map ([
    "return", return_info_as_value env return;
    "params", local_id_map_as_value (param_as_value env) params;
    "condition_types", smap_as_value (type_as_value env) condition_types;
    "parent_id", string_as_value parent_id;
    "parent", type_as_value env parent;
    "self_id", string_as_value self_id;
    "self", type_as_value env self;
    "static", bool_as_value static;
    "fun_kind", string_as_value (fun_kind_to_string fun_kind);
  ] @
  match fun_mutable with
  | None -> []
  | Some pm -> ["fun_mutable", string_as_value (Pp_type.show_param_mutability pm)]
  )
let env_as_value env =
  let {
    function_pos;
    pos;
    outer_pos;
    outer_reason = _;
    tenv;
    subst;
    lenv;
    genv;
    decl_env = _;
    todo;
    checking_todos;
    in_loop;
    in_try;
    in_case;
    inside_constructor;
    inside_ppl_class;
    global_tpenv;
    subtype_prop;
    log_levels = _;
    tvenv;
    tyvars_stack;
    allow_wildcards; } = env in
  make_map [
    "pos", pos_as_value pos;
    "outer_pos", pos_as_value outer_pos;
    "function_pos", pos_as_value function_pos;
    "tvenv", tvenv_as_value env tvenv;
    "tenv", tenv_as_value env tenv;
    "subst", subst_as_value subst;
    "tyvars_stack", tyvars_stack_as_value tyvars_stack;
    "lenv", lenv_as_value env lenv;
    "genv", genv_as_value env genv;
    "checking_todos", bool_as_value checking_todos;
    "in_loop", bool_as_value in_loop;
    "in_try", bool_as_value in_try;
    "in_case", bool_as_value in_case;
    "inside_constructor", bool_as_value inside_constructor;
    "inside_ppl_class", bool_as_value inside_ppl_class;
    "global_tpenv", tpenv_as_value env global_tpenv;
    "subtype_prop", subtype_prop_as_value env subtype_prop;
    "todo", todo_as_value todo;
    "allow_wildcards", bool_as_value allow_wildcards;
  ]

let log_env_diff p ?function_name old_env new_env =
  let value = env_as_value new_env in
  let old_value = env_as_value old_env in
  let d = compute_value_delta old_value value in
  match d with
  | Unchanged -> ()
  | _ ->
    log_position p ?function_name (fun () -> log_delta d)

(* Log the environment: local_types, subst, tenv and tpenv *)
let hh_show_env p env =
  let old_env = !lastenv in
  lastenv := env;
  log_env_diff p old_env env

let _ =
  Env.set_env_log_function (fun pos name old_env env ->
    log_env_diff pos ~function_name:name old_env env)

(* Log the type of an expression *)
let hh_show p env ty =
  let s1 = Pr.with_blank_tyvars (fun () -> Pr.debug env ty) in
  let s2_opt = Typing_print.constraints_for_type env ty in
  log_position p
    (fun () ->
       lprintf (Normal Green) "%s" s1;
       lnewline ();
       match s2_opt with
       | None -> ()
       | Some s2 -> (lprintf (Normal Green) "%s" s2; lnewline ()))

(* Simple type of possible log data *)
type log_structure =
| Log_head of string * log_structure list
| Log_type of string * Typing_defs.locl Typing_defs.ty

let log_with_level env key level log_f =
  if Env.get_log_level env key >= level then log_f ()
  else ()

let log_types p env items =
  log_position p
    (fun () ->
      let rec go items =
        List.iter items (fun item ->
          match item with
          | Log_head (message, items) ->
            indentEnv ~color:(Normal Yellow) message (fun () -> go items)
          | Log_type (message, ty) ->
            let s = Typing_print.debug env ty in
            lprintf (Bold Green) "%s: " message;
            lprintf (Normal Green) "%s" s;
            lnewline ()) in
      go items)

let log_prop ?(do_normalize = false) level p message env prop =
  log_with_level env "prop" level (fun () ->
    log_position p (fun () -> log_subtype_prop ~do_normalize env message prop))

let log_new_tvar env p tvar message =
  log_with_level env "prop" 2 (fun () ->
    log_types p env [Log_head (message, [Log_type ("type variable", tvar)])])

let log_tparam_instantiation env p tparam tvar =
  let message = Printf.sprintf "Instantiating type parameter %s with" (snd tparam.tp_name) in
  log_new_tvar env p tvar message

let log_new_tvar_for_new_object env p tvar cname tparam =
  let message = Printf.sprintf
    "Creating new type var for type parameter %s while instantiating object %s"
    (snd tparam.tp_name) (snd cname) in
  log_new_tvar env p tvar message

let log_new_tvar_for_tconst env p tvar tconstid tvar_for_tconst =
  let message = Printf.sprintf "Creating new type var for #%d::%s" tvar tconstid in
  log_new_tvar env p tvar_for_tconst message

let log_new_tvar_for_tconst_access env p tvar class_name tconst =
  let message = Printf.sprintf "Creating type var with the same constraints as %s::%s"
    class_name tconst in
  log_new_tvar env p tvar message

let increment_feature_count env s =
  if GlobalOptions.tco_language_feature_logging (Env.get_tcopt env)
  then Measure.sample s 1.0
