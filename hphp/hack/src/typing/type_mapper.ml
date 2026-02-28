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

class tvar_expanding_type_mapper =
  object (this)
    inherit deep_type_mapper

    method! on_tvar env r n =
      let (env, ty) = Env.get_type env r n in
      if Typing_defs.is_tyvar ty then
        (env, ty)
      else
        this#on_type env ty
  end
