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

let indentEnv ?(color=Normal Yellow) message f =
  lnewline ();
  lprintf color "%s" message;
  lnewline ();
  indentLevel := !indentLevel + 1;
  f ();
  lnewline ();
  indentLevel := !indentLevel - 1

(* Most recent environment. We only display diffs *)
let lastenv =ref (Env.empty TypecheckerOptions.default
  Relative_path.default None)
let iterations: int Pos.Map.t ref = ref Pos.Map.empty

(* Log all changes to subst *)
let log_subst_diff oldSubst newSubst =
  indentEnv (Printf.sprintf
    "subst(changes; old size = %d; new size = %d; size change = %d)"
    (IMap.cardinal oldSubst) (IMap.cardinal newSubst)
    (IMap.cardinal newSubst - IMap.cardinal oldSubst))
    (fun () ->
  begin
    IMap.iter (fun n n' ->
      match IMap.get n oldSubst with
      | None ->
        begin
          lprintf (Bold Green) "#%d: " n;
          lprintf (Normal Green) "#%d; " n'
        end
      | Some n'' ->
        if n'=n''
        then ()
        else
          begin
            lprintf (Bold Green) "#%d: " n';
            lprintf (Normal Green) "#%d; " n'
          end
    ) newSubst;
    IMap.iter (fun n _n' ->
      if IMap.mem n newSubst then ()
      else lprintf (Normal Red) "#%d deleted; " n) oldSubst
  end)

