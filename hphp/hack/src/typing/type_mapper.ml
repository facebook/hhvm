(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]
open Core_kernel
[@@@warning "+33"]
open Common
open Typing_defs

module Env = Typing_env
module Reason = Typing_reason
module ShapeMap = Nast.ShapeMap

(* Mapping environment threaded through all function calls:
 * - typing environment *)
type env = Env.env

(* Mapping result - updated environment, mapped type *)
type result = env * locl ty

let fresh_env env = env

class type type_mapper_type = object
  method on_tvar : env -> Reason.t -> int -> result
  method on_infinite_tvar : env -> Reason.t -> int -> result
  method on_tmixed : env -> Reason.t -> result
  method on_tnonnull : env -> Reason.t -> result
  method on_tdynamic : env -> Reason.t -> result
  method on_tany : env -> Reason.t -> result
  method on_terr : env -> Reason.t -> result
  method on_tanon : env -> Reason.t -> locl fun_arity -> Ident.t -> result
  method on_tprim : env -> Reason.t -> Nast.tprim -> result
  method on_tarraykind_akany : env -> Reason.t -> result
  method on_tarraykind_akempty : env -> Reason.t -> result
  method on_tarraykind_akvec : env -> Reason.t -> locl ty -> result
  method on_tarraykind_akvarray : env -> Reason.t -> locl ty -> result
  method on_tarraykind_akmap : env -> Reason.t -> locl ty -> locl ty -> result
  method on_tarraykind_akdarray :
    env -> Reason.t -> locl ty -> locl ty -> result
  method on_tarraykind_akshape :
    env -> Reason.t -> (locl ty * locl ty) ShapeMap.t -> result
  method on_tarraykind_aktuple :
    env -> Reason.t -> locl ty IMap.t -> result
  method on_tvarray_or_darray : env -> Reason.t -> locl ty -> result
  method on_ttuple : env -> Reason.t -> locl ty list -> result
  method on_tunresolved : env -> Reason.t -> locl ty list -> result
  method on_toption : env -> Reason.t -> locl ty -> result
  method on_tfun : env -> Reason.t -> locl fun_type -> result
  method on_tabstract :
    env -> Reason.t  -> abstract_kind -> locl ty option -> result
  method on_tclass : env -> Reason.t -> Nast.sid -> locl ty list -> result
  method on_tobject : env -> Reason.t -> result
  method on_tshape :
    env
      -> Reason.t
      -> shape_fields_known
      -> locl shape_field_type Nast.ShapeMap.t
      -> result

  method on_type : env -> locl ty -> result
end

(* Base type mapper implementation that doesn't recursively go into the
 * types. *)
class shallow_type_mapper: type_mapper_type = object(this)
  method on_tvar env r n = env, (r, Tvar n)
  method on_infinite_tvar = this#on_tvar
  method on_tmixed env r = env, (r, Tmixed)
  method on_tnonnull env r = env, (r, Tnonnull)
  method on_tdynamic env r = env, (r, Tdynamic)
  method on_tany env r = env, (r, Tany)
  method on_terr env r = env, (r, Terr)
  method on_tanon env r fun_arity id = env, (r, Tanon (fun_arity, id))
  method on_tprim env r p = env, (r, Tprim p)
  method on_tarraykind_akany env r = env, (r, Tarraykind AKany)
  method on_tarraykind_akempty env r = env, (r, Tarraykind AKempty)
  method on_tarraykind_akvec env r tv = env, (r, Tarraykind (AKvec tv))
  method on_tarraykind_akvarray env r tv = env, (r, Tarraykind (AKvarray tv))
  method on_tarraykind_akmap  env r tk tv =
    env, (r, Tarraykind (AKmap (tk, tv)))
  method on_tarraykind_akdarray env r tk tv =
    env, (r, Tarraykind (AKdarray (tk, tv)))
  method on_tarraykind_akshape env r fdm = env, (r, Tarraykind (AKshape fdm))
  method on_tarraykind_aktuple env r fields =
    env, (r, Tarraykind (AKtuple fields))
  method on_tvarray_or_darray env r tv =
    env, (r, Tarraykind (AKvarray_or_darray tv))
  method on_ttuple env r tyl = env, (r, Ttuple tyl)
  method on_tunresolved env r tyl = env, (r, Tunresolved tyl)
  method on_toption env r ty = env, (r, Toption ty)
  method on_tfun env r fun_type = env, (r, Tfun fun_type)
  method on_tabstract env r ak opt_ty = env, (r, Tabstract (ak, opt_ty))
  method on_tclass env r x tyl = env, (r, Tclass (x, tyl))
  method on_tobject env r = env, (r, Tobject)
  method on_tshape env r fields_known fdm = env, (r, Tshape (fields_known, fdm))

  method on_type env (r, ty) = match ty with
    | Tvar n -> this#on_tvar env r n
    | Tmixed -> this#on_tmixed env r
    | Tnonnull -> this#on_tnonnull env r
    | Tany -> this#on_tany env r
    | Terr -> this#on_terr env r
    | Tanon (fun_arity, id) -> this#on_tanon env r fun_arity id
    | Tprim p -> this#on_tprim env r p
    | Tarraykind AKany -> this#on_tarraykind_akany env r
    | Tarraykind AKempty -> this#on_tarraykind_akempty env r
    | Tarraykind AKvec tv -> this#on_tarraykind_akvec env r tv
    | Tarraykind AKvarray tv -> this#on_tarraykind_akvarray env r tv
    | Tarraykind AKmap (tk, tv) -> this#on_tarraykind_akmap env r tk tv
    | Tarraykind AKdarray (tk, tv) -> this#on_tarraykind_akdarray env r tk tv
    | Tarraykind (AKvarray_or_darray tv) ->
      this#on_tvarray_or_darray env r tv
    | Tarraykind (AKshape fdm) -> this#on_tarraykind_akshape env r fdm
    | Tarraykind (AKtuple fields) -> this#on_tarraykind_aktuple env r fields
    | Ttuple tyl -> this#on_ttuple env r tyl
    | Tunresolved tyl -> this#on_tunresolved env r tyl
    | Toption ty -> this#on_toption env r ty
    | Tfun fun_type -> this#on_tfun env r fun_type
    | Tabstract (ak, opt_ty) -> this#on_tabstract env r ak opt_ty
    | Tclass (x, tyl) -> this#on_tclass env r x tyl
    | Tdynamic -> this#on_tdynamic env r
    | Tobject -> this#on_tobject env r
    | Tshape (fields_known, fdm) -> this#on_tshape env r fields_known fdm
