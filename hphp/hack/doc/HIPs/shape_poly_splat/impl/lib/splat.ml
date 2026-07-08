open StdLabels
open Repr

(* == Row helpers =========================================================== *)

module Row_help = struct
  (* Substitute every OTHER spread variable (and every type variable) by its current
     solution recorded in [env], leaving [ty_var] (the one being solved) untouched.
     An unsolved spread var reads as the empty row (the merge identity), so until [env]
     is populated this matches the old empty substitution exactly. *)
  let solve_all_except (elems : Row.Splat_elem.t list) (ty_var : Ty_var.t) env =
    match Env.resolve_row ~except:ty_var env (Row.splat elems) with
    | Row.Row_splat { elems } -> elems
    | Row.Row_simple s -> [Row.Splat_elem.simple s]

  (* Substitute EVERY spread variable (and type variable) by its current solution. *)
  let solve_all (row : Row.t) env : Row.t = Env.resolve_row env row

  (* A row carries a UNION SPREAD when a splat element is a [Spread (Union …)] — canon
     leaves one only when it cannot be flattened (a union of a shape with an opaque leaf, or
     of shapes that stay a union at the base level).  Such a row denotes a base-level union
     of rows, so it must be decided at the base level, not projected field-by-field. *)
  let has_union_spread (r : Row.t) =
    match r with
    | Row.Row_simple _ -> false
    | Row.Row_splat { elems } ->
      List.exists elems ~f:(fun (Row.Splat_elem.Spread b) ->
          match b with
          | Base.Union _ -> true
          | _ -> false)

  (* The row a base bound denotes: a shape's row, or the bottom row for [Bottom].  A
     type parameter's bounds are well-formed shapes / bottom, so anything else is an
     invariant violation. *)
  let row_of_base (b : Base.t) : Row.t =
    match b with
    | Base.Shape r -> r
    | Base.Bottom -> Row.bot
    | Base.Top
    | Base.Prim _
    | Base.Union _
    | Base.Flex _
    | Base.Rigid _ ->
      failwith
        "Row_help.row_of_base: type parameter bound is not a shape or bottom"

  (* Labels a row of bare parameter spreads contributes NONE of via [Row.label_set] (a
     [Spread (Rigid α)] has no inline labels) — yet a label that lives only in a
     parameter's bound is still a real per-field obligation.  Collect those bound labels
     so the subrow machinery actually visits them; otherwise the field is silently
     skipped (an unsoundness).  Transitive: take the closure of the parameters under
     "appears in a bound", so a label living only in a NESTED parameter's bound is found
     too. *)
  let bound_label_set (env : Env.t) (params : Ty_param.t list) : Label.Set.t =
    let labels_of = function
      | Base.Shape r -> Row.label_set r
      | Base.Top
      | Base.Bottom
      | Base.Prim _
      | Base.Union _
      | Base.Flex _
      | Base.Rigid _ ->
        Label.Set.empty
    in
    let all =
      Env.Ty_param_env.closure env.ty_param (Ty_param.Set.of_list params)
    in
    Ty_param.Set.fold
      (fun p acc ->
        let Env.Ty_param_env.{ lower; upper } =
          Env.Ty_param_env.find p env.ty_param
        in
        Label.Set.union
          acc
          (Label.Set.union (labels_of lower) (labels_of upper)))
      all
      Label.Set.empty

  (* The full set of labels a splat subrow [sub <: super] must check: inline labels of
     both rows PLUS the bound labels of every parameter they mention. *)
  let subrow_label_set (env : Env.t) (sub : Row.t) (super : Row.t) : Label.Set.t
      =
    let params = Row.spread_params sub @ Row.spread_params super in
    Label.Set.union
      (Label.Set.union (Row.label_set sub) (Row.label_set super))
      (bound_label_set env params)

  let subrow_labels (env : Env.t) (sub : Row.t) (super : Row.t) :
      Label.t option list =
    None
    :: List.map
         ~f:Option.some
         (Label.Set.elements (subrow_label_set env sub super))

  (* The [field_bounds] of a type parameter at [label] under [fld_assignment] *)
  let field_bounds
      (ty_param : Ty_param.t)
      (label : Label.t option)
      (fld_assignment : Field_desc.t Ty_param.Map.t)
      (env : Env.t) =
    let Env.Ty_param_env.{ lower; upper } =
      Env.Ty_param_env.find ty_param env.ty_param
    in
    let row_lower = Row.proj (row_of_base lower) label fld_assignment
    and row_upper = Row.proj (row_of_base upper) label fld_assignment in
    (row_lower, row_upper)

  let corner_assignments
      (ty_params_topo : Ty_param.t list) (lbl : Label.t option) (env : Env.t) =
    let rec aux ty_params fld_assignment =
      match ty_params with
      | [] -> [fld_assignment]
      | ty_param :: ty_params ->
        let corners =
          let (lower, upper) = field_bounds ty_param lbl fld_assignment env in
          Field_desc.corners ~lower ~upper
        in
        List.concat_map corners ~f:(fun fld_desc ->
            let fld_assignment =
              Ty_param.Map.add ty_param fld_desc fld_assignment
            in
            aux ty_params fld_assignment)
    in
    aux ty_params_topo Ty_param.Map.empty

  type masking =
    | Masked
    | Unmasked
    | Unknown

  (** At a given [label] and [fld_assignment], within a given [row] we can
      determine a [ty_param] is:
      - [Masked] (cannot contribute to field requiredness or type) if at
        any type parameter to it's right that label must appear as [Req] i.e. is
        [Req] in the upper bound
      - [Unmasked] (must contribute to field requiredness and type) if at
        all type parameters to it's right that lavelmust appear as [Opt] i.e. is
        [Opt] in the lower bound
      - [Unknown] (may contribute to field requiredness or type) if neither of
        the previous conditions hold
      *)
  let masking_of_splat
      Row.Splat.{ elems }
      (label : Label.t option)
      (ty_param : Ty_param.t)
      (fld_assignment : Field_desc.t Ty_param.Map.t)
      (env : Env.t) : masking =
    let rec aux rev_elems acc =
      match rev_elems with
      | [] ->
        (* The target parameter doesn't occur in this row, so another condition already
           applies; return the most pessimistic answer to be safe. *)
        Unknown
      | Row.Splat_elem.Spread (Base.Rigid r) :: _ when Ty_param.equal ty_param r
        ->
        (* Reached the target without a masking parameter to its right, so it is unmasked
           or unknown — whichever we accumulated in [acc]. *)
        acc
      | Row.Splat_elem.Spread (Base.Rigid r) :: rest ->
        (* Whether the RIGHTWARD parameter [r] masks our target [ty_param] is decided by
           [r]'s OWN bounds at this label — not the target's. *)
        let (lower, upper) = field_bounds r label fld_assignment env in
        if Field_desc.is_req upper then
          (* [r]'s upper bound is [Req] ⇒ [r] always supplies the field ⇒ masks us *)
          Masked
        else if Field_desc.is_req lower then
          (* [r]'s lower [Req] but upper [Opt] ⇒ we cannot tell whether it masks us *)
          aux rest Unknown
        else
          (* [r] is [Opt] at both bounds ⇒ it does not mask; keep the accumulator *)
          aux rest acc
      | _ :: rest ->
        (* Only 'live' parameters reach here, so any simple row is already [Opt] at this
           label and cannot mask the target. *)
        aux rest acc
    in
    aux (List.rev elems) Unmasked

  let masking_of
      (row : Row.t)
      (label : Label.t option)
      (ty_param : Ty_param.t)
      (fld_assignment : Field_desc.t Ty_param.Map.t)
      (env : Env.t) =
    match row with
    | Row.Row_splat splat ->
      masking_of_splat splat label ty_param fld_assignment env
    | Row.Row_simple _ ->
      (* A parameter cannot occur in a simple row, so another condition applies. *)
      Unknown
end

(* == Base types ============================================================ *)
let rec subtype_base ~(sub : Base.t) ~(super : Base.t) (env : Env.t) :
    Env.t * Prop.t =
  (* Order matters.  ⊥-sub / ⊤-super first; then a sub-union (conjunctive, always
     exact); then RECORD any flex variable (a flex SUB against a union is a single
     upper bound, NOT a split — splitting would be incomplete); then distribute a
     super-union; then the rigid F-sub rule; only then the eager ground rejections
     (⊤-sub, ⊥-super). *)
  match (sub, super) with
  (* -- Bottom sub -- *)
  | (Bottom, _) -> (env, Prop.valid)
  (* -- Top super -- *)
  | (_, Top) -> (env, Prop.valid)
  (* -- Flex cases -- *)
  | (Flex _, _)
  | (_, Flex _) ->
    (env, Prop.(is_subtype_base ~sub ~super))
  (* -- Union left & right -- *)
  | (Union (sub1, sub2), _) ->
    let (env, prop1) = subtype_base ~sub:sub1 ~super env in
    let (env, prop2) = subtype_base ~sub:sub2 ~super env in
    (env, Prop.conj prop1 prop2)
  | (_, Union (super1, super2)) ->
    let (env, prop1) = subtype_base ~sub ~super:super1 env in
    let (env, prop2) = subtype_base ~sub ~super:super2 env in
    (env, Prop.disj prop1 prop2)
  (* -- Rigid type parameters (F-sub) --
     Reflexivity short-circuits; otherwise reduce EITHER side through its bound.
     [Rigid a <: T] holds when [a]'s upper bound does (a <: upper <: T); [T <: Rigid b]
     when [T <: lower <: b].  A distinct [Rigid a <: Rigid b] chains both, reducing to
     [upper(a) <: lower(b)].  A missing parameter is an invariant violation ([find]
     raises). *)
  | (Rigid a, Rigid b) when Ty_param.equal a b -> (env, Prop.valid)
  | (Rigid a, _) ->
    let Env.Ty_param_env.{ upper; _ } = Env.Ty_param_env.find a env.ty_param in
    subtype_base ~sub:upper ~super env
  | (_, Rigid b) ->
    let Env.Ty_param_env.{ lower; _ } = Env.Ty_param_env.find b env.ty_param in
    subtype_base ~sub ~super:lower env
  (* -- Bottom super -- *)
  | ((Prim _ | Shape _ | Top), Bottom) ->
    let fail = Prop.(is_subtype_base ~sub ~super) in
    (env, Prop.invalid fail)
  (* -- Top sub -- *)
  | (Top, (Prim _ | Shape _)) ->
    let fail = Prop.(is_subtype_base ~sub ~super) in
    (env, Prop.invalid fail)
  (* -- Prims -- *)
  | (Prim p1, Prim p2) ->
    if Prim.equal p1 p2 then
      (env, Prop.valid)
    else
      let fail = Prop.(is_subtype_base ~sub ~super) in
      (env, Prop.invalid fail)
  | (Prim _, Shape _)
  | (Shape _, Prim _) ->
    let fail = Prop.(is_subtype_base ~sub ~super) in
    (env, Prop.invalid fail)
  (* -- Shapes -- *)
  | (Shape sub, Shape super) -> subrow ~sub ~super env

(* == Rows ================================================================== *)
and subrow ~(sub : Row.t) ~(super : Row.t) (env : Env.t) : Env.t * Prop.t =
  if Row_help.has_union_spread sub || Row_help.has_union_spread super then
    (* a union spread denotes a base-level union of rows; decide it at the base level, where
       [subtype_base] distributes it via canon and handles unions soundly (conjunctive on the
       sub side, disjunctive on the super side).  Canon makes the shapes union-free before they
       re-enter [subrow], so this recursion is well-founded. *)
    subtype_base
      ~sub:(Base.canon (Base.shape sub))
      ~super:(Base.canon (Base.shape super))
      env
  else
    match (sub, super) with
    (* -- Simple rows --------------------------------------------------------- *)
    | (Row.Row_simple sub, Row.Row_simple super) ->
      subrow_simple ~sub ~super env
    | (Row_simple _, Row_splat _)
    | (Row_splat _, Row_simple _)
    | (Row_splat _, Row_splat _) ->
      let sub_spread_vars = Row.topo_spread_vars sub
      and super_spread_vars = Row.topo_spread_vars super in
      let sub_has_spread_var = not (List.is_empty sub_spread_vars)
      and super_has_spread_var = not (List.is_empty super_spread_vars) in
      if sub_has_spread_var && super_has_spread_var then
        subrow_infer_sub_super
          ~sub
          ~sub_spread_vars
          ~super
          ~super_spread_vars
          env
      else if sub_has_spread_var then
        subrow_infer_sub ~sub sub_spread_vars ~super env
      else if super_has_spread_var then
        subrow_infer_super ~sub ~super super_spread_vars env
      else
        subrow_splat ~sub ~super env

(* == Rows splats =========================================================== *)

(** Subrow proposition with splats on one or both sides. Decompose into
    per-field sub-field propositions. See [subrow_splat_at] for the gory
    details *)
and subrow_splat ~(sub : Row.t) ~(super : Row.t) (env : Env.t) : Env.t * Prop.t
    =
  let rec loop labels ~acc ~failures env =
    match labels with
    | [] when List.is_empty failures -> (env, Prop.conjs acc)
    | [] -> (env, Prop.invalids failures)
    | label :: labels ->
      let (env, prop) = subrow_splat_at label ~sub ~super env in
      let failures =
        if Prop.is_invalid prop then
          prop :: failures
        else
          failures
      in
      let acc = prop :: acc in
      loop labels ~acc ~failures env
  in
  let labels = Row_help.subrow_labels env sub super in
  loop labels ~acc:[] ~failures:[] env

(** This is the tricky case: we can't just take type parameters to their bounds
    since they can co-occur or be dependent on one another in both sub- and
    super-row. In the general case we have to show that the proposition holds
    for simultaneous assignments across all type parameters with each type
    parameter having up to 4 extremal assignements, 2 for each lattice in the
    product. This is co-assignment exponential in the number or type parameters
    but there are a few things in our favor:

    1) There aren't generally many type parameters and the bad cases will usually
      be minimized under the 'sole splat' condition

    2) We don't have to consider _all_ type parameters, only those which are
      'live' at the current label i.e. only those which occur to the right
      of any [Req] field in a simple row. Those that are to the left of such
      a field in both rows cannot participate in its type

    3) For 'live' type parameters which are 'free', we can often do better than
      the 4 extremal assignments. Here 'free' means that it doesn't occur in
      the bounds of any other live type parameter and therefore it's assignment
      doesn't affect the assignement of any other type parameter. We can consider
      only one assignment for these type parameters in the following cases:

      i)  If a free type parameter occurs in only the sub- or super-row then
          we only need to consider its upper- or lower-bound assignment.

      ii) If a free type parameter occurs in both sub- and super-rows but will
          always be masked by the assignement of another type parameter to its
          right in the sub-row then it cannot contribute to the sub-row, only
          the super-row, so we need only consider its lower-bound assignment
          (the assignment that minimises the super-row, i.e. the hardest case
          for `sub <: super`)

      iii) If a free type parameter occurs in both sub- and super-rows but will
          always be masked by the assignement of another type parameter to its
          right in the super-row then it cannot contribute to the super-row, only
          the sub-row, so we need only consider its upper-bound assignment
          (the assignment that maximizes the sub-row, i.e. the hardest case for
          `sub <: super`)

      iv) If a free type parameter occurs in both sub- and super-rows, will
        always have requiredness [Opt] in its assignment and can never be masked
        by any type parameter to its right (i.e. all assignments of rightward
        type parameters will have requiredness [Opt]) then we need only consider
        its lower-bound assignment. This is because in this case we known
        that the fields requiredness doesn't enter into the subfield decision,
        only it's type. At this point after the other conditions are applied
        we will have a subtype proposition of the form
        ```
        (U | t) <: (V | t)
        ```
        where `U` and `V` are the types of all other fields contributing
        to the merged field type and `t` is the type of the field in question.
        Because our field is free, we can pick a single assignment but it
        has to be the lower-bound i.e. the type that increases the right
        hand side of the inequality the least.

