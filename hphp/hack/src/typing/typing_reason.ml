(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SN = Naming_special_names

let strip_ns id =
  id |> Utils.strip_ns |> Hh_autoimport.strip_HH_namespace_if_autoimport

(* ~~ pos_id ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type pos_id = (Pos_or_decl.t[@hash.ignore]) * Ast_defs.id_
[@@deriving eq, hash, ord, show]

let positioned_id pos_or_decl (p, x) = (pos_or_decl p, x)

let pos_id_to_json (pos_or_decl, str) =
  Hh_json.(
    JSON_Object
      [("Tuple2", JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String str])])

(* ~~ arg_position ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)
type arg_position =
  | Aonly
  | Afirst
  | Asecond
[@@deriving eq, hash, show]

let arg_pos_str ap =
  match ap with
  | Aonly -> "only"
  | Afirst -> "first"
  | Asecond -> "second"

let arg_position_to_json = function
  | Aonly -> Hh_json.(JSON_Object [("Aonly", JSON_Array [])])
  | Afirst -> Hh_json.(JSON_Object [("Afirst", JSON_Array [])])
  | Asecond -> Hh_json.(JSON_Object [("Asecond", JSON_Array [])])

(* ~~ expr_dep_type_reason ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type expr_dep_type_reason =
  | ERexpr of Expression_id.t
  | ERstatic
  | ERclass of string
  | ERparent of string
  | ERself of string
  | ERpu of string
[@@deriving eq, hash, show]

let expr_dep_type_reason_to_json = function
  | ERexpr id ->
    Hh_json.(
      JSON_Object
        [("ERexpr", JSON_Array [JSON_String (Expression_id.debug id)])])
  | ERstatic -> Hh_json.(JSON_Object [("ERstatic", JSON_Array [])])
  | ERclass str ->
    Hh_json.(JSON_Object [("ERclass", JSON_Array [JSON_String str])])
  | ERparent str ->
    Hh_json.(JSON_Object [("ERparent", JSON_Array [JSON_String str])])
  | ERself str ->
    Hh_json.(JSON_Object [("ERself", JSON_Array [JSON_String str])])
  | ERpu str -> Hh_json.(JSON_Object [("ERpu", JSON_Array [JSON_String str])])

let expr_dep_type_reason_string e =
  match e with
  | ERexpr id ->
    "where "
    ^ Markdown_lite.md_codify (Expression_id.display id)
    ^ " is a reference to this expression"
  | ERstatic ->
    "where `<static>` refers to the late bound type of the enclosing class"
  | ERclass c ->
    "where the class "
    ^ (strip_ns c |> Markdown_lite.md_codify)
    ^ " was referenced here"
  | ERparent p ->
    "where the class "
    ^ (strip_ns p |> Markdown_lite.md_codify)
    ^ " (the parent of the enclosing) class was referenced here"
  | ERself c ->
    "where the class "
    ^ (strip_ns c |> Markdown_lite.md_codify)
    ^ " was referenced here via the keyword `self`"
  | ERpu s ->
    "where "
    ^ Markdown_lite.md_codify s
    ^ " is a type projected from this expression"

(* ~~ blame_source ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type blame_source =
  | BScall
  | BSlambda
  | BSassignment
  | BSout_of_scope
[@@deriving eq, hash, show]

let blame_source_to_json = function
  | BScall -> Hh_json.(JSON_Object [("BScall", JSON_Array [])])
  | BSlambda -> Hh_json.(JSON_Object [("BSlambda", JSON_Array [])])
  | BSassignment -> Hh_json.(JSON_Object [("BSassignment", JSON_Array [])])
  | BSout_of_scope -> Hh_json.(JSON_Object [("BSout_of_scope", JSON_Array [])])

(* ~~ blame ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type blame = Blame of Pos.t * blame_source [@@deriving eq, hash, show]

let pos_to_json pos = Pos.(json @@ to_absolute pos)

let blame_to_json (Blame (pos, src)) =
  Hh_json.(
    JSON_Object
      [("Blame", JSON_Array [pos_to_json pos; blame_source_to_json src])])

(* ~~ phase indices ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

(* create private types to represent the different type phases *)
type decl_phase = private DeclPhase [@@deriving eq, hash, show]

type locl_phase = private LoclPhase [@@deriving eq, hash, show]

(* This is to avoid a compile error with ppx_hash "Unbound value _hash_fold_phase". *)
let _hash_fold_phase hsv _ = hsv

(* ~~ projections ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type variance_dir =
  | Co
  | Contra
[@@deriving hash]

let variance_dir_to_json = function
  | Co -> Hh_json.(JSON_Object [("Co", JSON_Array [])])
  | Contra -> Hh_json.(JSON_Object [("Contr", JSON_Array [])])

let variance_dir_is_contra = function
  | Contra -> true
  | Co -> false

let explain_variance_dir = function
  | Contra -> "contravariant"
  | Co -> "covariant"

(** When recording the decomposition of a type during inference we want to keep
    track of variance so we can give intuition about the direction of 'flow'.
    In the case of invariant type paramters, we record both the fact that it was
    invariant and the direction in which the error occurred *)
type cstr_variance =
  | Dir of variance_dir
  | Inv of variance_dir
[@@deriving hash]

let cstr_variance_is_contra = function
  | Dir dir
  | Inv dir ->
    variance_dir_is_contra dir

let cstr_variance_to_json = function
  | Dir dir ->
    Hh_json.(JSON_Object [("Dir", JSON_Array [variance_dir_to_json dir])])
  | Inv dir ->
    Hh_json.(JSON_Object [("Inv", JSON_Array [variance_dir_to_json dir])])

(** Shape field kinds *)
type field_kind =
  | Absent
  | Optional
  | Required
[@@deriving hash]

let field_kind_to_json = function
  | Absent -> Hh_json.(JSON_Object [("Absent", JSON_Array [])])
  | Optional -> Hh_json.(JSON_Object [("Optional", JSON_Array [])])
  | Required -> Hh_json.(JSON_Object [("Required", JSON_Array [])])

let explain_field_kind = function
  | Required -> "required"
  | Optional -> "optional"
  | Absent -> "non-existent"

(** Symmetric projections are those in which the same decomposition is applied
    to both sub- and supertype during inference *)
type prj_symm =
  | Prj_symm_neg
  | Prj_symm_class of string * int * cstr_variance
  | Prj_symm_newtype of string * int * cstr_variance
  | Prj_symm_tuple of int
  | Prj_symm_shape of string * field_kind * field_kind
  | Prj_symm_fn_param of int * int
  | Prj_symm_fn_param_inout of int * int * variance_dir
  | Prj_symm_fn_ret
[@@deriving hash]

let prj_symm_is_contra = function
  | Prj_symm_class (_, _, var)
  | Prj_symm_newtype (_, _, var) ->
    cstr_variance_is_contra var
  | Prj_symm_fn_param _
  | Prj_symm_fn_param_inout (_, _, Contra) ->
    true
  | Prj_symm_fn_param_inout (_, _, Co)
  | Prj_symm_fn_ret
  | Prj_symm_neg
  | Prj_symm_tuple _
  | Prj_symm_shape _ ->
    false

let prj_symm_to_json = function
  | Prj_symm_neg -> Hh_json.JSON_String "Prj_symm_neg"
  | Prj_symm_class (nm, idx, variance) ->
    Hh_json.(
      JSON_Object
        [
          ( "Prj_symm_class",
            JSON_Array
              [
                JSON_String nm;
                JSON_Number (string_of_int idx);
                cstr_variance_to_json variance;
              ] );
        ])
  | Prj_symm_newtype (nm, idx, variance) ->
    Hh_json.(
      JSON_Object
        [
          ( "Prj_symm_newtype",
            JSON_Array
              [
                JSON_String nm;
                JSON_Number (string_of_int idx);
                cstr_variance_to_json variance;
              ] );
        ])
  | Prj_symm_tuple idx ->
    Hh_json.(JSON_Object [(" Prj_symm_tuple", JSON_Number (string_of_int idx))])
  | Prj_symm_shape (fld_nm, fld_kind_sub, fld_kind_super) ->
    Hh_json.(
      JSON_Object
        [
          ( " Prj_symm_shape",
            JSON_Array
              [
                JSON_String fld_nm;
                field_kind_to_json fld_kind_sub;
                field_kind_to_json fld_kind_super;
              ] );
        ])
  | Prj_symm_fn_param (idx_sub, idx_sup) ->
    Hh_json.(
      JSON_Object
        [
          ( "Prj_symm_fn_param",
            JSON_Array
              [
                JSON_Number (string_of_int idx_sub);
                JSON_Number (string_of_int idx_sup);
              ] );
        ])
  | Prj_symm_fn_param_inout (idx_sub, idx_sup, variance) ->
    Hh_json.(
      JSON_Object
        [
          ( "Prj_symm_fn_param_inout",
            JSON_Array
              [
                JSON_Number (string_of_int idx_sub);
                JSON_Number (string_of_int idx_sup);
                variance_dir_to_json variance;
              ] );
        ])
  | Prj_symm_fn_ret -> Hh_json.JSON_String "Prj_symm_fn_ret"

let int_to_ordinal =
  let sfxs = [| "th"; "st"; "nd"; "rd"; "th" |] in
  fun n ->
    let sfx =
      if n >= 10 && n <= 20 then
        "th"
      else
        sfxs.(min 4 (n mod 10))
    in
    Format.sprintf "%d%s" n sfx

let explain_symm_prj prj ~side =
  match prj with
  | Prj_symm_neg -> "via the negation type"
  | Prj_symm_class (nm, idx, Inv dir) ->
    Format.sprintf
      "as the invariant, %s type parameter of the class `%s`, when typing as %s"
      (int_to_ordinal (idx + 1))
      nm
      (explain_variance_dir dir)
  | Prj_symm_class (nm, idx, Dir dir) ->
    Format.sprintf
      "as the %s, %s type parameter of the class `%s`"
      (explain_variance_dir dir)
      (int_to_ordinal (idx + 1))
      nm
  | Prj_symm_newtype (nm, idx, Inv dir) ->
    Format.sprintf
      "as the invariant, %s type parameter of the type definition `%s`, when typing as %s"
      (int_to_ordinal (idx + 1))
      nm
      (explain_variance_dir dir)
  | Prj_symm_newtype (nm, idx, Dir dir) ->
    Format.sprintf
      "as the %s, %s type parameter of the type definition `%s`"
      (explain_variance_dir dir)
      (int_to_ordinal (idx + 1))
      nm
  | Prj_symm_tuple idx ->
    Format.sprintf "as the %s element of the tuple" (int_to_ordinal idx)
  | Prj_symm_shape (fld_nm, fld_kind_lhs, fld_kind_rhs) ->
    let fld_kind =
      match side with
      | `Lhs -> fld_kind_lhs
      | `Rhs -> fld_kind_rhs
    in
    Format.sprintf
      "as the %s shape field `'%s'`"
      (explain_field_kind fld_kind)
      fld_nm
  | Prj_symm_fn_param (idx_lhs, idx_rhs) ->
    let idx =
      match side with
      | `Lhs -> idx_lhs
      | `Rhs -> idx_rhs
    in
    Format.sprintf "as the %s function parameter" (int_to_ordinal (idx + 1))
  | Prj_symm_fn_param_inout (idx_lhs, idx_rhs, dir) ->
    let idx =
      match side with
      | `Lhs -> idx_lhs
      | `Rhs -> idx_rhs
    in
    Format.sprintf
      "as the invariant, %s `inout` function parameter, when typing as %s"
      (int_to_ordinal (idx + 1))
      (explain_variance_dir dir)
  | Prj_symm_fn_ret -> "as the function return type"

(** Asymmetric projections are those in which the same decomposition is applied
    to only one of the sub- or supertype during inference *)
type prj_asymm =
  | Prj_asymm_union
  | Prj_asymm_inter
  | Prj_asymm_neg
[@@deriving hash]

let prj_asymm_to_json = function
  | Prj_asymm_union -> Hh_json.JSON_String "Prj_asymm_union"
  | Prj_asymm_inter -> Hh_json.JSON_String "Prj_asymm_inter"
  | Prj_asymm_neg -> Hh_json.JSON_String "Prj_asymm_neg"

let explain_asymm_prj prj =
  match prj with
  | Prj_asymm_union -> "as an element of the union type"
  | Prj_asymm_inter -> "as an element of the intersection type"
  | Prj_asymm_neg -> "as the inner type of a negation"

(** For asymmetric projections we need to track which of the sub- or supertype
    was decomposed  *)
type side =
  | Sub
  | Super
[@@deriving hash]

let side_to_json = function
  | Sub -> Hh_json.JSON_String "Sub"
  | Super -> Hh_json.JSON_String "Super"

(** Top-level projections  *)
type prj =
  | Symm of prj_symm
  | Asymm of side * prj_asymm
[@@deriving hash]

let prj_to_json = function
  | Symm prj_symm ->
    Hh_json.(JSON_Object [("Symm", JSON_Array [prj_symm_to_json prj_symm])])
  | Asymm (side, prj_asymm) ->
    Hh_json.(
      JSON_Object
        [("Asymm", JSON_Array [side_to_json side; prj_asymm_to_json prj_asymm])])

let prj_is_contra = function
  | Asymm _ -> false
  | Symm prj_symm -> prj_symm_is_contra prj_symm

let explain_prj_left = function
  | Symm prj -> explain_symm_prj prj ~side:`Lhs
  | Asymm (_, prj) -> explain_asymm_prj prj

let explain_prj_right = function
  | Symm prj -> explain_symm_prj prj ~side:`Rhs
  | Asymm (_, prj) -> explain_asymm_prj prj
(* ~~ Reasons ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

(* The phase below helps enforce that only Pos_or_decl.t positions end up in the heap.
 * To enforce that, any reason taking a Pos.t should be a locl_phase t_
 * to prevent a decl ty from using it.
 * Reasons used for decl types should be 'phase t_ so that they can be localized
 * to be used in the localized version of the type. *)

(** The reason why something is expected to have a certain type *)
type _ t_ =
  | Rnone : 'phase t_
  | Rwitness : Pos.t -> locl_phase t_
  | Rwitness_from_decl : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Ridx : Pos.t * locl_phase t_ -> locl_phase t_
      (** Used as an index into a vector-like
          array or string. Position of indexing,
          reason for the indexed type *)
  | Ridx_vector : Pos.t -> locl_phase t_
  | Ridx_vector_from_decl : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
      (** Used as an index, in the Vector case *)
  | Rforeach : Pos.t -> locl_phase t_
      (** Because it is iterated in a foreach loop *)
  | Rasyncforeach : Pos.t -> locl_phase t_
      (** Because it is iterated "await as" in foreach *)
  | Rarith : Pos.t -> locl_phase t_
  | Rarith_ret : Pos.t -> locl_phase t_
  | Rarith_ret_float : Pos.t * locl_phase t_ * arg_position -> locl_phase t_
      (** pos, arg float typing reason, arg position *)
  | Rarith_ret_num : Pos.t * locl_phase t_ * arg_position -> locl_phase t_
      (** pos, arg num typing reason, arg position *)
  | Rarith_ret_int : Pos.t -> locl_phase t_
  | Rarith_dynamic : Pos.t -> locl_phase t_
  | Rbitwise_dynamic : Pos.t -> locl_phase t_
  | Rincdec_dynamic : Pos.t -> locl_phase t_
  | Rcomp : Pos.t -> locl_phase t_
  | Rconcat_ret : Pos.t -> locl_phase t_
  | Rlogic_ret : Pos.t -> locl_phase t_
  | Rbitwise : Pos.t -> locl_phase t_
  | Rbitwise_ret : Pos.t -> locl_phase t_
  | Rno_return : Pos.t -> locl_phase t_
  | Rno_return_async : Pos.t -> locl_phase t_
  | Rret_fun_kind : Pos.t * Ast_defs.fun_kind -> locl_phase t_
  | Rret_fun_kind_from_decl :
      (Pos_or_decl.t[@hash.ignore]) * Ast_defs.fun_kind
      -> 'phase t_
  | Rhint : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rthrow : Pos.t -> locl_phase t_
  | Rplaceholder : Pos.t -> locl_phase t_
  | Rret_div : Pos.t -> locl_phase t_
  | Ryield_gen : Pos.t -> locl_phase t_
  | Ryield_asyncgen : Pos.t -> locl_phase t_
  | Ryield_asyncnull : Pos.t -> locl_phase t_
  | Ryield_send : Pos.t -> locl_phase t_
  | Rlost_info : string * locl_phase t_ * blame -> locl_phase t_
  | Rformat : Pos.t * string * locl_phase t_ -> locl_phase t_
  | Rclass_class : (Pos_or_decl.t[@hash.ignore]) * string -> 'phase t_
  | Runknown_class : Pos.t -> locl_phase t_
  | Rvar_param : Pos.t -> locl_phase t_
  | Rvar_param_from_decl : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Runpack_param : Pos.t * (Pos_or_decl.t[@hash.ignore]) * int -> locl_phase t_
      (** splat pos, fun def pos, number of args before splat *)
  | Rinout_param : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rinstantiate : 'phase t_ * string * 'phase t_ -> 'phase t_
  | Rtypeconst :
      'phase t_
      * ((Pos_or_decl.t[@hash.ignore]) * string)
      * string Lazy.t
      * 'phase t_
      -> 'phase t_
  | Rtype_access :
      locl_phase t_ * (locl_phase t_ * string Lazy.t) list
      -> locl_phase t_
  | Rexpr_dep_type :
      'phase t_ * (Pos_or_decl.t[@hash.ignore]) * expr_dep_type_reason
      -> 'phase t_
  | Rnullsafe_op : Pos.t -> locl_phase t_  (** ?-> operator is used *)
  | Rtconst_no_cstr : pos_id -> 'phase t_
  | Rpredicated : Pos.t * string -> locl_phase t_
  | Ris_refinement : Pos.t -> locl_phase t_
  | Ras_refinement : Pos.t -> locl_phase t_
  | Requal : Pos.t -> locl_phase t_
  | Rvarray_or_darray_key : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rvec_or_dict_key : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rusing : Pos.t -> locl_phase t_
  | Rdynamic_prop : Pos.t -> locl_phase t_
  | Rdynamic_call : Pos.t -> locl_phase t_
  | Rdynamic_construct : Pos.t -> locl_phase t_
  | Ridx_dict : Pos.t -> locl_phase t_
  | Ridx_set_element : Pos.t -> locl_phase t_
  | Rmissing_optional_field :
      (Pos_or_decl.t[@hash.ignore]) * string
      -> 'phase t_
  | Runset_field : Pos.t * string -> locl_phase t_
  | Rcontravariant_generic : locl_phase t_ * string -> locl_phase t_
  | Rinvariant_generic : locl_phase t_ * string -> locl_phase t_
  | Rregex : Pos.t -> locl_phase t_
  | Rimplicit_upper_bound : (Pos_or_decl.t[@hash.ignore]) * string -> 'phase t_
  | Rtype_variable : Pos.t -> locl_phase t_
  | Rtype_variable_generics : Pos.t * string * string -> locl_phase t_
  | Rtype_variable_error : Pos.t -> locl_phase t_
  | Rglobal_type_variable_generics :
      (Pos_or_decl.t[@hash.ignore]) * string * string
      -> 'phase t_
  | Rsolve_fail : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rcstr_on_generics : (Pos_or_decl.t[@hash.ignore]) * pos_id -> 'phase t_
  | Rlambda_param : Pos.t * locl_phase t_ -> locl_phase t_
  | Rshape : Pos.t * string -> locl_phase t_
  | Rshape_literal : Pos.t -> locl_phase t_
  | Renforceable : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rdestructure : Pos.t -> locl_phase t_
  | Rkey_value_collection_key : Pos.t -> locl_phase t_
  | Rglobal_class_prop : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rglobal_fun_param : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rglobal_fun_ret : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rsplice : Pos.t -> locl_phase t_
  | Ret_boolean : Pos.t -> locl_phase t_
  | Rdefault_capability : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rconcat_operand : Pos.t -> locl_phase t_
  | Rinterp_operand : Pos.t -> locl_phase t_
  | Rdynamic_coercion of locl_phase t_
  | Rsupport_dynamic_type : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rdynamic_partial_enforcement :
      (Pos_or_decl.t[@hash.ignore]) * string * locl_phase t_
      -> locl_phase t_
  | Rrigid_tvar_escape :
      Pos.t * string * string * locl_phase t_
      -> locl_phase t_
  | Ropaque_type_from_module :
      (Pos_or_decl.t[@hash.ignore]) * string * locl_phase t_
      -> locl_phase t_
  | Rmissing_class : Pos.t -> locl_phase t_
  | Rinvalid : 'phase t_
  | Rcaptured_like : Pos.t -> locl_phase t_
  | Rpessimised_inout : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rpessimised_return : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rpessimised_prop : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Runsafe_cast : Pos.t -> locl_phase t_
  | Rpattern : Pos.t -> locl_phase t_
  | Rflow : locl_phase t_ * locl_phase t_ -> locl_phase t_
  | Rrev : locl_phase t_ -> locl_phase t_
  | Rprj : prj * locl_phase t_ -> locl_phase t_
  | Rmissing_field : locl_phase t_
  | Rpessimised_this : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
[@@deriving hash]

(** Perform actual reversal of reason flows *)
let rec normalize : locl_phase t_ -> locl_phase t_ = function
  | Rflow (t1, t2) -> Rflow (normalize t1, normalize t2)
  | Rrev t -> reverse t
  | Rprj (prj, t) -> Rprj (prj, normalize t)
  | t -> t

and reverse : locl_phase t_ -> locl_phase t_ = function
  | Rflow (t1, t2) -> Rflow (reverse t2, reverse t1)
  | Rrev t -> normalize t
  | Rprj (prj, t) -> Rprj (prj, reverse t)
  | t -> t

let rec to_raw_pos : type ph. ph t_ -> Pos_or_decl.t =
 fun r ->
  match r with
  | Rnone
  | Rinvalid
  | Rmissing_field ->
    Pos_or_decl.none
  | Rret_fun_kind_from_decl (p, _)
  | Rhint p
  | Rwitness_from_decl p
  | Rclass_class (p, _)
  | Rvar_param_from_decl p
  | Rglobal_fun_param p
  | Rglobal_fun_ret p
  | Renforceable p
  | Rimplicit_upper_bound (p, _)
  | Rsolve_fail p
  | Rmissing_optional_field (p, _)
  | Rvarray_or_darray_key p
  | Rvec_or_dict_key p
  | Rdefault_capability p
  | Rtconst_no_cstr (p, _)
  | Rtypeconst (Rnone, (p, _), _, _)
  | Rcstr_on_generics (p, _)
  | Rglobal_type_variable_generics (p, _, _)
  | Ridx_vector_from_decl p
  | Rinout_param p
  | Rsupport_dynamic_type p
  | Rpessimised_inout p
  | Rpessimised_return p
  | Rpessimised_prop p
  | Rpessimised_this p
  | Rglobal_class_prop p ->
    p
  | Rwitness p
  | Rcaptured_like p
  | Ridx (p, _)
  | Ridx_vector p
  | Rforeach p
  | Rasyncforeach p
  | Rarith p
  | Rarith_ret p
  | Rarith_dynamic p
  | Rcomp p
  | Rconcat_ret p
  | Rlogic_ret p
  | Rbitwise p
  | Rbitwise_ret p
  | Rno_return p
  | Rno_return_async p
  | Rret_fun_kind (p, _)
  | Rthrow p
  | Rplaceholder p
  | Rret_div p
  | Ryield_gen p
  | Ryield_asyncgen p
  | Ryield_asyncnull p
  | Ryield_send p
  | Rformat (p, _, _)
  | Runknown_class p
  | Rvar_param p
  | Runpack_param (p, _, _)
  | Rnullsafe_op p
  | Rpredicated (p, _)
  | Ris_refinement p
  | Ras_refinement p
  | Requal p
  | Rusing p
  | Rdynamic_prop p
  | Rdynamic_call p
  | Rdynamic_construct p
  | Ridx_dict p
  | Ridx_set_element p
  | Runset_field (p, _)
  | Rregex p
  | Rarith_ret_float (p, _, _)
  | Rarith_ret_num (p, _, _)
  | Rarith_ret_int p
  | Rbitwise_dynamic p
  | Rincdec_dynamic p
  | Rtype_variable p
  | Rtype_variable_generics (p, _, _)
  | Rtype_variable_error p
  | Rlambda_param (p, _)
  | Rshape (p, _)
  | Rshape_literal p
  | Rdestructure p
  | Rkey_value_collection_key p
  | Rsplice p
  | Ret_boolean p
  | Rconcat_operand p
  | Rinterp_operand p
  | Rrigid_tvar_escape (p, _, _, _)
  | Rpattern p ->
    Pos_or_decl.of_raw_pos p
  | Rinvariant_generic (r, _)
  | Rcontravariant_generic (r, _)
  | Rtype_access (r, _)
  | Rlost_info (_, r, _) ->
    to_raw_pos r
  | Rinstantiate (_, _, r)
  | Rexpr_dep_type (r, _, _)
  | Rtypeconst (r, _, _, _) ->
    to_raw_pos r
  | Rdynamic_coercion r -> to_raw_pos r
  | Rdynamic_partial_enforcement (p, _, _) -> p
  | Ropaque_type_from_module (p, _, _) -> p
  | Rmissing_class p -> Pos_or_decl.of_raw_pos p
  | Runsafe_cast p -> Pos_or_decl.of_raw_pos p
  | Rflow (from, _into) -> to_raw_pos from
  | Rrev r -> to_raw_pos_rev r
  | Rprj (_prj, r) -> to_raw_pos r

and to_raw_pos_rev = function
  | Rprj (_, r)
  | Rflow (_, r)
  | Rrev r
  | r ->
    to_raw_pos r

let compare : type phase. phase t_ -> phase t_ -> int =
 fun r1 r2 ->
  let get_pri : type ph. ph t_ -> int = function
    | Rnone -> 0
    | Rforeach _ -> 1
    | Rwitness _ -> 3
    | Rlost_info _ -> 5
    | _ -> 2
  in
  let d = get_pri r2 - get_pri r1 in
  if Int.( <> ) d 0 then
    d
  else
    Pos_or_decl.compare (to_raw_pos r1) (to_raw_pos r2)

let rec map_pos :
    type ph.
    (Pos.t -> Pos.t) -> (Pos_or_decl.t -> Pos_or_decl.t) -> ph t_ -> ph t_ =
 fun pos pos_or_decl -> function
  | Rnone -> Rnone
  | Rmissing_field -> Rmissing_field
  | Rwitness p -> Rwitness (pos p)
  | Rwitness_from_decl p -> Rwitness_from_decl (pos_or_decl p)
  | Ridx (p, r) -> Ridx (pos p, map_pos pos pos_or_decl r)
  | Ridx_vector p -> Ridx_vector (pos p)
  | Ridx_vector_from_decl p -> Ridx_vector_from_decl (pos_or_decl p)
  | Rforeach p -> Rforeach (pos p)
  | Rasyncforeach p -> Rasyncforeach (pos p)
  | Rarith p -> Rarith (pos p)
  | Rarith_ret p -> Rarith_ret (pos p)
  | Rcomp p -> Rcomp (pos p)
  | Rconcat_ret p -> Rconcat_ret (pos p)
  | Rlogic_ret p -> Rlogic_ret (pos p)
  | Rbitwise p -> Rbitwise (pos p)
  | Rbitwise_ret p -> Rbitwise_ret (pos p)
  | Rno_return p -> Rno_return (pos p)
  | Rno_return_async p -> Rno_return_async (pos p)
  | Rret_fun_kind (p, k) -> Rret_fun_kind (pos p, k)
  | Rret_fun_kind_from_decl (p, k) -> Rret_fun_kind_from_decl (pos_or_decl p, k)
  | Rhint p -> Rhint (pos_or_decl p)
  | Rthrow p -> Rthrow (pos p)
  | Rplaceholder p -> Rplaceholder (pos p)
  | Rret_div p -> Rret_div (pos p)
  | Ryield_gen p -> Ryield_gen (pos p)
  | Ryield_asyncgen p -> Ryield_asyncgen (pos p)
  | Ryield_asyncnull p -> Ryield_asyncnull (pos p)
  | Ryield_send p -> Ryield_send (pos p)
  | Rlost_info (s, r1, Blame (p2, l)) ->
    Rlost_info (s, map_pos pos pos_or_decl r1, Blame (pos p2, l))
  | Rformat (p1, s, r) -> Rformat (pos p1, s, map_pos pos pos_or_decl r)
  | Rclass_class (p, s) -> Rclass_class (pos_or_decl p, s)
  | Runknown_class p -> Runknown_class (pos p)
  | Rvar_param p -> Rvar_param (pos p)
  | Rvar_param_from_decl p -> Rvar_param_from_decl (pos_or_decl p)
  | Runpack_param (p1, p2, i) -> Runpack_param (pos p1, pos_or_decl p2, i)
  | Rinout_param p -> Rinout_param (pos_or_decl p)
  | Rinstantiate (r1, x, r2) ->
    Rinstantiate (map_pos pos pos_or_decl r1, x, map_pos pos pos_or_decl r2)
  | Rtypeconst (r1, (p, s1), s2, r2) ->
    Rtypeconst
      ( map_pos pos pos_or_decl r1,
        (pos_or_decl p, s1),
        s2,
        map_pos pos pos_or_decl r2 )
  | Rtype_access (r1, ls) ->
    Rtype_access
      ( map_pos pos pos_or_decl r1,
        List.map ls ~f:(fun (r, s) -> (map_pos pos pos_or_decl r, s)) )
  | Rexpr_dep_type (r, p, n) ->
    Rexpr_dep_type (map_pos pos pos_or_decl r, pos_or_decl p, n)
  | Rnullsafe_op p -> Rnullsafe_op (pos p)
  | Rtconst_no_cstr id -> Rtconst_no_cstr (positioned_id pos_or_decl id)
  | Rpredicated (p, f) -> Rpredicated (pos p, f)
  | Ris_refinement p -> Ris_refinement (pos p)
  | Ras_refinement p -> Ras_refinement (pos p)
  | Requal p -> Requal (pos p)
  | Rvarray_or_darray_key p -> Rvarray_or_darray_key (pos_or_decl p)
  | Rvec_or_dict_key p -> Rvec_or_dict_key (pos_or_decl p)
  | Rusing p -> Rusing (pos p)
  | Rdynamic_prop p -> Rdynamic_prop (pos p)
  | Rdynamic_call p -> Rdynamic_call (pos p)
  | Rdynamic_construct p -> Rdynamic_construct (pos p)
  | Ridx_dict p -> Ridx_dict (pos p)
  | Ridx_set_element p -> Ridx_set_element (pos p)
  | Rmissing_optional_field (p, n) -> Rmissing_optional_field (pos_or_decl p, n)
  | Runset_field (p, n) -> Runset_field (pos p, n)
  | Rcontravariant_generic (r1, n) ->
    Rcontravariant_generic (map_pos pos pos_or_decl r1, n)
  | Rinvariant_generic (r1, n) ->
    Rcontravariant_generic (map_pos pos pos_or_decl r1, n)
  | Rregex p -> Rregex (pos p)
  | Rimplicit_upper_bound (p, s) -> Rimplicit_upper_bound (pos_or_decl p, s)
  | Rarith_ret_int p -> Rarith_ret_int (pos p)
  | Rarith_ret_float (p, r, s) ->
    Rarith_ret_float (pos p, map_pos pos pos_or_decl r, s)
  | Rarith_ret_num (p, r, s) ->
    Rarith_ret_num (pos p, map_pos pos pos_or_decl r, s)
  | Rarith_dynamic p -> Rarith_dynamic (pos p)
  | Rbitwise_dynamic p -> Rbitwise_dynamic (pos p)
  | Rincdec_dynamic p -> Rincdec_dynamic (pos p)
  | Rtype_variable p -> Rtype_variable (pos p)
  | Rtype_variable_error p -> Rtype_variable_error (pos p)
  | Rtype_variable_generics (p, t, s) -> Rtype_variable_generics (pos p, t, s)
  | Rglobal_type_variable_generics (p, t, s) ->
    Rglobal_type_variable_generics (pos_or_decl p, t, s)
  | Rsolve_fail p -> Rsolve_fail (pos_or_decl p)
  | Rcstr_on_generics (p, sid) ->
    Rcstr_on_generics (pos_or_decl p, positioned_id pos_or_decl sid)
  | Rlambda_param (p, r) -> Rlambda_param (pos p, map_pos pos pos_or_decl r)
  | Rshape (p, fun_name) -> Rshape (pos p, fun_name)
  | Rshape_literal p -> Rshape_literal (pos p)
  | Renforceable p -> Renforceable (pos_or_decl p)
  | Rdestructure p -> Rdestructure (pos p)
  | Rkey_value_collection_key p -> Rkey_value_collection_key (pos p)
  | Rglobal_class_prop p -> Rglobal_class_prop (pos_or_decl p)
  | Rglobal_fun_param p -> Rglobal_fun_param (pos_or_decl p)
  | Rglobal_fun_ret p -> Rglobal_fun_ret (pos_or_decl p)
  | Rsplice p -> Rsplice (pos p)
  | Ret_boolean p -> Ret_boolean (pos p)
  | Rdefault_capability p -> Rdefault_capability (pos_or_decl p)
  | Rconcat_operand p -> Rconcat_operand (pos p)
  | Rinterp_operand p -> Rinterp_operand (pos p)
  | Rdynamic_coercion r -> Rdynamic_coercion (map_pos pos pos_or_decl r)
  | Rsupport_dynamic_type p -> Rsupport_dynamic_type (pos_or_decl p)
  | Rdynamic_partial_enforcement (p, cn, r) ->
    Rdynamic_partial_enforcement (pos_or_decl p, cn, map_pos pos pos_or_decl r)
  | Rrigid_tvar_escape (p, v, w, r) ->
    Rrigid_tvar_escape (pos p, v, w, map_pos pos pos_or_decl r)
  | Ropaque_type_from_module (p, m, r) ->
    Ropaque_type_from_module (pos_or_decl p, m, map_pos pos pos_or_decl r)
  | Rmissing_class p -> Rmissing_class (pos p)
  | Rinvalid -> Rinvalid
  | Rcaptured_like p -> Rcaptured_like (pos p)
  | Rpessimised_inout p -> Rpessimised_inout (pos_or_decl p)
  | Rpessimised_return p -> Rpessimised_return (pos_or_decl p)
  | Rpessimised_prop p -> Rpessimised_prop (pos_or_decl p)
  | Rpessimised_this p -> Rpessimised_this (pos_or_decl p)
  | Runsafe_cast p -> Runsafe_cast (pos p)
  | Rpattern p -> Rpattern (pos p)
  | Rflow (from, into) ->
    Rflow (map_pos pos pos_or_decl from, map_pos pos pos_or_decl into)
  | Rrev t -> Rrev (map_pos pos pos_or_decl t)
  | Rprj (prj, t) -> Rprj (prj, map_pos pos pos_or_decl t)

let to_constructor_string : type ph. ph t_ -> string = function
  | Rnone -> "Rnone"
  | Rwitness _ -> "Rwitness"
  | Rwitness_from_decl _ -> "Rwitness_from_decl"
  | Ridx _ -> "Ridx"
  | Ridx_vector _ -> "Ridx_vector"
  | Ridx_vector_from_decl _ -> "Ridx_vector_from_decl"
  | Rforeach _ -> "Rforeach"
  | Rasyncforeach _ -> "Rasyncforeach"
  | Rarith _ -> "Rarith"
  | Rarith_ret _ -> "Rarith_ret"
  | Rcomp _ -> "Rcomp"
  | Rconcat_ret _ -> "Rconcat_ret"
  | Rlogic_ret _ -> "Rlogic_ret"
  | Rbitwise _ -> "Rbitwise"
  | Rbitwise_ret _ -> "Rbitwise_ret"
  | Rno_return _ -> "Rno_return"
  | Rno_return_async _ -> "Rno_return_async"
  | Rret_fun_kind _ -> "Rret_fun_kind"
  | Rret_fun_kind_from_decl _ -> "Rret_fun_kind_from_decl"
  | Rhint _ -> "Rhint"
  | Rthrow _ -> "Rthrow"
  | Rplaceholder _ -> "Rplaceholder"
  | Rret_div _ -> "Rret_div"
  | Ryield_gen _ -> "Ryield_gen"
  | Ryield_asyncgen _ -> "Ryield_asyncgen"
  | Ryield_asyncnull _ -> "Ryield_asyncnull"
  | Ryield_send _ -> "Ryield_send"
  | Rlost_info _ -> "Rlost_info"
  | Rformat _ -> "Rformat"
  | Rclass_class _ -> "Rclass_class"
  | Runknown_class _ -> "Runknown_class"
  | Rvar_param _ -> "Rvar_param"
  | Rvar_param_from_decl _ -> "Rvar_param_from_decl"
  | Runpack_param _ -> "Runpack_param"
  | Rinout_param _ -> "Rinout_param"
  | Rinstantiate _ -> "Rinstantiate"
  | Rtypeconst _ -> "Rtypeconst"
  | Rtype_access _ -> "Rtype_access"
  | Rexpr_dep_type _ -> "Rexpr_dep_type"
  | Rnullsafe_op _ -> "Rnullsafe_op"
  | Rtconst_no_cstr _ -> "Rtconst_no_cstr"
  | Rpredicated _ -> "Rpredicated"
  | Ris_refinement _ -> "Ris_refinement"
  | Ras_refinement _ -> "Ras_refinement"
  | Requal _ -> "Requal"
  | Rvarray_or_darray_key _ -> "Rvarray_or_darray_key"
  | Rvec_or_dict_key _ -> "Rvec_or_dict_key"
  | Rusing _ -> "Rusing"
  | Rdynamic_prop _ -> "Rdynamic_prop"
  | Rdynamic_call _ -> "Rdynamic_call"
  | Rdynamic_construct _ -> "Rdynamic_construct"
  | Ridx_dict _ -> "Ridx_dict"
  | Ridx_set_element _ -> "Ridx_set_element"
  | Rmissing_optional_field _ -> "Rmissing_optional_field"
  | Runset_field _ -> "Runset_field"
  | Rcontravariant_generic _ -> "Rcontravariant_generic"
  | Rinvariant_generic _ -> "Rinvariant_generic"
  | Rregex _ -> "Rregex"
  | Rimplicit_upper_bound _ -> "Rimplicit_upper_bound"
  | Rarith_ret_num _ -> "Rarith_ret_num"
  | Rarith_ret_float _ -> "Rarith_ret_float"
  | Rarith_ret_int _ -> "Rarith_ret_int"
  | Rarith_dynamic _ -> "Rarith_dynamic"
  | Rbitwise_dynamic _ -> "Rbitwise_dynamic"
  | Rincdec_dynamic _ -> "Rincdec_dynamic"
  | Rtype_variable _ -> "Rtype_variable"
  | Rtype_variable_generics _ -> "Rtype_variable_generics"
  | Rtype_variable_error _ -> "Rtype_variable_error"
  | Rglobal_type_variable_generics _ -> "Rglobal_type_variable_generics"
  | Rsolve_fail _ -> "Rsolve_fail"
  | Rcstr_on_generics _ -> "Rcstr_on_generics"
  | Rlambda_param _ -> "Rlambda_param"
  | Rshape _ -> "Rshape"
  | Rshape_literal _ -> "Rshape_literal"
  | Renforceable _ -> "Renforceable"
  | Rdestructure _ -> "Rdestructure"
  | Rkey_value_collection_key _ -> "Rkey_value_collection_key"
  | Rglobal_class_prop _ -> "Rglobal_class_prop"
  | Rglobal_fun_param _ -> "Rglobal_fun_param"
  | Rglobal_fun_ret _ -> "Rglobal_fun_ret"
  | Rsplice _ -> "Rsplice"
  | Ret_boolean _ -> "Ret_boolean"
  | Rdefault_capability _ -> "Rdefault_capability"
  | Rconcat_operand _ -> "Rconcat_operand"
  | Rinterp_operand _ -> "Rinterp_operand"
  | Rdynamic_coercion _ -> "Rdynamic_coercion"
  | Rsupport_dynamic_type _ -> "Rsupport_dynamic_type"
  | Rdynamic_partial_enforcement _ -> "Rdynamic_partial_enforcement"
  | Rrigid_tvar_escape _ -> "Rrigid_tvar_escape"
  | Ropaque_type_from_module _ -> "Ropaque_type_from_module"
  | Rmissing_class _ -> "Rmissing_class"
  | Rinvalid -> "Rinvalid"
  | Rcaptured_like _ -> "Rcaptured_like"
  | Rpessimised_inout _ -> "Rpessimised_inout"
  | Rpessimised_return _ -> "Rpessimised_return"
  | Rpessimised_prop _ -> "Rpessimised_prop"
  | Runsafe_cast _ -> "Runsafe_cast"
  | Rpattern _ -> "Rpattern"
  | Rflow _ -> "Rflow"
  | Rrev _ -> "Rrev"
  | Rprj _ -> "Rprj"
  | Rmissing_field -> "Rmissing_field"
  | Rpessimised_this _ -> "Rpessimised_this"

let rec pp_t_ : type ph. _ -> ph t_ -> unit =
 fun fmt r ->
  let open_paren () = Format.fprintf fmt "(@[" in
  let close_paren () = Format.fprintf fmt "@])" in
  let open_bracket () = Format.fprintf fmt "[@[" in
  let close_bracket () = Format.fprintf fmt "@]]" in
  let comma_ fmt () = Format.fprintf fmt ",@ " in
  let comma () = comma_ fmt () in
  Format.pp_print_string fmt @@ to_constructor_string r;
  match r with
  | Rnone
  | Rinvalid
  | Rmissing_field ->
    ()
  | _ ->
    Format.fprintf fmt "@ (@[";
    (match r with
    | Rnone
    | Rinvalid
    | Rmissing_field ->
      failwith "already matched"
    | Rtypeconst (r1, (p, s1), (lazy s2), r2) ->
      pp_t_ fmt r1;
      comma ();
      open_paren ();
      Pos_or_decl.pp fmt p;
      comma ();
      Format.pp_print_string fmt s1;
      close_paren ();
      comma ();
      Format.pp_print_string fmt s2;
      comma ();
      pp_t_ fmt r2
    | Rtype_access (r, l) ->
      pp_t_ fmt r;
      comma ();
      open_bracket ();
      Format.pp_print_list
        ~pp_sep:comma_
        (fun fmt (r, (lazy s)) ->
          open_paren ();
          pp_t_ fmt r;
          comma ();
          Format.pp_print_string fmt s;
          close_paren ())
        fmt
        l;
      close_bracket ()
    | Rret_fun_kind (p, fk) ->
      Pos.pp fmt p;
      comma ();
      Ast_defs.pp_fun_kind fmt fk;
      ()
    | Rret_fun_kind_from_decl (p, fk) ->
      Pos_or_decl.pp fmt p;
      comma ();
      Ast_defs.pp_fun_kind fmt fk;
      ()
    | Rinstantiate (r1, s, r2) ->
      pp_t_ fmt r1;
      comma ();
      Format.pp_print_string fmt s;
      comma ();
      pp_t_ fmt r2
    | Rexpr_dep_type (r, p, edtr) ->
      pp_t_ fmt r;
      comma ();
      Pos_or_decl.pp fmt p;
      comma ();
      pp_expr_dep_type_reason fmt edtr
    | Rimplicit_upper_bound (p, s)
    | Rmissing_optional_field (p, s)
    | Rclass_class (p, s) ->
      Pos_or_decl.pp fmt p;
      comma ();
      Format.pp_print_string fmt s;
      ()
    | Rhint p
    | Rwitness_from_decl p
    | Rpessimised_inout p
    | Rpessimised_return p
    | Rpessimised_prop p
    | Rpessimised_this p
    | Rvar_param_from_decl p
    | Rglobal_fun_param p
    | Rglobal_fun_ret p
    | Renforceable p
    | Rsolve_fail p
    | Rvarray_or_darray_key p
    | Rvec_or_dict_key p
    | Rdefault_capability p
    | Ridx_vector_from_decl p
    | Rinout_param p
    | Rsupport_dynamic_type p
    | Rglobal_class_prop p ->
      Pos_or_decl.pp fmt p
    | Rtconst_no_cstr pid -> pp_pos_id fmt pid
    | Rcstr_on_generics (p, pid) ->
      Pos_or_decl.pp fmt p;
      comma ();
      pp_pos_id fmt pid
    | Rglobal_type_variable_generics (p, s1, s2) ->
      Pos_or_decl.pp fmt p;
      comma ();
      Format.pp_print_string fmt s1;
      comma ();
      Format.pp_print_string fmt s2;
      ()
    | Rtype_variable_generics (p, s1, s2) ->
      Pos.pp fmt p;
      comma ();
      Format.pp_print_string fmt s1;
      comma ();
      Format.pp_print_string fmt s2;
      ()
    | Ridx_vector p
    | Rforeach p
    | Rasyncforeach p
    | Rarith p
    | Rarith_ret p
    | Rarith_dynamic p
    | Rcomp p
    | Rconcat_ret p
    | Rlogic_ret p
    | Rbitwise p
    | Rbitwise_ret p
    | Rno_return p
    | Rno_return_async p
    | Rthrow p
    | Rplaceholder p
    | Rret_div p
    | Ryield_gen p
    | Ryield_asyncgen p
    | Ryield_asyncnull p
    | Ryield_send p
    | Runknown_class p
    | Rvar_param p
    | Rnullsafe_op p
    | Ris_refinement p
    | Ras_refinement p
    | Requal p
    | Rusing p
    | Rdynamic_prop p
    | Rdynamic_call p
    | Rdynamic_construct p
    | Ridx_dict p
    | Ridx_set_element p
    | Rregex p
    | Rarith_ret_int p
    | Rbitwise_dynamic p
    | Rincdec_dynamic p
    | Rtype_variable p
    | Rtype_variable_error p
    | Rshape_literal p
    | Rdestructure p
    | Rkey_value_collection_key p
    | Rsplice p
    | Ret_boolean p
    | Rconcat_operand p
    | Rinterp_operand p
    | Rmissing_class p
    | Rwitness p
    | Rcaptured_like p
    | Rpattern p ->
      Pos.pp fmt p
    | Runset_field (p, s)
    | Rshape (p, s)
    | Rpredicated (p, s) ->
      Pos.pp fmt p;
      comma ();
      Format.pp_print_string fmt s
    | Ridx (p, r)
    | Rlambda_param (p, r) ->
      Pos.pp fmt p;
      comma ();
      pp_t_ fmt r
    | Rformat (p, s, r) ->
      Pos.pp fmt p;
      comma ();
      Format.pp_print_string fmt s;
      comma ();
      pp_t_ fmt r
    | Rdynamic_partial_enforcement (p, s, r)
    | Ropaque_type_from_module (p, s, r) ->
      Pos_or_decl.pp fmt p;
      comma ();
      Format.pp_print_string fmt s;
      comma ();
      pp_t_ fmt r
    | Runpack_param (p1, p2, i) ->
      Pos.pp fmt p1;
      comma ();
      Pos_or_decl.pp fmt p2;
      comma ();
      Format.pp_print_int fmt i
    | Rarith_ret_float (p, r, ap)
    | Rarith_ret_num (p, r, ap) ->
      Pos.pp fmt p;
      comma ();
      pp_t_ fmt r;
      comma ();
      pp_arg_position fmt ap
    | Rrigid_tvar_escape (p, s1, s2, r) ->
      Pos.pp fmt p;
      comma ();
      Format.pp_print_string fmt s1;
      comma ();
      Format.pp_print_string fmt s2;
      comma ();
      pp_t_ fmt r
    | Rinvariant_generic (r, s)
    | Rcontravariant_generic (r, s) ->
      pp_t_ fmt r;
      comma ();
      Format.pp_print_string fmt s
    | Rlost_info (s, r, b) ->
      Format.pp_print_string fmt s;
      comma ();
      pp_t_ fmt r;
      comma ();
      pp_blame fmt b
    | Rdynamic_coercion r -> pp_t_ fmt r
    | Runsafe_cast p -> Pos.pp fmt p
    | Rflow (from, _into) -> pp_t_ fmt from
    | Rrev t -> pp_t_ fmt @@ normalize t
    | Rprj (_, t) -> pp_t_ fmt t);
    Format.fprintf fmt "@])"

and show_t_ : type ph. ph t_ -> string = (fun r -> Format.asprintf "%a" pp_t_ r)

let fun_kind_to_json = function
  | Ast_defs.FSync -> Hh_json.JSON_String "FSync"
  | Ast_defs.FAsync -> Hh_json.JSON_String "FAsync"
  | Ast_defs.FGenerator -> Hh_json.JSON_String "FGenerator"
  | Ast_defs.FAsyncGenerator -> Hh_json.JSON_String "FAsyncGenerator"

let rec to_json : type a. a t_ -> Hh_json.json =
 fun t ->
  match t with
  | Rnone -> Hh_json.(JSON_Object [("Rnone", JSON_Array [])])
  | Rwitness pos ->
    Hh_json.(JSON_Object [("Rwitness", JSON_Array [pos_to_json pos])])
  | Rwitness_from_decl pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rwitness_from_decl", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Ridx (pos, r) ->
    Hh_json.(JSON_Object [("Ridx", JSON_Array [pos_to_json pos; to_json r])])
  | Ridx_vector pos ->
    Hh_json.(JSON_Object [("Ridx_vector", JSON_Array [pos_to_json pos])])
  | Ridx_vector_from_decl pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Ridx_vector_from_decl", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rforeach pos ->
    Hh_json.(JSON_Object [("Rforeach", JSON_Array [pos_to_json pos])])
  | Rasyncforeach pos ->
    Hh_json.(JSON_Object [("Rasyncforeach", JSON_Array [pos_to_json pos])])
  | Rarith pos ->
    Hh_json.(JSON_Object [("Rarith", JSON_Array [pos_to_json pos])])
  | Rarith_ret pos ->
    Hh_json.(JSON_Object [("Rarith_ret", JSON_Array [pos_to_json pos])])
  | Rarith_ret_float (pos, r, arg_pos) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rarith_ret_float",
            JSON_Array
              [pos_to_json pos; to_json r; arg_position_to_json arg_pos] );
        ])
  | Rarith_ret_num (pos, r, arg_pos) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rarith_ret_num",
            JSON_Array
              [pos_to_json pos; to_json r; arg_position_to_json arg_pos] );
        ])
  | Rarith_ret_int pos ->
    Hh_json.(JSON_Object [("Rarith_ret_int", JSON_Array [pos_to_json pos])])
  | Rarith_dynamic pos ->
    Hh_json.(JSON_Object [("Rarith_dynamic", JSON_Array [pos_to_json pos])])
  | Rbitwise_dynamic pos ->
    Hh_json.(JSON_Object [("Rbitwise_dynamic", JSON_Array [pos_to_json pos])])
  | Rincdec_dynamic pos ->
    Hh_json.(JSON_Object [("Rincdec_dynamic", JSON_Array [pos_to_json pos])])
  | Rcomp pos -> Hh_json.(JSON_Object [("Rcomp", JSON_Array [pos_to_json pos])])
  | Rconcat_ret pos ->
    Hh_json.(JSON_Object [("Rconcat_ret", JSON_Array [pos_to_json pos])])
  | Rlogic_ret pos ->
    Hh_json.(JSON_Object [("Rlogic_ret", JSON_Array [pos_to_json pos])])
  | Rbitwise pos ->
    Hh_json.(JSON_Object [("Rbitwise", JSON_Array [pos_to_json pos])])
  | Rbitwise_ret pos ->
    Hh_json.(JSON_Object [("Rbitwise_ret", JSON_Array [pos_to_json pos])])
  | Rno_return pos ->
    Hh_json.(JSON_Object [("Rno_return", JSON_Array [pos_to_json pos])])
  | Rno_return_async pos ->
    Hh_json.(JSON_Object [("Rno_return_async", JSON_Array [pos_to_json pos])])
  | Rret_fun_kind (pos, fun_kind) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rret_fun_kind",
            JSON_Array [pos_to_json pos; fun_kind_to_json fun_kind] );
        ])
  | Rret_fun_kind_from_decl (pos_or_decl, fun_kind) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rret_fun_kind_from_decl",
            JSON_Array [Pos_or_decl.json pos_or_decl; fun_kind_to_json fun_kind]
          );
        ])
  | Rhint pos_or_decl ->
    Hh_json.(JSON_Object [("Rhint", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rthrow pos ->
    Hh_json.(JSON_Object [("Rthrow", JSON_Array [pos_to_json pos])])
  | Rplaceholder pos ->
    Hh_json.(JSON_Object [("Rplaceholder", JSON_Array [pos_to_json pos])])
  | Rret_div pos ->
    Hh_json.(JSON_Object [("Rret_div", JSON_Array [pos_to_json pos])])
  | Ryield_gen pos ->
    Hh_json.(JSON_Object [("Ryield_gen", JSON_Array [pos_to_json pos])])
  | Ryield_asyncgen pos ->
    Hh_json.(JSON_Object [("Ryield_asyncgen", JSON_Array [pos_to_json pos])])
  | Ryield_asyncnull pos ->
    Hh_json.(JSON_Object [("Ryield_asyncnull", JSON_Array [pos_to_json pos])])
  | Ryield_send pos ->
    Hh_json.(JSON_Object [("Ryield_send", JSON_Array [pos_to_json pos])])
  | Rlost_info (str, r, blame) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rlost_info",
            JSON_Array [JSON_String str; to_json r; blame_to_json blame] );
        ])
  | Rformat (pos, str, r) ->
    Hh_json.(
      JSON_Object
        [("Rformat", JSON_Array [pos_to_json pos; JSON_String str; to_json r])])
  | Rclass_class (pos_or_decl, str) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rclass_class",
            JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String str] );
        ])
  | Runknown_class pos ->
    Hh_json.(JSON_Object [("Runknown_class", JSON_Array [pos_to_json pos])])
  | Rvar_param pos ->
    Hh_json.(JSON_Object [("Rvar_param", JSON_Array [pos_to_json pos])])
  | Rvar_param_from_decl pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rvar_param_from_decl", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Runpack_param (pos, pos_or_decl, n) ->
    Hh_json.(
      JSON_Object
        [
          ( "Runpack_param",
            JSON_Array
              [
                pos_to_json pos;
                Pos_or_decl.json pos_or_decl;
                JSON_Number (string_of_int n);
              ] );
        ])
  | Rinout_param pos_or_decl ->
    Hh_json.(
      JSON_Object [("Rinout_param", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rinstantiate (r1, str, r2) ->
    Hh_json.(
      JSON_Object
        [("Rinstantiate", JSON_Array [to_json r1; JSON_String str; to_json r2])])
  | Rtypeconst (r1, (pos_or_decl, str), lazy_str, r2) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rtypeconst",
            JSON_Array
              [
                to_json r1;
                JSON_Object
                  [
                    ( "Tuple2",
                      JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String str]
                    );
                  ];
                JSON_String (Lazy.force lazy_str);
                to_json r2;
              ] );
        ])
  | Rtype_access (r, xs) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rtype_access",
            JSON_Array
              [
                to_json r;
                JSON_Array
                  (List.map xs ~f:(fun (r, lazy_str) ->
                       JSON_Object
                         [
                           ( "Tuple2",
                             JSON_Array
                               [to_json r; JSON_String (Lazy.force lazy_str)] );
                         ]));
              ] );
        ])
  | Rexpr_dep_type (r, pos_or_decl, expr_dep_type_reason) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rexpr_dep_type",
            JSON_Array
              [
                to_json r;
                Pos_or_decl.json pos_or_decl;
                expr_dep_type_reason_to_json expr_dep_type_reason;
              ] );
        ])
  | Rnullsafe_op pos ->
    Hh_json.(JSON_Object [("Rnullsafe_op", JSON_Array [pos_to_json pos])])
  | Rtconst_no_cstr pos_id ->
    Hh_json.(
      JSON_Object [("Rtconst_no_cstr", JSON_Array [pos_id_to_json pos_id])])
  | Rpredicated (pos, str) ->
    Hh_json.(
      JSON_Object
        [("Rpredicated", JSON_Array [pos_to_json pos; JSON_String str])])
  | Ris_refinement pos ->
    Hh_json.(JSON_Object [("Ris_refinement", JSON_Array [pos_to_json pos])])
  | Ras_refinement pos ->
    Hh_json.(JSON_Object [("Ras_refinement", JSON_Array [pos_to_json pos])])
  | Requal pos ->
    Hh_json.(JSON_Object [("Requal", JSON_Array [pos_to_json pos])])
  | Rvarray_or_darray_key pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rvarray_or_darray_key", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rvec_or_dict_key pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rvec_or_dict_key", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rusing pos ->
    Hh_json.(JSON_Object [("Rusing", JSON_Array [pos_to_json pos])])
  | Rdynamic_prop pos ->
    Hh_json.(JSON_Object [("Rdynamic_prop", JSON_Array [pos_to_json pos])])
  | Rdynamic_call pos ->
    Hh_json.(JSON_Object [("Rdynamic_call", JSON_Array [pos_to_json pos])])
  | Rdynamic_construct pos ->
    Hh_json.(JSON_Object [("Rdynamic_construct", JSON_Array [pos_to_json pos])])
  | Ridx_dict pos ->
    Hh_json.(JSON_Object [("Ridx_dict", JSON_Array [pos_to_json pos])])
  | Ridx_set_element pos ->
    Hh_json.(JSON_Object [("Ridx_set_element", JSON_Array [pos_to_json pos])])
  | Rmissing_optional_field (pos_or_decl, str) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rmissing_optional_field",
            JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String str] );
        ])
  | Runset_field (pos, str) ->
    Hh_json.(
      JSON_Object
        [("Runset_field", JSON_Array [pos_to_json pos; JSON_String str])])
  | Rcontravariant_generic (r, str) ->
    Hh_json.(
      JSON_Object
        [("Rcontravariant_generic", JSON_Array [to_json r; JSON_String str])])
  | Rinvariant_generic (r, str) ->
    Hh_json.(
      JSON_Object
        [("Rinvariant_generic", JSON_Array [to_json r; JSON_String str])])
  | Rregex pos ->
    Hh_json.(JSON_Object [("Rregex", JSON_Array [pos_to_json pos])])
  | Rimplicit_upper_bound (pos_or_decl, str) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rimplicit_upper_bound",
            JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String str] );
        ])
  | Rtype_variable pos ->
    Hh_json.(JSON_Object [("Rtype_variable", JSON_Array [pos_to_json pos])])
  | Rtype_variable_generics (pos, str1, str2) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rtype_variable_generics",
            JSON_Array [pos_to_json pos; JSON_String str1; JSON_String str2] );
        ])
  | Rtype_variable_error pos ->
    Hh_json.(
      JSON_Object [("Rtype_variable_error", JSON_Array [pos_to_json pos])])
  | Rglobal_type_variable_generics (pos_or_decl, str1, str2) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rglobal_type_variable_generics",
            JSON_Array
              [Pos_or_decl.json pos_or_decl; JSON_String str1; JSON_String str2]
          );
        ])
  | Rsolve_fail pos_or_decl ->
    Hh_json.(
      JSON_Object [("Rsolve_fail", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rcstr_on_generics (pos_or_decl, pos_id) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rcstr_on_generics",
            JSON_Array [Pos_or_decl.json pos_or_decl; pos_id_to_json pos_id] );
        ])
  | Rlambda_param (pos, r) ->
    Hh_json.(
      JSON_Object [("Rlambda_param", JSON_Array [pos_to_json pos; to_json r])])
  | Rshape (pos, str) ->
    Hh_json.(
      JSON_Object [("Rshape", JSON_Array [pos_to_json pos; JSON_String str])])
  | Rshape_literal pos ->
    Hh_json.(JSON_Object [("Rshape_literal", JSON_Array [pos_to_json pos])])
  | Renforceable pos_or_decl ->
    Hh_json.(
      JSON_Object [("Renforceable", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rdestructure pos ->
    Hh_json.(JSON_Object [("Rdestructure", JSON_Array [pos_to_json pos])])
  | Rkey_value_collection_key pos ->
    Hh_json.(
      JSON_Object [("Rkey_value_collection_key", JSON_Array [pos_to_json pos])])
  | Rglobal_class_prop pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rglobal_class_prop", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rglobal_fun_param pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rglobal_fun_param", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rglobal_fun_ret pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rglobal_fun_ret", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rsplice pos ->
    Hh_json.(JSON_Object [("Rsplice", JSON_Array [pos_to_json pos])])
  | Ret_boolean pos ->
    Hh_json.(JSON_Object [("Ret_boolean", JSON_Array [pos_to_json pos])])
  | Rdefault_capability pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rdefault_capability", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rconcat_operand pos ->
    Hh_json.(JSON_Object [("Rconcat_operand", JSON_Array [pos_to_json pos])])
  | Rinterp_operand pos ->
    Hh_json.(JSON_Object [("Rinterp_operand", JSON_Array [pos_to_json pos])])
  | Rdynamic_coercion r ->
    Hh_json.(JSON_Object [("Rdynamic_coercion", JSON_Array [to_json r])])
  | Rsupport_dynamic_type pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rsupport_dynamic_type", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rdynamic_partial_enforcement (pos_or_decl, str, r) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rdynamic_partial_enforcement",
            JSON_Array
              [Pos_or_decl.json pos_or_decl; JSON_String str; to_json r] );
        ])
  | Rrigid_tvar_escape (pos, str1, str2, r) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rrigid_tvar_escape",
            JSON_Array
              [pos_to_json pos; JSON_String str1; JSON_String str2; to_json r]
          );
        ])
  | Ropaque_type_from_module (pos_or_decl, str, r) ->
    Hh_json.(
      JSON_Object
        [
          ( "Ropaque_type_from_module",
            JSON_Array
              [Pos_or_decl.json pos_or_decl; JSON_String str; to_json r] );
        ])
  | Rmissing_class pos ->
    Hh_json.(JSON_Object [("Rmissing_class", JSON_Array [pos_to_json pos])])
  | Rinvalid -> Hh_json.(JSON_Object [("Rinvalid", JSON_Array [])])
  | Rcaptured_like pos ->
    Hh_json.(JSON_Object [("Rcaptured_like", JSON_Array [pos_to_json pos])])
  | Rpessimised_inout pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rpessimised_inout", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rpessimised_return pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rpessimised_return", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Rpessimised_prop pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rpessimised_prop", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Runsafe_cast pos ->
    Hh_json.(JSON_Object [("Runsafe_cast", JSON_Array [pos_to_json pos])])
  | Rpattern pos ->
    Hh_json.(JSON_Object [("Rpattern", JSON_Array [pos_to_json pos])])
  | Rflow (r_from, r_into) ->
    Hh_json.(
      JSON_Object [("Rflow", JSON_Array [to_json r_from; to_json r_into])])
  | Rrev r -> Hh_json.(JSON_Object [("Rrev", JSON_Array [to_json r])])
  | Rprj (prj, r) ->
    Hh_json.(JSON_Object [("Rprj", JSON_Array [prj_to_json prj; to_json r])])
  | Rmissing_field -> Hh_json.(JSON_Object [("Rmissing_field", JSON_Array [])])
  | Rpessimised_this pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Rpessimised_this", JSON_Array [Pos_or_decl.json pos_or_decl])])

