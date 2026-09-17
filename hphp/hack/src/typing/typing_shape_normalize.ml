(* (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary. *)

open Typing_defs_core

(** [Typing_shape_splat_key] memoizes every kind of splat operand using its
    structural identity. For [Tnewtype], it omits the stored bound because
    [is_nothing] re-fetches it from the typedef. Comparing that bound directly
    would follow both branches of chains such as [N1 as shape(...N2, ...N2)],
    [N2 as shape(...N3, ...N3)], and so on, taking exponential time. *)
type nothing_cache =
  (Typing_inference_env.t * bool) Typing_shape_splat_key.Map.t ref

(** A path identifies an operand in the original nested splat/distribution tree.

    Union distribution reprocesses the unvisited prefix once per element, so one
    input operand may be visited in several generated branches. Its path is a
    merge-local identity used to reuse one recovery type variable and report one
    error, while keeping distinct equal-looking operands separate. *)
module Elem_path = struct
  module Ord = struct
    type t = int list

    let compare = List.compare Int.compare
  end

  include Ord
  module Map = Stdlib.Map.Make (Ord)
  module Set = Stdlib.Set.Make (Ord)
end

let splat_is_nothing
    (cache : nothing_cache) (env : Typing_env_types.env) (ty : locl_phase ty) :
    bool =
  let key = Typing_shape_splat_key.of_ty ty in
  match Typing_shape_splat_key.Map.find_opt key !cache with
  | Some (inference_env, result) when inference_env == env.inference_env ->
    result
  | _ ->
    let result = Typing_utils.is_nothing env ty in
    cache :=
      Typing_shape_splat_key.Map.add key (env.inference_env, result) !cache;
    result

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
  | Union of locl_phase ty list
      (** A union operand was distributed outward: one fully-finalised type per
          union member (supportdyn already applied per member, so no [bool]). The
          caller unions these with its own reason (mirrors [Empty_shape], where
          the caller supplies the reason). *)
  | Intersection of locl_phase ty list
      (** As [Union], for an intersection operand: a value that is every member
          spreads to a row that is every member's row. *)

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

(** Turn the merge accumulator (once the element list is exhausted) into a
    [merge_result]. Pure; no [env]. *)
let finalize (merge_elem : merge_elem) (elems : locl_phase ty list) (sd : bool)
    : merge_result =
  match (merge_elem, elems) with
  | (Bottom reason, _) ->
    (* The whole type is [nothing]. *)
    Full (Typing_make_type.nothing reason, false)
  | (Merging (reason, shape_simple), []) ->
    Full (mk (reason, Tshape (Shape_simple shape_simple)), sd)
  | (Merging (reason, shape_simple), _) ->
    let elem = mk (reason, Tshape (Shape_simple shape_simple)) in
    Partial (elem :: elems, sd)
  | (Empty, []) ->
    (* Everything cancelled to the unit element: the empty closed shape. The
       caller supplies the reason with which to build [shape()]. *)
    Empty_shape sd
  | (Empty, _) -> Partial (elems, sd)

(** Finalise one distributed branch to a single type, built with the member's
    [reason]. supportdyn is applied per-branch here, so distributed results
    carry no sd flag. *)
let result_to_ty
    ~(reason : Typing_reason.t)
    (env : Typing_env_types.env)
    (res : merge_result) : Typing_env_types.env * locl_phase ty =
  let wrap sd ty =
    if sd then
      Typing_make_type.supportdyn (get_reason ty) ty
    else
      ty
  in
  match res with
  | Full (ty, sd) -> (env, wrap sd ty)
  | Empty_shape sd ->
    (env, wrap sd (Typing_make_type.closed_shape reason TShapeMap.empty))
  (* A splat of one element and nothing else IS that element. *)
  | Partial ([elem], sd) -> (env, wrap sd elem)
  | Partial (ss_elems, sd) ->
    (env, wrap sd (mk (reason, Tshape (Shape_splat { ss_elems }))))
  | Union tys ->
    (* Nested union (a member itself distributed): flatten by unioning. *)
    Typing_union.union_list env reason tys
  | Intersection tys -> Typing_intersection.intersect_list env reason tys

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
  let nothing_cache = ref Typing_shape_splat_key.Map.empty in
  let invalid_tyvars = ref Elem_path.Map.empty in
  let reported_invalids = ref Elem_path.Set.empty in
  let error_tyvar path reason env =
    match Elem_path.Map.find_opt path !invalid_tyvars with
    | Some ty -> (env, ty)
    | None ->
      let (env, ty) =
        Typing_env.fresh_type_error
          env
          (Pos_or_decl.unsafe_to_raw_pos (Typing_reason.to_pos reason))
      in
      invalid_tyvars := Elem_path.Map.add path ty !invalid_tyvars;
      (env, ty)
  in
  let add_splat_not_a_shape_error path reason errs =
    if Elem_path.Set.mem path !reported_invalids then
      errs
    else begin
      reported_invalids := Elem_path.Set.add path !reported_invalids;
      Option.fold on_error ~none:errs ~some:(fun on_error ->
          let err =
            Typing_error.apply_reasons
              ~on_error
              (Typing_error.Secondary.Splat_not_a_shape (Reason.to_pos reason))
          in
          err :: errs)
    end
  in
  let bottom_result ty =
    Full (Typing_make_type.nothing (get_reason ty), false)
  in
  let union_result tys =
    match tys with
    | first :: rest
      when Typing_defs.is_nothing first
           && List.for_all Typing_defs.is_nothing rest ->
      bottom_result first
    | _ -> Union tys
  in
  let intersection_result tys =
    match List.find_opt Typing_defs.is_nothing tys with
    | Some bottom -> bottom_result bottom
    | None -> Intersection tys
  in
  let rec loop rev_elems (merge_elem, elems, errs, sd, env) =
    match rev_elems with
    | [] -> (env, errs, finalize merge_elem elems sd)
    | (path, ty) :: rev_elems ->
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
             && splat_is_nothing nothing_cache env shape_left.s_unknown_value ->
        (* The empty closed shape is unit for merge *)
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Merge with bottom on rhs ---------------------------------------- *)
      | ( (_, (Tshape _ | Tgeneric _ | Tnewtype _ | Tvar _ | Tdynamic _)),
          Bottom _ ) ->
        (* We don't accumulate elements since the resulting type will be
           [nothing] *)
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Merge with bottom on both sides --------------------------------- *)
      | ((_, _), Bottom _) when splat_is_nothing nothing_cache env ty ->
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
      | ((reason, _), (Empty | Merging _))
        when splat_is_nothing nothing_cache env ty ->
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
      (* -- Accumulate opaque elements -------------------------------------- *)
      (* Type variables, type parameters, and newtypes. A newtype is opaque
         outside its defining file and behaves exactly like a rigid parameter;
         inside, localization has already expanded it to its definition. *)
      | ((_, (Tgeneric _ | Tnewtype _ | Tvar _)), Merging (shape_reason, shape))
        ->
        let elems =
          let elem = mk (shape_reason, Tshape (Shape_simple shape)) in
          ty :: elem :: elems
        in
        let merge_elem = Empty in
        let sd = sd || sd_elem in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      | ((_, (Tgeneric _ | Tnewtype _ | Tvar _)), Empty) ->
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
        let rev_elems =
          List.rev (List.mapi (fun index ty -> (index :: path, ty)) ss_elems)
          @ rev_elems
        in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      (* -- Distribute a union operand -------------------------------------- *)
      (* [shape(...A, ...(m1 | ... | mk), ...B)] distributes to
         [shape(...A, ...m1, ...B) | ... | shape(...A, ...mk, ...B)] *)
      | ((_, Tunion members), _) ->
        distribute
          path
          rev_elems
          (merge_elem, elems, errs, sd || sd_elem, env)
          members
          ~make_result:union_result
      (* -- Distribute an intersection operand ----------------------------- *)
      (* [shape(...A, ...(m1 & ... & mk), ...B)] is
         [shape(...A, ...m1, ...B) & ... & shape(...A, ...mk, ...B)] *)
      | ((_, Tintersection members), _) ->
        distribute
          path
          rev_elems
          (merge_elem, elems, errs, sd || sd_elem, env)
          members
          ~make_result:intersection_result
      (* -- Error conditions ------------------------------------------------ *)
      | ((reason, _), Merging (shape_reason, shape)) ->
        let (env, elem_err) = error_tyvar path reason env in
        let elems =
          let elem_shape = mk (shape_reason, Tshape (Shape_simple shape)) in
          elem_err :: elem_shape :: elems
        in
        let errs = add_splat_not_a_shape_error path reason errs in
        let merge_elem = Empty in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      | ((reason, _), Empty) ->
        let (env, elem_err) = error_tyvar path reason env in
        let elems = elem_err :: elems in
        let merge_elem = Empty in
        let errs = add_splat_not_a_shape_error path reason errs in
        loop rev_elems (merge_elem, elems, errs, sd, env)
      | ((reason, _), Bottom _) ->
        (* Still report malformed splats despite being bottom. *)
        let errs = add_splat_not_a_shape_error path reason errs in
        loop rev_elems (merge_elem, elems, errs, sd, env))
  and distribute
      path rev_elems (merge_elem, elems, errs, sd, env) members ~make_result =
    let (env, error_groups, branch_tys) =
      List.fold_left
        (fun (env, error_groups, tys) (path, member) ->
          (* [errs] belongs to the already-processed suffix. Branch-local
             errors start empty; the per-element cache prevents a shared
             prefix from reporting the same malformed operand again. *)
          let (env, branch_errs, res) =
            loop ((path, member) :: rev_elems) (merge_elem, elems, [], sd, env)
          in
          let (env, ty) = result_to_ty ~reason:(get_reason member) env res in
          (env, branch_errs :: error_groups, ty :: tys))
        (env, [errs], [])
        (List.mapi (fun index member -> (index :: path, member)) members)
    in
    (env, List.concat error_groups, make_result (List.rev branch_tys))
  in
  let (env, errs, result) =
    (* Merge simple shape elements from right to left so reverse the list.
       [loop] finalises via [finalize] at its base case (and via [result_to_ty]
       per branch for a distributed union or intersection). *)
    let rev_elems =
      List.rev (List.mapi (fun index ty -> ([index], ty)) elems)
    in
    loop rev_elems (Empty, [], [], false, env)
  in
  (env, Typing_error.multiple_opt errs, result)