(* Log all changes to tenv *)
let log_tenv_diff oldEnv newEnv =
  indentEnv (Printf.sprintf
    "tenv(changes; old size = %d; new size = %d; size change = %d)"
    (IMap.cardinal oldEnv.Env.tenv) (IMap.cardinal newEnv.Env.tenv)
    (IMap.cardinal newEnv.Env.tenv - IMap.cardinal oldEnv.Env.tenv))
    (fun () ->
  begin
    IMap.iter (fun n t ->
      match IMap.get n oldEnv.Env.tenv with
      | None ->
        begin
          lprintf (Bold Green) "#%d: " n;
          lprintf (Normal Green) "%s; " (Typing_print.full newEnv t)
        end
      | Some t' ->
        if t=t'
        then ()
        else
          begin
            lprintf (Bold Green) "#%d: " n;
            lprintf (Normal Green) "%s; " (Typing_print.full newEnv t')
          end
    ) newEnv.Env.tenv;
    IMap.iter (fun n _ ->
      if IMap.mem n newEnv.Env.tenv then ()
      else lprintf (Normal Red) "#%d deleted; " n) oldEnv.Env.tenv
  end)

(* Dump the diff between oldEnv and newEnv. TODO: lenv component *)
let log_env_diff oldEnv newEnv =
  begin
    log_subst_diff oldEnv.Env.subst newEnv.Env.subst;
    log_tenv_diff oldEnv newEnv;
    lnewline ()
  end

let rec log_type_list env tyl =
  match tyl with
    | [] -> ()
    | [ty] ->
      lprintf (Normal Green) "%s" (Typing_print.debug env ty)
    | ty::tyl ->
      lprintf (Normal Green) "%s, " (Typing_print.debug env ty);
      log_type_list env tyl

let log_continuation env name cont =
  indentEnv (Typing_continuations.to_string name) (fun () ->
    Local_id.Map.iter begin fun id (type_, expr_id) ->
      lnewline();
      lprintf (Bold Green) "%s[#%d]: "
        (Local_id.get_name id) (Local_id.to_int id);
      lprintf (Normal Green) "%s" (Typing_print.debug env type_);
      lprintf (Normal Green) " [eid: %s]" (Ident.debug expr_id) end
    cont)

let log_local_types env =
  indentEnv "local_types" (fun () ->
    Typing_continuations.Map.iter
      (log_continuation env)
      env.Env.lenv.Env.local_types)

let log_using_vars env =
  let using_vars = env.Env.lenv.Env.local_using_vars in
  if not (Local_id.Set.is_empty using_vars) then
  indentEnv "using_vars" (fun () ->
    Local_id.Set.iter (fun lvar ->
      lprintf (Normal Green) "%s " (Local_id.get_name lvar))
      using_vars)

let log_return_type env =
  indentEnv "return_type" (fun () ->
    let Typing_env_return_info.
      {return_type; return_disposable; return_mutable; return_explicit;
       return_void_to_rx; } = Env.get_return env in
    lprintf (Normal Green) "%s%s%s%s%s"
      (Typing_print.debug env return_type)
      (if return_disposable then " (disposable)" else "")
      (if return_mutable then " (mutable_return)" else "")
      (if return_explicit then " (explicit)" else "")
      (if return_void_to_rx then " (void_to_rx)" else "")
  )

let log_tpenv env =
  let tparams = Env.get_generic_parameters env in
  if not (List.is_empty tparams) then
  indentEnv "tpenv" (fun () ->
    List.iter tparams ~f:begin fun tparam ->
      let lower = Typing_set.elements (Env.get_lower_bounds env tparam) in
      let upper = Typing_set.elements (Env.get_upper_bounds env tparam) in
      lnewline ();
      (if not (List.is_empty lower)
      then (log_type_list env lower; lprintf (Normal Green) " <: "));
      lprintf (Bold Green) "%s" tparam;
      (if not (List.is_empty upper)
      then (lprintf (Normal Green) " <: "; log_type_list env upper))
        end)

let log_tvenv env =
  indentEnv "tvenv" (fun () ->
    IMap.iter begin fun var
      Env.{ lower_bounds; upper_bounds;
        appears_covariantly; appears_contravariantly; eager_solve_fail; _ } ->
      let lower = Typing_set.elements lower_bounds in
      let upper = Typing_set.elements upper_bounds in
      lnewline ();
      (if not (List.is_empty lower)
      then (log_type_list env lower; lprintf (Normal Green) " <: "));
      lprintf (Bold Green) "%s%s%s#%d"
        (if eager_solve_fail then "(solve_fail) " else "")
        (if appears_covariantly then "+" else "")
        (if appears_contravariantly then "-" else "")
        var;
      (if not (List.is_empty upper)
      then (lprintf (Normal Green) " <: "; log_type_list env upper))
      end env.Env.tvenv)

let log_tyvars env =
  indentEnv "tyvars_stack" (fun () ->
    lprintf (Normal Green) "%s"
      (String.concat ~sep:"/" (List.map ~f:(fun vars -> "{" ^ String.concat ~sep:","
        (List.map ~f:(fun i -> Printf.sprintf "#%d" i) vars) ^ "}")
        env.Env.tyvars_stack)))

let log_fake_members env =
  let lenv = env.Env.lenv in
  let fakes = lenv.Env.fake_members in
  indentEnv "fake_members" (fun () ->
    (match fakes.Env.last_call with
    | None -> ()
    | Some p ->
      begin
        lprintf (Normal Green) "last_call: %s" (Pos.string (Pos.to_absolute p));
        lnewline ()
      end);
    lprintf (Normal Green) "invalid:";
    SSet.iter (lprintf (Normal Green) " %s") fakes.Env.invalid ;
    lnewline ();
    lprintf (Normal Green) "valid:";
    SSet.iter (lprintf (Normal Green) " %s") fakes.Env.valid;
    lnewline ())

let log_position p f =
  let n =
    match Pos.Map.get p !iterations with
    | None -> iterations := Pos.Map.add p 1 !iterations; 1
    | Some n -> iterations := Pos.Map.add p (n+1) !iterations; n+1 in
  (* If we've hit this many iterations then something must have gone wrong
   * so let's not bother spewing to the log *)
  if n > 10000 then ()
  else
    indentEnv (Pos.string (Pos.to_absolute p)
      ^ (if n = 1 then "" else "[" ^ string_of_int n ^ "]")) f

let log_subtype_prop ?(do_normalize = false) env message prop =
  lprintf (Tty.Bold Tty.Green) "%s: " message;
  lprintf (Tty.Normal Tty.Green) "%s"
    (Typing_print.subtype_prop ~do_normalize env prop);
  lnewline ()

(* Log the environment: local_types, subst, tenv and tpenv *)
let hh_show_env p env =
  log_position p
    (fun () ->
       log_local_types env;
       log_using_vars env;
       log_fake_members env;
       log_return_type env;
       log_env_diff (!lastenv) env;
       log_tpenv env;
       log_tvenv env;
       log_tyvars env;
       log_subtype_prop env "subtype_prop" env.Env.subtype_prop);
  lastenv := env

(* Log the type of an expression *)
let hh_show p env ty =
  let s1 = Typing_print.debug env ty in
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
