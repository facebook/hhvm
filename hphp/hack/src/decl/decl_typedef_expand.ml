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

type cyclic_td_usage = {
  td_name: string;
  decl_pos: Pos_or_decl.t;
      (** Position of type alias usage that caused the cycle *)
}

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
    ?(force_expand = false) visited ctx r name ty_argl :
    decl_ty * cyclic_td_usage list =
  let (td_tparams, expanded_type, cycles) =
    expand_typedef_ ~force_expand visited ctx name
  in
  let subst = Decl_instantiate.make_subst td_tparams ty_argl in
  (Decl_instantiate.instantiate subst (mk (r, expanded_type)), cycles)

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
    decl_tparam list * decl_ty_ * cyclic_td_usage list =
  let td =
    unsafe_opt (Decl_entry.to_option @@ Decl_provider.get_typedef ctx name)
  in
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
  (* We don't want our visibility logic to depend on the filename of the caller,
     so the best we can do is determine visibility just based on td_vis. *)
  let should_expand =
    (not (SSet.mem name visited))
    && (force_expand
       ||
       match td_vis with
       | Aast.CaseType
       | Aast.OpaqueModule
       | Aast.Opaque ->
         false
       | Aast.Transparent -> true)
  in
  if should_expand then
    let visited = SSet.add name visited in
    let (ty, cycles) = expand_ visited ctx td_type in
    (td_tparams, get_node ty, cycles)
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
    (td_tparams, Tnewtype (name, tyl, cstr), [])

(** [expand_ visited ctx ty] traverses the type tree of [ty] and recursively expands all its transparent type alias.
    E.g. if `type B<Tb> = Map<int, Tb>`, [expand_ visited ctx (Vec<B<Ta>>)] returns `Vec<Map<int, Ta>>`.

    Parameters:
    - ty: the type to be recursed into to find all the transparent type aliases and expand them
    - visited: set of type alias names; used to detect cycles

    So this is just a decl_ty -> decl_ty mapper where the only interesting case is Tapply. *)
and expand_ visited ctx (ty : decl_ty) : decl_ty * cyclic_td_usage list =
  let (r, ty_) = deref ty in
  match ty_ with
  | Tapply ((_pos, name), tyl) ->
    if is_typedef ctx name then
      let (tyl, cycles1) =
        List.unzip @@ List.map tyl ~f:(expand_ visited ctx)
      in
      let (ty, cycles2) = expand_and_instantiate visited ctx r name tyl in
      (ty, List.concat cycles1 @ cycles2)
    else
      let (tyl, cycles) = List.unzip @@ List.map tyl ~f:(expand_ visited ctx) in
      (mk (r, Tapply ((_pos, name), tyl)), List.concat cycles)
  | Tthis -> failwith "should never happen"
  | (Tmixed | Twildcard | Tnonnull | Tdynamic | Tprim _ | Tgeneric _ | Tany _)
    as x ->
    (mk (r, x), [])
  | Trefinement (ty, rs) ->
    let (cycles, rs) =
      Class_refinement.fold_map
        (fun cycles1 ty ->
          let (ty, cycles2) = expand_ visited ctx ty in
          (cycles1 @ cycles2, ty))
        []
        rs
    in
    (mk (r, Trefinement (ty, rs)), cycles)
  | Tunion tyl ->
    let (tyl, cycles) = List.unzip @@ List.map tyl ~f:(expand_ visited ctx) in
    (mk (r, Tunion tyl), List.concat cycles)
  | Tintersection tyl ->
    let (tyl, cycles) = List.unzip @@ List.map tyl ~f:(expand_ visited ctx) in
    (mk (r, Tintersection tyl), List.concat cycles)
  | Taccess (ty, id) ->
    let (ty, cycles) = expand_ visited ctx ty in
    (mk (r, Taccess (ty, id)), cycles)
  | Toption ty ->
    let (ty, cycles) = expand_ visited ctx ty in
    (match get_node ty with
    | Toption _ as y -> (mk (r, y), cycles)
    | _ -> (mk (r, Toption ty), cycles))
  | Tlike ty ->
    let (ty, err) = expand_ visited ctx ty in
    (mk (r, Tlike ty), err)
  | Ttuple tyl ->
    let (tyl, cycles) = List.unzip @@ List.map tyl ~f:(expand_ visited ctx) in
    (mk (r, Ttuple tyl), List.concat cycles)
  | Tshape { s_origin = _; s_unknown_value = shape_kind; s_fields = fdm } ->
    let (cycles, fdm) =
      ShapeFieldMap.map_env
        (fun cycles1 ty ->
          let (ty, cycles2) = expand_ visited ctx ty in
          (cycles1 @ cycles2, ty))
        []
        fdm
    in
    ( mk
        ( r,
          (* TODO(shapes) Should this be resetting the origin? *)
          Tshape
            {
              s_origin = Missing_origin;
              s_unknown_value = shape_kind;
              (* TODO(shapes) should call expand_ on s_unknown_value *)
              s_fields = fdm;
            } ),
      cycles )
  | Tvec_or_dict (ty1, ty2) ->
    let (ty1, err1) = expand_ visited ctx ty1 in
    let (ty2, err2) = expand_ visited ctx ty2 in
    (mk (r, Tvec_or_dict (ty1, ty2)), err1 @ err2)
  | Tfun ft ->
    let tparams = ft.ft_tparams in
    let (params, cycles1) =
      List.unzip
      @@ List.map ft.ft_params ~f:(fun param ->
             let (ty, cycles) = expand_ visited ctx param.fp_type in
             ({ param with fp_type = ty }, cycles))
    in
    let (ret, cycles2) = expand_ visited ctx ft.ft_ret in
    let (tparams, cycles3) =
      List.unzip
      @@ List.map tparams ~f:(fun t ->
             let (constraints, cycles) =
               List.unzip
               @@ List.map t.tp_constraints ~f:(fun (ck, ty) ->
                      let (ty, cycles) = expand_ visited ctx ty in
                      ((ck, ty), cycles))
             in
             ({ t with tp_constraints = constraints }, List.concat cycles))
    in
    let (where_constraints, cycles4) =
      List.unzip
      @@ List.map ft.ft_where_constraints ~f:(fun (ty1, ck, ty2) ->
             let (ty1, cycles1) = expand_ visited ctx ty1 in
             let (ty2, cycles2) = expand_ visited ctx ty2 in
             ((ty1, ck, ty2), cycles1 @ cycles2))
    in
    ( mk
        ( r,
          Tfun
            {
              ft with
              ft_params = params;
              ft_ret = ret;
              ft_tparams = tparams;
              ft_where_constraints = where_constraints;
            } ),
      List.concat (cycles1 @ cycles3 @ cycles4) @ cycles2 )
  | Tnewtype (name, tyl, ty) ->
    let (tyl, cycles1) = List.unzip @@ List.map tyl ~f:(expand_ visited ctx) in
    let (ty, cycles2) = expand_ visited ctx ty in
    (mk (r, Tnewtype (name, tyl, ty)), List.concat cycles1 @ cycles2)

let expand_typedef ?(force_expand = false) ctx r name ty_argl =
  let visited = SSet.empty in
  let (ty, _) =
    expand_and_instantiate ~force_expand visited ctx r name ty_argl
  in
  ty

let expand_typedef_with_error ?(force_expand = false) ctx r name =
  let visited = SSet.empty in
  let (_, expanded_type, cycles) =
    expand_typedef_ ~force_expand visited ctx name
  in
  (mk (r, expanded_type), cycles)
