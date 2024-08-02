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

(** When recording the decomposition of a type during inference we want to keep
    track of variance so we can give intuition about the direction of 'flow'.
    In the case of invariant type paramters, we record both the fact that it was
    invariant and the direction in which the error occurred *)
type cstr_variance =
  | Dir of variance_dir
  | Inv of variance_dir
[@@deriving hash]

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

type ctor_kind =
  | Ctor_class
  | Ctor_newtype
[@@deriving hash]

let ctor_kind_to_json = function
  | Ctor_class -> Hh_json.string_ "Ctor_class"
  | Ctor_newtype -> Hh_json.string_ "Ctor_newtype"

(** Symmetric projections are those in which the same decomposition is applied
    to both sub- and supertype during inference *)
type prj_symm =
  | Prj_symm_neg
  | Prj_symm_nullable
  | Prj_symm_ctor of ctor_kind * string * int * cstr_variance
  | Prj_symm_tuple of int
  | Prj_symm_shape of string * field_kind * field_kind
  | Prj_symm_fn_param of int * int
  | Prj_symm_fn_param_inout of int * int * variance_dir
  | Prj_symm_fn_ret
[@@deriving hash]

let reverse_prj_symm = function
  | Prj_symm_shape (nm, kind_sub, kind_sup) ->
    Prj_symm_shape (nm, kind_sup, kind_sub)
  | Prj_symm_fn_param (idx_sub, idx_sup) -> Prj_symm_fn_param (idx_sup, idx_sub)
  | Prj_symm_fn_param_inout (idx_sub, idx_sup, var) ->
    Prj_symm_fn_param_inout (idx_sup, idx_sub, var)
  | t -> t

