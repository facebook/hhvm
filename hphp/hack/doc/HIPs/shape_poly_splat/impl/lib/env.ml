open StdLabels
open Repr

module Ty_param_env = struct
  type bounds = {
    upper: Base.t;
    lower: Base.t;
  }

  type t = bounds Ty_param.Map.t

  let empty = Ty_param.Map.empty

  let find ty_param t = Ty_param.Map.find ty_param t

  (* Canonicalise bounds on the way in, so the F-sub rule and [field_bounds]/[row_of_base]
     (which read bounds straight from the env) only ever see canonical types — the ty-var
     side already canons in [record_ty_upper]/[record_ty_lower]. *)
  let add t ty_param ~upper ~lower =
    Ty_param.Map.add
      ty_param
      { upper = Base.canon upper; lower = Base.canon lower }
      t

  let remove t ty_param = Ty_param.Map.remove ty_param t

  (* The rigid type parameters a [base] mentions at SPREAD position — i.e. those a
     bound's projection would look up in the corner [fld_assignment].  A param nested
     in a field TYPE (e.g. the [#b] of [shape({x: #b})]) is carried symbolically by the
     corner field and resolved by [subtype_base]'s F-sub rule, so it is NOT a corner
     dependency.  (Well-formed param bounds are shapes / bottom; other shapes give ∅.) *)
  let type_params_in_base (b : Base.t) : Ty_param.Set.t =
    match b with
    | Base.Shape r -> Row.spread_param_set r
    | Base.Top
    | Base.Bottom
    | Base.Prim _
    | Base.Union _
    | Base.Flex _
    | Base.Rigid _ ->
      Ty_param.Set.empty

  let type_params_in_bounds t ty_param =
    let { upper; lower } = Ty_param.Map.find ty_param t in
    Ty_param.Set.elements
    @@ Ty_param.Set.union
         (type_params_in_base upper)
         (type_params_in_base lower)

  (* Transitive closure under "appears (at spread position) in a bound": seed the
     worklist with the input set and accumulate (the [mem] guard terminates on cycles).
     A param reachable ONLY through another's bound is thereby pulled in, so the corner
     machinery enumerates it before projecting the bound that mentions it. *)
  let closure t ty_params =
    let rec aux worklist acc =
      match worklist with
      | [] -> acc
      | next :: rest when Ty_param.Set.mem next acc -> aux rest acc
      | next :: rest ->
        let delta = type_params_in_bounds t next in
        aux (delta @ rest) (Ty_param.Set.add next acc)
    in
    aux (Ty_param.Set.elements ty_params) Ty_param.Set.empty

  let topo t ty_params =
    let deps ty_param =
      List.filter
        ~f:(fun p -> Ty_param.Set.mem p ty_params)
        (type_params_in_bounds t ty_param)
    in
    let rec aux ty_param order stack =
      if
        List.exists ~f:(Ty_param.equal ty_param) order
        || List.exists ~f:(Ty_param.equal ty_param) stack
      then
        order
      else
        let order =
          List.fold_left (deps ty_param) ~init:order ~f:(fun order dep ->
              aux dep order (ty_param :: stack))
        in
        if List.exists ~f:(Ty_param.equal ty_param) order then
          order
        else
          order @ [ty_param]
    in
    Ty_param.Set.fold (fun ty_param acc -> aux ty_param acc []) ty_params []
end

module Ty_var_env = struct
  type bounds = {
    upper: Base.t list;
    lower: Base.t list;
  }

  type t = bounds Ty_var.Map.t

  let empty = Ty_var.Map.empty

  let find t (ty_var : Ty_var.t) = Ty_var.Map.find ty_var t

  let find_opt t (ty_var : Ty_var.t) = Ty_var.Map.find_opt ty_var t

  let get t (ty_var : Ty_var.t) = find t ty_var

  let uppers t (ty_var : Ty_var.t) = (get t ty_var).upper

  let lowers t (ty_var : Ty_var.t) = (get t ty_var).lower

  let add_upper t ty_var bound =
    let b = get t ty_var in
    if List.mem bound ~set:b.upper then
      (t, false)
    else
      (Ty_var.Map.add ty_var { b with upper = bound :: b.upper } t, true)

  let add_lower t ty_var bound =
    let b = get t ty_var in
    if List.mem bound ~set:b.lower then
      (t, false)
    else
      (Ty_var.Map.add ty_var { b with lower = bound :: b.lower } t, true)
end

type t = {
  ty_param: Ty_param_env.t;
  ty_var: Ty_var_env.t;
  next_ty_var: int; (* counter for minting fresh type variables *)
}

let empty =
  { ty_param = Ty_param_env.empty; ty_var = Ty_var_env.empty; next_ty_var = 0 }

(* Mint a fresh variable: record it with empty bounds AND advance the counter, so every
   variable in play was created here and is therefore present for the strict accessors. *)
let fresh_ty_var (t : t) =
  let v = t.next_ty_var in
  ( v,
    {
      t with
      next_ty_var = v + 1;
      ty_var = Ty_var.Map.add v Ty_var_env.{ upper = []; lower = [] } t.ty_var;
    } )

(* == Solving: a variable's current solution + recursive substitution =========
   The CURRENT solution of a flex variable is read ONLY from its bounds recorded in
   [env] — i.e. constraints that have already been DISCHARGED into the environment.
   We never read undischarged [Prop] atoms: those aren't known valid until their
   transitively-implied propositions are checked.  A variable with no recorded bound
   therefore reads as unsolved — a type var is left at [Bottom], and a SPREAD'd type
   var reads as the empty row (the merge identity, contributing nothing) — which is
   exactly the safe fallback.

   Bounds are resolved before being combined, so [Base.meet]/[join] only ever see
   ground types.  N.B. termination relies on the bound graph being acyclic
   (topologically eliminated before solving). *)
let rec solve_ty_var (env : t) (var : Ty_var.t) : Base.t =
  let Ty_var_env.{ upper; lower } = Ty_var_env.get env.ty_var var in
  match (lower, upper) with
  | (_ :: _, _) ->
    List.fold_left lower ~init:Base.bot ~f:(fun acc b ->
        Base.join acc (resolve_base env b))
  | ([], _ :: _) ->
    List.fold_left upper ~init:Base.top ~f:(fun acc b ->
        Base.meet acc (resolve_base env b))
  | ([], []) -> Base.bot

(* Recursively substitute every flex variable by its current solution.  [except]
   leaves one spread variable untouched — the one currently being solved. *)
and resolve_row ?except (env : t) (r : Row.t) : Row.t =
  match r with
  | Row.Row_simple s ->
    Row.canon (Row.Row_simple (resolve_simple ?except env s))
  | Row.Row_splat { elems } ->
    Row.canon
      (Row.splat
         (List.concat_map elems ~f:(fun e -> resolve_elem ?except env e)))

and resolve_elem ?except (env : t) (e : Row.Splat_elem.t) :
    Row.Splat_elem.t list =
  match e with
  | Row.Splat_elem.Spread (Base.Flex v) ->
    (match except with
    | Some keep when Ty_var.equal v keep -> [e]
    | _ -> resolve_spread_var env v)
  | Row.Splat_elem.Spread b ->
    [Row.Splat_elem.Spread (resolve_base ?except env b)]

and resolve_spread_var (env : t) (v : Ty_var.t) : Row.Splat_elem.t list =
  (* A spread'd flex var resolves to its current solution.  Combine its bound bases with
     [Base.join] of the lowers (covariant) / [Base.meet] of the uppers (contravariant) —
     the PRECISE principal bound.  For incomparable lower shapes that is a UNION of shapes;
     the union floats OUT of the splat at canon time ([canon_base] distributes a
     [Spread (Union …)] into a top-level union), so here we just emit one spread of the
     combined base.  An unbounded var is the EMPTY row (the merge identity, contributing
     nothing, NOT bottom). *)
  let Ty_var_env.{ upper; lower } = Ty_var_env.get env.ty_var v in
  match (lower, upper) with
  | ([], []) -> [Row.Splat_elem.simple Row.Simple.empty]
  | (_ :: _, _) ->
    [
      Row.Splat_elem.spread
        (List.fold_left lower ~init:Base.bot ~f:(fun acc b ->
             Base.join acc (resolve_base env b)));
    ]
  | ([], _ :: _) ->
    [
      Row.Splat_elem.spread
        (List.fold_left upper ~init:Base.top ~f:(fun acc b ->
             Base.meet acc (resolve_base env b)));
    ]

and resolve_simple ?except (env : t) (s : Row.Simple.t) : Row.Simple.t =
  Row.Simple.
    {
      known = Label.Map.map (fun fd -> resolve_field ?except env fd) s.known;
      unknown = resolve_base ?except env s.unknown;
    }

and resolve_field ?except (env : t) (fd : Field_desc.t) : Field_desc.t =
  { fd with base = resolve_base ?except env fd.base }

and resolve_base ?except (env : t) (b : Base.t) : Base.t =
  match b with
  | Base.Top
  | Base.Bottom
  | Base.Prim _
  | Base.Rigid _ ->
    b
  | Base.Union (a, c) ->
    Base.union (resolve_base ?except env a) (resolve_base ?except env c)
  (* [Base.canon] (not [Base.shape]) so a resolved splat carrying a union spread DISTRIBUTES
     out to a top-level union here; on a union-free shape it is idempotent. *)
  | Base.Shape r -> Base.canon (Base.Shape (resolve_row ?except env r))
  | Base.Flex v -> solve_ty_var env v
