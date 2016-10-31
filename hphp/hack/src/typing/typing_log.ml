(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Env = Typing_env

open Tty

(*****************************************************************************)
(* Logging type inference environment                                        *)
(*****************************************************************************)

(* Eventual input to Tty.cprint *)
let logBuffer = ref []
let indentLevel = ref 0
let accumulatedLength = ref 0
let lnewline () =
  match !logBuffer with
  | [] -> ()
  | _ ->
    begin
      cprint ~color_mode:Color_Auto
        ((Normal White, String.make (2 * !indentLevel) ' ') ::
           List.rev ((Normal White, "\n") :: !logBuffer));
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

let indentEnv message f =
  lnewline ();
  lprintf (Normal Yellow) "%s" message;
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
  indentEnv "subst(changes)" (fun () ->
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
  indentEnv "tenv(changes)" (fun () ->
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

let log_local_types env =
  let lenv = env.Env.lenv in
  let local_types = lenv.Env.local_types in
  indentEnv "local_types" (fun () ->
    Local_id.Map.iter begin fun id (_all_types, new_type, _expr_id) ->
      lnewline();
      lprintf (Bold Green) "%s: " (Local_id.get_name id);
      lprintf (Normal Green) "%s" (Typing_print.debug_with_tvars env new_type)
  end local_types)

let log_tpenv env =
  let rec dump_tys xl =
    match xl with
    | [] -> ()
    | [ty] ->
      lprintf (Normal Green) "%s" (Typing_print.debug_with_tvars env ty)
    | ty::tyl ->
      lprintf (Normal Green) "%s," (Typing_print.debug_with_tvars env ty);
      dump_tys tyl in
  let tparams = Env.get_generic_parameters env in
  if tparams != [] then
  indentEnv "tpenv" (fun () ->
    List.iter begin fun tparam ->
      let lower = Env.get_lower_bounds env tparam in
      let upper = Env.get_upper_bounds env tparam in
      lnewline ();
      (if lower != []
      then (dump_tys lower; lprintf (Normal Green) " <: "));
      lprintf (Bold Green) "%s" tparam;
      (if upper != []
      then (lprintf (Normal Green) " <: "; dump_tys upper))
        end tparams)

(* Log the environment: local_types, subst, tenv and tpenv *)
let hh_show_env p env =
  let n =
    match Pos.Map.get p !iterations with
    | None -> iterations := Pos.Map.add p 1 !iterations; 1
    | Some n -> iterations := Pos.Map.add p (n+1) !iterations; n+1 in
  indentEnv (Pos.string (Pos.to_absolute p) ^ "[" ^ string_of_int n ^ "]")
    (fun () ->
      log_local_types env;
      log_env_diff (!lastenv) env;
      log_tpenv env);
  lastenv := env

(* Log the type of an expression *)
let hh_show p env ty =
  let s = Typing_print.debug env ty in
  lprintf (Normal Yellow) "%s" (Pos.string (Pos.to_absolute p)); lnewline ();
  lprintf (Normal Green) "  %s" s; lnewline ()