let to_pos : type ph. ph t_ -> Pos_or_decl.t =
 fun r ->
  if !Errors.report_pos_from_reason then
    Pos_or_decl.set_from_reason (to_raw_pos r)
  else
    to_raw_pos r

(* Translate a reason to a (pos, string) list, suitable for error_l. This
 * previously returned a string, however the need to return multiple lines with
 * multiple locations meant that it needed to more than convert to a string *)
let rec to_string : type ph. string -> ph t_ -> (Pos_or_decl.t * string) list =
 fun prefix r ->
  let p = to_pos r in
  match r with
  | Rnone -> [(p, prefix)]
  | Rmissing_field -> [(p, prefix)]
  | Rinvalid -> [(p, prefix)]
  | Rwitness _ -> [(p, prefix)]
  | Rwitness_from_decl p -> [(p, prefix)]
  | Ridx (_, r2) ->
    [(p, prefix)]
    @ [
        ( (match r2 with
          | Rnone -> p
          | _ -> to_pos r2),
          "This can only be indexed with integers" );
      ]
  | Ridx_vector _
  | Ridx_vector_from_decl _ ->
    [
      ( p,
        prefix
        ^ " because only `int` can be used to index into a `Vector` or `vec`."
      );
    ]
  | Rforeach _ ->
    [(p, prefix ^ " because this is used in a `foreach` statement")]
  | Rasyncforeach _ ->
    [
      ( p,
        prefix
        ^ " because this is used in a `foreach` statement with `await as`" );
    ]
  | Rarith _ ->
    [(p, prefix ^ " because this is used in an arithmetic operation")]
  | Rarith_ret _ ->
    [(p, prefix ^ " because this is the result of an arithmetic operation")]
  | Rarith_ret_float (_, r, s) ->
    let rec find_last reason =
      match reason with
      | Rarith_ret_float (_, r, _) -> find_last r
      | r -> r
    in
    let r_last = find_last r in
    [
      ( p,
        prefix
        ^ " because this is the result of an arithmetic operation with a `float` as the "
        ^ arg_pos_str s
        ^ " argument." );
    ]
    @ to_string
        "Here is why I think the argument is a `float`: this is a `float`"
        r_last
  | Rarith_ret_num (_, r, s) ->
    let rec find_last reason =
      match reason with
      | Rarith_ret_num (_, r, _) -> find_last r
      | r -> r
    in
    let r_last = find_last r in
    [
      ( p,
        prefix
        ^ " because this is the result of an arithmetic operation with a `num` as the "
        ^ arg_pos_str s
        ^ " argument, and no `float`s." );
    ]
    @ to_string
        "Here is why I think the argument is a `num`: this is a `num`"
        r_last
  | Rarith_ret_int _ ->
    [
      ( p,
        prefix
        ^ " because this is the result of an integer arithmetic operation" );
    ]
  | Rcaptured_like _ ->
    [
      ( p,
        prefix
        ^ " because this is the type of a local that was captured in a closure"
      );
    ]
  | Rpessimised_inout _ ->
    [
      ( p,
        prefix
        ^ " because the type of this inout parameter is implicitly a like-type"
      );
    ]
  | Rpessimised_return _ ->
    [(p, prefix ^ " because the type of this return is implicitly a like-type")]
  | Rpessimised_prop _ ->
    [
      ( p,
        prefix ^ " because the type of this property is implicitly a like-type"
      );
    ]
  | Rpessimised_this _ ->
    [
      ( p,
        prefix
        ^ " from \"as this\" or \"is this\" in a class whose generic parameters (or those of a subclass) are erased at runtime"
      );
    ]
  | Rarith_dynamic _ ->
    [
      ( p,
        prefix
        ^ " because this is the result of an arithmetic operation with two arguments typed `dynamic`"
      );
    ]
  | Rbitwise_dynamic _ ->
    [
      ( p,
        prefix
        ^ " because this is the result of a bitwise operation with all arguments typed `dynamic`"
      );
    ]
  | Rincdec_dynamic _ ->
    [
      ( p,
        prefix
        ^ " because this is the result of an increment/decrement of an argument typed `dynamic`"
      );
    ]
  | Rcomp _ -> [(p, prefix ^ " because this is the result of a comparison")]
  | Rconcat_ret _ ->
    [(p, prefix ^ " because this is the result of a concatenation")]
  | Rlogic_ret _ ->
    [(p, prefix ^ " because this is the result of a logical operation")]
  | Rbitwise _ -> [(p, prefix ^ " because this is used in a bitwise operation")]
  | Rbitwise_ret _ ->
    [(p, prefix ^ " because this is the result of a bitwise operation")]
  | Rno_return _ ->
    [(p, prefix ^ " because this function does not always return a value")]
  | Rno_return_async _ ->
    [(p, prefix ^ " because this function does not always return a value")]
  | Rret_fun_kind (_, kind) ->
    [
      ( p,
        match kind with
        | Ast_defs.FAsyncGenerator ->
          prefix ^ " (result of `async function` containing a `yield`)"
        | Ast_defs.FGenerator ->
          prefix ^ " (result of function containing a `yield`)"
        | Ast_defs.FAsync -> prefix ^ " (result of an `async` function)"
        | Ast_defs.FSync -> prefix );
    ]
  | Rret_fun_kind_from_decl (p, kind) ->
    [
      ( p,
        match kind with
        | Ast_defs.FAsyncGenerator ->
          prefix ^ " (result of `async function` containing a `yield`)"
        | Ast_defs.FGenerator ->
          prefix ^ " (result of function containing a `yield`)"
        | Ast_defs.FAsync -> prefix ^ " (result of an `async` function)"
        | Ast_defs.FSync -> prefix );
    ]
  | Rhint p -> [(p, prefix)]
  | Rthrow _ -> [(p, prefix ^ " because it is used as an exception")]
  | Rplaceholder _ ->
    [(p, prefix ^ " (`$_` is a placeholder variable not meant to be used)")]
  | Rret_div _ -> [(p, prefix ^ " because it is the result of a division `/`")]
  | Ryield_gen _ ->
    [(p, prefix ^ " (result of function with `yield` in the body)")]
  | Ryield_asyncgen _ ->
    [(p, prefix ^ " (result of `async function` with `yield` in the body)")]
  | Ryield_asyncnull _ ->
    [
      ( p,
        prefix
        ^ " because `yield x` is equivalent to `yield null => x` in an `async` function"
      );
    ]
  | Ryield_send _ ->
    [
      ( p,
        prefix
        ^ " (`$generator->send()` can always send a `null` back to a `yield`)"
      );
    ]
  | Rvar_param _ -> [(p, prefix ^ " (variadic argument)")]
  | Rvar_param_from_decl p -> [(p, prefix ^ " (variadic argument)")]
  | Runpack_param _ -> [(p, prefix ^ " (it is unpacked with `...`)")]
  | Rinout_param _ -> [(p, prefix ^ " (`inout` parameter)")]
  | Rnullsafe_op _ -> [(p, prefix ^ " (use of `?->` operator)")]
  | Rlost_info (s, r1, Blame (p2, source_of_loss)) ->
    let s = strip_ns s in
    let cause =
      match source_of_loss with
      | BSlambda -> "by this lambda function"
      | BScall -> "during this call"
      | BSassignment -> "by this assignment"
      | BSout_of_scope -> "because of scope change"
    in
    to_string prefix r1
    @ [
        ( p2 |> Pos_or_decl.of_raw_pos,
          "All the local information about "
          ^ Markdown_lite.md_codify s
          ^ " has been invalidated "
          ^ cause
          ^ ".\nThis is a limitation of the type-checker; use a local if that's the problem."
        );
      ]
  | Rformat (_, s, t) ->
    let s =
      prefix
      ^ " because of the "
      ^ Markdown_lite.md_codify s
      ^ " format specifier"
    in
    (match to_string "" t with
    | [(_, "")] -> [(p, s)]
    | el -> [(p, s)] @ el)
  | Rclass_class (_, s) ->
    [
      ( p,
        prefix
        ^ "; implicitly defined constant `::class` is a string that contains the fully qualified name of "
        ^ (strip_ns s |> Markdown_lite.md_codify) );
    ]
  | Runknown_class _ -> [(p, prefix ^ "; this class name is unknown to Hack")]
  | Rinstantiate (r_orig, generic_name, r_inst) ->
    to_string prefix r_orig
    @ to_string
        ("  via this generic " ^ Markdown_lite.md_codify generic_name)
        r_inst
  | Rtype_variable _ ->
    [(p, prefix ^ " because a type could not be determined here")]
  | Rtype_variable_error _ -> [(p, prefix ^ " because there was another error")]
  | Rtype_variable_generics (_, tp_name, s)
  | Rglobal_type_variable_generics (_, tp_name, s) ->
    [
      ( p,
        prefix
        ^ " because type parameter "
        ^ Markdown_lite.md_codify tp_name
        ^ " of "
        ^ Markdown_lite.md_codify s
        ^ " could not be determined. Please add explicit type parameters to the invocation of "
        ^ Markdown_lite.md_codify s );
    ]
  | Rsolve_fail _ ->
    [(p, prefix ^ " because a type could not be determined here")]
  | Rtypeconst (Rnone, (pos, tconst), (lazy ty_str), r_root) ->
    let prefix =
      if String.equal prefix "" then
        ""
      else
        prefix ^ "\n  "
    in
    [
      ( pos,
        sprintf
          "%sby accessing the type constant %s"
          prefix
          (Markdown_lite.md_codify tconst) );
    ]
    @ to_string ("on " ^ ty_str) r_root
  | Rtypeconst (r_orig, (pos, tconst), (lazy ty_str), r_root) ->
    to_string prefix r_orig
    @ [
        (pos, sprintf "  resulting from accessing the type constant '%s'" tconst);
      ]
    @ to_string ("  on " ^ ty_str) r_root
  | Rtype_access (Rtypeconst (Rnone, _, _, _), (r, _) :: l) ->
    to_string prefix (Rtype_access (r, l))
  | Rtype_access (Rtypeconst (r, _, _, _), x) ->
    to_string prefix (Rtype_access (r, x))
  | Rtype_access (Rtype_access (r, expand2), expand1) ->
    to_string prefix (Rtype_access (r, expand1 @ expand2))
  | Rtype_access (r, []) -> to_string prefix r
  | Rtype_access (r, (r_hd, (lazy tconst)) :: tail) ->
    to_string prefix r
    @ to_string
        ("  resulting from expanding the type constant "
        ^ Markdown_lite.md_codify tconst)
        r_hd
    @ List.concat_map tail ~f:(fun (r, (lazy s)) ->
          to_string
            ("  then expanding the type constant " ^ Markdown_lite.md_codify s)
            r)
  | Rexpr_dep_type (r, p, e) ->
    to_string prefix r @ [(p, "  " ^ expr_dep_type_reason_string e)]
  | Rtconst_no_cstr (_, n) ->
    [(p, prefix ^ " because the type constant " ^ n ^ " lacks a constraint")]
  | Rpredicated (_, f) ->
    [(p, prefix ^ " from the argument to this " ^ f ^ " test")]
  | Ris_refinement _ -> [(p, prefix ^ " from this `is` expression test")]
  | Ras_refinement _ -> [(p, prefix ^ " from this \"as\" assertion")]
  | Requal _ -> [(p, prefix ^ " from this equality test")]
  | Rvarray_or_darray_key _ ->
    [
      ( p,
        "This is varray_or_darray, which requires arraykey-typed keys when used with an array (used like a hashtable)"
      );
    ]
  | Rvec_or_dict_key _ ->
    [(p, "This is vec_or_dict, which requires keys that have type arraykey")]
  | Rusing _ -> [(p, prefix ^ " because it was assigned in a `using` clause")]
  | Rdynamic_prop _ ->
    [(p, prefix ^ ", the result of accessing a property of a `dynamic` type")]
  | Rdynamic_call _ ->
    [(p, prefix ^ ", the result of calling a `dynamic` type as a function")]
  | Rdynamic_construct _ ->
    [
      ( p,
        prefix ^ ", the result of constructing an object with a `dynamic` type"
      );
    ]
  | Ridx_dict _ ->
    [
      ( p,
        prefix
        ^ " because only array keys can be used to index into a `Map`, `dict`, `darray`, `Set`, or `keyset`"
      );
    ]
  | Ridx_set_element _ ->
    [
      ( p,
        prefix
        ^ " because only array keys can be used as elements of `keyset` or `Set`"
      );
    ]
  | Rmissing_optional_field (_, name) ->
    [
      ( p,
        prefix
        ^ " because the field "
        ^ Markdown_lite.md_codify name
        ^ " may be set to any type in this shape" );
    ]
  | Runset_field (_, name) ->
    [
      ( p,
        prefix
        ^ " because the field "
        ^ Markdown_lite.md_codify name
        ^ " was unset here" );
    ]
  | Rcontravariant_generic (r_orig, class_name) ->
    to_string prefix r_orig
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " only allows supertypes (it is contravariant)" );
      ]
  | Rinvariant_generic (r_orig, class_name) ->
    to_string prefix r_orig
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " must match exactly (it is invariant)" );
      ]
  | Rregex _ -> [(p, prefix ^ " resulting from this regex pattern")]
  | Rimplicit_upper_bound (_, cstr) ->
    [
      ( p,
        prefix
        ^ " arising from an implicit "
        ^ Markdown_lite.md_codify ("as " ^ cstr)
        ^ " constraint on this type" );
    ]
  | Rcstr_on_generics _ -> [(p, prefix)]
  (* If type originated with an unannotated lambda parameter with type variable type,
   * suggested annotating the lambda parameter. Otherwise defer to original reason. *)
  | Rlambda_param
      ( _,
        ( Rsolve_fail _ | Rtype_variable_generics _ | Rtype_variable _
        | Rinstantiate _ ) ) ->
    [
      ( p,
        prefix
        ^ " because the type of the lambda parameter could not be determined. "
        ^ "Please add a type hint to the parameter" );
    ]
  | Rlambda_param (_, r_orig) -> to_string prefix r_orig
  | Rshape (_, fun_name) ->
    [
      ( p,
        prefix
        ^ " because "
        ^ Markdown_lite.md_codify fun_name
        ^ " expects a shape" );
    ]
  | Rshape_literal _ -> [(p, prefix)]
  | Renforceable _ -> [(p, prefix ^ " because it is an unenforceable type")]
  | Rdestructure _ ->
    [(p, prefix ^ " resulting from a list destructuring assignment or a splat")]
  | Rkey_value_collection_key _ ->
    [
      (p, "This is a key-value collection, which requires `arraykey`-typed keys");
    ]
  | Rglobal_class_prop p -> [(p, prefix)]
  | Rglobal_fun_param p -> [(p, prefix)]
  | Rglobal_fun_ret p -> [(p, prefix)]
  | Rsplice _ ->
    [
      (p, prefix ^ " because this is being spliced into another Expression Tree");
    ]
  | Ret_boolean _ ->
    [
      ( p,
        prefix
        ^ " because Expression Trees do not allow coercion of truthy values" );
    ]
  | Rdefault_capability _ ->
    [(p, prefix ^ " because the function did not have an explicit context")]
  | Rconcat_operand _ -> [(p, "Expected `string` or `int`")]
  | Rinterp_operand _ -> [(p, "Expected `string` or `int`")]
  | Rdynamic_coercion r -> to_string prefix r
  | Rsupport_dynamic_type _ ->
    [(p, prefix ^ " because method must be callable in a dynamic context")]
  | Rdynamic_partial_enforcement (p, cn, r_orig) ->
    to_string prefix r_orig
    @ [(p, "while allowing dynamic to flow into " ^ Utils.strip_all_ns cn)]
  | Rrigid_tvar_escape (p, what, tvar, r_orig) ->
    let tvar = Markdown_lite.md_codify tvar in
    ( Pos_or_decl.of_raw_pos p,
      prefix ^ " because " ^ tvar ^ " escaped from " ^ what )
    :: to_string ("  where " ^ tvar ^ " originates from") r_orig
  | Ropaque_type_from_module (p, module_, r_orig) ->
    ( p,
      prefix
      ^ " because this is an internal symbol from module "
      ^ module_
      ^ ", which is opaque outside of the module." )
    :: to_string "The type originated from here" r_orig
  | Rmissing_class p ->
    [(Pos_or_decl.of_raw_pos p, prefix ^ " because class was missing")]
  | Runsafe_cast p ->
    let unsafe_cast =
      Markdown_lite.md_codify @@ Utils.strip_ns SN.PseudoFunctions.unsafe_cast
    in
    [
      ( Pos_or_decl.of_raw_pos p,
        prefix
        ^ " because the expression went through a "
        ^ unsafe_cast
        ^ ". The type might be a lie!" );
    ]
  | Rpattern p ->
    [(Pos_or_decl.of_raw_pos p, prefix ^ " because of this pattern")]
  | Rflow (from, _into) -> to_string prefix from
  | Rrev r -> to_string prefix @@ reverse r
  | Rprj (_prj, r) -> to_string prefix r