end

(* Mixin class - adding it to shallow type mapper creates a mapper that
 * traverses the type by going inside Tunresolved *)
class virtual tunresolved_type_mapper = object(this)
  method on_tunresolved env r tyl: result =
    let env, tyl = List.map_env env tyl (this#on_type) in
    env, (r, Tunresolved tyl)

  method virtual on_type : env -> locl ty -> result
end

(* Implementation of type_mapper that recursively visits everything in the
 * type.
 * NOTE: by default it doesn't to anything to Tvars. Include one of the mixins
 * below to specify how you want to treat type variables. *)
class deep_type_mapper = object(this)
  inherit shallow_type_mapper
  inherit! tunresolved_type_mapper

  method! on_tarraykind_akvec env r tv =
    let env, tv = this#on_type env tv in
    env, (r, Tarraykind (AKvec tv))
  method! on_tarraykind_akvarray env r tv =
    let env, tv = this#on_type env tv in
    env, (r, Tarraykind (AKvarray tv))
  method! on_tarraykind_akmap env r tk tv =
    let env, tk = this#on_type env tk in
    let env, tv = this#on_type env tv in
    env, (r, Tarraykind (AKmap (tk, tv)))
  method! on_tarraykind_akdarray env r tk tv =
    let env, tk = this#on_type env tk in
    let env, tv = this#on_type env tv in
    env, (r, Tarraykind (AKdarray (tk, tv)))
  method! on_tarraykind_akshape env r fdm =
    let env, fdm = Nast.ShapeMap.map_env begin fun env _key (tk, tv) ->
      let env, tk = this#on_type env tk in
      let env, tv = this#on_type env tv in
      env, (tk, tv)
    end env fdm in
    env, (r, Tarraykind (AKshape fdm))
  method! on_tarraykind_aktuple env r fields =
    let on_type env _key = this#on_type env in
    let env, fields = IMap.map_env on_type env fields in
    env, (r, Tarraykind (AKtuple fields))
  method! on_tvarray_or_darray env r tv =
    let env, tv = this#on_type env tv in
    env, (r, Tarraykind (AKvarray_or_darray tv))
  method! on_ttuple env r tyl =
    let env, tyl = List.map_env env tyl this#on_type in
    env, (r, Ttuple tyl)
  method! on_toption env r ty =
    let env, ty = this#on_type env ty in
    env, (r, Toption ty)
  method! on_tfun env r ft =
    let on_param env param =
      let env, ty = this#on_type env param.fp_type in
      env, { param with fp_type = ty } in
    let env, params = List.map_env env ft.ft_params on_param in
    let env, ret = this#on_type env ft.ft_ret in
    let env, arity = match ft.ft_arity with
      | Fvariadic (min, ({ fp_type = p_ty; _ } as param)) ->
        let env, p_ty = this#on_type env p_ty in
        env, Fvariadic (min, { param with fp_type = p_ty })
      | x -> env, x
    in
    env, (r, Tfun { ft with
      ft_params = params;
      ft_arity = arity;
      ft_ret = ret
    })
  method! on_tabstract env r ak cstr =
    match ak with
      | AKgeneric x ->
          let env, cstr = this#on_opt_type env cstr in
          env, (r, Tabstract (AKgeneric x, cstr))
      | AKnewtype (x, tyl) ->
          let env, tyl = List.map_env env tyl this#on_type in
          let env, cstr = this#on_opt_type env cstr in
          env, (r, Tabstract (AKnewtype (x, tyl), cstr))
      | _ ->
          let env, cstr = this#on_opt_type env cstr in
          env, (r, Tabstract (ak, cstr))
  method! on_tclass env r x tyl =
    let env, tyl = List.map_env env tyl this#on_type in
    env, (r, Tclass (x, tyl))
  method! on_tshape env r fields_known fdm =
    let env, fdm = ShapeFieldMap.map_env this#on_type env fdm in
    env, (r, Tshape (fields_known, fdm))

  method private on_opt_type env x = match x with
    | None -> env, None
    | Some x ->
       let env, x = this#on_type env x in
       env, Some x
end

(* Mixin that expands type variables. *)
class virtual tvar_expanding_type_mapper = object(this)
  method on_infinite_tvar (env : env) (r : Reason.t) (_ : int) : result =
    env, (r, Tany)

  method on_tvar env (r : Reason.t) n =
    let env, ty = Env.get_type env r n in
    this#on_type env ty

  method virtual on_type : env -> locl ty -> result
end

(* Mixin that maps across the type inside the typevar, and then changes
 * its value to the result. *)
class virtual tvar_substituting_type_mapper = object(this)
  method on_tvar (env : env) (r : Reason.t) n =
    let env, ty = Env.get_type env r n in
    let env, ty = this#on_type env ty in
    let env = Env.add env n ty in
    env, ty
  method virtual on_type : env -> locl ty -> result
end
