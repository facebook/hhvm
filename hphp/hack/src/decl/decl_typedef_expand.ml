(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs
open Utils
module MakeType = Typing_make_type

let is_typedef ctx name =
  match Naming_provider.get_type_kind ctx name with
  | Some Naming_types.TTypedef -> true
  | _ -> false

(** [expand_and_instantiate visited ctx name ty_argl] returns the full expansion of the type alias named [name] and
    performs type parameter substitution on the result using [ty_argl].
    E.g. if `type A<T> = Vec<B<T>>; type B<T> = Map<int, T>`, [expand_and_instantiate ctx "A" [string]] returns `Vec<Map<int, string>>`.

    Parameters:
    - force_expand: expand regardless of typedef visibility (default: false)
    - visited: set of type alias names. Used to detect cycles.
    - r: reason to use as part of the returned decl_ty
    - name: name of type alias to expand
    - ty_argl: list of type arguments used to substitute the expanded typedef's type parameters *)
let rec expand_and_instantiate
    ?(force_expand = false) visited ctx r name ty_argl =
  let (td_tparams, expanded_type) =
    expand_typedef_ ~force_expand visited ctx name
  in
  let subst = Decl_instantiate.make_subst td_tparams ty_argl in
  Decl_instantiate.instantiate subst (mk (r, expanded_type))

(** [expand_typedef_ visited ctx name ty_argl] returns the full expansion of the type alias named [name].
    E.g. if `type A<T> = Vec<B<T>>; type B<T> = Map<int, T>`, [expand_typedef] on "A" returns `(["T"], Vec<Map<int, T>>)`.

    Parameters:
    - force_expand: expand regardless of typedef visibility (default: false)
    - name: name of type alias to expand
    - visited: set of type alias names. Used to detect cycles.

    Returned values:
    - the list of type parameters for the expanded type alias. Useful to perform substitution.
    - the full type alias expansion

    Opaque typedefs are not expanded, regardless of the current file.
    Detects cycles, but does not raise an error - it just stops expanding instead. *)
and expand_typedef_ ?(force_expand = false) visited ctx (name : string) :
    decl_tparam list * decl_ty_ =
  let td = unsafe_opt @@ Decl_provider.get_typedef ctx name in
  let {
    td_pos;
    td_module = _;
    td_vis;
    td_tparams;
    td_type;
    td_as_constraint;
    td_super_constraint = _;
    td_is_ctx = _;
    td_attributes = _;
    td_internal = _;
    td_docs_url = _;
  } =
    td
  in
  if SSet.mem name visited then
    (td_tparams, get_node td_type)
  else
    let visited = SSet.add name visited in
    (* We don't want our visibility logic to depend on the filename of the caller,
       so the best we can do is determine visibility just based on td_vis. *)
    let should_expand =
      force_expand
      ||
      match td_vis with
      | Aast.OpaqueModule
      | Aast.Opaque ->
        false
      | Aast.Transparent -> true
    in
    if should_expand then
      (td_tparams, get_node (expand_ visited ctx td_type))
    else
      let tparam_to_tgeneric tparam =
        mk (Reason.Rhint (fst tparam.tp_name), Tgeneric (snd tparam.tp_name, []))
      in
      let (tyl : decl_ty list) = List.map td_tparams ~f:tparam_to_tgeneric in
      let cstr =
        match td_as_constraint with
        | Some cstr -> cstr
        | None ->
          let r_cstr = Reason.Rimplicit_upper_bound (td_pos, "?nonnull") in
          MakeType.mixed r_cstr
      in
      (td_tparams, Tnewtype (name, tyl, cstr))

(** [expand_ visited ctx ty] traverses the type tree of [ty] and recursively expands all its transparent type alias.
    E.g. if `type B<Tb> = Map<int, Tb>`, [expand_ visited ctx (Vec<B<Ta>>)] returns `Vec<Map<int, Ta>>`.

    Parameters:
    - ty: the type to be recursed into to find all the transparent type aliases and expand them
    - visited: set of type alias names; used to detect cycles

    So this is just a decl_ty -> decl_ty mapper where the only interesting case is Tapply. *)
and expand_ visited ctx (ty : decl_ty) : decl_ty =
  let (r, ty_) = deref ty in
  match ty_ with
  | Tapply (((_pos, name) as _id), tyl) ->
    if is_typedef ctx name then
      let tyl = List.map tyl ~f:(expand_ visited ctx) in
      expand_and_instantiate visited ctx r name tyl
    else
      let tyl = List.map tyl ~f:(expand_ visited ctx) in
      mk (r, Tapply ((_pos, name), tyl))
  | Tvar _ -> failwith "not implemented"
  | Tthis -> failwith "should never happen"
  | (Tmixed | Terr | Tnonnull | Tdynamic | Tprim _ | Tgeneric _ | Tany _) as x
    ->
    mk (r, x)
  | Trefinement (ty, rs) ->
    let ty = expand_ visited ctx ty in
    let rs = Class_refinement.map (expand_ visited ctx) rs in
    mk (r, Trefinement (ty, rs))
  | Tunion tyl ->
    let tyl = List.map tyl ~f:(expand_ visited ctx) in
    mk (r, Tunion tyl)
  | Tintersection tyl ->
    let tyl = List.map tyl ~f:(expand_ visited ctx) in
    mk (r, Tintersection tyl)
  | Taccess (ty, id) ->
    let ty = expand_ visited ctx ty in
    mk (r, Taccess (ty, id))
  | Toption ty ->
    let x = expand_ visited ctx ty in
    (match get_node x with
    | Toption _ as y -> mk (r, y)
    | _ -> mk (r, Toption x))
  | Tlike ty -> mk (r, Tlike (expand_ visited ctx ty))
  | Ttuple tyl ->
    let tyl = List.map tyl ~f:(expand_ visited ctx) in
    mk (r, Ttuple tyl)
  | Tshape (shape_kind, fdm) ->
    let fdm = ShapeFieldMap.map (expand_ visited ctx) fdm in
    mk (r, Tshape (shape_kind, fdm))
  | Tvec_or_dict (ty1, ty2) ->
    let ty1 = expand_ visited ctx ty1 in
    let ty2 = expand_ visited ctx ty2 in
    mk (r, Tvec_or_dict (ty1, ty2))
  | Tfun ft ->
    let tparams = ft.ft_tparams in
    let params =
      List.map ft.ft_params ~f:(fun param ->
          let ty = expand_possibly_enforced_ty visited ctx param.fp_type in
          { param with fp_type = ty })
    in
    let ret = expand_possibly_enforced_ty visited ctx ft.ft_ret in
    let tparams =
      List.map tparams ~f:(fun t ->
          {
            t with
            tp_constraints =
              List.map t.tp_constraints ~f:(fun (ck, ty) ->
                  (ck, expand_ visited ctx ty));
          })
    in
    let where_constraints =
      List.map ft.ft_where_constraints ~f:(fun (ty1, ck, ty2) ->
          (expand_ visited ctx ty1, ck, expand_ visited ctx ty2))
    in
    mk
      ( r,
        Tfun
          {
            ft with
            ft_params = params;
            ft_ret = ret;
            ft_tparams = tparams;
            ft_where_constraints = where_constraints;
          } )
  | Tnewtype (name, tyl, ty) ->
    let tyl = List.map tyl ~f:(expand_ visited ctx) in
    let ty = expand_ visited ctx ty in
    mk (r, Tnewtype (name, tyl, ty))

and expand_possibly_enforced_ty visited ctx et =
  { et_type = expand_ visited ctx et.et_type; et_enforced = et.et_enforced }

let expand_typedef ?(force_expand = false) ctx r name ty_argl =
  let visited = SSet.empty in
  expand_and_instantiate ~force_expand visited ctx r name ty_argl