*)
and subrow_splat_at label ~(sub : Row.t) ~(super : Row.t) (env : Env.t) :
    Env.t * Prop.t =
  (* At a given label, only these type parameters occuring to the right of a [Req]
     field can impact the subfield decision so can restrict the enumeration to
     just those. *)
  let live_sub = Ty_param.Set.of_list (Row.live_spread_at sub label)
  and live_super = Ty_param.Set.of_list (Row.live_spread_at super label) in
  (* Topologically sort the type parameters so that later parameters only depend
     on earlier ones for which we have an assignment (see [loop_params], below) *)
  let ty_params_topo =
    let live =
      Env.Ty_param_env.closure
        env.ty_param
        (Ty_param.Set.union live_sub live_super)
    in
    Env.Ty_param_env.topo env.ty_param live
  in
  (* Those type parameters which appear in the bounds of another type parameter
     are depended on by those parameters for their assignments *)
  let depended_on : Ty_param.Set.t =
    List.fold_left
      ty_params_topo
      ~init:Ty_param.Set.empty
      ~f:(fun acc ty_param ->
        let delta =
          Env.Ty_param_env.type_params_in_bounds env.ty_param ty_param
        in
        Ty_param.Set.(union acc @@ of_list delta))
  in
  (* Apply conditions i-iv (above) to minimize the number of assignments we need
     to check per type parameter *)
  let corners_for ty_param fld_assignment =
    let (lower, upper) =
      Row_help.field_bounds ty_param label fld_assignment env
    in
    let is_free = not (Ty_param.Set.mem ty_param depended_on)
    and in_sub = Ty_param.Set.mem ty_param live_sub
    and in_super = Ty_param.Set.mem ty_param live_super in
    if is_free && in_sub && not in_super then
      [upper]
    else if is_free && (not in_sub) && in_super then
      [lower]
    else if is_free && in_sub && in_super then
      let masking_sub =
        Row_help.(masking_of sub label ty_param fld_assignment env)
      and masking_super =
        Row_help.(masking_of super label ty_param fld_assignment env)
      in
      match (masking_sub, masking_super) with
      | (Masked, _) -> [lower]
      | (_, Masked) -> [upper]
      | (_, Unmasked) when Field_desc.is_opt lower -> [lower]
      | _ -> Field_desc.corners ~lower ~upper
    else
      Field_desc.corners ~lower ~upper
  in
  (* Loop over the type parameters in topological order; given the field assignments
     it depends on, loop over each assignment and ensure the subfield proposition
     holds at each one *)
  let rec loop_params ty_params fld_assignment env =
    match ty_params with
    | ty_param :: ty_params ->
      let assignments = corners_for ty_param fld_assignment in
      loop_assignments
        ty_param
        assignments
        ty_params
        fld_assignment
        ~acc:[]
        ~failures:[]
        env
    | _ ->
      (* We have a complete field assigment so we can project at the current
         label for the subfield proposition *)
      let sub = Row.proj sub label fld_assignment
      and super = Row.proj super label fld_assignment in
      subfield label ~sub ~super env
  and loop_assignments
      ty_param assigments ty_params fld_assignment ~acc ~failures env =
    match assigments with
    | [] when List.is_empty failures -> (env, Prop.conjs acc)
    | [] -> (env, Prop.invalids failures)
    | assigment :: assigments ->
      let (env, prop) =
        let fld_assignment =
          Ty_param.Map.add ty_param assigment fld_assignment
        in
        loop_params ty_params fld_assignment env
      in
      let failures =
        if Prop.is_invalid prop then
          prop :: failures
        else
          failures
      in
      let acc = prop :: acc in
      loop_assignments
        ty_param
        assigments
        ty_params
        fld_assignment
        ~acc
        ~failures
        env
  in
  loop_params ty_params_topo Ty_param.Map.empty env