(* -- Constructors ---------------------------------------------------------- *)

let none = Rnone

let witness p = Rwitness p

let witness_from_decl p = Rwitness_from_decl p

let idx (p, r) = Ridx (p, r)

let idx_vector p = Ridx_vector p

let idx_vector_from_decl p = Ridx_vector_from_decl p

let foreach p = Rforeach p

let asyncforeach p = Rasyncforeach p

let arith p = Rarith p

let arith_ret p = Rarith_ret p

let arith_ret_float (p, r, a) = Rarith_ret_float (p, r, a)

let arith_ret_num (p, r, a) = Rarith_ret_num (p, r, a)

let arith_ret_int p = Rarith_ret_int p

let arith_dynamic p = Rarith_dynamic p

let bitwise_dynamic p = Rbitwise_dynamic p

let incdec_dynamic p = Rincdec_dynamic p

let comp p = Rcomp p

let concat_ret p = Rconcat_ret p

let logic_ret p = Rlogic_ret p

let bitwise p = Rbitwise p

let bitwise_ret p = Rbitwise_ret p

let no_return p = Rno_return p

let no_return_async p = Rno_return_async p

let ret_fun_kind (p, k) = Rret_fun_kind (p, k)

let ret_fun_kind_from_decl (p, k) = Rret_fun_kind_from_decl (p, k)

