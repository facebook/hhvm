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

let element_tys elements =
  List.map elements ~f:Typing_shape_normalize.Row.Element.ty

(* The key identifying a spread element. Two occurrences of the same parameter
   are the same key, so it is compared by type rather than by name: [NT<int>] and
   [NT<string>] are different elements. *)
module Splat_elem = struct
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
  (* Project a simple row at a label; an absent label and the [None] unknown
     tail project to [Opt unknown]. *)
  (** Project a normalized splat row at a label under an [Assignment.t] of
      all type parameters contributing to the type. *)
  val proj :
    env ->
    Typing_shape_normalize.Row.t ->
    TShapeField.t option ->
    Assignment.t ->
    env * locl_phase shape_field_type

  (** Type parameters to the right of any required field at [label]: only these
      can contribute to the merged field. *)
  val live_spreads :
    Typing_shape_normalize.Row.t -> TShapeField.t option -> locl_ty list

  (** Set of all known field labels in a normalized row  *)
  val label_set : Typing_shape_normalize.Row.t -> TShapeSet.t

  (** Type parameters and newtypes in spread position in the normalized row. *)
  val spread_elements : Typing_shape_normalize.Row.t -> locl_ty list
end = struct
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
      (assignment : Assignment.t) : env * locl_phase shape_field_type =
    let r = get_reason ty in
    match get_node ty with
    | Tgeneric _
    | Tnewtype _ ->
      (match Splat_elem.Map.find_opt ty assignment with
      | Some fd -> (env, fd)
      | None ->
        (* Unreachable: Every live generic must have a corner value before row
           projection. Missing here indicates a traversal/dependency bug. *)
        (env, { sft_optional = true; sft_ty = Typing_make_type.nothing r }))
    | Tshape (Shape_simple { s_fields; s_unknown_value; _ }) ->
      (env, proj_simple ~s_fields ~s_unknown_value label)
    | _ when Typing_defs.is_nothing ty ->
      (env, { sft_optional = false; sft_ty = Typing_make_type.nothing r })
    | _ -> (env, { sft_optional = true; sft_ty = Typing_make_type.nothing r })

  (* Right-to-left rightmost-wins fold over splat elements, short-circuiting once
     the accumulated field is required. *)
  let rec proj_splat_help
      env rev_left label assignment (fd_right : locl_phase shape_field_type) =
    if not fd_right.sft_optional then
      (env, fd_right)
    else
      match rev_left with
      | [] -> (env, fd_right)
      | elem :: rest ->
        let (env, fd_left) = proj_element env elem label assignment in
        let (env, fd) = Field.merge env ~left:fd_left ~right:fd_right in
        proj_splat_help env rest label assignment fd

  let proj_splat env (ss_elems : locl_ty list) label assignment =
    match List.rev ss_elems with
    | [] ->
      (* An empty splat is the identity, i.e. the empty closed shape. Every label
         projects to absent (Opt nothing) *)
      ( env,
        {
          sft_optional = true;
          sft_ty = Typing_make_type.nothing Typing_reason.none;
        } )
    | rightmost :: rev_left ->
      let (env, fd) = proj_element env rightmost label assignment in
      proj_splat_help env rev_left label assignment fd

  let proj env (row : Typing_shape_normalize.Row.t) label assignment =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () ->
        ( env,
          {
            sft_optional = false;
            sft_ty = Typing_make_type.nothing Typing_reason.none;
          } ))
      ~simple:(fun { s_fields; s_unknown_value; _ } ->
        (env, proj_simple ~s_fields ~s_unknown_value label))
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

(* -- Field bounds of an opaque spread element ------------------------------ *)

module Bounds : sig
  val combined_upper_bound :
    env -> Splat_elem.t -> Typing_reason.t -> env * locl_ty

  val concrete_supertypes : env -> locl_ty -> env * locl_ty list

  val strip_supportdyn : env -> locl_ty -> env * locl_ty

  val shape_types : env -> locl_ty list -> env * locl_phase shape_type list

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
    env -> Splat_elem.t -> Assignment.t -> Typing_reason.t -> env * Upper.t

  module Lower : sig
    type t =
      | Shapes of Typing_shape_normalize.Row.t list
          (** Likewise, but the element is ABOVE every one of them, so combine by
          join. *)
      | Bottom
  end

  val bound_shape_lower :
    env -> Splat_elem.t -> Assignment.t -> Typing_reason.t -> env * Lower.t

  val field_bounds :
    env ->
    Splat_elem.t ->
    TShapeField.t option ->
    Assignment.t ->
    Typing_reason.t ->
    env * locl_phase shape_field_type * locl_phase shape_field_type
