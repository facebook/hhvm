(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This is the current notion of type in the typed AST.
 * In future we might want to reconsider this and define a new representation
 * that omits type inference artefacts such as type variables and lambda
 * identifiers.
 *)
type ty = Typing_defs.locl Typing_defs.ty

type saved_env = {
  tcopt : TypecheckerOptions.t;
  tenv : ty IMap.t;
  subst : int IMap.t;
  tpenv : Type_parameter_env.t;
}

let pp_saved_env fmt env =
  Format.fprintf fmt "@[<hv 2>{ ";

  Format.fprintf fmt "@[%s =@ " "tcopt";
  Format.fprintf fmt "<opaque>";
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "tenv";
  IMap.pp Pp_type.pp_ty fmt env.tenv;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "subst";
  IMap.pp Format.pp_print_int fmt env.subst;
  Format.fprintf fmt "@]";
  Format.fprintf fmt ";@ ";

  Format.fprintf fmt "@[%s =@ " "tpenv";
  Type_parameter_env.pp fmt env.tpenv;
  Format.fprintf fmt "@]";

  Format.fprintf fmt " }@]"

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
  module PosAndTypeAnnotation = struct
    type t = Pos.t * ty
    let pp fmt (pos, ty) =
      Format.fprintf fmt "(@[";
      Pos.pp fmt pos;
      Format.fprintf fmt ",@ ";
      Pp_type.pp_ty fmt ty;
      Format.fprintf fmt "@])"
  end

  module ExprAnnotation = PosAndTypeAnnotation
  module ClassIdAnnotation = PosAndTypeAnnotation

  module EnvAnnotation = struct
    type t = saved_env
    let pp = pp_saved_env
  end
end

module TypeAndPosAnnotatedAST = Nast.AnnotatedAST(Annotations)

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

let nast_mapping_env env =
  NastMapper.{
    env;
    map_env_annotation = (fun _ -> ());
    map_expr_annotation = (fun _ (pos, _) -> pos);
    map_class_id_annotation = (fun _ (pos, _) -> pos);
  }

let to_nast program =
  NastMapper.map_program
    ~map_env_annotation:(fun _ -> ())
    ~map_expr_annotation:(fun _ (pos, _) -> pos)
    ~map_class_id_annotation:(fun _ (pos, _) -> pos)
    program
