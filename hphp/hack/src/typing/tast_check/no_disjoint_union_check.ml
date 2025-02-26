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

(** Looks for pairwise disjointness of a union in a type argument if the
    corresponding type parameter is annotated with __NoDisjointUnion. If found,
    we raise a warning. *)
let check pos env user_attributesl targs =
  match List.zip targs user_attributesl with
  | List.Or_unequal_lengths.Ok zipped ->
    List.iter zipped ~f:(fun ((ty, _), user_attributes) ->
        if List.exists user_attributes ~f:is_no_union then begin
          let ty = Tast_env.strip_dynamic env ty in
          let (_, ty) = Tast_env.strip_supportdyn env ty in
          match Typing_defs.get_node ty with
          | Typing_defs.Tunion ([] | [_]) -> ()
          | Typing_defs.Tunion tyl -> begin
            if is_pairwise_disjoint env tyl then
              Typing_warning_utils.add
                (Tast_env.tast_env_as_typing_env env)
                ( pos,
                  Typing_warning.No_disjoint_union_check,
                  lazy (Tast_env.print_ty env ty) )
          end
          | _ -> ()
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
          | Typing_defs.(Tfun { ft_tparams; _ }) ->
            let user_attributesl =
              List.map
                ~f:(fun tparam -> tparam.Typing_defs.tp_user_attributes)
                ft_tparams
            in
            check p env user_attributesl targs
          | _ -> ()
        end
      | (_, p, Aast_defs.(New ((_, _, CI (_, cid)), targs, _, _, _))) -> begin
        match Tast_env.get_class env cid with
        | Decl_entry.Found elt ->
          let tparams = Folded_class.tparams elt in
          let user_attributesl =
            List.map
              ~f:(fun tparam -> tparam.Typing_defs.tp_user_attributes)
              tparams
          in
          check p env user_attributesl targs
        | Decl_entry.DoesNotExist
        | Decl_entry.NotYetAvailable ->
          ()
      end
      | _ -> ()
  end