type normalize_result =
  | Normalized_shape of locl_phase shape_type
  | Normalized_union of locl_phase ty list
  | Normalized_intersection of locl_phase ty list
  | Normalized_bottom
      (** The merge collapsed to the bottom row [nothing]: the row is
          uninhabited. *)

module Row = struct
  module Opaque = struct
    type t =
      | Type_parameter of locl_phase ty
      | Type_variable of locl_phase ty
      | Newtype of locl_phase ty

    type view =
      | Type_parameter of locl_phase ty
      | Type_variable of locl_phase ty
      | Newtype of locl_phase ty

    let view (opaque : t) : view =
      match opaque with
      | Type_parameter ty -> Type_parameter ty
      | Type_variable ty -> Type_variable ty
      | Newtype ty -> Newtype ty

    let ty (opaque : t) =
      match opaque with
      | Type_parameter ty
      | Type_variable ty
      | Newtype ty ->
        ty

    let of_ty ty : t =
      match get_node ty with
      | Tgeneric _ -> (Type_parameter ty : t)
      | Tvar _ -> (Type_variable ty : t)
      | Tnewtype _ -> (Newtype ty : t)
      | _ -> failwith "non-opaque type in normalized shape splat"
  end

  type simple = Typing_reason.t * locl_phase shape_type_simple

  type after_opaque =
    | End
    | Next_shape of simple * after_shape
    | Next_opaque of Opaque.t * after_opaque

  and after_shape =
    | End_after_shape
    | Opaque_after_shape of Opaque.t * after_opaque

  type after_opaque_nonempty =
    | First_shape of simple * after_shape
    | First_opaque of Opaque.t * after_opaque

  type t =
    | Bottom
    | Simple of locl_phase shape_type_simple
    | Type_parameter of locl_phase ty
    | Type_variable of locl_phase ty
    | Newtype of locl_phase ty
    | Opaque_first of Opaque.t * after_opaque_nonempty
    | Shape_first of simple * Opaque.t * after_opaque

  type normalized =
    | Normalized_row of t
    | Normalized_union of locl_phase ty list
    | Normalized_intersection of locl_phase ty list

  let as_row = function
    | Normalized_row row -> Some row
    | Normalized_union _
    | Normalized_intersection _ ->
      None

  let fold_normalized normalized ~row ~union ~intersection =
    match normalized with
    | Normalized_row normalized -> row normalized
    | Normalized_union tys -> union tys
    | Normalized_intersection tys -> intersection tys

  let of_opaque opaque : t =
    match Opaque.view opaque with
    | Opaque.Type_parameter ty -> Type_parameter ty
    | Opaque.Type_variable ty -> Type_variable ty
    | Opaque.Newtype ty -> Newtype ty

  module Element = struct
    type t =
      | Shape of simple
      | Opaque of Opaque.t

    type view =
      | Shape of simple
      | Opaque of Opaque.t

    let view (element : t) : view =
      match element with
      | Shape shape -> Shape shape
      | Opaque opaque -> Opaque opaque

    let ty (element : t) =
      match element with
      | Shape (reason, shape) -> mk (reason, Tshape (Shape_simple shape))
      | Opaque opaque -> Opaque.ty opaque

    let shape shape : t = Shape shape

    let opaque opaque : t = Opaque opaque
  end

  let is_bottom (row : t) =
    match row with
    | Bottom -> true
    | Simple _
    | Type_parameter _
    | Type_variable _
    | Newtype _
    | Opaque_first _
    | Shape_first _ ->
      false

  let as_simple (row : t) =
    match row with
    | Simple shape -> Some shape
    | Bottom
    | Type_parameter _
    | Type_variable _
    | Newtype _
    | Opaque_first _
    | Shape_first _ ->
      None

  let of_simple shape : t = Simple shape

  let rec after_opaque_elements = function
    | End -> []
    | Next_shape (shape, rest) ->
      Element.shape shape :: after_shape_elements rest
    | Next_opaque (opaque, rest) ->
      Element.opaque opaque :: after_opaque_elements rest

  and after_shape_elements = function
    | End_after_shape -> []
    | Opaque_after_shape (opaque, rest) ->
      Element.opaque opaque :: after_opaque_elements rest

  let elements (row : t) =
    match row with
    | Bottom -> []
    | Simple shape -> [Element.shape (Reason.none, shape)]
    | Type_parameter ty
    | Type_variable ty
    | Newtype ty ->
      [Element.opaque (Opaque.of_ty ty)]
    | Opaque_first (opaque, rest) ->
      Element.opaque opaque
      ::
      (match rest with
      | First_shape (shape, rest) ->
        Element.shape shape :: after_shape_elements rest
      | First_opaque (opaque, rest) ->
        Element.opaque opaque :: after_opaque_elements rest)
    | Shape_first (shape, opaque, rest) ->
      Element.shape shape :: Element.opaque opaque :: after_opaque_elements rest

  let fold (row : t) ~bottom ~simple ~elements:fold_elements =
    match row with
    | Bottom -> bottom ()
    | Simple shape -> simple shape
    | Type_parameter _
    | Type_variable _
    | Newtype _
    | Opaque_first _
    | Shape_first _ ->
      fold_elements (elements row)

  let shape_of_ty ty =
    match deref ty with
    | (reason, Tshape (Shape_simple shape)) -> Some (reason, shape)
    | _ -> None

  let splat_of_elems elems : t =
    let rec after_opaque elems =
      match elems with
      | [] -> End
      | first :: rest ->
        (match shape_of_ty first with
        | Some shape -> Next_shape (shape, after_shape rest)
        | None -> Next_opaque (Opaque.of_ty first, after_opaque rest))
    and after_shape elems =
      match elems with
      | [] -> End_after_shape
      | first :: rest ->
        (match shape_of_ty first with
        | Some _ -> failwith "contiguous shapes in normalized shape splat"
        | None -> Opaque_after_shape (Opaque.of_ty first, after_opaque rest))
    in
    match elems with
    | [] -> failwith "empty normalized shape splat"
    | [only] ->
      (match shape_of_ty only with
      | Some _ -> failwith "singleton shape must normalize to a simple row"
      | None -> of_opaque (Opaque.of_ty only))
    | first :: rest ->
      (match shape_of_ty first with
      | None ->
        (match rest with
        | [] -> assert false
        | second :: rest ->
          (match shape_of_ty second with
          | Some shape ->
            Opaque_first
              (Opaque.of_ty first, First_shape (shape, after_shape rest))
          | None ->
            Opaque_first
              ( Opaque.of_ty first,
                First_opaque (Opaque.of_ty second, after_opaque rest) )))
      | Some shape ->
        (match rest with
        | [] -> failwith "normalized shape splat contains no opaque operand"
        | opaque :: rest ->
          Shape_first (shape, Opaque.of_ty opaque, after_opaque rest)))

  let of_merge_result ~(reason : Typing_reason.t) : merge_result -> t = function
    | Full (ty, _) ->
      (match deref ty with
      | (_, Tshape (Shape_simple shape)) -> Simple shape
      | (_, Tshape (Shape_splat { ss_elems })) -> splat_of_elems ss_elems
      | _ -> Bottom)
    | Empty_shape _ ->
      Simple
        {
          s_origin = Missing_origin;
          s_unknown_value = Typing_make_type.nothing reason;
          s_fields = TShapeMap.empty;
        }
    | Partial ([], _) ->
      Simple
        {
          s_origin = Missing_origin;
          s_unknown_value = Typing_make_type.nothing reason;
          s_fields = TShapeMap.empty;
        }
    | Partial (elems, _) -> splat_of_elems elems
    | Union _
    | Intersection _ ->
      failwith "a distributed result is not a normalized row"

  let to_ty ~(reason : Typing_reason.t) (row : t) =
    match row with
    | Bottom -> Typing_make_type.nothing reason
    | Simple shape -> mk (reason, Tshape (Shape_simple shape))
    | Type_parameter ty
    | Type_variable ty
    | Newtype ty ->
      ty
    | Opaque_first _
    | Shape_first _ ->
      (match List.map Element.ty (elements row) with
      | [elem] -> elem
      | ss_elems -> mk (reason, Tshape (Shape_splat { ss_elems })))

  let normalize
      ~(on_error : Typing_error.Reasons_callback.t option)
      (reason : Typing_reason.t)
      (shape_ty : locl_phase shape_type)
      (env : Typing_env_types.env) :
      Typing_env_types.env * Typing_error.t option * normalized =
    let elems =
      match shape_ty with
      | Shape_simple _ -> [mk (reason, Tshape shape_ty)]
      | Shape_splat { ss_elems } -> ss_elems
    in
    let (env, err_opt, result) = merge ~on_error elems env in
    let normalized =
      match result with
      | Union tys -> Normalized_union tys
      | Intersection tys -> Normalized_intersection tys
      | result -> Normalized_row (of_merge_result ~reason result)
    in
    (env, err_opt, normalized)

  let normalized_to_ty
      (env : Typing_env_types.env)
      ~(reason : Typing_reason.t)
      (normalized : normalized) =
    match normalized with
    | Normalized_row row -> (env, to_ty ~reason row)
    | Normalized_union tys -> Typing_union.union_list env reason tys
    | Normalized_intersection tys ->
      Typing_intersection.intersect_list env reason tys