let hint p = Rhint p

let throw p = Rthrow p

let placeholder p = Rplaceholder p

let ret_div p = Rret_div p

let yield_gen p = Ryield_gen p

let yield_asyncgen p = Ryield_asyncgen p

let yield_asyncnull p = Ryield_asyncnull p

let yield_send p = Ryield_send p

let lost_info (s, r, b) = Rlost_info (s, r, b)

let format (p, s, r) = Rformat (p, s, r)

let class_class (p, s) = Rclass_class (p, s)

let unknown_class p = Runknown_class p

let var_param p = Rvar_param p

let var_param_from_decl p = Rvar_param_from_decl p

let unpack_param (p1, p2, n) = Runpack_param (p1, p2, n)

let inout_param p = Rinout_param p

let instantiate (r1, s, r2) = Rinstantiate (r1, s, r2)

let typeconst (r, p, s, r2) = Rtypeconst (r, p, s, r2)

let type_access (r, l) = Rtype_access (r, l)

let expr_dep_type (r, p, d) = Rexpr_dep_type (r, p, d)

let nullsafe_op p = Rnullsafe_op p

let tconst_no_cstr id = Rtconst_no_cstr id

let predicated (p, s) = Rpredicated (p, s)

let is_refinement p = Ris_refinement p

let as_refinement p = Ras_refinement p

