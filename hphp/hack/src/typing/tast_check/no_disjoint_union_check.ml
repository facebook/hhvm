open Hh_prelude
module SN = Naming_special_names

let is_no_union Typing_defs.{ ua_name = (_, id); _ } =
  String.equal id SN.UserAttributes.uaNoDisjointUnion

(** Chek for pairwise disjointness of types. *)
let rec is_pairwise_disjoint env = function
  | []
  | [_] ->
    true
  | ty :: tyl ->
    List.for_all
      ~f:(fun ty' ->
        match Tast_env.is_disjoint ~is_dynamic_call:true env ty ty' with
        | Tast_env.Disjoint
        | Tast_env.DisjointIgnoringDynamic _ ->
          true
        | Tast_env.NonDisjoint -> false)
      tyl
    && is_pairwise_disjoint env tyl

let rec tyl_of_union env ty =
  let ty = Tast_env.strip_dynamic env ty in
  let (_, ty) = Tast_env.strip_supportdyn env ty in
  match Typing_defs.get_node ty with
  | Typing_defs.Tunion tyl -> Some tyl
  | Typing_defs.Toption ty -> tyl_of_union env ty
  | _ -> None

(** Looks for pairwise disjointness of a union in a type argument if the
    corresponding type parameter is annotated with __NoDisjointUnion. If found,
    we raise a warning. *)
let check pos env tparams targs =
  match List.zip targs tparams with
  | List.Or_unequal_lengths.Ok zipped ->
    List.iter zipped ~f:(fun ((ty, _), tparam) ->
        let user_attributes = tparam.Typing_defs.tp_user_attributes in
        if List.exists user_attributes ~f:is_no_union then begin
          let ty = Tast_env.strip_dynamic env ty in
          let (_, ty) = Tast_env.strip_supportdyn env ty in
          match tyl_of_union env ty with
          | Some ([] | [_])
          | None ->
            ()
          | Some tyl -> begin
            if is_pairwise_disjoint env tyl then
              let reason_of_ty ty =
                ( Typing_reason.to_pos @@ Typing_defs.get_reason ty,
                  Tast_env.print_ty env ty )
              in
              let disjuncts =
                lazy
                  (List.map ~f:reason_of_ty tyl
                  |> List.dedup_and_sort ~compare:(fun (p, _) (p', _) ->
                         Pos_or_decl.compare p p'))
              in
              let tparam_pos = fst tparam.Typing_defs.tp_name in
              Typing_warning_utils.add
                (Tast_env.tast_env_as_typing_env env)
                ( pos,
                  Typing_warning.No_disjoint_union_check,
                  Typing_warning.No_disjoint_union_check.
                    { disjuncts; tparam_pos } )
          end
        end)
  | List.Or_unequal_lengths.Unequal_lengths -> ()

(** Flag violations on __NoDisjointUnion on call and new expressions. *)
let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_expr env =
      function
      | (_, p, Aast_defs.(Call { func = (fun_ty, _, _); targs; _ })) ->
        let (_, fun_ty) = Tast_env.strip_supportdyn env fun_ty in
        begin
          match Typing_defs.get_node fun_ty with
          | Typing_defs.(Tfun { ft_tparams; _ }) -> check p env ft_tparams targs
          | _ -> ()
        end
      | (_, p, Aast_defs.(New ((_, _, CI (_, cid)), targs, _, _, _))) -> begin
        match Tast_env.get_class env cid with
        | Decl_entry.Found elt ->
          let tparams = Folded_class.tparams elt in
          check p env tparams targs
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          ()
      end
      | _ -> ()
  end
