open Hh_prelude
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
  rank: int;
}

and named_kind = Typing_defs.pos_id * kind [@@deriving hash, show]

let dummy_name = (Pos_or_decl.none, "")

let kind_of_decl_tparam_no_bounds
    ?(rank = 0) ?(require_dynamic = false) decl_tparam =
  let { tp_reified = reified; tp_user_attributes; _ } = decl_tparam in
  let enforceable =
    Attributes.mem SN.UserAttributes.uaEnforceable tp_user_attributes
  in
  let newable = Attributes.mem SN.UserAttributes.uaNewable tp_user_attributes in

  {
    lower_bounds = TySet.empty;
    upper_bounds = TySet.empty;
    reified;
    enforceable;
    newable;
    require_dynamic;
    rank;
  }

let force_lazy_values (kind : kind) =
  let {
    lower_bounds;
    upper_bounds;
    reified;
    enforceable;
    newable;
    require_dynamic;
    rank;
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
    rank;
  }
