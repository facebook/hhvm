(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_defs_constraints
module SN = Naming_special_names
module Reason = Typing_reason
module Nast = Aast

let class_type r name tyl =
  mk (r, Tclass ((Reason.to_pos r, name), nonexact, tyl))

let prim_type r t = mk (r, Tprim t)

(* Make a negation type *)
let neg r neg_t =
  match snd neg_t with
  (* Represent the negation of Tnull as Tnonnull, instead of Tneg Tnull *)
  | IsTag NullTag -> mk (r, Tnonnull)
  | _ -> mk (r, Tneg neg_t)

let traversable r ty = class_type r SN.Collections.cTraversable [ty]

let keyed_traversable r kty vty =
  class_type r SN.Collections.cKeyedTraversable [kty; vty]

let keyed_container r kty vty =
  class_type r SN.Collections.cKeyedContainer [kty; vty]

let awaitable r ty = class_type r SN.Classes.cAwaitable [ty]

let generator r key value send =
  class_type r SN.Classes.cGenerator [key; value; send]

let async_generator r key value send =
  class_type r SN.Classes.cAsyncGenerator [key; value; send]

let async_iterator r ty = class_type r SN.Classes.cAsyncIterator [ty]

let async_keyed_iterator r ty1 ty2 =
  class_type r SN.Classes.cAsyncKeyedIterator [ty1; ty2]

let pair r ty1 ty2 = class_type r SN.Collections.cPair [ty1; ty2]

let dict r kty vty = class_type r SN.Collections.cDict [kty; vty]

let keyset r ty = class_type r SN.Collections.cKeyset [ty]

let vec r ty = class_type r SN.Collections.cVec [ty]

let any_array r kty vty = class_type r SN.Collections.cAnyArray [kty; vty]

let container r ty = class_type r SN.Collections.cContainer [ty]

let throwable r = class_type r SN.Classes.cThrowable []

let datetime r = class_type r SN.Classes.cDateTime []

let datetime_immutable r = class_type r SN.Classes.cDateTimeImmutable []

let const_vector r ty = class_type r SN.Collections.cConstVector [ty]

let const_collection r ty = class_type r SN.Collections.cConstCollection [ty]

let collection r ty = class_type r SN.Collections.cCollection [ty]

let spliceable r ty1 ty2 ty3 =
  class_type r SN.Classes.cSpliceable [ty1; ty2; ty3]

let vec_or_dict r kty vty = mk (r, Tvec_or_dict (kty, vty))

let int r = prim_type r Nast.Tint

let bool r = prim_type r Nast.Tbool

let string r = class_type r SN.Classes.cString []

(* Mirror of the classname.hhi definition. This sucks: the type of the ::class
 * constant is burned into decls at folding time so this supports the case when
 * you want to wrap a locl ty e.g. from [Typing.class_expr] in a classname.
 * TODO: eliminate the use site of this for CIexpr and make class_expr
 * produce a class type directly *)
let typename r tyl = mk (r, Tnewtype (SN.Classes.cTypename, tyl, string r))

let classname r tyl =
  mk (r, Tnewtype (SN.Classes.cClassname, tyl, typename r tyl))

let float r = prim_type r Nast.Tfloat

let num r = prim_type r Nast.Tnum

let arraykey r = prim_type r Nast.Tarraykey

let void (type ph) r : ph ty = prim_type r Nast.Tvoid

let null r = prim_type r Nast.Tnull

let nonnull r = mk (r, Tnonnull)

let dynamic r = mk (r, Tdynamic)

let is_dynamic_or_like_or_mixed ty =
  match get_node ty with
  | Tdynamic
  | Tlike _
  | Tmixed ->
    true
  | _ -> false

let like r ty =
  if is_dynamic_or_like_or_mixed ty then
    ty
  else
    match get_node ty with
    | Tapply ((_, n), [inner_ty])
      when String.equal n Naming_special_names.Classes.cSupportDyn
           && is_dynamic_or_like_or_mixed inner_ty ->
      ty
    | _ -> mk (r, Tlike ty)

let locl_like r ty =
  if Typing_defs.is_dynamic ty then
    ty
  else
    match get_node ty with
    | Tprim Aast.Tnoreturn
    | Tany _ ->
      ty
    | Tunion tys when List.exists Typing_defs.is_dynamic tys -> ty
    | _ -> mk (r, Tunion [dynamic r; ty])

(* Wrap supportdyn around a type unless it's already got it
 * or trivially supports dynamic
 *)
let supportdyn r ty =
  match get_node ty with
  | Tnewtype (c, _, _) when String.equal c SN.Classes.cSupportDyn -> ty
  | Tunion []
  | Tdynamic
  | Tprim _ ->
    ty
  | _ -> mk (r, Tnewtype (SN.Classes.cSupportDyn, [ty], ty))

let supportdyn_nonnull r = supportdyn r (nonnull r)

let mixed r = mk (r, Toption (nonnull r))

let nothing r = mk (r, Tunion [])

let shape r kind map =
  mk
    ( r,
      Tshape
        { s_origin = Missing_origin; s_unknown_value = kind; s_fields = map } )

let closed_shape r map = shape r (nothing r) map

let open_shape ~kind r map = shape r kind map

let supportdyn_mixed r = supportdyn r (mixed r)

let resource r = prim_type r Nast.Tresource

let tyvar r v = mk (r, Tvar v)

let generic r n = mk (r, Tgeneric n)

let this r = mk (r, Tgeneric SN.Typehints.this)

let taccess r ty id = mk (r, Taccess (ty, id))

let nullable : type a. a Reason.t_ -> a ty -> a ty =
 fun r ty ->
  (* Cheap avoidance of double nullable *)
  match get_node ty with
  | Toption _ as ty_ -> mk (r, ty_)
  | Tunion [] -> null r
  | _ -> mk (r, Toption ty)

let apply r id tyl = mk (r, Tapply (id, tyl))

let decl_string r = apply r (Reason.to_pos r, SN.Classes.cString) []

let tuple r tyl =
  mk
    ( r,
      Ttuple
        { t_required = tyl; t_optional = []; t_extra = Tvariadic (nothing r) }
    )

let union r tyl =
  match tyl with
  | [ty] -> ty
  | _ -> mk (r, Tunion tyl)

let intersection r tyl =
  match tyl with
  | [] -> mixed r
  | [ty] -> ty
  | _ -> mk (r, Tintersection tyl)

let function_ref r ty = mk (r, Tnewtype (SN.Classes.cFunctionRef, [ty], ty))

let label r ty_in ty_out =
  mk (r, Tnewtype (SN.Classes.cEnumClassLabel, [ty_in; ty_out], mixed r))

let has_member r ~name ~ty ~class_id ~methd =
  ConstraintType
    (mk_constraint_type
       ( r,
         Thas_member
           {
             hm_name = name;
             hm_type = ty;
             hm_class_id = class_id;
             hm_method = methd;
           } ))

let list_destructure r tyl =
  ConstraintType
    (mk_constraint_type
       ( r,
         Tdestructure
           {
             d_required = tyl;
             d_optional = [];
             d_variadic = None;
             d_kind = ListDestructure;
           } ))

let simple_variadic_splat r ty =
  ConstraintType
    (mk_constraint_type
       ( r,
         Tdestructure
           {
             d_required = [];
             d_optional = [];
             d_variadic = Some ty;
             d_kind = SplatUnpack;
           } ))

let capability r name : locl_ty = class_type r name []

let default_capability p : locl_ty =
  let r = Reason.default_capability p in
  intersection
    r
    SN.Capabilities.
      [
        class_type r writeProperty [];
        class_type r accessGlobals [];
        class_type r rxLocal [];
        class_type r systemLocal [];
        class_type r implicitPolicyLocal [];
        class_type r io [];
      ]

let default_capability_decl p : decl_ty =
  let r = Reason.default_capability p in
  intersection
    r
    SN.Capabilities.
      [
        apply r (p, writeProperty) [];
        apply r (p, accessGlobals) [];
        apply r (p, rxLocal) [];
        apply r (p, systemLocal) [];
        apply r (p, implicitPolicyLocal) [];
        apply r (p, io) [];
      ]

let default_capability_unsafe p : locl_ty = mixed (Reason.hint p)

(** Construct the default type for `__construct` *)
let default_construct (type ph) r : ph ty =
  mk
    ( r,
      Tfun
        {
          ft_tparams = [];
          ft_where_constraints = [];
          ft_params = [];
          ft_implicit_params = { capability = CapDefaults (Reason.to_pos r) };
          ft_flags =
            Typing_defs_flags.Fun.make
              Ast_defs.FSync
              ~return_disposable:false
              ~returns_readonly:false
              ~readonly_this:false
              ~support_dynamic_type:false
              ~is_memoized:false
              ~variadic:false;
          ft_ret = void r;
          ft_instantiated = true;
        } )

let make_dynamic_tfun r : locl_ty =
  mk
    ( r,
      Tfun
        {
          ft_tparams = [];
          ft_where_constraints = [];
          ft_params =
            [
              {
                fp_pos = Reason.to_pos r;
                fp_name = None;
                fp_type = dynamic r;
                fp_flags =
                  Typing_defs_flags.FunParam.make
                    ~inout:false
                    ~accept_disposable:false
                    ~is_optional:false
                    ~readonly:false
                    ~ignore_readonly_error:false
                    ~splat:true
                    ~named:false;
                fp_def_value = None;
              };
            ];
          ft_implicit_params = { capability = CapDefaults (Reason.to_pos r) };
          ft_flags =
            Typing_defs_flags.Fun.make
              Ast_defs.FSync
              ~return_disposable:false
              ~returns_readonly:false
              ~readonly_this:false
              ~support_dynamic_type:false
              ~is_memoized:false
              ~variadic:true;
          ft_ret = dynamic r;
          ft_instantiated = true;
        } )
