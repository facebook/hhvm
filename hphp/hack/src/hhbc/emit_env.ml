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

type t = {
  env_pipe_var: Local.t option;
  env_scope: Ast_scope.Scope.t;
  env_namespace: Namespace_env.env;
  env_needs_local_this: bool;
  env_jump_targets: Jump_targets.t;
  env_in_try: bool;
  env_allows_array_append: bool;
  env_in_rx_body: bool;
  env_call_context: string option;
}

type closure_enclosing_class_info = {
  kind: Ast_defs.class_kind;
  name: string;
  parent_class_name: string option;
}

type global_state = {
  global_explicit_use_set: SSet.t;
  global_closure_namespaces: Namespace_env.env SMap.t;
  global_closure_enclosing_classes: closure_enclosing_class_info SMap.t;
  global_functions_with_finally: SSet.t;
  global_function_to_labels_map: bool SMap.t SMap.t;
  global_lambda_rx_of_scope: Rx.t SMap.t;
}

let global_state_to_string
    ({
       global_explicit_use_set;
       global_closure_enclosing_classes;
       global_closure_namespaces;
       global_functions_with_finally;
       global_function_to_labels_map;
       global_lambda_rx_of_scope;
     } :
      global_state) : string =
  let buf = Buffer.create 1024 in
  Printf.bprintf buf "global_explicit_use_set\n";
  SSet.iter (Printf.bprintf buf "%s\n") global_explicit_use_set;
  Printf.bprintf buf "global_closure_enclosing_classes\n";
  SMap.iter
    (fun x _y -> Printf.bprintf buf "%s\n" x)
    global_closure_enclosing_classes;
  Printf.bprintf buf "global_closure_namespaces\n";
  SMap.iter (fun x _y -> Printf.bprintf buf "%s\n" x) global_closure_namespaces;
  Printf.bprintf buf "global_functions_with_finally\n";
  SSet.iter (Printf.bprintf buf "%s\n") global_functions_with_finally;
  Printf.bprintf buf "global_function_to_labels_map\n";
  SMap.iter
    begin
      fun x y ->
      Printf.bprintf buf "%s:\n" x;
      SMap.iter
        begin
          fun x y ->
          Printf.bprintf buf "  %s - %b\n" x y
        end
        y
    end
    global_function_to_labels_map;
  SMap.iter
    (fun x y ->
      Printf.bprintf
        buf
        "%s %s\n"
        x
        (Option.value (Rx.rx_level_to_attr_string y) ~default:"None"))
    global_lambda_rx_of_scope;
  Buffer.contents buf

let empty_global_state =
  {
    global_explicit_use_set = SSet.empty;
    global_closure_namespaces = SMap.empty;
    global_closure_enclosing_classes = SMap.empty;
    global_functions_with_finally = SSet.empty;
    global_function_to_labels_map = SMap.empty;
    global_lambda_rx_of_scope = SMap.empty;
  }

let is_systemlib_ = ref false

let global_state_ = ref empty_global_state

let set_is_systemlib v = is_systemlib_ := v

let is_systemlib () = !is_systemlib_

let get_explicit_use_set () = !global_state_.global_explicit_use_set

let get_closure_namespaces () = !global_state_.global_closure_namespaces

let get_closure_enclosing_classes () =
  !global_state_.global_closure_enclosing_classes

let get_functions_with_finally () = !global_state_.global_functions_with_finally

let get_function_to_labels_map () = !global_state_.global_function_to_labels_map

let get_unique_id_for_main () = "|"

let get_unique_id_for_function fn = "|" ^ snd fn.T.f_name

let get_unique_id_for_method cls mthd =
  snd cls.T.c_name ^ "|" ^ snd mthd.T.m_name

let get_lambda_rx_of_scope cd md =
  let key = get_unique_id_for_method cd md in
  SMap.find_opt key !global_state_.global_lambda_rx_of_scope
  |> Option.value ~default:Rx.NonRx

let set_global_state s = global_state_ := s

let clear_global_state () = set_global_state empty_global_state

let empty =
  {
    env_pipe_var = None;
    env_scope = Ast_scope.Scope.toplevel;
    env_namespace = Namespace_env.empty_with_default;
    env_needs_local_this = false;
    env_jump_targets = Jump_targets.empty;
    env_in_try = false;
    env_allows_array_append = false;
    env_in_rx_body = false;
    env_call_context = None;
  }

let make_class_env ast_class =
  {
    env_pipe_var = None;
    env_scope = [Ast_scope.ScopeItem.Class ast_class];
    env_namespace = ast_class.T.c_namespace;
    env_needs_local_this = false;
    env_jump_targets = Jump_targets.empty;
    env_in_try = false;
    env_allows_array_append = false;
    env_in_rx_body = false;
    env_call_context = None;
  }

let get_pipe_var env = env.env_pipe_var

let get_scope env = env.env_scope

let get_namespace env = env.env_namespace

let get_needs_local_this env = env.env_needs_local_this

let get_jump_targets env = env.env_jump_targets

let is_in_try env = env.env_in_try

let does_env_allow_array_append env = env.env_allows_array_append

let is_in_rx_body env = env.env_in_rx_body

let get_call_context env = env.env_call_context

(* Environment is second parameter so we can chain these e.g.
 *   empty |> with_scope scope |> with_namespace ns
 *)
let with_scope scope env = { env with env_scope = scope }

let with_namespace namespace env = { env with env_namespace = namespace }

let with_needs_local_this needs_local_this env =
  { env with env_needs_local_this = needs_local_this }

let with_pipe_var v env = { env with env_pipe_var = Some v }

let with_try env = { env with env_in_try = true }

let with_rx_body rx_body env = { env with env_in_rx_body = rx_body }

let with_call_context ctx env = { env with env_call_context = ctx }

let do_in_loop_body break_label continue_label ?iter env b f =
  Jump_targets.with_loop break_label continue_label iter env.env_jump_targets b
  @@ fun env_jump_targets b -> f { env with env_jump_targets } b

let do_in_switch_body end_label env s f =
  Jump_targets.with_switch end_label env.env_jump_targets s
  @@ fun env_jump_targets s -> f { env with env_jump_targets } s

let do_in_try_body finally_label env b f =
  Jump_targets.with_try finally_label env.env_jump_targets b
  @@ fun env_jump_targets b -> f { env with env_jump_targets } b

let do_in_finally_body env b f =
  Jump_targets.with_finally env.env_jump_targets b @@ fun env_jump_targets b ->
  f { env with env_jump_targets } b

let do_in_using_body finally_label env b f =
  Jump_targets.with_using finally_label env.env_jump_targets b
  @@ fun env_jump_targets b -> f { env with env_jump_targets } b

let do_function (env : t) (s : Tast.program) f =
  Jump_targets.with_function env.env_jump_targets s @@ fun env_jump_targets s ->
  f { env with env_jump_targets } s
