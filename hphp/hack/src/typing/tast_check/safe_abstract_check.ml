(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* Implement the checks from the Safe Abstract proposal,
   * which makes the combination of "abstract" and "static" safer
   * (https://fburl.com/hack-safe-abstract) *)
open Hh_prelude

type abstractness =
  | Concrete
  | Abstract  (** Statically known to be abstract *)
  | Maybe_abstract  (** May be either abstract or concrete at runtime *)

module Class_use : sig
  type t = {
    class_: Folded_class.t;
    abstractness: abstractness;
        (** Refers to what we know about the class at the *use* site.
        This information is distinct from what `Folded_class.abstract class_` provides *)
  }

  val get :
    Tast_env.env ->
    (Typing_defs.locl_ty, 'a) Tast.class_id_ ->
    current_method:Tast.method_ option ->
    t option
end = struct
  type t = {
    class_: Folded_class.t;
    abstractness: abstractness;
  }

  let static_keyword_refers_to_concrete_class current_method =
    match current_method with
    | None -> false
    | Some method_ -> not method_.Aast_defs.m_static
  (* TODO:  also return `true` when the current method has the <<__NeedsConcrete>> attribute *)

  let get env class_id ~(current_method : _ Aast_defs.method_ option) : t option
      =
    let open Option.Let_syntax in
    let get_class class_name =
      Tast_env.get_class env class_name |> Decl_entry.to_option
    in
    let abstractness_of_folded_class class_ =
      if Folded_class.abstract class_ then
        Abstract
      else
        Concrete
    in
    match class_id with
    | Aast.CIstatic ->
      let* class_name = Tast_env.get_self_id env in
      let+ class_ = get_class class_name in
      let abstractness =
        if static_keyword_refers_to_concrete_class current_method then
          Concrete
        else
          Maybe_abstract
      in
      { class_; abstractness }
    | Aast.CIparent ->
      let* class_name = Tast_env.get_parent_id env in
      let+ class_ = get_class class_name in
      { class_; abstractness = abstractness_of_folded_class class_ }
    | Aast.CIself ->
      let* class_name = Tast_env.get_self_id env in
      let+ class_ = get_class class_name in
      { class_; abstractness = abstractness_of_folded_class class_ }
    | Aast.CI (_, class_name) ->
      let+ class_ = get_class class_name in
      { class_; abstractness = abstractness_of_folded_class class_ }
    | Aast.CIexpr (ty, _, _) ->
      (* extract from classname<T> or concrete_classname<T> *)
      let from_targ ty_arg abstractness : t option =
        match snd @@ Typing_defs_core.deref ty_arg with
        | Typing_defs.Tclass ((_, class_name), _exact, _targs) ->
          let+ class_ = get_class class_name in
          { class_; abstractness }
        | _ -> None
      in
      begin
        match snd @@ Typing_defs_core.deref ty with
        | Typing_defs.Tnewtype (new_type, [hd_targ], _)
          when String.equal new_type Naming_special_names.Classes.cClassname ->
          from_targ hd_targ Maybe_abstract
        | Typing_defs.Tnewtype (new_type, hd_targ :: [], _)
          when String.equal
                 new_type
                 Naming_special_names.Classes.cConcreteclassname ->
          from_targ hd_targ Concrete
        | _ -> None
      end
end

let warn env pos (abstract_static_warning : Typing_warning.Safe_abstract.t) :
    unit =
  Typing_warning_utils.add
    (Tast_env.tast_env_as_typing_env env)
    (pos, Typing_warning.Safe_abstract, abstract_static_warning)

let check_for_call_abstract
    Class_use.{ class_; abstractness } env pos method_ folded_method : unit =
  let method_may_be_abstract =
    match abstractness with
    | Abstract
    | Maybe_abstract ->
      Typing_defs.get_ce_abstract folded_method
    | Concrete -> false
  in
  let call_warning_would_be_redundant =
    match abstractness with
    | Abstract -> true
    | Maybe_abstract
    | Concrete ->
      false
  in
  if method_may_be_abstract && not call_warning_would_be_redundant then
    warn
      env
      pos
      Typing_warning.Safe_abstract.
        { kind = Call_abstract { method_ }; class_ = Folded_class.name class_ }

let check_for_new_abstract Class_use.{ class_; abstractness } env new_pos =
  let is_final_class = Folded_class.final class_ in
  let receiver_may_be_abstract =
    match abstractness with
    | Abstract
    | Maybe_abstract ->
      true
    | Concrete -> false
  in
  let warning_would_be_redundant =
    match abstractness with
    | Abstract -> is_final_class
    | Maybe_abstract
    | Concrete ->
      false
  in
  if receiver_may_be_abstract && not warning_would_be_redundant then
    warn
      env
      new_pos
      Typing_warning.Safe_abstract.
        { kind = New_abstract; class_ = Folded_class.name class_ }

let handler =
  let current_method = ref None in

  object
    inherit Tast_visitor.handler_base

    method! at_fun_def _env _fun_def = current_method := None

    method! at_method_ _env m = current_method := Some m

    method! at_expr env expr =
      match expr with
      | Aast.
          ( _,
            call_pos,
            Call
              { func = (_, _, Class_const ((_, _, class_id), (_, method_))); _ }
          ) ->
        let open Option.Let_syntax in
        Option.value ~default:()
        @@ let* class_use =
             Class_use.get env class_id ~current_method:!current_method
           in
           let+ folded_method =
             Folded_class.get_smethod class_use.Class_use.class_ method_
           in
           check_for_call_abstract class_use env call_pos method_ folded_method
      | Aast.
          ( _,
            new_pos,
            New ((_, _, class_id), _targs, _exprs, _expr, _constructor) ) ->
        Option.iter
          (Class_use.get env class_id ~current_method:!current_method)
          ~f:(fun class_use -> check_for_new_abstract class_use env new_pos)
      | _ -> ()
  end
