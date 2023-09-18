(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types
module Env = Typing_env
module Map = Type_mapper_generic

type result = env * locl_ty

let fresh_env env = env

class type type_mapper_type =
  object
    inherit [env] Map.type_mapper_type
  end

class shallow_type_mapper : type_mapper_type =
  object
    inherit [env] Map.shallow_type_mapper
  end

class deep_type_mapper : type_mapper_type =
  object
    inherit [env] Map.deep_type_mapper
  end

class virtual tunion_type_mapper =
  object
    inherit [env] Map.tunion_type_mapper
  end

class virtual tinter_type_mapper =
  object
    inherit [env] Map.tinter_type_mapper
  end

(* Mixin that expands type variables. *)
class virtual tvar_expanding_type_mapper =
  object (this)
    method on_tvar env (r : Reason.t) n =
      let (env, ty) = Env.get_type env r n in
      if is_tyvar ty then
        (env, ty)
      else
        this#on_type env ty

    method virtual on_type : env -> locl_ty -> result
  end

(** Type mapper which only maps types under combinations of unions, options and intersections. *)
class union_inter_type_mapper =
  object
    inherit shallow_type_mapper

    inherit! tunion_type_mapper

    inherit! tinter_type_mapper

    inherit! tvar_expanding_type_mapper
  end