end

let normalize_shape_type
    ~(on_error : Typing_error.Reasons_callback.t option)
    (reason : Typing_reason.t)
    (shape_ty : locl_phase shape_type)
    (env : Typing_env_types.env) :
    Typing_env_types.env * Typing_error.t option * normalize_result =
  (* [_sd] (whether an element carried [supportdyn]) is unused here: the return
     is a bare [shape_type] which cannot hold a [supportdyn] wrapper, and
     subtyping reasons about dynamic separately. *)
  let (env, err_opt, normalized) =
    Row.normalize ~on_error reason shape_ty env
  in
  let distributed_to_shape env ty =
    match deref ty with
    | (_, Tunion []) -> (env, err_opt, Normalized_bottom)
    | (_, Tunion tys) -> (env, err_opt, Normalized_union tys)
    | (_, Tintersection tys) -> (env, err_opt, Normalized_intersection tys)
    | (_, Tshape shape_ty) -> (env, err_opt, Normalized_shape shape_ty)
    | _ -> (env, err_opt, Normalized_shape (Shape_splat { ss_elems = [ty] }))
  in
  Row.fold_normalized
    normalized
    ~row:(function
      | Row.Simple shape -> (env, err_opt, Normalized_shape (Shape_simple shape))
      | ( Row.Type_parameter _ | Row.Type_variable _ | Row.Newtype _
        | Row.Opaque_first _ | Row.Shape_first _ ) as row ->
        let ty = Row.to_ty ~reason row in
        (match deref ty with
        | (_, Tshape shape_ty) -> (env, err_opt, Normalized_shape shape_ty)
        | _ -> (env, err_opt, Normalized_shape (Shape_splat { ss_elems = [ty] })))
      | Row.Bottom -> (env, err_opt, Normalized_bottom))
    ~union:(fun tys ->
      let (env, ty) = Typing_union.union_list env reason tys in
      distributed_to_shape env ty)
    ~intersection:(fun tys ->
      let (env, ty) = Typing_intersection.intersect_list env reason tys in
      distributed_to_shape env ty)

