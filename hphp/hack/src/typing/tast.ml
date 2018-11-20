(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* This is the current notion of type in the typed AST.
 * In future we might want to reconsider this and define a new representation
 * that omits type inference artefacts such as type variables and lambda
 * identifiers.
 *)
type ty = Typing_defs.locl Typing_defs.ty
let pp_ty fmt ty = Pp_type.pp_ty () fmt ty
let show_ty ty = Pp_type.show_ty () ty

type saved_env = {
  tcopt : TypecheckerOptions.t [@opaque];
  tenv : ty IMap.t;
  subst : int IMap.t;
  tpenv : Type_parameter_env.t;
} [@@deriving show]

let empty_saved_env tcopt : saved_env = {
  tcopt;
  tenv = IMap.empty;
  subst = IMap.empty;
  tpenv = SMap.empty;
}

(* Typed AST.
 *
 * We re-use the NAST, but annotate expressions with position *and* type, not
 * just position.
 *
 * Going forward, we will need further annotations
 * such as type arguments to generic methods and `new`, annotations on locals
 * at merge points to make flow typing explicit, bound type parameters for
 * `instanceof` refinements, and possibly other features.
 *
 * Ideally we should record anything that is computed by the type inference
 * algorithm that can't be deduced again cheaply from the embedded type.
 *
 *)
module Annotations = struct
  module ExprAnnotation = struct
    type t = Pos.t * ty [@@deriving show]
  end

  module EnvAnnotation = struct
    type t = saved_env [@@deriving show]
  end
end

module TypeAndPosAnnotatedAST = Aast.AnnotatedAST(Annotations)

include TypeAndPosAnnotatedAST

(* Helper function to create an annotation for a typed and positioned expression.
 * Do not construct this tuple directly - at some point we will build
 * some abstraction in so that we can change the representation (e.g. put
 * further annotations on the expression) as we see fit.
 *)
let make_expr_annotation p ty : Annotations.ExprAnnotation.t = (p, ty)

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

module NastMapper = Aast_mapper.MapAnnotatedAST(Annotations)(Nast.Annotations)

let nast_mapping_env =
  NastMapper.{
    map_env_annotation = (fun _ -> ());
    map_expr_annotation = fst;
  }

let to_nast program =
  NastMapper.map_program
    ~map_env_annotation:(fun _ -> ())
    ~map_expr_annotation:fst
    program

let to_nast_expr =
  NastMapper.map_expr nast_mapping_env

let to_nast_class_id_ =
  NastMapper.map_class_id_ nast_mapping_env