let equal p = Requal p

let varray_or_darray_key p = Rvarray_or_darray_key p

let vec_or_dict_key p = Rvec_or_dict_key p

let using p = Rusing p

let dynamic_prop p = Rdynamic_prop p

let dynamic_call p = Rdynamic_call p

let dynamic_construct p = Rdynamic_construct p

let idx_dict p = Ridx_dict p

let idx_set_element p = Ridx_set_element p

let missing_optional_field (p, s) = Rmissing_optional_field (p, s)

let unset_field (p, s) = Runset_field (p, s)

let contravariant_generic (r, s) = Rcontravariant_generic (r, s)

let invariant_generic (r, s) = Rinvariant_generic (r, s)

let regex p = Rregex p

let implicit_upper_bound (p, s) = Rimplicit_upper_bound (p, s)

let type_variable p = Rtype_variable p

let type_variable_generics (p, n1, n2) = Rtype_variable_generics (p, n1, n2)

let type_variable_error p = Rtype_variable_error p

let global_type_variable_generics (p, n1, n2) =
  Rglobal_type_variable_generics (p, n1, n2)

let solve_fail p = Rsolve_fail p

let cstr_on_generics (p, id) = Rcstr_on_generics (p, id)

let lambda_param (p, r) = Rlambda_param (p, r)

