(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Common
open Typing_defs
open Typing_env_types
module Env = Typing_env
module Inf = Typing_inference_env
module ITySet = Internal_type_set
module TL = Typing_logic
module TySet = Typing_set
module Utils = Typing_solver_utils

let raise_warning message =
  let stack = Caml.Printexc.get_callstack 50 in
  Printf.eprintf "%s\n" message;
  Caml.Printexc.print_raw_backtrace stderr stack;
  Printf.eprintf "%!"

(** A module to implement some equivalence classes.
    These are some examples of equivalence classes:
    - connected components
    - cycles in a graph
    *)
module EquivalenceClasses : sig
  (** A representative of an equivalence class, i.e. a member
      of that class serving as an identifier for it. *)
  module Representative : sig
    [@@@warning "-32"]

    type t [@@deriving show, eq, ord]

    [@@@warning "+32"]

    val id : t -> Ident.t
  end

  module Rep = Representative

  module RepresentativeMap : sig
    include WrappedMap.S with type key = Rep.t

    [@@@warning "-32"]

    val show : ('a -> string) -> 'a t -> string

    val pp :
      (Format.formatter -> 'a -> unit) -> Format.formatter -> 'a t -> unit

    [@@@warning "+32"]

    val find_opt : Ident.t -> 'a t -> 'a option

    val remove : Ident.t -> 'a t -> 'a t
  end

  module RepMap = RepresentativeMap

  module RepresentativeSet : sig
    include Caml.Set.S with type elt = Rep.t

    [@@@warning "-32"]

    val show : t -> string

    val pp : Format.formatter -> t -> unit

    [@@@warning "+32"]
  end

  [@@@warning "-32"]

  type t [@@deriving show]

  val equal : t -> t -> bool

  [@@@warning "+32"]

  val init : Ident.t list -> t

  val merge_classes : Ident.t -> Ident.t -> t -> t

  val get_rep : Ident.t -> t -> t * Rep.t

  val of_lists : Ident.t list list -> t

  val fold : t -> init:'a -> f:('a -> Ident.t -> Rep.t -> 'a) -> t * 'a

  val are_same_class : t -> Ident.t -> Ident.t -> t * bool
end = struct
  module Representative = struct
    type t = Ident.t [@@deriving show, eq, ord]

    let id rep = rep
  end

  module Rep = Representative

  module RepresentativeMap = struct
    include WrappedMap.Make (Rep)

    let show (show_value : 'a -> string) (m : 'a t) =
      let s = "{ " in
      let s =
        fold
          (fun rep value s ->
            Printf.sprintf "%s%s -> %s; " s (Rep.show rep) (show_value value))
          m
          s
      in
      let s = s ^ "}" in
      s

    let pp
        (pp_value : Format.formatter -> 'a -> unit)
        (fmt : Format.formatter)
        (m : 'a t) : unit =
      Format.fprintf fmt "@[{ @[";
      iter
        (fun rep value ->
          Format.fprintf fmt "%s -> " (Rep.show rep);
          pp_value fmt value;
          Format.fprintf fmt ";@ ")
        m;
      Format.fprintf fmt "@]@ }@]";
      ()
  end

  module RepMap = RepresentativeMap

  module RepresentativeSet = struct
    include Caml.Set.Make (Rep)

    let show (x : t) =
      let s = "{ " in
      let s =
        fold (fun rep s -> Printf.sprintf "%s%s; " s (Rep.show rep)) x s
      in
      let s = s ^ "}" in
      s

    let pp (fmt : Format.formatter) (x : t) =
      Format.fprintf fmt "@[{@ ";
      iter (fun rep -> Format.fprintf fmt "%s;@ " (Rep.show rep)) x;
      Format.fprintf fmt "@ }@]";
      ()
  end

  (** This map allows to retrieve for each class member the representative of its class.
      This is done recursively as the mapping from members to their
      representative may not be direct. *)
  type t = Ident.t IMap.t [@@deriving show]

  let init ids = IMap.of_function ids (fun v -> v)

  let set_rep (v : Ident.t) (rep : Rep.t) (m : t) : t = IMap.add v rep m

  let get_rep (v : Ident.t) (m : t) : t * Rep.t =
    let rec get v m ~seen =
      (* TODO: this find_opt should not be necessary since we initialize the
       * map by mapping each node to itself, but it turns out it is,
       * likely due to inconsistencies in the environment
       * (e.g. a type var appearing in some bound without actual entry for it in tvenv).
       * TODO: Figure out what's happening. *)
      let v' =
        match IMap.find_opt v m with
        | Some v' -> v'
        | None ->
          raise_warning
            (Printf.sprintf
               "Trying to get representative for #%d which is missing. Returning #%d itself."
               v
               v);
          v
      in
      if Ident.equal v' v then
        (* path shortening *)
        let m = ISet.fold (fun v'' m -> set_rep v'' v m) seen m in
        (m, v')
      else
        get v' m ~seen:(ISet.add v seen)
    in
    get v m ~seen:ISet.empty

  let merge_classes v1 v2 m =
    let (m, rep1) = get_rep v1 m in
    let (m, rep2) = get_rep v2 m in
    set_rep rep1 rep2 m

  (* Test equality of the mathematical object represented by the two given maps,
     i.e. modulo path compression and choice of the representative. *)
  let equal m1 m2 =
    let vars1 = IMap.keys m1 |> ISet.of_list in
    let vars2 = IMap.keys m2 |> ISet.of_list in
    if not @@ ISet.equal vars1 vars2 then
      false
    else
      let all_vars = vars1 in
      (* Check for each v that m2[m1[v]] = m2[v] and m1[m2[v]] = m1[v] *)
      ISet.for_all
        (fun v ->
          let (_, rep1) = get_rep v m1 in
          let (_, rep2) = get_rep v m2 in
          Ident.equal (snd @@ get_rep rep1 m2) rep2
          && Ident.equal (snd @@ get_rep rep2 m1) rep1)
        all_vars

  let of_lists (classes : Ident.t list list) : t =
    List.fold classes ~init:IMap.empty ~f:(fun m vars ->
        match vars with
        | [] -> m
        | v :: vars ->
          let m = set_rep v v m in
          List.fold vars ~init:m ~f:(fun m v' -> set_rep v' v m))

  let fold (m : t) ~init:acc ~f =
    List.fold (IMap.keys m) ~init:(m, acc) ~f:(fun (m, acc) v ->
        let (m, rep) = get_rep v m in
        let acc = f acc v rep in
        (m, acc))

  let are_same_class (m : t) v1 v2 =
    let (m, rep1) = get_rep v1 m in
    let (m, rep2) = get_rep v2 m in
    (m, Ident.equal rep1 rep2)
end

module Equiv = EquivalenceClasses

(** A module to maintain connected components (CC) and their dependencies.
    A CC A depends on (or is blocked by) another CC B if solving B will add
    edges in A. Therefore A can't be solved until B has been solved.
    For example, consider the three following CCs:

    CC A: C<#1> <: #2 <: C<#3>
    CC B: #1 (on its own)
    CC C: #3 (on its own)

    Both B and C are blocked by A, because solving A will solve #2 to C<#1> then
    call C<#1> <: C<#3> which will add some edges between #1 and #3.

    In other words, CC1 is blocking CC2 if there exist #1 in CC1 and #2 in CC2
    such that either #1 <: sometype<#2> or sometype<#2> <: #1, where sometype<#2>
    is a type that depends on #2 but is not #2 itself. *)
module ConnectedComponents : sig
  module Rep : sig
    [@@@warning "-32"]

    type t [@@deriving ord, show]

    [@@@warning "+32"]
  end

  module RepresentativeSet : sig
    include Caml.Set.S with type elt = Rep.t

    [@@@warning "-32"]

    val show : t -> string

    val pp : Format.formatter -> t -> unit

    [@@@warning "+32"]
  end

  module RepSet = RepresentativeSet

  [@@@warning "-32"]

  type t [@@deriving show]

  val equal : t -> t -> bool

  val from_ccs_as_lists : Ident.t list list -> t

  [@@@warning "+32"]

  val init : Ident.t list -> t

  val update : t -> Equiv.t -> t

  val add_deps : t -> (Ident.t * Ident.t) list -> t

  val get_vars : t -> Rep.t -> ISet.t

  val make_non_blocking : t * RepSet.t -> Rep.t -> t * RepSet.t

  val get_all_unblocked_ccs : t -> RepSet.t

  val to_equiv : t -> Equiv.t

  val print_stats : t -> unit
end = struct
  module Rep = Equiv.Rep
  module RepresentativeSet = Equiv.RepresentativeSet
  module RepSet = RepresentativeSet
  module RepMap = Equiv.RepMap

  [@@@warning "-32"]

  type node = {
    vars: ISet.t;
    is_blocked_by_ccs: ISet.t;  (** Other CCs which are blocking this CC. *)
    is_blocking_ccs: ISet.t;  (** Other CCs which are blocked by this CC. *)
  }
  [@@deriving show]

  type graph = node RepMap.t [@@deriving show]

  type t = {
    cc_cls: Equiv.t;
    cc_deps: graph;
  }
  [@@deriving show]

  [@@@warning "+32"]

  let get_rep (cc : t) (v : Ident.t) : t * Rep.t =
    let (cc_cls, rep) = Equiv.get_rep v cc.cc_cls in
    ({ cc with cc_cls }, rep)

  let set_node (cc : t) (v : Rep.t) (node : node) : t =
    { cc with cc_deps = RepMap.add v node cc.cc_deps }

  let init_node rep =
    {
      vars = ISet.singleton @@ Rep.id rep;
      is_blocked_by_ccs = ISet.empty;
      is_blocking_ccs = ISet.empty;
    }

  let lookup_node_ (deps : graph) (v : Rep.t) : node =
    (* TODO: this find_opt should not be necessary since we initialize the
     * dependency map with default nodes, but it turns out it is,
     * likely due to inconsistencies in the environment.
     * TODO: Figure out what's happening. *)
    match RepMap.find_opt (Rep.id v) deps with
    | Some n -> n
    | None ->
      raise_warning
        (Printf.sprintf
           "Trying to get CC node for #%d which is missing. Returning default node itself."
           (Rep.id v));
      init_node v

  let lookup_node (cc : t) (rep : Rep.t) : node = lookup_node_ cc.cc_deps rep

  (** Get the set of representatives of a set of members. *)
  let rep_set_of cc (deps : ISet.t) : t * RepSet.t =
    let (cc, deps) =
      deps |> ISet.elements |> List.fold_map ~init:cc ~f:get_rep
    in
    let deps = RepSet.of_list deps in
    (cc, deps)

  (** Get the ids of a set of representatives. *)
  let ident_set_of (reps : RepSet.t) : ISet.t =
    reps |> RepSet.elements |> List.map ~f:Rep.id |> ISet.of_list

  (** Get the variables in a connected component, identified by
      its representative. *)
  let get_vars (cc : t) (rep : Rep.t) : ISet.t =
    let node = lookup_node cc rep in
    node.vars

  (** Get the CCs blocking a CC (identified by its representative) *)
  let get_blockers (cc : t) (rep : Rep.t) : t * RepSet.t =
    let c = lookup_node cc rep in
    (* /!\ the "blockers" of v are those v "is blocked by" *)
    let reps = c.is_blocked_by_ccs in
    let (cc, reps) = rep_set_of cc reps in
    let c = { c with is_blocked_by_ccs = ident_set_of reps } in
    let cc = set_node cc rep c in
    (cc, reps)

  (** Get the CCs blocked a CC (identified by its representative) *)
  let get_blocked (cc : t) (rep : Rep.t) : t * RepSet.t =
    let c = lookup_node cc rep in
    (* /!\ the "blocked" CCs of v are those v "is blocking" *)
    let reps = c.is_blocking_ccs in
    let (cc, reps) = rep_set_of cc reps in
    let c = { c with is_blocking_ccs = ident_set_of reps } in
    let cc = set_node cc rep c in
    (cc, reps)

  (** Initialize a CC graph, where each member is unique in its own independent CC. *)
  let init (vars : Ident.t list) =
    let cc = Equiv.init vars in
    let (cc_cls, reps) =
      List.fold_map vars ~init:cc ~f:(fun cc v -> Equiv.get_rep v cc)
    in
    let cc_deps = RepMap.of_function reps init_node in
    { cc_cls; cc_deps }

  (** Merge two nodes. *)
  let union_ccs (n1 : node) (n2 : node) : node =
    let {
      vars = vars1;
      is_blocked_by_ccs = is_blocked_by1;
      is_blocking_ccs = is_blocking1;
    } =
      n1
    in
    let {
      vars = vars2;
      is_blocked_by_ccs = is_blocked_by2;
      is_blocking_ccs = is_blocking2;
    } =
      n2
    in
    {
      vars = ISet.union vars1 vars2;
      is_blocked_by_ccs = ISet.union is_blocked_by1 is_blocked_by2;
      is_blocking_ccs = ISet.union is_blocking1 is_blocking2;
    }

  (** Update a CC graph using updated equivalence classes.
      The [cc_cls] argument is only valid if it has previously
      been retrieved using [to_equiv cc], then possibly updated (via
      calls to [Equiv.merge_classes]). This garantees that for all
      x and y which are in the same CC in [cc], x and y are in the same CC
      in [cc_cls].
      This is more performant than multiple calls to a function that
      would merge two given CCs, because we reduce the overall number
      of set unions performed. *)
  let update (cc : t) (cc_cls : Equiv.t) : t =
    let { cc_cls = _; cc_deps } = cc in
    let (cc_cls, cc_deps) =
      Equiv.fold cc_cls ~init:cc_deps ~f:(fun deps v rep ->
          if Ident.equal v (Rep.id rep) then
            deps
          else
            match RepMap.find_opt v deps with
            | None -> deps
            | Some n ->
              let n = union_ccs n (lookup_node_ deps rep) in
              let deps = RepMap.add rep n deps in
              let deps = RepMap.remove v deps in
              deps)
    in
    { cc_cls; cc_deps }

  let add_dep cc ~blocker:v ~blocked:v' =
    let (cc, rep) = get_rep cc v in
    let (cc, rep') = get_rep cc v' in
    let c = lookup_node cc rep in
    (* add rep' to cc.cc_deps[v].is_blocking_ccs *)
    let c =
      { c with is_blocking_ccs = ISet.add (Rep.id rep') c.is_blocking_ccs }
    in
    let cc = set_node cc rep c in
    (* add rep to cc.cc_deps[v'].is_blocked_by_ccs *)
    let c' = lookup_node cc rep' in
    let c' =
      { c' with is_blocked_by_ccs = ISet.add (Rep.id rep) c'.is_blocked_by_ccs }
    in
    let cc = set_node cc rep' c' in
    cc

  let add_deps (cc : t) (deps : (Ident.t * Ident.t) list) : t =
    List.fold deps ~init:cc ~f:(fun cc (blocker, blocked) ->
        add_dep cc ~blocker ~blocked)

  let is_blocked (cc : t) (rep : Rep.t) : bool =
    let (_cc, blockers) = get_blockers cc rep in
    not @@ RepSet.is_empty blockers

  let remove_blocker (cc : t) ~(blocker : Rep.t) ~(blocked : Rep.t) : t =
    let rep = blocked in
    let c = lookup_node cc rep in
    (* /!\ the "blockers" of v are those v "is blocked by" *)
    let reps = c.is_blocked_by_ccs in
    let (cc, reps) = rep_set_of cc reps in
    let reps = RepSet.remove blocker reps in
    let c = { c with is_blocked_by_ccs = ident_set_of reps } in
    let cc = set_node cc rep c in
    cc

  (** Once a CC has been solved, the CCs it was blocking have received additional edges
      and are not blocked anymore by this CC. The said CC can be removed as a blocker
      via this function.
      This function alse returns a set or newly unblocked CCs which will be
      ready to solve next. *)
  let make_non_blocking ((cc, unblocked) : t * RepSet.t) (rep : Rep.t) :
      t * RepSet.t =
    let (cc, blocked) = get_blocked cc rep in
    let (cc, unblocked) =
      RepSet.fold
        (fun rep' (cc, unblocked) ->
          let cc = remove_blocker cc ~blocker:rep ~blocked:rep' in
          let unblocked =
            if not @@ is_blocked cc rep' then
              RepSet.add rep' unblocked
            else
              unblocked
          in
          (cc, unblocked))
        blocked
        (cc, unblocked)
    in
    let cc =
      set_node cc rep { (lookup_node cc rep) with is_blocking_ccs = ISet.empty }
    in
    (cc, unblocked)

  let get_all_unblocked_ccs (cc : t) : RepSet.t =
    RepMap.fold
      (fun rep node unblocked ->
        if ISet.is_empty node.is_blocked_by_ccs then
          RepSet.add rep unblocked
        else
          unblocked)
      cc.cc_deps
      RepSet.empty

  let equal cc1 cc2 =
    let get_rep (v : Ident.t) ~(m : Equiv.t) : Rep.t =
      snd @@ Equiv.get_rep v m
    in
    let rep_set_equal (s1 : ISet.t) (s2 : ISet.t) (m : Equiv.t) : bool =
      let rep_set_of (s : ISet.t) (m : Equiv.t) : RepSet.t =
        s |> ISet.elements |> List.map ~f:(get_rep ~m) |> RepSet.of_list
      in
      RepSet.equal (rep_set_of s1 m) (rep_set_of s2 m)
    in
    let node_equal (n1 : node) (n2 : node) (m : Equiv.t) : bool =
      let { vars = v1; is_blocked_by_ccs = bed1; is_blocking_ccs = bing1 } =
        n1
      in
      let { vars = v2; is_blocked_by_ccs = bed2; is_blocking_ccs = bing2 } =
        n2
      in
      ISet.equal v1 v2
      && rep_set_equal bed1 bed2 m
      && rep_set_equal bing1 bing2 m
    in
    let node_exists_in_graph
        (m2 : Equiv.t) (deps2 : graph) (rep1 : Rep.t) (n1 : node) =
      let rep2 = get_rep (Rep.id rep1) ~m:m2 in
      match RepMap.find_opt (Rep.id rep2) deps2 with
      | None -> false
      | Some n2 -> node_equal n1 n2 m2
    in

    let { cc_cls = m1; cc_deps = deps1 } = cc1 in
    let { cc_cls = m2; cc_deps = deps2 } = cc2 in
    Equiv.equal m1 m2
    && RepMap.for_all (node_exists_in_graph m2 deps2) deps1
    && RepMap.for_all (node_exists_in_graph m1 deps1) deps2

  let from_ccs_as_lists (ccs : Ident.t list list) : t =
    let cc_cls = Equiv.of_lists ccs in
    let cc_deps =
      List.fold ccs ~init:RepMap.empty ~f:(fun deps vars ->
          match vars with
          | [] -> deps
          | v :: _ ->
            let n =
              {
                vars = ISet.of_list vars;
                is_blocked_by_ccs = ISet.empty;
                is_blocking_ccs = ISet.empty;
              }
            in
            let (_, rep) = Equiv.get_rep v cc_cls in
            RepMap.add rep n deps)
    in
    { cc_cls; cc_deps }

  let to_equiv cc = cc.cc_cls

  let print_stats cc =
    let n_ccs = RepMap.cardinal cc.cc_deps in
    Printf.printf "There are %d connected components.\n" n_ccs;
    let (max_cc_size, min_cc_size) =
      RepMap.fold
        (fun _rep n (max, min) ->
          let size = ISet.cardinal n.vars in
          let max =
            if size > max then
              size
            else
              max
          in
          let min =
            if size < min then
              size
            else
              min
          in
          (max, min))
        cc.cc_deps
        (0, Int.max_value)
    in
    Printf.printf "The largest has %d variables.\n" max_cc_size;
    Printf.printf "The smallest has %d variables.\n" min_cc_size;
    Printf.printf "%!"
end

module CC = ConnectedComponents
module Rep = CC.Rep
module RepSet = CC.RepresentativeSet

type visited_vars = ISet.t

(** Initial merging of subgraphs, so that all information we have on a
    variable is gathered in a single entry in the environment. Does not do
    anything clever like maintaining a transitive closure. *)
let merge_graphs (env : env) (graphs : Inf.t_global_with_pos list) : env =
  List.fold graphs ~init:env ~f:(fun env (_p, graph) ->
      List.fold (Inf.get_vars_g graph) ~init:env ~f:(fun env v ->
          {
            env with
            inference_env =
              Inf.move_tyvar_from_genv_to_env
                v
                ~from:graph
                ~to_:env.inference_env;
          }))

(** Checks whether a type has the from ?#n with #n a type variable.
    If yes, return that type variable id. *)
let is_nullable_var env ty =
  match ty with
  | ConstraintType _ -> (env, None)
  | LoclType ty ->
    let (env, ty) = Env.expand_type env ty in
    (match get_node ty with
    | Toption ty ->
      let (env, ty) = Env.expand_type env ty in
      (match get_node ty with
      | Tvar v -> (env, Some v)
      | _ -> (env, None))
    | _ -> (env, None))

(** Checks whether a type is either a type variable or a nullable variable.
    If yes, return that type variable id. *)
let is_var_or_nullable_var env ty =
  match get_var_i ty with
  | Some v -> (env, Some v)
  | None -> is_nullable_var env ty

(** Checks whether a type has the from (nonnull & #n) with #n a type variable.
    If yes, return that type variable id. *)
let is_nonnull_and_var env ty =
  match ty with
  | ConstraintType _ -> (env, None)
  | LoclType ty ->
    let (env, ty) = Env.expand_type env ty in
    (match get_node ty with
    | Tintersection [ty1; ty2] ->
      let (env, ty1) = Env.expand_type env ty1 in
      let (env, ty2) = Env.expand_type env ty2 in
      (match (get_node ty1, get_node ty2) with
      | (Tnonnull, Tvar v)
      | (Tvar v, Tnonnull) ->
        (env, Some v)
      | _ -> (env, None))
    | _ -> (env, None))

(** Checks whether a type is either a type variable or nonnull and a variable.
    If yes, return that type variable id. *)
let is_var_or_nonnull_and_var env ty =
  match get_var_i ty with
  | Some v -> (env, Some v)
  | None -> is_nonnull_and_var env ty

(** Union a type variable with null. *)
let null_or_var env v =
  (* TODO: figure out reasons *)
  let r = Reason.none in
  let tvar = mk (r, Tvar v) in
  let tnull = Typing_make_type.null r in
  let (env, ty) = Typing_union.union env tnull tvar in
  (env, LoclType ty)

(** Intersect a type variable with nonnull. *)
let nonnull_and_var env v =
  let r = Reason.none in
  let tvar = mk (r, Tvar v) in
  let tnonnull = Typing_make_type.nonnull r in
  let (env, ty) = Typing_intersection.intersect env ~r tnonnull tvar in
  (env, LoclType ty)

(** Traverse a type a get unsolved type variables appearing in it. *)
let get_unsolved_vars_in_type env ty =
  let gatherer =
    object (this)
      inherit [env * ISet.t] Type_visitor.internal_type_visitor

      method! on_tvar (env, vars) r v =
        let (env, ty) = Env.expand_var env r v in
        match get_node ty with
        | Tvar v' ->
          let vars = ISet.add v' vars in
          (env, vars)
        | _ -> this#on_locl_type (env, vars) ty
    end
  in
  gatherer#on_internal_type (env, ISet.empty) ty

type cc_dep_list = (Ident.t * Ident.t) list

type prop_to_env_acc = env * Equiv.t * cc_dep_list

(** Process a subtyping proposition and:
    - add bounds to given environment
    - update connected components
    - update a list of CC dependencies *)
let rec prop_to_env_cc (acc : prop_to_env_acc) (prop : TL.subtype_prop) :
    prop_to_env_acc =
  match prop with
  | TL.IsSubtype (None, ty_sub, ty_super) ->
    is_sub_prop_to_env_cc acc ty_sub ty_super
  | TL.IsSubtype (_dd, _ty_sub, _ty_super) ->
    (* For global inference, for now, we don't do coercion. *)
    assert false
  | TL.Conj props -> conj_prop_to_env_cc acc props
  | TL.Disj (err_f, props) -> disj_prop_to_env_cc acc err_f props

and conj_prop_to_env_cc acc props = List.fold props ~init:acc ~f:prop_to_env_cc

and disj_prop_to_env_cc acc fail props =
  match props with
  | [] ->
    (* Add error as a side effect *)
    Option.iter ~f:Typing_error_utils.add_typing_error fail;
    acc
  | prop :: _props ->
    (* Stupidly take the first proposition for now. *)
    prop_to_env_cc acc prop

and is_sub_prop_to_env_cc (env, cc, deps) ty_sub ty_super =
  match (get_var_i ty_sub, get_var_i ty_super) with
  | (Some v_sub, Some v_super) ->
    (* Make type vars bounds of each other and merge their respective CC. *)
    let cc = Equiv.merge_classes v_sub v_super cc in
    let env = Env.add_tyvar_upper_bound env v_sub ty_super in
    let env = Env.add_tyvar_lower_bound env v_super ty_sub in
    (env, cc, deps)
  | (Some v_sub, None) ->
    let (env, ty_super) =
      (* If v appears under unions or intersection in the upper
       * bound, it can be simplified away. *)
      Utils.remove_tyvar_from_upper_bound env v_sub ty_super
    in
    let (env, ty_super_is_nullable_var) = is_nullable_var env ty_super in
    (match ty_super_is_nullable_var with
    | Some v_super ->
      (* If the supertype is of the form ?#v_super, then we can also add
       * lower bound (nonnull & #v_sub) to #v_super and make them belong to
       * the same connected component. *)
      let cc = Equiv.merge_classes v_sub v_super cc in
      let env = Env.add_tyvar_upper_bound env v_sub ty_super in
      let (env, ty_sub) = nonnull_and_var env v_sub in
      let env = Env.add_tyvar_lower_bound env v_super ty_sub in
      (env, cc, deps)
    | None ->
      (* Otherwise, we look for unsolved variables appearing inside
       * ty_super. The CCs of those variables will be blocked by the
       * CC of v_sub. *)
      let env = Env.add_tyvar_upper_bound env v_sub ty_super in
      let (env, vars) = get_unsolved_vars_in_type env ty_super in
      let deps =
        ISet.fold (fun v_super deps -> (v_sub, v_super) :: deps) vars deps
      in
      (env, cc, deps))
  | (None, Some v_super) ->
    (* What happens here is the dual of the previous match case. *)
    let (env, ty_sub) =
      Utils.remove_tyvar_from_lower_bound env v_super ty_sub
    in
    let (env, ty_sub_is_nonnull_and_var) = is_nonnull_and_var env ty_sub in
    (match ty_sub_is_nonnull_and_var with
    | Some v_sub ->
      let cc = Equiv.merge_classes v_sub v_super cc in
      let env = Env.add_tyvar_lower_bound env v_super ty_sub in
      let (env, ty_super) = null_or_var env v_super in
      let env = Env.add_tyvar_upper_bound env v_sub ty_super in
      (env, cc, deps)
    | None ->
      let env = Env.add_tyvar_lower_bound env v_super ty_sub in
      let (env, vars) = get_unsolved_vars_in_type env ty_sub in
      let deps =
        ISet.fold (fun v_sub deps -> (v_super, v_sub) :: deps) vars deps
      in
      (env, cc, deps))
  | (None, None) -> assert false

let prop_to_env_cc ((env, cc, deps) : prop_to_env_acc) (prop : TL.subtype_prop)
    : prop_to_env_acc =
  let env_before = env in
  let (env, cc, deps) = prop_to_env_cc (env, cc, deps) prop in
  let env = Env.log_env_change "prop_to_env_cc" env_before env in
  (env, cc, deps)

let subtype env ty_sub ty_super ~on_error =
  Typing_subtype.simplify_subtype_i env ty_sub ty_super ~on_error

module Debug = struct
  let get_log_level env = Env.get_log_level env "solver"

  let print_build_cc_start env v =
    if get_log_level env >= 3 then
      Format.printf "Computing CCs: starting processing variable #%d...@." v

  let print_build_cc_end ((env, cc, deps) : env * Equiv.t * cc_dep_list) v =
    if get_log_level env >= 3 then (
      Format.printf
        "@[<v 0>Computing CCs: finished processing variable #%d.@ "
        v;
      Format.printf "@[<hov 2>CCs:@ ";
      Equiv.pp Format.std_formatter cc;
      Format.printf "@]@ @[<hov 2>Deps:@ ";
      Format.printf
        "%s\n"
        (List.to_string deps ~f:(fun (id1, id2) ->
             Format.sprintf "(%d, %d)" id1 id2));
      Format.printf "@]@]@.";
      ()
    )

  let print_build_cc_result (env, cc) =
    if get_log_level env >= 1 then (
      CC.print_stats cc;
      if get_log_level env >= 2 then (
        Format.print_string "@[Finished building CCs.@]@.";
        Typing_log.hh_show_env Pos.none env;
        CC.pp Format.std_formatter cc;
        Format.print_newline ();
        ()
      )
    )

  let print_collapse_cycle_equiv env rep equiv_map =
    if get_log_level env >= 3 then (
      Format.printf "@[<v 0>Collapsing cycles for CC of #%s@ " (Rep.show rep);
      Format.printf "@[<hov 2>Equivalence classes:@ ";
      Equiv.pp Format.std_formatter equiv_map;
      Format.printf "@]@.";
      ()
    )

  let print_collapse_cycle_env env =
    if get_log_level env >= 3 then (
      Format.printf "@[Env:@ @]@.";
      Typing_log.hh_show_env Pos.none env;
      ()
    )

  let print_solve_cc (env, cc, unblocked) rep =
    if get_log_level env >= 2 then (
      Format.printf "@[<v 0>Solved CC of #%s:@ " (Rep.show rep);
      Format.printf "CCs:@ ";
      CC.pp Format.std_formatter cc;
      Format.printf "@ Unblocked CCs:@ ";
      RepSet.pp Format.std_formatter unblocked;
      Format.printf "@ Env:@ @]@.";
      Typing_log.hh_show_env Pos.none env;
      ()
    )
end

(** Build the connected components and their dependencies from an inference environment. *)
let build_ccs (env : env) ~make_on_error : env * CC.t =
  let vars = Inf.get_unsolved_vars env.inference_env in
  let cc = Equiv.init vars in
  let deps = [] in
  let (env, cc, deps) =
    List.fold vars ~init:(env, cc, deps) ~f:(fun (env, cc, deps) v ->
        (* It turns out there are corner cases where v can already be solved.
           TODO: Remove this check and figure out how. *)
        let env = Env.unsolve env v in
        Debug.print_build_cc_start env v;
        let tvar = LoclType (mk (Reason.none, Tvar v)) in
        let on_error = Some (make_on_error v) in
        let bounds = Env.get_tyvar_lower_bounds env v in
        (* Remove the bounds, they'll be readded cleanly in prop_to_env_cc, thus also making
         * sure that the constraint graph is consistent, i.e. if v1 is a lower bound of v2,
         * then v2 is an upper bound of v1. This will also allow to simplify away bounds like
         * #1 <: (#1 | #2) *)
        let env = Env.set_tyvar_lower_bounds env v ITySet.empty in
        let (env, cc, deps) =
          ITySet.fold
            (fun ty_sub (env, cc, deps) ->
              (* Essentially, we break down unions in lower bounds and intersections
                 in upper bounds, which allows to easily see which
                 variables are in the same CCs and which CCs depend on
                 each other.
                 The advantage of using simplify_subtype here is that simplify_subtype is the unique source of
                 truth for how to break down unions in lower bounds and intersections in upper
                 bounds. E.g. (A & (B | C)) <: #1 could be broken down as [A & B, A & C] <: #1,
                 but if simplify_subtype does not break it down this way, then we don't either.
                 This way, we make sure all the edges in the
                 graph are consistent, i.e. if v has bound v', then v' has bound v.
                 Besides, in prop_to_env_cc below, we also make sure that for each bound of the form
                 #1 <: ?#2, we have the bound (nonnull & #1) <: #2
                 NB: these calls to subtype should be relatively cheap because they're all of the form X <: #n *)
              let (env, prop) = subtype env ty_sub tvar ~on_error in
              let (env, cc, deps) = prop_to_env_cc (env, cc, deps) prop in
              (env, cc, deps))
            bounds
            (env, cc, deps)
        in
        (* Then do the same again with upper bounds. *)
        let bounds = Env.get_tyvar_upper_bounds env v in
        let env = Env.set_tyvar_upper_bounds env v ITySet.empty in
        let (env, cc, deps) =
          ITySet.fold
            (fun ty_super (env, cc, deps) ->
              let (env, prop) = subtype env tvar ty_super ~on_error in
              let (env, cc, deps) = prop_to_env_cc (env, cc, deps) prop in
              (env, cc, deps))
            bounds
            (env, cc, deps)
        in
        Debug.print_build_cc_end (env, cc, deps) v;
        (env, cc, deps))
  in
  let cc = CC.update (CC.init vars) cc in
  let cc = CC.add_deps cc deps in
  Debug.print_build_cc_result (env, cc);
  (env, cc)

(** Get type variables which are lower bounds of v. *)
let get_lower_vars env v =
  ITySet.fold
    (fun ty (env, vars) ->
      let (env, is_var_or_nonnull_and_var) = is_var_or_nonnull_and_var env ty in
      let vars =
        Option.fold is_var_or_nonnull_and_var ~init:vars ~f:(fun vars v ->
            ISet.add v vars)
      in
      (env, vars))
    (Env.get_tyvar_lower_bounds env v)
    (env, ISet.empty)

(** Get type variables which are upper bounds of v. *)
let get_upper_vars env v =
  ITySet.fold
    (fun ty (env, vars) ->
      let (env, is_var_or_nullable_var) = is_var_or_nullable_var env ty in
      let vars =
        Option.fold is_var_or_nullable_var ~init:vars ~f:(fun vars v ->
            ISet.add v vars)
      in
      (env, vars))
    (Env.get_tyvar_upper_bounds env v)
    (env, ISet.empty)

let rec collapse_cycles_from_var
    (path : Ident.t list)
    (v : Ident.t)
    ((env, equiv_map, visited) : env * Equiv.t * visited_vars) :
    env * Equiv.t * visited_vars =
  if Inf.tyvar_is_solved env.inference_env v then
    (env, equiv_map, visited)
  else
    (* Here we're traversing a connected component recursively (DFS) to find cycles.
     * The [path] parameter contains all the variables we've seen in the branch we're
     * currently exploring, so that if [v] appears in [path], we know there is a cycle
     * and we can update our equivalence classes for the cycles. *)
    let rec find_v_get_tail v path =
      match path with
      | [] -> None
      | v' :: tl when snd @@ Equiv.are_same_class equiv_map v v' -> Some tl
      | _ :: path -> find_v_get_tail v path
    in
    match find_v_get_tail v (List.rev path) with
    | Some cycle ->
      let equiv_map =
        List.fold cycle ~init:equiv_map ~f:(fun equiv_map v' ->
            Equiv.merge_classes v' v equiv_map)
      in
      (env, equiv_map, visited)
    | None ->
      if ISet.mem v visited then
        (env, equiv_map, visited)
      else
        let path = v :: path in
        let (env, lower_vars) = get_lower_vars env v in
        let (env, equiv_map, visited) =
          ISet.fold
            (collapse_cycles_from_var path)
            lower_vars
            (env, equiv_map, visited)
        in
        let visited = ISet.add v visited in
        (env, equiv_map, visited)

let collapse_cycles_from_var = collapse_cycles_from_var []

let collapse_cycles env cc rep =
  let vars = CC.get_vars cc rep in
  let (env, equiv_map, _visited) =
    ISet.fold
      collapse_cycles_from_var
      vars
      (env, Equiv.init (ISet.elements vars), ISet.empty)
  in
  Debug.print_collapse_cycle_equiv env rep equiv_map;
  let merge_tyvars env v rep = Inf.merge_tyvars env v (Equiv.Rep.id rep) in
  let (_equiv_map, inference_env) =
    Equiv.fold equiv_map ~init:env.inference_env ~f:merge_tyvars
  in
  let env = { env with inference_env } in
  Debug.print_collapse_cycle_env env;
  env

type variance = bool * bool

let get_variance env v : variance =
  let covariant = Env.get_tyvar_appears_covariantly env v in
  let contravariant = Env.get_tyvar_appears_contravariantly env v in
  (covariant, contravariant)

let set_variance env v (cov, contra) =
  if Inf.tyvar_is_solved env.inference_env v then
    env
  else
    let env =
      if cov then
        Env.set_tyvar_appears_covariantly env v
      else
        env
    in
    let env =
      if contra then
        Env.set_tyvar_appears_contravariantly env v
      else
        env
    in
    env

let merge_variances (cov1, contra1) (cov2, contra2) =
  (cov1 || cov2, contra1 || contra2)

(** Unifies the variances of all type variables in the same CC and return
    the resulting variance. *)
let get_cc_variance env cc (rep : Rep.t) : env * variance =
  let vars = CC.get_vars cc rep in
  let variance =
    ISet.fold
      (fun v variance -> merge_variances variance (get_variance env v))
      vars
      (false, false)
  in
  let env = ISet.fold (fun v env -> set_variance env v variance) vars env in
  (env, variance)

(** Find a type within the constraints for v which is the smallest possible,
    usually the union of lower bounds. *)
let find_small_type env v =
  let bounds = Env.get_tyvar_lower_bounds env v in
  (* For now assert there are no constraint types in lower bounds. If it turns
     out there are, we'll fix that. *)
  assert (ITySet.for_all is_locl_type bounds);
  let bounds = TySet.elements @@ Utils.filter_locl_types bounds in
  (* We don't care for reasons for global inference. *)
  let r = Reason.none in
  let (env, ty) = Typing_union.union_list env r bounds in
  let ty = Utils.err_if_var_in_ty env v ty in
  (env, ty)

(** Solve a type variable to the union of its lower bounds. If there are unsolved
    type variables in those lower bounds, solve them first recursively. *)
let rec solve_down v (env, cc, deps) ~make_on_error =
  if Inf.tyvar_is_solved env.inference_env v then
    (env, cc, deps)
  else
    let (env, vars) = get_lower_vars env v in
    let (env, cc, deps) =
      ISet.fold (solve_down ~make_on_error) vars (env, cc, deps)
    in
    let upper_bounds = Env.get_tyvar_upper_bounds env v in
    let (env, ty) = find_small_type env v in
    let env = Env.add env v ty in
    let (env, cc, deps) =
      ITySet.fold
        (fun ty_super (env, cc, deps) ->
          (* TODO: unclear if we need to handle type constants. Convert them to constraint types?
             or at least `make_all_type_const_equal` should also return a prop. Also if
             this needs to be done, there is CC dependencies between this cc and
             the cc of vars in type constants of this var. *)
          let (env, prop) =
            subtype
              env
              (LoclType ty)
              ty_super
              ~on_error:(Some (make_on_error v))
          in
          let (env, cc, deps) = prop_to_env_cc (env, cc, deps) prop in
          (env, cc, deps))
        upper_bounds
        (env, cc, deps)
    in
    (env, cc, deps)

(** Find a type within the constraints for v which is the largest possible,
    usually the intersection of upper bounds. *)
let find_large_type env v =
  let bounds = Env.get_tyvar_upper_bounds env v in
  (* We don't care for reasons for global inference. *)
  let r = Reason.none in
  if ITySet.exists is_constraint_type bounds then
    let (env, lower_vars) = get_lower_vars env v in
    if ISet.is_empty lower_vars then
      find_small_type env v
    else
      Env.fresh_type_error env Pos.none
  else
    let bounds = TySet.elements @@ Utils.filter_locl_types bounds in
    let (env, ty) = Typing_intersection.intersect_list env r bounds in
    let ty = Utils.err_if_var_in_ty env v ty in
    (env, ty)

(** Solve a type variable to the intersection of its upper bounds. If there are unsolved
    type variables in those upper bounds, solve them first recursively. *)
let rec solve_up v (env, cc, deps) ~make_on_error =
  (* What happens here is the dual of function solve_down. *)
  if Inf.tyvar_is_solved env.inference_env v then
    (env, cc, deps)
  else
    let (env, vars) = get_upper_vars env v in
    let (env, cc, deps) =
      ISet.fold (solve_up ~make_on_error) vars (env, cc, deps)
    in
    let (env, ty) = find_large_type env v in
    let lower_bounds = Env.get_tyvar_lower_bounds env v in
    let env = Env.add env v ty in
    let (env, cc, deps) =
      ITySet.fold
        (fun ty_sub (env, cc, deps) ->
          let (env, prop) =
            subtype env ty_sub (LoclType ty) ~on_error:(Some (make_on_error v))
          in
          let (env, cc, deps) = prop_to_env_cc (env, cc, deps) prop in
          (env, cc, deps))
        lower_bounds
        (env, cc, deps)
    in
    (env, cc, deps)

let solve (covariant, contravariant) =
  match (covariant, contravariant) with
  | (false, true) -> solve_up
  | (true, false)
  | (false, false)
  | (true, true) ->
    solve_down

let solve_cc (env : env) (cc : CC.t) (rep : Rep.t) make_on_error : env * CC.t =
  let env = collapse_cycles env cc rep in
  (* Check all vars in cc have the same variance. *)
  let (env, variance) = get_cc_variance env cc rep in
  let (env, cls, deps) =
    ISet.fold
      (solve variance ~make_on_error)
      (CC.get_vars cc rep)
      (env, CC.to_equiv cc, [])
  in
  let cc = CC.update cc cls in
  let cc = CC.add_deps cc deps in
  (env, cc)

let solve_ccs (env, cc) ~make_on_error =
  let rec solve_unblocked env cc (unblocked_ccs : RepSet.t) =
    match RepSet.choose_opt unblocked_ccs with
    | None -> (env, cc)
    | Some rep ->
      let unblocked_ccs = RepSet.remove rep unblocked_ccs in
      let (env, cc) = solve_cc env cc rep make_on_error in
      let (cc, unblocked_ccs) = CC.make_non_blocking (cc, unblocked_ccs) rep in
      Debug.print_solve_cc (env, cc, unblocked_ccs) rep;
      solve_unblocked env cc unblocked_ccs
  in
  let (env, _cc) = solve_unblocked env cc (CC.get_all_unblocked_ccs cc) in
  (* TODO: deal with remaining unsolved CCs.
     Those should still be blocked because they should belong to cycles or be blocked by cycles. *)
  env

let solve_env (env : env) make_on_error : env =
  env |> build_ccs ~make_on_error |> solve_ccs ~make_on_error
