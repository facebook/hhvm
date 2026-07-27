(* (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary. *)

open Typing_defs_core

type merge_result =
  | Full of locl_phase ty * bool
      (** The merge collapsed to a single type (a merged shape, or [nothing] for
          the bottom row). The [bool] is the supportdyn flag. *)
  | Empty_shape of bool
      (** The merge collapsed to the empty closed shape [shape()] — the unit for
          merge (e.g. spreading only empty shapes). We cannot build the [ty] here
          because the merge site has no [Reason.t] to attach to it; the caller,
          which does have a reason in scope, constructs the [shape()] type. The
          [bool] is the supportdyn flag. *)
  | Partial of locl_phase ty list * bool
      (** The operands could not be fully merged (e.g. a residual type variable);
          the remaining element list must be kept as a splat. *)

(** Merge field descriptors: under right-most wins semantics, we have:
    - (Req _ | Opt _) , Req t -> Req t
    - Req t_l, Opt t_r        -> Req (t_l | t_r)
    - Opt t_l, Opt t_r        -> Opt (t_l | t_r)
*)
let merge_field_descs
    ~(fd_left : locl_phase shape_field_type)
    ~(fd_right : locl_phase shape_field_type)
    (env : Typing_env_types.env) :
    Typing_env_types.env * locl_phase shape_field_type =
  match (fd_left.sft_optional, fd_right.sft_optional) with
  | (_, false) ->
    (* Right-hand field is not optional so it 'wins' *)
    (env, fd_right)
  | (false, true) ->
    (* Right-hand field is optional, left-hand field is not so we take the union
       of their types and the resulting field is not optional *)
    (* TODO[mjt] Record flow for extended reason  *)
    let (env, sft_ty) = Typing_union.union env fd_left.sft_ty fd_right.sft_ty in
    (env, Typing_defs_core.{ sft_optional = false; sft_ty })
  | (true, true) ->
    (* Both fields are optional; take the union of both types with the resulting
       field also being optional *)
    (* TODO[mjt] Record flow for extended reason *)
    let (env, sft_ty) = Typing_union.union env fd_left.sft_ty fd_right.sft_ty in
    (env, { sft_optional = true; sft_ty })

let proj_field
    (fields : locl_phase shape_field_type TShapeMap.t)
    (unknown : locl_phase ty)
    (key : TShapeField.t) : locl_phase shape_field_type =
  match TShapeMap.find_opt key fields with
  | Some fd -> fd
  | None -> { sft_optional = true; sft_ty = unknown }

(** Simple shapes are shapes which contain now splats, only fields. Under
    right-most wins semantics, we right-merge each pair of fields. Any field
    which is not present in one of the shapes can equivalently be considered
    to be an optional fields with the type given by the unknown field upper bound *)
let merge_shapes_simple
    ~(shape_left : locl_phase shape_type_simple)
    ~(shape_right : locl_phase shape_type_simple)
    (env : Typing_env_types.env) :
    Typing_env_types.env * locl_phase shape_type_simple =
  (* The merged shape will contain all _keys_ *)
  let keys =
    let keys_left =
      TShapeMap.fold
        (fun k _ acc -> TShapeSet.add k acc)
        shape_left.s_fields
        TShapeSet.empty
    in
    TShapeMap.fold
      (fun k _ acc -> TShapeSet.add k acc)
      shape_right.s_fields
      keys_left
  in
  let (env, s_fields) =
    TShapeSet.fold
      (fun key (env, acc) ->
        let fd_left =
          proj_field shape_left.s_fields shape_left.s_unknown_value key
        in
        let fd_right =
          proj_field shape_right.s_fields shape_right.s_unknown_value key
        in
        let (env, fd) = merge_field_descs ~fd_left ~fd_right env in
        (env, TShapeMap.add key fd acc))
      keys
      (env, TShapeMap.empty)
  in
  let (env, s_unknown_value) =
    Typing_union.union
      env
      shape_left.s_unknown_value
      shape_right.s_unknown_value
  in
  ( env,
    {
      (* TODO[mjt] can this be improved? *)
      s_origin = Missing_origin;
      s_unknown_value;
      s_fields;
    } )

type merge_elem =
  | Empty
  | Merging of Typing_reason.t * locl_phase shape_type_simple
  | Bottom of Typing_reason.t

(** Merge adjacent simple shapes in a list. Used during shape normalization in
    localization and subtyping. If all elements are simple shapes the result
    will be a single element list which is itself a simple shape. Otherwise,
    the list will be simple shapes interspersed with spreads of other types.
    If we encounter a type which cannot be spread we raise an error and replace
    it with a fresh type variable representing the error so as to avoid
    re-raising the error later. We don't check the bounds of type parameters
    at this point so they may well lead to ill-formed shapes and errors will
    be reported during subtyping *)
let merge
    ~(on_error : Typing_error.Reasons_callback.t option)
    (elems : locl_phase ty list)
    (env : Typing_env_types.env) :
    Typing_env_types.env * Typing_error.t option * merge_result =
  let rec loop rev_elems ((merge_elem, elems, errs, sd, env) as acc) =
    match rev_elems with
    | [] -> acc
    | ty :: rev_elems ->
      (* Under sound dynamic a non-enforceable splat operand (e.g. an open
         shape, whose unknown fields are [mixed]) is localized wrapped in
         [supportdyn<...>]. Strip it so we can see the underlying shape, and
         remember that the normalized result must be wrapped back up in
         [supportdyn] to stay sound. Closed shapes with enforceable fields are
         not wrapped, so this leaves them (and the [everything_sdt=false] case)
         untouched. *)
      let (sd_elem, env, ty) = Typing_utils.strip_supportdyn env ty in
      (match (deref ty, merge_elem) with
      (* -- Merge identity -------------------------------------------------- *)
      | ((_, Tshape (Shape_simple shape_left)), _)
        when TShapeMap.is_empty shape_left.s_fields
             && Typing_utils.is_nothing env shape_left.s_unknown_value ->
        (* The empty closed shape is unit for merge *)
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Merge with bottom on rhs ---------------------------------------- *)
      | ((_, (Tshape _ | Tgeneric _ | Tvar _ | Tdynamic _)), Bottom _) ->
        (* We don't accumulate elements since the resulting type will be
           [nothing] *)
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Merge with bottom on both sides --------------------------------- *)
      | ((_, _), Bottom _) when Typing_utils.is_nothing env ty ->
        (* Spreading [nothing] onto the bottom row stays bottom; don't fall
           through to the error case below *)
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Start accumulating ---------------------------------------------- *)
      | ((reason, Tshape (Shape_simple shape)), Empty) ->
        (* Start accumulating the merged shape *)
        let merge_elem = Merging (reason, shape) in
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Merge with bottom as the left element --------------------------- *)
      | ((reason, _), (Empty | Merging _)) when Typing_utils.is_nothing env ty
        ->
        (* speading [nothing] gives us the bottom row: a row in which the
           all unknown fields are required, irrespective of their type or the
           presence of known fields. Elsewhere we don't collapse shapes with
           known required fields with type [nothing] to bottom but we HAVE to
           here since we have no other representation *)
        let merge_elem = Bottom reason in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Merge simple shapes --------------------------------------------- *)
      | ((_, Tshape (Shape_simple shape_left)), Merging (reason, shape_right))
        ->
        let (env, shape) = merge_shapes_simple ~shape_left ~shape_right env in
        let merge_elem = Merging (reason, shape) in
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Accumulate type variables and parameters ------------------------ *)
      | ((_, (Tgeneric _ | Tvar _)), Merging (shape_reason, shape)) ->
        let elems =
          let elem = mk (shape_reason, Tshape (Shape_simple shape)) in
          ty :: elem :: elems
        in
        let merge_elem = Empty in
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      | ((_, (Tgeneric _ | Tvar _)), Empty) ->
        let elems = ty :: elems in
        let merge_elem = Empty in
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Spread dynamic as an open row with dynamic as upper bound ------- *)
      | ((_, Tdynamic _), Merging (reason, shape_right)) ->
        let shape_left =
          {
            s_origin = Missing_origin;
            s_unknown_value = ty;
            s_fields = TShapeMap.empty;
          }
        in
        let (env, shape) = merge_shapes_simple ~shape_left ~shape_right env in
        let merge_elem = Merging (reason, shape) in
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      | ((reason, Tdynamic _), Empty) ->
        let shape =
          {
            s_origin = Missing_origin;
            s_unknown_value = ty;
            s_fields = TShapeMap.empty;
          }
        in
        let merge_elem = Merging (reason, shape) in
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Flatten nested splats ------------------------------------------- *)
      (* Splice the inner splat's elements in place and KEEP the current merge
         accumulator, so simple shapes adjacent across the nested-splat boundary
         still merge into one element (canonical form). Flushing the accumulator
         here would leave them as separate un-merged elements. *)
      | ((_, Tshape (Shape_splat { ss_elems })), (Empty | Merging _)) ->
        let sd = sd || sd_elem in
        let rev_elems = List.rev ss_elems @ rev_elems in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Error conditions ------------------------------------------------ *)
      | ((reason, _), Merging (shape_reason, shape)) ->
        let (env, elem_err) =
          Typing_env.fresh_type_error
            env
            (Pos_or_decl.unsafe_to_raw_pos (Typing_reason.to_pos reason))
        in
        let elems =
          let elem_shape = mk (shape_reason, Tshape (Shape_simple shape)) in
          elem_err :: elem_shape :: elems
        in
        let errs =
          Option.fold on_error ~none:errs ~some:(fun on_error ->
              let err =
                Typing_error.apply_reasons
                  ~on_error
                  (Typing_error.Secondary.Splat_not_a_shape
                     (Reason.to_pos reason))
              in
              err :: errs)
        in
        let merge_elem = Empty in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      | ((reason, _), Empty) ->
        let (env, elem_err) =
          Typing_env.fresh_type_error
            env
            (Pos_or_decl.unsafe_to_raw_pos (Typing_reason.to_pos reason))
        in
        let elems = elem_err :: elems in
        let merge_elem = Empty in
        let errs =
          Option.fold on_error ~none:errs ~some:(fun on_error ->
              let err =
                Typing_error.apply_reasons
                  ~on_error
                  (Typing_error.Secondary.Splat_not_a_shape
                     (Reason.to_pos reason))
              in
              err :: errs)
        in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      | ((reason, _), Bottom _) ->
        let errs =
          Option.fold on_error ~none:errs ~some:(fun on_error ->
              (* Still report malformed splats despite being bottom *)
              let err =
                Typing_error.apply_reasons
                  ~on_error
                  (Typing_error.Secondary.Splat_not_a_shape
                     (Reason.to_pos reason))
              in
              err :: errs)
        in
        loop rev_elems (merge_elem, elems, errs, sd, env))
  in
  let (merge_elem, elems, errs, sd, env) =
    (* Merge simple shape elements from right to left so reverse the list *)
    let rev_elems = List.rev elems in
    loop rev_elems (Empty, [], [], false, env)
  in
  let err_opt = Typing_error.multiple_opt errs in
  (* Combine the current merging element with any others; there two special
     cases here:
     1) The merge elem is [Bottom]: this means that the entire type is [nothing]
     2) The merge elem is the only element; since
        `shape(...shape([whatever])) = shape([whatever])`
        we can flatten *)
  let result =
    match (merge_elem, elems) with
    | (Bottom reason, _) ->
      let ty = Typing_make_type.nothing reason in
      Full (ty, false)
    | (Merging (reason, shape_simple), []) ->
      let ty = mk (reason, Tshape (Shape_simple shape_simple)) in
      Full (ty, sd)
    | (Merging (reason, shape_simple), _) ->
      let elem = mk (reason, Tshape (Shape_simple shape_simple)) in
      Partial (elem :: elems, sd)
    | (Empty, []) ->
      (* Everything cancelled to the unit element: the empty closed shape. The
         caller supplies the reason with which to build [shape()]. *)
      Empty_shape sd
    | (Empty, _) -> Partial (elems, sd)
  in
  (env, err_opt, result)