(** Canonical constructor for a shape splat: normalize [elems] and return the
    result in normal form. Every operation that rewrites a row must build its
    result with this rather than assembling a [Shape_splat] by hand -- rewriting
    can empty an element, leave a lone element, or make two simple shapes
    adjacent, none of which are normal forms. *)
let splat
    ~(on_error : Typing_error.Reasons_callback.t option)
    ~(reason : Typing_reason.t)
    (elems : locl_phase ty list)
    (env : Typing_env_types.env) :
    Typing_env_types.env * Typing_error.t option * locl_phase ty =
  let (env, err_opt, res) = merge ~on_error elems env in
  let (env, ty) = result_to_ty ~reason env res in
  (env, err_opt, ty)

let rec pessimize_existing_fields
    ~(overwritten_field : tshape_field_name)
    ~(reason : Typing_reason.t)
    (env : Typing_env_types.env)
    (elems : locl_phase ty list) : Typing_env_types.env * locl_phase ty list =
  match elems with
  | [] -> (env, [])
  | ty :: rest ->
    let (env, ty) =
      match deref ty with
      | (r, Tshape (Shape_simple s)) ->
        let (env, s_unknown_value) =
          Typing_utils.make_supportdyn reason env s.s_unknown_value
        in
        let (env, s_fields) =
          TShapeMap.map_env
            (fun env name ({ sft_optional; sft_ty } as field_ty) ->
              if TShapeField.equal name overwritten_field then
                (env, field_ty)
              else
                let (env, sft_ty) =
                  Typing_utils.make_supportdyn reason env sft_ty
                in
                (env, { sft_optional; sft_ty }))
            env
            s.s_fields
        in
        ( env,
          mk
            ( r,
              Tshape
                (Shape_simple
                   { s_origin = Missing_origin; s_unknown_value; s_fields }) )
        )
      | _ -> (env, ty)
    in
    let (env, rest) =
      pessimize_existing_fields ~overwritten_field ~reason env rest
    in
    (env, ty :: rest)