let prj_symm_to_json = function
  | Prj_symm_neg -> Hh_json.JSON_String "Prj_symm_neg"
  | Prj_symm_nullable -> Hh_json.JSON_String "Prj_symm_nullable"
  | Prj_symm_ctor (ctor_kind, nm, idx, variance) ->
    Hh_json.(
      JSON_Object
        [
          ( "Prj_symm_ctor",
            JSON_Array
              [
                ctor_kind_to_json ctor_kind;
                JSON_String nm;
                int_ idx;
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

(** Asymmetric projections are those in which the same decomposition is applied
    to only one of the sub- or supertype during inference *)
type prj_asymm =
  | Prj_asymm_union
  | Prj_asymm_inter
  | Prj_asymm_neg
  | Prj_asymm_nullable
[@@deriving hash]

let prj_asymm_to_json = function
  | Prj_asymm_union -> Hh_json.JSON_String "Prj_asymm_union"
  | Prj_asymm_inter -> Hh_json.JSON_String "Prj_asymm_inter"
  | Prj_asymm_neg -> Hh_json.JSON_String "Prj_asymm_neg"
  | Prj_asymm_nullable -> Hh_json.JSON_String "Prj_asymm_nullable"

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

let reverse_prj = function
  | Symm prj_symm -> Symm (reverse_prj_symm prj_symm)
  | t -> t

let prj_to_json = function
  | Symm prj_symm ->
    Hh_json.(JSON_Object [("Symm", JSON_Array [prj_symm_to_json prj_symm])])
  | Asymm (side, prj_asymm) ->
    Hh_json.(
      JSON_Object
        [("Asymm", JSON_Array [side_to_json side; prj_asymm_to_json prj_asymm])])

(* ~~ Flow kinds ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type flow_kind =
  | Flow_assign
  | Flow_local
  | Flow_solved
  | Flow_subtype
  | Flow_subtype_toplevel
  | Flow_prj
  | Flow_extends
  | Flow_transitive
  | Flow_fun_return
  | Flow_param_hint
  | Flow_return_expr
  | Flow_upper_bound
  | Flow_lower_bound
[@@deriving hash, show]

let flow_kind_to_json = function
  | Flow_assign -> Hh_json.string_ "Flow_assign"
  | Flow_local -> Hh_json.string_ "Flow_local"
  | Flow_solved -> Hh_json.string_ "Flow_solved"
  | Flow_subtype -> Hh_json.string_ "Flow_subtype"
  | Flow_subtype_toplevel -> Hh_json.string_ "Flow_subtype_toplevel"
  | Flow_prj -> Hh_json.string_ "Flow_prj"
  | Flow_extends -> Hh_json.string_ "Flow_extends"
  | Flow_transitive -> Hh_json.string_ "Flow_transitive"
  | Flow_fun_return -> Hh_json.string_ "Flow_fun_return"
  | Flow_param_hint -> Hh_json.string_ "Flow_param_hint"
  | Flow_return_expr -> Hh_json.string_ "Flow_return_expr"
  | Flow_upper_bound -> Hh_json.string_ "Flow_upper_bound"
  | Flow_lower_bound -> Hh_json.string_ "Flow_lower_bound"

(* ~~ Witnesses ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type witness_locl =
  | Witness of Pos.t
  | Idx_vector of Pos.t
  | Foreach of Pos.t
  | Asyncforeach of Pos.t
  | Arith of Pos.t
  | Arith_ret of Pos.t
  | Arith_ret_int of Pos.t
  | Arith_dynamic of Pos.t
  | Bitwise_dynamic of Pos.t
  | Incdec_dynamic of Pos.t
  | Comp of Pos.t
  | Concat_ret of Pos.t
  | Logic_ret of Pos.t
  | Bitwise of Pos.t
  | Bitwise_ret of Pos.t
  | No_return of Pos.t
  | No_return_async of Pos.t
  | Ret_fun_kind of Pos.t * Ast_defs.fun_kind
  | Throw of Pos.t
  | Placeholder of Pos.t
  | Ret_div of Pos.t
  | Yield_gen of Pos.t
  | Yield_asyncgen of Pos.t
  | Yield_asyncnull of Pos.t
  | Yield_send of Pos.t
  | Unknown_class of Pos.t
  | Var_param of Pos.t
  | Unpack_param of Pos.t * (Pos_or_decl.t[@hash.ignore]) * int
  | Nullsafe_op of Pos.t
  | Predicated of Pos.t * string
  | Is_refinement of Pos.t
  | As_refinement of Pos.t
  | Equal of Pos.t
  | Using of Pos.t
  | Dynamic_prop of Pos.t
  | Dynamic_call of Pos.t
  | Dynamic_construct of Pos.t
  | Idx_dict of Pos.t
  | Idx_set_element of Pos.t
  | Unset_field of Pos.t * string
  | Regex of Pos.t
  | Type_variable of Pos.t
  | Type_variable_generics of Pos.t * string * string
  | Type_variable_error of Pos.t
  | Shape of Pos.t * string
  | Shape_literal of Pos.t
  | Destructure of Pos.t
  | Key_value_collection_key of Pos.t
  | Splice of Pos.t
  | Et_boolean of Pos.t
  | Concat_operand of Pos.t
  | Interp_operand of Pos.t
  | Missing_class of Pos.t
  | Captured_like of Pos.t
  | Unsafe_cast of Pos.t
  | Pattern of Pos.t
[@@deriving hash]

let witness_locl_to_raw_pos = function
  | Witness pos
  | Idx_vector pos
  | Foreach pos
  | Asyncforeach pos
  | Arith pos
  | Arith_ret pos
  | Arith_ret_int pos
  | Arith_dynamic pos
  | Bitwise_dynamic pos
  | Incdec_dynamic pos
  | Comp pos
  | Concat_ret pos
  | Logic_ret pos
  | Bitwise pos
  | Bitwise_ret pos
  | No_return pos
  | No_return_async pos
  | Ret_fun_kind (pos, _)
  | Throw pos
  | Placeholder pos
  | Ret_div pos
  | Yield_gen pos
  | Yield_asyncgen pos
  | Yield_asyncnull pos
  | Yield_send pos
  | Unknown_class pos
  | Var_param pos
  | Unpack_param (pos, _, _)
  | Nullsafe_op pos
  | Predicated (pos, _)
  | Is_refinement pos
  | As_refinement pos
  | Equal pos
  | Using pos
  | Dynamic_prop pos
  | Dynamic_call pos
  | Dynamic_construct pos
  | Idx_dict pos
  | Idx_set_element pos
  | Unset_field (pos, _)
  | Regex pos
  | Type_variable pos
  | Type_variable_generics (pos, _, _)
  | Type_variable_error pos
  | Shape (pos, _)
  | Shape_literal pos
  | Destructure pos
  | Key_value_collection_key pos
  | Splice pos
  | Et_boolean pos
  | Concat_operand pos
  | Interp_operand pos
  | Missing_class pos
  | Captured_like pos
  | Unsafe_cast pos
  | Pattern pos ->
    Pos_or_decl.of_raw_pos pos

let get_pri_witness_locl = function
  | Foreach _ -> 1
  | Witness _ -> 3
  | _ -> 2

let map_pos_witness_locl pos pos_or_decl w =
  match w with
  | Witness p -> Witness (pos p)
  | Idx_vector p -> Idx_vector (pos p)
  | Foreach p -> Foreach (pos p)
  | Asyncforeach p -> Asyncforeach (pos p)
  | Arith p -> Arith (pos p)
  | Arith_ret p -> Arith_ret (pos p)
  | Arith_ret_int p -> Arith_ret_int (pos p)
  | Arith_dynamic p -> Arith_dynamic (pos p)
  | Bitwise_dynamic p -> Bitwise_dynamic (pos p)
  | Incdec_dynamic p -> Incdec_dynamic (pos p)
  | Comp p -> Comp (pos p)
  | Concat_ret p -> Concat_ret (pos p)
  | Logic_ret p -> Logic_ret (pos p)
  | Bitwise p -> Bitwise (pos p)
  | Bitwise_ret p -> Bitwise_ret (pos p)
  | No_return p -> No_return (pos p)
  | No_return_async p -> No_return_async (pos p)
  | Ret_fun_kind (p, k) -> Ret_fun_kind (pos p, k)
  | Throw p -> Throw (pos p)
  | Placeholder p -> Placeholder (pos p)
  | Ret_div p -> Ret_div (pos p)
  | Yield_gen p -> Yield_gen (pos p)
  | Yield_asyncgen p -> Yield_asyncgen (pos p)
  | Yield_asyncnull p -> Yield_asyncnull (pos p)
  | Yield_send p -> Yield_send (pos p)
  | Unknown_class p -> Unknown_class (pos p)
  | Var_param p -> Var_param (pos p)
  | Unpack_param (p1, p2, i) -> Unpack_param (pos p1, pos_or_decl p2, i)
  | Nullsafe_op p -> Nullsafe_op (pos p)
  | Predicated (p, f) -> Predicated (pos p, f)
  | Is_refinement p -> Is_refinement (pos p)
  | As_refinement p -> As_refinement (pos p)
  | Equal p -> Equal (pos p)
  | Using p -> Using (pos p)
  | Dynamic_prop p -> Dynamic_prop (pos p)
  | Dynamic_call p -> Dynamic_call (pos p)
  | Dynamic_construct p -> Dynamic_construct (pos p)
  | Idx_dict p -> Idx_dict (pos p)
  | Idx_set_element p -> Idx_set_element (pos p)
  | Unset_field (p, n) -> Unset_field (pos p, n)
  | Regex p -> Regex (pos p)
  | Type_variable p -> Type_variable (pos p)
  | Type_variable_generics (p, t, s) -> Type_variable_generics (pos p, t, s)
  | Type_variable_error p -> Type_variable_error (pos p)
  | Shape (p, fun_name) -> Shape (pos p, fun_name)
  | Shape_literal p -> Shape_literal (pos p)
  | Destructure p -> Destructure (pos p)
  | Key_value_collection_key p -> Key_value_collection_key (pos p)
  | Splice p -> Splice (pos p)
  | Et_boolean p -> Et_boolean (pos p)
  | Concat_operand p -> Concat_operand (pos p)
  | Interp_operand p -> Interp_operand (pos p)
  | Missing_class p -> Missing_class (pos p)
  | Captured_like p -> Captured_like (pos p)
  | Unsafe_cast p -> Unsafe_cast (pos p)
  | Pattern p -> Pattern (pos p)

let constructor_string_of_witness_locl = function
  | Witness _ -> "Rwitness"
  | Idx_vector _ -> "Ridx_vector"
  | Foreach _ -> "Rforeach"
  | Asyncforeach _ -> "Rasyncforeach"
  | Arith _ -> "Rarith"
  | Arith_ret _ -> "Rarith_ret"
  | Arith_ret_int _ -> "Rarith_ret_int"
  | Arith_dynamic _ -> "Rarith_dynamic"
  | Bitwise_dynamic _ -> "Rbitwise_dynamic"
  | Incdec_dynamic _ -> "Rincdec_dynamic"
  | Comp _ -> "Rcomp"
  | Concat_ret _ -> "Rconcat_ret"
  | Logic_ret _ -> "Rlogic_ret"
  | Bitwise _ -> "Rbitwise"
  | Bitwise_ret _ -> "Rbitwise_ret"
  | No_return _ -> "Rno_return"
  | No_return_async _ -> "Rno_return_async"
  | Ret_fun_kind _ -> "Rret_fun_kind"
  | Throw _ -> "Rthrow"
  | Placeholder _ -> "Rplaceholder"
  | Ret_div _ -> "Rret_div"
  | Yield_gen _ -> "Ryield_gen"
  | Yield_asyncgen _ -> "Ryield_asyncgen"
  | Yield_asyncnull _ -> "Ryield_asyncnull"
  | Yield_send _ -> "Ryield_send"
  | Unknown_class _ -> "Runknown_class"
  | Var_param _ -> "Rvar_param"
  | Unpack_param _ -> "Runpack_param"
  | Nullsafe_op _ -> "Rnullsafe_op"
  | Predicated _ -> "Rpredicated"
  | Is_refinement _ -> "Ris_refinement"
  | As_refinement _ -> "Ras_refinement"
  | Equal _ -> "Requal"
  | Using _ -> "Rusing"
  | Dynamic_prop _ -> "Rdynamic_prop"
  | Dynamic_call _ -> "Rdynamic_call"
  | Dynamic_construct _ -> "Rdynamic_construct"
  | Idx_dict _ -> "Ridx_dict"
  | Idx_set_element _ -> "Ridx_set_element"
  | Unset_field _ -> "Runset_field"
  | Regex _ -> "Rregex"
  | Type_variable _ -> "Rtype_variable"
  | Type_variable_generics _ -> "Rtype_variable_generics"
  | Type_variable_error _ -> "Rtype_variable_error"
  | Shape _ -> "Rshape"
  | Shape_literal _ -> "Rshape_literal"
  | Destructure _ -> "Rdestructure"
  | Key_value_collection_key _ -> "Rkey_value_collection_key"
  | Splice _ -> "Rsplice"
  | Et_boolean _ -> "Ret_boolean"
  | Concat_operand _ -> "Rconcat_operand"
  | Interp_operand _ -> "Rinterp_operand"
  | Missing_class _ -> "Rmissing_class"
  | Captured_like _ -> "Rcaptured_like"
  | Unsafe_cast _ -> "Runsafe_cast"
  | Pattern _ -> "Rpattern"

let pp_witness_locl fmt witness =
  let comma_ fmt () = Format.fprintf fmt ",@ " in
  let comma () = comma_ fmt () in
  Format.pp_print_string fmt @@ constructor_string_of_witness_locl witness;
  Format.fprintf fmt "@ (@[";
  let (_ : unit) =
    match witness with
    | Ret_fun_kind (p, fk) ->
      Pos.pp fmt p;
      comma ();
      Ast_defs.pp_fun_kind fmt fk;
      ()
    | Type_variable_generics (p, s1, s2) ->
      Pos.pp fmt p;
      comma ();
      Format.pp_print_string fmt s1;
      comma ();
      Format.pp_print_string fmt s2;
      ()
    | Idx_vector p
    | Foreach p
    | Asyncforeach p
    | Arith p
    | Arith_ret p
    | Arith_dynamic p
    | Comp p
    | Concat_ret p
    | Logic_ret p
    | Bitwise p
    | Bitwise_ret p
    | No_return p
    | No_return_async p
    | Throw p
    | Placeholder p
    | Ret_div p
    | Yield_gen p
    | Yield_asyncgen p
    | Yield_asyncnull p
    | Yield_send p
    | Unknown_class p
    | Var_param p
    | Nullsafe_op p
    | Is_refinement p
    | As_refinement p
    | Equal p
    | Using p
    | Dynamic_prop p
    | Dynamic_call p
    | Dynamic_construct p
    | Idx_dict p
    | Idx_set_element p
    | Regex p
    | Arith_ret_int p
    | Bitwise_dynamic p
    | Incdec_dynamic p
    | Type_variable p
    | Type_variable_error p
    | Shape_literal p
    | Destructure p
    | Key_value_collection_key p
    | Splice p
    | Et_boolean p
    | Concat_operand p
    | Interp_operand p
    | Missing_class p
    | Witness p
    | Captured_like p
    | Pattern p ->
      Pos.pp fmt p
    | Unset_field (p, s)
    | Shape (p, s)
    | Predicated (p, s) ->
      Pos.pp fmt p;
      comma ();
      Format.pp_print_string fmt s
    | Unpack_param (p1, p2, i) ->
      Pos.pp fmt p1;
      comma ();
      Pos_or_decl.pp fmt p2;
      comma ();
      Format.pp_print_int fmt i
    | Unsafe_cast p -> Pos.pp fmt p
  in
  Format.fprintf fmt "@])"

let fun_kind_to_json = function
  | Ast_defs.FSync -> Hh_json.JSON_String "FSync"
  | Ast_defs.FAsync -> Hh_json.JSON_String "FAsync"
  | Ast_defs.FGenerator -> Hh_json.JSON_String "FGenerator"
  | Ast_defs.FAsyncGenerator -> Hh_json.JSON_String "FAsyncGenerator"

let witness_locl_to_json witness =
  match witness with
  | Witness pos ->
    Hh_json.(JSON_Object [("Witness", JSON_Array [pos_to_json pos])])
  | Idx_vector pos ->
    Hh_json.(JSON_Object [("Idx_vector", JSON_Array [pos_to_json pos])])
  | Foreach pos ->
    Hh_json.(JSON_Object [("Foreach", JSON_Array [pos_to_json pos])])
  | Asyncforeach pos ->
    Hh_json.(JSON_Object [("Asyncforeach", JSON_Array [pos_to_json pos])])
  | Arith pos -> Hh_json.(JSON_Object [("Arith", JSON_Array [pos_to_json pos])])
  | Arith_ret pos ->
    Hh_json.(JSON_Object [("Arith_ret", JSON_Array [pos_to_json pos])])
  | Arith_ret_int pos ->
    Hh_json.(JSON_Object [("Arith_ret_int", JSON_Array [pos_to_json pos])])
  | Arith_dynamic pos ->
    Hh_json.(JSON_Object [("Arith_dynamic", JSON_Array [pos_to_json pos])])
  | Bitwise_dynamic pos ->
    Hh_json.(JSON_Object [("Bitwise_dynamic", JSON_Array [pos_to_json pos])])
  | Incdec_dynamic pos ->
    Hh_json.(JSON_Object [("Incdec_dynamic", JSON_Array [pos_to_json pos])])
  | Comp pos -> Hh_json.(JSON_Object [("Comp", JSON_Array [pos_to_json pos])])
  | Concat_ret pos ->
    Hh_json.(JSON_Object [("Concat_ret", JSON_Array [pos_to_json pos])])
  | Logic_ret pos ->
    Hh_json.(JSON_Object [("Logic_ret", JSON_Array [pos_to_json pos])])
  | Bitwise pos ->
    Hh_json.(JSON_Object [("Bitwise", JSON_Array [pos_to_json pos])])
  | Bitwise_ret pos ->
    Hh_json.(JSON_Object [("Bitwise_ret", JSON_Array [pos_to_json pos])])
  | No_return pos ->
    Hh_json.(JSON_Object [("No_return", JSON_Array [pos_to_json pos])])
  | No_return_async pos ->
    Hh_json.(JSON_Object [("No_return_async", JSON_Array [pos_to_json pos])])
  | Ret_fun_kind (pos, fun_kind) ->
    Hh_json.(
      JSON_Object
        [
          ( "Ret_fun_kind",
            JSON_Array [pos_to_json pos; fun_kind_to_json fun_kind] );
        ])
  | Throw pos -> Hh_json.(JSON_Object [("Throw", JSON_Array [pos_to_json pos])])
  | Placeholder pos ->
    Hh_json.(JSON_Object [("Placeholder", JSON_Array [pos_to_json pos])])
  | Ret_div pos ->
    Hh_json.(JSON_Object [("Ret_div", JSON_Array [pos_to_json pos])])
  | Yield_gen pos ->
    Hh_json.(JSON_Object [("Yield_gen", JSON_Array [pos_to_json pos])])
  | Yield_asyncgen pos ->
    Hh_json.(JSON_Object [("Yield_asyncgen", JSON_Array [pos_to_json pos])])
  | Yield_asyncnull pos ->
    Hh_json.(JSON_Object [("Yield_asyncnull", JSON_Array [pos_to_json pos])])
  | Yield_send pos ->
    Hh_json.(JSON_Object [("Yield_send", JSON_Array [pos_to_json pos])])
  | Unknown_class pos ->
    Hh_json.(JSON_Object [("Unknown_class", JSON_Array [pos_to_json pos])])
  | Var_param pos ->
    Hh_json.(JSON_Object [("Var_param", JSON_Array [pos_to_json pos])])
  | Unpack_param (pos, pos_or_decl, n) ->
    Hh_json.(
      JSON_Object
        [
          ( "Unpack_param",
            JSON_Array
              [
                pos_to_json pos;
                Pos_or_decl.json pos_or_decl;
                JSON_Number (string_of_int n);
              ] );
        ])
  | Nullsafe_op pos ->
    Hh_json.(JSON_Object [("Nullsafe_op", JSON_Array [pos_to_json pos])])
  | Predicated (pos, str) ->
    Hh_json.(
      JSON_Object
        [("Predicated", JSON_Array [pos_to_json pos; JSON_String str])])
  | Is_refinement pos ->
    Hh_json.(JSON_Object [("Is_refinement", JSON_Array [pos_to_json pos])])
  | As_refinement pos ->
    Hh_json.(JSON_Object [("As_refinement", JSON_Array [pos_to_json pos])])
  | Equal pos -> Hh_json.(JSON_Object [("Equal", JSON_Array [pos_to_json pos])])
  | Using pos -> Hh_json.(JSON_Object [("Using", JSON_Array [pos_to_json pos])])
  | Dynamic_prop pos ->
    Hh_json.(JSON_Object [("Dynamic_prop", JSON_Array [pos_to_json pos])])
  | Dynamic_call pos ->
    Hh_json.(JSON_Object [("Dynamic_call", JSON_Array [pos_to_json pos])])
  | Dynamic_construct pos ->
    Hh_json.(JSON_Object [("Dynamic_construct", JSON_Array [pos_to_json pos])])
  | Idx_dict pos ->
    Hh_json.(JSON_Object [("Idx_dict", JSON_Array [pos_to_json pos])])
  | Idx_set_element pos ->
    Hh_json.(JSON_Object [("Idx_set_element", JSON_Array [pos_to_json pos])])
  | Unset_field (pos, str) ->
    Hh_json.(
      JSON_Object
        [("Unset_field", JSON_Array [pos_to_json pos; JSON_String str])])
  | Regex pos -> Hh_json.(JSON_Object [("Regex", JSON_Array [pos_to_json pos])])
  | Type_variable pos ->
    Hh_json.(JSON_Object [("Type_variable", JSON_Array [pos_to_json pos])])
  | Type_variable_generics (pos, str1, str2) ->
    Hh_json.(
      JSON_Object
        [
          ( "Type_variable_generics",
            JSON_Array [pos_to_json pos; JSON_String str1; JSON_String str2] );
        ])
  | Type_variable_error pos ->
    Hh_json.(
      JSON_Object [("Type_variable_error", JSON_Array [pos_to_json pos])])
  | Shape (pos, str) ->
    Hh_json.(
      JSON_Object [("Shape", JSON_Array [pos_to_json pos; JSON_String str])])
  | Shape_literal pos ->
    Hh_json.(JSON_Object [("Shape_literal", JSON_Array [pos_to_json pos])])
  | Destructure pos ->
    Hh_json.(JSON_Object [("Destructure", JSON_Array [pos_to_json pos])])
  | Key_value_collection_key pos ->
    Hh_json.(
      JSON_Object [("Key_value_collection_key", JSON_Array [pos_to_json pos])])
  | Splice pos ->
    Hh_json.(JSON_Object [("Splice", JSON_Array [pos_to_json pos])])
  | Et_boolean pos ->
    Hh_json.(JSON_Object [("Et_boolean", JSON_Array [pos_to_json pos])])
  | Concat_operand pos ->
    Hh_json.(JSON_Object [("Concat_operand", JSON_Array [pos_to_json pos])])
  | Interp_operand pos ->
    Hh_json.(JSON_Object [("Interp_operand", JSON_Array [pos_to_json pos])])
  | Missing_class pos ->
    Hh_json.(JSON_Object [("Missing_class", JSON_Array [pos_to_json pos])])
  | Captured_like pos ->
    Hh_json.(JSON_Object [("Captured_like", JSON_Array [pos_to_json pos])])
  | Unsafe_cast pos ->
    Hh_json.(JSON_Object [("Unsafe_cast", JSON_Array [pos_to_json pos])])
  | Pattern pos ->
    Hh_json.(JSON_Object [("Pattern", JSON_Array [pos_to_json pos])])

let witness_locl_to_string prefix witness =
  match witness with
  | Witness pos -> (Pos_or_decl.of_raw_pos pos, prefix)
  | Idx_vector pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because only `int` can be used to index into a `Vector` or `vec`." )
  | Foreach pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is used in a `foreach` statement" )
  | Asyncforeach pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is used in a `foreach` statement with `await as`"
    )
  | Arith pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is used in an arithmetic operation" )
  | Arith_ret pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is the result of an arithmetic operation" )
  | Arith_ret_int pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is the result of an integer arithmetic operation"
    )
  | Arith_dynamic pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because this is the result of an arithmetic operation with two arguments typed `dynamic`"
    )
  | Bitwise_dynamic pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because this is the result of a bitwise operation with all arguments typed `dynamic`"
    )
  | Incdec_dynamic pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because this is the result of an increment/decrement of an argument typed `dynamic`"
    )
  | Comp pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is the result of a comparison" )
  | Concat_ret pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is the result of a concatenation" )
  | Logic_ret pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is the result of a logical operation" )
  | Bitwise pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is used in a bitwise operation" )
  | Bitwise_ret pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is the result of a bitwise operation" )
  | No_return pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this function does not always return a value" )
  | No_return_async pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this function does not always return a value" )
  | Ret_fun_kind (pos, kind) ->
    ( Pos_or_decl.of_raw_pos pos,
      (match kind with
      | Ast_defs.FAsyncGenerator ->
        prefix ^ " (result of `async function` containing a `yield`)"
      | Ast_defs.FGenerator ->
        prefix ^ " (result of function containing a `yield`)"
      | Ast_defs.FAsync -> prefix ^ " (result of an `async` function)"
      | Ast_defs.FSync -> prefix) )
  | Throw pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " because it is used as an exception")
  | Placeholder pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " (`$_` is a placeholder variable not meant to be used)" )
  | Ret_div pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because it is the result of a division `/`" )
  | Yield_gen pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " (result of function with `yield` in the body)" )
  | Yield_asyncgen pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " (result of `async function` with `yield` in the body)" )
  | Yield_asyncnull pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because `yield x` is equivalent to `yield null => x` in an `async` function"
    )
  | Yield_send pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " (`$generator->send()` can always send a `null` back to a `yield`)" )
  | Unknown_class pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ "; this class name is unknown to Hack")
  | Var_param pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " (variadic argument)")
  | Unpack_param (pos, _, _) ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " (it is unpacked with `...`)")
  | Nullsafe_op pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " (use of `?->` operator)")
  | Predicated (pos, f) ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " from the argument to this " ^ f ^ " test" )
  | Is_refinement pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " from this `is` expression test")
  | As_refinement pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " from this \"as\" assertion")
  | Equal pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " from this equality test")
  | Using pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because it was assigned in a `using` clause" )
  | Dynamic_prop pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ ", the result of accessing a property of a `dynamic` type" )
  | Dynamic_call pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ ", the result of calling a `dynamic` type as a function" )
  | Dynamic_construct pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ ", the result of constructing an object with a `dynamic` type" )
  | Idx_dict pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because only array keys can be used to index into a `Map`, `dict`, `darray`, `Set`, or `keyset`"
    )
  | Idx_set_element pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because only array keys can be used as elements of `keyset` or `Set`"
    )
  | Unset_field (pos, name) ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because the field "
      ^ Markdown_lite.md_codify name
      ^ " was unset here" )
  | Regex pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " resulting from this regex pattern")
  | Type_variable pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because a type could not be determined here" )
  | Type_variable_generics (pos, tp_name, s) ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because type parameter "
      ^ Markdown_lite.md_codify tp_name
      ^ " of "
      ^ Markdown_lite.md_codify s
      ^ " could not be determined. Please add explicit type parameters to the invocation of "
      ^ Markdown_lite.md_codify s )
  | Type_variable_error pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " because there was another error")
  | Shape (pos, fun_name) ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because "
      ^ Markdown_lite.md_codify fun_name
      ^ " expects a shape" )
  | Shape_literal pos -> (Pos_or_decl.of_raw_pos pos, prefix)
  | Destructure pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " resulting from a list destructuring assignment or a splat" )
  | Key_value_collection_key pos ->
    ( Pos_or_decl.of_raw_pos pos,
      "This is a key-value collection, which requires `arraykey`-typed keys" )
  | Splice pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because this is being spliced into another Expression Tree" )
  | Et_boolean pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because Expression Trees do not allow coercion of truthy values" )
  | Concat_operand pos ->
    (Pos_or_decl.of_raw_pos pos, "Expected `string` or `int`")
  | Interp_operand pos ->
    (Pos_or_decl.of_raw_pos pos, "Expected `string` or `int`")
  | Missing_class pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " because class was missing")
  | Captured_like pos ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because this is the type of a local that was captured in a closure" )
  | Unsafe_cast pos ->
    let unsafe_cast =
      Markdown_lite.md_codify @@ Utils.strip_ns SN.PseudoFunctions.unsafe_cast
    in
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because the expression went through a "
      ^ unsafe_cast
      ^ ". The type might be a lie!" )
  | Pattern pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " because of this pattern")

