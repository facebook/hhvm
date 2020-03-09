(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

include Aast_defs

[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

(* This is the current notion of type in the typed AST.
 * In future we might want to reconsider this and define a new representation
 * that omits type inference artefacts such as type variables and lambda
 * identifiers.
 *)
type ty = Typing_defs.locl_ty

type possibly_enforced_ty = Typing_defs.locl_possibly_enforced_ty

type decl_ty = Typing_defs.decl_ty

type reactivity = Typing_defs.reactivity

type mutability_env = Typing_mutability_env.mutability_env

type type_param_mutability = Typing_defs.param_mutability

type val_kind = Typing_defs.val_kind

let pp_ty = Pp_type.pp_locl_ty

let show_ty = Pp_type.show_locl_ty

let pp_decl_ty = Pp_type.pp_decl_ty

let show_decl_ty = Pp_type.show_decl_ty

let pp_reactivity fmt r = Pp_type.pp_reactivity fmt r

let show_reactivity r = Pp_type.show_reactivity r

let show_mutability_env _ = "<mutability-env>"

let pp_mutability_env fmt _ = Format.fprintf fmt "<mutability-env>"

let show_param_mutability = Pp_type.show_param_mutability

let pp_type_param_mutability fmt v =
  Format.fprintf fmt "%s" (show_param_mutability v)

type saved_env = {
  tcopt: TypecheckerOptions.t; [@opaque]
  inference_env: Typing_inference_env.t;
  tpenv: Type_parameter_env.t;
  reactivity: reactivity;
  local_mutability: mutability_env;
  fun_mutable: type_param_mutability option;
  condition_types: decl_ty SMap.t;
  pessimize: bool;
}
[@@deriving show]

type func_body_ann =
  (* True if there are any UNSAFE blocks *)
  | HasUnsafeBlocks
  | NoUnsafeBlocks
[@@deriving eq, show]

type program = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.program
[@@deriving show]

type def = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.def

type expr = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.expr

type expr_ = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.expr_

type stmt = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.stmt

type block = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.block

type class_ = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.class_

type class_id = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.class_id

type type_hint = ty Aast.type_hint

type targ = ty Aast.targ

type class_get_expr =
  (Pos.t * ty, func_body_ann, saved_env, ty) Aast.class_get_expr

type class_typeconst =
  (Pos.t * ty, func_body_ann, saved_env, ty) Aast.class_typeconst

type user_attribute =
  (Pos.t * ty, func_body_ann, saved_env, ty) Aast.user_attribute

type fun_ = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.fun_

type file_attribute =
  (Pos.t * ty, func_body_ann, saved_env, ty) Aast.file_attribute

type fun_def = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.fun_def

type fun_param = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.fun_param

type func_body = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.func_body

type method_ = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.method_

type method_redeclaration =
  (Pos.t * ty, func_body_ann, saved_env, ty) Aast.method_redeclaration

type class_var = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.class_var

type class_tparams =
  (Pos.t * ty, func_body_ann, saved_env, ty) Aast.class_tparams

type class_const = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.class_const

type tparam = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.tparam

type typedef = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.typedef

type record_def = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.record_def

type gconst = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.gconst

type pu_enum = (Pos.t * ty, func_body_ann, saved_env, ty) Aast.pu_enum

let empty_saved_env tcopt : saved_env =
  {
    tcopt;
    inference_env = Typing_inference_env.empty_inference_env;
    tpenv = Type_parameter_env.empty;
    reactivity = Typing_defs.Nonreactive;
    local_mutability = Local_id.Map.empty;
    fun_mutable = None;
    condition_types = SMap.empty;
    pessimize = false;
  }

(* Used when an env is needed in codegen.
 * TODO: (arkumar,wilfred,thomasjiang) T42509373 Fix when when needed
 *)
let dummy_saved_env = empty_saved_env GlobalOptions.default

let dummy_type_hint (hint : hint option) : ty * hint option =
  (Typing_defs.mk (Typing_reason.Rnone, Typing_defs.Tdynamic), hint)

(* Helper function to create an annotation for a typed and positioned expression.
 * Do not construct this tuple directly - at some point we will build
 * some abstraction in so that we can change the representation (e.g. put
 * further annotations on the expression) as we see fit.
 *)
let make_expr_annotation p ty : Pos.t * ty = (p, ty)

(* Helper function to create a typed and positioned expression.
 * Do not construct this triple directly - at some point we will build
 * some abstraction in so that we can change the representation (e.g. put
 * further annotations on the expression) as we see fit.
 *)
let make_typed_expr p ty te : expr = (make_expr_annotation p ty, te)

(* Get the position of an expression *)
let get_position (((p, _), _) : expr) = p

(* Get the type of an expression *)
let get_type (((_, ty), _) : expr) = ty

let nast_converter =
  object
    inherit [_] Aast.map

    method on_'ex _ (p, _ex) = p

    method on_'fb _ fb =
      match fb with
      | HasUnsafeBlocks -> Nast.NamedWithUnsafeBlocks
      | _ -> Nast.Named

    method on_'en _ _ = ()

    method on_'hi _ _ = ()
  end

let to_nast p = nast_converter#on_program () p

let to_nast_expr (tast : expr) : Nast.expr = nast_converter#on_expr () tast

let to_nast_class_id_ cid = nast_converter#on_class_id_ () cid