end = struct
  let combined_upper_bound env key r =
    match get_node key with
    | Tnewtype (name, targs, _) ->
      Typing_utils.get_newtype_super env (get_reason key) name targs
    | Tgeneric name ->
      let bounds = Typing_env.get_upper_bounds env name in
      if Typing_set.is_empty bounds then
        (env, Typing_make_type.mixed r)
      else
        Typing_intersection.intersect_list env r (Typing_set.elements bounds)
    | _ -> (env, Typing_make_type.mixed r)

  let combined_lower_bound env key r =
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
    | _ -> (env, Typing_make_type.nothing r)

  let concrete_supertypes env ty =
    Typing_utils.get_concrete_supertypes ~abstract_enum:false env ty

  let concrete_subtypes env ty = Typing_utils.get_concrete_subtypes env ty

  let strip_supportdyn env ty =
    let (_supportdyn, env, ty) = Typing_utils.strip_supportdyn env ty in
    (env, ty)

  let shape_types env tys =
    let (env, shapes) =
      List.fold_map tys ~init:env ~f:(fun env ty ->
          let (env, ty) = strip_supportdyn env ty in
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

  (* Reading a row that is already known to be a shape, in each direction's own
     answer type. *)
  let normalized_upper env r shape_ty =
    let (env, _err, row) =
      Typing_shape_normalize.Row.normalize r shape_ty env ~on_error:None
    in
    if Typing_shape_normalize.Row.is_bottom row then
      (env, Upper.Bottom)
    else
      (env, Upper.Shapes [row])

  let normalized_lower env r shape_ty =
    let (env, _err, row) =
      Typing_shape_normalize.Row.normalize r shape_ty env ~on_error:None
    in
    if Typing_shape_normalize.Row.is_bottom row then
      (env, Lower.Bottom)
    else
      (env, Lower.Shapes [row])

  (* Spreading [dynamic] is an open row whose unknown fields are [dynamic]
     ([shape(_ => dynamic)]), matching [Typing_shape_normalize]. *)
  let dynamic_row r =
    Typing_shape_normalize.Row.of_simple
      {
        s_origin = Missing_origin;
        s_unknown_value = Typing_make_type.dynamic r;
        s_fields = TShapeMap.empty;
      }

  let bound_shape_upper env name assignment r =
    let (env, bound_ty) = combined_upper_bound env name r in
    let (env, bound_ty) = Typing_env.expand_type env bound_ty in
    let (env, bound_ty) = strip_supportdyn env bound_ty in
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
      let (env, supers) = concrete_supertypes env bound_ty in
      let (env, shapes) = shape_types env supers in
      (match shapes with
      | [] -> (env, Upper.Unconstrained)
      | _ ->
        let (env, normalized) =
          List.fold_map shapes ~init:env ~f:(fun env shape_ty ->
              match normalized_upper env r shape_ty with
              | (env, Upper.Shapes [row]) -> (env, Some row)
              | (env, _) -> (env, None))
        in
        (match List.filter_opt normalized with
        | [] -> (env, Upper.Bottom)
        | shapes -> (env, Upper.Shapes shapes)))

  let bound_shape_lower env name assignment r =
    let (env, bound_ty) = combined_lower_bound env name r in
    let (env, bound_ty) = Typing_env.expand_type env bound_ty in
    let (env, bound_ty) = strip_supportdyn env bound_ty in
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
      let (env, subs) = concrete_subtypes env bound_ty in
      let (env, shapes) = shape_types env subs in
      (match shapes with
      | [] -> (env, Lower.Bottom)
      | _ ->
        let (env, normalized) =
          List.fold_map shapes ~init:env ~f:(fun env shape_ty ->
              match normalized_lower env r shape_ty with
              | (env, Lower.Shapes [row]) -> (env, Some row)
              | (env, _) -> (env, None))
        in
        (match List.filter_opt normalized with
        | [] -> (env, Lower.Bottom)
        | shapes -> (env, Lower.Shapes shapes)))

  let proj_upper_bound env name label assignment r =
    let (env, view) = bound_shape_upper env name assignment r in
    match view with
    | Upper.Shapes shapes ->
      (match shapes with
      | [] -> (env, { sft_optional = true; sft_ty = Typing_make_type.mixed r })
      | first :: rest ->
        let (env, fd) = Row.proj env first label assignment in
        List.fold rest ~init:(env, fd) ~f:(fun (env, acc) shape_ty ->
            let (env, fd) = Row.proj env shape_ty label assignment in
            Field.meet env ~left:acc ~right:fd))
    | Upper.Bottom ->
      (env, { sft_optional = false; sft_ty = Typing_make_type.nothing r })
    (* A non-shape upper bound constrains nothing: the label may be absent and
       its type may be anything. *)
    | Upper.Unconstrained ->
      (env, { sft_optional = true; sft_ty = Typing_make_type.mixed r })

  let proj_lower_bound env name label assignment r =
    let (env, view) = bound_shape_lower env name assignment r in
    match view with
    | Lower.Shapes shapes ->
      (match shapes with
      | [] ->
        (env, { sft_optional = false; sft_ty = Typing_make_type.nothing r })
      | first :: rest ->
        let (env, fd) = Row.proj env first label assignment in
        List.fold rest ~init:(env, fd) ~f:(fun (env, acc) shape_ty ->
            let (env, fd) = Row.proj env shape_ty label assignment in
            Field.join env ~left:acc ~right:fd))
    | Lower.Bottom ->
      (env, { sft_optional = false; sft_ty = Typing_make_type.nothing r })

  let field_bounds env name label assignment r =
    let (env, lower) = proj_lower_bound env name label assignment r in
    let (env, upper) = proj_upper_bound env name label assignment r in
    (env, lower, upper)
end

(* -- Dependency graph over type parameters --------------------------------- *)

module Dependency_graph = struct
  let type_params_in_upper_bound env name r =
    let (env, bound_ty) = Bounds.combined_upper_bound env name r in
    let (env, bound_ty) = Typing_env.expand_type env bound_ty in
    let (env, bound_ty) = Bounds.strip_supportdyn env bound_ty in
    match get_node bound_ty with
    (* A bound that IS a parameter: the upper view projects [shape(...T)]. *)
    | Tgeneric _ -> [bound_ty]
    | Tnewtype (n, _, _)
      when not (String.equal n Naming_special_names.Classes.cSupportDyn) ->
      [bound_ty]
    | _ ->
      (* Every shape the bound can resolve to, not only the one the view happens
         to pick. An intersection offers several, and a parameter spread into one
         that is not picked would otherwise never be found, leaving it unordered.
         Reporting one that turns out not to be read only adds an ordering
         constraint that was not needed. *)
      let (env, supers) = Bounds.concrete_supertypes env bound_ty in
      let (env, shapes) = Bounds.shape_types env supers in
      ignore env;
      List.concat_map shapes ~f:(fun shape_ty ->
          let (_env, _err, row) =
            Typing_shape_normalize.Row.normalize ~on_error:None r shape_ty env
          in
          Row.spread_elements row)

  let type_params_in_lower_bound env name r =
    let (env, view) =
      Bounds.bound_shape_lower env name Splat_elem.Map.empty r
    in
    ignore env;
    match view with
    | Bounds.Lower.Shapes shapes ->
      List.concat_map shapes ~f:Row.spread_elements
    | Bounds.Lower.Bottom -> []

  let type_params_in_bounds env key r =
    let up = type_params_in_upper_bound env key r in
    let lo = type_params_in_lower_bound env key r in
    Splat_elem.Set.elements
      (Splat_elem.Set.union
         (Splat_elem.Set.of_list up)
         (Splat_elem.Set.of_list lo))

  (* Transitive closure of a set of type parameters *)
  let closure env names r =
    let rec aux worklist acc =
      match worklist with
      | [] -> acc
      | next :: rest when Splat_elem.Set.mem next acc -> aux rest acc
      | next :: rest ->
        let delta = type_params_in_bounds env next r in
        aux (delta @ rest) (Splat_elem.Set.add next acc)
    in
    aux (Splat_elem.Set.elements names) Splat_elem.Set.empty

  (* Topological sort of type parameters. A type parameter that appears in
     another's bound is assigned first; a cycle just skips the offending edge. *)
  let topo env roots r =
    let equal a b = Int.equal (Splat_elem.compare a b) 0 in
    let rec visit key order stack =
      if List.mem order key ~equal || List.mem stack key ~equal then
        order
      else
        let order =
          List.fold_left
            (type_params_in_bounds env key r)
            ~init:order
            ~f:(fun order dep -> visit dep order (key :: stack))
        in
        if List.mem order key ~equal then
          order
        else
          order @ [key]
    in
    Splat_elem.Set.fold (fun key acc -> visit key acc []) roots []
end

module Labels : sig
  val subrow_label_set :
    env ->
    sub:Typing_shape_normalize.Row.t ->
    super:Typing_shape_normalize.Row.t ->
    Typing_reason.t ->
    TShapeSet.t

  (** The full label list a splat subrow must check: [None] (the unknown tail)
      and every label appearing inline and in the bounds of opaque elements. *)
  val subrow_labels :
    env ->
    sub:Typing_shape_normalize.Row.t ->
    super:Typing_shape_normalize.Row.t ->
    Typing_reason.t ->
    TShapeField.t option list
end = struct
  let bound_labels_upper env name r =
    let (env, view) =
      Bounds.bound_shape_upper env name Splat_elem.Map.empty r
    in
    ignore env;
    match view with
    | Bounds.Upper.Shapes shapes ->
      List.fold shapes ~init:TShapeSet.empty ~f:(fun acc shape_ty ->
          TShapeSet.union acc (Row.label_set shape_ty))
    | Bounds.Upper.Bottom
    | Bounds.Upper.Unconstrained ->
      TShapeSet.empty

  let bound_labels_lower env name r =
    let (env, view) =
      Bounds.bound_shape_lower env name Splat_elem.Map.empty r
    in
    ignore env;
    match view with
    | Bounds.Lower.Shapes shapes ->
      List.fold shapes ~init:TShapeSet.empty ~f:(fun acc shape_ty ->
          TShapeSet.union acc (Row.label_set shape_ty))
    | Bounds.Lower.Bottom -> TShapeSet.empty

  let bound_label_set env names r =
    let all = Dependency_graph.closure env (Splat_elem.Set.of_list names) r in
    Splat_elem.Set.fold
      (fun name acc ->
        let up = bound_labels_upper env name r
        and lo = bound_labels_lower env name r in
        TShapeSet.union acc (TShapeSet.union up lo))
      all
      TShapeSet.empty

  let subrow_label_set
      env
      ~(sub : Typing_shape_normalize.Row.t)
      ~(super : Typing_shape_normalize.Row.t)
      r =
    let params = Row.spread_elements sub @ Row.spread_elements super in
    TShapeSet.union
      (TShapeSet.union (Row.label_set sub) (Row.label_set super))
      (bound_label_set env params r)

  let subrow_labels env ~sub ~super r : TShapeField.t option list =
    None
    :: List.map
         (TShapeSet.elements (subrow_label_set env ~sub ~super r))
         ~f:Option.some
end

(* -- Masking --------------------------------------------------------------- *)

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
  let of_splat env ss_elems label key assignment r =
    let rec aux rev_elems acc =
      match rev_elems with
      | [] -> Unknown
      | ty :: rest ->
        (match get_node ty with
        | Tgeneric _
        | Tnewtype _
          when Int.equal (Splat_elem.compare ty key) 0 ->
          acc
        | Tgeneric _
        | Tnewtype _ ->
          let (_env, lower, upper) =
            Bounds.field_bounds env ty label assignment r
          in
          if Field.is_required upper then
            Masked
          else if Field.is_required lower then
            aux rest Unknown
          else
            aux rest acc
        | _ -> aux rest acc)
    in
    aux (List.rev ss_elems) Unmasked

  let of_row env (row : Typing_shape_normalize.Row.t) label key assignment r =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> Unknown)
      ~simple:(fun _ -> Unknown)
      ~elements:(fun elements ->
        of_splat env (element_tys elements) label key assignment r)
end

(* -- Corner search --------------------------------------------------------- *)

(* Enumerate corner assignments in dependency order. Bound analysis is
   recomputed at each use, and every reachable assignment path is traversed. *)
module Corner_search = struct
  let corners_for
      env
      ~depended_on
      ~live_sub
      ~live_super
      ~(sub : Typing_shape_normalize.Row.t)
      ~(super : Typing_shape_normalize.Row.t)
      label
      key
      assignment
      r : env * Field.Corners.t =
    let (env, lower, upper) = Bounds.field_bounds env key label assignment r in
    let is_free = not (Splat_elem.Set.mem key depended_on)
    and in_sub = Splat_elem.Set.mem key live_sub
    and in_super = Splat_elem.Set.mem key live_super in
    if is_free && in_sub && not in_super then
      (env, Field.Corners.Values [upper])
    else if is_free && (not in_sub) && in_super then
      (env, Field.Corners.Values [lower])
    else if is_free && in_sub && in_super then
      let m_sub = Masking.of_row env sub label key assignment r
      and m_super = Masking.of_row env super label key assignment r in
      match (m_sub, m_super) with
      | (Masking.Masked, _) -> (env, Field.Corners.Values [lower])
      | (_, Masking.Masked) -> (env, Field.Corners.Values [upper])
      | (_, Masking.Unmasked) when Field.is_optional lower ->
        (env, Field.Corners.Values [lower])
      | _ -> (env, Field.Corners.of_bounds ~lower ~upper)
    else
      (env, Field.Corners.of_bounds ~lower ~upper)

  let check_subrow_corners
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
    let ty_params_topo = Dependency_graph.topo env all_live r in
    let depended_on =
      List.fold_left
        ty_params_topo
        ~init:Splat_elem.Set.empty
        ~f:(fun acc key ->
          Splat_elem.Set.union
            acc
            (Splat_elem.Set.of_list
               (Dependency_graph.type_params_in_bounds env key r)))
    in
    let rec loop keys assignment env =
      match keys with
      | key :: rest ->
        let (env, corners) =
          corners_for
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
        in
        (match corners with
        | Field.Corners.Inverted -> init env
        | Field.Corners.Values assignments ->
          List.fold_left assignments ~init:(init env) ~f:(fun acc field ->
              conj acc (fun env ->
                  loop rest (Splat_elem.Map.add key field assignment) env)))
      | [] ->
        let (env, sub) = Row.proj env sub label assignment in
        let (env, super) = Row.proj env super label assignment in
        f env ~sub ~super
    in
    loop ty_params_topo Splat_elem.Map.empty env

  let assignments
      env (ty_params_topo : locl_ty list) (label : TShapeField.t option) r :
      env * Assignment.t list =
    let rec aux env keys (assignment : Assignment.t) =
      match keys with
      | [] -> (env, [assignment])
      | key :: rest ->
        let (env, lower, upper) =
          Bounds.field_bounds env key label assignment r
        in
        let corners =
          match Field.Corners.of_bounds ~lower ~upper with
          | Field.Corners.Values corners -> corners
          | Field.Corners.Inverted -> []
        in
        let (env, rev_chunks) =
          List.fold_left corners ~init:(env, []) ~f:(fun (env, chunks) field ->
              let (env, assignments) =
                aux env rest (Splat_elem.Map.add key field assignment)
              in
              (env, assignments :: chunks))
        in
        (env, List.concat (List.rev rev_chunks))
    in
    aux env ty_params_topo Splat_elem.Map.empty
end

module Spread_var = struct
  (* Spread type-variable ids at spread position, in source order *)
  let ids (row : Typing_shape_normalize.Row.t) : Tvid.t list =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> [])
      ~simple:(fun _ -> [])
      ~elements:(fun elements ->
        List.filter_map (element_tys elements) ~f:(fun ty ->
            match get_node ty with
            | Tvar v -> Some v
            | _ -> None))

  (* Split a splat's elements around the first occurrence of spread var [v]
     returning the elements before and after. *)
  let partition (row : Typing_shape_normalize.Row.t) (v : Tvid.t) :
      (locl_ty list * locl_ty list) option =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> None)
      ~simple:(fun _ -> None)
      ~elements:(fun elements ->
        let ss_elems = element_tys elements in
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
  let solve env r (row : Typing_shape_normalize.Row.t) :
      env * Typing_shape_normalize.Row.t =
    Typing_shape_normalize.Row.fold
      row
      ~bottom:(fun () -> (env, row))
      ~simple:(fun _ -> (env, row))
      ~elements:(fun elements ->
        let ss_elems = element_tys elements in
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
        let solved = Shape_splat { ss_elems = List.rev rev } in
        let (env, _err, row) =
          Typing_shape_normalize.Row.normalize ~on_error:None r solved env
        in
        (env, row))
end

(* -- API ------------------------------------------------------------------- *)

(* Resolve a splat to a single simple shape for field reads; takes every live
   type parameter to its upper bound in topological order then read each label off
   the projected row. Reuses the corner machinery ([closure]/[topo]/[field_bounds]/
   [proj]) so mutually-referencing bounds (e.g. [T2 as shape(...T1)]) resolve in
   order. *)
let resolve_for_read env r elems : env * locl_ty =
  (* Project a single row to a resolved simple shape (generics -> upper bound). *)
  let project env row =
    let labels =
      None
      :: List.map
           (TShapeSet.elements
              (Labels.subrow_label_set env ~sub:row ~super:row r))
           ~f:Option.some
    in
    let (env, known, unknown) =
      List.fold_left
        labels
        ~init:(env, TShapeMap.empty, Typing_make_type.nothing r)
        ~f:(fun (env, known, unknown) label ->
          let live = Splat_elem.Set.of_list (Row.live_spreads row label) in
          let ty_params_topo = Dependency_graph.topo env live r in
          (* Assign each type param its upper bound, in topo order so a param's
             bound is projected under the upper corners it depends on. *)
          let (env, assignment) =
            List.fold_left
              ty_params_topo
              ~init:(env, Splat_elem.Map.empty)
              ~f:(fun (env, a) key ->
                let (env, _lower, upper) =
                  Bounds.field_bounds env key label a r
                in
                (env, Splat_elem.Map.add key upper a))
          in
          let (env, fd) = Row.proj env row label assignment in
          match label with
          | Some lbl -> (env, TShapeMap.add lbl fd known, unknown)
          | None -> (env, known, fd.sft_ty))
    in
    ( env,
      mk
        ( r,
          Tshape
            (Shape_simple
               {
                 s_origin = Missing_origin;
                 s_unknown_value = unknown;
                 s_fields = known;
               }) ) )
  in
  let (env, _err, row) =
    let shape_ty = Shape_splat { ss_elems = elems } and on_error = None in
    Typing_shape_normalize.Row.normalize ~on_error r shape_ty env
  in
  if Typing_shape_normalize.Row.is_bottom row then
    (env, Typing_shape_normalize.Row.to_ty ~reason:r row)
  else
    project env row

let proj = Row.proj

let row_live_spread_at = Row.live_spreads

let subrow_label_set = Labels.subrow_label_set

let subrow_labels = Labels.subrow_labels

let topo = Dependency_graph.topo

let check_subrow_corners = Corner_search.check_subrow_corners

let corner_assignments = Corner_search.assignments

let spread_tyvar_ids = Spread_var.ids

let partition_at_tyvar = Spread_var.partition

let solve_spread_vars = Spread_var.solve

module For_test = struct
  type upper_bound_view =
    | Upper_shapes of Typing_shape_normalize.Row.t list
    | Upper_bottom
    | Upper_unconstrained

  type lower_bound_view =
    | Lower_shapes of Typing_shape_normalize.Row.t list
    | Lower_bottom

  module Masking = Masking

  let closure = Dependency_graph.closure

  let bound_shape_upper env ty assignment r =
    let (env, view) = Bounds.bound_shape_upper env ty assignment r in
    match view with
    | Bounds.Upper.Shapes rows -> (env, Upper_shapes rows)
    | Bounds.Upper.Bottom -> (env, Upper_bottom)
    | Bounds.Upper.Unconstrained -> (env, Upper_unconstrained)

  let bound_shape_lower env ty assignment r =
    let (env, view) = Bounds.bound_shape_lower env ty assignment r in
    match view with
    | Bounds.Lower.Shapes rows -> (env, Lower_shapes rows)
    | Bounds.Lower.Bottom -> (env, Lower_bottom)

  let field_bounds = Bounds.field_bounds

  let type_params_in_upper_bound = Dependency_graph.type_params_in_upper_bound

  let type_params_in_lower_bound = Dependency_graph.type_params_in_lower_bound

  let type_params_in_bounds = Dependency_graph.type_params_in_bounds

  let corners_for
      env ~depended_on ~live_sub ~live_super ~sub ~super label key assignment r
      =
    Corner_search.corners_for
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
