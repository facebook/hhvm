(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SN = Naming_special_names
module T = Typing_defs

(* The primary guarantee provided with this check:

    In the absence of unsoundness, `switch` statements will not throw a
    `RuntimeException` because the implicit default case is hit. The check
    either proves that all cases are covered exhaustively or insist on a
    default.

   The secondary utility of this check:

     It detects cases in switch statements that will not be hit/needed. These
     are surfaced as warnings rather than errors and are best effort.
*)

let is_nothing env ty =
  Tast_env.is_sub_type env ty (Typing_make_type.nothing Typing_reason.none)

let is_null env ty =
  Tast_env.is_sub_type env ty (Typing_make_type.null Typing_reason.none)

(** If we have a type `((A & B) | C)` or `(C | (A & B))`,
    normalise it to `((C | A) & (C | B))`.

    We do this because pessimisation might lead to types such as
    `(arraykey & dynamic) | E` from which we cannot strip the like type and
    consequently cannot check for exhaustivity. The transposition gives us
    `(arraykey | E) & (dynamic | E)` to which the like type stripping can apply
    and hence we can check for exhaustivity, i.e., by covering all values of
    `E`. *)
let transpose_union_of_intersection env ty =
  let transpose_disjunct_binary ty1 ty2 =
    let (env, ty1) = Tast_env.expand_type env ty1 in
    let (_, ty1) = Tast_env.strip_supportdyn env ty1 in
    let (_env, ty2) = Tast_env.expand_type env ty2 in
    let (_, ty2) = Tast_env.strip_supportdyn env ty2 in
    let components =
      match (T.get_node ty1, T.get_node ty2) with
      | (T.Tintersection tyl, _) -> Some (tyl, ty2)
      | (_, T.Tintersection tyl) -> Some (tyl, ty1)
      | (_, _) -> None
    in
    begin
      match components with
      | Some (conjuncts, ty) ->
        let conjuncts =
          List.map conjuncts ~f:(fun conjunct ->
              Typing_make_type.union (T.get_reason conjunct) [conjunct; ty])
        in
        Typing_make_type.intersection (T.get_reason ty) conjuncts
      | None -> Typing_make_type.union (T.get_reason ty1) [ty1; ty2]
    end
  in
  let rec transpose_disjunct_list = function
    | [] -> Typing_make_type.union (T.get_reason ty) []
    | [ty] -> ty
    | [ty1; ty2] -> transpose_disjunct_binary ty1 ty2
    | ty1 :: tyl ->
      let ty2 = transpose_disjunct_list tyl in
      transpose_disjunct_binary ty1 ty2
  in
  match T.get_node ty with
  | T.Tunion tyl -> transpose_disjunct_list tyl
  | _ -> ty

(** Values that are supported in the context of exhaustivity checking. *)
module Value = struct
  type t =
    | True
    | False
    | Null
    | Int of string  (** Only used for duplicate case detection *)
    | String of string  (** Only used for duplicate case detection *)
    | Id of string  (** Only used for duplicate case detection *)
    | EnumConstant of string * string
    | EnumClassLabel of string
    | Tuple of t list
  [@@deriving ord]

  let rec show = function
    | True -> "true"
    | False -> "false"
    | Null -> "null"
    | Int str
    | String str
    | Id str ->
      str
    | EnumConstant (enum, label) ->
      Format.sprintf "%s::%s" (Utils.strip_ns enum) label
    | EnumClassLabel label -> Format.sprintf "#%s" label
    | Tuple values ->
      List.map values ~f:show
      |> String.concat ~sep:", "
      |> Format.sprintf "(%s)"

  let rec of_ast (_, _, expr) : t option =
    let open Option.Let_syntax in
    match expr with
    | Aast.True -> Some True
    | Aast.False -> Some False
    | Aast.Null -> Some Null
    | Aast.Int str -> Some (Int str)
    | Aast.String str -> Some (String str)
    | Aast.Id (_, str) -> Some (Id str)
    | Aast.Class_const ((_, _, Aast.CI (_, enum_id)), (_, label)) ->
      (* Don't worry if the class constant is not an enum, we'll use this as the
         second operand to set diference and the first operand only. *)
      Some (EnumConstant (enum_id, label))
    | Aast.EnumClassLabel (_, label) -> Some (EnumClassLabel label)
    | Aast.Tuple exprs ->
      let* vals = List.map ~f:of_ast exprs |> Option.all in
      Some (Tuple vals)
    | _ -> None
end

module ValueWithPos = struct
  type t = Pos.t * Value.t

  (** We want different cases with same contents to comapre equal and that is
      only possible if we disregard their positions. *)
  let compare (_, v1) (_, v2) = Value.compare v1 v2

  let of_value v = (Pos.none, v)
end

module ValueSet = struct
  include Stdlib.Set.Make (ValueWithPos)

  (** Turn a list of cases into a set of duplicate cases and a value set
      representation of the cases that are recognised. *)
  let of_case_list cases : (Pos.t * Pos.t * Value.t) list * t =
    List.fold
      cases
      ~init:([], empty)
      ~f:(fun (duplicates, values) (((_, pos, _) as label), _) ->
        match Value.of_ast label with
        | Some value ->
          let pval = (pos, value) in
          begin
            match find_opt pval values with
            | Some (first_occurrence, _) ->
              ((first_occurrence, pos, value) :: duplicates, values)
            | None -> (duplicates, add pval values)
          end
        | None -> (duplicates, values))
end

module AlternativeSet = struct
  (** Disjunctive-normal form for set of values we can satisfy to make the
      switch statement exhaustive. Each value set is one way of satisfying
      switch exhaustivity.

      Example: for enums E and F, switching over `E & F` can be exhaustive by
      covering all cases of `E` or all cases of `F`. *)
  type t = ValueSet.t list

  (** This helper handles vanilla enums and enum classes (`HH\EnumClass\Label`
      and `HH\MemberOf`).

      For enums and enum classes referenced through `HH\MemberOf` we are
      interested in constant cases. For `HH\EnumClass\Label` we are interested
      in label cases.

      For enum classes, the `bound` parameter is set and is used to eliminate
      cases don't need to be covered, e.g., `HH\MemberOf<E, int>` need not
      consider enum class values that has type string. *)
  let of_enum_name ~is_enum_class_label ~bound env name =
    let (bound_is_nothing, bound) =
      match bound with
      | Some bound ->
        let like_bound =
          Typing_utils.make_like (Tast_env.tast_env_as_typing_env env) bound
        in
        (is_nothing env bound, Some like_bound)
      | None -> (false, None)
    in
    if bound_is_nothing then
      (* If we have `HH\MemberOf<Foo, nothing>` or
         `HH\EnumClass\Label<Foo, nothing>` or similar, there won't be any cases
         that needs matching. *)
      (env, Some [ValueSet.empty])
    else
      match Tast_env.get_enum env name with
      | Decl_entry.DoesNotExist
      | Decl_entry.NotYetAvailable ->
        (env, None)
      | Decl_entry.Found decl ->
        let of_const ~bound (const, decl) =
          let value_of_const =
            (* If we are scrutinising `HH\EnumClass\Label` then we need to match
               a label value, otherwise (`HH\MemberOf` or vanilla enum) we need an
               enum constant. *)
            lazy
              (if is_enum_class_label then
                Some (Value.EnumClassLabel const)
              else
                Some (Value.EnumConstant (name, const)))
          in
          if String.equal const SN.Members.mClass then
            (* Every class has `class` constant defined and since enums are
               represented as classes, so do they. Skip those. *)
            None
          else begin
            Option.map ~f:ValueWithPos.of_value
            @@
            match bound with
            | Some bound ->
              let const_ty =
                match T.get_node decl.T.cc_type with
                | T.Tapply ((pos, name), args)
                  when is_enum_class_label
                       && String.equal name SN.Classes.cMemberOf ->
                  (* Enum class constants are typed with `HH\MemberOf`,
                     but we have an enum class label at hand.
                     Construct the corresponding `HH\EnumClass\Label`
                     so that subtype based filtering works. *)
                  T.mk
                    ( T.get_reason decl.T.cc_type,
                      T.Tapply ((pos, SN.Classes.cEnumClassLabel), args) )
                | _ -> decl.T.cc_type
              in
              (* If there is a bound (we're dealing with an enum class), filter the case out *)
              let (_env, const_ty) =
                Tast_env.localize env T.empty_expand_env const_ty
              in
              if Tast_env.is_sub_type env const_ty bound then
                Lazy.force value_of_const
              else
                None
            | None -> Lazy.force value_of_const
          end
        in
        let consts = Folded_class.consts decl in
        let values = List.filter_map ~f:(of_const ~bound) consts in
        let values =
          (* If there are no values in the presence of a bound, it is possible
             that we eliminated everything which shouldn't happen. Instead of
             requiring a default from the get go, retry without a bound which would
             treat exhaustivity checking of enum class as if it was an enum and
             require covering every value. *)
          if Option.is_some bound && List.is_empty values then
            List.filter_map ~f:(of_const ~bound:None) consts
          else
            values
        in
        (env, Some [ValueSet.of_list values])

  (** Helper to shallowly expand types found in TAST so that we can recurse and
      scrutenise types. It strips like types and specially treats `~nothing` and
      `~null`. *)
  let expand_type env ty =
    let (env, ty) = Tast_env.expand_type env ty in
    match T.get_node ty with
    | T.Tunion tyl ->
      let (env, tyl) = List.fold_map ~init:env tyl ~f:Tast_env.expand_type in
      let (dynamic_tyl, other_tyl) =
        List.partition_tf tyl ~f:(fun ty ->
            match T.get_node ty with
            | T.Tdynamic -> true
            | _ -> false)
      in
      let r = T.get_reason ty in
      let ty =
        match dynamic_tyl with
        | [] ->
          (* We expanded tyl, let's not throw it away and reconstruct the union. *)
          Typing_make_type.union r tyl
        | _ :: _ -> begin
          match other_tyl with
          | [ty] when is_nothing env ty || is_null env ty ->
            (* Treat `~nothing` and `~null` as `dynamic . This is where writing
               a check is more of an art than science.

               `~nothing` is morally equivalent to `dynamic` and we want it to
               be treated as such.

               As for `~null`, although technically we should treat this the
               same as we treat other like types, almost always gets produced
               due to an operator such as `idx` or `??` through the default
               branch and it is more useful to ask for a default which would
               likely coincide with the null case anyway. *)
            Typing_make_type.dynamic r
          | _ ->
            (* Reconstruct the union without the like types *)
            Typing_make_type.union r other_tyl
        end
      in
      (env, ty)
    | _ -> (env, ty)

  (** If the type can be exhaustively checked returns a set of alternative
      values to satisfy.

      None corresponds to the universal set which we use for non-enumerable
      types such as `string` but also for enumerable ones that we do not
      support, e.g., shape of enumerable values.

      The universal set is useful because if we have a `E & string`, we can
      satisfy exhaustivity by enumerating `E` despite `string` leading to the
      universal value set (i.e., `None`).  *)
  let rec of_ty env ty : Tast_env.t * t option =
    let open Option.Let_syntax in
    let (env, ty) = expand_type env ty in
    let ty = transpose_union_of_intersection env ty in
    match T.get_node ty with
    | T.Tprim Aast.Tbool ->
      ( env,
        Some
          [
            ValueSet.of_list
            @@ List.map ~f:ValueWithPos.of_value Value.[True; False];
          ] )
    | T.Tprim Aast.Tnull ->
      (env, Some [ValueSet.singleton @@ ValueWithPos.of_value Value.Null])
    | T.Toption ty ->
      (* ?ty is exhaustively covered if `null` is covered and all values of `ty`
         are covered. *)
      let (env, disjuncts) = of_ty env ty in
      let value_set =
        let* disjuncts = disjuncts in
        Some
          (List.map
             disjuncts
             ~f:(ValueSet.add @@ ValueWithPos.of_value Value.Null))
      in
      (env, value_set)
    | T.Tnewtype (name, [enum_class; _], _)
      when List.mem
             ~equal:String.equal
             [SN.Classes.cMemberOf; SN.Classes.cEnumClassLabel]
             name -> begin
      (* This case covers switching on enum classes through `HH\EnumClass\Label`
         and `HH\MemberOf`. *)
      let (env, enum_class) = Tast_env.expand_type env enum_class in
      match T.get_node enum_class with
      | T.Tclass ((_, enum_class_name), _, _) ->
        of_enum_name
          ~is_enum_class_label:(String.equal name SN.Classes.cEnumClassLabel)
          ~bound:(Some ty)
          env
          enum_class_name
      | _ -> (env, None)
    end
    | T.Tnewtype (enum_name, _, _) when Tast_env.is_enum env enum_name ->
      (* An enum E is exhaustively switched if there is a case for all of its
         declared values (including inherited ones). *)
      of_enum_name ~is_enum_class_label:false ~bound:None env enum_name
    | T.Ttuple
        T.{ t_required; t_extra = Textra { t_optional = []; t_variadic } }
      when is_nothing env t_variadic ->
      (* We only consider closed tuples for exhaustivity. *)
      (* A tuple (E, F) can be satisfied by tuples formed by the cartesian
         product of all values in E and F. *)
      let (env, dnfs) = List.fold_map ~init:env t_required ~f:of_ty in
      let dnf =
        let* dnfs = Option.all dnfs in
        Some
          (List.Cartesian_product.all dnfs
          |> List.map ~f:(fun vsl ->
                 List.map ~f:ValueSet.to_list vsl
                 |> List.Cartesian_product.all
                 |> List.map ~f:(fun tyl ->
                        ValueWithPos.of_value
                        @@ Value.Tuple (List.map ~f:snd tyl))
                 |> ValueSet.of_list))
      in
      (env, dnf)
    | T.Tunion tyl ->
      (* A switch on E | F must satisfy values of both E and F to be exhaustive. *)
      let (env, dnfs) = List.fold_map ~init:env tyl ~f:of_ty in
      let dnf =
        let* dnfs = dnfs |> Option.all in
        Some
          (List.Cartesian_product.all dnfs
          |> List.map ~f:(List.fold ~init:ValueSet.empty ~f:ValueSet.union))
      in
      (env, dnf)
    | T.Tintersection tyl ->
      (* A switch on E & F can satisfy values of either E or F to be exhaustive. *)
      let (env, dnfs) = List.fold_map tyl ~init:env ~f:of_ty in
      let dnfs = List.filter_map ~f:Fn.id dnfs in
      (* If none of the conjuncts can be made exhaustive, neither can the
         intersection type. *)
      let dnf =
        if List.is_empty dnfs then
          None
        else
          Some (List.concat dnfs)
      in
      (env, dnf)
    | _ -> (env, None)

  type result =
    | Satisfiable of { model: ValueSet.t }
    | Unsatisfiable of { missing: ValueSet.t Lazy.t list }

  (** Given a set of cases we observed and possible value sets that can be
      satisfied to make the siwtch exhaustive, decides if any of the
      alternatives are covered. If so returns a model (the set of values that
      were satisfied), otherwise, return the set of alternative values that can
      be covered to satisfy ehxaustivity. *)
  let satisfies (seen_values : ValueSet.t) (alternatives : t) : result =
    let satisfiable_alternative =
      List.find alternatives ~f:(fun values_to_exhaust ->
          ValueSet.is_empty (ValueSet.diff values_to_exhaust seen_values))
    in
    match satisfiable_alternative with
    | Some values_to_exhaust -> Satisfiable { model = values_to_exhaust }
    | None ->
      let alternative_missing_sets =
        List.map alternatives ~f:(fun values_to_exhaust ->
            lazy (ValueSet.diff values_to_exhaust seen_values))
      in
      Unsatisfiable { missing = alternative_missing_sets }
end

let pick_n_values n values =
  ValueSet.to_list values
  |> (fun vals -> List.take vals n)
  |> List.map ~f:(fun (_, v) -> Value.show v |> Utils.strip_ns)

let check_exhaustiveness env ~switch_pos ~scrutinee_pos ty case_list default =
  let scrutinee_type = lazy (Tast_env.print_ty env ty) in
  let (_env, alternative_set) = AlternativeSet.of_ty env ty in
  let (duplicates, seen_values) = ValueSet.of_case_list case_list in
  (* If there are identical cases, report duplicate case errors. One error for
     each duplicate. *)
  List.iter duplicates ~f:(fun (first_occurrence, second_occurrence, case) ->
      let case = lazy (Utils.strip_ns @@ Value.show case) in
      Tast_env.add_warning
        env
        Typing_warning.
          ( second_occurrence,
            Switch_redundancy,
            Switch_redundancy.DuplicatedCase { first_occurrence; case } ));
  match alternative_set with
  | Some alternative_set -> begin
    match AlternativeSet.satisfies seen_values alternative_set with
    | AlternativeSet.Satisfiable { model } ->
      begin
        match default with
        | Some (default_pos, _) ->
          (* The switch is exhaustive but has a default, report an error for
             dead code. *)
          Tast_env.add_warning
            env
            Typing_warning.
              ( default_pos,
                Switch_redundancy,
                Switch_redundancy.RedundantDefault )
        | None -> ()
      end;
      let seen_size = List.length case_list in
      let model_size = ValueSet.cardinal model in
      if model_size < seen_size then
        (* There must be some redundancies since the values needed to satisfy
           exhaustivity is smaller than there are cases. *)
        let positions =
          lazy
            ( Pos.Set.diff
                (List.map ~f:(fun ((_, p, _), _) -> p) case_list
                |> Pos.Set.of_list)
                (ValueSet.to_list (ValueSet.inter seen_values model)
                |> List.map ~f:fst
                |> Pos.Set.of_list)
            |> Pos.Set.elements
            |> fun els -> List.take els 10 )
        in

        Tast_env.add_warning
          env
          Typing_warning.
            ( switch_pos,
              Switch_redundancy,
              Switch_redundancy.SwitchHasRedundancy
                { positions; redundancy_size = seen_size - model_size } )
    | AlternativeSet.Unsatisfiable { missing } -> begin
      match default with
      | None ->
        List.iter missing ~f:(fun missing_values ->
            let missing =
              lazy (pick_n_values 10 @@ Lazy.force missing_values)
            in
            (* The values do not satisfy any of the value sets. For each
               alternative value set, emit an exhaustivity error reporting what
               values need to be provided to make the switch exhaustive. *)
            Tast_env.add_typing_error
              ~env
              Typing_error.(
                switch
                @@ Primary.Switch.Switch_nonexhaustive
                     { switch_pos; scrutinee_pos; scrutinee_type; missing }))
      | Some _ -> ()
    end
  end
  | None -> begin
    match default with
    | None ->
      (* We can't check the type for exhaustivity and there is no default, require
         one. *)
      Tast_env.add_typing_error
        ~env
        Typing_error.(
          switch
          @@ Primary.Switch.Switch_needs_default
               { switch_pos; scrutinee_pos; scrutinee_type })
    | Some _ -> ()
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt env (st_pos, st) =
      match st with
      | Aast.Switch ((scrutinee_ty, scrutinee_pos, _), case_list, default) ->
        check_exhaustiveness
          ~switch_pos:st_pos
          ~scrutinee_pos
          env
          scrutinee_ty
          case_list
          default
      | _ -> ()
  end
