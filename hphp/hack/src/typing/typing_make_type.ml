(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
module SN = Naming_special_names
module Reason = Typing_reason
module Nast = Aast

let class_type r name tyl =
  mk (r, Tclass ((Reason.to_pos r, name), nonexact, tyl))

let prim_type r t = mk (r, Tprim t)

(* Make a negation type *)
let neg r neg_t =
  match neg_t with
  (* Represent the negation of Tnull as Tnonnull, instead of Tneg Tnull *)
  | Neg_prim Nast.Tnull -> mk (r, Tnonnull)
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

let string r = prim_type r Nast.Tstring

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

let void r = prim_type r Nast.Tvoid

let null r = prim_type r Nast.Tnull

let nonnull r = mk (r, Tnonnull)

let dynamic r = mk (r, Tdynamic)

let like r ty = mk (r, Tlike ty)

let locl_like r ty =
  if is_dynamic ty then
    ty
  else
    match get_node ty with
    | Tprim Aast.Tnoreturn
    | Tany _ ->
      ty
    | Tunion tys when List.exists Typing_defs.is_dynamic tys -> ty
    | _ -> mk (r, Tunion [dynamic r; ty])

let supportdyn r ty =
  match get_node ty with
  | Tnewtype (c, _, _) when String.equal c SN.Classes.cSupportDyn -> ty
  | Tunion [] -> ty
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

let open_shape r map = shape r (mixed r) map

let supportdyn_mixed ?(mixed_reason = Reason.Rnone) r =
  supportdyn r (mixed mixed_reason)

let hh_formatstring r ty =
  mk (r, Tnewtype (SN.Classes.cHHFormatString, [ty], mixed r))

let resource r = prim_type r Nast.Tresource

let tyvar r v = mk (r, Tvar v)

let generic ?(type_args = []) r n = mk (r, Tgeneric (n, type_args))

let this r = mk (r, Tgeneric (SN.Typehints.this, []))

let taccess r ty id = mk (r, Taccess (ty, id))

let nullable : type a. a Reason.t_ -> a ty -> a ty =
 fun r ty ->
  (* Cheap avoidance of double nullable *)
  match get_node ty with
  | Toption _ as ty_ -> mk (r, ty_)
  | Tunion [] -> null r
  | _ -> mk (r, Toption ty)

let apply r id tyl = mk (r, Tapply (id, tyl))

let tuple r tyl = mk (r, Ttuple tyl)

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

let unenforced ty = { et_type = ty; et_enforced = Unenforced }

let enforced ty = { et_type = ty; et_enforced = Enforced }

let has_member r ~name ~ty ~class_id ~explicit_targs =
  ConstraintType
    (mk_constraint_type
       ( r,
         Thas_member
           {
             hm_name = name;
             hm_type = ty;
             hm_class_id = class_id;
             hm_explicit_targs = explicit_targs;
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
  let r = Reason.Rdefault_capability p in
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
  let r = Reason.Rdefault_capability p in
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

let default_capability_unsafe p : locl_ty = mixed (Reason.Rhint p)