let shape (p, s) = Rshape (p, s)

let shape_literal p = Rshape_literal p

let enforceable p = Renforceable p

let destructure p = Rdestructure p

let key_value_collection_key p = Rkey_value_collection_key p

let global_class_prop p = Rglobal_class_prop p

let global_fun_param p = Rglobal_fun_param p

let global_fun_ret p = Rglobal_fun_ret p

let splice p = Rsplice p

let et_boolean p = Ret_boolean p

let default_capability p = Rdefault_capability p

let concat_operand p = Rconcat_operand p

let interp_operand p = Rinterp_operand p

let dynamic_coercion r = Rdynamic_coercion r

let support_dynamic_type p = Rsupport_dynamic_type p

let dynamic_partial_enforcement (p, s, r) =
  Rdynamic_partial_enforcement (p, s, r)

let rigid_tvar_escape (p, n1, n2, r) = Rrigid_tvar_escape (p, n1, n2, r)

let opaque_type_from_module (p, s, r) = Ropaque_type_from_module (p, s, r)

let missing_class p = Rmissing_class p

let invalid = Rinvalid

let captured_like p = Rcaptured_like p

let pessimised_inout p = Rpessimised_inout p

let pessimised_return p = Rpessimised_return p

let pessimised_prop p = Rpessimised_prop p

