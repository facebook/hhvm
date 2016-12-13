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
open Core

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

let rec log_type_list env tyl =
  match tyl with
    | [] -> ()
    | [ty] ->
      lprintf (Normal Green) "%s" (Typing_print.debug_with_tvars env ty)
    | ty::tyl ->
      lprintf (Normal Green) "%s, " (Typing_print.debug_with_tvars env ty);
      log_type_list env tyl

let log_local_types env =
  let local_types_with_history = Env.merge_locals_and_history env.Env.lenv in
  indentEnv "local_types" (fun () ->
    Local_id.Map.iter begin fun id (all_types, new_type, expr_id) ->
      lnewline();
      lprintf (Bold Green) "%s[#%d]: "
        (Local_id.get_name id) (Local_id.to_int id);
      lprintf (Normal Green) "%s" (Typing_print.debug_with_tvars env new_type);
      lprintf (Normal Green) " [history: ";
      log_type_list env all_types;
      lprintf (Normal Green) "] [eid: %s]" (Ident.debug expr_id) end
    local_types_with_history)

let log_tpenv env =
  let tparams = Env.get_generic_parameters env in
  if tparams != [] then
  indentEnv "tpenv" (fun () ->
    List.iter tparams ~f:begin fun tparam ->
      let lower = Env.get_lower_bounds env tparam in
      let upper = Env.get_upper_bounds env tparam in
      lnewline ();
      (if lower != []
      then (log_type_list env lower; lprintf (Normal Green) " <: "));
      lprintf (Bold Green) "%s" tparam;
      (if upper != []
      then (lprintf (Normal Green) " <: "; log_type_list env upper))
        end)

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

let log_position p =
  let n =
    match Pos.Map.get p !iterations with
    | None -> iterations := Pos.Map.add p 1 !iterations; 1
    | Some n -> iterations := Pos.Map.add p (n+1) !iterations; n+1 in
  indentEnv (Pos.string (Pos.to_absolute p)
    ^ (if n = 1 then "" else "[" ^ string_of_int n ^ "]"))

(* Log the environment: local_types, subst, tenv and tpenv *)
let hh_show_env p env =
  log_position p
    (fun () ->
       log_local_types env;
       log_fake_members env;
       log_env_diff (!lastenv) env;
       log_tpenv env);
  lastenv := env

(* Log the type of an expression *)
let hh_show p env ty =
  let s = Typing_print.debug env ty in
  log_position p
    (fun () ->
       lprintf (Normal Green) "%s" s; lnewline ())

let log_type p env message ty =
  let s = Typing_print.debug_with_tvars env ty in
  log_position p
    (fun () ->
       lprintf (Bold Green) "%s: " message;
       lprintf (Normal Green) "%s" s;
       lnewline ())

let log_types p env pairs =
  log_position p
    (fun () ->
       List.iter pairs ~f:(fun (message, ty) ->
         let s = Typing_print.debug_with_tvars env ty in
         lprintf (Bold Green) "%s: " message;
         lprintf (Normal Green) "%s"  s;
         lnewline ()))