(** Set [field] to [field_ty] on a shape splat under rightmost-wins semantics. A
    field write is a runtime dict write, so it may add a new field or overwrite
    an existing one, and the written field must win over every element. It is
    therefore set on the rightmost element: if that element is a concrete simple
    shape the field is added or overwritten there in place; otherwise (a type
    parameter, variable or nested splat) a new closed simple shape carrying just
    the field is appended.

    If [pessimize_existing] is set, the row came from beneath [supportdyn], so
    every existing concrete field, including the unknown-field tail, must retain
    that pessimization after the outer wrapper is removed. The newly written
    field remains precise. *)
let set_rightmost_field
    (elems : locl_phase ty list)
    (field : tshape_field_name)
    (field_ty : locl_phase shape_field_type)
    ~(pessimize_existing : bool)
    ~(reason : Typing_reason.t)
    (env : Typing_env_types.env) :
    Typing_env_types.env * Typing_error.t option * locl_phase ty =
  let (env, elems) =
    if pessimize_existing then
      pessimize_existing_fields ~overwritten_field:field ~reason env elems
    else
      (env, elems)
  in
  let append () =
    let s =
      {
        s_origin = Missing_origin;
        s_unknown_value = Typing_make_type.nothing reason;
        s_fields = TShapeMap.add field field_ty TShapeMap.empty;
      }
    in
    elems @ [mk (reason, Tshape (Shape_simple s))]
  in
  let elems =
    match List.rev elems with
    | last :: rev_rest ->
      (match deref last with
      | (r, Tshape (Shape_simple s)) ->
        let s =
          {
            s with
            s_origin = Missing_origin;
            s_fields = TShapeMap.add field field_ty s.s_fields;
          }
        in
        List.rev (mk (r, Tshape (Shape_simple s)) :: rev_rest)
      | _ -> append ())
    | [] -> append ()
  in
  splat ~on_error:None ~reason elems env