let unsafe_cast p = Runsafe_cast p

let pattern p = Rpattern p

let flow ~from ~into = Rflow (from, into)

let reverse r = Rrev r

let prj_symm t ~prj = Rprj (Symm prj, t)

let prj_class t ~nm ~idx ~var = prj_symm t ~prj:(Prj_symm_class (nm, idx, var))

let prj_class_co t ~nm ~idx = prj_class t ~nm ~idx ~var:(Dir Co)

let prj_class_contra t ~nm ~idx = prj_class t ~nm ~idx ~var:(Dir Contra)

let prj_class_inv_co t ~nm ~idx = prj_class t ~nm ~idx ~var:(Inv Co)

let prj_class_inv_contra t ~nm ~idx = prj_class t ~nm ~idx ~var:(Inv Contra)

let prj_newtype t ~nm ~idx ~var =
  prj_symm t ~prj:(Prj_symm_newtype (nm, idx, var))

let prj_newtype_co t ~nm ~idx = prj_newtype t ~nm ~idx ~var:(Dir Co)

let prj_newtype_contra t ~nm ~idx = prj_newtype t ~nm ~idx ~var:(Dir Contra)

let prj_newtype_inv_co t ~nm ~idx = prj_newtype t ~nm ~idx ~var:(Inv Co)

let prj_newtype_inv_contra t ~nm ~idx = prj_newtype t ~nm ~idx ~var:(Inv Contra)

let prj_neg t = prj_symm t ~prj:Prj_symm_neg

let prj_tuple t ~idx = prj_symm t ~prj:(Prj_symm_tuple idx)

let prj_shape t ~lbl ~kind_sub ~kind_super =
  prj_symm t ~prj:(Prj_symm_shape (lbl, kind_sub, kind_super))

let prj_fn_param t ~idx_sub ~idx_super =
  prj_symm t ~prj:(Prj_symm_fn_param (idx_sub, idx_super))

let prj_fn_param_inout_co t ~idx_sub ~idx_super =
  prj_symm t ~prj:(Prj_symm_fn_param_inout (idx_sub, idx_super, Co))

let prj_fn_param_inout_contra t ~idx_sub ~idx_super =
  prj_symm t ~prj:(Prj_symm_fn_param_inout (idx_sub, idx_super, Contra))

let prj_fn_ret t = prj_symm t ~prj:Prj_symm_fn_ret

let prj_asymm_sub t ~prj = Rprj (Asymm (Sub, prj), t)

let prj_asymm_super t ~prj = Rprj (Asymm (Super, prj), t)

let prj_union_sub t = prj_asymm_sub t ~prj:Prj_asymm_union

let prj_union_super t = prj_asymm_super t ~prj:Prj_asymm_union

let prj_inter_sub t = prj_asymm_sub t ~prj:Prj_asymm_inter

let prj_inter_super t = prj_asymm_super t ~prj:Prj_asymm_inter

let prj_neg_sub t = prj_asymm_sub t ~prj:Prj_asymm_neg

let prj_neg_super t = prj_asymm_super t ~prj:Prj_asymm_neg

let missing_field = Rmissing_field

let pessimised_this p = Rpessimised_this p

