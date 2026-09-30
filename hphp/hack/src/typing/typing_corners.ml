(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* == Ground corner subrow procedure =========================================
   Type parameters in spread position are not collapsed to a single bound;
   instead each type parameter that is 'live' at a given label (i.e. can
   influence its type) is enumerated over the <=4 extremal 'corners' of its
   field interval [lower, upper] at each label, and the per-field subtyping
   obligation is checked at every corner co-assignment.
   ========================================================================== *)

open Hh_prelude
open Typing_defs
open Typing_env_types

(* The key identifying a spread element. Two occurrences of the same parameter
   are the same key, so it is compared by type rather than by name: [NT<int>] and
   [NT<string>] are different elements. *)
module Splat_elem : sig
  type t = locl_ty

  val compare : t -> t -> int

  module Map : Stdlib.Map.S with type key := t

  module Set : Stdlib.Set.S with type elt := t
end = struct
  module Minimal = struct
    type t = locl_ty

    let compare ty1 ty2 =
      let same_opacity =
        match (get_node ty1, get_node ty2) with
        | (Tnewtype _, Tnewtype _) ->
          Bool.equal
            (Typing_reason.Predicates.is_opaque_type_from_module
               (get_reason ty1))
            (Typing_reason.Predicates.is_opaque_type_from_module
               (get_reason ty2))
        | _ -> true
      in
      if phys_equal (get_node ty1) (get_node ty2) && same_opacity then
        0
      else
        Typing_shape_splat_key.compare
          (Typing_shape_splat_key.of_ty ty1)
          (Typing_shape_splat_key.of_ty ty2)
  end

  include Minimal
  module Map = Stdlib.Map.Make (Minimal)
  module Set = Stdlib.Set.Make (Minimal)
end

(* A single assignment of all type parameters to a ground shape field type *)
module Assignment : sig
  type t = locl_phase shape_field_type Splat_elem.Map.t
end = struct
  (* A per-label assignment of each live spread element to a corner field. *)
  type t = locl_phase shape_field_type Splat_elem.Map.t
end

module Missing_assignment : sig
  type t = {
    element: locl_ty;
    position: Pos_or_decl.t;
  }

  val make : locl_ty -> t

  val element : t -> locl_ty

  val position : t -> Pos_or_decl.t
end = struct
  type t = {
    element: Splat_elem.t;
    position: Pos_or_decl.t;
  }

  let make element = { element; position = get_pos element }

  let element missing = missing.element

  let position missing = missing.position
end

module Env_memo : sig
  (** A cached value tied to the exact environment in which it was computed. *)
  type 'a entry

  (** Memoize an environment-threading computation only when it returns the
      physically identical environment. Reusing a value that changed the
      environment could couple otherwise independent branches. *)
  val memoize :
    'a entry Splat_elem.Map.t ref ->
    env ->
    Splat_elem.t ->
    (unit -> env * 'a) ->
    env * 'a

  (** Memoize an environment-indexed value that does not thread environment
      changes. *)
  val memoize_value :
    'a entry Splat_elem.Map.t ref -> env -> Splat_elem.t -> (unit -> 'a) -> 'a
end = struct
  type 'a entry = {
    input_env: env;
    value: 'a;
  }

  let memoize cache env key f =
    match Splat_elem.Map.find_opt key !cache with
    | Some { input_env; value } when phys_equal input_env env -> (env, value)
    | _ ->
      let (output_env, value) = f () in
      (* Reusing generative localization output could couple otherwise separate
         branches, so raw resolutions are cached only when they do not modify
         the env. *)
      if phys_equal output_env env then
        cache := Splat_elem.Map.add key { input_env = env; value } !cache;
      (output_env, value)

  let memoize_value cache env key f =
    match Splat_elem.Map.find_opt key !cache with
    | Some { input_env; value } when phys_equal input_env env -> value
    | _ ->
      let value = f () in
      cache := Splat_elem.Map.add key { input_env = env; value } !cache;
      value
end

module Field : sig
  (** A required or optional localized shape field. *)
  type t = locl_phase shape_field_type

  (** Whether the field must be present. *)
  val is_required : t -> bool

  (** Whether the field may be absent. *)
  val is_optional : t -> bool

  (** Whether the field is optional with type [nothing], hence always absent. *)
  val is_absent : t -> env -> bool

  (** [sub] is at least as required as [super]. *)
  val requiredness_lte : sub:t -> super:t -> bool

  (** Rightmost-wins merge of two fields. *)
  val merge : left:t -> right:t -> env -> env * t

  (** Greatest lower bound of two fields. *)
  val meet : left:t -> right:t -> env -> env * t

  (** Least upper bound of two fields. *)
  val join : left:t -> right:t -> env -> env * t

  (** Extremal field values induced by lower and upper bounds. *)
  module Corners : sig
    (** The field descriptor enumerated at each corner. *)
    type field = t

    (** The reachable corner fields, or an uninhabited inverted interval. *)
    type t =
      | Values of field list
      | Inverted

    (** Enumerate the distinct corners between [lower] and [upper]. *)
    val of_bounds : lower:field -> upper:field -> t
  end
end = struct
  type t = locl_phase shape_field_type

  let is_required field = not field.sft_optional

  let is_optional field = field.sft_optional

  (* An optional field of [nothing] must be absent. *)
  let is_absent field env =
    field.sft_optional && Typing_utils.is_nothing env field.sft_ty

  (* A subtype field must be at least as required as the supertype field. *)
  let requiredness_lte ~sub ~super =
    (not sub.sft_optional) || super.sft_optional

  (* Rightmost-wins field merge. *)
  let merge ~(left : t) ~(right : t) env =
    Typing_shape_normalize.merge_field_descs ~fd_left:left ~fd_right:right env

  let meet ~(left : t) ~(right : t) env =
    let (env, sft_ty) =
      Typing_intersection.intersect
        env
        ~r:(get_reason left.sft_ty)
        left.sft_ty
        right.sft_ty
    in
    (env, { sft_optional = left.sft_optional && right.sft_optional; sft_ty })

  let join ~(left : t) ~(right : t) env =
    let (env, sft_ty) = Typing_union.union env left.sft_ty right.sft_ty in
    (env, { sft_optional = left.sft_optional || right.sft_optional; sft_ty })

  module Corners = struct
    type field = t

    type nonrec t =
      | Values of field list
      | Inverted

    (* An inverted bound pair is uninhabited and contributes no obligations. *)
    let of_bounds ~(lower : field) ~(upper : field) =
      match (lower.sft_optional, upper.sft_optional) with
      | (false, true) ->
        Values
          [
            lower;
            upper;
            { lower with sft_optional = true };
            { upper with sft_optional = false };
          ]
      | (false, false)
      | (true, true) ->
        Values [lower; upper]
      | (true, false) -> Inverted
  end
end

(* -- Projection ------------------------------------------------------------ *)

module Row : sig
  val element_tys : Typing_shape_normalize.Row.Element.t list -> locl_ty list

  (** Project a normalized splat row at a label under an [Assignment.t] of
      all type parameters contributing to the type. *)
  val proj :
    env ->
    Typing_shape_normalize.Row.t ->
    TShapeField.t option ->
    Assignment.t ->
    env * (locl_phase shape_field_type, Missing_assignment.t) result

  (** Type parameters to the right of any required field at [label]: only these
      can contribute to the merged field. *)
  val live_spreads :
    Typing_shape_normalize.Row.t -> TShapeField.t option -> locl_ty list

  (** Set of all known field labels in a normalized row  *)
  val label_set : Typing_shape_normalize.Row.t -> TShapeSet.t

  (** Type parameters and newtypes in spread position in the normalized row. *)
  val spread_elements : Typing_shape_normalize.Row.t -> locl_ty list
