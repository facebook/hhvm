(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type Value = sig
  type t =
    | Str of string  (** String values *)
    | Unit  (** Represents the unit value *)
end

module type LanguageServiceProvider = sig
  type t

  val env : t -> Tast_env.t

  val find_class : ctx:t -> Aast.class_name -> Nast.class_

  val find_static_method : ctx:t -> Nast.class_ -> Aast.pstring -> Nast.method_

  val find_function : ctx:t -> Aast.sid -> Nast.fun_def

  module Class : sig
    type ctx = t

    type t

    val find : ctx:ctx -> Aast.class_name -> t

    val constructor : t -> Typing_defs.class_elt option

    val methods : t -> (string * Typing_defs.class_elt) list

    val static_methods : t -> (string * Typing_defs.class_elt) list

    val fields : t -> (string * Typing_defs.class_elt) list

    val static_fields : t -> (string * Typing_defs.class_elt) list
  end
end

module type PromptCtxProvider = sig
  type t

  type data

  module type Class = sig
    val constructor : t -> Aast.class_name -> data

    val methods : t -> Aast.class_name -> data

    val static_methods : t -> Aast.class_name -> data

    val fields : t -> Aast.class_name -> data

    val static_fields : t -> Aast.class_name -> data
  end
end