type witness_decl =
  | Witness_from_decl of (Pos_or_decl.t[@hash.ignore])
  | Idx_vector_from_decl of (Pos_or_decl.t[@hash.ignore])
  | Hint of (Pos_or_decl.t[@hash.ignore])
  | Class_class of (Pos_or_decl.t[@hash.ignore]) * string
  | Var_param_from_decl of (Pos_or_decl.t[@hash.ignore])
  | Vec_or_dict_key of (Pos_or_decl.t[@hash.ignore])
  | Ret_fun_kind_from_decl of (Pos_or_decl.t[@hash.ignore]) * Ast_defs.fun_kind
  | Inout_param of (Pos_or_decl.t[@hash.ignore])
  | Tconst_no_cstr of pos_id
  | Varray_or_darray_key of (Pos_or_decl.t[@hash.ignore])
  | Missing_optional_field of (Pos_or_decl.t[@hash.ignore]) * string
  | Implicit_upper_bound of (Pos_or_decl.t[@hash.ignore]) * string
  | Global_type_variable_generics of
      (Pos_or_decl.t[@hash.ignore]) * string * string
  | Solve_fail of (Pos_or_decl.t[@hash.ignore])
  | Cstr_on_generics of (Pos_or_decl.t[@hash.ignore]) * pos_id
  | Enforceable of (Pos_or_decl.t[@hash.ignore])
  | Global_class_prop of (Pos_or_decl.t[@hash.ignore])
  | Global_fun_param of (Pos_or_decl.t[@hash.ignore])
  | Global_fun_ret of (Pos_or_decl.t[@hash.ignore])
  | Default_capability of (Pos_or_decl.t[@hash.ignore])
  | Support_dynamic_type of (Pos_or_decl.t[@hash.ignore])
  | Pessimised_inout of (Pos_or_decl.t[@hash.ignore])
  | Pessimised_return of (Pos_or_decl.t[@hash.ignore])
  | Pessimised_prop of (Pos_or_decl.t[@hash.ignore])
  | Pessimised_this of (Pos_or_decl.t[@hash.ignore])
[@@deriving hash]

let witness_decl_to_raw_pos = function
  | Witness_from_decl pos_or_decl
  | Idx_vector_from_decl pos_or_decl
  | Ret_fun_kind_from_decl (pos_or_decl, _)
  | Hint pos_or_decl
  | Class_class (pos_or_decl, _)
  | Var_param_from_decl pos_or_decl
  | Inout_param pos_or_decl
  | Tconst_no_cstr (pos_or_decl, _)
  | Varray_or_darray_key pos_or_decl
  | Vec_or_dict_key pos_or_decl
  | Missing_optional_field (pos_or_decl, _)
  | Implicit_upper_bound (pos_or_decl, _)
  | Global_type_variable_generics (pos_or_decl, _, _)
  | Solve_fail pos_or_decl
  | Cstr_on_generics (pos_or_decl, _)
  | Enforceable pos_or_decl
  | Global_class_prop pos_or_decl
  | Global_fun_param pos_or_decl
  | Global_fun_ret pos_or_decl
  | Default_capability pos_or_decl
  | Support_dynamic_type pos_or_decl
  | Pessimised_inout pos_or_decl
  | Pessimised_return pos_or_decl
  | Pessimised_prop pos_or_decl
  | Pessimised_this pos_or_decl ->
    pos_or_decl

let map_pos_witness_decl pos_or_decl witness =
  match witness with
  | Witness_from_decl p -> Witness_from_decl (pos_or_decl p)
  | Idx_vector_from_decl p -> Idx_vector_from_decl (pos_or_decl p)
  | Ret_fun_kind_from_decl (p, k) -> Ret_fun_kind_from_decl (pos_or_decl p, k)
  | Hint p -> Hint (pos_or_decl p)
  | Class_class (p, s) -> Class_class (pos_or_decl p, s)
  | Var_param_from_decl p -> Var_param_from_decl (pos_or_decl p)
  | Inout_param p -> Inout_param (pos_or_decl p)
  | Tconst_no_cstr id -> Tconst_no_cstr (positioned_id pos_or_decl id)
  | Varray_or_darray_key p -> Varray_or_darray_key (pos_or_decl p)
  | Vec_or_dict_key p -> Vec_or_dict_key (pos_or_decl p)
  | Missing_optional_field (p, n) -> Missing_optional_field (pos_or_decl p, n)
  | Implicit_upper_bound (p, s) -> Implicit_upper_bound (pos_or_decl p, s)
  | Global_type_variable_generics (p, t, s) ->
    Global_type_variable_generics (pos_or_decl p, t, s)
  | Solve_fail p -> Solve_fail (pos_or_decl p)
  | Cstr_on_generics (p, sid) ->
    Cstr_on_generics (pos_or_decl p, positioned_id pos_or_decl sid)
  | Enforceable p -> Enforceable (pos_or_decl p)
  | Global_class_prop p -> Global_class_prop (pos_or_decl p)
  | Global_fun_param p -> Global_fun_param (pos_or_decl p)
  | Global_fun_ret p -> Global_fun_ret (pos_or_decl p)
  | Default_capability p -> Default_capability (pos_or_decl p)
  | Support_dynamic_type p -> Support_dynamic_type (pos_or_decl p)
  | Pessimised_inout p -> Pessimised_inout (pos_or_decl p)
  | Pessimised_return p -> Pessimised_return (pos_or_decl p)
  | Pessimised_prop p -> Pessimised_prop (pos_or_decl p)
  | Pessimised_this p -> Pessimised_this (pos_or_decl p)