(* == Rows with spread variables ============================================ *)

(* -- Spread variables in sub-row only -------------------------------------- *)

and subrow_infer_sub
    ~(sub : Row.t)
    (sub_spread_vars : Ty_var.t list)
    ~(super : Row.t)
    (env : Env.t) : Env.t * Prop.t =
  let rec loop spread_vars ~acc ~failures env =
    match spread_vars with
    | [] when List.is_empty failures -> (env, Prop.conjs acc)
    | [] -> (env, Prop.invalids failures)
    | spread_var :: spread_vars ->
      (match Row.partition_at_var sub spread_var with
      | None ->
        (* shouldn't happen: [spread_var] came from [sub] *)
        loop spread_vars ~acc ~failures env
      | Some (pre, post) ->
        let pre = Row_help.solve_all_except pre spread_var env in
        let post = Row_help.solve_all_except post spread_var env in
        let sub_pre = Row.(canon @@ splat pre)
        and sub_post = Row.(canon @@ splat post) in
        let (env, prop) =
          subrow_infer_sub_part spread_var ~sub_pre ~sub_post ~sub ~super env
        in
        let failures =
          if Prop.is_invalid prop then
            prop :: failures
          else
            failures
        in
        let acc = prop :: acc in
        loop spread_vars ~acc ~failures env)
  in
  loop sub_spread_vars ~acc:[] ~failures:[] env

and subrow_infer_sub_part
    spread_var
    ~(sub_pre : Row.t)
    ~(sub_post : Row.t)
    ~(sub : Row.t)
    ~(super : Row.t)
    (env : Env.t) : Env.t * Prop.t =
  let labels = Row_help.subrow_labels env sub super in
  let (env, known, unknown, props) =
    List.fold_left
      labels
      ~init:(env, Label.Map.empty, Base.top, [])
      ~f:(fun (env, known, unknown, props) label ->
        let (env, field, prop) =
          subrow_infer_sub_part_at label ~sub_pre ~sub_post ~sub ~super env
        in
        (* The unknown is opt-only, so its contribution is just the field's base. *)
        let (known, unknown) =
          match label with
          | Some lbl -> (Label.Map.add lbl field known, unknown)
          | None -> (known, field.Field_desc.base)
        in
        (env, known, unknown, prop :: props))
  in
  (* The single residual constraint: ?ρ <: shape({ the per-label fields }). *)
  let bound = Row.Row_simple { known; unknown } in
  ( env,
    Prop.conjs
      (Prop.is_subtype_base
         ~sub:(Base.flex spread_var)
         ~super:(Base.shape bound)
      :: props) )

(** [subrow_infer_sub_part] at a single [label]:
    - If  [sub_post] requires the field at every assignment it overwrites any
      spread variable, which is then free here so we give an upper bound of
      the the top-field descriptor and just check the [sub_post <: super] under
      each assignment

    - Otherwise the spread variable contributes to the labels field descriptor;
      its requiredness is [Opt] unless it occurs as [Req] in the super-row
      and there is no [Req] in [sub_pre]. In this case we construct the
      resulting union and check against super *)
and subrow_infer_sub_part_at
    label
    ~(sub_pre : Row.t)
    ~(sub_post : Row.t)
    ~(sub : Row.t)
    ~(super : Row.t)
    (env : Env.t) : Env.t * Field_desc.t * Prop.t =
  (* 'live' type parameters at this label in topological order *)
  let ty_params_topo =
    let live =
      Ty_param.Set.of_list
        (Row.live_spread_at sub label @ Row.live_spread_at super label)
    in
    Env.Ty_param_env.(topo env.ty_param (closure env.ty_param live))
  in
  (* We need all assignments during inference *)
  let assignments = Row_help.corner_assignments ty_params_topo label env in
  (* per assignment:
     - is the spread variable masked by a [Req] in the row to its right, [sub_post]?
     - the field descriptor in the super-row
     - the field assignment *)
  let info =
    List.map assignments ~f:(fun fld_assignment ->
        let fd_post = Row.proj sub_post label fld_assignment
        and fd_super = Row.proj super label fld_assignment in
        (Field_desc.is_req fd_post, fd_super, fld_assignment))
  in
  (* Those assignments where the spread variable is not masked *)
  let live = List.filter info ~f:(fun (masked, _, _) -> not masked) in
  match live with
  | [] ->
    (* the spread variable is overwritten by post everywhere so the upper bound
       is top *)
    let (env, props) =
      List.fold_left
        info
        ~init:(env, [])
        ~f:(fun (env, props) (_, super, fld_assignment) ->
          let sub = Row.proj sub_post label fld_assignment in
          let (env, prop) = subfield label ~sub ~super env in
          (env, prop :: props))
    in
    (env, Field_desc.top, Prop.conjs props)
  | _ ->
    (* If the field occurs with requiredness [Req] in the super-row, the
       field isn't masked and it doesn't occur in the row to the left at [Req]
       then this spread variable MUST be at [Req]*)
    let is_req =
      (* At the UNKNOWN label the spread variable's contribution is opt-only: a required
         unknown is unrepresentable (uninhabited regardless of base), so we never force
         [Req] there to mask an ill-typed [sub_pre].  Instead the [Opt] branch keeps
         sub_pre's type in the merged union and checks it against super directly; if it
         doesn't fit, the query rejects — correct and complete, since the only witness
         would have been a [Req]-unknown row, which no longer exists. *)
      match label with
      | None -> false
      | Some _ ->
        List.exists live ~f:(fun (_, fd_super, fld_assignement) ->
            let fd_sub_pre = Row.proj sub_pre label fld_assignement in
            (* Force the spread variable to [Req] if either:
               - the field is [Req] in super and [Opt] (absent-supplying) in sub_pre, so
                 only the spread variable can supply the requiredness; or
               - sub_pre's TYPE does not fit super: then the spread variable must be [Req]
                 to MASK the offending sub_pre (a [Req] field overwrites everything to its
                 left).  Without this, the [Opt] branch keeps sub_pre's type in the merged
                 union and the (satisfiable) field is over-rejected.
               Mirrors the [must_req] disjunction of scratch [construct_upper]. *)
            (Field_desc.is_req fd_super && Field_desc.is_opt fd_sub_pre)
            || Prop.is_invalid
                 (snd
                    (subtype_base ~sub:fd_sub_pre.base ~super:fd_super.base env)))
    in
    (* Since the spread variable is live under some assignements we generate a
       fresh type variable and accumulate constraints *)
    let (ty_var, env) = Env.fresh_ty_var env in
    let base_sub = Base.flex ty_var in
    let (env, props) =
      List.fold_left
        info
        ~init:(env, [])
        ~f:(fun (env, props) (is_masked, super, fld_assignement) ->
          if is_masked then
            let sub = Row.proj sub_post label fld_assignement in
            let (env, prop) = subfield label ~sub ~super env in
            (env, prop :: props)
          else
            (* The merged sub field's type at this assignment is the union of its
               contributors — ?t and sub_post, plus sub_pre UNLESS ?ρ is [Req]
               (then it masks sub_pre). *)
            let sub_base =
              let fld_post = Row.proj sub_post label fld_assignement in
              let ty = Base.union base_sub fld_post.base in
              if is_req then
                ty
              else
                let fld_pre = Row.proj sub_pre label fld_assignement in
                Base.union fld_pre.base ty
            in
            let (env, prop) =
              subtype_base ~sub:sub_base ~super:super.base env
            in
            (env, prop :: props))
    in
    let field =
      if is_req then
        Field_desc.req (Base.flex ty_var)
      else
        Field_desc.opt (Base.flex ty_var)
    in
    (env, field, Prop.conjs props)

(* -- Spread variables in super-row only ------------------------------------ *)
and subrow_infer_super
    ~(sub : Row.t)
    ~(super : Row.t)
    (super_spread_vars : Ty_var.t list)
    (env : Env.t) : Env.t * Prop.t =
  let rec loop spread_vars ~acc ~failures env =
    match spread_vars with
    | [] when List.is_empty failures -> (env, Prop.conjs acc)
    | [] -> (env, Prop.invalids failures)
    | spread_var :: spread_vars ->
      (match Row.partition_at_var super spread_var with
      | None ->
        (* shouldn't happen: [spread_var] came from [super] *)
        loop spread_vars ~acc ~failures env
      | Some (pre, post) ->
        let pre = Row_help.solve_all_except pre spread_var env in
        let post = Row_help.solve_all_except post spread_var env in
        let super_pre = Row.(canon @@ splat pre)
        and super_post = Row.(canon @@ splat post) in
        let (env, prop) =
          subrow_infer_super_part
            spread_var
            ~super_pre
            ~super_post
            ~super
            ~sub
            env
        in
        let acc = prop :: acc
        and failures =
          if Prop.is_invalid prop then
            prop :: failures
          else
            failures
        in
        loop spread_vars ~acc ~failures env)
  in
  loop super_spread_vars ~acc:[] ~failures:[] env

and subrow_infer_super_part
    spread_var
    ~(super_pre : Row.t)
    ~(super_post : Row.t)
    ~(super : Row.t)
    ~(sub : Row.t)
    (env : Env.t) : Env.t * Prop.t =
  let labels = Row_help.subrow_labels env sub super in
  let (env, known, unknown, props) =
    List.fold_left
      labels
      ~init:(env, Label.Map.empty, Base.bot, [])
      ~f:(fun (env, known, unknown, props) label ->
        let (env, field, prop) =
          subrow_infer_super_part_at
            label
            ~super_pre
            ~super_post
            ~super
            ~sub
            env
        in
        (* The unknown is opt-only, so its contribution is just the field's base (the
           degenerate [live = []] case returns [Field_desc.bot], i.e. the ⊥ base). *)
        let (known, unknown) =
          match label with
          | Some lbl -> (Label.Map.add lbl field known, unknown)
          | None -> (known, field.Field_desc.base)
        in
        (env, known, unknown, prop :: props))
  in
  (* The single residual constraint: shape({ the per-label fields }) <: ?ρ. *)
  let bound = Row.Row_simple { known; unknown } in
  ( env,
    Prop.conjs
      (Prop.is_subtype_base
         ~sub:(Base.shape bound)
         ~super:(Base.flex spread_var)
      :: props) )

(** [subrow_infer_super_part] at a single [label]

    - If super_post requires the field at every assigment it overwrites the
      spread variable, which is then free here — its field is the empty field and
      we emit only [sub <: super_post].

    - Otherwise the spread variable contributes: its requiredness mirrors the sub
      row:
      — [Opt] if some live sub row is [Opt] (the merge must then stay [Opt]),
      - else [Req] (the bottom requiredness).

      Its type is a fresh ty var and we assert sub.ty <: the merged union
      (the fresh var and post, plus pre unless the spread var is Req).

      A sub field that is [Opt] while super pre is [Req] is unsatisfiable since
      the merge is forced [Req] and [Opt] </:_req [Req]. *)
and subrow_infer_super_part_at
    label
    ~(sub : Row.t)
    ~(super_pre : Row.t)
    ~(super_post : Row.t)
    ~(super : Row.t)
    (env : Env.t) : Env.t * Field_desc.t * Prop.t =
  (* 'live' type parameters at this label in topological order *)
  let ty_params_topo =
    let live =
      Ty_param.Set.of_list
        (Row.live_spread_at sub label @ Row.live_spread_at super label)
    in
    Env.Ty_param_env.(topo env.ty_param (closure env.ty_param live))
  in
  (* We need all assignments during inference *)
  let assignments = Row_help.corner_assignments ty_params_topo label env in
  (* per assignment:
     - is the spread variable masked by a [Req] in the row to its right, [super_post]?
     - the field descriptor in the sub-row (the counterpart [cf])
     - the field assignment *)
  let info =
    List.map assignments ~f:(fun fld_assignment ->
        let fd_post = Row.proj super_post label fld_assignment
        and fd_sub = Row.proj sub label fld_assignment in
        (Field_desc.is_req fd_post, fd_sub, fld_assignment))
  in
  (* Those assignments where the spread variable is not masked *)
  let live = List.filter info ~f:(fun (masked, _, _) -> not masked) in
  match live with
  | [] ->
    (* the spread variable is overwritten by post everywhere so the lower bound is the
       empty field; emit the ground checks cf <: post. *)
    let (env, props) =
      List.fold_left
        info
        ~init:(env, [])
        ~f:(fun (env, props) (_, sub, fld_assignment) ->
          let (env, prop) =
            subfield
              label
              ~sub
              ~super:(Row.proj super_post label fld_assignment)
              env
          in
          (env, prop :: props))
    in
    (env, Field_desc.bot, Prop.conjs props)
  | _ ->
    (* If fd_sub occurs with requiredness [Opt] at some live assignment, the
       merge must stay [Opt], so this spread variable is [Opt]; otherwise it is
       [Req] (requiredness botto). *)
    let is_opt =
      List.exists live ~f:(fun (_, fd_sub, _) -> Field_desc.is_opt fd_sub)
    in
    (* Since the spread variable is live under some assignements we generate a fresh
       type variable and accumulate constraints *)
    let (ty_var, env) = Env.fresh_ty_var env in
    let base_super = Base.flex ty_var in
    let (env, props) =
      List.fold_left
        info
        ~init:(env, [])
        ~f:(fun (env, props) (is_masked, fd_sub, fld_assignment) ->
          if is_masked then
            let (env, prop) =
              subfield
                label
                ~sub:fd_sub
                ~super:(Row.proj super_post label fld_assignment)
                env
            in
            (env, prop :: props)
          else if
            Field_desc.is_opt fd_sub
            && Field_desc.is_req (Row.proj super_pre label fld_assignment)
          then
            (* Opt </:_req Req *)
            let fail =
              Prop.(
                is_subfield
                  label
                  ~sub:fd_sub
                  ~super:(Row.proj super_pre label fld_assignment))
            in
            (env, Prop.invalid fail :: props)
          else
            (* fd_sub.ty must be a subtyped of the merged super field's type: the
               union of its contributors — ?t and super_post, plus super_pre
               UNLESS ?ρ is [Req] (then it masks super_pre). *)
            let super_base =
              let fld_post = Row.proj super_post label fld_assignment in
              let ty = Base.union base_super fld_post.base in
              if is_opt then
                let fld_pre = Row.proj super_pre label fld_assignment in
                Base.union fld_pre.base ty
              else
                ty
            in
            let (env, prop) =
              subtype_base ~sub:fd_sub.base ~super:super_base env
            in
            (env, prop :: props))
    in
    let field =
      if is_opt then
        Field_desc.opt (Base.flex ty_var)
      else
        Field_desc.req (Base.flex ty_var)
    in
    (env, field, Prop.conjs props)

(* -- Spread variables in both sub- and super-row --------------------------- *)

(** Exactly ONE spread variable on each side: couple them through a fresh intermediary
    row [mid] (a fresh field per label) instead of resolving either side's variable to
    the empty row.  [sub <: mid] bounds [?ρ_sub] from above and [mid <: super] bounds
    [?ρ_super] from below; [mid]'s fresh type variables link the two, so [?ρ_sub]'s
    contribution flows through to [?ρ_super].  Sound by transitivity ([sub <: mid <:
    super]).  [mid]'s KNOWN fields are fixed [Req] (measured far better than [Opt] —
    forcing the sub side [Req] over-rejects much less than forcing the super side [Opt]),
    so the keep-both requiredness gap remains; the point is that the cross-side coupling
    is no longer dropped.  [mid]'s unknown is opt-only (a fresh base) — a required unknown
    is unrepresentable. *)
and subrow_infer_couple
    ~(sub : Row.t)
    ~(sub_spread_var : Ty_var.t)
    ~(super : Row.t)
    ~(super_spread_var : Ty_var.t)
    (env : Env.t) : Env.t * Prop.t =
  let fresh_req_field env =
    let (ty_var, env) = Env.fresh_ty_var env in
    (env, Field_desc.req (Base.flex ty_var))
  in
  let (env, known) =
    Label.Set.fold
      (fun lbl (env, known) ->
        let (env, field) = fresh_req_field env in
        (env, Label.Map.add lbl field known))
      (Row_help.subrow_label_set env sub super)
      (env, Label.Map.empty)
  in
  let (ty_var, env) = Env.fresh_ty_var env in
  let mid = Row.Row_simple { known; unknown = Base.flex ty_var } in
  let (env, prop_sub) = subrow_infer_sub ~sub [sub_spread_var] ~super:mid env in
  let (env, prop_super) =
    subrow_infer_super ~sub:mid ~super [super_spread_var] env
  in
  (env, Prop.conj prop_sub prop_super)

and subrow_infer_sub_super
    ~(sub : Row.t)
    ~(sub_spread_vars : Ty_var.t list)
    ~(super : Row.t)
    ~(super_spread_vars : Ty_var.t list)
    (env : Env.t) : Env.t * Prop.t =
  match (sub_spread_vars, super_spread_vars) with
  | ([sub_spread_var], [super_spread_var]) ->
    subrow_infer_couple ~sub ~sub_spread_var ~super ~super_spread_var env
  | _ ->
    (* More than one spread variable on a side: a side's field is then a merge of two
       unknowns, which has no sound per-label coupling, so we fall back to the DECOUPLED
       solver — bound every variable on each side by resolving the OTHER side's variables
       to their current solutions (an unsolved one becomes the empty row), making it
       concrete, then solve this side against it.  Sound but the cross-side
       ?ρ_sub ↔ ?ρ_super coupling is dropped. *)
    let (env, prop_sub) =
      let super' = Row_help.solve_all super env in
      (* if the resolved super is a union of rows, decide [sub <: super'] at the base level
         (disjunctively) rather than projecting the union spread per-label *)
      if Row_help.has_union_spread super' then
        subtype_base
          ~sub:(Base.shape sub)
          ~super:(Base.canon (Base.shape super'))
          env
      else
        subrow_infer_sub ~sub sub_spread_vars ~super:super' env
    in
    let (env, prop_super) =
      let sub' = Row_help.solve_all sub env in
      if Row_help.has_union_spread sub' then
        subtype_base
          ~sub:(Base.canon (Base.shape sub'))
          ~super:(Base.shape super)
          env
      else
        subrow_infer_super ~sub:sub' ~super super_spread_vars env
    in
    (env, Prop.conj prop_sub prop_super)

(* == Simple rows =========================================================== *)

and subrow_simple ~(sub : Row.Simple.t) ~(super : Row.Simple.t) (env : Env.t) :
    Env.t * Prop.t =
  let rec loop labels ~acc ~failures env =
    match labels with
    | [] when List.is_empty failures -> (env, Prop.conjs acc)
    | [] -> (env, Prop.invalids failures)
    | label :: labels ->
      let (env, prop) =
        let sub = Row.Simple.proj sub label
        and super = Row.Simple.proj super label in
        subfield label ~sub ~super env
      in
      let failures =
        if Prop.is_invalid prop then
          prop :: failures
        else
          failures
      in
      let acc = prop :: acc in
      loop labels ~acc ~failures env
  in
  let labels =
    let labels_sub = Row.Simple.label_set sub
    and labels_super = Row.Simple.label_set super in
    let known = Label.Set.(elements @@ union labels_sub labels_super) in
    None :: List.map known ~f:Option.some
  in
  loop labels ~acc:[] ~failures:[] env

(* == Field descriptors ===================================================== *)

(* Since fields descriptors are a product of two lattices, we need both
   the sub field requiredness to be less-than-or-equal to the super field
   requiredness and the sub field type to be *)
and subfield label ~(sub : Field_desc.t) ~(super : Field_desc.t) (env : Env.t) :
    Env.t * Prop.t =
  (* Requiredness is a 2-point chain Req < Opt; a sub field must be at least as
     required as the super field, i.e. sub.req <= super.req.  If that holds, the
     field is a subfield exactly when its type is a subtype. *)
  if Field_req.lteq sub.req super.req then
    subtype_base ~sub:sub.base ~super:super.base env
  else
    let fail = Prop.(is_subfield label ~sub ~super) in
    (env, Prop.(invalid fail))

(* == Dispatch: record variable constraints + their transitive implications ===
   [subtype_base]/[subrow] turn two types into a residual [Prop.t] over type
   variables.  [dispatch] walks that proposition, records each variable constraint in
   [env], and for every NEW bound generates the implied transitive proposition
   (lower <: upper through the variable) which it dispatches in turn.  Generating
   implications only for newly-recorded bounds is what makes the recursion stop. *)

let rec tell_prop (prop : Prop.t) (env : Env.t) : Env.t * Prop.t =
  match prop with
  | Prop.Conj props -> tell_conj props ~acc:[] ~failures:[] env
  | Prop.Disj { props; failures } -> tell_disj props ~acc:[] ~failures env
  | Prop.Atom atom -> tell_atom atom env

and tell_disj props ~acc ~failures env =
  match props with
  | [] -> (env, Prop.Disj { failures; props = acc })
  | prop :: props ->
    let (env, prop) = tell_prop prop env in
    if Prop.is_valid prop then
      (env, prop)
    else if Prop.is_invalid prop then
      let failures = prop :: failures in
      tell_disj props ~acc ~failures env
    else
      let acc = prop :: acc in
      tell_disj props ~acc ~failures env

and tell_conj props ~acc ~failures env =
  match props with
  | [] when List.is_empty failures -> (env, Prop.conjs acc)
  | [] -> (env, Prop.invalids failures)
  | prop :: props ->
    let (env, prop) = tell_prop prop env in
    let failures =
      if Prop.is_invalid prop then
        prop :: failures
      else
        failures
    and acc = prop :: acc in
    tell_conj props ~acc ~failures env

and tell_atom (atom : Prop.atom) (env : Env.t) : Env.t * Prop.t =
  match atom with
  | Prop.Subtype_base { sub; super } -> tell_atom_base ~sub ~super env
  | Prop.Subfield { label; sub; super } ->
    (* Shouldn't happen *)
    subfield label ~sub ~super env

and tell_atom_base ~sub ~super env =
  match (sub, super) with
  | (Base.Flex var_sub, Base.Flex var_super) ->
    let (env, prop1) = record_ty_upper var_sub super env in
    let (env, prop2) = record_ty_lower sub var_super env in
    (env, Prop.conj prop1 prop2)
  | (Base.Flex var_sub, _) -> record_ty_upper var_sub super env
  | (_, Base.Flex var_super) -> record_ty_lower sub var_super env
  | _ -> subtype_base ~sub ~super env

and record_ty_upper var_sub super env =
  let super = Base.canon super in
  let (ty_var, is_new) = Env.Ty_var_env.add_upper env.ty_var var_sub super in
  let env = { env with ty_var } in
  if not is_new then
    (env, Prop.valid)
  else
    let subs = Env.Ty_var_env.lowers env.ty_var var_sub in
    let props =
      List.map subs ~f:(fun sub -> Prop.is_subtype_base ~sub ~super)
    in
    tell_conj props ~acc:[] ~failures:[] env

and record_ty_lower sub var_super env =
  let sub = Base.canon sub in
  let (ty_var, is_new) = Env.Ty_var_env.add_lower env.ty_var var_super sub in
  let env = { env with ty_var } in
  if not is_new then
    (env, Prop.valid)
  else
    let supers = Env.Ty_var_env.uppers env.ty_var var_super in
    let props =
      List.map supers ~f:(fun super -> Prop.is_subtype_base ~sub ~super)
    in
    tell_conj props ~acc:[] ~failures:[] env

(* ==Top-level entry points ================================================= *)
let tell_subtype ~(sub : Base.t) ~(super : Base.t) (env : Env.t) :
    Env.t * Prop.t =
  (* Canonicalise the inputs so [subtype_base]'s recursion (down to [Splat.proj], which
     assumes flat splats) only ever sees canonical types.  One canon at this boundary
     suffices: it descends into already-canonical sub-terms. *)
  let sub = Base.canon sub and super = Base.canon super in
  let (env, prop) = subtype_base ~sub ~super env in
  tell_prop prop env

let tell_subrow ~(sub : Row.t) ~(super : Row.t) (env : Env.t) : Env.t * Prop.t =
  let sub = Row.canon sub and super = Row.canon super in
  let (env, prop) = subrow ~sub ~super env in
  tell_prop prop env
