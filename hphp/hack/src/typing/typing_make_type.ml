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
  mk (r, Tclass ((Reason.to_pos r, name), Nonexact, tyl))

let prim_type r t = mk (r, Tprim t)

let neg r t =
  match t with
  | Nast.Tnull -> mk (r, Tnonnull)
  | _ -> mk (r, Tneg t)

let traversable r ty = class_type r SN.Collections.cTraversable [ty]

let keyed_traversable r kty vty =
  class_type r SN.Collections.cKeyedTraversable [kty; vty]

let keyed_container r kty vty =
  class_type r SN.Collections.cKeyedContainer [kty; vty]

let shape r kind map = mk (r, Tshape (kind, map))

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

let varray_or_darray ~unification r kty vty =
  if unification then
    mk (r, Tvec_or_dict (kty, vty))
  else
    mk (r, Tvarray_or_darray (kty, vty))

let varray ~unification r ty =
  if unification then
    vec r ty
  else
    mk (r, Tvarray ty)

let darray ~unification r kty vty =
  if unification then
    dict r kty vty
  else
    mk (r, Tdarray (kty, vty))

let int r = prim_type r Nast.Tint

let bool r = prim_type r Nast.Tbool

let string r = prim_type r Nast.Tstring

let float r = prim_type r Nast.Tfloat

let num r = prim_type r Nast.Tnum

let arraykey r = prim_type r Nast.Tarraykey

let void r = prim_type r Nast.Tvoid

let null r = prim_type r Nast.Tnull

let nonnull r = mk (r, Tnonnull)

let dynamic r = mk (r, Tdynamic)

let like r ty = mk (r, Tlike ty)

let mixed r = mk (r, Toption (nonnull r))

let resource r = prim_type r Nast.Tresource

let ty_object r = mk (r, Tobject)

let tyvar r v = mk (r, Tvar v)

let generic ?(type_args = []) r n = mk (r, Tgeneric (n, type_args))

let this r = mk (r, Tgeneric (SN.Typehints.this, []))

let err r = mk (r, Terr)

let taccess r ty id = mk (r, Taccess (ty, id))

let new_type r name tyl = mk (r, Tnewtype (name, tyl, mixed r))

let nullable_decl r ty =
  (* Cheap avoidance of double nullable *)
  match get_node ty with
  | Toption _ as ty_ -> mk (r, ty_)
  | _ -> mk (r, Toption ty)

let nullable_locl r ty =
  (* Cheap avoidance of double nullable *)
  match get_node ty with
  | Toption _ as ty_ -> mk (r, ty_)
  | Tunion [] -> null r
  | _ -> mk (r, Toption ty)

let nothing r = mk (r, Tunion [])

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

let default_capability p : locl_ty = nothing (Reason.Rdefault_capability p)

(* ^ TODO(coeffects) after implementing lower bounds on const ctx/type, do:
  intersection
    Reason.Rnone
    [
      class_type
        Reason.Rnone
        Naming_special_names.Capabilities.accessStaticVariable
        [];
      class_type Reason.Rnone Naming_special_names.Capabilities.writeProperty [];
      class_type Reason.Rnone Naming_special_names.Capabilities.output [];
    ]
 *)