end = struct
  let element_tys elements =
    List.map elements ~f:Typing_shape_normalize.Row.Element.ty

  let proj_simple
      ~(s_fields : locl_phase shape_field_type TShapeMap.t)
      ~(s_unknown_value : locl_ty)
      (label : TShapeField.t option) : locl_phase shape_field_type =
    match Option.bind label ~f:(fun l -> TShapeMap.find_opt l s_fields) with
    | Some fd -> fd
    | None -> { sft_optional = true; sft_ty = s_unknown_value }

  let proj_element
      env
      (ty : locl_ty)
      (label : TShapeField.t option)
      (assignment : Assignment.t) :
      env * (locl_phase shape_field_type, Missing_assignment.t) result =
    let r = get_reason ty in
    match get_node ty with
    | Tgeneric _
    | Tnewtype _ ->
      (match Splat_elem.Map.find_opt ty assignment with
      | Some fd -> (env, Ok fd)
      | None -> (env, Error (Missing_assignment.make ty)))
    | Tshape (Shape_simple { s_fields; s_unknown_value; _ }) ->
      (env, Ok (proj_simple ~s_fields ~s_unknown_value label))
    | _ when Typing_defs.is_nothing ty ->
      (env, Ok { sft_optional = false; sft_ty = Typing_make_type.nothing r })
    | _ -> (env, Ok { sft_optional = true; sft_ty = Typing_make_type.nothing r })

  (* Right-to-left rightmost-wins fold over splat elements, short-circuiting once
     the accumulated field is required. *)
  let rec proj_splat_help
      env rev_left label assignment (fd_right : locl_phase shape_field_type) =
    if not fd_right.sft_optional then
      (env, Ok fd_right)
    else
      match rev_left with
      | [] -> (env, Ok fd_right)
      | elem :: rest ->
        let (env, result) = proj_element env elem label assignment in
        (match result with
        | Error missing -> (env, Error missing)
        | Ok fd_left ->
          let (env, fd) = Field.merge env ~left:fd_left ~right:fd_right in
          proj_splat_help env rest label assignment fd)

  let proj_splat env (ss_elems : locl_ty list) label assignment =
    match List.rev ss_elems with
    | [] ->
      (* An empty splat is the identity, i.e. the empty closed shape. Every label
         projects to absent (Opt nothing) *)
      ( env,
        Ok
          {
            sft_optional = true;
            sft_ty = Typing_make_type.nothing Typing_reason.none;
          } )
    | rightmost :: rev_left ->
      let (env, result) = proj_element env rightmost label assignment in
      (match result with
      | Error missing -> (env, Error missing)
      | Ok fd -> proj_splat_help env rev_left label assignment fd)

  let proj env (row : Typing_shape_normalize.Row.t) label assignment =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () ->
        ( env,
          Ok
            {
              sft_optional = false;
              sft_ty = Typing_make_type.nothing Typing_reason.none;
            } ))
      ~simple:(fun { s_fields; s_unknown_value; _ } ->
        (env, Ok (proj_simple ~s_fields ~s_unknown_value label)))
      ~elements:(fun elements ->
        proj_splat env (element_tys elements) label assignment)

  (* -- Live parameters ----------------------------------------------------- *)

  (* Type parameters to the right of any [Req] field at [label]: only these can
     contribute to the merged field. *)
  let live_spreads_at (ss_elems : locl_ty list) (label : TShapeField.t option) :
      locl_ty list =
    let rec aux rev_elems acc =
      match rev_elems with
      | [] -> acc
      | ty :: left ->
        (match get_node ty with
        | Tgeneric _
        | Tnewtype _ ->
          aux left (ty :: acc)
        | _ when Typing_defs.is_nothing ty ->
          (* bottom row: forces [Req bottom]; everything to its left is masked *)
          acc
        | Tshape (Shape_simple { s_fields; s_unknown_value; _ }) ->
          let fd = proj_simple ~s_fields ~s_unknown_value label in
          if fd.sft_optional then
            aux left acc
          else
            acc
        | _ -> aux left acc)
    in
    aux (List.rev ss_elems) []

  let live_spreads (row : Typing_shape_normalize.Row.t) label =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> [])
      ~simple:(fun _ -> [])
      ~elements:(fun elements -> live_spreads_at (element_tys elements) label)

  (* -- Labels ---------------------------------------------------------------- *)

  (* Inline labels a row contributes, looking through inline shape spreads.
     Opaque splat elements contribute none. *)
  let rec label_set_shape (row : locl_phase shape_type) : TShapeSet.t =
    match row with
    | Shape_simple { s_fields; _ } ->
      TShapeMap.fold
        (fun k _ acc -> TShapeSet.add k acc)
        s_fields
        TShapeSet.empty
    | Shape_splat { ss_elems } ->
      List.fold_left ss_elems ~init:TShapeSet.empty ~f:(fun acc ty ->
          TShapeSet.union acc (element_label_set ty))

  and element_label_set (ty : locl_ty) : TShapeSet.t =
    match get_node ty with
    | Tshape shape_ty -> label_set_shape shape_ty
    | _ -> TShapeSet.empty

  let label_set (row : Typing_shape_normalize.Row.t) : TShapeSet.t =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> TShapeSet.empty)
      ~simple:(fun shape -> label_set_shape (Shape_simple shape))
      ~elements:(fun elements ->
        List.fold_left
          (element_tys elements)
          ~init:TShapeSet.empty
          ~f:(fun labels ty -> TShapeSet.union labels (element_label_set ty)))

  let spread_elements (row : Typing_shape_normalize.Row.t) : locl_ty list =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> [])
      ~simple:(fun _ -> [])
      ~elements:(fun elements ->
        List.filter (element_tys elements) ~f:(fun ty ->
            match get_node ty with
            | Tgeneric _
            | Tnewtype _ ->
              true
            | _ -> false))
end

(* -- Bound lookup ----------------------------------------------------------
   Determine the bounds of an opaque shape splat element under the typing
   environment.
   -------------------------------------------------------------------------- *)
module Bound_lookup : sig
  module Cache : sig
    type t

    val create : unit -> t
  end

  val upper_bounds :
    Cache.t -> env -> Splat_elem.t -> Typing_reason.t -> env * locl_ty list

  val lower_bounds :
    Cache.t -> env -> Splat_elem.t -> Typing_reason.t -> env * locl_ty list

  val combined_upper_bound :
    Cache.t -> env -> Splat_elem.t -> Typing_reason.t -> env * locl_ty

  val combined_lower_bound :
    Cache.t -> env -> Splat_elem.t -> Typing_reason.t -> env * locl_ty

  val concrete_supertypes : Cache.t -> env -> locl_ty -> env * locl_ty list

  val concrete_subtypes : Cache.t -> env -> locl_ty -> env * locl_ty list

  val strip_supportdyn : env -> locl_ty -> env * locl_ty
end = struct
  module Cache = struct
    type t = {
      upper_bounds: locl_ty Env_memo.entry Splat_elem.Map.t ref;
      lower_bounds: locl_ty Env_memo.entry Splat_elem.Map.t ref;
      concrete_supers: locl_ty list Env_memo.entry Splat_elem.Map.t ref;
      concrete_subs: locl_ty list Env_memo.entry Splat_elem.Map.t ref;
    }

    let create () =
      {
        upper_bounds = ref Splat_elem.Map.empty;
        lower_bounds = ref Splat_elem.Map.empty;
        concrete_supers = ref Splat_elem.Map.empty;
        concrete_subs = ref Splat_elem.Map.empty;
      }
  end

  let combined_upper_bound cache env key r =
    Env_memo.memoize cache.Cache.upper_bounds env key (fun () ->
        match get_node key with
        | Tnewtype (name, targs, _) ->
          Typing_utils.get_newtype_super env (get_reason key) name targs
        | Tgeneric name ->
          let bounds = Typing_env.get_upper_bounds env name in
          if Typing_set.is_empty bounds then
            (env, Typing_make_type.mixed r)
          else
            Typing_intersection.intersect_list
              env
              r
              (Typing_set.elements bounds)
        | _ -> (env, Typing_make_type.mixed r))

  let combined_lower_bound cache env key r =
    Env_memo.memoize cache.Cache.lower_bounds env key (fun () ->
        match get_node key with
        | Tnewtype (name, targs, _) ->
          let (env, lower) = Typing_utils.get_newtype_sub_opt env name targs in
          (env, Option.value lower ~default:(Typing_make_type.nothing r))
        | Tgeneric name ->
          let bounds = Typing_env.get_lower_bounds env name in
          if Typing_set.is_empty bounds then
            (env, Typing_make_type.nothing r)
          else
            Typing_union.union_list env r (Typing_set.elements bounds)
        | _ -> (env, Typing_make_type.nothing r))

  let upper_bounds cache env key r =
    match get_node key with
    | Tgeneric name ->
      (env, Typing_set.elements (Typing_env.get_upper_bounds env name))
    | Tnewtype _ ->
      let (env, bound) = combined_upper_bound cache env key r in
      (env, [bound])
    | _ -> (env, [])

  let lower_bounds cache env key r =
    match get_node key with
    | Tgeneric name ->
      (env, Typing_set.elements (Typing_env.get_lower_bounds env name))
    | Tnewtype _ ->
      let (env, bound) = combined_lower_bound cache env key r in
      (env, [bound])
    | _ -> (env, [])

  let concrete_supertypes cache env ty =
    Env_memo.memoize cache.Cache.concrete_supers env ty (fun () ->
        Typing_utils.get_concrete_supertypes ~abstract_enum:false env ty)

  let concrete_subtypes cache env ty =
    Env_memo.memoize cache.Cache.concrete_subs env ty (fun () ->
        Typing_utils.get_concrete_subtypes env ty)

  let strip_supportdyn env ty =
    let (_supportdyn, env, ty) = Typing_utils.strip_supportdyn env ty in
    (env, ty)
end

(* -- Exact field-bound evaluation -------------------------------------------
   Normalize shape bounds and project them under the curren assignment.
   -------------------------------------------------------------------------- *)