let constructor_string_of_witness_decl = function
  | Witness_from_decl _ -> "Rwitness_from_decl"
  | Idx_vector_from_decl _ -> "Ridx_vector_from_decl"
  | Ret_fun_kind_from_decl _ -> "Rret_fun_kind_from_decl"
  | Hint _ -> "Rhint"
  | Class_class _ -> "Rclass_class"
  | Var_param_from_decl _ -> "Rvar_param_from_decl"
  | Inout_param _ -> "Rinout_param"
  | Tconst_no_cstr _ -> "Rtconst_no_cstr"
  | Varray_or_darray_key _ -> "Rvarray_or_darray_key"
  | Vec_or_dict_key _ -> "Rvec_or_dict_key"
  | Missing_optional_field _ -> "Rmissing_optional_field"
  | Implicit_upper_bound _ -> "Rimplicit_upper_bound"
  | Global_type_variable_generics _ -> "Rglobal_type_variable_generics"
  | Solve_fail _ -> "Rsolve_fail"
  | Cstr_on_generics _ -> "Rcstr_on_generics"
  | Enforceable _ -> "Renforceable"
  | Global_class_prop _ -> "Rglobal_class_prop"
  | Global_fun_param _ -> "Rglobal_fun_param"
  | Global_fun_ret _ -> "Rglobal_fun_ret"
  | Default_capability _ -> "Rdefault_capability"
  | Support_dynamic_type _ -> "Rsupport_dynamic_type"
  | Pessimised_inout _ -> "Rpessimised_inout"
  | Pessimised_return _ -> "Rpessimised_return"
  | Pessimised_prop _ -> "Rpessimised_prop"
  | Pessimised_this _ -> "Rpessimised_this"

let pp_witness_decl fmt witness =
  let comma_ fmt () = Format.fprintf fmt ",@ " in
  let comma () = comma_ fmt () in
  Format.pp_print_string fmt @@ constructor_string_of_witness_decl witness;
  Format.fprintf fmt "@ (@[";
  let (_ : unit) =
    match witness with
    | Ret_fun_kind_from_decl (p, fk) ->
      Pos_or_decl.pp fmt p;
      comma ();
      Ast_defs.pp_fun_kind fmt fk;
      ()
    | Implicit_upper_bound (p, s)
    | Missing_optional_field (p, s)
    | Class_class (p, s) ->
      Pos_or_decl.pp fmt p;
      comma ();
      Format.pp_print_string fmt s;
      ()
    | Hint p
    | Witness_from_decl p
    | Pessimised_inout p
    | Pessimised_return p
    | Pessimised_prop p
    | Pessimised_this p
    | Var_param_from_decl p
    | Global_fun_param p
    | Global_fun_ret p
    | Enforceable p
    | Solve_fail p
    | Varray_or_darray_key p
    | Vec_or_dict_key p
    | Default_capability p
    | Idx_vector_from_decl p
    | Inout_param p
    | Support_dynamic_type p
    | Global_class_prop p ->
      Pos_or_decl.pp fmt p
    | Tconst_no_cstr pid -> pp_pos_id fmt pid
    | Cstr_on_generics (p, pid) ->
      Pos_or_decl.pp fmt p;
      comma ();
      pp_pos_id fmt pid
    | Global_type_variable_generics (p, s1, s2) ->
      Pos_or_decl.pp fmt p;
      comma ();
      Format.pp_print_string fmt s1;
      comma ();
      Format.pp_print_string fmt s2;
      ()
  in
  Format.fprintf fmt "@])"

let fun_kind_to_json = function
  | Ast_defs.FSync -> Hh_json.JSON_String "FSync"
  | Ast_defs.FAsync -> Hh_json.JSON_String "FAsync"
  | Ast_defs.FGenerator -> Hh_json.JSON_String "FGenerator"
  | Ast_defs.FAsyncGenerator -> Hh_json.JSON_String "FAsyncGenerator"

