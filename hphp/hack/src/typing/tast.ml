(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This is the current notion of type in the typed AST.
 * In future we might want to reconsider this and define a new representation
 * that omits type inference artefacts such as type variables and lambda
 * identifiers.
 *)
type ty = Typing_defs.locl Typing_defs.ty
let pp_ty fmt ty = Format.pp_print_string fmt (Typing_print.suggest ty)

(* Typed AST.
 * We re-use the NAST but annotate expressions with position *and*
 * type not just position. The type is optional, the idea being that *if*
 * present then it should be correct according to the typing rules, and if
 * absent it should be cheaply and uniquely derivable from the subexpressions,
 * given particular types for local variables.
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
module AnnotationType = struct
  type t = Pos.t * ty option
  let pp fmt (_, ty) = match ty with
    | None -> Format.pp_print_string fmt "None"
    | Some ty ->
      Format.pp_print_string fmt "(Some ";
      pp_ty fmt ty;
      Format.pp_print_string fmt ")"
end
module TypeAndPosAnnotatedAST = Nast.AnnotatedAST(AnnotationType)

include TypeAndPosAnnotatedAST

(* Helper function to create a typed and positioned expression.
 * Do not construct this triple directly - at some point we will build
 * some abstraction in so that we can change the representation (e.g. put
 * further annotations on the expression) as we see fit.
 *)
let make_typed_expr p ty te : expr = ((p, Some ty), te)

(* Given types for locals, the type of the expression should be cheaply
 * and uniquely derivable.
 *)
let make_implicitly_typed_expr p te : expr = ((p, None), te)

(* Get the position of an expression *)
let get_position (((p, _), _) : expr) = p

(* If the expression has a type, return it *)
let get_type_exn (((_, tyopt), _) : expr) =
  match tyopt with
  | None -> failwith "get_type_exn: no type"
  | Some ty -> ty
