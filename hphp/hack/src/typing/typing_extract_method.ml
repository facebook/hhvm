(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let mk_tparam pos name tp_constraints =
  Typing_defs_core.
    {
      tp_variance = Ast_defs.Invariant;
      tp_name = (pos, name);
      tp_tparams = [];
      tp_constraints;
      tp_reified = Ast_defs.Erased;
      tp_user_attributes = [];
    }
(* -- This analysis --------------------------------------------------------- *)

module This_variance : sig
  val of_ty :
    Typing_defs_core.decl_ty ->
    env:Typing_env_types.env ->
    Ast_defs.variance option

  val of_fun_ty :
    Typing_defs_core.decl_phase Typing_defs_core.ty Typing_defs_core.fun_type ->
    env:Typing_env_types.env ->
    Ast_defs.variance option
end = struct
  let mul v1 v2 =
    let open Ast_defs in
    match (v1, v2) with
    | (Invariant, _)
    | (_, Invariant) ->
      Invariant
    | (Covariant, v)
    | (v, Covariant) ->
      v
    | (Contravariant, Contravariant) -> Covariant

  let join v1 v2 =
    let open Ast_defs in
    match (v1, v2) with
    | (Covariant, Covariant) -> Covariant
    | (Contravariant, Contravariant) -> Contravariant
    | (Invariant, _)
    | (_, Invariant)
    | (Covariant, Contravariant)
    | (Contravariant, Covariant) ->
      Invariant

  let update acc var =
    Some (Option.value_map acc ~default:var ~f:(fun acc -> join acc var))

  let is_invariant v =
    Option.value_map v ~default:false ~f:(function
        | Ast_defs.Invariant -> true
        | _ -> false)

  let rec find (ty : Typing_defs_core.decl_ty) ~var ~acc ~env =
    let open Typing_defs_core in
    match get_node ty with
    | Tthis -> update acc var
    | Tmixed
    | Twildcard
    | Tany _
    | Tnonnull
    | Tdynamic
    | Tprim _
    | Tgeneric _
    | Taccess _ ->
      acc
    | Toption ty
    | Tlike ty
    | Tclass_ptr ty ->
      find ty ~var ~acc ~env
    | Tvec_or_dict (ty_k, ty_v) ->
      let acc = find ty_k ~var ~acc ~env in
      if is_invariant acc then
        acc
      else
        find ty_v ~var ~acc ~env
    | Tunion tys
    | Tintersection tys ->
      find_list tys ~var ~acc ~env
    | Trefinement (ty, _class_refinement) ->
      (* TODO - should this include refinements?? *)
      find ty ~var ~acc ~env
    | Tfun fun_ty -> find_fun_ty fun_ty ~var ~acc ~env
    | Ttuple tuple_ty -> find_tuple_ty tuple_ty ~var ~acc ~env
    | Tshape shape_ty -> find_shape_ty shape_ty ~var ~acc ~env
    | Tapply ((_, class_name), tys) -> begin
      let tparams_opt =
        Option.map
          (Decl_entry.to_option (Typing_env.get_class env class_name))
          ~f:Folded_class.tparams
      in

      Option.value_map tparams_opt ~default:acc ~f:(fun tparams ->
          find_with_tparams tys tparams ~var ~acc ~env)
    end

  and find_list tys ~var ~acc ~env =
    match tys with
    | [] -> acc
    | ty :: tys ->
      let acc = find ty ~var ~acc ~env in
      if is_invariant acc then
        acc
      else
        find_list tys ~var ~acc ~env

  and find_with_tparams tys tparams ~var ~acc ~env =
    match (tys, tparams) with
    | (ty :: tys, { tp_variance; _ } :: tparams) ->
      let acc = find ty ~var:(mul var tp_variance) ~acc ~env in
      if is_invariant acc then
        acc
      else
        find_with_tparams tys tparams ~var ~acc ~env
    | ([], _)
    | (_, []) ->
      (* Assumes well-formedness *)
      acc

  and find_tuple_ty { t_required; t_extra } ~var ~acc ~env =
    let acc = find_list t_required ~var ~acc ~env in
    if is_invariant acc then
      acc
    else begin
      match t_extra with
      | Tsplat ty -> find ty ~var ~acc ~env
      | Textra { t_optional; t_variadic } -> begin
        let acc = find_list t_optional ~var ~acc ~env in
        if is_invariant acc then
          acc
        else
          find t_variadic ~var ~acc ~env
      end
    end

  and find_shape_ty { s_unknown_value; s_fields; _ } ~var ~acc ~env =
    let acc = find s_unknown_value ~var ~acc ~env in
    if is_invariant acc then
      acc
    else
      let tys =
        List.map
          (Typing_defs_core.TShapeMap.bindings s_fields)
          ~f:(fun (_, { sft_ty; _ }) -> sft_ty)
      in
      find_list tys ~var ~acc ~env

  and find_fun_ty { ft_params; ft_ret; _ } ~var ~acc ~env =
    let acc = find ft_ret ~var ~acc ~env in
    if is_invariant acc then
      acc
    else
      let var = mul var Ast_defs.Contravariant in
      let tys = List.map ft_params ~f:(fun { fp_type; _ } -> fp_type) in
      find_list tys ~var ~acc ~env

  let of_ty ty ~env = find ty ~var:Covariant ~acc:None ~env

  let of_fun_ty fun_ty ~env = find_fun_ty fun_ty ~var:Covariant ~acc:None ~env
end

(* -- Type constant analysis and substitutions ------------------------------ *)

module Typeconst_analysis : sig
  module Trie : sig
    type t
  end

  type t = {
    this_name: string;
    this_folded_class: Folded_class.t;
    this_trie: Trie.t;
    tries: (Typing_defs_core.decl_ty * Trie.t) SMap.t;
  }

  (** Generate an analysis of a function type which resolves all type constants
      mentioned in the functin type  *)
  val of_fun_type :
    Typing_defs_core.decl_phase Typing_defs_core.ty Typing_defs_core.fun_type ->
    string ->
    Folded_class.t ->
    Typing_env_types.env ->
    t

  (** Pretty print an analysis for debugging *)
  val show : t -> Typing_env_types.env -> string

  module Subst : sig
    type t = {
      this_name: string;
      this_ty: Typing_defs_core.decl_phase Typing_defs_core.ty;
      tparams:
        Typing_defs_core.decl_phase Typing_defs_core.ty Typing_defs_core.tparam
        list;
      this_subst: Typing_defs_core.decl_phase Typing_defs_core.ty SMap.t;
      class_subst: Typing_defs_core.decl_phase Typing_defs_core.ty SMap.t SMap.t;
    }

    val apply :
      t ->
      Typing_defs_core.decl_phase Typing_defs_core.ty ->
      Typing_defs_core.decl_phase Typing_defs_core.ty

    val apply_fun_ty :
      t ->
      Typing_defs_core.decl_phase Typing_defs_core.ty Typing_defs_core.fun_type ->
      Typing_defs_core.decl_phase Typing_defs_core.ty Typing_defs_core.fun_type
  end

  val to_subst :
    t ->
    Typing_defs_core.decl_phase Typing_defs_core.ty ->
    Typing_env_types.env ->
    Subst.t
end = struct
  (** Helper to accumulate the path of projections on a given root type *)
  let rec access_path root prefix =
    let open Typing_defs_core in
    match get_node root with
    | Taccess (root, ty_const) -> access_path root (ty_const :: prefix)
    | _ -> (root, prefix)

  (** Helper to lift transformation on [decl_ty] into transformations on a
      [typeconst]  *)
  let transform_typeconst typeconst ~f =
    let open Typing_defs in
    match typeconst with
    | TCConcrete { tc_type } ->
      let tc_type = f tc_type in
      TCConcrete { tc_type }
    | TCAbstract { atc_as_constraint; atc_super_constraint; atc_default } ->
      let atc_as_constraint = Option.map ~f atc_as_constraint
      and atc_super_constraint = Option.map ~f atc_super_constraint
      and atc_default = Option.map ~f atc_default in
      TCAbstract { atc_as_constraint; atc_super_constraint; atc_default }

  module Trie = struct
    type base =
      | Root
      | Typeconst of {
          typeconst: Typing_defs.typeconst;
          pos: Pos_or_decl.t;
          origin: string option;
        }

    let transform_base base ~f =
      match base with
      | Root -> Root
      | Typeconst { typeconst; pos; origin } ->
        let typeconst = transform_typeconst typeconst ~f in
        Typeconst { typeconst; pos; origin }

    type t = {
      base: base;
      children: status SMap.t;
    }

    and status =
      | Undefined
      | Defined of t

    let rec merge_help s1 s2 =
      match (s1, s2) with
      | (Undefined, s)
      | (s, Undefined) ->
        Some s
      | (Defined t1, Defined t2) ->
        Option.map ~f:(fun t -> Defined t) (merge t1 t2)

    and merge t1 t2 : t option =
      let children =
        SMap.union
          ~combine:(fun _key s1 s2 -> merge_help s1 s2)
          t1.children
          t2.children
      in
      Some { t1 with children }

    let rec transform { base; children } ~f =
      let base = transform_base base ~f
      and children = SMap.map (transform_status ~f) children in
      { base; children }

    and transform_status status ~f =
      match status with
      | Undefined -> Undefined
      | Defined t -> Defined (transform t ~f)
    (* -- Pretty printing ----------------------------------------------------- *)

    let rec show_help t ~indent ~acc ~env =
      List.fold_left (SMap.bindings t) ~init:acc ~f:(fun acc (nm, status) ->
          show_status (nm, status) ~indent ~acc ~env)

    and show_status (nm, status) ~indent ~acc ~env =
      match status with
      | Undefined ->
        let padding = String.make indent ' ' in
        Format.sprintf "%s%s == undefined\n" padding nm :: acc
      | Defined { base = Typeconst { typeconst; _ }; children; _ } ->
        let acc = show_typeconst (nm, typeconst) ~indent ~acc ~env in
        let indent = indent + 4 in
        show_help children ~indent ~acc ~env
      | Defined { base = Root; children; _ } ->
        let padding = String.make indent ' ' in
        let acc = Format.sprintf "%sthis\n" padding :: acc in
        let indent = indent + 4 in
        show_help children ~indent ~acc ~env

    and show_typeconst (nm, typeconst) ~indent ~acc ~env =
      let padding = String.make indent ' ' in
      let data =
        let open Typing_defs in
        match typeconst with
        | TCConcrete { tc_type } ->
          Format.sprintf
            "%s%s == %s\n"
            padding
            nm
            (Typing_print.debug_decl env tc_type)
        | TCAbstract { atc_as_constraint; atc_super_constraint; _ } ->
          Format.sprintf
            "%s%s as %s super %s\n"
            padding
            nm
            (Option.value_map
               atc_as_constraint
               ~default:"(none)"
               ~f:(Typing_print.debug_decl env))
            (Option.value_map
               atc_super_constraint
               ~default:"(none)"
               ~f:(Typing_print.debug_decl env))
      in
      data :: acc

    let show (name, t) env =
      String.concat ~sep:""
      @@ List.rev
      @@ show_help t.children ~indent:4 ~acc:[Format.sprintf "%s\n" name] ~env

    (* -- Helpers ------------------------------------------------------------  *)
    let empty = { base = Root; children = SMap.empty }

    let constants t = SMap.keys t.children

    let update t const_name child =
      let { children; _ } = t in
      let children = SMap.add const_name child children in
      { t with children }

    let of_typeconst ?origin typeconst ~const_pos =
      let base = Typeconst { typeconst; pos = const_pos; origin } in
      { base; children = SMap.empty }

    let status_of_typeconst ?origin typeconst ~const_pos =
      let t = of_typeconst typeconst ?origin ~const_pos in
      Defined t

    let rec find_path t path =
      match path with
      | [] -> Some (Defined t)
      | (_, next) :: rest -> begin
        match SMap.find_opt next t.children with
        | None -> None
        | Some Undefined -> Some Undefined
        | Some (Defined t) -> find_path t rest
      end
  end

  (** An analysis is a set of of type constant tries one of which is considered
      to be [this] *)
  type t = {
    this_name: string;
    this_folded_class: Folded_class.t;
    this_trie: Trie.t;
    tries: (Typing_defs_core.decl_ty * Trie.t) SMap.t;
  }

  let create this_name this_folded_class =
    { this_name; this_folded_class; this_trie = Trie.empty; tries = SMap.empty }

  let show { this_name; this_trie; tries; _ } env =
    let this = (Format.sprintf "this (%s)" this_name, this_trie)
    and others =
      List.map ~f:(fun (key, (_, trie)) -> (key, trie)) (SMap.bindings tries)
    in
    let lines =
      List.map (this :: others) ~f:(fun (name, trie) ->
          Trie.show (name, trie) env)
    in
    String.concat lines ~sep:"\n\n"

  let find_trie_for { this_name; this_trie; tries; _ } class_name =
    if String.equal class_name this_name then
      Some this_trie
    else
      Option.map ~f:snd (SMap.find_opt class_name tries)
  (* -- Helpers ------------------------------------------------------------- *)

  (** When accessing a type constant through another type constant we need to
      determine the type that contains the next type constant.
      For a 'concrete' type constant this is the definition and for an 'abstract' type constant it is
      the upper-bound, if it exists. *)
  let type_for_access typeconst =
    let open Typing_defs in
    match typeconst with
    | TCConcrete { tc_type } -> Some tc_type
    | TCAbstract { atc_as_constraint; _ } -> atc_as_constraint

  (** Helper to convert between type constant refinement representations  *)
  let typeconst_of_refined_const Typing_defs_core.{ rc_bound; _ } ~const_pos =
    match rc_bound with
    | Typing_defs_core.TRexact tc_type -> Typing_defs.(TCConcrete { tc_type })
    | Typing_defs_core.TRloose bounds ->
      let Typing_defs_core.{ tr_lower; tr_upper } = bounds in
      let atc_as_constraint =
        match tr_upper with
        | [] -> None
        | _ ->
          let reason = Typing_reason.witness_from_decl const_pos in
          Some (Typing_make_type.intersection reason tr_upper)
      and atc_super_constraint =
        match tr_lower with
        | [] -> None
        | _ ->
          let reason = Typing_reason.witness_from_decl const_pos in
          Some (Typing_make_type.union reason tr_lower)
      in
      Typing_defs.(
        TCAbstract
          { atc_as_constraint; atc_super_constraint; atc_default = None })

  (** When following a path of type constant accesses the meaning of [this] is
      relative to the class containing the type constant. Since we want to
      extract the contants for use from some base class we need to make these
      references absolute.

      As an example, consider the following

      ```
      class C<T> { }

      class B {
        const type TB as C<this>;
      }

      abstract class A {
        abstract const type TA as B;
        public static function wibble(this::TA::TB $_): void {}
      }
      ```

      When access `this::TA::TB` we end up with `C<this>` where `this` corresponds
      to the class in which `TB` is defined. Since we have access this type
      through `A` we can make it 'absolute' by subsituting occurrences of
      this in the definition of `this::TA::TB` with prefix of that path i.e.
      `this::TA`. Doing so gives us `this::TA::TB as C<this::TA>`
  *)
  let this_to_absolute ty ~prefix =
    let open Typing_defs_core in
    let rec build this reason prefix ~k =
      match prefix with
      | [] -> k this
      | next :: rest ->
        build this reason rest ~k:(fun root ->
            k (mk (reason, Taccess (root, next))))
    in
    let f ty ~ctx =
      match deref ty with
      | (reason, Tthis) -> build ty reason prefix ~k:(fun ty -> (ctx, `Stop ty))
      | _ -> (ctx, `Continue ty)
    in
    Typing_defs_core.transform_top_down_decl_ty ty ~ctx:() ~f

  let replace_this_with ty ~replacement =
    let open Typing_defs_core in
    let f ty ~ctx =
      match get_node ty with
      | Tthis -> (ctx, `Stop replacement)
      | _ -> (ctx, `Continue ty)
    in
    Typing_defs_core.transform_top_down_decl_ty ty ~ctx:() ~f

  (** Accumulate all types mentioned in the definition or bounds of a type constant *)
  let accumulate_tys typeconst tys =
    let open Typing_defs in
    match typeconst with
    | TCConcrete { tc_type } -> tc_type :: tys
    | TCAbstract { atc_as_constraint; atc_super_constraint; _ } ->
      (match (atc_as_constraint, atc_super_constraint) with
      | (Some ty1, Some ty2) -> ty1 :: ty2 :: tys
      | (Some ty, _)
      | (_, Some ty) ->
        ty :: tys
      | _ -> tys)

  type path =
    | This of (Pos_or_decl.t * string) list
    | Class of Typing_defs_core.decl_ty * string * (Pos_or_decl.t * string) list

  (** Fold over [ty] and accumulate type constant access paths for any
      [Taccess] which is rooted in [TThis] *)
  let paths_of_ty ty =
    let paths = ref [] in
    let on_ty decl_ty ~ctx =
      let open Typing_defs_core in
      match get_node decl_ty with
      | Taccess (root_ty, ty_const) ->
        let (ty, path) = access_path root_ty [ty_const] in
        let () =
          match get_node ty with
          | Tthis -> paths := This path :: !paths
          | Tapply ((_, class_name), _) ->
            paths := Class (ty, class_name, path) :: !paths
          | _ -> ()
        in
        (ctx, `Stop decl_ty)
      | _ -> (ctx, `Continue decl_ty)
    in
    let _ = Typing_defs_core.transform_top_down_decl_ty ty ~ctx:() ~f:on_ty in
    !paths
  (* -- Core logic ---------------------------------------------------------  *)

  (** Find the definition or bounds of the next type constant [const_name]
      when projecting from [base]. If the constant already appeared in some
      other path we can use the information stored in [children] otherwise
      we have to look it up from the decl *)
  let rec find_type_const trie tys const prefix analysis env =
    let Trie.{ children; base } = trie and (const_pos, const_name) = const in
    match SMap.find_opt const_name children with
    (* Happy path - we've already seen the constant as part of some other path
       so reuse the result *)
    | Some status -> (status, tys, analysis)
    (* We haven't already seen this constant within this path we need to find the
       definition within the current type we are projecting from *)
    | None -> begin
      match base with
      | Trie.Root ->
        (* If we are projecting from the root we can use the [folded_class]
           corresponding to that root directly - since we are at the root
           we don't need to rewrite `this` to make absolute *)
        let typeconst_opt =
          Folded_class.get_typeconst analysis.this_folded_class const_name
        in
        let (status, tys) =
          Option.value_map
            typeconst_opt
            ~default:(Trie.Undefined, tys)
            ~f:(fun Typing_defs.{ ttc_kind = typeconst; ttc_origin = origin; _ }
               ->
              let status = Trie.status_of_typeconst typeconst ~origin ~const_pos
              (* The type constant may be an alias - if so we need need to add
                 the path to ensure it is substituted *)
              and tys = accumulate_tys typeconst tys in
              (status, tys))
        in
        (status, tys, analysis)
      | Trie.Typeconst { typeconst; _ } -> begin
        (* If the root is a type constant we first need to get the type we're
           accessing through; for a 'concrete' type constant this is the definition
           and for an 'abstract' type constant it is the upper bound *)
        match type_for_access typeconst with
        | None -> (Undefined, tys, analysis)
        | Some base_ty -> access_typeconst base_ty tys const prefix analysis env
      end
    end

  (** Access [const_name] through [base_ty] and modify occurences of `this` in the
      typeconstant definition so they are absolute with respect to [folded_class]

      This returns a `(typeconst option, path) result` since it may be the case
      that [base_ty] is a path which we haven't yet added to the analysis
  *)
  and access_typeconst base_ty tys const prefix analysis env =
    let open Typing_defs_core in
    let (const_pos, const_name) = const in
    match get_node base_ty with
    | Tthis ->
      (* For this use the [folded_class] corresponding to the absolute root
         class for the analysis *)
      let typeconst_opt =
        Folded_class.get_typeconst analysis.this_folded_class const_name
      in
      let (status, tys) =
        Option.value_map
          typeconst_opt
          ~default:(Trie.Undefined, tys)
          ~f:(fun Typing_defs.{ ttc_kind = typeconst; ttc_origin = origin; _ }
             ->
            (* Ensure references to `this` are made absolute *)
            let typeconst =
              transform_typeconst ~f:(this_to_absolute ~prefix) typeconst
            in
            let status = Trie.status_of_typeconst typeconst ~origin ~const_pos
            (* Accumulate the types in the type constant - if they are aliases
               to another type constant we will need to add them to the
               analysis *)
            and tys = accumulate_tys typeconst tys in
            (status, tys))
      in
      (status, tys, analysis)
    | Tapply ((_, class_name), _) ->
      let folded_class_opt =
        Decl_entry.to_option (Typing_env.get_class env class_name)
      in
      let typeconst_opt =
        Option.bind folded_class_opt ~f:(fun folded_class ->
            Folded_class.get_typeconst folded_class const_name)
      in
      let (status, tys) =
        Option.value_map
          typeconst_opt
          ~default:(Trie.Undefined, tys)
          ~f:(fun Typing_defs.{ ttc_kind = typeconst; ttc_origin = origin; _ }
             ->
            let typeconst =
              transform_typeconst ~f:(this_to_absolute ~prefix) typeconst
            in
            let status = Trie.status_of_typeconst typeconst ~origin ~const_pos
            (* Accumulate the types in the type constant - if they are aliases
               to another type constant we will need to add them to the
               analysis *)
            and tys = accumulate_tys typeconst tys in
            (status, tys))
      in
      (status, tys, analysis)
    | Trefinement (base_ty, { cr_consts }) -> begin
      (* If there is a refinement check if it contains the contant we want;
         if not, search in the root type *)
      match SMap.find_opt const_name cr_consts with
      | Some refined_const ->
        (* For the definition of the refined constant [this] is in the same
           class so don't make absolute *)
        let typeconst = typeconst_of_refined_const refined_const ~const_pos in
        let status = Trie.status_of_typeconst typeconst ~const_pos in
        (* Accumulate the types in the type constant - if they are aliases
           to another type constant we will need to add them to the
           analysis *)
        let tys = accumulate_tys typeconst tys in
        (status, tys, analysis)
      | None -> access_typeconst base_ty tys const prefix analysis env
    end
    | Taccess (root_ty, ty_const) -> begin
      (* We're accessing a type constant through a type constant access path;
         ensure the path has been added then access the type constant through
         its definition or upper bound *)
      let (ty, path) = access_path root_ty [ty_const] in
      let (analysis, status_opt) =
        match get_node ty with
        | Tthis ->
          let paths = [This path] in
          let analysis = add_paths analysis paths env in
          let status_opt = Trie.find_path analysis.this_trie path in
          (analysis, status_opt)
        | Tapply ((_, class_name), _) ->
          let paths = [Class (ty, class_name, path)] in
          let analysis = add_paths analysis paths env in
          let status_opt =
            Option.bind
              ~f:(fun trie -> Trie.find_path trie path)
              (find_trie_for analysis class_name)
          in
          (analysis, status_opt)
        | _ ->
          (* Here we assume the root must either be [Tapply] or [Tthis] -
             the other possibility, [Trefinement], is disallowed *)
          (analysis, None)
      in
      let base_ty_opt =
        Option.bind status_opt ~f:(fun status ->
            match status with
            | Trie.Undefined -> None
            | Trie.Defined { base = Root; _ } -> Some ty
            | Trie.Defined { base = Typeconst { typeconst; _ }; _ } ->
              Option.map
                (type_for_access typeconst)
                ~f:(replace_this_with ~replacement:ty))
      in
      Option.value_map
        base_ty_opt
        ~default:(Trie.Undefined, tys, analysis)
        ~f:(fun base_ty ->
          access_typeconst base_ty tys const prefix analysis env)
    end
    (* We can't access type constants through any other type so return [Undefined] *)
    | _ -> (Trie.Undefined, tys, analysis)

  and add_path trie tys (prefix, path) analysis env =
    match path with
    | [] ->
      (* End of the type constant access path; return the updated trie and
         the types appearing in the definitions or bounds of type constants
         discovered *)
      (trie, tys, analysis)
    | ((_, const_name) as const) :: path -> begin
      (* Find the type constant [const_name] *)
      match find_type_const trie tys const prefix analysis env with
      | (Trie.Undefined, tys, analysis) ->
        (* If it was undefined we have an error so we can't add the rest of the
           path *)
        let trie = Trie.update trie const_name Trie.Undefined in
        (trie, tys, analysis)
      | (Trie.Defined child, tys, analysis) ->
        (* We found the constant so continue adding the remainder of the path *)
        let (child, tys, analysis) =
          add_path child tys (const :: prefix, path) analysis env
        in
        (* Update the trie with the subtrie for [child] *)
        (Trie.update trie const_name (Trie.Defined child), tys, analysis)
    end

  (** Add a number of type constant access [paths] to the analysis [t] *)
  and add_paths t paths env =
    match paths with
    | [] -> t
    | This path :: paths -> begin
      let (this_trie, tys, t) = add_path t.this_trie [] ([], path) t env in
      let paths = List.concat (paths :: List.map tys ~f:paths_of_ty) in
      let t = { t with this_trie } in
      add_paths t paths env
    end
    | Class (this_ty, class_name, path) :: paths
      when String.equal class_name Naming_special_names.Classes.cSelf ->
      let paths = Class (this_ty, t.this_name, path) :: paths in
      add_paths t paths env
    | Class (this_ty, class_name, path) :: paths -> begin
      let folded_class_opt =
        Decl_entry.to_option (Typing_env.get_class env class_name)
      in
      match folded_class_opt with
      | None -> add_paths t paths env
      | Some folded_class -> begin
        (* Stash the current notion of this so we can restore after analysing
           the path rooted in a different class *)
        let { this_name; this_trie; this_folded_class; _ } = t in
        (* Find the exist trie for this class if it exists *)
        let class_trie =
          Option.value ~default:Trie.empty (find_trie_for t class_name)
        in
        (* Add the path with the class as [this] *)
        let (class_trie, tys, t) =
          let t =
            {
              t with
              this_trie = class_trie;
              this_name = class_name;
              this_folded_class = folded_class;
            }
          in
          add_path class_trie [] ([], path) t env
        in
        (* Add the paths ensuring that we replace occurrences of this with class type *)
        let paths =
          let new_paths =
            List.map tys ~f:(fun ty ->
                paths_of_ty (replace_this_with ty ~replacement:this_ty))
          in
          List.concat (paths :: new_paths)
        in
        (* Move the trie for the current class in to the map *)
        let tries =
          SMap.update
            class_name
            (function
              | None -> Some (this_ty, class_trie)
              | Some (this_ty, existing_trie) ->
                Option.map
                  ~f:(fun trie -> (this_ty, trie))
                  (Trie.merge existing_trie class_trie))
            t.tries
        in
        (* Restore the previous notion of [this] *)
        let t = { this_name; this_trie; this_folded_class; tries } in
        add_paths t paths env
      end
    end

  (* -- API ----------------------------------------------------------------- *)

  let add_type t ty env =
    let paths = paths_of_ty ty in
    let t = add_paths t paths env in
    let { this_name; this_trie; tries; _ } = t in
    (* For analysis of classes other than this we need to subsitute occurrences
       of [this] at the root of typeconst accesses for the class type *)
    let tries =
      SMap.map
        (fun (replacement, trie) ->
          let f ty = replace_this_with ty ~replacement in
          (replacement, Trie.transform trie ~f))
        tries
    in
    (* For the analysis of this we need to subsitute occurrences of typeconst
       access rooted in the explicit class with [this] *)
    let this_trie =
      let f ty ~ctx =
        let open Typing_defs_core in
        match deref ty with
        | (reason, Tapply ((_, class_name), _))
          when String.equal this_name class_name ->
          (ctx, `Stop (mk (reason, Tthis)))
        | _ -> (ctx, `Continue ty)
      in
      Trie.transform
        this_trie
        ~f:(Typing_defs_core.transform_top_down_decl_ty ~ctx:() ~f)
    in
    { t with this_trie; tries }

  let add_fun_type t fun_ty env =
    let open Typing_defs_core in
    let ty = mk (Typing_reason.none, Tfun fun_ty) in
    add_type t ty env

  let of_fun_type fun_ty class_name folded_class env =
    let empty = create class_name folded_class in
    add_fun_type empty fun_ty env

  let this_constants { this_trie; _ } = Trie.constants this_trie

  module Subst = struct
    type t = {
      this_name: string;
      this_ty: Typing_defs_core.decl_phase Typing_defs_core.ty;
      tparams:
        Typing_defs_core.decl_phase Typing_defs_core.ty Typing_defs_core.tparam
        list;
      this_subst: Typing_defs_core.decl_phase Typing_defs_core.ty SMap.t;
      class_subst: Typing_defs_core.decl_phase Typing_defs_core.ty SMap.t SMap.t;
    }

    (* -- Application ------------------------------------------------------- *)
    let apply { this_name; this_ty; this_subst; class_subst; _ } ty =
      let transform ty ~ctx =
        let open Typing_defs_core in
        match get_node ty with
        | Tthis -> (ctx, `Stop this_ty)
        | Taccess (root_ty, ty_const) -> begin
          (* Try and find a substibution for this type *)
          let (root_ty, path) = access_path root_ty [ty_const] in
          let key = String.concat ~sep:"::" (List.map ~f:snd path) in
          if SSet.mem key ctx then
            (* If we have made this subsitution before doing so again will
               cause us to never terminate so we bail; in practice the
               type constant cycle check should already have caught this
               TODO(mjt) add error *)
            (ctx, `Stop ty)
          else
            (* Try and find a substitution *)
            let subst_ty_opt =
              match get_node root_ty with
              | Tthis ->
                let ty_opt = SMap.find_opt key this_subst in
                if Option.is_some ty_opt then
                  ty_opt
                else
                  let subst_opt = SMap.find_opt this_name class_subst in
                  Option.bind subst_opt ~f:(fun subst ->
                      let key =
                        String.concat ~sep:"::" (List.map ~f:snd path)
                      in
                      SMap.find_opt key subst)
              | Tapply ((_, class_name), _) ->
                let subst_opt = SMap.find_opt class_name class_subst in
                Option.bind subst_opt ~f:(fun subst -> SMap.find_opt key subst)
              | _ -> None
            in
            (* If there is no subsitution, stop; if there is record that we've
               seen the source key before and restart the rewrite *)
            Option.value_map
              subst_ty_opt
              ~default:(ctx, `Stop ty)
              ~f:(fun ty ->
                let ctx = SSet.add key ctx in
                (ctx, `Restart ty))
        end
        | _ -> (ctx, `Continue ty)
      in
      Typing_defs_core.transform_top_down_decl_ty
        ty
        ~f:transform
        ~ctx:SSet.empty

    let apply_fun_ty t fun_ty =
      let open Typing_defs_core in
      let ty = mk (Typing_reason.none, Tfun fun_ty) in
      match get_node (apply t ty) with
      | Tfun fun_ty -> fun_ty
      | _ -> failwith "Expected function type"

    (* -- Build substitution for -------------------------------------------- *)

    type generic_info = {
      pos: Pos_or_decl.t;
      upper_bound: Typing_defs_core.decl_phase Typing_defs_core.ty option;
      lower_bound: Typing_defs_core.decl_phase Typing_defs_core.ty option;
    }

    (** Generate locally fresh names for generics *)
    let fresh_name names prefix =
      let n =
        match SMap.find_opt prefix names with
        | Some n -> n + 1
        | None -> 0
      in
      let name = Format.sprintf {|%s#%d|} prefix n in
      (SMap.add prefix n names, name)

    (* -- Build substitution for concrete type constants -------------------- *)

    let class_subst_help trie names generics =
      let rec aux Trie.{ base; children } (const_name, rev_path) acc =
        let acc =
          match base with
          | Typeconst { typeconst = Typing_defs.(TCConcrete { tc_type }); _ } ->
            let (subst, names, generics) = acc in
            let key = String.concat ~sep:"::" (List.rev rev_path) in
            (SMap.add key tc_type subst, names, generics)
          | Typeconst
              {
                typeconst =
                  Typing_defs.(
                    TCAbstract
                      {
                        atc_as_constraint = upper_bound;
                        atc_super_constraint = lower_bound;
                        _;
                      });
                pos;
                _;
              } ->
            let (subst, names, generics) = acc in
            let (names, generic_name) = fresh_name names const_name in
            let subst =
              let key = String.concat ~sep:"::" (List.rev rev_path) in
              let reason = Typing_reason.witness_from_decl pos in
              let ty = Typing_defs_core.(mk (reason, Tgeneric generic_name)) in
              SMap.add key ty subst
            in
            let generics =
              SMap.add generic_name { pos; upper_bound; lower_bound } generics
            in
            (subst, names, generics)
          | Root -> acc
        in
        SMap.fold
          (fun key status acc ->
            match status with
            | Trie.Undefined -> acc
            | Trie.Defined t -> aux t (key, key :: rev_path) acc)
          children
          acc
      in
      aux trie ("", []) (SMap.empty, names, generics)

    let class_subst { tries; _ } names generics =
      SMap.fold
        (fun class_name (_ty, trie) (tries, names, generics) ->
          let (subst, names, generics) = class_subst_help trie names generics in
          (SMap.add class_name subst tries, names, generics))
        tries
        (SMap.empty, names, generics)

    (* -- Build substitution for abstract type constants -------------------- *)

    let add_refinement rfmts const_name ~path ~subst =
      let key = String.concat ~sep:"::" (List.rev (const_name :: path)) in
      let ty_opt = SMap.find_opt key subst in
      Option.value_map ty_opt ~default:rfmts ~f:(fun ty ->
          SMap.add
            const_name
            Typing_defs_core.{ rc_bound = TRexact ty; rc_is_ctx = false }
            rfmts)

    let refine_ty decl_ty subst path children =
      let open Typing_defs_core in
      match deref decl_ty with
      | (reason, Typing_defs_core.Tapply _) -> begin
        match SMap.keys children with
        | [] -> decl_ty
        | abstr_consts ->
          let cr_consts =
            List.fold_left
              abstr_consts
              ~init:SMap.empty
              ~f:(fun rfmts const_name ->
                add_refinement rfmts const_name ~path ~subst)
          in
          let class_refinements = Typing_defs_core.{ cr_consts } in
          let ty_ = Typing_defs_core.Trefinement (decl_ty, class_refinements) in
          mk (reason, ty_)
      end
      | (reason, Typing_defs_core.Trefinement (inner_decl_ty, { cr_consts })) ->
      begin
        match SMap.keys children with
        | [] -> decl_ty
        | abstr_consts ->
          let cr_consts =
            List.fold_left
              abstr_consts
              ~init:cr_consts
              ~f:(fun rfmts const_name ->
                add_refinement rfmts const_name ~path ~subst)
          in
          let class_refinements = Typing_defs_core.{ cr_consts } in
          mk
            ( reason,
              Typing_defs_core.Trefinement (inner_decl_ty, class_refinements) )
      end
      | _ -> decl_ty

    let update acc ~key ~typeconst ~pos ~children ~path =
      match typeconst with
      | Typing_defs.(TCConcrete { tc_type }) ->
        let (subst, names, generics) = acc in
        let subst =
          SMap.add (String.concat (List.rev path) ~sep:"::") tc_type subst
        in
        (subst, names, generics)
      | Typing_defs.(TCAbstract { atc_as_constraint; atc_super_constraint; _ })
        ->
        let (subst, names, generics) = acc in
        let (names, generic_name) = fresh_name names key in
        let subst =
          let key = String.concat (List.rev path) ~sep:"::" in
          let reason = Typing_reason.witness_from_decl pos in
          let ty = Typing_defs_core.(mk (reason, Tgeneric generic_name)) in
          SMap.add key ty subst
        in
        let upper_bound =
          Option.map atc_as_constraint ~f:(fun ty ->
              refine_ty ty subst path children)
        and lower_bound =
          Option.map atc_super_constraint ~f:(fun ty ->
              refine_ty ty subst path children)
        in
        let generics =
          SMap.add generic_name { pos; upper_bound; lower_bound } generics
        in
        (subst, names, generics)

    let this_subst { this_trie; _ } names generics =
      let Trie.{ base; children } = this_trie in
      let rec aux children ~path ~init =
        SMap.fold
          (fun key status acc ->
            match status with
            | Trie.Undefined -> acc
            | Trie.(Defined { children; base }) -> begin
              match base with
              | Trie.Typeconst { typeconst; pos; _ } ->
                let path = key :: path in
                let acc = aux children ~path ~init:acc in
                update acc ~key ~typeconst ~pos ~children ~path
              | Trie.Root -> acc
            end)
          children
          init
      in
      let init = (SMap.empty, names, generics) in
      match base with
      | Trie.Root -> aux children ~path:[] ~init
      | _ -> init

    (* -- Build all substitutions from the analysis --------------------------- *)
    let mk_tparam pos name tp_constraints =
      Typing_defs_core.
        {
          tp_variance = Ast_defs.Invariant;
          tp_name = (pos, name);
          tp_tparams = [];
          tp_constraints;
          tp_reified = Ast_defs.Erased;
          tp_user_attributes = [];
        }

    let refine_this this_ty this_subst type_constants =
      if List.is_empty type_constants then
        this_ty
      else
        let open Typing_defs_core in
        match deref this_ty with
        | (reason, Tapply _) ->
          let cr_consts =
            List.fold_left
              ~f:(fun rfmts name ->
                match SMap.find_opt name this_subst with
                | Some ty ->
                  SMap.add
                    name
                    { rc_bound = TRexact ty; rc_is_ctx = false }
                    rfmts
                | _ -> rfmts)
              ~init:SMap.empty
              type_constants
          in
          let class_refinement = Typing_defs.{ cr_consts } in
          mk (reason, Trefinement (this_ty, class_refinement))
        | ( reason,
            Typing_defs_core.Trefinement (this_ty, Typing_defs.{ cr_consts }) )
          ->
          let cr_consts =
            List.fold_left
              ~f:(fun rfmts name ->
                match SMap.find_opt name this_subst with
                | Some ty ->
                  SMap.add
                    name
                    { rc_bound = TRexact ty; rc_is_ctx = false }
                    rfmts
                | _ -> rfmts)
              ~init:cr_consts
              type_constants
          in
          let class_refinement = Typing_defs.{ cr_consts } in
          mk (reason, Trefinement (this_ty, class_refinement))
        | _ -> this_ty

    let mk_tparams generics env =
      List.map
        (SMap.bindings generics)
        ~f:(fun (name, { pos; upper_bound; lower_bound }) ->
          (* We're generating a decl type so we need to add implicit
             upper bounds of [supportdyn<mixed>] under sdt or [mixed] otherwise
          *)
          let tp_constraints =
            let reason = Typing_reason.witness_from_decl pos in
            let ub =
              let tcopt = Typing_env_types.(env.genv.tcopt) in
              if TypecheckerOptions.everything_sdt tcopt then
                ( Ast_defs.Constraint_as,
                  Decl_enforceability.supportdyn_mixed pos reason )
              else
                (Ast_defs.Constraint_as, Typing_defs_core.(mk (reason, Tmixed)))
            in
            List.filter_opt
              [
                Option.map upper_bound ~f:(fun ty ->
                    (Ast_defs.Constraint_as, ty));
                Option.map lower_bound ~f:(fun ty ->
                    (Ast_defs.Constraint_super, ty));
                Some ub;
              ]
          in
          mk_tparam pos name tp_constraints)

    let of_typeconst_analysis analysis this_name this_ty env =
      let (this_subst, class_subst, generics) =
        let names = SMap.empty and generics = SMap.empty in
        let (this_subst, names, generics) =
          this_subst analysis names generics
        in
        let (class_subst, _names, generics) =
          class_subst analysis names generics
        in
        (this_subst, class_subst, generics)
      in
      let tparams = mk_tparams generics env in
      let subst = { this_name; this_ty; this_subst; class_subst; tparams } in
      (* Apply refinements to [this] to ensure it lines up with any abstract constants *)
      let this_ty = refine_this this_ty this_subst (this_constants analysis) in
      { subst with this_ty }
  end

  let to_subst t this_ty env =
    let { this_name; _ } = t in
    Subst.of_typeconst_analysis t this_name this_ty env
end

(* -- Helpers --------------------------------------------------------------- *)
let ty_generics names ty =
  let acc = ref names in
  let on_ty decl_ty ~ctx =
    let open Typing_defs_core in
    match get_node decl_ty with
    | Tgeneric nm ->
      let () = acc := SSet.add nm !acc in
      (ctx, `Stop decl_ty)
    | _ -> (ctx, `Continue decl_ty)
  in
  let _ = Typing_defs_core.transform_top_down_decl_ty ty ~ctx:() ~f:on_ty in
  !acc

let drop_unused_generics fun_ty ~names =
  let open Typing_defs_core in
  let {
    ft_tparams;
    ft_params;
    ft_ret;
    ft_implicit_params = { capability };
    ft_where_constraints;
    _;
  } =
    fun_ty
  in
  (* First find any generics mentioned in parameters or in the return type *)
  let names =
    let names = ty_generics names ft_ret in
    let names =
      match capability with
      | CapTy ty -> ty_generics names ty
      | CapDefaults _ -> names
    in
    let names =
      List.fold_left ft_params ~init:names ~f:(fun names ft_param ->
          ty_generics names ft_param.fp_type)
    in
    (* We have to include where constraints to cover legacy projection from
       generics *)
    List.fold_left
      ft_where_constraints
      ~init:names
      ~f:(fun names (ty1, _, ty2) -> ty_generics (ty_generics names ty1) ty2)
  in
  (* Now add in any generics which are mentioned in constraints in those
     generics; repeat until we have discovered all transitively used generics *)
  let rec aux (acc, delta) =
    let acc = SSet.union delta acc in
    let delta =
      List.fold_left
        ~init:SSet.empty
        ft_tparams
        ~f:(fun acc { tp_name = (_, name); tp_constraints; _ } ->
          if SSet.mem name delta then
            let generics =
              List.fold_left tp_constraints ~init:acc ~f:(fun acc (_, ty) ->
                  ty_generics acc ty)
            in
            (* If the generic has an upper or lower bound which mentions the
               generic we would end up in an infinite loop *)
            let generics = SSet.remove name generics in
            SSet.union acc generics
          else
            acc)
    in
    let delta = SSet.diff delta acc in
    if SSet.is_empty delta then
      acc
    else
      aux (acc, delta)
  in
  let names = aux (SSet.empty, names) in
  let ft_tparams =
    List.filter ft_tparams ~f:(fun { tp_name = (_, name); _ } ->
        SSet.mem name names)
  in
  { fun_ty with ft_tparams }

(** Find a nice name for a generic corresponding to [this] for polymorphic
    function pointers *)
let gen_this_name ft_tparams =
  let names =
    SSet.of_list
      (List.map ft_tparams ~f:(fun Typing_defs_core.{ tp_name = (_, nm); _ } ->
           nm))
  in
  let base = "Tthis" in
  let rec aux n_opt =
    let name =
      Option.value_map n_opt ~default:base ~f:(fun n ->
          Format.sprintf "%s#%n" base n)
    in
    if SSet.mem name names then
      aux (Option.value_map n_opt ~default:(Some 0) ~f:(fun n -> Some (n + 1)))
    else
      name
  in
  aux None

(* -- API ------------------------------------------------------------------- *)
let extract_static_method fun_ty ~class_name ~folded_class ~env =
  (* We will need to handle substitution of [this] differently depending on
     the variance of its appearance in the function type:
     - If occurs co- or contravariantly we can substitute it with the the
       class itself or introduce a new type parameter with a lower or upper bound
       respectively. We prefer the former since instantiated types can take
       advantage of bidirectional typing
     - If it occurs invariantly we want to introduce a new type parameter so
       that function pointers are polymorphic in the type of [this] unless
       the class is final. In this case we can substitute [this] with the class
  *)
  let this_variance =
    if Folded_class.final folded_class then
      (* There is no point being polymorphic in [this] if it corresponds to a
         final class *)
      None
    else
      This_variance.of_fun_ty fun_ty ~env
  in

  (* Record the list of type parameter appearing in the original function type;
     we need this since even though a parameter may be unused it may have
     reify directives which should be preserved *)
  let original_tparam_names =
    let open Typing_defs_core in
    let names =
      List.map ~f:(fun { tp_name = (_, name); _ } -> name) fun_ty.ft_tparams
    in
    SSet.of_list names
  in

  (* Get the class level generics; we need this to build a type for [this] and
     to add to the function generics *)
  let class_tparams = Folded_class.tparams folded_class in

  (* Build the type corresponding to [this] or its upper bound *)
  let this_ty =
    Typing_defs_core.(
      mk
        ( Typing_reason.witness_from_decl (fst class_name),
          Tapply
            ( class_name,
              List.map
                class_tparams
                ~f:(fun Typing_defs_core.{ tp_name = (pos, name); _ } ->
                  Typing_defs_core.(
                    mk (Typing_reason.witness_from_decl pos, Tgeneric name))) )
        ))
  in

  (* Analyse the function type to determine which type constants have to
     be quantified over and the definition of concrete type constants *)
  let analysis =
    Typeconst_analysis.of_fun_type fun_ty (snd class_name) folded_class env
  in

  (* Generate the type parameters corresponding to abstract type constants, a
     substitution from to be applied to [Taccess] types and a refined version
     of [this] with equalities to generics standing in for abstract type
     constants *)
  let subst = Typeconst_analysis.to_subst analysis this_ty env in

  (* Add class-level generics, the generics for [this] and any generics
     for abstract type constants; some of these may end up not being used
     but we will clear that up later *)
  let fun_ty =
    let open Typing_defs_core in
    let { ft_tparams; _ } = fun_ty in
    let Typeconst_analysis.Subst.{ tparams; _ } = subst in
    let ft_tparams = class_tparams @ tparams @ ft_tparams in
    { fun_ty with ft_tparams }
  in

  (* At this point the 'this_ty' in the substitution is refined but may still
     contain occurrences of [this]. Depending on the variance of this in the
     function type we need to replace this with the generic it will be the
     upper bound of (invariant case) or the unrefined version of 'this_ty'.
     In the former case, we need to add the type parameter to function type *)
  let (fun_ty, subst) =
    match this_variance with
    | Some Ast_defs.Invariant ->
      let Typeconst_analysis.Subst.{ this_ty = refined_this_ty; _ } = subst in
      let Typing_defs_core.{ ft_tparams; _ } = fun_ty in
      let this_generic_name = gen_this_name ft_tparams in
      let this_generic_ty =
        Typing_defs_core.(
          mk
            ( Typing_reason.witness_from_decl (fst class_name),
              Tgeneric this_generic_name ))
      in
      let subst = { subst with this_ty = this_generic_ty } in
      (* Replace occurrences of this in the upperbound with the generic *)
      let this_ty_upper_bound =
        Typeconst_analysis.Subst.apply subst refined_this_ty
      in
      (* Add the generic to the function type *)
      let this_tparam =
        mk_tparam
          (fst class_name)
          this_generic_name
          [(Ast_defs.Constraint_as, this_ty_upper_bound)]
      in
      let ft_tparams = this_tparam :: ft_tparams in
      (Typing_defs_core.{ fun_ty with ft_tparams }, subst)
    | _ ->
      let Typeconst_analysis.Subst.{ this_ty = refined_this_ty; _ } = subst in
      let this_ty =
        Typeconst_analysis.Subst.(apply { subst with this_ty } refined_this_ty)
      in
      let subst = Typeconst_analysis.Subst.{ subst with this_ty } in
      (fun_ty, subst)
  in

  (* Apply a subsitution replacing
     - [Tthis] with either a type parameter or the refined class
     - any concrete [Taccess] with its definition (except where its definition is abstract...)
     - any abstract [Taccess] with the corresponding generic *)
  let fun_ty = Typeconst_analysis.Subst.apply_fun_ty subst fun_ty in

  (* We may have introduced some unused generics; check which ones appear
     in parameters, implicit parameters or the return type and then find the
     transitive closure. Any generics not in this set can be dropped.
     Note: the original set of generic is used as the starting point since
     we need to keep the original type params if they are marked as reify *)
  let fun_ty = drop_unused_generics fun_ty ~names:original_tparam_names in

  fun_ty