let witness_decl_to_json = function
  | Witness_from_decl pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Witness_from_decl", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Idx_vector_from_decl pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Idx_vector_from_decl", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Ret_fun_kind_from_decl (pos_or_decl, fun_kind) ->
    Hh_json.(
      JSON_Object
        [
          ( "Ret_fun_kind_from_decl",
            JSON_Array [Pos_or_decl.json pos_or_decl; fun_kind_to_json fun_kind]
          );
        ])
  | Hint pos_or_decl ->
    Hh_json.(JSON_Object [("Hint", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Class_class (pos_or_decl, str) ->
    Hh_json.(
      JSON_Object
        [
          ( "Class_class",
            JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String str] );
        ])
  | Var_param_from_decl pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Var_param_from_decl", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Inout_param pos_or_decl ->
    Hh_json.(
      JSON_Object [("Inout_param", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Tconst_no_cstr pos_id ->
    Hh_json.(
      JSON_Object [("Tconst_no_cstr", JSON_Array [pos_id_to_json pos_id])])
  | Varray_or_darray_key pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Varray_or_darray_key", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Vec_or_dict_key pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Vec_or_dict_key", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Missing_optional_field (pos_or_decl, str) ->
    Hh_json.(
      JSON_Object
        [
          ( "Missing_optional_field",
            JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String str] );
        ])
  | Implicit_upper_bound (pos_or_decl, str) ->
    Hh_json.(
      JSON_Object
        [
          ( "Implicit_upper_bound",
            JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String str] );
        ])
  | Global_type_variable_generics (pos_or_decl, str1, str2) ->
    Hh_json.(
      JSON_Object
        [
          ( "Global_type_variable_generics",
            JSON_Array
              [Pos_or_decl.json pos_or_decl; JSON_String str1; JSON_String str2]
          );
        ])
  | Solve_fail pos_or_decl ->
    Hh_json.(
      JSON_Object [("Solve_fail", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Cstr_on_generics (pos_or_decl, pos_id) ->
    Hh_json.(
      JSON_Object
        [
          ( "Cstr_on_generics",
            JSON_Array [Pos_or_decl.json pos_or_decl; pos_id_to_json pos_id] );
        ])
  | Enforceable pos_or_decl ->
    Hh_json.(
      JSON_Object [("Enforceable", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Global_class_prop pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Global_class_prop", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Global_fun_param pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Global_fun_param", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Global_fun_ret pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Global_fun_ret", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Default_capability pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Default_capability", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Support_dynamic_type pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Support_dynamic_type", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Pessimised_inout pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Pessimised_inout", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Pessimised_return pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Pessimised_return", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Pessimised_prop pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Pessimised_prop", JSON_Array [Pos_or_decl.json pos_or_decl])])
  | Pessimised_this pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Pessimised_this", JSON_Array [Pos_or_decl.json pos_or_decl])])

let witness_decl_to_string prefix witness =
  match witness with
  | Witness_from_decl pos_or_decl -> (pos_or_decl, prefix)
  | Idx_vector_from_decl pos_or_decl ->
    ( pos_or_decl,
      prefix
      ^ " because only `int` can be used to index into a `Vector` or `vec`." )
  | Ret_fun_kind_from_decl (pos_or_decl, kind) ->
    ( pos_or_decl,
      (match kind with
      | Ast_defs.FAsyncGenerator ->
        prefix ^ " (result of `async function` containing a `yield`)"
      | Ast_defs.FGenerator ->
        prefix ^ " (result of function containing a `yield`)"
      | Ast_defs.FAsync -> prefix ^ " (result of an `async` function)"
      | Ast_defs.FSync -> prefix) )
  | Hint pos_or_decl -> (pos_or_decl, prefix)
  | Class_class (pos_or_decl, s) ->
    ( pos_or_decl,
      prefix
      ^ "; implicitly defined constant `::class` is a string that contains the fully qualified name of "
      ^ (strip_ns s |> Markdown_lite.md_codify) )
  | Var_param_from_decl pos_or_decl ->
    (pos_or_decl, prefix ^ " (variadic argument)")
  | Inout_param pos_or_decl -> (pos_or_decl, prefix ^ " (`inout` parameter)")
  | Tconst_no_cstr (pos_or_decl, n) ->
    ( pos_or_decl,
      prefix ^ " because the type constant " ^ n ^ " lacks a constraint" )
  | Varray_or_darray_key pos_or_decl ->
    ( pos_or_decl,
      "This is varray_or_darray, which requires arraykey-typed keys when used with an array (used like a hashtable)"
    )
  | Vec_or_dict_key pos_or_decl ->
    ( pos_or_decl,
      "This is vec_or_dict, which requires keys that have type arraykey" )
  | Missing_optional_field (pos_or_decl, name) ->
    ( pos_or_decl,
      prefix
      ^ " because the field "
      ^ Markdown_lite.md_codify name
      ^ " may be set to any type in this shape" )
  | Implicit_upper_bound (pos_or_decl, cstr) ->
    ( pos_or_decl,
      prefix
      ^ " arising from an implicit "
      ^ Markdown_lite.md_codify ("as " ^ cstr)
      ^ " constraint on this type" )
  | Global_type_variable_generics (pos_or_decl, tp_name, s) ->
    ( pos_or_decl,
      prefix
      ^ " because type parameter "
      ^ Markdown_lite.md_codify tp_name
      ^ " of "
      ^ Markdown_lite.md_codify s
      ^ " could not be determined. Please add explicit type parameters to the invocation of "
      ^ Markdown_lite.md_codify s )
  | Solve_fail pos_or_decl ->
    (pos_or_decl, prefix ^ " because a type could not be determined here")
  | Cstr_on_generics (pos_or_decl, _) -> (pos_or_decl, prefix)
  | Enforceable pos_or_decl ->
    (pos_or_decl, prefix ^ " because it is an unenforceable type")
  | Global_class_prop pos_or_decl -> (pos_or_decl, prefix)
  | Global_fun_param pos_or_decl -> (pos_or_decl, prefix)
  | Global_fun_ret pos_or_decl -> (pos_or_decl, prefix)
  | Default_capability pos_or_decl ->
    ( pos_or_decl,
      prefix ^ " because the function did not have an explicit context" )
  | Support_dynamic_type pos_or_decl ->
    ( pos_or_decl,
      prefix ^ " because method must be callable in a dynamic context" )
  | Pessimised_inout pos_or_decl ->
    ( pos_or_decl,
      prefix
      ^ " because the type of this inout parameter is implicitly a like-type" )
  | Pessimised_return pos_or_decl ->
    ( pos_or_decl,
      prefix ^ " because the type of this return is implicitly a like-type" )
  | Pessimised_prop pos_or_decl ->
    ( pos_or_decl,
      prefix ^ " because the type of this property is implicitly a like-type" )
  | Pessimised_this pos_or_decl ->
    ( pos_or_decl,
      prefix
      ^ " from \"as this\" or \"is this\" in a class whose generic parameters (or those of a subclass) are erased at runtime"
    )

(* ~~ Reasons ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

(* The phase below helps enforce that only Pos_or_decl.t positions end up in the heap.
 * To enforce that, any reason taking a Pos.t should be a locl_phase t_
 * to prevent a decl ty from using it.
 * Reasons used for decl types should be 'phase t_ so that they can be localized
 * to be used in the localized version of the type. *)

(** The reason why something is expected to have a certain type *)
type _ t_ =
  (* -- Superset of reasons used by decl provider -- *)
  | From_witness_decl : witness_decl -> 'phase t_
  | Instantiate : 'phase t_ * string * 'phase t_ -> 'phase t_
  (* -- Used when generating substitutions in silent mode only -- *)
  | No_reason : 'phase t_
  (* -- Core flow constructors -- *)
  | From_witness_locl : witness_locl -> locl_phase t_
  | Flow : locl_phase t_ * flow_kind * locl_phase t_ -> locl_phase t_
  | Prj : prj * locl_phase t_ -> locl_phase t_
  | Rev : locl_phase t_ -> locl_phase t_
  | Def : (Pos_or_decl.t[@hash.ignore]) * locl_phase t_ -> locl_phase t_
  (* -- Invalid markers -- *)
  | Invalid : 'phase t_
  | Missing_field : locl_phase t_
  (* -- Legacy non-witness reasons -- *)
  | Idx : Pos.t * locl_phase t_ -> locl_phase t_
      (** Used as an index into a vector-like
          array or string. Position of indexing,
          reason for the indexed type *)
  | Arith_ret_float : Pos.t * locl_phase t_ * arg_position -> locl_phase t_
      (** pos, arg float typing reason, arg position *)
  | Arith_ret_num : Pos.t * locl_phase t_ * arg_position -> locl_phase t_
      (** pos, arg num typing reason, arg position *)
  | Lost_info : string * locl_phase t_ * blame -> locl_phase t_
  | Format : Pos.t * string * locl_phase t_ -> locl_phase t_
  | Typeconst :
      'phase t_
      * ((Pos_or_decl.t[@hash.ignore]) * string)
      * string Lazy.t
      * 'phase t_
      -> 'phase t_
  | Type_access :
      locl_phase t_ * (locl_phase t_ * string Lazy.t) list
      -> locl_phase t_
  | Expr_dep_type :
      'phase t_ * (Pos_or_decl.t[@hash.ignore]) * expr_dep_type_reason
      -> 'phase t_
  | Contravariant_generic : locl_phase t_ * string -> locl_phase t_
  | Invariant_generic : locl_phase t_ * string -> locl_phase t_
  | Lambda_param : Pos.t * locl_phase t_ -> locl_phase t_
  | Dynamic_coercion : locl_phase t_ -> locl_phase t_
  | Dynamic_partial_enforcement :
      (Pos_or_decl.t[@hash.ignore]) * string * locl_phase t_
      -> locl_phase t_
  | Rigid_tvar_escape : Pos.t * string * string * locl_phase t_ -> locl_phase t_
  | Opaque_type_from_module :
      (Pos_or_decl.t[@hash.ignore]) * string * locl_phase t_
      -> locl_phase t_
[@@deriving hash]

(** Perform actual reversal of reason flows *)
let rec normalize : locl_phase t_ -> locl_phase t_ =
 fun t ->
  match t with
  | Flow (t1, kind, t2) -> Flow (normalize t1, kind, normalize t2)
  | Rev t -> reverse t
  | Prj (prj, t) -> Prj (prj, normalize t)
  | Def (def, t) -> Def (def, normalize t)
  | Idx (pos, t) -> Idx (pos, normalize t)
  | Arith_ret_float (pos, t, arg_pos) ->
    Arith_ret_float (pos, normalize t, arg_pos)
  | Arith_ret_num (pos, t, arg_pos) -> Arith_ret_num (pos, normalize t, arg_pos)
  | Lost_info (str, t, blame) -> Lost_info (str, normalize t, blame)
  | Format (pos, str, t) -> Format (pos, str, normalize t)
  | Instantiate (t1, str, t2) -> Instantiate (normalize t1, str, normalize t2)
  | Typeconst (t1, cls, lzy_str, t2) ->
    Typeconst (normalize t1, cls, lzy_str, normalize t2)
  | Type_access (t, ts) ->
    Type_access
      (normalize t, List.map ts ~f:(fun (t, str) -> (normalize t, str)))
  | Expr_dep_type (t, pos_or_decl, expr_dep_ty_reason) ->
    Expr_dep_type (normalize t, pos_or_decl, expr_dep_ty_reason)
  | Contravariant_generic (t, str) -> Contravariant_generic (normalize t, str)
  | Invariant_generic (t, str) -> Invariant_generic (normalize t, str)
  | Lambda_param (pos, t) -> Lambda_param (pos, normalize t)
  | Dynamic_coercion t -> Dynamic_coercion (normalize t)
  | Dynamic_partial_enforcement (pos_or_decl, str, t) ->
    Dynamic_partial_enforcement (pos_or_decl, str, normalize t)
  | Rigid_tvar_escape (pos, str1, str2, t) ->
    Rigid_tvar_escape (pos, str1, str2, normalize t)
  | Opaque_type_from_module (pos_or_decl, str, t) ->
    Opaque_type_from_module (pos_or_decl, str, normalize t)
  | No_reason
  | Invalid
  | Missing_field
  | From_witness_locl _
  | From_witness_decl _ ->
    t

and reverse : locl_phase t_ -> locl_phase t_ =
 fun t ->
  match t with
  | Flow (t1, kind, t2) -> Flow (reverse t2, kind, reverse t1)
  | Rev t -> normalize t
  | Prj (prj, t) -> Prj (reverse_prj prj, reverse t)
  | Def (def, t) -> Def (def, reverse t)
  | Idx (pos, t) -> Idx (pos, reverse t)
  | Arith_ret_float (pos, t, arg_pos) ->
    Arith_ret_float (pos, reverse t, arg_pos)
  | Arith_ret_num (pos, t, arg_pos) -> Arith_ret_num (pos, reverse t, arg_pos)
  | Lost_info (str, t, blame) -> Lost_info (str, reverse t, blame)
  | Format (pos, str, t) -> Format (pos, str, reverse t)
  | Instantiate (t1, str, t2) -> Instantiate (reverse t1, str, reverse t2)
  | Typeconst (t1, cls, lzy_str, t2) ->
    Typeconst (reverse t1, cls, lzy_str, reverse t2)
  | Type_access (t, ts) ->
    Type_access (reverse t, List.map ts ~f:(fun (t, str) -> (reverse t, str)))
  | Expr_dep_type (t, pos_or_decl, expr_dep_ty_reason) ->
    Expr_dep_type (reverse t, pos_or_decl, expr_dep_ty_reason)
  | Contravariant_generic (t, str) -> Contravariant_generic (reverse t, str)
  | Invariant_generic (t, str) -> Invariant_generic (reverse t, str)
  | Lambda_param (pos, t) -> Lambda_param (pos, reverse t)
  | Dynamic_coercion t -> Dynamic_coercion (reverse t)
  | Dynamic_partial_enforcement (pos_or_decl, str, t) ->
    Dynamic_partial_enforcement (pos_or_decl, str, reverse t)
  | Rigid_tvar_escape (pos, str1, str2, t) ->
    Rigid_tvar_escape (pos, str1, str2, reverse t)
  | Opaque_type_from_module (pos_or_decl, str, t) ->
    Opaque_type_from_module (pos_or_decl, str, reverse t)
  | No_reason
  | Invalid
  | Missing_field
  | From_witness_locl _
  | From_witness_decl _ ->
    t

let rec to_raw_pos : type ph. ph t_ -> Pos_or_decl.t =
 fun r ->
  match r with
  | No_reason
  | Invalid
  | Missing_field ->
    Pos_or_decl.none
  | From_witness_locl witness_locl -> witness_locl_to_raw_pos witness_locl
  | From_witness_decl witness_decl -> witness_decl_to_raw_pos witness_decl
  (* Deal with legacy reasons which are witness-like but contain other reasons *)
  | Idx (pos, _)
  | Arith_ret_float (pos, _, _)
  | Arith_ret_num (pos, _, _)
  | Format (pos, _, _)
  | Lambda_param (pos, _)
  | Rigid_tvar_escape (pos, _, _, _) ->
    Pos_or_decl.of_raw_pos pos
  | Dynamic_partial_enforcement (pos_or_decl, _, _)
  (* TODO(mjt) why are we putting [No_reason] here? *)
  | Typeconst (No_reason, (pos_or_decl, _), _, _)
  | Opaque_type_from_module (pos_or_decl, _, _) ->
    pos_or_decl
  (* Recurse into flow-like reasons to find the position of the leftmost witness *)
  | Flow (t, _, _)
  | Prj (_, t)
  | Def (_, t)
  | Lost_info (_, t, _)
  | Type_access (t, _)
  | Invariant_generic (t, _)
  | Contravariant_generic (t, _)
  | Dynamic_coercion t ->
    to_raw_pos t
  | Expr_dep_type (t, _, _) -> to_raw_pos t
  | Typeconst (t, _, _, _) -> to_raw_pos t
  | Instantiate (_, _, t) -> to_raw_pos t
  (* Reversal means we want the rightmost witness *)
  | Rev t -> to_raw_pos_rev t

and to_raw_pos_rev : type ph. ph t_ -> Pos_or_decl.t =
 fun r ->
  match r with
  | No_reason
  | Invalid
  | Missing_field ->
    Pos_or_decl.none
  | From_witness_locl witness -> witness_locl_to_raw_pos witness
  | From_witness_decl witness -> witness_decl_to_raw_pos witness
  (* Deal with legacy reasons which are witness-like but contain other reasons *)
  | Idx (pos, _)
  | Arith_ret_float (pos, _, _)
  | Arith_ret_num (pos, _, _)
  | Format (pos, _, _)
  | Rigid_tvar_escape (pos, _, _, _)
  | Lambda_param (pos, _) ->
    Pos_or_decl.of_raw_pos pos
  | Dynamic_partial_enforcement (pos_or_decl, _, _)
  | Opaque_type_from_module (pos_or_decl, _, _) ->
    pos_or_decl
  (* Carry the reversal through these constructors to find the rightmost witness *)
  | Prj (_, t)
  | Def (_, t)
  | Flow (_, _, t)
  | Lost_info (_, t, _)
  | Type_access (t, _)
  | Invariant_generic (t, _)
  | Contravariant_generic (t, _)
  | Dynamic_coercion t ->
    to_raw_pos_rev t
  | Instantiate (_, _, t)
  | Typeconst (t, _, _, _)
  | Expr_dep_type (t, _, _) ->
    to_raw_pos_rev t
  (* Reversal means we want the leftmost witness *)
  | Rev t -> to_raw_pos t

let get_pri : type ph. ph t_ -> int = function
  | No_reason -> 0
  | From_witness_locl witness -> get_pri_witness_locl witness
  | From_witness_decl _ -> 2
  | Lost_info _ -> 5
  | _ -> 2

let compare : type phase. phase t_ -> phase t_ -> int =
 fun r1 r2 ->
  let d = get_pri r2 - get_pri r1 in
  if Int.( <> ) d 0 then
    d
  else
    Pos_or_decl.compare (to_raw_pos r1) (to_raw_pos r2)

let rec map_pos :
    type ph.
    (Pos.t -> Pos.t) -> (Pos_or_decl.t -> Pos_or_decl.t) -> ph t_ -> ph t_ =
 fun pos pos_or_decl -> function
  | No_reason -> No_reason
  | Missing_field -> Missing_field
  | From_witness_locl witness ->
    From_witness_locl (map_pos_witness_locl pos pos_or_decl witness)
  | From_witness_decl witness ->
    From_witness_decl (map_pos_witness_decl pos_or_decl witness)
  | Idx (p, r) -> Idx (pos p, map_pos pos pos_or_decl r)
  | Lost_info (s, r1, Blame (p2, l)) ->
    Lost_info (s, map_pos pos pos_or_decl r1, Blame (pos p2, l))
  | Format (p1, s, r) -> Format (pos p1, s, map_pos pos pos_or_decl r)
  | Instantiate (r1, x, r2) ->
    Instantiate (map_pos pos pos_or_decl r1, x, map_pos pos pos_or_decl r2)
  | Typeconst (r1, (p, s1), s2, r2) ->
    Typeconst
      ( map_pos pos pos_or_decl r1,
        (pos_or_decl p, s1),
        s2,
        map_pos pos pos_or_decl r2 )
  | Type_access (r1, ls) ->
    Type_access
      ( map_pos pos pos_or_decl r1,
        List.map ls ~f:(fun (r, s) -> (map_pos pos pos_or_decl r, s)) )
  | Expr_dep_type (r, p, n) ->
    Expr_dep_type (map_pos pos pos_or_decl r, pos_or_decl p, n)
  | Lambda_param (p, r) -> Lambda_param (pos p, map_pos pos pos_or_decl r)
  | Contravariant_generic (r1, n) ->
    Contravariant_generic (map_pos pos pos_or_decl r1, n)
  | Invariant_generic (r1, n) ->
    Contravariant_generic (map_pos pos pos_or_decl r1, n)
  | Arith_ret_float (p, r, s) ->
    Arith_ret_float (pos p, map_pos pos pos_or_decl r, s)
  | Arith_ret_num (p, r, s) ->
    Arith_ret_num (pos p, map_pos pos pos_or_decl r, s)
  | Dynamic_coercion r -> Dynamic_coercion (map_pos pos pos_or_decl r)
  | Dynamic_partial_enforcement (p, cn, r) ->
    Dynamic_partial_enforcement (pos_or_decl p, cn, map_pos pos pos_or_decl r)
  | Rigid_tvar_escape (p, v, w, r) ->
    Rigid_tvar_escape (pos p, v, w, map_pos pos pos_or_decl r)
  | Opaque_type_from_module (p, m, r) ->
    Opaque_type_from_module (pos_or_decl p, m, map_pos pos pos_or_decl r)
  | Invalid -> Invalid
  | Flow (from, kind, into) ->
    Flow (map_pos pos pos_or_decl from, kind, map_pos pos pos_or_decl into)
  | Rev t -> Rev (map_pos pos pos_or_decl t)
  | Prj (prj, t) -> Prj (prj, map_pos pos pos_or_decl t)
  | Def (def, of_) -> Def (pos_or_decl def, map_pos pos pos_or_decl of_)

let to_constructor_string : type ph. ph t_ -> string = function
  | No_reason -> "Rnone"
  | Invalid -> "Rinvalid"
  | Missing_field -> "Rmissing_field"
  | From_witness_locl witness -> constructor_string_of_witness_locl witness
  | From_witness_decl witness -> constructor_string_of_witness_decl witness
  | Flow _ -> "Rflow"
  | Prj _ -> "Rprj"
  | Rev _ -> "Rrev"
  | Def _ -> "Rdef"
  | Idx _ -> "Ridx"
  | Arith_ret_float _ -> "Rarith_ret_float"
  | Arith_ret_num _ -> "Rarith_ret_num"
  | Lost_info _ -> "Rlost_info"
  | Format _ -> "Rformat"
  | Instantiate _ -> "Rinstantiate"
  | Typeconst _ -> "Rtypeconst"
  | Type_access _ -> "Rtype_access"
  | Expr_dep_type _ -> "Rexpr_dep_type"
  | Contravariant_generic _ -> "Rcontravariant_generic"
  | Invariant_generic _ -> "Rinvariant_generic"
  | Dynamic_coercion _ -> "Rdynamic_coercion"
  | Dynamic_partial_enforcement _ -> "Rdynamic_partial_enforcement"
  | Rigid_tvar_escape _ -> "Rrigid_tvar_escape"
  | Opaque_type_from_module _ -> "Ropaque_type_from_module"
  | Lambda_param _ -> "Rlambda_param"

let rec pp_t_ : type ph. _ -> ph t_ -> unit =
 fun fmt r ->
  let open_paren () = Format.fprintf fmt "(@[" in
  let close_paren () = Format.fprintf fmt "@])" in
  let open_bracket () = Format.fprintf fmt "[@[" in
  let close_bracket () = Format.fprintf fmt "@]]" in
  let comma_ fmt () = Format.fprintf fmt ",@ " in
  let comma () = comma_ fmt () in
  match r with
  | No_reason
  | Invalid
  | Missing_field ->
    Format.pp_print_string fmt @@ to_constructor_string r
  | From_witness_locl witness -> pp_witness_locl fmt witness
  | From_witness_decl witness -> pp_witness_decl fmt witness
  | _ ->
    Format.pp_print_string fmt @@ to_constructor_string r;
    Format.fprintf fmt "@ (@[";
    (match r with
    | No_reason
    | Invalid
    | Missing_field
    | From_witness_locl _
    | From_witness_decl _ ->
      failwith "already matched"
    | Typeconst (r1, (p, s1), (lazy s2), r2) ->
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
    | Type_access (r, l) ->
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
    | Instantiate (r1, s, r2) ->
      pp_t_ fmt r1;
      comma ();
      Format.pp_print_string fmt s;
      comma ();
      pp_t_ fmt r2
    | Expr_dep_type (r, p, edtr) ->
      pp_t_ fmt r;
      comma ();
      Pos_or_decl.pp fmt p;
      comma ();
      pp_expr_dep_type_reason fmt edtr
    | Idx (p, r)
    | Lambda_param (p, r) ->
      Pos.pp fmt p;
      comma ();
      pp_t_ fmt r
    | Format (p, s, r) ->
      Pos.pp fmt p;
      comma ();
      Format.pp_print_string fmt s;
      comma ();
      pp_t_ fmt r
    | Dynamic_partial_enforcement (p, s, r)
    | Opaque_type_from_module (p, s, r) ->
      Pos_or_decl.pp fmt p;
      comma ();
      Format.pp_print_string fmt s;
      comma ();
      pp_t_ fmt r
    | Arith_ret_float (p, r, ap)
    | Arith_ret_num (p, r, ap) ->
      Pos.pp fmt p;
      comma ();
      pp_t_ fmt r;
      comma ();
      pp_arg_position fmt ap
    | Rigid_tvar_escape (p, s1, s2, r) ->
      Pos.pp fmt p;
      comma ();
      Format.pp_print_string fmt s1;
      comma ();
      Format.pp_print_string fmt s2;
      comma ();
      pp_t_ fmt r
    | Invariant_generic (r, s)
    | Contravariant_generic (r, s) ->
      pp_t_ fmt r;
      comma ();
      Format.pp_print_string fmt s
    | Lost_info (s, r, b) ->
      Format.pp_print_string fmt s;
      comma ();
      pp_t_ fmt r;
      comma ();
      pp_blame fmt b
    | Dynamic_coercion r -> pp_t_ fmt r
    | Flow (from, _kind, _into) -> pp_t_ fmt from
    | Rev t -> pp_t_ fmt @@ normalize t
    | Prj (_, t) -> pp_t_ fmt t
    | Def (_, t) -> pp_t_ fmt t);
    Format.fprintf fmt "@])"

and show_t_ : type ph. ph t_ -> string = (fun r -> Format.asprintf "%a" pp_t_ r)

let rec to_json_help : type a. a t_ -> Hh_json.json list -> Hh_json.json list =
 fun t acc ->
  match t with
  | No_reason -> Hh_json.(JSON_Object [("No_reason", JSON_Array [])]) :: acc
  | Missing_field ->
    Hh_json.(JSON_Object [("Missing_field", JSON_Array [])]) :: acc
  | Invalid -> Hh_json.(JSON_Object [("Invalid", JSON_Array [])]) :: acc
  | From_witness_locl witness -> witness_locl_to_json witness :: acc
  | From_witness_decl witness -> witness_decl_to_json witness :: acc
  | Idx (pos, r) ->
    Hh_json.(JSON_Object [("Idx", JSON_Array [pos_to_json pos; to_json r])])
    :: acc
  | Arith_ret_float (pos, r, arg_pos) ->
    Hh_json.(
      JSON_Object
        [
          ( "Arith_ret_float",
            JSON_Array
              [pos_to_json pos; to_json r; arg_position_to_json arg_pos] );
        ])
    :: acc
  | Arith_ret_num (pos, r, arg_pos) ->
    Hh_json.(
      JSON_Object
        [
          ( "Arith_ret_num",
            JSON_Array
              [pos_to_json pos; to_json r; arg_position_to_json arg_pos] );
        ])
    :: acc
  | Lost_info (str, r, blame) ->
    Hh_json.(
      JSON_Object
        [
          ( "Lost_info",
            JSON_Array [JSON_String str; to_json r; blame_to_json blame] );
        ])
    :: acc
  | Format (pos, str, r) ->
    Hh_json.(
      JSON_Object
        [("Format", JSON_Array [pos_to_json pos; JSON_String str; to_json r])])
    :: acc
  | Instantiate (r1, str, r2) ->
    Hh_json.(
      JSON_Object
        [("Instantiate", JSON_Array [to_json r1; JSON_String str; to_json r2])])
    :: acc
  | Typeconst (r1, (pos_or_decl, str), lazy_str, r2) ->
    Hh_json.(
      JSON_Object
        [
          ( "Typeconst",
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
    :: acc
  | Type_access (r, xs) ->
    Hh_json.(
      JSON_Object
        [
          ( "Type_access",
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
    :: acc
  | Expr_dep_type (r, pos_or_decl, expr_dep_type_reason) ->
    Hh_json.(
      JSON_Object
        [
          ( "Expr_dep_type",
            JSON_Array
              [
                to_json r;
                Pos_or_decl.json pos_or_decl;
                expr_dep_type_reason_to_json expr_dep_type_reason;
              ] );
        ])
    :: acc
  | Lambda_param (pos, r) ->
    Hh_json.(
      JSON_Object [("Lambda_param", JSON_Array [pos_to_json pos; to_json r])])
    :: acc
  | Contravariant_generic (r, str) ->
    Hh_json.(
      JSON_Object
        [("Contravariant_generic", JSON_Array [to_json r; JSON_String str])])
    :: acc
  | Invariant_generic (r, str) ->
    Hh_json.(
      JSON_Object
        [("Invariant_generic", JSON_Array [to_json r; JSON_String str])])
    :: acc
  | Dynamic_coercion r ->
    Hh_json.(JSON_Object [("Dynamic_coercion", JSON_Array [to_json r])]) :: acc
  | Dynamic_partial_enforcement (pos_or_decl, str, r) ->
    Hh_json.(
      JSON_Object
        [
          ( "Dynamic_partial_enforcement",
            JSON_Array
              [Pos_or_decl.json pos_or_decl; JSON_String str; to_json r] );
        ])
    :: acc
  | Rigid_tvar_escape (pos, str1, str2, r) ->
    Hh_json.(
      JSON_Object
        [
          ( "Rigid_tvar_escape",
            JSON_Array
              [pos_to_json pos; JSON_String str1; JSON_String str2; to_json r]
          );
        ])
    :: acc
  | Opaque_type_from_module (pos_or_decl, str, r) ->
    Hh_json.(
      JSON_Object
        [
          ( "Opaque_type_from_module",
            JSON_Array
              [Pos_or_decl.json pos_or_decl; JSON_String str; to_json r] );
        ])
    :: acc
  | Flow (r_from, kind, r_into) ->
    to_json_help r_into (flow_kind_to_json kind :: to_json_help r_from acc)
  | Rev r -> Hh_json.(JSON_Object [("Rev", JSON_Array [to_json r])]) :: acc
  | Def (def, r) ->
    Hh_json.(
      JSON_Object [("Def", JSON_Array [Pos_or_decl.json def; to_json r])])
    :: acc
  | Prj (prj, r) ->
    Hh_json.(JSON_Object [("Prj", JSON_Array [prj_to_json prj; to_json r])])
    :: acc

and to_json : type a. a t_ -> Hh_json.json =
 fun t ->
  let acc = to_json_help t [] in
  Hh_json.JSON_Array (List.rev acc)

let to_pos : type ph. ph t_ -> Pos_or_decl.t =
 fun r ->
  if !Errors.report_pos_from_reason then
    Pos_or_decl.set_from_reason (to_raw_pos r)
  else
    to_raw_pos r

(* Translate a reason to a (pos, string) list, suitable for error_l. This
 * previously returned a string, however the need to return multiple lines with
 * multiple locations meant that it needed to more than convert to a string *)
let rec to_string : type a. string -> a t_ -> (Pos_or_decl.t * string) list =
 fun prefix r ->
  let p = to_pos r in
  match r with
  | No_reason -> [(p, prefix)]
  | Missing_field -> [(p, prefix)]
  | Invalid -> [(p, prefix)]
  | From_witness_locl witness -> [witness_locl_to_string prefix witness]
  | From_witness_decl witness -> [witness_decl_to_string prefix witness]
  | Flow (r, _, _)
  | Def (_, r)
  | Prj (_, r) ->
    to_string prefix r
  | Rev r -> to_rev_string prefix r
  | Idx (_, r2) ->
    [(p, prefix)]
    @ [
        ( (match r2 with
          | No_reason -> p
          | _ -> to_pos r2),
          "This can only be indexed with integers" );
      ]
  | Arith_ret_float (_, r, s) ->
    let rec find_last reason =
      match reason with
      | Arith_ret_float (_, r, _) -> find_last r
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
  | Arith_ret_num (_, r, s) ->
    let rec find_last reason =
      match reason with
      | Arith_ret_num (_, r, _) -> find_last r
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
  | Lost_info (s, r1, Blame (p2, source_of_loss)) ->
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
  | Format (_, s, t) ->
    let s =
      prefix
      ^ " because of the "
      ^ Markdown_lite.md_codify s
      ^ " format specifier"
    in
    (match to_string "" t with
    | [(_, "")] -> [(p, s)]
    | el -> [(p, s)] @ el)
  | Instantiate (r_orig, generic_name, r_inst) ->
    to_string prefix r_orig
    @ to_string
        ("  via this generic " ^ Markdown_lite.md_codify generic_name)
        r_inst
  | Typeconst (No_reason, (pos, tconst), (lazy ty_str), r_root) ->
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
  | Typeconst (r_orig, (pos, tconst), (lazy ty_str), r_root) ->
    to_string prefix r_orig
    @ [
        (pos, sprintf "  resulting from accessing the type constant '%s'" tconst);
      ]
    @ to_string ("  on " ^ ty_str) r_root
  | Type_access (Typeconst (No_reason, _, _, _), (r, _) :: l) ->
    to_string prefix (Type_access (r, l))
  | Type_access (Typeconst (r, _, _, _), x) ->
    to_string prefix (Type_access (r, x))
  | Type_access (Type_access (r, expand2), expand1) ->
    to_string prefix (Type_access (r, expand1 @ expand2))
  | Type_access (r, []) -> to_string prefix r
  | Type_access (r, (r_hd, (lazy tconst)) :: tail) ->
    to_string prefix r
    @ to_string
        ("  resulting from expanding the type constant "
        ^ Markdown_lite.md_codify tconst)
        r_hd
    @ List.concat_map tail ~f:(fun (r, (lazy s)) ->
          to_string
            ("  then expanding the type constant " ^ Markdown_lite.md_codify s)
            r)
  | Expr_dep_type (r, p, e) ->
    to_string prefix r @ [(p, "  " ^ expr_dep_type_reason_string e)]
  | Contravariant_generic (r_orig, class_name) ->
    to_string prefix r_orig
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " only allows supertypes (it is contravariant)" );
      ]
  | Invariant_generic (r_orig, class_name) ->
    to_string prefix r_orig
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " must match exactly (it is invariant)" );
      ]
  (* If type originated with an unannotated lambda parameter with type variable type,
   * suggested annotating the lambda parameter. Otherwise defer to original reason. *)
  | Lambda_param
      ( _,
        ( From_witness_decl (Solve_fail _)
        | From_witness_locl (Type_variable_generics _ | Type_variable _)
        | Instantiate _ ) ) ->
    [
      ( p,
        prefix
        ^ " because the type of the lambda parameter could not be determined. "
        ^ "Please add a type hint to the parameter" );
    ]
  | Lambda_param (_, r_orig) -> to_string prefix r_orig
  | Dynamic_coercion r -> to_string prefix r
  | Dynamic_partial_enforcement (p, cn, r_orig) ->
    to_string prefix r_orig
    @ [(p, "while allowing dynamic to flow into " ^ Utils.strip_all_ns cn)]
  | Rigid_tvar_escape (p, what, tvar, r_orig) ->
    let tvar = Markdown_lite.md_codify tvar in
    ( Pos_or_decl.of_raw_pos p,
      prefix ^ " because " ^ tvar ^ " escaped from " ^ what )
    :: to_string ("  where " ^ tvar ^ " originates from") r_orig
  | Opaque_type_from_module (p, module_, r_orig) ->
    ( p,
      prefix
      ^ " because this is an internal symbol from module "
      ^ module_
      ^ ", which is opaque outside of the module." )
    :: to_string "The type originated from here" r_orig

and to_rev_string : type ph. string -> ph t_ -> (Pos_or_decl.t * string) list =
 fun prefix r ->
  let p = to_pos r in
  match r with
  | No_reason -> [(p, prefix)]
  | Missing_field -> [(p, prefix)]
  | Invalid -> [(p, prefix)]
  | From_witness_locl witness -> [witness_locl_to_string prefix witness]
  | From_witness_decl witness -> [witness_decl_to_string prefix witness]
  | Flow (_, _, r)
  | Def (_, r)
  | Prj (_, r) ->
    to_rev_string prefix r
  | Rev r -> to_rev_string prefix r
  | Idx (_, r2) ->
    [(p, prefix)]
    @ [
        ( (match r2 with
          | No_reason -> p
          | _ -> to_pos r2),
          "This can only be indexed with integers" );
      ]
  | Arith_ret_float (_, r, s) ->
    let rec find_last reason =
      match reason with
      | Arith_ret_float (_, r, _) -> find_last r
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
    @ to_rev_string
        "Here is why I think the argument is a `float`: this is a `float`"
        r_last
  | Arith_ret_num (_, r, s) ->
    let rec find_last reason =
      match reason with
      | Arith_ret_num (_, r, _) -> find_last r
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
    @ to_rev_string
        "Here is why I think the argument is a `num`: this is a `num`"
        r_last
  | Lost_info (s, r1, Blame (p2, source_of_loss)) ->
    let s = strip_ns s in
    let cause =
      match source_of_loss with
      | BSlambda -> "by this lambda function"
      | BScall -> "during this call"
      | BSassignment -> "by this assignment"
      | BSout_of_scope -> "because of scope change"
    in
    to_rev_string prefix r1
    @ [
        ( p2 |> Pos_or_decl.of_raw_pos,
          "All the local information about "
          ^ Markdown_lite.md_codify s
          ^ " has been invalidated "
          ^ cause
          ^ ".\nThis is a limitation of the type-checker; use a local if that's the problem."
        );
      ]
  | Format (_, s, t) ->
    let s =
      prefix
      ^ " because of the "
      ^ Markdown_lite.md_codify s
      ^ " format specifier"
    in
    (match to_rev_string "" t with
    | [(_, "")] -> [(p, s)]
    | el -> [(p, s)] @ el)
  | Instantiate (r_orig, generic_name, r_inst) ->
    to_rev_string prefix r_orig
    @ to_rev_string
        ("  via this generic " ^ Markdown_lite.md_codify generic_name)
        r_inst
  | Typeconst (No_reason, (pos, tconst), (lazy ty_str), r_root) ->
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
    @ to_rev_string ("on " ^ ty_str) r_root
  | Typeconst (r_orig, (pos, tconst), (lazy ty_str), r_root) ->
    to_rev_string prefix r_orig
    @ [
        (pos, sprintf "  resulting from accessing the type constant '%s'" tconst);
      ]
    @ to_rev_string ("  on " ^ ty_str) r_root
  | Type_access (Typeconst (No_reason, _, _, _), (r, _) :: l) ->
    to_rev_string prefix (Type_access (r, l))
  | Type_access (Typeconst (r, _, _, _), x) ->
    to_rev_string prefix (Type_access (r, x))
  | Type_access (Type_access (r, expand2), expand1) ->
    to_rev_string prefix (Type_access (r, expand1 @ expand2))
  | Type_access (r, []) -> to_rev_string prefix r
  | Type_access (r, (r_hd, (lazy tconst)) :: tail) ->
    to_rev_string prefix r
    @ to_rev_string
        ("  resulting from expanding the type constant "
        ^ Markdown_lite.md_codify tconst)
        r_hd
    @ List.concat_map tail ~f:(fun (r, (lazy s)) ->
          to_rev_string
            ("  then expanding the type constant " ^ Markdown_lite.md_codify s)
            r)
  | Expr_dep_type (r, p, e) ->
    to_rev_string prefix r @ [(p, "  " ^ expr_dep_type_reason_string e)]
  | Contravariant_generic (r_orig, class_name) ->
    to_rev_string prefix r_orig
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " only allows supertypes (it is contravariant)" );
      ]
  | Invariant_generic (r_orig, class_name) ->
    to_rev_string prefix r_orig
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " must match exactly (it is invariant)" );
      ]
  (* If type originated with an unannotated lambda parameter with type variable type,
   * suggested annotating the lambda parameter. Otherwise defer to original reason. *)
  | Lambda_param
      ( _,
        ( From_witness_locl (Type_variable_generics _ | Type_variable _)
        | From_witness_decl (Solve_fail _)
        | Instantiate _ ) ) ->
    [
      ( p,
        prefix
        ^ " because the type of the lambda parameter could not be determined. "
        ^ "Please add a type hint to the parameter" );
    ]
  | Lambda_param (_, r_orig) -> to_rev_string prefix r_orig
  | Dynamic_coercion r -> to_rev_string prefix r
  | Dynamic_partial_enforcement (p, cn, r_orig) ->
    to_rev_string prefix r_orig
    @ [(p, "while allowing dynamic to flow into " ^ Utils.strip_all_ns cn)]
  | Rigid_tvar_escape (p, what, tvar, r_orig) ->
    let tvar = Markdown_lite.md_codify tvar in
    ( Pos_or_decl.of_raw_pos p,
      prefix ^ " because " ^ tvar ^ " escaped from " ^ what )
    :: to_rev_string ("  where " ^ tvar ^ " originates from") r_orig
  | Opaque_type_from_module (p, module_, r_orig) ->
    ( p,
      prefix
      ^ " because this is an internal symbol from module "
      ^ module_
      ^ ", which is opaque outside of the module." )
    :: to_rev_string "The type originated from here" r_orig

(* -- Constructors ---------------------------------------------------------- *)

let none = No_reason

let from_witness_locl witness = From_witness_locl witness

let from_witness_decl witness = From_witness_decl witness

let witness p = from_witness_locl @@ Witness p

let witness_from_decl p = from_witness_decl @@ Witness_from_decl p

let idx (p, r) = Idx (p, r)

let idx_vector p = from_witness_locl @@ Idx_vector p

let idx_vector_from_decl p = from_witness_decl @@ Idx_vector_from_decl p

let foreach p = from_witness_locl @@ Foreach p

let asyncforeach p = from_witness_locl @@ Asyncforeach p

let arith p = from_witness_locl @@ Arith p

let arith_ret p = from_witness_locl @@ Arith_ret p

let arith_ret_float (p, r, a) = Arith_ret_float (p, r, a)

let arith_ret_num (p, r, a) = Arith_ret_num (p, r, a)

let arith_ret_int p = from_witness_locl @@ Arith_ret_int p

let arith_dynamic p = from_witness_locl @@ Arith_dynamic p

let bitwise_dynamic p = from_witness_locl @@ Bitwise_dynamic p

let incdec_dynamic p = from_witness_locl @@ Incdec_dynamic p

let comp p = from_witness_locl @@ Comp p

let concat_ret p = from_witness_locl @@ Concat_ret p

let logic_ret p = from_witness_locl @@ Logic_ret p

let bitwise p = from_witness_locl @@ Bitwise p

let bitwise_ret p = from_witness_locl @@ Bitwise_ret p

let no_return p = from_witness_locl @@ No_return p

let no_return_async p = from_witness_locl @@ No_return_async p

let ret_fun_kind (p, k) = from_witness_locl @@ Ret_fun_kind (p, k)

let ret_fun_kind_from_decl (p, k) =
  from_witness_decl @@ Ret_fun_kind_from_decl (p, k)

let hint p = from_witness_decl @@ Hint p

let throw p = from_witness_locl @@ Throw p

let placeholder p = from_witness_locl @@ Placeholder p

let ret_div p = from_witness_locl @@ Ret_div p

let yield_gen p = from_witness_locl @@ Yield_gen p

let yield_asyncgen p = from_witness_locl @@ Yield_asyncgen p

let yield_asyncnull p = from_witness_locl @@ Yield_asyncnull p

let yield_send p = from_witness_locl @@ Yield_send p

let lost_info (s, r, b) = Lost_info (s, r, b)

let format (p, s, r) = Format (p, s, r)

let class_class (p, s) = from_witness_decl @@ Class_class (p, s)

let unknown_class p = from_witness_locl @@ Unknown_class p

let var_param p = from_witness_locl @@ Var_param p

let var_param_from_decl p = from_witness_decl @@ Var_param_from_decl p

let unpack_param (p1, p2, n) = from_witness_locl @@ Unpack_param (p1, p2, n)

let inout_param p = from_witness_decl @@ Inout_param p

let instantiate (r1, s, r2) = Instantiate (r1, s, r2)

let typeconst (r, p, s, r2) = Typeconst (r, p, s, r2)

let type_access (r, l) = Type_access (r, l)

let expr_dep_type (r, p, d) = Expr_dep_type (r, p, d)

let nullsafe_op p = from_witness_locl @@ Nullsafe_op p

let tconst_no_cstr id = from_witness_decl @@ Tconst_no_cstr id

let predicated (p, s) = from_witness_locl @@ Predicated (p, s)

let is_refinement p = from_witness_locl @@ Is_refinement p

let as_refinement p = from_witness_locl @@ As_refinement p

let equal p = from_witness_locl @@ Equal p

let varray_or_darray_key p = from_witness_decl @@ Varray_or_darray_key p

let vec_or_dict_key p = from_witness_decl @@ Vec_or_dict_key p

let using p = from_witness_locl @@ Using p

let dynamic_prop p = from_witness_locl @@ Dynamic_prop p

let dynamic_call p = from_witness_locl @@ Dynamic_call p

let dynamic_construct p = from_witness_locl @@ Dynamic_construct p

let idx_dict p = from_witness_locl @@ Idx_dict p

let idx_set_element p = from_witness_locl @@ Idx_set_element p

let missing_optional_field (p, s) =
  from_witness_decl @@ Missing_optional_field (p, s)

let unset_field (p, s) = from_witness_locl @@ Unset_field (p, s)

let contravariant_generic (r, s) = Contravariant_generic (r, s)

let invariant_generic (r, s) = Invariant_generic (r, s)

let regex p = from_witness_locl @@ Regex p

let implicit_upper_bound (p, s) =
  from_witness_decl @@ Implicit_upper_bound (p, s)

let type_variable p = from_witness_locl @@ Type_variable p

let type_variable_generics (p, n1, n2) =
  from_witness_locl @@ Type_variable_generics (p, n1, n2)

let type_variable_error p = from_witness_locl @@ Type_variable_error p

let global_type_variable_generics (p, n1, n2) =
  from_witness_decl @@ Global_type_variable_generics (p, n1, n2)

let solve_fail p = from_witness_decl @@ Solve_fail p

let cstr_on_generics (p, id) = from_witness_decl @@ Cstr_on_generics (p, id)

let lambda_param (p, r) = Lambda_param (p, r)

let shape (p, s) = from_witness_locl @@ Shape (p, s)

let shape_literal p = from_witness_locl @@ Shape_literal p

let enforceable p = from_witness_decl @@ Enforceable p

let destructure p = from_witness_locl @@ Destructure p

let key_value_collection_key p = from_witness_locl @@ Key_value_collection_key p

let global_class_prop p = from_witness_decl @@ Global_class_prop p

let global_fun_param p = from_witness_decl @@ Global_fun_param p

let global_fun_ret p = from_witness_decl @@ Global_fun_ret p

let splice p = from_witness_locl @@ Splice p

let et_boolean p = from_witness_locl @@ Et_boolean p

let default_capability p = from_witness_decl @@ Default_capability p

let concat_operand p = from_witness_locl @@ Concat_operand p

let interp_operand p = from_witness_locl @@ Interp_operand p

let dynamic_coercion r = Dynamic_coercion r

let support_dynamic_type p = from_witness_decl @@ Support_dynamic_type p

let dynamic_partial_enforcement (p, s, r) = Dynamic_partial_enforcement (p, s, r)

let rigid_tvar_escape (p, n1, n2, r) = Rigid_tvar_escape (p, n1, n2, r)

let opaque_type_from_module (p, s, r) = Opaque_type_from_module (p, s, r)

let missing_class p = from_witness_locl @@ Missing_class p

let invalid = Invalid

let captured_like p = from_witness_locl @@ Captured_like p

let pessimised_inout p = from_witness_decl @@ Pessimised_inout p

let pessimised_return p = from_witness_decl @@ Pessimised_return p

let pessimised_prop p = from_witness_decl @@ Pessimised_prop p

let unsafe_cast p = from_witness_locl @@ Unsafe_cast p

let pattern p = from_witness_locl @@ Pattern p

let flow ~from ~into ~kind = Flow (from, kind, into)

let definition def of_ = Def (def, of_)

let reverse r = Rev r

(* -- Symmetric projections -- *)
let prj_symm t ~prj = Prj (Symm prj, t)

let prj_ctor_co
    ~sub:(r_sub, r_sub_prj) ~super:r_super ctor_kind nm idx is_invariant =
  let parent_flow = flow ~from:r_sub ~into:r_super ~kind:Flow_subtype in
  let var =
    if is_invariant then
      Inv Co
    else
      Dir Co
  in
  let prj = Prj_symm_ctor (ctor_kind, nm, idx, var) in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_sub_prj ~into ~kind:Flow_prj

let prj_ctor_contra
    ~sub:r_sub ~super:(r_super, r_super_prj) ctor_kind nm idx is_invariant =
  let parent_flow =
    flow ~from:r_super ~into:(reverse r_sub) ~kind:Flow_subtype
  in
  let var =
    if is_invariant then
      Inv Co
    else
      Dir Co
  in
  let prj = Prj_symm_ctor (ctor_kind, nm, idx, var) in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_super_prj ~into ~kind:Flow_prj

let prj_neg ~sub:(r_sub, r_sub_prj) ~super =
  let parent_flow = flow ~from:r_sub ~into:super ~kind:Flow_subtype in
  let prj = Prj_symm_neg in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_sub_prj ~into ~kind:Flow_prj

let prj_nullable ~sub:(r_sub, r_sub_prj) ~super =
  let parent_flow = flow ~from:r_sub ~into:super ~kind:Flow_subtype in
  let prj = Prj_symm_nullable in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_sub_prj ~into ~kind:Flow_prj

let prj_tuple ~sub:(r_sub, r_sub_prj) ~super idx =
  let parent_flow = flow ~from:r_sub ~into:super ~kind:Flow_subtype in
  let prj = Prj_symm_tuple idx in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_sub_prj ~into ~kind:Flow_prj

let prj_shape ~sub:(r_sub, r_sub_prj) ~super lbl ~kind_sub ~kind_super =
  let parent_flow = flow ~from:r_sub ~into:super ~kind:Flow_subtype in
  let prj = Prj_symm_shape (lbl, kind_sub, kind_super) in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_sub_prj ~into ~kind:Flow_prj

let prj_fn_param ~super:(r_super, r_super_prj) ~sub ~idx_sub ~idx_super =
  let parent_flow = flow ~from:r_super ~into:(reverse sub) ~kind:Flow_subtype in
  let prj = Prj_symm_fn_param (idx_super, idx_sub) in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_super_prj ~into ~kind:Flow_prj

let prj_fn_param_inout_co ~sub:(r_sub, r_sub_prj) ~super ~idx_sub ~idx_super =
  let parent_flow = flow ~from:r_sub ~into:super ~kind:Flow_subtype in
  let prj = Prj_symm_fn_param_inout (idx_sub, idx_super, Co) in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_sub_prj ~into ~kind:Flow_prj

let prj_fn_param_inout_contra
    ~super:(r_super, r_super_prj) ~sub ~idx_sub ~idx_super =
  let parent_flow = flow ~from:r_super ~into:(reverse sub) ~kind:Flow_subtype in
  let prj = Prj_symm_fn_param_inout (idx_super, idx_sub, Contra) in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_super_prj ~into ~kind:Flow_prj

let prj_fn_ret ~sub:(r_sub, r_sub_prj) ~super =
  let parent_flow = flow ~from:r_sub ~into:super ~kind:Flow_subtype in
  let prj = Prj_symm_fn_ret in
  let into = prj_symm parent_flow ~prj in
  flow ~from:r_sub_prj ~into ~kind:Flow_prj

(* -- Asymmetric projections -- *)
let prj_asymm_sub ~r_sub ~r_sub_prj prj =
  let into = Prj (Asymm (Sub, prj), r_sub) in
  flow ~from:r_sub_prj ~into ~kind:Flow_prj

let prj_asymm_super ~r_super ~r_super_prj prj =
  let from = Prj (Asymm (Super, prj), r_super) in
  flow ~from ~into:r_super_prj ~kind:Flow_prj

let prj_union_sub ~r_sub ~r_sub_prj =
  prj_asymm_sub ~r_sub ~r_sub_prj Prj_asymm_union

let prj_union_super ~r_super ~r_super_prj =
  prj_asymm_super ~r_super ~r_super_prj Prj_asymm_union

let prj_inter_sub ~r_sub ~r_sub_prj =
  prj_asymm_sub ~r_sub ~r_sub_prj Prj_asymm_inter

let prj_inter_super ~r_super ~r_super_prj =
  prj_asymm_super ~r_super ~r_super_prj Prj_asymm_inter

let prj_neg_sub ~r_sub ~r_sub_prj =
  prj_asymm_sub ~r_sub ~r_sub_prj Prj_asymm_neg

let prj_neg_super ~r_super ~r_super_prj =
  prj_asymm_super ~r_super ~r_super_prj Prj_asymm_neg

let prj_nullable_sub ~r_sub ~r_sub_prj =
  prj_asymm_sub ~r_sub ~r_sub_prj Prj_asymm_nullable

let prj_nullable_super ~r_super ~r_super_prj =
  prj_asymm_super ~r_super ~r_super_prj Prj_asymm_nullable

let missing_field = Missing_field

let pessimised_this p = from_witness_decl @@ Pessimised_this p

(* -- Visitor --------------------------------------------------------------- *)
module Visitor = struct
  class map =
    object (this)
      method on_reason r =
        match r with
        | Type_access (r, l) ->
          Type_access
            ( this#on_reason r,
              List.map l ~f:(fun (r, x) -> (this#on_reason r, this#on_lazy x))
            )
        | Typeconst (r1, x, s, r2) ->
          Typeconst (this#on_reason r1, x, this#on_lazy s, this#on_reason r2)
        | Arith_ret_float (x, r, z) -> Arith_ret_float (x, this#on_reason r, z)
        | Arith_ret_num (x, r, z) -> Arith_ret_num (x, this#on_reason r, z)
        | Lost_info (x, r, z) -> Lost_info (x, this#on_reason r, z)
        | Format (x, y, r) -> Format (x, y, this#on_reason r)
        | Instantiate (r1, x, r2) ->
          Instantiate (this#on_reason r1, x, this#on_reason r2)
        | Expr_dep_type (r, y, z) -> Expr_dep_type (this#on_reason r, y, z)
        | Contravariant_generic (x, y) ->
          Contravariant_generic (this#on_reason x, y)
        | Invariant_generic (r, y) -> Invariant_generic (this#on_reason r, y)
        | Lambda_param (x, r) -> Lambda_param (x, this#on_reason r)
        | Dynamic_coercion r -> Dynamic_coercion (this#on_reason r)
        | Dynamic_partial_enforcement (x, y, r) ->
          Dynamic_partial_enforcement (x, y, this#on_reason r)
        | Rigid_tvar_escape (x, y, z, r) ->
          Rigid_tvar_escape (x, y, z, this#on_reason r)
        | Opaque_type_from_module (x, y, r) ->
          Opaque_type_from_module (x, y, this#on_reason r)
        | No_reason -> No_reason
        | Invalid -> Invalid
        | Missing_field -> Missing_field
        | From_witness_locl witness -> From_witness_locl witness
        | From_witness_decl witness -> From_witness_decl witness
        | Idx (x, y) -> Idx (x, y)
        | Flow (from, kind, into) ->
          Flow (this#on_reason from, kind, this#on_reason into)
        | Rev t -> Rev (this#on_reason t)
        | Def (def, t) -> Def (def, this#on_reason t)
        | Prj (prj, t) -> Prj (prj, this#on_reason t)

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
    | Opaque_type_from_module _ -> true
    | _ -> false

  let is_none r =
    match r with
    | No_reason -> true
    | _ -> false

  let is_instantiate r =
    match r with
    | Instantiate _ -> true
    | _ -> false

  let is_hint r =
    match r with
    | From_witness_decl (Hint _) -> true
    | _ -> false

  let unpack_expr_dep_type_opt r =
    match r with
    | Expr_dep_type (r, p, e) -> Some (r, p, e)
    | _ -> None

  let unpack_unpack_param_opt r =
    match r with
    | From_witness_locl (Unpack_param (p, p2, i)) -> Some (p, p2, i)
    | _ -> None

  let unpack_cstr_on_generics_opt r =
    match r with
    | From_witness_decl (Cstr_on_generics (p, sid)) -> Some (p, sid)
    | _ -> None

  let unpack_shape_literal_opt r =
    match r with
    | From_witness_locl (Shape_literal p) -> Some p
    | _ -> None
end

(* ~~ Extended reasons rendering ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

let explain _ ~complexity:_ = []

let debug t =
  [
    ( Pos_or_decl.none,
      Format.sprintf "Reason:\n%s"
      @@ Hh_json.json_to_string ~pretty:true
      @@ to_json t );
  ]

(* ~~ Aliases ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)
type t = locl_phase t_

let pp : type ph. _ -> ph t_ -> unit = (fun fmt r -> pp_t_ fmt r)

let show r = Format.asprintf "%a" pp r

type decl_t = decl_phase t_

let rec localize : decl_phase t_ -> locl_phase t_ = function
  | No_reason -> No_reason
  | Invalid -> Invalid
  | Typeconst (r1, p, q, r2) -> Typeconst (localize r1, p, q, localize r2)
  | Instantiate (r1, s, r2) -> Instantiate (localize r1, s, localize r2)
  | Expr_dep_type (r, s, t) -> Expr_dep_type (localize r, s, t)
  | From_witness_decl witness -> From_witness_decl witness

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
