(* (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary. *)

open Hh_prelude
open Typing_defs

module Label_bound = struct
  (** The upper bound on the shape field labels a shape _may_ include.
    [Bottom_plus] describes labels coming from closed upper bounds whilst
    [Top_minus] describes a set including all labels except those declared
    as [absent] (i.e. open upper bounds) *)
  type t =
    | Bottom_plus of TShapeSet.t
    | Top_minus of TShapeSet.t

  (** The [top] label bound has all fields possible present, `Top - {} = Top` *)
  let top = Top_minus TShapeSet.empty

  (** The [bottom] label bound has no fields present, `Bot + {} = Bot` *)
  let bottom = Bottom_plus TShapeSet.empty

  let equal t1 t2 =
    match (t1, t2) with
    | (Bottom_plus labels1, Bottom_plus labels2)
    | (Top_minus labels1, Top_minus labels2) ->
      TShapeSet.equal labels1 labels2
    | (Bottom_plus _, Top_minus _)
    | (Top_minus _, Bottom_plus _) ->
      false

  let is_top = function
    | Top_minus absent -> TShapeSet.is_empty absent
    | Bottom_plus _ -> false

  let is_empty = function
    | Bottom_plus present -> TShapeSet.is_empty present
    | Top_minus _ -> false

  let disjoint t1 t2 =
    match (t1, t2) with
    | (Bottom_plus present1, Bottom_plus present2) ->
      (* Concrete case; if the intersection of the known labels is empty then
         t1 and t2 must be disjoint *)
      let present_both = TShapeSet.inter present1 present2 in
      TShapeSet.is_empty present_both
    | (Bottom_plus present, Top_minus absent)
    | (Top_minus absent, Bottom_plus present) ->
      (* We have the labels known to be present in on label bound and those
         known to be absent in the other. If the set of labels known to be
         present in the first is contained by the set of labels known to be
         absent in the second then t1 and t2 must be disjoint *)
      TShapeSet.subset present absent
    | (Top_minus _, Top_minus _) ->
      (* Always false - a finite set of absent labels in both t1 and t2 doesn't
         prove there are no other labels which may not be disjoint *)
      false

  let join t1 t2 =
    match (t1, t2) with
    | (Bottom_plus present1, Bottom_plus present2) ->
      let maybe_present = TShapeSet.union present1 present2 in
      Bottom_plus maybe_present
    | (Top_minus absent1, Top_minus absent2) ->
      let definitely_absent = TShapeSet.inter absent1 absent2 in
      Top_minus definitely_absent
    | (Bottom_plus present, Top_minus absent)
    | (Top_minus absent, Bottom_plus present) ->
      Top_minus (TShapeSet.diff absent present)

  let meet t1 t2 =
    match (t1, t2) with
    | (Bottom_plus present1, Bottom_plus present2) ->
      let definitely_present = TShapeSet.inter present1 present2 in
      Bottom_plus definitely_present
    | (Top_minus absent1, Top_minus absent2) ->
      let maybe_absent = TShapeSet.union absent1 absent2 in
      Top_minus maybe_absent
    | (Bottom_plus present, Top_minus absent)
    | (Top_minus absent, Bottom_plus present) ->
      Bottom_plus (TShapeSet.diff present absent)

  (** Equivalent to [meet b1 (Bottom_plus labels)] then extracting the labels
      but avoids being partial *)
  let restrict t labels =
    match t with
    | Bottom_plus s -> TShapeSet.inter labels s
    | Top_minus s -> TShapeSet.diff labels s
end

module Element = struct
  (** Describes the labels which are and may be present in a given splat element
      [definitely_present] corresponds which are known to exist whilst
      [possibly_present] uses a [Label_bound] representation to encode those
      that may exist *)
  type t = {
    definitely_present: TShapeSet.t;
    possibly_present: Label_bound.t;
  }

  (** A closed splat element has all undeclared labels at [nothing]. *)
  let closed_elem definitely_present possibly_present =
    { definitely_present; possibly_present = Bottom_plus possibly_present }

  let empty =
    {
      definitely_present = TShapeSet.empty;
      possibly_present = Bottom_plus TShapeSet.empty;
    }

  (** An open splat element may have any label present *)
  let open_elem =
    {
      definitely_present = TShapeSet.empty;
      possibly_present = Top_minus TShapeSet.empty;
    }

  let ty_param_source name = Printf.sprintf "type parameter `%s`" name

  let newtype_source name = Printf.sprintf "newtype `%s`" (Utils.strip_ns name)

  let demote_except retained { definitely_present; possibly_present } =
    let demoted = TShapeSet.diff definitely_present retained in
    let possibly_present =
      match possibly_present with
      | Label_bound.Bottom_plus s ->
        Label_bound.Bottom_plus (TShapeSet.union s demoted)
      | Label_bound.Top_minus s ->
        Label_bound.Top_minus (TShapeSet.diff s demoted)
    in
    { definitely_present = retained; possibly_present }

  (** Make every known-present label merely possible. *)
  let possible = demote_except TShapeSet.empty

  (** Merge two elements that both contribute: the union of what each certainly
    carries, and the join of what each may carry. *)
  let join e1 e2 =
    let possibly_present =
      Label_bound.join e1.possibly_present e2.possibly_present
    in
    let definitely_present =
      TShapeSet.union e1.definitely_present e2.definitely_present
    in
    { definitely_present; possibly_present }
end

module Positions = struct
  type t = {
    first: Pos_or_decl.t option;
    all: Pos_or_decl.Set.t;
  }

  let empty = { first = None; all = Pos_or_decl.Set.empty }

  let singleton pos = { first = Some pos; all = Pos_or_decl.Set.singleton pos }

  let add ({ first; all } as positions) pos =
    if Pos_or_decl.Set.mem pos all then
      positions
    else
      {
        first = Option.first_some first (Some pos);
        all = Pos_or_decl.Set.add pos all;
      }

  let add_list positions additional =
    List.fold additional ~init:positions ~f:add

  let union left right =
    match left.first with
    | None -> right
    | Some _ ->
      Pos_or_decl.Set.fold
        (fun pos positions -> add positions pos)
        right.all
        left

  let elements { first; all } =
    match first with
    | None -> []
    | Some first ->
      first :: Pos_or_decl.Set.elements (Pos_or_decl.Set.remove first all)
end

module Overlap_source = struct
  module Minimal = struct
    type t = Typing_error.Primary.Shape_splat.overlap_source

    let compare_origin left right =
      let open Typing_error.Primary.Shape_splat in
      match (left, right) with
      | (Shape_field left, Shape_field right)
      | (Description left, Description right) ->
        String.compare left right
      | (Shape_field _, Description _) -> -1
      | (Description _, Shape_field _) -> 1

    let compare (left : t) (right : t) =
      let result = Pos_or_decl.compare left.pos right.pos in
      if Int.equal result 0 then
        compare_origin left.origin right.origin
      else
        result
  end

  module Map = Stdlib.Map.Make (Minimal)
  module Set = Stdlib.Set.Make (Minimal)
end

module Sources = struct
  type t = {
    rev_sources: Typing_error.Primary.Shape_splat.overlap_source list;
    source_set: Overlap_source.Set.t;
  }

  let empty = { rev_sources = []; source_set = Overlap_source.Set.empty }

  let add ({ rev_sources; source_set } as sources) source =
    if Overlap_source.Set.mem source source_set then
      sources
    else
      {
        rev_sources = source :: rev_sources;
        source_set = Overlap_source.Set.add source source_set;
      }

  let add_list sources additional = List.fold additional ~init:sources ~f:add

  let elements { rev_sources; _ } = List.rev rev_sources
end

module Possible_source = struct
  type t = {
    source: Typing_error.Primary.Shape_splat.overlap_source;
    bound: Label_bound.t;
  }
end

module Possible_sources = struct
  type t = {
    rev_sources: Typing_error.Primary.Shape_splat.overlap_source list;
    bounds: Label_bound.t Overlap_source.Map.t;
    aggregate_bound: Label_bound.t;
  }

  let empty =
    {
      rev_sources = [];
      bounds = Overlap_source.Map.empty;
      aggregate_bound = Label_bound.bottom;
    }

  let add ({ rev_sources; bounds; aggregate_bound } as sources) possible_source
      =
    let Possible_source.{ source; bound } = possible_source in
    let aggregate_bound = Label_bound.join aggregate_bound bound in
    match Overlap_source.Map.find_opt source bounds with
    | None ->
      {
        rev_sources = source :: rev_sources;
        bounds = Overlap_source.Map.add source bound bounds;
        aggregate_bound;
      }
    | Some previous_bound ->
      {
        sources with
        bounds =
          Overlap_source.Map.add
            source
            (Label_bound.join previous_bound bound)
            bounds;
        aggregate_bound;
      }

  let singleton possible_source = add empty possible_source

  let elements { rev_sources; bounds; _ } =
    List.rev_map rev_sources ~f:(fun source ->
        Possible_source.
          { source; bound = Overlap_source.Map.find source bounds })

  let sources { rev_sources; _ } = List.rev rev_sources

  let union left right = List.fold (elements right) ~init:left ~f:add

  let restrict sources restriction =
    List.fold (elements sources) ~init:empty ~f:(fun acc possible_source ->
        let Possible_source.{ source; bound } = possible_source in
        let bound = Label_bound.meet bound restriction in
        if Label_bound.is_empty bound then
          acc
        else
          add acc Possible_source.{ source; bound })

  let is_empty { rev_sources; _ } = List.is_empty rev_sources

  let aggregate_bound { aggregate_bound; _ } = aggregate_bound
end

let partition_fields env s_fields =
  TShapeMap.fold
    (fun label sft (required, optional, absent) ->
      if not sft.sft_optional then
        (TShapeSet.add label required, optional, absent)
      else if Typing_utils.is_nothing env sft.sft_ty then
        (required, optional, TShapeSet.add label absent)
      else
        (required, TShapeSet.add label optional, absent))
    s_fields
    (TShapeSet.empty, TShapeSet.empty, TShapeSet.empty)

let label_bound_from_shape_disjointness env ty =
  let r = get_reason ty in
  let top_shape =
    Typing_make_type.shape r (Typing_make_type.mixed r) TShapeMap.empty
  in
  if Typing_utils.is_type_disjoint env ty top_shape then
    Label_bound.bottom
  else
    Label_bound.top

(** Computes the label bounds of recursive type parameters and newtypes. Only
    label bounds are cached; detailed elements retain use-site provenance. *)
module Solver : sig
  type t

  val create : unit -> t

  val solve : t -> Typing_env_types.env -> locl_ty -> Label_bound.t
end = struct
  module Node = struct
    type newtype_node = {
      key: Typing_shape_splat_key.t;
      bound_ty: locl_ty;
    }

    type t =
      | Ty_param of string
      | Newtype of newtype_node

    let compare left right =
      match (left, right) with
      | (Ty_param left, Ty_param right) -> String.compare left right
      | (Newtype left, Newtype right) ->
        Typing_shape_splat_key.compare left.key right.key
      | (Ty_param _, Newtype _) -> -1
      | (Newtype _, Ty_param _) -> 1

    let of_ty ty =
      match get_node ty with
      | Tgeneric name -> Ty_param name
      | Tnewtype (_, _, bound_ty) ->
        (* The canonical key deliberately omits [bound_ty], so map comparison
           never traverses a recursively duplicated bound. Equal keys in the
           same environment denote the same localized newtype bound. *)
        Newtype { key = Typing_shape_splat_key.of_ty ty; bound_ty }
      | _ -> invalid_arg "Solver expected a type parameter or newtype"

    module Map = Stdlib.Map.Make (struct
      type nonrec t = t

      let compare = compare
    end)

    module Set = Stdlib.Set.Make (struct
      type nonrec t = t

      let compare = compare
    end)
  end

  type entry = {
    input_env: Typing_env_types.env;
    bound: Label_bound.t;
  }

  type t = { cache: entry Node.Map.t ref }

  let create () = { cache = ref Node.Map.empty }

  let find solver env node =
    match Node.Map.find_opt node !(solver.cache) with
    | Some { input_env; bound } when phys_equal input_env env -> Some bound
    | _ -> None

  let add solver env node bound =
    solver.cache := Node.Map.add node { input_env = env; bound } !(solver.cache)

  let simple_shape_bound env s_fields s_unknown_value =
    let (required, optional, absent) = partition_fields env s_fields in
    if Typing_utils.is_nothing env s_unknown_value then
      Label_bound.Bottom_plus (TShapeSet.union required optional)
    else
      Label_bound.Top_minus absent

  let rec nodes_in_ty env nodes ty =
    let (_sd, env, ty) = Typing_utils.strip_supportdyn env ty in
    let (env, ty) = Typing_env.expand_type env ty in
    match get_node ty with
    | Tshape (Shape_splat { ss_elems })
    | Tunion ss_elems
    | Tintersection ss_elems ->
      List.fold ss_elems ~init:nodes ~f:(nodes_in_ty env)
    | Toption bound_ty
    | Tdependent (_, bound_ty) ->
      nodes_in_ty env nodes bound_ty
    | Tgeneric _
    | Tnewtype _ ->
      Node.Set.add (Node.of_ty ty) nodes
    | Tshape (Shape_simple _)
    | Tany _
    | Tnonnull
    | Tdynamic _
    | Tprim _
    | Tfun _
    | Ttuple _
    | Tvec_or_dict _
    | Taccess _
    | Tclass_ptr _
    | Tvar _
    | Tclass _
    | Tneg _
    | Tlabel _ ->
      nodes

  let bound_tys env = function
    | Node.Ty_param name ->
      Typing_set.elements (Typing_env.get_upper_bounds env name)
    | Node.Newtype { bound_ty; _ } -> [bound_ty]

  let dependencies env node =
    List.fold (bound_tys env node) ~init:Node.Set.empty ~f:(nodes_in_ty env)

  let reachable_nodes solver env root =
    let rec visit node (seen, rev_order) =
      if Node.Set.mem node seen || Option.is_some (find solver env node) then
        (seen, rev_order)
      else
        let seen = Node.Set.add node seen in
        let (seen, rev_order) =
          Node.Set.fold visit (dependencies env node) (seen, rev_order)
        in
        (seen, node :: rev_order)
    in
    let (_seen, rev_order) = visit root (Node.Set.empty, []) in
    List.rev rev_order

  let rec bound_of_ty ~lookup env ty =
    let (_sd, env, ty) = Typing_utils.strip_supportdyn env ty in
    let (env, ty) = Typing_env.expand_type env ty in
    match get_node ty with
    | Tshape (Shape_simple { s_fields; s_unknown_value; _ }) ->
      simple_shape_bound env s_fields s_unknown_value
    | Tshape (Shape_splat { ss_elems })
    | Tunion ss_elems ->
      join_ty_bounds ~lookup env ss_elems
    | Tintersection tys -> meet_ty_bounds ~lookup env tys
    | Toption bound_ty
    | Tdependent (_, bound_ty) ->
      bound_of_ty ~lookup env bound_ty
    | Tgeneric _
    | Tnewtype _ ->
      lookup (Node.of_ty ty)
    | Tdynamic _
    | Tvar _ ->
      Label_bound.top
    | Tany _
    | Tnonnull
    | Tprim _
    | Tfun _
    | Ttuple _
    | Tvec_or_dict _
    | Taccess _
    | Tclass_ptr _
    | Tclass _
    | Tneg _
    | Tlabel _ ->
      label_bound_from_shape_disjointness env ty

  and join_ty_bounds ~lookup env = function
    | [] -> Label_bound.bottom
    | ty :: tys ->
      let bound = bound_of_ty ~lookup env ty in
      if Label_bound.is_top bound then
        bound
      else
        Label_bound.join bound (join_ty_bounds ~lookup env tys)

  and meet_ty_bounds ~lookup env = function
    | [] -> Label_bound.top
    | ty :: tys ->
      let bound = bound_of_ty ~lookup env ty in
      if Label_bound.is_empty bound then
        bound
      else
        Label_bound.meet bound (meet_ty_bounds ~lookup env tys)

  (* Recursive type-parameter and newtype bounds form monotone equations over
     [Label_bound]. For example, [A = meet B {a}] and [B = join A {b}]
     converge as follows:

       [(A, B) = (top, top) -> ({a}, top) -> ({a}, {a, b})]

     Starting at top gives the greatest solution without distinguishing
     expansion paths. *)
  let solve solver env ty =
    let root = Node.of_ty ty in
    match find solver env root with
    | Some bound -> bound
    | None ->
      let nodes = reachable_nodes solver env root in
      let initial_bounds =
        List.fold nodes ~init:Node.Map.empty ~f:(fun bounds node ->
            Node.Map.add node Label_bound.top bounds)
      in
      let rec converge bounds =
        let changed = ref false in
        let next_bounds =
          List.fold nodes ~init:Node.Map.empty ~f:(fun next_bounds node ->
              let previous = Node.Map.find node bounds in
              let lookup dependency =
                match find solver env dependency with
                | Some bound -> bound
                | None ->
                  (match Node.Map.find_opt dependency next_bounds with
                  | Some bound -> bound
                  | None ->
                    Option.value
                      ~default:Label_bound.top
                      (Node.Map.find_opt dependency bounds))
              in
              let bound = meet_ty_bounds ~lookup env (bound_tys env node) in
              if not (Label_bound.equal previous bound) then changed := true;
              Node.Map.add node bound next_bounds)
        in
        if !changed then
          converge next_bounds
        else
          next_bounds
      in
      let bounds = converge initial_bounds in
      Node.Map.iter (fun node bound -> add solver env node bound) bounds;
      Node.Map.find root bounds
end

module Detailed_element = struct
  (** An [Element.t] together with all the provenance that its sets intentionally
      discard. [TShapeField.compare] ignores positions, which is right for the
      lattice but not for a diagnostic. *)
  type t = {
    element: Element.t;
    present_positions: Positions.t TShapeMap.t;
    possible_sources: Possible_sources.t;
  }

  let positions_of_fields fields =
    TShapeSet.fold
      (fun field positions ->
        TShapeMap.add
          field
          (Positions.singleton (TShapeField.pos field))
          positions)
      fields
      TShapeMap.empty

  let merge_positions left right =
    TShapeMap.fold
      (fun field positions acc ->
        let previous =
          Option.value ~default:Positions.empty (TShapeMap.find_opt field acc)
        in
        TShapeMap.add field (Positions.union previous positions) acc)
      right
      left

  let empty =
    let element = Element.empty
    and present_positions = TShapeMap.empty
    and possible_sources = Possible_sources.empty in
    { element; present_positions; possible_sources }

  let source pos description bound =
    let open Typing_error.Primary.Shape_splat in
    Possible_source.
      { source = { pos; origin = Description description }; bound }

  let field_source pos label bound =
    let open Typing_error.Primary.Shape_splat in
    Possible_source.{ source = { pos; origin = Shape_field label }; bound }

  let possible_sources_of_fields fields =
    TShapeSet.fold
      (fun field sources ->
        let bound = Label_bound.Bottom_plus (TShapeSet.singleton field) in
        Possible_sources.add
          sources
          (field_source (TShapeField.pos field) (TShapeField.name field) bound))
      fields
      Possible_sources.empty

  (** Demote all fields that were definitely present to be only possibly present
      except those contained in [retained]; used when we have a union shape
      splat element *)
  let demote_except retained t =
    let (present_positions, field_sources) =
      TShapeMap.fold
        (fun field position_set (present_positions, field_sources) ->
          if TShapeSet.mem field retained then
            (TShapeMap.add field position_set present_positions, field_sources)
          else
            let label = TShapeField.name field in
            let bound = Label_bound.Bottom_plus (TShapeSet.singleton field) in
            let field_sources =
              List.fold
                (Positions.elements position_set)
                ~init:field_sources
                ~f:(fun sources pos ->
                  Possible_sources.add sources (field_source pos label bound))
            in
            (present_positions, field_sources))
        t.present_positions
        (TShapeMap.empty, Possible_sources.empty)
    in
    let element = Element.demote_except retained t.element in
    let possible_sources =
      Possible_sources.union t.possible_sources field_sources
    in
    { element; present_positions; possible_sources }

  let possible = demote_except TShapeSet.empty

  let unknown source_pos description =
    let element = Element.open_elem
    and present_positions = TShapeMap.empty
    and possible_sources =
      Possible_sources.singleton (source source_pos description Label_bound.top)
    in
    { element; present_positions; possible_sources }

  (* A non-shape upper bound can still admit shape inhabitants, as [nonnull]
     and dict-compatible classes do. Only discard it when disjointness proves
     that their intersection is empty. *)
  let of_other_type env source_pos ty =
    let bound = label_bound_from_shape_disjointness env ty in
    if Label_bound.is_empty bound then
      empty
    else
      unknown source_pos "a type which may contain a shape"

  let join e1 e2 =
    {
      element = Element.join e1.element e2.element;
      present_positions =
        merge_positions e1.present_positions e2.present_positions;
      possible_sources =
        Possible_sources.union e1.possible_sources e2.possible_sources;
    }

  let join_alternatives e1 e2 =
    let retained =
      TShapeSet.inter
        e1.element.Element.definitely_present
        e2.element.Element.definitely_present
    in
    join (demote_except retained e1) (demote_except retained e2)

  (** An intersection can only supply labels allowed by every member. Concrete
      fields are possible rather than certain until all members are considered. *)
  let intersect e1 e2 =
    let e1 = possible e1 in
    let e2 = possible e2 in
    let possibly_present =
      Label_bound.meet
        e1.element.Element.possibly_present
        e2.element.Element.possibly_present
    in
    let possible_sources =
      Possible_sources.union
        (Possible_sources.restrict e1.possible_sources possibly_present)
        (Possible_sources.restrict e2.possible_sources possibly_present)
    in
    let element =
      Element.{ definitely_present = TShapeSet.empty; possibly_present }
    in
    { element; present_positions = TShapeMap.empty; possible_sources }

  let rec of_ty_ ~solver env ty =
    let source_pos = Reason.to_pos (get_reason ty) in
    let (_sd, env, ty) = Typing_utils.strip_supportdyn env ty in
    let (env, ty) = Typing_env.expand_type env ty in
    match get_node ty with
    | Tshape (Shape_simple { s_fields; s_unknown_value; _ }) ->
      let (definitely_present, optional, absent) =
        partition_fields env s_fields
      in
      let optional_sources = possible_sources_of_fields optional in
      if Typing_utils.is_nothing env s_unknown_value then
        let element = Element.closed_elem definitely_present optional
        and present_positions = positions_of_fields definitely_present
        and possible_sources = optional_sources in
        { element; present_positions; possible_sources }
      else
        let declared =
          TShapeSet.union definitely_present (TShapeSet.union optional absent)
        in
        let row_bound = Label_bound.Top_minus declared in
        let possibly_present =
          Label_bound.join (Label_bound.Bottom_plus optional) row_bound
        in
        let element = Element.{ definitely_present; possibly_present }
        and present_positions = positions_of_fields definitely_present
        and possible_sources =
          Possible_sources.add
            optional_sources
            (source source_pos "an open shape" row_bound)
        in
        { element; present_positions; possible_sources }
    | Tshape (Shape_splat { ss_elems }) ->
      (* Every element of a nested splat contributes to the outer one. *)
      List.fold ss_elems ~init:empty ~f:(fun acc elem ->
          join acc (of_ty_ ~solver env elem))
    | Tunion [] ->
      (* The bottom row is uninhabited, so it can never overlap with anything. *)
      empty
    | Tunion (first :: rest) ->
      (* Only labels present in every member remain certainly present; demote
         any other labels to be only possibly present *)
      List.fold rest ~init:(of_ty_ ~solver env first) ~f:(fun acc member ->
          let e = of_ty_ ~solver env member in
          join_alternatives acc e)
    | Tintersection [] -> unknown source_pos "`mixed`"
    | Tintersection (first :: rest) ->
      List.fold rest ~init:(of_ty_ ~solver env first) ~f:(fun acc member ->
          let e = of_ty_ ~solver env member in
          intersect acc e)
    | Tnewtype (name, _, _) ->
      let description = Element.newtype_source name in
      let possibly_present = Solver.solve solver env ty in
      let element =
        Element.{ definitely_present = TShapeSet.empty; possibly_present }
      and present_positions = TShapeMap.empty
      and possible_sources =
        Possible_sources.singleton
          (source source_pos description possibly_present)
      in
      { element; present_positions; possible_sources }
    | Toption inner -> possible (of_ty_ ~solver env inner)
    | Tdependent (_, bound_ty) -> possible (of_ty_ ~solver env bound_ty)
    | Tgeneric name ->
      let possibly_present = Solver.solve solver env ty in
      let description = Element.ty_param_source name in
      let element =
        Element.{ definitely_present = TShapeSet.empty; possibly_present }
      and present_positions = TShapeMap.empty
      and possible_sources =
        Possible_sources.singleton
          (source source_pos description possibly_present)
      in
      { element; present_positions; possible_sources }
    | Tdynamic _ ->
      (* A dynamic splat element is the open shape where each unknown field
         has dynamic as its upper bound *)
      unknown source_pos "`dynamic`"
    | Tvar _ -> unknown source_pos "an unresolved type variable"
    | Tany _
    | Tnonnull
    | Tprim _
    | Tfun _
    | Ttuple _
    | Tvec_or_dict _
    | Taccess _
    | Tclass_ptr _
    | Tclass _
    | Tneg _
    | Tlabel _ ->
      of_other_type env source_pos ty

  let of_tys tys env =
    let solver = Solver.create () in
    List.map tys ~f:(fun ty -> of_ty_ ~solver env ty)
end

module Possible_overlap = struct
  type possible_overlap = {
    positions: Positions.t;
    sources: Sources.t;
  }
end

let add_positions field positions map =
  let previous =
    Option.value ~default:Positions.empty (TShapeMap.find_opt field map)
  in
  TShapeMap.add field (Positions.add_list previous positions) map

let add_possible_overlap field positions sources overlaps =
  let previous =
    Option.value
      ~default:
        Possible_overlap.
          { positions = Positions.empty; sources = Sources.empty }
      (TShapeMap.find_opt field overlaps)
  in
  TShapeMap.add
    field
    Possible_overlap.
      {
        positions = Positions.add_list previous.positions positions;
        sources = Sources.add_list previous.sources sources;
      }
    overlaps

let possibly possible certain sources positions labels =
  if Possible_sources.is_empty sources || TShapeSet.is_empty labels then
    possible
  else
    let sources = Possible_sources.elements sources in
    TShapeSet.fold
      (fun field possible ->
        let matching_sources =
          List.filter_map sources ~f:(fun source ->
              if
                TShapeSet.mem
                  field
                  (Label_bound.restrict
                     source.bound
                     (TShapeSet.singleton field))
              then
                Some source.source
              else
                None)
        in
        if List.is_empty matching_sources then
          possible
        else
          let positions =
            Positions.elements
              (Option.value
                 ~default:Positions.empty
                 (TShapeMap.find_opt field positions))
          in
          add_possible_overlap field positions matching_sources possible)
      (TShapeSet.diff labels certain)
      possible
(* -- API ------------------------------------------------------------------- *)

(** Given a list of shape splat elements, [ss_elems], ranging over simple shapes
    newtypes and type parameters, determine all groups of elements which violate
    disjointness.
*)
let violations ss_elems env =
  (* Convert each element to a representation with which we can reason about
     fields which are definitely present (via simple shapes) and/or possible
     present (via type parameters, new types etc) *)
  let detailed_elems = Detailed_element.of_tys ss_elems env in
  let (all, overlapping, possible, has_unresolved) =
    List.fold_left
      detailed_elems
      ~init:(Detailed_element.empty, TShapeMap.empty, TShapeMap.empty, false)
      ~f:(fun (seen, overlapping, possible, has_unresolved) detailed_elem ->
        let seen_element = seen.Detailed_element.element in
        let element = detailed_elem.Detailed_element.element in

        (* Known fields overlap *)
        let certain =
          TShapeSet.inter
            element.definitely_present
            seen_element.definitely_present
        in
        let overlapping =
          TShapeSet.fold
            (fun field overlapping ->
              let positions element =
                Option.value
                  ~default:Positions.empty
                  (TShapeMap.find_opt
                     field
                     element.Detailed_element.present_positions)
              in
              let new_positions =
                Positions.elements (positions detailed_elem)
              in
              let positions =
                if TShapeMap.mem field overlapping then
                  new_positions
                else
                  Positions.elements (positions seen) @ new_positions
              in
              add_positions field positions overlapping)
            certain
            overlapping
        in

        (* Known fields of one element overlap with fields which may be present
           in the other *)
        let possible =
          possibly
            possible
            certain
            seen.Detailed_element.possible_sources
            detailed_elem.Detailed_element.present_positions
            element.definitely_present
        in
        let possible =
          possibly
            possible
            certain
            detailed_elem.Detailed_element.possible_sources
            seen.Detailed_element.present_positions
            seen_element.definitely_present
        in

        (* Overlap in fields which may be present *)
        let seen_possible = seen.Detailed_element.possible_sources in
        let element_possible =
          detailed_elem.Detailed_element.possible_sources
        in
        let has_unresolved =
          has_unresolved
          || (not (Possible_sources.is_empty seen_possible))
             && (not (Possible_sources.is_empty element_possible))
             && not
                  (Label_bound.disjoint
                     (Possible_sources.aggregate_bound seen_possible)
                     (Possible_sources.aggregate_bound element_possible))
        in
        ( Detailed_element.join seen detailed_elem,
          overlapping,
          possible,
          has_unresolved ))
  in
  let possible =
    TShapeMap.fold
      (fun field overlap filtered ->
        if TShapeMap.mem field overlapping then
          filtered
        else
          TShapeMap.add field overlap filtered)
      possible
      TShapeMap.empty
  in
  let overlapping =
    TShapeMap.fold
      (fun field position_set violations ->
        Typing_error.Primary.Shape_splat.Overlapping_field
          {
            label = TShapeField.name field;
            positions = Positions.elements position_set;
          }
        :: violations)
      overlapping
      []
  in
  let possible =
    TShapeMap.fold
      (fun field Possible_overlap.{ positions; sources } violations ->
        Typing_error.Primary.Shape_splat.Possible_overlapping_field
          {
            label = TShapeField.name field;
            positions = Positions.elements positions;
            sources = Sources.elements sources;
          }
        :: violations)
      possible
      overlapping
  in
  let rev_violations =
    if has_unresolved then
      Typing_error.Primary.Shape_splat.Unresolved_sources
        {
          sources =
            Possible_sources.sources all.Detailed_element.possible_sources;
        }
      :: possible
    else
      possible
  in
  List.rev rev_violations

module For_test = struct
  type label_bound = Label_bound.t =
    | Bottom_plus of TShapeSet.t
    | Top_minus of TShapeSet.t

  let restrict = Label_bound.restrict

  let disjoint = Label_bound.disjoint

  let join = Label_bound.join

  let meet = Label_bound.meet

  type element = Element.t

  let make_element ~present ~bound =
    Element.{ definitely_present = present; possibly_present = bound }

  let possible = Element.possible

  let element_bound element = element.Element.possibly_present
end
