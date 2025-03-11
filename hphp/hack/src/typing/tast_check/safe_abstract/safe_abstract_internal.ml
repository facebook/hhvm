(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

type class_use_kind =
  | Static_method_call
  | New
  | Const_access

type class_use_info =
  class_use_kind * Pos.t * Typing_warning.Safe_abstract.t list

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
    reason: Typing_reason.t option;
        (** The reason for the type, if the class information came from a type.
    * For example: in `new $class()` the reason represents provenance of the type of `$class` *)
  }

  (**
  Useful for generating warnings on a use of a class.
  In simple cases, this is equivalent to returning just a `t`,
  however, we need to traverse intersections and unions.
   *)
  val fold :
    'acc.
    Tast_env.env ->
    ( Typing_reason.locl_phase Typing_defs_core.ty,
      Tast.saved_env )
    Aast_defs.class_id_ ->
    current_method:Tast.method_ option ->
    f:(t -> 'acc) ->
    intersect:('acc list -> 'acc) ->
    union:('acc list -> 'acc) ->
    'acc option
end = struct
  type t = {
    class_: Folded_class.t;
    abstractness: abstractness;
    reason: Typing_reason.t option;
  }

  let has_needs_concrete_attribute Aast_defs.{ m_user_attributes; _ } =
    List.exists
      m_user_attributes
      ~f:(fun Aast_defs.{ ua_name = (_, name); _ } ->
        String.equal name Naming_special_names.UserAttributes.uaNeedsConcrete)

  let static_keyword_refers_to_concrete_class current_method =
    match current_method with
    | None -> false
    | Some method_ ->
      (not method_.Aast_defs.m_static) || has_needs_concrete_attribute method_

  let fold
      (type acc)
      env
      class_id
      ~(current_method : Tast.method_ option)
      ~(f : t -> acc)
      ~(intersect : acc list -> acc)
      ~(union : acc list -> acc) : acc option =
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
      f { class_; abstractness; reason = None }
    | Aast.CIparent ->
      let* class_name = Tast_env.get_parent_id env in
      let+ class_ = get_class class_name in
      f
        {
          class_;
          abstractness = abstractness_of_folded_class class_;
          reason = None;
        }
    | Aast.CIself ->
      let* class_name = Tast_env.get_self_id env in
      let+ class_ = get_class class_name in
      f
        {
          class_;
          abstractness = abstractness_of_folded_class class_;
          reason = None;
        }
    | Aast.CI (_, class_name) ->
      let+ class_ = get_class class_name in
      f
        {
          class_;
          abstractness = abstractness_of_folded_class class_;
          reason = None;
        }
    | Aast.CIexpr (ty, _, _) ->
      (*
      At a high level, the `CIExpr` case is similar to that for `CI` above.
      The contortion is to handle unions and intersections.

      Examples: `string & (dynamic | classname<C & D>)` and `classname<C> | concreteclassname<D>`
      *)
      let aggregate items partial_fn aggregation_fn =
        let accs = List.filter_map items ~f:partial_fn in
        if List.is_empty accs then
          None
        else
          Some (aggregation_fn accs)
      in
      (* extract class information from the T in classname<T> or concrete_classname<T>
       * `None` indicates no class information found *)
      let rec fold_targ abstractness ty_arg : acc option =
        match Typing_defs_core.deref ty_arg with
        | (reason, Typing_defs.Tclass ((_, class_name), _exact, _targs)) ->
        begin
          match get_class class_name with
          | Some class_ ->
            Some (f { class_; abstractness; reason = Some reason })
          | None -> None
        end
        | (_, Typing_defs.Tintersection tys) ->
          aggregate tys (fold_targ abstractness) intersect
        | (_, Typing_defs.Tunion tys) ->
          aggregate tys (fold_targ abstractness) union
        | _ -> None
      in
      (* extract class information from classname<T> or concrete_classname<T>
       * `None` indicates no class information found *)
      let rec fold_ty abstractness ty : acc option =
        match snd @@ Typing_defs_core.deref ty with
        | Typing_defs.Tnewtype (new_type, [hd_targ], _)
          when String.equal new_type Naming_special_names.Classes.cConcrete ->
          fold_ty Concrete hd_targ
        | Typing_defs.Tnewtype (new_type, [hd_targ], _)
          when String.equal new_type Naming_special_names.Classes.cClassname ->
          fold_targ abstractness hd_targ
        | Typing_defs.Tintersection tys ->
          aggregate tys (fold_ty abstractness) intersect
        | Typing_defs.Tunion tys -> aggregate tys (fold_ty abstractness) union
        | _ -> None
      in
      fold_ty Maybe_abstract ty
end

let check_for_call_abstract
    Class_use.{ class_; abstractness; reason } method_ folded_method :
    Typing_warning.Safe_abstract.t option =
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
    Some
      Typing_warning.Safe_abstract.
        {
          kind = Call_abstract { method_ };
          class_ = Folded_class.name class_;
          reason;
        }
  else
    None

(** Check for $class::the_const where the_const might be abstract.
 * This is very similar to check_for_call_abstract above
 *)
let check_for_abstract_const_access
    Class_use.{ class_; abstractness = class_abstractness; reason }
    const_name
    (const : Typing_defs.class_const) : Typing_warning.Safe_abstract.t option =
  let const_may_be_abstract =
    match class_abstractness with
    | Abstract
    | Maybe_abstract -> begin
      match const.Typing_defs.cc_abstract with
      | Typing_defs.CCAbstract _has_default -> true
      | Typing_defs.CCConcrete -> false
    end
    | Concrete -> false
  in
  let warning_would_be_redundant =
    match class_abstractness with
    | Abstract -> true
    | Maybe_abstract
    | Concrete ->
      false
  in
  if const_may_be_abstract && not warning_would_be_redundant then
    Some
      Typing_warning.Safe_abstract.
        {
          kind = Const_access_abstract { const = const_name };
          class_ = Folded_class.name class_;
          reason;
        }
  else
    None

let check_for_call_needs_concrete
    Class_use.{ class_; abstractness; reason } method_ folded_method :
    Typing_warning.Safe_abstract.t option =
  let callee_method_needs_concrete =
    Typing_defs.get_ce_readonly_prop_or_needs_concrete folded_method
  in
  match abstractness with
  | Abstract
  | Maybe_abstract ->
    if callee_method_needs_concrete then
      Some
        Typing_warning.Safe_abstract.
          {
            kind = Call_needs_concrete { method_ };
            class_ = Folded_class.name class_;
            reason;
          }
    else
      None
  | Concrete -> None

let check_for_new_abstract Class_use.{ class_; abstractness; reason } :
    Typing_warning.Safe_abstract.t option =
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
    Some
      Typing_warning.Safe_abstract.
        { kind = New_abstract; class_ = Folded_class.name class_; reason }
  else
    None

(**
A use of a union of classnames is ill-typed iff *any* branch is ill-typed. Save all the warnings.
For example,

```
<<__ConsistentConstruct>>
abstract class Abs1 {}
<<__ConsistentConstruct>>
abstract class Abs2 { }
<<__ConsistentConstruct>>
final class C1 { }

 $class : (classname<Abs1> | classname<Abs2> | concreteclassname<C1>)
// warning: Unsafe use of new: Abs1 might be abstract
// warning: Unsafe use of new: Abs2 might be abstract
new $class();
 ```

 **)
let union_warnings = List.concat

(**
A use of an intersection of classnames is ill-typed iff *all* branches are ill-typed.

For simplicity (of implementation and user experience) report just the first warning.
(we'd do something fancier if this situation were common, but I doubt it's common)

```
 <<__ConsistentConstruct>>
 interface A {}
 <<__ConsistentConstruct>>
 interface B { }

 // $class: (classname<A> & classname<B>)
 // warning: unsafe to instantiate A because it might be abstract
 new $class();
 ```

 **)
let intersect_warnings (warnings : Typing_warning.Safe_abstract.t list list) :
    Typing_warning.Safe_abstract.t list =
  match List.find warnings ~f:List.is_empty with
  | Some l -> l
  | None -> List.hd warnings |> Option.value ~default:[]

let is_hhi expr =
  expr
  |> Tuple3.get2
  |> Pos.filename
  |> Relative_path.prefix
  |> Relative_path.is_hhi

let calc_warnings env expr ~(current_method : Tast.method_ option) :
    class_use_info option =
  if is_hhi expr then
    None
  else
    match expr with
    | Aast.
        ( _,
          call_pos,
          Call
            { func = (_, _, Class_const ((_, _, class_id), (_, method_))); _ }
        ) ->
      let make_warnings class_use =
        let folded_method_opt =
          Tast_env.get_static_member
            (* is_method *) true
            env
            class_use.Class_use.class_
            method_
        in
        match folded_method_opt with
        | Some folded_method ->
          Option.to_list
            (check_for_call_abstract class_use method_ folded_method)
          @ Option.to_list
              (check_for_call_needs_concrete class_use method_ folded_method)
        | None -> []
      in
      let warnings =
        match
          Class_use.fold
            env
            class_id
            ~current_method
            ~f:make_warnings
            ~intersect:intersect_warnings
            ~union:union_warnings
        with
        | Some warnings -> warnings
        | None -> []
      in
      Some (Static_method_call, call_pos, warnings)
    | Aast.(_, const_pos, Class_const ((_, _, class_id), (_, const_name))) ->
      let make_warnings class_use =
        let const_opt =
          Tast_env.get_const env class_use.Class_use.class_ const_name
        in
        match const_opt with
        | Some const ->
          Option.to_list
            (check_for_abstract_const_access class_use const_name const)
        | None -> []
      in
      let warnings =
        match
          Class_use.fold
            env
            class_id
            ~current_method
            ~f:make_warnings
            ~intersect:intersect_warnings
            ~union:union_warnings
        with
        | Some warnings -> warnings
        | None -> []
      in
      Some (Const_access, const_pos, warnings)
    | Aast.
        (_, new_pos, New ((_, _, class_id), _targs, _exprs, _expr, _constructor))
      ->
      let make_warnings class_use =
        Option.to_list (check_for_new_abstract class_use)
      in
      let warnings =
        match
          Class_use.fold
            env
            class_id
            ~current_method
            ~f:make_warnings
            ~intersect:intersect_warnings
            ~union:union_warnings
        with
        | Some warnings -> warnings
        | None -> []
      in
      Some (New, new_pos, warnings)
    | _ -> None