(* -- Visitor --------------------------------------------------------------- *)
module Visitor = struct
  class map =
    object (this)
      method on_reason r =
        match r with
        | Rtype_access (r, l) ->
          Rtype_access
            ( this#on_reason r,
              List.map l ~f:(fun (r, x) -> (this#on_reason r, this#on_lazy x))
            )
        | Rtypeconst (r1, x, s, r2) ->
          Rtypeconst (this#on_reason r1, x, this#on_lazy s, this#on_reason r2)
        | Rarith_ret_float (x, r, z) -> Rarith_ret_float (x, this#on_reason r, z)
        | Rarith_ret_num (x, r, z) -> Rarith_ret_num (x, this#on_reason r, z)
        | Rlost_info (x, r, z) -> Rlost_info (x, this#on_reason r, z)
        | Rformat (x, y, r) -> Rformat (x, y, this#on_reason r)
        | Rinstantiate (r1, x, r2) ->
          Rinstantiate (this#on_reason r1, x, this#on_reason r2)
        | Rexpr_dep_type (r, y, z) -> Rexpr_dep_type (this#on_reason r, y, z)
        | Rcontravariant_generic (x, y) ->
          Rcontravariant_generic (this#on_reason x, y)
        | Rinvariant_generic (r, y) -> Rinvariant_generic (this#on_reason r, y)
        | Rlambda_param (x, r) -> Rlambda_param (x, this#on_reason r)
        | Rdynamic_coercion r -> Rdynamic_coercion (this#on_reason r)
        | Rdynamic_partial_enforcement (x, y, r) ->
          Rdynamic_partial_enforcement (x, y, this#on_reason r)
        | Rrigid_tvar_escape (x, y, z, r) ->
          Rrigid_tvar_escape (x, y, z, this#on_reason r)
        | Ropaque_type_from_module (x, y, r) ->
          Ropaque_type_from_module (x, y, this#on_reason r)
        | Rnone -> Rnone
        | Rinvalid -> Rinvalid
        | Rmissing_field -> Rmissing_field
        | Rwitness x -> Rwitness x
        | Rwitness_from_decl x -> Rwitness_from_decl x
        | Ridx_vector x -> Ridx_vector x
        | Ridx_vector_from_decl x -> Ridx_vector_from_decl x
        | Rforeach x -> Rforeach x
        | Rasyncforeach x -> Rasyncforeach x
        | Rarith x -> Rarith x
        | Rarith_ret x -> Rarith_ret x
        | Rarith_ret_int x -> Rarith_ret_int x
        | Rarith_dynamic x -> Rarith_dynamic x
        | Rbitwise_dynamic x -> Rbitwise_dynamic x
        | Rincdec_dynamic x -> Rincdec_dynamic x
        | Rcomp x -> Rcomp x
        | Rconcat_ret x -> Rconcat_ret x
        | Rlogic_ret x -> Rlogic_ret x
        | Rbitwise x -> Rbitwise x
        | Rbitwise_ret x -> Rbitwise_ret x
        | Rno_return x -> Rno_return x
        | Rno_return_async x -> Rno_return_async x
        | Rhint x -> Rhint x
        | Rthrow x -> Rthrow x
        | Rplaceholder x -> Rplaceholder x
        | Rret_div x -> Rret_div x
        | Ryield_gen x -> Ryield_gen x
        | Ryield_asyncgen x -> Ryield_asyncgen x
        | Ryield_asyncnull x -> Ryield_asyncnull x
        | Ryield_send x -> Ryield_send x
        | Runknown_class x -> Runknown_class x
        | Rvar_param x -> Rvar_param x
        | Rvar_param_from_decl x -> Rvar_param_from_decl x
        | Rnullsafe_op x -> Rnullsafe_op x
        | Rtconst_no_cstr x -> Rtconst_no_cstr x
        | Ris_refinement x -> Ris_refinement x
        | Ras_refinement x -> Ras_refinement x
        | Requal x -> Requal x
        | Rvarray_or_darray_key x -> Rvarray_or_darray_key x
        | Rvec_or_dict_key x -> Rvec_or_dict_key x
        | Rusing x -> Rusing x
        | Rdynamic_prop x -> Rdynamic_prop x
        | Rdynamic_call x -> Rdynamic_call x
        | Rdynamic_construct x -> Rdynamic_construct x
        | Ridx_dict x -> Ridx_dict x
        | Ridx_set_element x -> Ridx_set_element x
        | Rregex x -> Rregex x
        | Rtype_variable x -> Rtype_variable x
        | Rsolve_fail x -> Rsolve_fail x
        | Rshape_literal x -> Rshape_literal x
        | Renforceable x -> Renforceable x
        | Rdestructure x -> Rdestructure x
        | Rkey_value_collection_key x -> Rkey_value_collection_key x
        | Rglobal_class_prop x -> Rglobal_class_prop x
        | Rglobal_fun_param x -> Rglobal_fun_param x
        | Rglobal_fun_ret x -> Rglobal_fun_ret x
        | Rsplice x -> Rsplice x
        | Ret_boolean x -> Ret_boolean x
        | Rdefault_capability x -> Rdefault_capability x
        | Rconcat_operand x -> Rconcat_operand x
        | Rinterp_operand x -> Rinterp_operand x
        | Rsupport_dynamic_type x -> Rsupport_dynamic_type x
        | Rmissing_class x -> Rmissing_class x
        | Rinout_param x -> Rinout_param x
        | Rtype_variable_error x -> Rtype_variable_error x
        | Ridx (x, y) -> Ridx (x, y)
        | Rret_fun_kind (x, y) -> Rret_fun_kind (x, y)
        | Rret_fun_kind_from_decl (x, y) -> Rret_fun_kind_from_decl (x, y)
        | Rpredicated (x, y) -> Rpredicated (x, y)
        | Rmissing_optional_field (x, y) -> Rmissing_optional_field (x, y)
        | Runset_field (x, y) -> Runset_field (x, y)
        | Rimplicit_upper_bound (x, y) -> Rimplicit_upper_bound (x, y)
        | Rcstr_on_generics (x, y) -> Rcstr_on_generics (x, y)
        | Rshape (x, y) -> Rshape (x, y)
        | Rclass_class (x, y) -> Rclass_class (x, y)
        | Runpack_param (x, y, z) -> Runpack_param (x, y, z)
        | Rtype_variable_generics (x, y, z) -> Rtype_variable_generics (x, y, z)
        | Rglobal_type_variable_generics (x, y, z) ->
          Rglobal_type_variable_generics (x, y, z)
        | Rcaptured_like x -> Rcaptured_like x
        | Rpessimised_inout x -> Rpessimised_inout x
        | Rpessimised_return x -> Rpessimised_return x
        | Rpessimised_prop x -> Rpessimised_prop x
        | Runsafe_cast x -> Runsafe_cast x
        | Rpattern x -> Rpattern x
        | Rflow (from, into) -> Rflow (this#on_reason from, this#on_reason into)
        | Rrev t -> Rrev (this#on_reason t)
        | Rprj (prj, t) -> Rprj (prj, this#on_reason t)
        | Rpessimised_this x -> Rpessimised_this x

      method on_lazy l = l
    end
end

let force_lazy_values r =
  let visitor =
    object
      inherit Visitor.map

      method! on_lazy l = Lazy.force l |> Lazy.from_val
    end
  in
  visitor#on_reason r

(* -- Predicates ------------------------------------------------------------ *)
module Predicates = struct
  let is_opaque_type_from_module r =
    match r with
    | Ropaque_type_from_module _ -> true
    | _ -> false

  let is_none r =
    match r with
    | Rnone -> true
    | _ -> false

  let is_instantiate r =
    match r with
    | Rinstantiate _ -> true
    | _ -> false

  let is_hint r =
    match r with
    | Rhint _ -> true
    | _ -> false

  let unpack_expr_dep_type_opt r =
    match r with
    | Rexpr_dep_type (r, p, e) -> Some (r, p, e)
    | _ -> None

  let unpack_unpack_param_opt r =
    match r with
    | Runpack_param (p, p2, i) -> Some (p, p2, i)
    | _ -> None

  let unpack_cstr_on_generics_opt r =
    match r with
    | Rcstr_on_generics (p, sid) -> Some (p, sid)
    | _ -> None

  let unpack_shape_literal_opt r =
    match r with
    | Rshape_literal p -> Some p
    | _ -> None
end

(* ~~ Path ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type direction =
  | Fwd
  | Bwd

let flip = function
  | Fwd -> Bwd
  | Bwd -> Fwd

type path_elem =
  | Edge of {
      out_of: prj option;
      dir: direction;
      in_to: prj option;
    }
  | Node of locl_phase t_

let edge out_of dir in_to = Edge { out_of; dir; in_to }

type stats = {
  max_depth: int;
  length: int;
  reversals: int;
}

let explain_reversals n =
  if n = 1 then
    "1 reversal"
  else
    Format.sprintf "%d reversals" n

let explain_stats { max_depth; length; reversals } =
  Format.sprintf
    "This error has %d steps, a maximum depth of %d and contains %s."
    length
    max_depth
    (explain_reversals reversals)

let empty_stats = { max_depth = 0; length = 0; reversals = 0 }

let append_stats
    { max_depth = d1; length = l1; reversals = r1 }
    { max_depth = d2; length = l2; reversals = r2 } =
  { max_depth = max d1 d2; length = l1 + l2; reversals = r1 + r2 }

let incr_stats_length t = { t with length = t.length + 1 }

type path = {
  path_elems: path_elem list;
  stats: stats;
}

let bracket prj =
  match prj with
  | Symm _ -> (Some prj, Some prj)
  | Asymm (Sub, _) -> (Some prj, None)
  | Asymm (Super, _) -> (None, Some prj)

(** Convert a normalized [locl_phase t_] into a [path]  *)
let to_path t =
  let rec aux t ~dir ~cur_depth =
    match t with
    | Rrev _ -> failwith "unnormalized reason"
    | Rflow (l, r) ->
      let (prj_ll_opt, elem_l, prj_lr_opt, stats_l, depth_l) =
        aux l ~dir ~cur_depth
      and (prj_rl_opt, elem_r, prj_rr_opt, stats_r, depth_r) =
        aux r ~dir ~cur_depth
      in
      let stats = incr_stats_length @@ append_stats stats_l stats_r in
      ( prj_ll_opt,
        elem_l @ (edge prj_lr_opt dir prj_rl_opt :: elem_r),
        prj_rr_opt,
        stats,
        max depth_l depth_r )
    | Rprj (prj, t) ->
      let (dir, flipped) =
        if prj_is_contra prj then
          (flip dir, true)
        else
          (dir, false)
      in
      let cur_depth = cur_depth + 1 in
      (match aux t ~dir ~cur_depth with
      | (None, elem, None, stats, max_depth) ->
        let (prj_l_opt, prj_r_opt) = bracket prj in
        let stats =
          if flipped then
            { stats with reversals = stats.reversals + 1 }
          else
            stats
        in
        (prj_l_opt, elem, prj_r_opt, stats, max_depth)
      | _ -> failwith "ill formed path")
    | witness -> (None, [Node witness], None, empty_stats, cur_depth)
  in
  match aux t ~dir:Fwd ~cur_depth:0 with
  | (None, path_elems, None, stats, max_depth) ->
    let stats = { stats with max_depth } in
    { path_elems; stats }
  | _ -> failwith "ill-formed path"

(* TODO(mjt) refactor so that extended reasons use a separate type for witnesses
   and ensure we handle all cases statically *)
let rec explain_witness = function
  | Rhint pos -> (pos, "this hint")
  | Ris_refinement pos -> (Pos_or_decl.of_raw_pos pos, "this `is` expression")
  | Rwitness pos -> (Pos_or_decl.of_raw_pos pos, "this expression")
  | Rmissing_field -> (Pos_or_decl.none, "nothing")
  | Rwitness_from_decl pos -> (pos, "this declaration")
  | Rsupport_dynamic_type pos -> (pos, "this function or method ")
  | Rvar_param_from_decl pos -> (pos, "this variadic parameter declaration")
  | Rtype_variable pos -> (Pos_or_decl.of_raw_pos pos, "this type variable")
  | Rcstr_on_generics (pos, _) ->
    (pos, "the constraint on the generic parameter")
  | Rtype_variable_generics (pos, x, y) ->
    (Pos_or_decl.of_raw_pos pos, Format.sprintf "the generic `%s` on `%s`" x y)
  | Rimplicit_upper_bound (pos, nm) ->
    ( pos,
      Format.sprintf
        "the implicit upper bound (`%s`) on the generic parameter"
        nm )
  | Runpack_param (pos, _, _) ->
    (Pos_or_decl.of_raw_pos pos, "this unpacked parameter")
  | Rbitwise pos -> (Pos_or_decl.of_raw_pos pos, "this expression")
  | Rarith pos -> (Pos_or_decl.of_raw_pos pos, "this arithmetic expression")
  | Rlambda_param (pos, _) ->
    (Pos_or_decl.of_raw_pos pos, "this lambda parameter")
  | Rinout_param pos -> (pos, "this inout parameter")
  | Ridx_vector pos -> (Pos_or_decl.of_raw_pos pos, "this index expression")
  | Ridx (pos, _) -> (Pos_or_decl.of_raw_pos pos, "this index expression")
  | Rsplice pos -> (Pos_or_decl.of_raw_pos pos, "this splice expression")
  | Rno_return pos -> (Pos_or_decl.of_raw_pos pos, "this declaration")
  | Rshape_literal pos -> (Pos_or_decl.of_raw_pos pos, "this shape literal")
  | Rtypeconst (_, (pos, _), _, _) -> (pos, "this type constant")
  | Rtype_access (r, _) -> explain_witness r
  | r ->
    (to_raw_pos r, Format.sprintf "this thing (`%s`)" (to_constructor_string r))

let explain_step (edge, node) ~prefix =
  match (edge, node) with
  | ((None, Fwd, None), rhs) ->
    let (pos, expl) = explain_witness rhs in
    (pos, Format.sprintf "%sflows into %s" prefix expl)
  | ((None, Bwd, None), rhs) ->
    let (pos, expl) = explain_witness rhs in
    (pos, Format.sprintf "%sflows from %s" prefix expl)
  | ((Some out_of, Fwd, None), rhs) ->
    let prj_expl = explain_prj_right out_of in
    let (pos, expl) = explain_witness rhs in
    (pos, Format.sprintf "%sflows down into %s %s" prefix expl prj_expl)
  | ((Some out_of, Bwd, None), rhs) ->
    let prj_expl = explain_prj_right out_of in
    let (pos, expl) = explain_witness rhs in
    (pos, Format.sprintf "%sflows down from %s %s" prefix expl prj_expl)
  | ((None, Fwd, Some in_to), rhs) ->
    let prj_expl = explain_prj_left in_to in
    let (pos, expl) = explain_witness rhs in
    (pos, Format.sprintf "%sflows up into %s %s" prefix expl prj_expl)
  | ((None, Bwd, Some in_to), rhs) ->
    let prj_expl = explain_prj_left in_to in
    let (pos, expl) = explain_witness rhs in
    (pos, Format.sprintf "%sflows up from %s %s" prefix expl prj_expl)
  | _ -> failwith "ill-formed path"

let explain_path { path_elems; stats } =
  let rec aux path_elems acc =
    match path_elems with
    | Edge { out_of; dir; in_to } :: Node rhs :: path_elems ->
      let expl =
        explain_step ~prefix:"which itself " ((out_of, dir, in_to), rhs)
      in
      aux path_elems (expl :: acc)
    | [] -> List.rev acc
    | _ -> failwith "ill-formed path"
  in

  match path_elems with
  | Node lhs :: Edge { out_of; dir; in_to } :: Node rhs :: path_elems ->
    let (lhs_pos, lhs_expl) = explain_witness lhs in
    let lhs_expl =
      Format.sprintf "%s\n\nHere's why: %s" (explain_stats stats) lhs_expl
    in
    let (rhs_pos, rhs_expl) =
      explain_step ~prefix:"" ((out_of, dir, in_to), rhs)
    in
    aux path_elems [(rhs_pos, rhs_expl); (lhs_pos, lhs_expl)]
  | _ -> failwith "ill-formed path"

(* ~~ Extended reasons rendering ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

let explain t ~complexity:_ = explain_path @@ to_path @@ normalize t

let debug t =
  explain t ~complexity:0
  @ [
      ( Pos_or_decl.none,
        Format.sprintf "Flow:\n%s"
        @@ Hh_json.json_to_string ~pretty:true
        @@ to_json
        @@ normalize t );
    ]

(* ~~ Aliases ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)
type t = locl_phase t_

let pp : type ph. _ -> ph t_ -> unit = (fun fmt r -> pp_t_ fmt r)

let show r = Format.asprintf "%a" pp r

type decl_t = decl_phase t_

let rec localize : decl_phase t_ -> locl_phase t_ = function
  | Rnone -> Rnone
  | Ridx_vector_from_decl p -> Ridx_vector_from_decl p
  | Rinout_param p -> Rinout_param p
  | Rtypeconst (r1, p, q, r2) -> Rtypeconst (localize r1, p, q, localize r2)
  | Rcstr_on_generics (a, b) -> Rcstr_on_generics (a, b)
  | Rwitness_from_decl p -> Rwitness_from_decl p
  | Rret_fun_kind_from_decl (p, r) -> Rret_fun_kind_from_decl (p, r)
  | Rclass_class (p, r) -> Rclass_class (p, r)
  | Rvar_param_from_decl p -> Rvar_param_from_decl p
  | Rglobal_fun_param p -> Rglobal_fun_param p
  | Renforceable p -> Renforceable p
  | Rimplicit_upper_bound (p, q) -> Rimplicit_upper_bound (p, q)
  | Rsolve_fail p -> Rsolve_fail p
  | Rmissing_optional_field (p, q) -> Rmissing_optional_field (p, q)
  | Rglobal_class_prop p -> Rglobal_class_prop p
  | Rhint p -> Rhint p
  | Rvarray_or_darray_key p -> Rvarray_or_darray_key p
  | Rvec_or_dict_key p -> Rvec_or_dict_key p
  | Rglobal_fun_ret p -> Rglobal_fun_ret p
  | Rinstantiate (r1, s, r2) -> Rinstantiate (localize r1, s, localize r2)
  | Rexpr_dep_type (r, s, t) -> Rexpr_dep_type (localize r, s, t)
  | Rtconst_no_cstr id -> Rtconst_no_cstr id
  | Rdefault_capability p -> Rdefault_capability p
  | Rdynamic_coercion r -> Rdynamic_coercion r
  | Rsupport_dynamic_type p -> Rsupport_dynamic_type p
  | Rglobal_type_variable_generics (p, tp, n) ->
    Rglobal_type_variable_generics (p, tp, n)
  | Rinvalid -> Rinvalid
  | Rpessimised_inout p -> Rpessimised_inout p
  | Rpessimised_return p -> Rpessimised_return p
  | Rpessimised_prop p -> Rpessimised_prop p
  | Rpessimised_this p -> Rpessimised_this p

(* -- ureason --------------------------------------------------------------- *)
type ureason =
  | URnone
  | URassign
  | URassign_inout
  | URhint
  | URreturn
  | URforeach
  | URthrow
  | URvector
  | URkey of string
  | URvalue of string
  | URawait
  | URyield
  | URxhp of string * string  (** Name of XHP class, Name of XHP attribute *)
  | URxhp_spread
  | URindex of string
  | URelement of string
  | URparam
  | URparam_inout
  | URarray_value
  | URpair_value
  | URtuple_access
  | URpair_access
  | URnewtype_cstr
  | URclass_req
  | URenum
  | URenum_include
  | URenum_cstr
  | URenum_underlying
  | URenum_incompatible_cstr
  | URtypeconst_cstr
  | URsubsume_tconst_cstr
  | URsubsume_tconst_assign
  | URclone
  | URusing
  | URstr_concat
  | URstr_interp
  | URdynamic_prop
  | URlabel
[@@deriving show]

let index_array = URindex "array"

let index_tuple = URindex "tuple"

let index_class s = URindex (strip_ns s)

let set_element s = URelement (strip_ns s)

let string_of_ureason = function
  | URnone -> "Typing error"
  | URreturn -> "Invalid return type"
  | URhint -> "Wrong type hint"
  | URassign -> "Invalid assignment"
  | URassign_inout -> "Invalid assignment to an `inout` parameter"
  | URforeach -> "Invalid `foreach`"
  | URthrow -> "Invalid exception"
  | URvector -> "Some elements in this collection are incompatible"
  | URkey s -> "The keys of this `" ^ strip_ns s ^ "` are incompatible"
  | URvalue s -> "The values of this `" ^ strip_ns s ^ "` are incompatible"
  | URawait -> "`await` can only operate on an `Awaitable`"
  | URyield -> "Invalid `yield`"
  | URxhp (cls, attr) ->
    "Invalid xhp value for attribute "
    ^ Markdown_lite.md_codify attr
    ^ " in "
    ^ (strip_ns cls |> Markdown_lite.md_codify)
  | URxhp_spread -> "The attribute spread operator cannot be called on non-XHP"
  | URindex s -> "Invalid index type for this " ^ strip_ns s
  | URelement s -> "Invalid element type for this " ^ strip_ns s
  | URparam -> "Invalid argument"
  | URparam_inout -> "Invalid argument to an `inout` parameter"
  | URarray_value -> "Incompatible field values"
  | URpair_value -> "Incompatible pair values"
  | URtuple_access ->
    "Tuple elements can only be accessed with an integer literal"
  | URpair_access ->
    "Pair elements can only be accessed with an integer literal"
  | URnewtype_cstr -> "Invalid constraint on `newtype`"
  | URclass_req -> "Unable to satisfy trait/interface requirement"
  | URenum -> "Constant does not match the type of the enum it is in"
  | URenum_include -> "Inclusion of enum of incompatible type"
  | URenum_cstr -> "Invalid constraint on enum"
  | URenum_underlying -> "Invalid underlying type for enum"
  | URenum_incompatible_cstr ->
    "Underlying type for enum is incompatible with constraint"
  | URtypeconst_cstr -> "Unable to satisfy constraint on this type constant"
  | URsubsume_tconst_cstr ->
    "The constraint on this type constant is inconsistent with its parent"
  | URsubsume_tconst_assign ->
    "The assigned type of this type constant is inconsistent with its parent"
  | URclone -> "Clone cannot be called on non-object"
  | URusing -> "Using cannot be used on non-disposable expression"
  | URstr_concat ->
    "String concatenation can only be performed on string and int arguments"
  | URstr_interp ->
    "Only string and int values can be used in string interpolation"
  | URdynamic_prop -> "Dynamic access of property"
  | URlabel ->
    "This label is not a valid reference to a member of the given enum class"