module Field_bounds : sig
  module Upper : sig
    type t =
      | Shapes of Typing_shape_normalize.Row.t list
          (** All the rows the bound resolves to. The element is below every one of
          them, so its field is below each of their fields: combine by meet. A
          bound can resolve to several, an intersection being the obvious case,
          and keeping only one silently drops what the others say. *)
      | Bottom
          (** The bottom row: every field present, at the uninhabited type. *)
      | Unconstrained  (** Not a shape, so it rules nothing out. *)
  end

  val bound_shape_upper :
    Bound_lookup.Cache.t ->
    env ->
    Splat_elem.t ->
    Assignment.t ->
    Typing_reason.t ->
    env * Upper.t

  module Lower : sig
    type t =
      | Shapes of Typing_shape_normalize.Row.t list
          (** Likewise, but the element is ABOVE every one of them, so combine by
          join. *)
      | Bottom
  end

  val bound_shape_lower :
    Bound_lookup.Cache.t ->
    env ->
    Splat_elem.t ->
    Assignment.t ->
    Typing_reason.t ->
    env * Lower.t

  val field_bounds :
    Bound_lookup.Cache.t ->
    env ->
    Splat_elem.t ->
    TShapeField.t option ->
    Assignment.t ->
    Typing_reason.t ->
    env
    * ( locl_phase shape_field_type * locl_phase shape_field_type,
        Missing_assignment.t )
      result

  val upper_field_bound :
    Bound_lookup.Cache.t ->
    env ->
    Splat_elem.t ->
    TShapeField.t option ->
    Assignment.t ->
    Typing_reason.t ->
    env * (locl_phase shape_field_type, Missing_assignment.t) result

  val field_bounds_for_equality :
    Bound_lookup.Cache.t ->
    env ->
    preferred_members:Splat_elem.Set.t ->
    Splat_elem.Set.t ->
    TShapeField.t option ->
    Assignment.t ->
    Typing_reason.t ->
    env
    * ( locl_phase shape_field_type * locl_phase shape_field_type,
        Missing_assignment.t )
      result

  val upper_field_bound_for_equality :
    Bound_lookup.Cache.t ->
    env ->
    preferred_members:Splat_elem.Set.t ->
    Splat_elem.Set.t ->
    TShapeField.t option ->
    Assignment.t ->
    Typing_reason.t ->
    env * (locl_phase shape_field_type, Missing_assignment.t) result
