open Hh_prelude
open Common
open Typing_defs
module TySet = Typing_set
module SN = Naming_special_names

type tparam_bounds = TySet.t [@@deriving hash, show]

type kind = {
  lower_bounds: tparam_bounds;
  upper_bounds: tparam_bounds;
  reified: Aast.reify_kind;
  enforceable: bool;
  newable: bool;
  require_dynamic: bool;
  parameters: named_kind list;
}

and named_kind = pos_id * kind [@@deriving hash, show]

let dummy_name = (Pos_or_decl.none, "")

let with_dummy_name k = (dummy_name, k)

let get_arity k = List.length k.parameters

let string_of_kind (kind : kind) =
  let rec stringify toplevel k =
    match k.parameters with
    | [] -> "Type"
    | params ->
      let parts = List.map params ~f:(fun (_, pk) -> stringify false pk) in
      let res = String.concat ~sep:" -> " parts ^ " -> Type" in
      if toplevel then
        res
      else
        "(" ^ res ^ ")"
  in
  stringify true kind

let description_of_kind kind =
  match kind.parameters with
  | [] -> "a fully-applied type"
  | params
    when List.for_all params ~f:(fun (_, param) ->
             Int.( = ) 0 (get_arity param)) ->
    (* no higher-order arguments *)
    let param_count = List.length params in
    let args_desc =
      if Int.( = ) 1 param_count then
        "a single (fully-applied) type argument"
      else
        string_of_int param_count ^ "(fully-applied) type arguments"
    in
    "a type constructor expecting " ^ args_desc
  | _ -> "a type constructor of kind " ^ string_of_kind kind

let rec remove_bounds kind =
  {
    kind with
    lower_bounds = TySet.empty;
    upper_bounds = TySet.empty;
    parameters =
      List.map kind.parameters ~f:(fun (n, k) -> (n, remove_bounds k));
  }

module Simple = struct
  type bounds_for_wildcard =
    | NonLocalized of (Ast_defs.constraint_kind * decl_ty) list
    | Localized of {
        wc_lower: tparam_bounds;
        wc_upper: tparam_bounds;
      }

  (* Gives us access to the non-simple kind after shadowing it below *)
  type full_kind = kind

  type named_full_kind = named_kind

  type kind = full_kind * bounds_for_wildcard

  type named_kind = pos_id * kind

  (* let without_wildcard_bounds k = (k, None) *)

  let string_of_kind (k, _) = string_of_kind k

  let description_of_kind (k, _) = description_of_kind k

  let get_arity sk = get_arity (fst sk)

  let fully_applied_type
      ?(reified = Aast.Erased) ?(enforceable = false) ?(newable = false) () :
      kind =
    ( {
        lower_bounds = TySet.empty;
        upper_bounds = TySet.empty;
        reified;
        enforceable;
        newable;
        require_dynamic = false;
        parameters = [];
      },
      NonLocalized [] )

  let to_full_kind_without_bounds kind = remove_bounds (fst kind)

  let get_wilcard_bounds = snd

  (* not public *)
  let rec named_internal_kind_of_decl_tparam decl_tparam : named_full_kind =
    let { tp_name; tp_reified = reified; tp_user_attributes; _ } =
      decl_tparam
    in
    let enforceable =
      Attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes
    in
    let newable =
      Attributes.mem SN.UserAttributes.uaNewable tp_user_attributes
    in
    let (st, _) = fully_applied_type ~reified ~enforceable ~newable () in
    ( tp_name,
      {
        st with
        parameters = named_internal_kinds_of_decl_tparams decl_tparam.tp_tparams;
      } )

  (* not public *)
  and named_internal_kinds_of_decl_tparams (tparams : decl_tparam list) :
      named_full_kind list =
    List.map tparams ~f:named_internal_kind_of_decl_tparam

  (* public *)
  and named_kind_of_decl_tparam decl_tparam : named_kind =
    let (name, internal_kind) =
      named_internal_kind_of_decl_tparam decl_tparam
    in
    (name, (internal_kind, NonLocalized decl_tparam.tp_constraints))

  (* public *)
  let named_kinds_of_decl_tparams decl_tparams : named_kind list =
    List.map decl_tparams ~f:named_kind_of_decl_tparam

  let type_with_params_to_simple_kind ?reified ?enforceable ?newable tparams =
    let (st, _) = fully_applied_type ?reified ?enforceable ?newable () in
    ( { st with parameters = named_internal_kinds_of_decl_tparams tparams },
      NonLocalized [] )

  let get_named_parameter_kinds (kind, _) : named_kind list =
    List.map kind.parameters ~f:(fun (n, fk) -> (n, (fk, NonLocalized [])))

  let from_full_kind fk =
    let wildcard_bounds =
      Localized { wc_lower = fk.lower_bounds; wc_upper = fk.upper_bounds }
    in
    (* We don't actually have to remove any of the bounds inside of the kind itself.
       The fact that the bounds are still there is hidden behind this  module's interface, which
       denies access to them *)
    (fk, wildcard_bounds)

  let with_dummy_name = with_dummy_name
end

let rec force_lazy_values (kind : kind) =
  let {
    lower_bounds;
    upper_bounds;
    reified;
    enforceable;
    newable;
    require_dynamic;
    parameters;
  } =
    kind
  in
  {
    lower_bounds = TySet.force_lazy_values lower_bounds;
    upper_bounds = TySet.force_lazy_values upper_bounds;
    reified;
    enforceable;
    newable;
    require_dynamic;
    parameters = List.map parameters ~f:force_lazy_values_named_kind;
  }

and force_lazy_values_named_kind ((p, kind) : named_kind) =
  (p, force_lazy_values kind)
