(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open String_utils
open Typing_defs
module Env = Typing_env
module Cls = Decl_provider.Class

let can_access_internal
    ~(env : Typing_env_types.env)
    ~(current : string option)
    ~(target : string option) =
  match (current, target) with
  | (None, None)
  | (Some _, None) ->
    `Yes
  | (None, Some m) -> `Outside m
  | (Some m_current, Some m_target) when String.equal m_current m_target ->
    (match Env.get_self_id env with
    | None -> `Yes
    | Some self ->
      (match Env.get_class env self with
      | Some cls
        when Ast_defs.is_c_trait (Cls.kind cls) && not (Cls.internal cls) ->
        `OutsideViaTrait (Cls.pos cls)
      | Some _
      | None ->
        `Yes))
  | (Some current, Some target) -> `Disjoint (current, target)

let satisfies_rule test_module_opt rule =
  match rule with
  (* A 'global' rule, so match if the module is empty (i.e. the code is not in a module) *)
  | MRGlobal -> Option.is_none test_module_opt
  (* A wildcard rule like 'a.x.*', so match if the rule matches the prefix of the module *)
  | MRPrefix rule_module_prefix ->
    (* The '*' rule matches everyone *)
    if String.length rule_module_prefix = 0 then
      true
    else (
      match test_module_opt with
      (* No module, so fail *)
      | None -> false
      (* Make sure the prefix is correct (and is followed by '.') *)
      | Some module_ ->
        let prefix_length = String.length rule_module_prefix in
        let test_length = String.length module_ in
        prefix_length = 0
        || string_starts_with module_ rule_module_prefix
           && (test_length = prefix_length
              || Char.equal module_.[prefix_length] '.')
    )
  (* An exact rule like 'a.x', so match if the name matches the module *)
  | MRExact rule_module ->
    (match test_module_opt with
    (* No module, so fail *)
    | None -> false
    (* Make sure names are exact *)
    | Some module_ -> String.equal rule_module module_)

let satisfies_rules test_module_opt rules =
  match rules with
  | None -> true
  | Some rules_list ->
    if List.length rules_list = 0 then
      false
    else
      List.exists rules_list ~f:(satisfies_rule test_module_opt)

let find_module_symbol env name_opt =
  match name_opt with
  | None -> None
  | Some name -> Some (name, Env.get_module env name)

let satisfies_import_rules env current target =
  match find_module_symbol env current with
  | None
  | Some (_, None) ->
    None
  | Some (current_module_name, Some current_module_symbol) ->
    if satisfies_rules target current_module_symbol.mdt_imports then
      None
    else
      Some (current_module_name, current_module_symbol.mdt_pos)

let satisfies_export_rules env current target =
  match find_module_symbol env target with
  | None
  | Some (_, None) ->
    None
  | Some (target_module_name, Some target_module_symbol) ->
    if satisfies_rules current target_module_symbol.mdt_exports then
      None
    else
      Some (target_module_name, target_module_symbol.mdt_pos)

let can_access_public
    ~(env : Typing_env_types.env)
    ~(current : string option)
    ~(target : string option) =
  if Option.equal String.equal current target then
    `Yes
  else
    match satisfies_import_rules env current target with
    | Some current_module -> `ImportsNotSatisfied current_module
    | None ->
      (match satisfies_export_rules env current target with
      | Some target_module -> `ExportsNotSatisfied target_module
      | None -> `Yes)

let is_class_visible (env : Typing_env_types.env) (cls : Cls.t) =
  if Cls.internal cls then
    match
      can_access_internal
        ~env
        ~current:(Env.get_current_module env)
        ~target:(Cls.get_module cls)
    with
    | `Yes -> true
    | _ -> false
  else
    true
