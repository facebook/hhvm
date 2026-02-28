(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* cf: typing_env_types_sig.mli - These files should be the same *)
open Hh_prelude
open Typing_defs

type locl_ty = Typing_defs.locl_ty

type local_id_set_t = Local_id.Set.t

type local_env = {
  per_cont_env: Typing_per_cont_env.t;
  local_using_vars: local_id_set_t;
}

type expr_tree_env = {
  dsl: Aast.class_name;
  outer_locals: Typing_local_types.t;
}

(** See the .mli file for the documentation of fields. *)
type env = {
  expression_id_provider: Expression_id.provider;
  tvar_id_provider: Tvid.provider;
  fresh_typarams: SSet.t;
  lenv: local_env;
  genv: genv;
  decl_env: Decl_env.env;
  in_loop: bool;
  in_try: bool;
  in_lambda: bool;
  in_expr_tree: expr_tree_env option;
  in_macro_splice: Typing_local_types.t option;
  checked: Tast.check_status;
  tracing_info: Decl_counters.tracing_info option;
  tpenv: Type_parameter_env.t;
  log_levels: int SMap.t;
  inference_env: Typing_inference_env.t;
  rank: int;
  check_rank: bool;
  allow_wildcards: bool;
  big_envs: (Pos.t * env) list ref;
  fun_tast_info: Tast.fun_tast_info option;
  emit_string_coercion_error: bool;
}

(** See the .mli file for the documentation of fields. *)
and genv = {
  tcopt: TypecheckerOptions.t;
  callable_pos: Pos.t;
  function_pos: Pos.t;
  readonly: bool;
  return: Typing_env_return_info.t;
  params: (locl_ty * Pos.t * locl_ty option) Local_id.Map.t;
  parent: (string * decl_ty) option;
  self: (string * locl_ty) option;
  static: bool;
  fun_kind: Ast_defs.fun_kind;
  val_kind: Typing_defs.val_kind;
  fun_is_ctor: bool;
  file: Relative_path.t;
  current_module: Ast_defs.id option;
  current_package: Aast_defs.package_membership option;
  soft_package_requirement: Ast_defs.id option;
  this_internal: bool;
  this_support_dynamic_type: bool;
  no_auto_likes: bool;
  needs_concrete: bool;
}

let initial_local tpenv =
  {
    per_cont_env =
      Typing_per_cont_env.(initial_locals { empty_entry with tpenv });
    local_using_vars = Local_id.Set.empty;
  }

let empty ?origin ?(mode = FileInfo.Mstrict) ctx file ~droot =
  {
    expression_id_provider = Expression_id.make_provider ();
    tvar_id_provider = Tvid.make_provider ();
    fresh_typarams = SSet.empty;
    lenv = initial_local Type_parameter_env.empty;
    in_loop = false;
    in_try = false;
    in_lambda = false;
    in_expr_tree = None;
    in_macro_splice = None;
    checked = Tast.COnce;
    decl_env = { Decl_env.mode; droot; droot_member = None; ctx };
    tracing_info =
      Option.map origin ~f:(fun origin -> { Decl_counters.origin; file });
    genv =
      {
        tcopt = Provider_context.get_tcopt ctx;
        callable_pos = Pos.none;
        function_pos = Pos.none;
        readonly = false;
        return =
          {
            (* Actually should get set straight away anyway *)
            Typing_env_return_info.return_type = mk (Reason.none, Tunion []);
            return_disposable = false;
            return_ignore_readonly = false;
          };
        params = Local_id.Map.empty;
        self = None;
        static = false;
        val_kind = Other;
        parent = None;
        fun_kind = Ast_defs.FSync;
        fun_is_ctor = false;
        file;
        current_module = None;
        current_package = None;
        soft_package_requirement = None;
        this_internal = false;
        this_support_dynamic_type = false;
        no_auto_likes = false;
        needs_concrete = false;
      };
    tpenv = Type_parameter_env.empty;
    log_levels = TypecheckerOptions.log_levels (Provider_context.get_tcopt ctx);
    inference_env = Typing_inference_env.empty_inference_env;
    rank = 0;
    check_rank = false;
    allow_wildcards = false;
    big_envs = ref [];
    fun_tast_info = None;
    emit_string_coercion_error = true;
  }

let get_log_level env key =
  Option.value (SMap.find_opt key env.log_levels) ~default:0

let next_cont_opt env =
  Typing_per_cont_env.get_cont_option
    Typing_continuations.Next
    env.lenv.per_cont_env

let get_tpenv env =
  match next_cont_opt env with
  | None -> Type_parameter_env.empty
  | Some entry -> entry.Typing_per_cont_env.tpenv

let get_pos_and_kind_of_generic env name =
  match Type_parameter_env.get_with_pos name (get_tpenv env) with
  | Some r -> Some r
  | None -> Type_parameter_env.get_with_pos name env.tpenv

let get_lower_bounds env name =
  let tpenv = get_tpenv env in
  let local = Type_parameter_env.get_lower_bounds tpenv name in
  let global = Type_parameter_env.get_lower_bounds env.tpenv name in
  Typing_set.union local global

let get_upper_bounds env name =
  let tpenv = get_tpenv env in
  let local = Type_parameter_env.get_upper_bounds tpenv name in
  let global = Type_parameter_env.get_upper_bounds env.tpenv name in
  Typing_set.union local global

(* Get bounds that are both an upper and lower of a given generic *)
let get_equal_bounds env name =
  let lower = get_lower_bounds env name in
  let upper = get_upper_bounds env name in
  Typing_set.inter lower upper

let get_tparams_in_ty_and_acc env acc ty =
  let tparams_visitor env =
    object (this)
      inherit [SSet.t] Type_visitor.locl_type_visitor

      method! on_tgeneric acc _ s =
        (* Not traversing args, although they may contain Tgenerics (higher kinds only) *)
        SSet.add s acc

      method! on_tvar acc r ix =
        let (_env, ty) =
          Typing_inference_env.expand_var env.inference_env r ix
        in
        match get_node ty with
        | Tvar _ -> acc
        | _ -> this#on_type acc ty
    end
  in
  (tparams_visitor env)#on_type acc ty

let get_rank { rank; _ } = rank

let increment_rank env = { env with rank = env.rank + 1; check_rank = true }

let decrement_rank env = { env with rank = env.rank - 1 }

let should_check_rank { check_rank; _ } = check_rank