end = struct
  let shape_types env tys =
    let (env, shapes) =
      List.fold_map tys ~init:env ~f:(fun env ty ->
          let (env, ty) = Bound_lookup.strip_supportdyn env ty in
          match get_node ty with
          | Tshape shape_ty -> (env, Some shape_ty)
          | _ -> (env, None))
    in
    (env, List.filter_opt shapes)

  module Upper = struct
    type t =
      | Shapes of Typing_shape_normalize.Row.t list
          (** All the rows the bound resolves to. The element is below every one of
          them, so its field is below each of their fields: combine by meet. A
          bound can resolve to several, an intersection being the obvious case,
          and keeping only one silently drops what the others say. *)
      | Bottom
          (** The bottom row: every field present, at the uninhabited type. *)
      | Unconstrained  (** Not a shape, so it rules nothing out. *)
  end

  module Lower = struct
    type t =
      | Shapes of Typing_shape_normalize.Row.t list
          (** Likewise, but the element is ABOVE every one of them, so combine by
          join. *)
      | Bottom
  end

  let intersect_upper_views views =
    if
      List.exists views ~f:(function
          | Upper.Bottom -> true
          | Upper.Shapes _
          | Upper.Unconstrained ->
            false)
    then
      Upper.Bottom
    else
      match
        List.concat_map views ~f:(function
            | Upper.Shapes rows -> rows
            | Upper.Bottom
            | Upper.Unconstrained ->
              [])
      with
      | [] -> Upper.Unconstrained
      | rows -> Upper.Shapes rows

  let union_lower_views views =
    match
      List.concat_map views ~f:(function
          | Lower.Shapes rows -> rows
          | Lower.Bottom -> [])
    with
    | [] -> Lower.Bottom
    | rows -> Lower.Shapes rows

  (* Read a normalized shape in each direction's own answer type. *)
  let rec normalized_upper env r shape_ty =
    let (env, _err, normalized) =
      Typing_shape_normalize.Row.normalize r shape_ty env ~on_error:None
    in
    Typing_shape_normalize.Row.fold_normalized
      normalized
      ~row:(fun row ->
        if Typing_shape_normalize.Row.is_bottom row then
          (env, Upper.Bottom)
        else
          (env, Upper.Shapes [row]))
      ~union:(fun _ -> (env, Upper.Unconstrained))
      ~intersection:(fun tys ->
        let (env, views) =
          List.fold_map tys ~init:env ~f:(fun env ty ->
              normalized_upper
                env
                (get_reason ty)
                (Shape_splat { ss_elems = [ty] }))
        in
        (env, intersect_upper_views views))

  let rec normalized_lower env r shape_ty =
    let (env, _err, normalized) =
      Typing_shape_normalize.Row.normalize r shape_ty env ~on_error:None
    in
    Typing_shape_normalize.Row.fold_normalized
      normalized
      ~row:(fun row ->
        if Typing_shape_normalize.Row.is_bottom row then
          (env, Lower.Bottom)
        else
          (env, Lower.Shapes [row]))
      ~union:(fun tys ->
        let (env, views) =
          List.fold_map tys ~init:env ~f:(fun env ty ->
              normalized_lower
                env
                (get_reason ty)
                (Shape_splat { ss_elems = [ty] }))
        in
        (env, union_lower_views views))
      ~intersection:(fun _ -> (env, Lower.Bottom))

  (* Spreading [dynamic] is an open row whose unknown fields are [dynamic]
     ([shape(_ => dynamic)]), matching [Typing_shape_normalize]. *)
  let dynamic_row r =
    Typing_shape_normalize.Row.of_simple
      {
        s_origin = Missing_origin;
        s_unknown_value = Typing_make_type.dynamic r;
        s_fields = TShapeMap.empty;
      }

  let bound_shape_upper_of_ty cache env bound_ty assignment r =
    let (env, bound_ty) = Typing_env.expand_type env bound_ty in
    let (env, bound_ty) = Bound_lookup.strip_supportdyn env bound_ty in
    let is_assigned_param () =
      Splat_elem.Map.mem bound_ty assignment
      &&
      match get_node bound_ty with
      | Tnewtype (n, _, _) ->
        not (String.equal n Naming_special_names.Classes.cSupportDyn)
      | _ -> true
    in
    match get_node bound_ty with
    | Tshape shape_ty -> normalized_upper env r shape_ty
    | Tdynamic _ -> (env, Upper.Shapes [dynamic_row r])
    | _ when Typing_defs.is_nothing bound_ty -> (env, Upper.Bottom)
    | Tgeneric _
    | Tnewtype _
      when is_assigned_param () ->
      normalized_upper env r (Shape_splat { ss_elems = [bound_ty] })
    | _ ->
      let (env, supers) = Bound_lookup.concrete_supertypes cache env bound_ty in
      let (env, shapes) = shape_types env supers in
      (match shapes with
      | [] -> (env, Upper.Unconstrained)
      | _ ->
        let (env, row_groups) =
          List.fold_map shapes ~init:env ~f:(fun env shape_ty ->
              match normalized_upper env r shape_ty with
              | (env, Upper.Shapes rows) -> (env, Some rows)
              | (env, _) -> (env, None))
        in
        (match List.concat (List.filter_opt row_groups) with
        | [] -> (env, Upper.Bottom)
        | shapes -> (env, Upper.Shapes shapes)))

  let bound_shape_upper cache env name assignment r =
    let (env, bound_ty) = Bound_lookup.combined_upper_bound cache env name r in
    bound_shape_upper_of_ty cache env bound_ty assignment r

  let bound_shape_lower_of_ty cache env bound_ty assignment r =
    let (env, bound_ty) = Typing_env.expand_type env bound_ty in
    let (env, bound_ty) = Bound_lookup.strip_supportdyn env bound_ty in
    match get_node bound_ty with
    | Tshape shape_ty -> normalized_lower env r shape_ty
    | Tdynamic _ -> (env, Lower.Shapes [dynamic_row r])
    | _ when Typing_defs.is_nothing bound_ty -> (env, Lower.Bottom)
    (* A lower bound that is a parameter, where that parameter ALREADY has a
       value. Then it is not an approximation at all: this element is at least
       whatever that one turned out to be, so use it.

       Unlike the upper case this creates no ordering requirement, and
       [type_params_in_lower_bound] deliberately reports no dependency for it. An
       edge here would make the relation symmetric and turn a single constraint
       into a cycle. Taking the value only when it happens to be there keeps the
       coupling without the edge, which is what [where T1 = T2] needs: the two
       parameters constrain each other in both directions, and with only the upper
       half enforced one could be given a value below the other. *)
    | Tgeneric _
    | Tnewtype _
      when Splat_elem.Map.mem bound_ty assignment
           &&
           match get_node bound_ty with
           | Tnewtype (n, _, _) ->
             not (String.equal n Naming_special_names.Classes.cSupportDyn)
           | _ -> true ->
      normalized_lower env r (Shape_splat { ss_elems = [bound_ty] })
    | _ ->
      let (env, subs) = Bound_lookup.concrete_subtypes cache env bound_ty in
      let (env, shapes) = shape_types env subs in
      (match shapes with
      | [] -> (env, Lower.Bottom)
      | _ ->
        let (env, row_groups) =
          List.fold_map shapes ~init:env ~f:(fun env shape_ty ->
              match normalized_lower env r shape_ty with
              | (env, Lower.Shapes rows) -> (env, Some rows)
              | (env, _) -> (env, None))
        in
        (match List.concat (List.filter_opt row_groups) with
        | [] -> (env, Lower.Bottom)
        | shapes -> (env, Lower.Shapes shapes)))

  let bound_shape_lower cache env name assignment r =
    let (env, bound_ty) = Bound_lookup.combined_lower_bound cache env name r in
    bound_shape_lower_of_ty cache env bound_ty assignment r

  let project_shapes env shapes label assignment ~empty ~combine =
    let rec project_rest env acc = function
      | [] -> (env, Ok acc)
      | shape :: rest ->
        let (env, result) = Row.proj env shape label assignment in
        (match result with
        | Error missing -> (env, Error missing)
        | Ok field ->
          let (env, acc) = combine ~left:acc ~right:field env in
          project_rest env acc rest)
    in
    match shapes with
    | [] -> (env, Ok empty)
    | first :: rest ->
      let (env, result) = Row.proj env first label assignment in
      (match result with
      | Error missing -> (env, Error missing)
      | Ok field -> project_rest env field rest)

  let proj_upper_view env view label assignment r =
    match view with
    | Upper.Shapes shapes ->
      project_shapes
        env
        shapes
        label
        assignment
        ~empty:{ sft_optional = true; sft_ty = Typing_make_type.mixed r }
        ~combine:Field.meet
    | Upper.Bottom ->
      (env, Ok { sft_optional = false; sft_ty = Typing_make_type.nothing r })
    (* A non-shape upper bound constrains nothing: the label may be absent and
       its type may be anything. *)
    | Upper.Unconstrained ->
      (env, Ok { sft_optional = true; sft_ty = Typing_make_type.mixed r })

  let proj_lower_view env view label assignment r =
    match view with
    | Lower.Shapes shapes ->
      project_shapes
        env
        shapes
        label
        assignment
        ~empty:{ sft_optional = false; sft_ty = Typing_make_type.nothing r }
        ~combine:Field.join
    | Lower.Bottom ->
      (env, Ok { sft_optional = false; sft_ty = Typing_make_type.nothing r })

  let proj_upper_bound cache env name label assignment r =
    let (env, view) = bound_shape_upper cache env name assignment r in
    proj_upper_view env view label assignment r

  let upper_field_bound = proj_upper_bound

  let proj_lower_bound cache env name label assignment r =
    let (env, view) = bound_shape_lower cache env name assignment r in
    proj_lower_view env view label assignment r

  let field_bounds cache env name label assignment r =
    let (env, lower) = proj_lower_bound cache env name label assignment r in
    match lower with
    | Error missing -> (env, Error missing)
    | Ok lower ->
      let (env, upper) = proj_upper_bound cache env name label assignment r in
      (match upper with
      | Error missing -> (env, Error missing)
      | Ok upper -> (env, Ok (lower, upper)))

  let fold_equality_bounds
      cache
      env
      ~preferred_members
      members
      label
      assignment
      r
      ~get_bounds
      ~project
      ~combine
      ~(init : Field.t) =
    let is_direct_member env bound =
      let (env, bound) = Typing_env.expand_type env bound in
      let is_member =
        match get_node bound with
        | Tgeneric _ -> Splat_elem.Set.mem bound members
        | _ -> false
      in
      (env, bound, is_member)
    in
    let rec fold_one_member env acc = function
      | [] -> (env, Ok acc)
      | member :: rest ->
        let (env, bounds) = get_bounds cache env member r in
        let rec fold_one_bound env acc = function
          | [] -> fold_one_member env acc rest
          | bound :: bounds ->
            let (env, bound, is_member) = is_direct_member env bound in
            if is_member then
              fold_one_bound env acc bounds
            else
              let (env, result) = project env bound label assignment r in
              (match result with
              | Error missing -> (env, Error missing)
              | Ok field ->
                let (env, acc) = combine ~left:acc ~right:field env in
                fold_one_bound env acc bounds)
        in
        fold_one_bound env acc bounds
    in
    let preferred = Splat_elem.Set.inter members preferred_members in
    let remaining = Splat_elem.Set.diff members preferred in
    fold_one_member
      env
      init
      (Splat_elem.Set.elements preferred @ Splat_elem.Set.elements remaining)

  let lower_field_bound_for_equality
      cache env ~preferred_members members label assignment r =
    let project env bound label assignment r =
      let (env, view) = bound_shape_lower_of_ty cache env bound assignment r in
      proj_lower_view env view label assignment r
    in
    fold_equality_bounds
      cache
      env
      ~preferred_members
      members
      label
      assignment
      r
      ~get_bounds:Bound_lookup.lower_bounds
      ~project
      ~combine:Field.join
      ~init:{ sft_optional = false; sft_ty = Typing_make_type.nothing r }

  let upper_field_bound_for_equality
      cache env ~preferred_members members label assignment r =
    let project env bound label assignment r =
      let (env, view) = bound_shape_upper_of_ty cache env bound assignment r in
      proj_upper_view env view label assignment r
    in
    fold_equality_bounds
      cache
      env
      ~preferred_members
      members
      label
      assignment
      r
      ~get_bounds:Bound_lookup.upper_bounds
      ~project
      ~combine:Field.meet
      ~init:{ sft_optional = true; sft_ty = Typing_make_type.mixed r }

  let field_bounds_for_equality
      cache env ~preferred_members members label assignment r =
    let (env, lower) =
      lower_field_bound_for_equality
        cache
        env
        ~preferred_members
        members
        label
        assignment
        r
    in
    match lower with
    | Error missing -> (env, Error missing)
    | Ok lower ->
      let (env, upper) =
        upper_field_bound_for_equality
          cache
          env
          ~preferred_members
          members
          label
          assignment
          r
      in
      (match upper with
      | Error missing -> (env, Error missing)
      | Ok upper -> (env, Ok (lower, upper)))
end

(* -- Dependency analysis --------------------------------------------------- *)
module Analysis : sig
  module Dependency : sig
    (** How evaluating a bound reaches another spread element. *)
    type kind =
      | Direct_upper
      | Indirect_upper
      | Nested_upper
      | Nested_lower

    type t

    val source : t -> Splat_elem.t

    val target : t -> Splat_elem.t

    val kind : t -> kind

    val position : t -> Pos_or_decl.t
  end

  module Cycle_info : sig
    type t

    val members : t -> Splat_elem.Set.t

    val dependencies : t -> Dependency.t list
  end

  module Component : sig
    (** One strongly connected component. Only direct reciprocal type-parameter
           constraints prove equality; every other cyclic component is unsupported. *)
    type t =
      | Acyclic of Splat_elem.t
      | Proven_equal of Cycle_info.t
      | Unsupported_cycle of Cycle_info.t

    val members : t -> Splat_elem.Set.t
  end

  module Cache : sig
    type t = {
      dependencies: Dependency.t list Env_memo.entry Splat_elem.Map.t ref;
    }

    val create : unit -> t
  end

  module Labels : sig
    val subrow_label_set :
      Bound_lookup.Cache.t ->
      Cache.t ->
      env ->
      sub:Typing_shape_normalize.Row.t ->
      super:Typing_shape_normalize.Row.t ->
      Reason.t ->
      TShapeSet.t

    val subrow_labels :
      Bound_lookup.Cache.t ->
      Cache.t ->
      env ->
      sub:Typing_shape_normalize.Row.t ->
      super:Typing_shape_normalize.Row.t ->
      Reason.t ->
      tshape_field_name option list
  end

  type t = {
    components: Component.t list;
    dependencies: Dependency.t list;
    node_visits: int;
    edge_visits: int;
  }

  val type_params_in_bounds :
    Bound_lookup.Cache.t ->
    Cache.t ->
    env ->
    locl_ty ->
    Reason.t ->
    locl_ty list

  val type_params_in_upper_bound :
    Bound_lookup.Cache.t ->
    Cache.t ->
    env ->
    locl_ty ->
    Reason.t ->
    locl_ty list

  val type_params_in_lower_bound :
    Bound_lookup.Cache.t ->
    Cache.t ->
    env ->
    locl_ty ->
    Reason.t ->
    locl_ty list

  val components : t -> Component.t list

  val dependencies : t -> Dependency.t list

  val closure :
    Bound_lookup.Cache.t ->
    Cache.t ->
    env ->
    Splat_elem.Set.t ->
    Reason.t ->
    Splat_elem.Set.t

  val analyze :
    Bound_lookup.Cache.t -> Cache.t -> env -> Splat_elem.Set.t -> Reason.t -> t

  (* -- For test only -- *)
  val node_visits : t -> int

  val edge_visits : t -> int
end = struct
  module Dependency = struct
    type kind =
      | Direct_upper
      | Indirect_upper
      | Nested_upper
      | Nested_lower

    type t = {
      source: Splat_elem.t;
      target: Splat_elem.t;
      kind: kind;
      position: Pos_or_decl.t;
    }

    let make ~source ~target ~kind =
      { source; target; kind; position = get_pos target }

    let source dependency = dependency.source

    let target dependency = dependency.target

    let kind dependency = dependency.kind

    let position dependency = dependency.position
  end

  module Cycle_info = struct
    type t = {
      members: Splat_elem.Set.t;
      dependencies: Dependency.t list;
    }

    let make ~members ~dependencies = { members; dependencies }

    let members info = info.members

    let dependencies info = info.dependencies
  end

  module Component = struct
    type t =
      | Acyclic of Splat_elem.t
      | Proven_equal of Cycle_info.t
      | Unsupported_cycle of Cycle_info.t

    let members = function
      | Acyclic member -> Splat_elem.Set.singleton member
      | Proven_equal info
      | Unsupported_cycle info ->
        Cycle_info.members info
  end

  module Cache = struct
    type t = {
      dependencies: Dependency.t list Env_memo.entry Splat_elem.Map.t ref;
    }

    let create () = { dependencies = ref Splat_elem.Map.empty }
  end

  type t = {
    components: Component.t list;
    dependencies: Dependency.t list;
    node_visits: int;
    edge_visits: int;
  }

  let components analysis = analysis.components

  let dependencies analysis = analysis.dependencies

  let node_visits analysis = analysis.node_visits

  let edge_visits analysis = analysis.edge_visits

  let classified_upper_dependencies bounds_cache env source r =
    let rec indirect_dependencies env ty =
      let (env, ty) = Bound_lookup.strip_supportdyn env ty in
      match get_node ty with
      | Tgeneric _ -> (env, [(ty, Dependency.Indirect_upper)])
      | Tnewtype (name, _, _)
        when not (String.equal name Naming_special_names.Classes.cSupportDyn) ->
        (env, [(ty, Dependency.Indirect_upper)])
      | Tshape shape_ty ->
        let (env, _err, normalized) =
          Typing_shape_normalize.Row.normalize ~on_error:None r shape_ty env
        in
        Typing_shape_normalize.Row.fold_normalized
          normalized
          ~row:(fun row ->
            ( env,
              List.map (Row.spread_elements row) ~f:(fun target ->
                  (target, Dependency.Nested_upper)) ))
          ~union:(indirect_dependencies_of_tys env)
          ~intersection:(indirect_dependencies_of_tys env)
      | Tunion tys
      | Tintersection tys ->
        indirect_dependencies_of_tys env tys
      | _ -> (env, [])
    and indirect_dependencies_of_tys env tys =
      let (env, dependencies) =
        List.fold_map tys ~init:env ~f:indirect_dependencies
      in
      (env, List.concat dependencies)
    in
    let dependencies_of_bound env bound =
      let (env, bound) = Typing_env.expand_type env bound in
      match get_node bound with
      | Tgeneric _ -> (env, [(bound, Dependency.Direct_upper)])
      | Tnewtype (name, _, _)
        when not (String.equal name Naming_special_names.Classes.cSupportDyn) ->
        (env, [(bound, Dependency.Direct_upper)])
      | _ ->
        let (env, bound) = Bound_lookup.strip_supportdyn env bound in
        let (env, supers) =
          Bound_lookup.concrete_supertypes bounds_cache env bound
        in
        indirect_dependencies_of_tys env supers
    in
    let (env, bounds) = Bound_lookup.upper_bounds bounds_cache env source r in
    let (_env, dependencies) =
      List.fold_map bounds ~init:env ~f:dependencies_of_bound
    in
    List.concat dependencies

  let classified_lower_dependencies bounds_cache env source r =
    let (env, view) =
      Field_bounds.bound_shape_lower
        bounds_cache
        env
        source
        Splat_elem.Map.empty
        r
    in
    ignore env;
    match view with
    | Field_bounds.Lower.Shapes shapes ->
      List.concat_map shapes ~f:Row.spread_elements
      |> List.map ~f:(fun target -> (target, Dependency.Nested_lower))
    | Field_bounds.Lower.Bottom -> []

  (* This classified edge list is the sole dependency representation. Closure,
     compatibility queries, SCC analysis, and evaluation order all derive from
     it, so they cannot silently disagree about a reachable element. *)
  let dependencies_from bounds_cache cache env source r =
    Env_memo.memoize_value cache.Cache.dependencies env source (fun () ->
        let upper = classified_upper_dependencies bounds_cache env source r in
        let lower = classified_lower_dependencies bounds_cache env source r in
        List.map (upper @ lower) ~f:(fun (target, kind) ->
            Dependency.make ~source ~target ~kind))

  let targets dependencies ~f =
    List.fold_left dependencies ~init:Splat_elem.Set.empty ~f:(fun acc edge ->
        if f (Dependency.kind edge) then
          Splat_elem.Set.add (Dependency.target edge) acc
        else
          acc)
    |> Splat_elem.Set.elements

  let type_params_in_upper_bound bounds_cache cache env source r =
    targets (dependencies_from bounds_cache cache env source r) ~f:(function
        | Dependency.Direct_upper
        | Dependency.Indirect_upper
        | Dependency.Nested_upper ->
          true
        | Dependency.Nested_lower -> false)

  let type_params_in_lower_bound bounds_cache cache env source r =
    targets (dependencies_from bounds_cache cache env source r) ~f:(function
        | Dependency.Nested_lower -> true
        | Dependency.Direct_upper
        | Dependency.Indirect_upper
        | Dependency.Nested_upper ->
          false)

  let type_params_in_bounds bounds_cache cache env source r =
    targets (dependencies_from bounds_cache cache env source r) ~f:(fun _ ->
        true)

  let closure bounds_cache cache env names r =
    let rec aux worklist acc =
      match worklist with
      | [] -> acc
      | next :: rest when Splat_elem.Set.mem next acc -> aux rest acc
      | next :: rest ->
        let delta = type_params_in_bounds bounds_cache cache env next r in
        aux (delta @ rest) (Splat_elem.Set.add next acc)
    in
    aux (Splat_elem.Set.elements names) Splat_elem.Set.empty

  let analyze bounds_cache analysis_cache env roots r =
    let next_index = ref 0 in
    let indices = ref Splat_elem.Map.empty in
    let lowlinks = ref Splat_elem.Map.empty in
    let stack = ref [] in
    let on_stack = ref Splat_elem.Set.empty in
    let components = ref [] in
    let dependencies = ref [] in
    let node_visits = ref 0 in
    let edge_visits = ref 0 in
    let find map key = Option.value_exn (Splat_elem.Map.find_opt key !map) in
    let rec visit source =
      let index = !next_index in
      Int.incr next_index;
      Int.incr node_visits;
      indices := Splat_elem.Map.add source index !indices;
      lowlinks := Splat_elem.Map.add source index !lowlinks;
      stack := source :: !stack;
      on_stack := Splat_elem.Set.add source !on_stack;
      let outgoing =
        dependencies_from bounds_cache analysis_cache env source r
      in
      dependencies := List.rev_append outgoing !dependencies;
      List.iter outgoing ~f:(fun dependency ->
          Int.incr edge_visits;
          let target = Dependency.target dependency in
          if not (Splat_elem.Map.mem target !indices) then begin
            visit target;
            lowlinks :=
              Splat_elem.Map.add
                source
                (Int.min (find lowlinks source) (find lowlinks target))
                !lowlinks
          end else if Splat_elem.Set.mem target !on_stack then
            lowlinks :=
              Splat_elem.Map.add
                source
                (Int.min (find lowlinks source) (find indices target))
                !lowlinks);
      if Int.equal (find lowlinks source) (find indices source) then begin
        let rec pop members =
          match !stack with
          | [] -> failwith "empty stack while completing an SCC"
          | member :: rest ->
            stack := rest;
            on_stack := Splat_elem.Set.remove member !on_stack;
            let members = Splat_elem.Set.add member members in
            if Int.equal (Splat_elem.compare member source) 0 then
              members
            else
              pop members
        in
        components := pop Splat_elem.Set.empty :: !components
      end
    in
    Splat_elem.Set.iter
      (fun root -> if not (Splat_elem.Map.mem root !indices) then visit root)
      roots;
    (* Tarjan pops dependencies before their dependents; the accumulator
       reverses that order. *)
    let components = List.rev !components in
    let dependencies = List.rev !dependencies in
    let component_by_member =
      List.foldi
        components
        ~init:Splat_elem.Map.empty
        ~f:(fun index acc members ->
          Splat_elem.Set.fold
            (fun member acc -> Splat_elem.Map.add member index acc)
            members
            acc)
    in
    let internal_dependencies = Stdlib.Array.make (List.length components) [] in
    List.iter dependencies ~f:(fun dependency ->
        let source_component =
          Splat_elem.Map.find (Dependency.source dependency) component_by_member
        in
        let target_component =
          Splat_elem.Map.find (Dependency.target dependency) component_by_member
        in
        if Int.equal source_component target_component then
          internal_dependencies.(source_component) <-
            dependency :: internal_dependencies.(source_component));
    let components =
      List.mapi components ~f:(fun index members ->
          let dependencies = List.rev internal_dependencies.(index) in
          let is_self_dependency dependency =
            Int.equal
              (Splat_elem.compare
                 (Dependency.source dependency)
                 (Dependency.target dependency))
              0
          in
          if
            Int.equal (Splat_elem.Set.cardinal members) 1
            && not (List.exists dependencies ~f:is_self_dependency)
          then
            Component.Acyclic (Splat_elem.Set.choose members)
          else
            let info = Cycle_info.make ~members ~dependencies in
            let is_ty_param ty =
              match get_node ty with
              | Tgeneric _ -> true
              | _ -> false
            in
            (* Strong connectivity through direct upper constraints proves
               mutual subtyping. Other paths merely prove dependency. *)
            if
              Splat_elem.Set.for_all is_ty_param members
              && List.for_all dependencies ~f:(fun dependency ->
                     match Dependency.kind dependency with
                     | Dependency.Direct_upper -> true
                     | Dependency.Indirect_upper
                     | Dependency.Nested_upper
                     | Dependency.Nested_lower ->
                       false)
            then
              Component.Proven_equal info
            else
              Component.Unsupported_cycle info)
    in
    {
      components;
      dependencies;
      node_visits = !node_visits;
      edge_visits = !edge_visits;
    }

  module Labels = struct
    let bound_labels_upper bounds_cache env name r =
      let (env, view) =
        Field_bounds.bound_shape_upper
          bounds_cache
          env
          name
          Splat_elem.Map.empty
          r
      in
      ignore env;
      match view with
      | Field_bounds.Upper.Shapes shapes ->
        List.fold shapes ~init:TShapeSet.empty ~f:(fun acc shape_ty ->
            TShapeSet.union acc (Row.label_set shape_ty))
      | Field_bounds.Upper.Bottom
      | Field_bounds.Upper.Unconstrained ->
        TShapeSet.empty

    let bound_labels_lower bounds_cache env name r =
      let (env, view) =
        Field_bounds.bound_shape_lower
          bounds_cache
          env
          name
          Splat_elem.Map.empty
          r
      in
      ignore env;
      match view with
      | Field_bounds.Lower.Shapes shapes ->
        List.fold shapes ~init:TShapeSet.empty ~f:(fun acc shape_ty ->
            TShapeSet.union acc (Row.label_set shape_ty))
      | Field_bounds.Lower.Bottom -> TShapeSet.empty

    let bound_label_set bounds_cache analysis_cache env names r =
      let all =
        closure bounds_cache analysis_cache env (Splat_elem.Set.of_list names) r
      in
      Splat_elem.Set.fold
        (fun name acc ->
          let up = bound_labels_upper bounds_cache env name r
          and lo = bound_labels_lower bounds_cache env name r in
          TShapeSet.union acc (TShapeSet.union up lo))
        all
        TShapeSet.empty

    let subrow_label_set
        bounds_cache
        analysis_cache
        env
        ~(sub : Typing_shape_normalize.Row.t)
        ~(super : Typing_shape_normalize.Row.t)
        r =
      let params = Row.spread_elements sub @ Row.spread_elements super in
      TShapeSet.union
        (TShapeSet.union (Row.label_set sub) (Row.label_set super))
        (bound_label_set bounds_cache analysis_cache env params r)

    let subrow_labels bounds_cache analysis_cache env ~sub ~super r :
        TShapeField.t option list =
      None
      :: List.map
           (TShapeSet.elements
              (subrow_label_set bounds_cache analysis_cache env ~sub ~super r))
           ~f:Option.some
  end
end

(* Public compatibility aliases. The representations themselves are owned by
   dependency analysis. *)
module Dependency = Analysis.Dependency
module Cycle_info = Analysis.Cycle_info
module Component = Analysis.Component

type 'a computation =
  | Computed of 'a
  | Empty
  | Unsupported_cycle of Cycle_info.t

module Cache : sig
  type t = {
    bounds: Bound_lookup.Cache.t;
    analysis: Analysis.Cache.t;
  }

  val create : unit -> t
end = struct
  type t = {
    bounds: Bound_lookup.Cache.t;
    analysis: Analysis.Cache.t;
  }

  let create () =
    {
      bounds = Bound_lookup.Cache.create ();
      analysis = Analysis.Cache.create ();
    }
end

(* -- Planning -------------------------------------------------------------- *)
module Plan : sig
  type group =
    | One of locl_ty
    | Equal of Cycle_info.t

  type t

  val groups : t -> group list

  val depended_on : t -> Splat_elem.Set.t

  val order : Cache.t -> env -> Splat_elem.Set.t -> Reason.t -> locl_ty list

  val of_analysis : Analysis.t -> (t, Cycle_info.t) result
end = struct
  type group =
    | One of Splat_elem.t
    | Equal of Analysis.Cycle_info.t

  type t = {
    groups: group list;
    depended_on: Splat_elem.Set.t;
  }

  let groups plan = plan.groups

  let depended_on plan = plan.depended_on

  (* Compatibility view for tests and callers which still request a flat
     topological order. Search consumes the component plan below. *)
  let order cache env roots r =
    Analysis.analyze cache.Cache.bounds cache.Cache.analysis env roots r
    |> Analysis.components
    |> List.concat_map ~f:(fun component ->
           Analysis.Component.members component |> Splat_elem.Set.elements)

  (* Compile dependency analysis into the only representation consumed by
     search: dependency-ordered singleton or proven-equality groups. *)
  let of_analysis analysis =
    let depended_on =
      List.fold_left
        (Analysis.dependencies analysis)
        ~init:Splat_elem.Set.empty
        ~f:(fun acc dependency ->
          Splat_elem.Set.add (Analysis.Dependency.target dependency) acc)
    in
    List.fold_result
      (Analysis.components analysis)
      ~init:[]
      ~f:(fun groups component ->
        match component with
        | Analysis.Component.Acyclic element -> Ok (One element :: groups)
        | Analysis.Component.Proven_equal info -> Ok (Equal info :: groups)
        | Analysis.Component.Unsupported_cycle info -> Error info)
    |> Result.map ~f:(fun groups -> { groups = List.rev groups; depended_on })
end

(* -- Corner search --------------------------------------------------------- *)
module Search : sig
  val corners_for :
    Cache.t ->
    env ->
    depended_on:Splat_elem.Set.t ->
    live_sub:Splat_elem.Set.t ->
    live_super:Splat_elem.Set.t ->
    sub:Typing_shape_normalize.Row.t ->
    super:Typing_shape_normalize.Row.t ->
    tshape_field_name option ->
    locl_ty ->
    Assignment.t ->
    Reason.t ->
    env * (Field.Corners.t, Missing_assignment.t) result

  val check_subrow_corners :
    Cache.t ->
    env ->
    sub:Typing_shape_normalize.Row.t ->
    super:Typing_shape_normalize.Row.t ->
    tshape_field_name option ->
    Reason.t ->
    init:(env -> env * 'a) ->
    conj:(env * 'a -> ('b -> env * 'a) -> env * 'a) ->
    f:
      (env ->
      sub:locl_phase shape_field_type ->
      super:locl_phase shape_field_type ->
      env * 'a) ->
    env * ('a computation, Missing_assignment.t) result

  val assignments :
    Cache.t ->
    env ->
    Splat_elem.Set.t ->
    tshape_field_name option ->
    Reason.t ->
    env * (Assignment.t list computation, Missing_assignment.t) result

  (* -- For test only -- *)

  (** Whether rightward spreads prevent an element from affecting a label. *)
  module Masking : sig
    (** Definite masking, definite visibility, or a bounds-dependent result. *)
    type t =
      | Masked
      | Unmasked
      | Unknown

    (** Determine how [key] is masked in a row under the current assignment. *)
    val of_row :
      Cache.t ->
      env ->
      Typing_shape_normalize.Row.t ->
      TShapeField.t option ->
      locl_ty ->
      Assignment.t ->
      Typing_reason.t ->
      (t, Missing_assignment.t) result
  end
end = struct
  module Masking = struct
    (* Describes how a type parameter influences leftward labels when projecting
       at that label under rightmost-wins semantics. *)
    type t =
      | Masked
      | Unmasked
      | Unknown

    (* Whether a type parameter to the right of [key] in [row] masks it at
       [label]: a rightward generic masks iff its own upper bound is [Req] there;
       [Req] lower but [Opt] upper is [Unknown] (pessimistic). *)
    let of_splat cache env ss_elems label key assignment r =
      let rec aux rev_elems acc =
        match rev_elems with
        | [] -> Ok Unknown
        | ty :: rest ->
          (match get_node ty with
          | Tgeneric _
          | Tnewtype _
            when Int.equal (Splat_elem.compare ty key) 0 ->
            Ok acc
          | Tgeneric _
          | Tnewtype _ ->
            let (_env, bounds) =
              Field_bounds.field_bounds
                cache.Cache.bounds
                env
                ty
                label
                assignment
                r
            in
            (match bounds with
            | Error missing -> Error missing
            | Ok (lower, upper) ->
              if Field.is_required upper then
                Ok Masked
              else if Field.is_required lower then
                aux rest Unknown
              else
                aux rest acc)
          | _ -> aux rest acc)
      in
      aux (List.rev ss_elems) Unmasked

    let of_row
        cache env (row : Typing_shape_normalize.Row.t) label key assignment r =
      Typing_shape_normalize.Row.fold
        row
        ~bottom:(fun () -> Ok Unknown)
        ~simple:(fun _ -> Ok Unknown)
        ~elements:(fun elements ->
          of_splat cache env (Row.element_tys elements) label key assignment r)

    (* let of_row env row label key assignment r =
       of_row_cached (Cache.create ()) env row label key assignment r *)
  end

  (* Enumerate corner assignments in dependency order. Bound resolution is cached
     within one shape-splat operation, but every assignment path is traversed. *)
  let corners_for
      cache
      env
      ~depended_on
      ~live_sub
      ~live_super
      ~(sub : Typing_shape_normalize.Row.t)
      ~(super : Typing_shape_normalize.Row.t)
      label
      key
      assignment
      r : env * (Field.Corners.t, Missing_assignment.t) result =
    let (env, bounds) =
      Field_bounds.field_bounds cache.Cache.bounds env key label assignment r
    in
    match bounds with
    | Error missing -> (env, Error missing)
    | Ok (lower, upper) ->
      let is_free = not (Splat_elem.Set.mem key depended_on)
      and in_sub = Splat_elem.Set.mem key live_sub
      and in_super = Splat_elem.Set.mem key live_super in
      if is_free && in_sub && not in_super then
        (env, Ok (Field.Corners.Values [upper]))
      else if is_free && (not in_sub) && in_super then
        (env, Ok (Field.Corners.Values [lower]))
      else if is_free && in_sub && in_super then
        let m_sub = Masking.of_row cache env sub label key assignment r
        and m_super = Masking.of_row cache env super label key assignment r in
        match (m_sub, m_super) with
        | (Error missing, _)
        | (_, Error missing) ->
          (env, Error missing)
        | (Ok Masking.Masked, _) -> (env, Ok (Field.Corners.Values [lower]))
        | (_, Ok Masking.Masked) -> (env, Ok (Field.Corners.Values [upper]))
        | (_, Ok Masking.Unmasked) when Field.is_optional lower ->
          (env, Ok (Field.Corners.Values [lower]))
        | (Ok _, Ok _) -> (env, Ok (Field.Corners.of_bounds ~lower ~upper))
      else
        (env, Ok (Field.Corners.of_bounds ~lower ~upper))

  let bounds_for_group cache env ~preferred_members label assignment r =
    function
    | Plan.One key ->
      Field_bounds.field_bounds cache.Cache.bounds env key label assignment r
    | Plan.Equal info ->
      Field_bounds.field_bounds_for_equality
        cache.Cache.bounds
        env
        ~preferred_members
        (Cycle_info.members info)
        label
        assignment
        r

  let corners_for_group
      cache
      env
      ~depended_on
      ~live_sub
      ~live_super
      ~sub
      ~super
      ~preferred_members
      label
      group
      assignment
      r =
    match group with
    | Plan.One key ->
      corners_for
        cache
        env
        ~depended_on
        ~live_sub
        ~live_super
        ~sub
        ~super
        label
        key
        assignment
        r
    | Plan.Equal _ as group ->
      let (env, bounds) =
        bounds_for_group cache env ~preferred_members label assignment r group
      in
      (match bounds with
      | Error missing -> (env, Error missing)
      | Ok (lower, upper) -> (env, Ok (Field.Corners.of_bounds ~lower ~upper)))

  let assign_group group field assignment =
    match group with
    | Plan.One member -> Splat_elem.Map.add member field assignment
    | Plan.Equal info ->
      Splat_elem.Set.fold
        (fun member assignment -> Splat_elem.Map.add member field assignment)
        (Cycle_info.members info)
        assignment

  (* Traverse one dependency-ordered plan. [corners] chooses the candidates for
     a group; [leaf], [empty], and [combine] interpret the same traversal for
     ground checking or for collecting inference assignments. *)
  let rec fold_plan groups assignment env ~corners ~leaf ~empty ~combine =
    match groups with
    | [] -> leaf assignment env
    | group :: rest ->
      let (env, result) = corners env group assignment in
      (match result with
      | Error missing -> (env, Error missing)
      | Ok Field.Corners.Inverted -> empty env
      | Ok (Field.Corners.Values []) ->
        failwith "Field.Corners.Values must be nonempty"
      | Ok (Field.Corners.Values (first :: fields)) ->
        let branch field env =
          fold_plan
            rest
            (assign_group group field assignment)
            env
            ~corners
            ~leaf
            ~empty
            ~combine
        in
        List.fold_left fields ~init:(branch first env) ~f:(fun acc field ->
            combine acc (branch field)))

  let check_subrow_corners
      cache
      env
      ~(sub : Typing_shape_normalize.Row.t)
      ~(super : Typing_shape_normalize.Row.t)
      label
      r
      ~init
      ~conj
      ~f =
    let live_sub = Splat_elem.Set.of_list (Row.live_spreads sub label)
    and live_super = Splat_elem.Set.of_list (Row.live_spreads super label) in
    let all_live = Splat_elem.Set.union live_sub live_super in
    let analysis =
      Analysis.analyze cache.Cache.bounds cache.Cache.analysis env all_live r
    in
    match Plan.of_analysis analysis with
    | Error info -> (env, Ok (Unsupported_cycle info))
    | Ok plan ->
      let preferred_members = Splat_elem.Set.union live_sub live_super in
      let empty env =
        let (env, value) = init env in
        (env, Ok (value, false))
      in
      let combine (env, result) next =
        match result with
        | Error missing -> (env, Error missing)
        | Ok (left, left_completed) ->
          let env_before = env in
          let (env_after, right) = next env_before in
          (match right with
          | Error missing -> (env_after, Error missing)
          | Ok (right, right_completed) ->
            let (env, value) =
              conj (env_before, left) (fun _ -> (env_after, right))
            in
            (env, Ok (value, left_completed || right_completed)))
      in
      let finish assignment env =
        let (env, sub_result) = Row.proj env sub label assignment in
        match sub_result with
        | Error missing -> (env, Error missing)
        | Ok sub ->
          let (env, super_result) = Row.proj env super label assignment in
          (match super_result with
          | Error missing -> (env, Error missing)
          | Ok super ->
            let (env, value) = f env ~sub ~super in
            (env, Ok (value, true)))
      in
      let corners env group assignment =
        corners_for_group
          cache
          env
          ~depended_on:(Plan.depended_on plan)
          ~live_sub
          ~live_super
          ~sub
          ~super
          ~preferred_members
          label
          group
          assignment
          r
      in
      let (env, result) =
        fold_plan
          (Plan.groups plan)
          Splat_elem.Map.empty
          env
          ~corners
          ~leaf:finish
          ~empty
          ~combine
      in
      (match result with
      | Error missing -> (env, Error missing)
      | Ok (value, true) -> (env, Ok (Computed value))
      | Ok (_value, false) -> (env, Ok Empty))

  let assignments cache env roots (label : TShapeField.t option) r =
    let analysis =
      Analysis.analyze cache.Cache.bounds cache.Cache.analysis env roots r
    in
    match Plan.of_analysis analysis with
    | Error info -> (env, Ok (Unsupported_cycle info))
    | Ok plan ->
      let corners env group assignment =
        let (env, bounds) =
          bounds_for_group
            cache
            env
            ~preferred_members:roots
            label
            assignment
            r
            group
        in
        match bounds with
        | Error missing -> (env, Error missing)
        | Ok (lower, upper) -> (env, Ok (Field.Corners.of_bounds ~lower ~upper))
      in
      (* Difference lists make collection linear in the number of generated
         assignments; only the Cartesian corner search remains exponential. *)
      let empty env = (env, Ok Fn.id) in
      let combine (env, result) next =
        match result with
        | Error missing -> (env, Error missing)
        | Ok left ->
          let (env, result) = next env in
          (match result with
          | Error missing -> (env, Error missing)
          | Ok right -> (env, Ok (fun tail -> left (right tail))))
      in
      let leaf assignment env = (env, Ok (fun tail -> assignment :: tail)) in
      let (env, result) =
        fold_plan
          (Plan.groups plan)
          Splat_elem.Map.empty
          env
          ~corners
          ~leaf
          ~empty
          ~combine
      in
      (match result with
      | Error missing -> (env, Error missing)
      | Ok build_assignments ->
        (match build_assignments [] with
        | [] -> (env, Ok Empty)
        | assignments -> (env, Ok (Computed assignments))))
end

(* -- Inference helpers ----------------------------------------------------- *)
module Inference : sig
  val spread_tyvar_ids : Typing_shape_normalize.Row.t -> Tvid.t list

  val partition_at_tyvar :
    Typing_shape_normalize.Row.t ->
    Tvid.t ->
    (locl_ty list * locl_ty list) option

  val solve_spread_vars :
    env ->
    Reason.t ->
    Typing_shape_normalize.Row.t ->
    env * Typing_shape_normalize.Row.normalized
end = struct
  (* Spread type-variable ids at spread position, in source order *)
  let spread_tyvar_ids (row : Typing_shape_normalize.Row.t) : Tvid.t list =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> [])
      ~simple:(fun _ -> [])
      ~elements:(fun elements ->
        List.filter_map (Row.element_tys elements) ~f:(fun ty ->
            match get_node ty with
            | Tvar v -> Some v
            | _ -> None))

  (* Split a splat's elements around the first occurrence of spread var [v]
     returning the elements before and after. *)
  let partition_at_tyvar (row : Typing_shape_normalize.Row.t) (v : Tvid.t) :
      (locl_ty list * locl_ty list) option =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> None)
      ~simple:(fun _ -> None)
      ~elements:(fun elements ->
        let ss_elems = Row.element_tys elements in
        let rec loop elems left =
          match elems with
          | [] -> None
          | ty :: rest ->
            (match get_node ty with
            | Tvar v' when Tvid.equal v v' -> Some (List.rev left, rest)
            | _ -> loop rest (ty :: left))
        in
        loop ss_elems [])

  (* Resolve a row's spread type variables to their current solutions, for the
     decoupled both-sides fallback: expand each spread var; keep it if it
     resolved to a non-var, drop it (the empty-row contribution) if still
     unsolved. Sound (the dropped var contributes nothing).

     Substituting breaks normal form three ways: dropping a var can leave a
     lone element, a solution can itself be a splat, and a solution can land
     next to another simple shape, so the rewritten row is re-normalized
     before it goes back to the corner. *)
  let solve_spread_vars env r (row : Typing_shape_normalize.Row.t) :
      env * Typing_shape_normalize.Row.normalized =
    let normalize env ss_elems =
      let (env, _err, normalized) =
        Typing_shape_normalize.Row.normalize
          ~on_error:None
          r
          (Shape_splat { ss_elems })
          env
      in
      (env, normalized)
    in
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () ->
        normalize env [Typing_shape_normalize.Row.to_ty ~reason:r row])
      ~simple:(fun _ ->
        normalize env [Typing_shape_normalize.Row.to_ty ~reason:r row])
      ~elements:(fun elements ->
        let ss_elems = Row.element_tys elements in
        let (env, rev) =
          List.fold_left ss_elems ~init:(env, []) ~f:(fun (env, acc) ty ->
              match get_node ty with
              | Tvar _ ->
                let (env, ty') = Typing_env.expand_type env ty in
                (match get_node ty' with
                | Tvar _ -> (env, acc)
                | _ -> (env, ty' :: acc))
              | _ -> (env, ty :: acc))
        in
        normalize env (List.rev rev))
end

(* -- Resolve a splat to a simple shape for reads ---------------------------
   Unsupported cyclic components are assigned [Opt mixed]; projecting the
   original row afterward preserves precision from concrete fields that mask them
   on the right.
   -------------------------------------------------------------------------- *)
module Read_resolution : sig
  val resolve :
    env ->
    Reason.t ->
    locl_phase ty list ->
    env * (locl_phase ty, Missing_assignment.t) result
end = struct
  let resolve env r elems =
    let cache = Cache.create () in
    let assign_field assignment members field =
      Splat_elem.Set.fold
        (fun member assignment -> Splat_elem.Map.add member field assignment)
        members
        assignment
    in
    let upper_for_component env ~preferred_members assignment label = function
      | Component.Acyclic key ->
        let (env, upper) =
          Field_bounds.upper_field_bound
            cache.Cache.bounds
            env
            key
            label
            assignment
            r
        in
        (match upper with
        | Error missing -> (env, Error missing)
        | Ok upper -> (env, Ok (Splat_elem.Map.add key upper assignment)))
      | Component.Proven_equal info ->
        let members = Cycle_info.members info in
        let (env, upper) =
          Field_bounds.upper_field_bound_for_equality
            cache.Cache.bounds
            env
            ~preferred_members
            members
            label
            assignment
            r
        in
        (match upper with
        | Error missing -> (env, Error missing)
        | Ok upper -> (env, Ok (assign_field assignment members upper)))
      | Component.Unsupported_cycle _ -> (env, Ok assignment)
    in
    let assignment_for_read env row label =
      let live = Splat_elem.Set.of_list (Row.live_spreads row label) in
      let analysis =
        Analysis.analyze cache.Cache.bounds cache.Cache.analysis env live r
      in
      let top = { sft_optional = true; sft_ty = Typing_make_type.mixed r } in
      let rec assign env assignment = function
        | [] -> (env, Ok assignment)
        | Component.Unsupported_cycle info :: rest ->
          let assignment =
            assign_field assignment (Cycle_info.members info) top
          in
          assign env assignment rest
        | ((Component.Acyclic _ | Component.Proven_equal _) as component)
          :: rest ->
          let (env, result) =
            upper_for_component
              env
              ~preferred_members:live
              assignment
              label
              component
          in
          (match result with
          | Error missing -> (env, Error missing)
          | Ok assignment -> assign env assignment rest)
      in
      assign env Splat_elem.Map.empty (Analysis.components analysis)
    in
    (* Project a single row to a resolved simple shape (generics -> upper bound). *)
    let project env row =
      let labels =
        None
        :: List.map
             (TShapeSet.elements
                (Analysis.Labels.subrow_label_set
                   cache.Cache.bounds
                   cache.Cache.analysis
                   env
                   ~sub:row
                   ~super:row
                   r))
             ~f:Option.some
      in
      let rec project_labels env known unknown = function
        | [] ->
          ( env,
            Ok
              (mk
                 ( r,
                   Tshape
                     (Shape_simple
                        {
                          s_origin = Missing_origin;
                          s_unknown_value = unknown;
                          s_fields = known;
                        }) )) )
        | label :: rest ->
          let (env, assignment) = assignment_for_read env row label in
          (match assignment with
          | Error missing -> (env, Error missing)
          | Ok assignment ->
            let (env, field) = Row.proj env row label assignment in
            (match field with
            | Error missing -> (env, Error missing)
            | Ok field ->
              (match label with
              | Some name ->
                project_labels env (TShapeMap.add name field known) unknown rest
              | None -> project_labels env known field.sft_ty rest)))
      in
      project_labels env TShapeMap.empty (Typing_make_type.nothing r) labels
    in
    let (env, _err, normalized) =
      let shape_ty = Shape_splat { ss_elems = elems } and on_error = None in
      Typing_shape_normalize.Row.normalize ~on_error r shape_ty env
    in
    Typing_shape_normalize.Row.fold_normalized
      normalized
      ~row:(fun row ->
        if Typing_shape_normalize.Row.is_bottom row then
          (env, Ok (Typing_shape_normalize.Row.to_ty ~reason:r row))
        else
          project env row)
      ~union:(fun _ ->
        let (env, ty) =
          Typing_shape_normalize.Row.normalized_to_ty env ~reason:r normalized
        in
        (env, Ok ty))
      ~intersection:(fun _ ->
        let (env, ty) =
          Typing_shape_normalize.Row.normalized_to_ty env ~reason:r normalized
        in
        (env, Ok ty))
end

(* -- API ------------------------------------------------------------------- *)

let proj = Row.proj

let row_live_spread_at = Row.live_spreads

let resolve_for_read = Read_resolution.resolve

let subrow_label_set cache env ~sub ~super r =
  Analysis.Labels.subrow_label_set
    cache.Cache.bounds
    cache.Cache.analysis
    env
    ~sub
    ~super
    r

let subrow_labels cache env ~sub ~super r =
  Analysis.Labels.subrow_labels
    cache.Cache.bounds
    cache.Cache.analysis
    env
    ~sub
    ~super
    r

let topo = Plan.order

let check_subrow_corners = Search.check_subrow_corners

let corner_assignments = Search.assignments

let spread_tyvar_ids = Inference.spread_tyvar_ids

let partition_at_tyvar = Inference.partition_at_tyvar

let solve_spread_vars = Inference.solve_spread_vars

module For_test = struct
  type upper_bound_view =
    | Upper_shapes of Typing_shape_normalize.Row.t list
    | Upper_bottom
    | Upper_unconstrained

  type lower_bound_view =
    | Lower_shapes of Typing_shape_normalize.Row.t list
    | Lower_bottom

  module Masking = Search.Masking
  module Dependency = Analysis.Dependency
  module Cycle_info = Analysis.Cycle_info
  module Component = Analysis.Component
  module Analysis = Analysis

  let analyze_dependencies env roots r =
    let cache = Cache.create () in
    Analysis.analyze cache.Cache.bounds cache.Cache.analysis env roots r

  let closure env names r =
    let cache = Cache.create () in
    Analysis.closure cache.Cache.bounds cache.Cache.analysis env names r

  let bound_shape_upper env ty assignment r =
    let cache = Cache.create () in
    let (env, view) =
      Field_bounds.bound_shape_upper cache.Cache.bounds env ty assignment r
    in
    match view with
    | Field_bounds.Upper.Shapes rows -> (env, Upper_shapes rows)
    | Field_bounds.Upper.Bottom -> (env, Upper_bottom)
    | Field_bounds.Upper.Unconstrained -> (env, Upper_unconstrained)

  let bound_shape_lower env ty assignment r =
    let cache = Cache.create () in
    let (env, view) =
      Field_bounds.bound_shape_lower cache.Cache.bounds env ty assignment r
    in
    match view with
    | Field_bounds.Lower.Shapes rows -> (env, Lower_shapes rows)
    | Field_bounds.Lower.Bottom -> (env, Lower_bottom)

  let field_bounds env name label assignment r =
    let cache = Cache.create () in
    Field_bounds.field_bounds cache.Cache.bounds env name label assignment r

  let type_params_in_upper_bound env splat_elem r =
    let cache = Cache.create () in
    Analysis.type_params_in_upper_bound
      cache.Cache.bounds
      cache.Cache.analysis
      env
      splat_elem
      r

  let type_params_in_lower_bound env splat_elem r =
    let cache = Cache.create () in
    Analysis.type_params_in_lower_bound
      cache.Cache.bounds
      cache.Cache.analysis
      env
      splat_elem
      r

  let type_params_in_bounds env splat_elem r =
    let cache = Cache.create () in
    Analysis.type_params_in_bounds
      cache.Cache.bounds
      cache.Cache.analysis
      env
      splat_elem
      r

  let corners_for
      env ~depended_on ~live_sub ~live_super ~sub ~super label key assignment r
      =
    let cache = Cache.create () in
    Search.corners_for
      cache
      env
      ~depended_on
      ~live_sub
      ~live_super
      ~sub
      ~super
      label
      key
      assignment
      r
end
