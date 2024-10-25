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
[@@deriving eq, hash]

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
  | Prj_symm_supportdyn
[@@deriving hash]

let prj_symm_to_json = function
  | Prj_symm_neg -> Hh_json.JSON_String "Prj_symm_neg"
  | Prj_symm_nullable -> Hh_json.JSON_String "Prj_symm_nullable"
  | Prj_symm_supportdyn -> Hh_json.JSON_String "Prj_symm_supportdyn"
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
  | Prj_asymm_arraykey
  | Prj_asymm_num
[@@deriving hash]

let prj_asymm_to_json = function
  | Prj_asymm_union -> Hh_json.JSON_String "Prj_asymm_union"
  | Prj_asymm_inter -> Hh_json.JSON_String "Prj_asymm_inter"
  | Prj_asymm_neg -> Hh_json.JSON_String "Prj_asymm_neg"
  | Prj_asymm_nullable -> Hh_json.JSON_String "Prj_asymm_nullable"
  | Prj_asymm_arraykey -> Hh_json.JSON_String "Prj_asymm_arraykey"
  | Prj_asymm_num -> Hh_json.JSON_String "Prj_asymm_num"

(* ~~ Flow kinds ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

type flow_kind =
  | Flow_assign
  | Flow_call
  | Flow_prop_access
  | Flow_local
  | Flow_fun_return
  | Flow_param_hint
  | Flow_return_expr
  | Flow_instantiate of string
[@@deriving hash]

let flow_kind_to_json = function
  | Flow_assign -> Hh_json.string_ "Flow_assign"
  | Flow_call -> Hh_json.string_ "Flow_call"
  | Flow_prop_access -> Hh_json.string_ "Flow_prop_access"
  | Flow_local -> Hh_json.string_ "Flow_local"
  | Flow_fun_return -> Hh_json.string_ "Flow_fun_return"
  | Flow_param_hint -> Hh_json.string_ "Flow_param_hint"
  | Flow_return_expr -> Hh_json.string_ "Flow_return_expr"
  | Flow_instantiate str ->
    Hh_json.(JSON_Object [("Flow_instantiate", string_ str)])
(* ~~ Witnesses ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

(** Witness the reason for a type during typing using the position of a hint or
    expression  *)
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
  | Type_variable of Pos.t * Tvid.t
  | Type_variable_generics of Pos.t * string * string * Tvid.t
  | Type_variable_error of Pos.t * Tvid.t
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
  | Join_point of Pos.t
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
  | Type_variable (pos, _)
  | Type_variable_generics (pos, _, _, _)
  | Type_variable_error (pos, _)
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
  | Pattern pos
  | Join_point pos ->
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
  | Type_variable (p, tvid) -> Type_variable (pos p, tvid)
  | Type_variable_generics (p, t, s, tvid) ->
    Type_variable_generics (pos p, t, s, tvid)
  | Type_variable_error (p, tvid) -> Type_variable_error (pos p, tvid)
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
  | Join_point p -> Join_point (pos p)

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
  | Join_point _ -> "Rjoin_point"

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
    | Type_variable_generics (p, s1, s2, _) ->
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
    | Type_variable (p, _)
    | Type_variable_error (p, _)
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
    | Pattern p
    | Join_point p ->
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
  | Type_variable (pos, tvid) ->
    Hh_json.(
      JSON_Object
        [
          ( "Type_variable",
            JSON_Array [pos_to_json pos; string_ (Tvid.show tvid)] );
        ])
  | Type_variable_generics (pos, str1, str2, tvid) ->
    Hh_json.(
      JSON_Object
        [
          ( "Type_variable_generics",
            JSON_Array
              [
                pos_to_json pos;
                JSON_String str1;
                JSON_String str2;
                string_ (Tvid.show tvid);
              ] );
        ])
  | Type_variable_error (pos, tvid) ->
    Hh_json.(
      JSON_Object
        [
          ( "Type_variable_error",
            JSON_Array [pos_to_json pos; string_ (Tvid.show tvid)] );
        ])
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
  | Join_point pos ->
    Hh_json.(JSON_Object [("Join_point", JSON_Array [pos_to_json pos])])

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
  | Type_variable (pos, _) ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix ^ " because a type could not be determined here" )
  | Type_variable_generics (pos, tp_name, s, _) ->
    ( Pos_or_decl.of_raw_pos pos,
      prefix
      ^ " because type parameter "
      ^ Markdown_lite.md_codify tp_name
      ^ " of "
      ^ Markdown_lite.md_codify s
      ^ " could not be determined. Please add explicit type parameters to the invocation of "
      ^ Markdown_lite.md_codify s )
  | Type_variable_error (pos, _) ->
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
  | Join_point pos ->
    (Pos_or_decl.of_raw_pos pos, prefix ^ " because of this statement")

(** Witness the reason for a type during decling using the position of a hint *)
type witness_decl =
  | Witness_from_decl of (Pos_or_decl.t[@hash.ignore])
  | Idx_vector_from_decl of (Pos_or_decl.t[@hash.ignore])
  | Hint of (Pos_or_decl.t[@hash.ignore])
  | Class_class of (Pos_or_decl.t[@hash.ignore]) * string
  | Var_param_from_decl of (Pos_or_decl.t[@hash.ignore])
  | Tuple_from_splat of (Pos_or_decl.t[@hash.ignore])
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
  | Illegal_recursive_type of (Pos_or_decl.t[@hash.ignore]) * string
[@@deriving hash]

let witness_decl_to_raw_pos = function
  | Witness_from_decl pos_or_decl
  | Idx_vector_from_decl pos_or_decl
  | Ret_fun_kind_from_decl (pos_or_decl, _)
  | Hint pos_or_decl
  | Class_class (pos_or_decl, _)
  | Var_param_from_decl pos_or_decl
  | Tuple_from_splat pos_or_decl
  | Inout_param pos_or_decl
  | Tconst_no_cstr (pos_or_decl, _)
  | Varray_or_darray_key pos_or_decl
  | Vec_or_dict_key pos_or_decl
  | Missing_optional_field (pos_or_decl, _)
  | Implicit_upper_bound (pos_or_decl, _)
  | Pessimised_this pos_or_decl
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
  | Illegal_recursive_type (pos_or_decl, _) ->
    pos_or_decl

let map_pos_witness_decl pos_or_decl witness =
  match witness with
  | Witness_from_decl p -> Witness_from_decl (pos_or_decl p)
  | Idx_vector_from_decl p -> Idx_vector_from_decl (pos_or_decl p)
  | Ret_fun_kind_from_decl (p, k) -> Ret_fun_kind_from_decl (pos_or_decl p, k)
  | Hint p -> Hint (pos_or_decl p)
  | Class_class (p, s) -> Class_class (pos_or_decl p, s)
  | Var_param_from_decl p -> Var_param_from_decl (pos_or_decl p)
  | Tuple_from_splat p -> Tuple_from_splat (pos_or_decl p)
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
  | Illegal_recursive_type (p, name) ->
    Illegal_recursive_type (pos_or_decl p, name)

let constructor_string_of_witness_decl = function
  | Witness_from_decl _ -> "Rwitness_from_decl"
  | Idx_vector_from_decl _ -> "Ridx_vector_from_decl"
  | Ret_fun_kind_from_decl _ -> "Rret_fun_kind_from_decl"
  | Hint _ -> "Rhint"
  | Class_class _ -> "Rclass_class"
  | Var_param_from_decl _ -> "Rvar_param_from_decl"
  | Tuple_from_splat _ -> "Rtuple_from_splat"
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
  | Illegal_recursive_type _ -> "Rillegal_recursive_type"

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
    | Illegal_recursive_type (p, s)
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
    | Tuple_from_splat p
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
  | Tuple_from_splat pos_or_decl ->
    Hh_json.(
      JSON_Object
        [("Tuple_from_splat", JSON_Array [Pos_or_decl.json pos_or_decl])])
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
  | Illegal_recursive_type (pos_or_decl, name) ->
    Hh_json.(
      JSON_Object
        [
          ( "Illegal_recursive_type",
            JSON_Array [Pos_or_decl.json pos_or_decl; JSON_String name] );
        ])

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
  | Tuple_from_splat pos_or_decl ->
    (pos_or_decl, prefix ^ " (tuple from parameters)")
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
  | Illegal_recursive_type (pos_or_decl, name) ->
    let name = strip_ns name in
    ( pos_or_decl,
      prefix
      ^ Printf.sprintf
          ". Using `mixed` instead of %s because this is an illegal recursive use of %s in the definition of %s"
          name
          name
          name )

(* ~~ Axiom ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

(** Axioms are information about types provided by the user in class or type
    parameter declarations. We make use of this information during subtype
    constraint simplification  *)
type axiom =
  | Extends
  | Upper_bound
  | Lower_bound
[@@deriving hash]

let axiom_to_json = function
  | Extends -> Hh_json.string_ "Extends"
  | Upper_bound -> Hh_json.string_ "Upper_bound"
  | Lower_bound -> Hh_json.string_ "Lower_bound"

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
      (** Lift a decl-time witness into a reason   *)
  | Instantiate : 'phase t_ * string * 'phase t_ -> 'phase t_
  (* -- Used when generating substitutions in silent mode only -- *)
  | No_reason : 'phase t_
  (* -- Core flow constructors -- *)
  | From_witness_locl : witness_locl -> locl_phase t_
      (** Lift a typing-time witness into a reason   *)
  | Lower_bound : {
      bound: locl_phase t_;
      of_: locl_phase t_;
    }
      -> locl_phase t_
      (** Records that a type with reason [bound] acted as a lower bound
          for the type with reason [of_] *)
  | Flow : {
      from: locl_phase t_;
      kind: flow_kind;
      into: locl_phase t_;
    }
      -> locl_phase t_
      (** Records the flow of a type from an expression or hint into an
          expression during typing  *)
  | Prj_both : {
      sub_prj: locl_phase t_;
      prj: prj_symm;
      sub: locl_phase t_;
      super: locl_phase t_;
    }
      -> locl_phase t_
      (** Represents the projection of the sub- and supertype during subtype
          constraints simplifiction. [sub_prj] is the subtype resulting from the
          projection whilst [sub] and [super] and the reasons for the parent
          types *)
  | Prj_one : {
      part: locl_phase t_;
      whole: locl_phase t_;
      prj: prj_asymm;
    }
      -> locl_phase t_
      (** Represents the projection of the sub- or supertype during subtype
          constraints simplifiction. [part] is the sub/supertype resulting from
          the projection whilst [whole] is the reason for the parent type. *)
  | Axiom : {
      next: locl_phase t_;
      prev: locl_phase t_;
      axiom: axiom;
    }
      -> locl_phase t_
      (** Represents the use of a user-defined axiom about either the
          subtype or supertype during subtype constraints simplifiction.
          [next] is the sub/supertype resulting from the application of the
          axiom whilst [prev] is reason for original type. *)
  | Def : (Pos_or_decl.t[@hash.ignore]) * locl_phase t_ -> locl_phase t_
      (** Records the definition site of type alongside the reason recording its
          use. *)
  | Solved : {
      solution: locl_phase t_;
      of_: Tvid.t;
      in_: locl_phase t_;
    }
      -> locl_phase t_
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
  | Flow { from = t; _ }
  | Lower_bound { bound = t; _ }
  | Axiom { next = t; _ }
  | Prj_both { sub_prj = t; _ }
  | Prj_one { part = t; _ }
  | Solved { solution = t; _ }
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
  | Lower_bound { bound; of_ } ->
    Lower_bound
      {
        bound = map_pos pos pos_or_decl bound;
        of_ = map_pos pos pos_or_decl of_;
      }
  | Axiom { prev; axiom; next } ->
    Axiom
      {
        prev = map_pos pos pos_or_decl prev;
        next = map_pos pos pos_or_decl next;
        axiom;
      }
  | Flow { from; kind; into } ->
    Flow
      {
        from = map_pos pos pos_or_decl from;
        kind;
        into = map_pos pos pos_or_decl into;
      }
  | Prj_both { sub_prj; prj; sub; super } ->
    Prj_both
      {
        sub_prj = map_pos pos pos_or_decl sub_prj;
        prj;
        sub = map_pos pos pos_or_decl sub;
        super = map_pos pos pos_or_decl super;
      }
  | Prj_one { whole; prj; part } ->
    Prj_one
      {
        whole = map_pos pos pos_or_decl whole;
        prj;
        part = map_pos pos pos_or_decl part;
      }
  | Def (def, of_) -> Def (pos_or_decl def, map_pos pos pos_or_decl of_)
  | Solved { solution; of_; in_ } ->
    Solved
      {
        solution = map_pos pos pos_or_decl solution;
        of_;
        in_ = map_pos pos pos_or_decl in_;
      }

let to_constructor_string : type ph. ph t_ -> string = function
  | No_reason -> "Rnone"
  | Invalid -> "Rinvalid"
  | Missing_field -> "Rmissing_field"
  | From_witness_locl witness -> constructor_string_of_witness_locl witness
  | From_witness_decl witness -> constructor_string_of_witness_decl witness
  | Axiom _ -> "Raxiom"
  | Lower_bound _ -> "Rlower_bound"
  | Flow _ -> "Rflow"
  | Prj_both _ -> "Rprj_both"
  | Prj_one _ -> "Rprj_one"
  | Def _ -> "Rdef"
  | Solved _ -> "Rsolved"
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
    | Flow { from; _ } -> pp_t_ fmt from
    | Axiom { next; _ } -> pp_t_ fmt next
    | Lower_bound { bound; _ } -> pp_t_ fmt bound
    | Prj_both { sub_prj; _ } -> pp_t_ fmt sub_prj
    | Prj_one { part; _ } -> pp_t_ fmt part
    | Def (_, t) -> pp_t_ fmt t
    | Solved { solution; _ } -> pp_t_ fmt solution);
    Format.fprintf fmt "@])"

and show_t_ : type ph. ph t_ -> string = (fun r -> Format.asprintf "%a" pp_t_ r)

let pos_or_decl_to_json pos_or_decl =
  if Pos_or_decl.is_hhi pos_or_decl then
    let pos = Pos_or_decl.unsafe_to_raw_pos pos_or_decl in

    let (line_start, char_start, line_end, char_end) = Pos.destruct_range pos in
    let fn =
      let raw = Pos.filename @@ Pos.to_relative_string pos in
      match String.split raw ~on:'/' with
      | _ :: "tmp" :: _ :: rest -> String.concat ~sep:"/" rest
      | _ -> raw
    in
    if line_end = line_start then
      Hh_json.JSON_Object
        [
          ("filename", Hh_json.JSON_String fn);
          ("line", Hh_json.int_ line_start);
          ("char_start", Hh_json.int_ char_start);
          ("char_end", Hh_json.int_ (char_end - 1));
        ]
    else
      Hh_json.JSON_Object
        [
          ("filename", Hh_json.JSON_String fn);
          ("line_start", Hh_json.int_ line_start);
          ("char_start", Hh_json.int_ char_start);
          ("line_end", Hh_json.int_ line_end);
          ("char_end", Hh_json.int_ (char_end - 1));
        ]
  else
    Pos_or_decl.json pos_or_decl

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
  | Flow { from; kind; into } ->
    Hh_json.(
      JSON_Object
        [
          ( "Flow",
            JSON_Object
              [
                ("from", to_json from);
                ("kind", flow_kind_to_json kind);
                ("into", to_json into);
              ] );
        ])
    :: acc
  | Lower_bound { bound; of_ } ->
    Hh_json.(
      JSON_Object
        [
          ( "Lower_bound",
            JSON_Object [("bound", to_json bound); ("of", to_json of_)] );
        ])
    :: acc
  | Solved { solution; of_; in_ } ->
    Hh_json.(
      JSON_Object
        [
          ( "Solved",
            JSON_Object
              [
                ("solution", to_json solution);
                ("of_", string_ (Tvid.show of_));
                ("in_", to_json in_);
              ] );
        ])
    :: acc
  | Axiom { prev; axiom; next } ->
    Hh_json.(
      JSON_Object
        [
          ( "Axiom",
            JSON_Object
              [
                ("prev", to_json prev);
                ("axiom", axiom_to_json axiom);
                ("next", to_json next);
              ] );
        ])
    :: acc
  | Def (def, r) ->
    Hh_json.(
      JSON_Object [("Def", JSON_Array [pos_or_decl_to_json def; to_json r])])
    :: acc
  | Prj_both { sub_prj; prj; sub; super } ->
    Hh_json.(
      JSON_Object
        [
          ( "Prj_both",
            JSON_Object
              [
                ("sub_prj", to_json sub_prj);
                ("prj", prj_symm_to_json prj);
                ("sub", to_json sub);
                ("super", to_json super);
              ] );
        ])
    :: acc
  | Prj_one { part; prj; whole } ->
    Hh_json.(
      JSON_Object
        [
          ( "Prj_one",
            JSON_Object
              [
                ("part", to_json part);
                ("prj", prj_asymm_to_json prj);
                ("whole", to_json whole);
              ] );
        ])
    :: acc

and to_json : type a. a t_ -> Hh_json.json =
 (fun t -> Hh_json.JSON_Array (List.rev @@ to_json_help t []))

let to_pos : type ph. ph t_ -> Pos_or_decl.t =
 fun r ->
  if !Errors.report_pos_from_reason then
    Pos_or_decl.set_from_reason (to_raw_pos r)
  else
    to_raw_pos r

let rec flow_contains_tyvar = function
  | Flow
      {
        from = From_witness_locl (Type_variable_generics _ | Type_variable _);
        _;
      }
  | From_witness_locl (Type_variable_generics _ | Type_variable _) ->
    true
  | Flow { into; _ } -> flow_contains_tyvar into
  | _ -> false

let reverse_flow t =
  let rec aux ~k = function
    | Flow { from; into; kind } ->
      aux into ~k:(fun into -> k @@ Flow { from = into; into = from; kind })
    | t -> k t
  in
  aux ~k:(fun r -> r) t

(* Translate a reason to a (pos, string) list, suitable for error_l. This
 * previously returned a string, however the need to return multiple lines with
 * multiple locations meant that it needed to more than convert to a string *)
let rec to_string_help :
    type a.
    string -> locl_phase t_ Tvid.Map.t -> a t_ -> (Pos_or_decl.t * string) list
    =
 fun prefix solutions r ->
  let p = to_pos r in
  match r with
  | No_reason -> [(p, prefix)]
  | Missing_field -> [(p, prefix)]
  | Invalid -> [(p, prefix)]
  | From_witness_locl
      (Type_variable_generics (_, _, _, tvid) | Type_variable (_, tvid))
    when Tvid.Map.mem tvid solutions ->
    let r = Tvid.Map.find tvid solutions in
    let solutions = Tvid.Map.remove tvid solutions in
    to_string_help prefix solutions r
  | From_witness_locl witness -> [witness_locl_to_string prefix witness]
  | From_witness_decl witness -> [witness_decl_to_string prefix witness]
  | Axiom { next = r; _ }
  | Def (_, r)
  | Prj_one { part = r; _ } ->
    to_string_help prefix solutions r
  (* If we don't have a solution for a type variable use the origin of the flow *)
  | Flow { from; _ }
    when Tvid.Map.is_empty solutions || not (flow_contains_tyvar r) ->
    to_string_help prefix solutions from
  (* otherwise, follow the flow until we reach the type variable *)
  | Flow { from; into; _ } ->
    (match from with
    | From_witness_locl
        (Type_variable_generics (_, _, _, tvid) | Type_variable (_, tvid))
      when Tvid.Map.mem tvid solutions ->
      let r = Tvid.Map.find tvid solutions in
      let solutions = Tvid.Map.remove tvid solutions in
      to_string_help prefix solutions r
    | _ -> to_string_help prefix solutions into)
  | Solved { solution; of_; in_ = r } ->
    let solutions = Tvid.Map.add of_ solution solutions in
    to_string_help prefix solutions r
  | Lower_bound
      {
        bound = r;
        of_ = Prj_both { prj = Prj_symm_ctor (_, class_name, _, Dir Contra); _ };
      } ->
    to_string_help prefix solutions r
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " only allows supertypes (it is contravariant)" );
      ]
  | Lower_bound
      {
        bound = r;
        of_ = Prj_both { prj = Prj_symm_ctor (_, class_name, _, Inv _); _ };
      } ->
    to_string_help prefix solutions r
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " must match exactly (it is invariant)" );
      ]
  | Lower_bound { bound = r; _ } -> to_string_help prefix solutions r
  | Prj_both { sub_prj; prj = Prj_symm_ctor (_, class_name, _, Dir Contra); _ }
    ->
    to_string_help prefix solutions sub_prj
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " only allows supertypes (it is contravariant)" );
      ]
  | Prj_both { sub_prj; prj = Prj_symm_ctor (_, class_name, _, Inv _); _ } ->
    to_string_help prefix solutions sub_prj
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " must match exactly (it is invariant)" );
      ]
  | Prj_both { sub_prj; _ } -> to_string_help prefix solutions sub_prj
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
      | Flow { from = r; _ }
      | Arith_ret_float (_, r, _) ->
        find_last r
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
    @ to_string_help
        "Here is why I think the argument is a `float`: this is a `float`"
        solutions
        r_last
  | Arith_ret_num (_, r, s) ->
    let rec find_last reason =
      match reason with
      | Flow { from = r; _ }
      | Arith_ret_num (_, r, _) ->
        find_last r
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
    @ to_string_help
        "Here is why I think the argument is a `num`: this is a `num`"
        solutions
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
    to_string_help prefix solutions r1
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
    (match to_string_help "" solutions t with
    | [(_, "")] -> [(p, s)]
    | el -> [(p, s)] @ el)
  | Instantiate (r_orig, generic_name, r_inst) ->
    to_string_help prefix solutions r_orig
    @ to_string_help
        ("  via this generic " ^ Markdown_lite.md_codify generic_name)
        solutions
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
    @ to_string_help ("on " ^ ty_str) solutions r_root
  | Typeconst (r_orig, (pos, tconst), (lazy ty_str), r_root) ->
    to_string_help prefix solutions r_orig
    @ [
        (pos, sprintf "  resulting from accessing the type constant '%s'" tconst);
      ]
    @ to_string_help ("  on " ^ ty_str) solutions r_root
  | Type_access (Typeconst (No_reason, _, _, _), (r, _) :: l) ->
    to_string_help prefix solutions (Type_access (r, l))
  | Type_access (Typeconst (r, _, _, _), x) ->
    to_string_help prefix solutions (Type_access (r, x))
  | Type_access (Type_access (r, expand2), expand1) ->
    to_string_help prefix solutions (Type_access (r, expand1 @ expand2))
  | Type_access (r, []) -> to_string_help prefix solutions r
  | Type_access (r, (r_hd, (lazy tconst)) :: tail) ->
    to_string_help prefix solutions r
    @ to_string_help
        ("  resulting from expanding the type constant "
        ^ Markdown_lite.md_codify tconst)
        solutions
        r_hd
    @ List.concat_map tail ~f:(fun (r, (lazy s)) ->
          to_string_help
            ("  then expanding the type constant " ^ Markdown_lite.md_codify s)
            solutions
            r)
  | Expr_dep_type (r, p, e) ->
    to_string_help prefix solutions r
    @ [(p, "  " ^ expr_dep_type_reason_string e)]
  | Contravariant_generic (r_orig, class_name) ->
    to_string_help prefix solutions r_orig
    @ [
        ( p,
          "This type argument to "
          ^ (strip_ns class_name |> Markdown_lite.md_codify)
          ^ " only allows supertypes (it is contravariant)" );
      ]
  | Invariant_generic (r_orig, class_name) ->
    to_string_help prefix solutions r_orig
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
        From_witness_locl
          (Type_variable_generics (_, _, _, tvid) | Type_variable (_, tvid)) )
    when Tvid.Map.mem tvid solutions ->
    let r = Tvid.Map.find tvid solutions in
    let solutions = Tvid.Map.remove tvid solutions in
    to_string_help prefix solutions r
  | Lambda_param
      ( _,
        ( From_witness_decl (Solve_fail _)
        | From_witness_locl (Type_variable_generics _ | Type_variable _) ) ) ->
    [
      ( p,
        prefix
        ^ " because the type of the lambda parameter could not be determined. "
        ^ "Please add a type hint to the parameter" );
    ]
  | Lambda_param (_, r_orig) -> to_string_help prefix solutions r_orig
  | Dynamic_coercion r -> to_string_help prefix solutions r
  | Dynamic_partial_enforcement (p, cn, r_orig) ->
    to_string_help prefix solutions r_orig
    @ [(p, "while allowing dynamic to flow into " ^ Utils.strip_all_ns cn)]
  | Rigid_tvar_escape (p, what, tvar, r_orig) ->
    let tvar = Markdown_lite.md_codify tvar in
    ( Pos_or_decl.of_raw_pos p,
      prefix ^ " because " ^ tvar ^ " escaped from " ^ what )
    :: to_string_help ("  where " ^ tvar ^ " originates from") solutions r_orig
  | Opaque_type_from_module (p, module_, r_orig) ->
    ( p,
      prefix
      ^ " because this is an internal symbol from module "
      ^ module_
      ^ ", which is opaque outside of the module." )
    :: to_string_help "The type originated from here" solutions r_orig

let to_string : type a. string -> a t_ -> (Pos_or_decl.t * string) list =
 (fun prefix r -> to_string_help prefix Tvid.Map.empty r)
(* -- Constructors ---------------------------------------------------------- *)

module Constructors = struct
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

  let tuple_from_splat p = from_witness_decl @@ Tuple_from_splat p

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

  let type_variable p tvid = from_witness_locl @@ Type_variable (p, tvid)

  let type_variable_generics (p, n1, n2) tvid =
    from_witness_locl @@ Type_variable_generics (p, n1, n2, tvid)

  let type_variable_error p tvid =
    from_witness_locl @@ Type_variable_error (p, tvid)

  let global_type_variable_generics (p, n1, n2) =
    from_witness_decl @@ Global_type_variable_generics (p, n1, n2)

  let solve_fail p = from_witness_decl @@ Solve_fail p

  let cstr_on_generics (p, id) = from_witness_decl @@ Cstr_on_generics (p, id)

  let lambda_param (p, r) = Lambda_param (p, r)

  let shape (p, s) = from_witness_locl @@ Shape (p, s)

  let shape_literal p = from_witness_locl @@ Shape_literal p

  let enforceable p = from_witness_decl @@ Enforceable p

  let destructure p = from_witness_locl @@ Destructure p

  let key_value_collection_key p =
    from_witness_locl @@ Key_value_collection_key p

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

  let dynamic_partial_enforcement (p, s, r) =
    Dynamic_partial_enforcement (p, s, r)

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

  let join_point p = from_witness_locl @@ Join_point p

  let rec flow ~from ~into ~kind =
    match (from, into) with
    | (Flow { from; into = into_l; kind = kind_l }, _) ->
      Flow { from; into = flow ~from:into_l ~into ~kind; kind = kind_l }
    | _ -> Flow { from; kind; into }

  let flow_assign ~rhs ~lval = flow ~from:rhs ~into:lval ~kind:Flow_assign

  let flow_local ~def ~use = flow ~from:def ~into:use ~kind:Flow_local

  let flow_call ~def ~use = flow ~from:def ~into:use ~kind:Flow_call

  let flow_prop_access ~def ~use =
    flow ~from:def ~into:use ~kind:Flow_prop_access

  let flow_return_expr ~expr ~ret =
    flow ~from:expr ~into:ret ~kind:Flow_return_expr

  let flow_return_hint ~hint ~use =
    flow ~from:hint ~into:use ~kind:Flow_fun_return

  let flow_param_hint ~hint ~param =
    flow ~from:hint ~into:param ~kind:Flow_param_hint

  let solved of_ ~solution ~in_ = Solved { solution; of_; in_ }

  let axiom_extends ~child ~ancestor =
    Axiom { axiom = Extends; prev = child; next = ancestor }

  let axiom_upper_bound ~bound ~of_ =
    Axiom { axiom = Upper_bound; prev = of_; next = bound }

  let axiom_lower_bound ~bound ~of_ =
    Axiom { axiom = Lower_bound; prev = of_; next = bound }

  let trans_lower_bound ~bound ~of_ = Lower_bound { bound; of_ }

  let definition def of_ = Def (def, of_)

  (* -- Symmetric projections -- *)
  let prj_symm_co ~sub ~sub_prj ~super prj =
    Prj_both { sub_prj; prj; sub; super }

  let prj_symm_contra ~sub ~super ~super_prj prj =
    Prj_both { sub_prj = super_prj; prj; sub; super }

  let prj_ctor_co ~sub ~sub_prj ~super ctor_kind nm idx is_invariant =
    let var =
      if is_invariant then
        Inv Co
      else
        Dir Co
    in
    let prj = Prj_symm_ctor (ctor_kind, nm, idx, var) in
    prj_symm_co ~sub ~sub_prj ~super prj

  let prj_ctor_contra ~sub ~super ~super_prj ctor_kind nm idx is_invariant =
    let var =
      if is_invariant then
        Inv Contra
      else
        Dir Contra
    in
    let prj = Prj_symm_ctor (ctor_kind, nm, idx, var) in
    prj_symm_contra ~sub ~super ~super_prj prj

  let prj_neg ~sub ~sub_prj ~super =
    prj_symm_co ~sub ~sub_prj ~super Prj_symm_neg

  let prj_supportdyn ~sub ~sub_prj ~super =
    prj_symm_co ~sub ~sub_prj ~super Prj_symm_supportdyn

  let prj_nullable ~sub ~sub_prj ~super =
    prj_symm_co ~sub ~sub_prj ~super Prj_symm_nullable

  let prj_tuple ~sub ~sub_prj ~super idx =
    let prj = Prj_symm_tuple idx in
    prj_symm_co ~sub ~sub_prj ~super prj

  let prj_shape ~sub ~sub_prj ~super lbl ~kind_sub ~kind_super =
    let prj = Prj_symm_shape (lbl, kind_sub, kind_super) in
    prj_symm_co ~sub ~sub_prj ~super prj

  let prj_fn_param ~sub ~super ~super_prj ~idx_sub ~idx_super =
    let prj = Prj_symm_fn_param (idx_super, idx_sub) in
    prj_symm_contra ~sub ~super ~super_prj prj

  let prj_fn_param_inout_co ~sub ~sub_prj ~super ~idx_sub ~idx_super =
    let prj = Prj_symm_fn_param_inout (idx_sub, idx_super, Co) in
    prj_symm_co ~sub ~sub_prj ~super prj

  let prj_fn_param_inout_contra ~sub ~super ~super_prj ~idx_sub ~idx_super =
    let prj = Prj_symm_fn_param_inout (idx_super, idx_sub, Contra) in
    prj_symm_contra ~sub ~super ~super_prj prj

  let prj_fn_ret ~sub ~sub_prj ~super =
    let prj = Prj_symm_fn_ret in
    prj_symm_co ~sub ~sub_prj ~super prj

  (* -- Asymmetric projections -- *)

  let prj_asymm_sub ~sub ~sub_prj prj =
    Prj_one { part = sub_prj; prj; whole = sub }

  let prj_asymm_super ~super ~super_prj prj =
    Prj_one { part = super_prj; prj; whole = super }

  let prj_union_sub ~sub ~sub_prj = prj_asymm_sub ~sub ~sub_prj Prj_asymm_union

  let prj_union_super ~super ~super_prj =
    prj_asymm_super ~super ~super_prj Prj_asymm_union

  let prj_inter_sub ~sub ~sub_prj = prj_asymm_sub ~sub ~sub_prj Prj_asymm_inter

  let prj_inter_super ~super ~super_prj =
    prj_asymm_super ~super ~super_prj Prj_asymm_inter

  let prj_neg_sub ~sub ~sub_prj = prj_asymm_sub ~sub ~sub_prj Prj_asymm_neg

  let prj_neg_super ~super ~super_prj =
    prj_asymm_super ~super ~super_prj Prj_asymm_neg

  let prj_nullable_sub ~sub ~sub_prj =
    prj_asymm_sub ~sub ~sub_prj Prj_asymm_nullable

  let prj_nullable_super ~super ~super_prj =
    prj_asymm_super ~super ~super_prj Prj_asymm_nullable

  let prj_num_sub ~sub ~sub_prj = prj_asymm_sub ~sub ~sub_prj Prj_asymm_num

  let prj_num_super ~super ~super_prj =
    prj_asymm_super ~super ~super_prj Prj_asymm_num

  let prj_arraykey_sub ~sub ~sub_prj =
    prj_asymm_sub ~sub ~sub_prj Prj_asymm_arraykey

  let prj_arraykey_super ~super ~super_prj =
    prj_asymm_super ~super ~super_prj Prj_asymm_arraykey

  let missing_field = Missing_field

  let pessimised_this p = from_witness_decl @@ Pessimised_this p

  let illegal_recursive_type p name =
    from_witness_decl @@ Illegal_recursive_type (p, name)
end

include Constructors

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
        | Flow { from; kind; into } ->
          Flow { from = this#on_reason from; kind; into = this#on_reason into }
        | Axiom { prev; axiom; next } ->
          Axiom
            { prev = this#on_reason prev; axiom; next = this#on_reason next }
        | Lower_bound { bound; of_ } ->
          Lower_bound { bound = this#on_reason bound; of_ = this#on_reason of_ }
        | Solved { solution; of_; in_ } ->
          Solved
            {
              solution = this#on_reason solution;
              of_;
              in_ = this#on_reason in_;
            }
        | Def (def, t) -> Def (def, this#on_reason t)
        | Prj_both { sub_prj; prj; sub; super } ->
          Prj_both
            {
              sub_prj = this#on_reason sub_prj;
              prj;
              sub = this#on_reason sub;
              super = this#on_reason super;
            }
        | Prj_one { part; prj; whole } ->
          Prj_one
            { part = this#on_reason part; prj; whole = this#on_reason whole }

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
  let on_outermost t f =
    let rec aux t ~solutions =
      match t with
      | Solved { solution; of_; in_ = t } ->
        let solutions = Tvid.Map.add of_ solution solutions in
        aux t ~solutions
      | Flow { from = t; _ }
      | Lower_bound { bound = t; _ }
      | Axiom { next = t; _ }
      | Prj_both { sub_prj = t; _ }
      | Prj_one { part = t; _ }
      | Def (_, t) ->
        aux t ~solutions
      | From_witness_locl
          (Type_variable_generics (_, _, _, tvid) | Type_variable (_, tvid))
        when Tvid.Map.mem tvid solutions ->
        let t = Tvid.Map.find tvid solutions in
        let solutions = Tvid.Map.remove tvid solutions in
        aux t ~solutions
      | _ -> f t
    in
    aux t ~solutions:Tvid.Map.empty

  let is_opaque_type_from_module r =
    let p r =
      match r with
      | Opaque_type_from_module _ -> true
      | _ -> false
    in
    on_outermost r p

  let is_none r =
    let p r =
      match r with
      | No_reason -> true
      | _ -> false
    in
    on_outermost r p

  let is_instantiate r =
    let p r =
      match r with
      | Instantiate _ -> true
      | _ -> false
    in
    on_outermost r p

  let is_captured_like r =
    let p r =
      match r with
      | From_witness_locl (Captured_like _) -> true
      | _ -> false
    in
    on_outermost r p

  let is_hint r =
    let p r =
      match r with
      | From_witness_decl (Hint _) -> true
      | _ -> false
    in
    on_outermost r p

  let unpack_expr_dep_type_opt r =
    let f r =
      match r with
      | Expr_dep_type (r, p, e) -> Some (r, p, e)
      | _ -> None
    in
    on_outermost r f

  let unpack_unpack_param_opt r =
    let f r =
      match r with
      | From_witness_locl (Unpack_param (p, p2, i)) -> Some (p, p2, i)
      | _ -> None
    in
    on_outermost r f

  let unpack_cstr_on_generics_opt r =
    let f r =
      match r with
      | From_witness_decl (Cstr_on_generics (p, sid)) -> Some (p, sid)
      | _ -> None
    in
    on_outermost r f

  let unpack_shape_literal_opt r =
    let f r =
      match r with
      | From_witness_locl (Shape_literal p) -> Some p
      | _ -> None
    in
    on_outermost r f
end

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

(* ~~ Extended reasons rendering ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

module Derivation = struct
  module Subtype_rule : sig
    (** Tells us why a derivation step follows the previous one *)
    type t

    (** We reached the next step in the derivation by projecting into the subtype *)
    val using_prj_sub : prj_asymm -> t

    (** We reached the next step in the derivation by projecting into the supertype *)
    val using_prj_super : prj_asymm -> t

    (** We reached the next step in the derivation by projecting into both the
       subtype and supertype *)
    val using_prj : prj_symm -> t

    (** We reached the next step in the derivation by making use of some
        user-declared axiom on about the subtype *)
    val using_axiom_sub : axiom -> t

    (** We reached the next step in the derivation by making use of some
        user-declared axiom on about the supertype *)
    val using_axiom_super : axiom -> t

    val to_json : t -> Hh_json.json

    val explain : t -> string

    val is_supportdyn : t -> bool
  end = struct
    type t =
      | Using_prj of prj_symm
      | Using_prj_sub of prj_asymm
      | Using_prj_super of prj_asymm
      | Using_axiom_sub of axiom
      | Using_axiom_super of axiom

    let is_supportdyn = function
      | Using_prj Prj_symm_supportdyn -> true
      | _ -> false

    let using_prj prj = Using_prj prj

    let using_prj_sub prj = Using_prj_sub prj

    let using_prj_super prj = Using_prj_super prj

    let using_axiom_sub axiom = Using_axiom_sub axiom

    let using_axiom_super axiom = Using_axiom_super axiom

    let to_json = function
      | Using_prj prj ->
        Hh_json.(JSON_Object [("Using_prj", prj_symm_to_json prj)])
      | Using_prj_sub prj ->
        Hh_json.(JSON_Object [("Using_prj_sub", prj_asymm_to_json prj)])
      | Using_prj_super prj ->
        Hh_json.(JSON_Object [("Using_prj_super", prj_asymm_to_json prj)])
      | Using_axiom_sub axiom ->
        Hh_json.(JSON_Object [("Using axiom_sub", axiom_to_json axiom)])
      | Using_axiom_super axiom ->
        Hh_json.(JSON_Object [("Using axiom_super", axiom_to_json axiom)])

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

    let explain_axiom axiom ~sub =
      let (subject, other) =
        if sub then
          ("subtype", "supertype")
        else
          ("supertype", "subtype")
      in
      match axiom with
      | Extends ->
        Format.sprintf
          "The %s extends or implements the %s class or interface so next I checked that subtype constraint."
          subject
          other
      | Upper_bound ->
        Format.sprintf
          "The %s declares an upper bound so next I checked that was a %s of the %s."
          subject
          subject
          other
      | Lower_bound ->
        Format.sprintf
          "The %s declares a lower bound so next I checked that was a %s of the %s."
          subject
          subject
          other

    let explain_ctor_kind = function
      | Ctor_class -> "class or interface"
      | Ctor_newtype -> "newtype"

    let explain_field_kind = function
      | Absent -> "missing"
      | Required -> "required"
      | Optional -> "optional"

    let explain_prj = function
      | Prj_symm_neg -> "These are negated types so I checked the inner type."
      | Prj_symm_nullable ->
        "These are nullable types so next I checked the non-null parts."
      | Prj_symm_supportdyn ->
        "These are `supportdyn` types so next I checked the non-dynamic parts."
      | Prj_symm_fn_ret ->
        "These are function types so next I checked function return types."
      | Prj_symm_tuple idx ->
        Format.sprintf
          "These are tuple types so next I checked %s elements."
          (int_to_ordinal @@ (idx + 1))
      | Prj_symm_fn_param (idx_sub, idx_sup) when Int.equal idx_sub idx_sup ->
        Format.sprintf
          "These are function types so next I checked the %s function parameters.\nFunctions are contravariant in their parameters so the direction of the subtype relationship is reversed."
          (int_to_ordinal @@ (idx_sub + 1))
      | Prj_symm_fn_param (idx_sub, idx_sup) ->
        Format.sprintf
          "These are function types so next I checked the %s function parameter of the subtype and the %s parameter of the supertype.\nFunctions are contravariant in their parameters the direction of the subtype relationship is reversed."
          (int_to_ordinal @@ (idx_sub + 1))
          (int_to_ordinal @@ (idx_sup + 1))
      | Prj_symm_fn_param_inout (idx_sub, idx_sup, Co)
        when Int.equal idx_sub idx_sup ->
        Format.sprintf
          "These are function types so next I checked the %s function parameters.\nThis is an `inout` parameter which is invariant. This means the subtype relation must hold in both directions.\nHere I checked the covariant direction."
          (int_to_ordinal @@ (idx_sub + 1))
      | Prj_symm_fn_param_inout (idx_sub, idx_sup, Co) ->
        Format.sprintf
          "These are function types so next I checked the %s function parameter of the subtype and the %s parameter of the supertype.\nThese are `inout` parameters which are invariant. This means the subtype relation must hold in both directions.\nHere I checked the covariant direction."
          (int_to_ordinal @@ (idx_sub + 1))
          (int_to_ordinal @@ (idx_sup + 1))
      | Prj_symm_fn_param_inout (idx_sub, idx_sup, Contra)
        when Int.equal idx_sub idx_sup ->
        Format.sprintf
          "These are function types so next I checked the %s function parameters.\nThis is an `inout` parameter which is invariant. This means the subtype relation must hold in both directions.\nHere I checked the contravariant case so the direction of the subtype relationship is reversed."
          (int_to_ordinal @@ (idx_sub + 1))
      | Prj_symm_fn_param_inout (idx_sub, idx_sup, Contra) ->
        Format.sprintf
          "These are function types so next I checked the %s function parameter of the supertype and the %s parameter of the subtype. These are `inout` parameters which are invariant. This means the relation must hold in both directions.\nHere I checked the contravariant case so the direction of the subtype relationship is reversed."
          (int_to_ordinal @@ (idx_sup + 1))
          (int_to_ordinal @@ (idx_sub + 1))
      | Prj_symm_ctor (ctor_kind, nm, idx, Dir Co) ->
        Format.sprintf
          "`%s` is a %s so next I checked the %s type arguments."
          (strip_ns nm)
          (explain_ctor_kind ctor_kind)
          (int_to_ordinal @@ (idx + 1))
      | Prj_symm_ctor (ctor_kind, nm, idx, Dir Contra) ->
        Format.sprintf
          "`%s` is a %s so next I checked the %s type arguments.\nThe type parameter is contravariant so the direction of the subtype relationship is reversed."
          (strip_ns nm)
          (explain_ctor_kind ctor_kind)
          (int_to_ordinal @@ (idx + 1))
      | Prj_symm_ctor (ctor_kind, nm, idx, Inv Co) ->
        Format.sprintf
          "`%s` is a %s so next I checked the %s type arguments are subtypes.\nThe type parameter is invariant so the subtype relationship must hold in both directions.\nHere I check the covariant case."
          (strip_ns nm)
          (explain_ctor_kind ctor_kind)
          (int_to_ordinal @@ (idx + 1))
      | Prj_symm_ctor (ctor_kind, nm, idx, Inv Contra) ->
        Format.sprintf
          "`%s` is a %s so next I checked the %s type arguments are subtypes.\nThe type parameter is invariant so the subtype relationship must hold in both directions.\nHere I check the contravariant case so the direction of the subtype relationship is reversed."
          (strip_ns nm)
          (explain_ctor_kind ctor_kind)
          (int_to_ordinal @@ (idx + 1))
      | Prj_symm_shape (nm, Absent, fld_kind_sup) ->
        Format.sprintf
          "These are shape types so next I tried to check the `%s` field.\nThe field is %s in the supertype but missing in the subtype."
          nm
          (explain_field_kind fld_kind_sup)
      | Prj_symm_shape (nm, fld_kind_sub, Absent) ->
        Format.sprintf
          "These are shape types so next I tried to check the `%s` field.\nThe field is %s in the subtype but missing in the supertype."
          nm
          (explain_field_kind fld_kind_sub)
      | Prj_symm_shape (nm, fld_kind_sub, fld_kind_sup)
        when equal_field_kind fld_kind_sub fld_kind_sup ->
        Format.sprintf
          "These are shape types so next I checked the %s `%s` field."
          (explain_field_kind fld_kind_sub)
          nm
      | Prj_symm_shape (nm, fld_kind_sub, fld_kind_sup) ->
        Format.sprintf
          "These are shape types so next I checked the `%s` field.\nThe field was %s in the subtype and %s in the supertype."
          nm
          (explain_field_kind fld_kind_sub)
          (explain_field_kind fld_kind_sup)

    let explain_prj_asymm_sub prj =
      match prj with
      | Prj_asymm_union ->
        "The subtype is a union type so next I checked the subtype constraint is satisfied for all its elements."
      | Prj_asymm_inter ->
        "The subtype is an intersection type so next I checked that the subtype constraint is satisfied for at least one of its element."
      | Prj_asymm_nullable ->
        "The subtype is a nullable type so next I checked the subtype constraint is satisfied for both the null & non-null parts."
      | Prj_asymm_num ->
        "The subtype is a num type so next I checked the subtype constraint is satisfied for both the int and float parts."
      | Prj_asymm_arraykey ->
        "The subtype is an arraykey type so next I checked the subtype constraint is satisfied for both the int and string parts."
      | Prj_asymm_neg ->
        "The subtype is a negated type so next I checked the inner type."

    let explain_prj_asymm_super prj =
      match prj with
      | Prj_asymm_union ->
        "The supertype is a union type so next I checked the subtype constraint is satisfied for at least one element."
      | Prj_asymm_inter ->
        "The supertype is an intersection type so next I checked the subtype constraint is satsified for all of its elements."
      | Prj_asymm_nullable ->
        "The supertype is a nullable type so next I checked the subtype constraint is satisfied for either the null or non-null part."
      | Prj_asymm_num ->
        "The supertype is a num type so next I checked the subtype constraint is satisfied for either the int or float part."
      | Prj_asymm_arraykey ->
        "The supertype is an arraykey type so next I checked the subtype constraint is satisfied for either the int or string part."
      | Prj_asymm_neg ->
        "The supertype is a negated type so I checked the inner type."

    let explain = function
      | Using_prj prj -> explain_prj prj
      | Using_prj_sub prj -> explain_prj_asymm_sub prj
      | Using_prj_super prj -> explain_prj_asymm_super prj
      | Using_axiom_sub axiom -> explain_axiom axiom ~sub:true
      | Using_axiom_super axiom -> explain_axiom axiom ~sub:false
  end

  module Subtype_step : sig
    type arg =
      | Subtype
      | Supertype
      | Both

    (** Represents a step in a subtyping derivation for a single subtype proposition*)
    type t =
      | Begin of {
          sub: locl_phase t_;
          super: locl_phase t_;
        }
      | Step of {
          rule: Subtype_rule.t;
          on_: arg;
          sub: locl_phase t_;
          super: locl_phase t_;
        }

    val begin_ : sub:locl_phase t_ -> super:locl_phase t_ -> t

    val step :
      Subtype_rule.t -> arg -> sub:locl_phase t_ -> super:locl_phase t_ -> t

    val to_json : t -> Hh_json.json

    val uses_supportdyn : t -> bool
  end = struct
    type arg =
      | Subtype
      | Supertype
      | Both

    let arg_to_json = function
      | Subtype -> Hh_json.string_ "Subtype"
      | Supertype -> Hh_json.string_ "Supertype"
      | Both -> Hh_json.string_ "Both"

    type t =
      | Begin of {
          sub: locl_phase t_;
          super: locl_phase t_;
        }
      | Step of {
          rule: Subtype_rule.t;
          on_: arg;
          sub: locl_phase t_;
          super: locl_phase t_;
        }

    let step rule on_ ~sub ~super = Step { sub; super; rule; on_ }

    let uses_supportdyn = function
      | Begin _ -> false
      | Step { rule; _ } -> Subtype_rule.is_supportdyn rule

    let begin_ ~sub ~super = Begin { sub; super }

    let to_json = function
      | Begin { sub; super } ->
        Hh_json.(
          JSON_Object
            [
              ( "Begin",
                JSON_Object [("sub", to_json sub); ("super", to_json super)] );
            ])
      | Step { sub; super; rule; on_ } ->
        Hh_json.(
          JSON_Object
            [
              ( "Step",
                JSON_Object
                  [
                    ("rule", Subtype_rule.to_json rule);
                    ("on_", arg_to_json on_);
                    ("sub", to_json sub);
                    ("super", to_json super);
                  ] );
            ])
  end

  (** Represents a complete derivation for a single subtype proposition and
        the typing rule used to reach it from the previous subtype proposition *)
  type t =
    | Derivation of Subtype_step.t list
    | Lower of {
        bound: t;
        in_: t;
      }
    | Transitive of {
        lower: t;
        upper: t;
        in_: t;
        on_: locl_phase t_;
      }

  let derivation steps = Derivation steps

  let transitive ~upper ~lower ~on_ ~in_ = Transitive { in_; on_; upper; lower }

  let lower_bound ~bound ~in_ = Lower { bound; in_ }

  let to_json t =
    let rec aux = function
      | Derivation steps ->
        Hh_json.(
          JSON_Object [("Derivation", array_ Subtype_step.to_json steps)])
      | Lower { bound; in_ } ->
        Hh_json.(
          JSON_Object
            [("Lower", JSON_Object [("bound", aux bound); ("in_", aux in_)])])
      | Transitive { lower; upper; on_; in_ } ->
        Hh_json.(
          JSON_Object
            [
              ( "Transitive",
                JSON_Object
                  [
                    ("lower", aux lower);
                    ("upper", aux upper);
                    ("on_", to_json on_);
                    ("in_", aux in_);
                  ] );
            ])
    in
    aux t

  (* ~~ Construct a derviation from a reason ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)
  let rec push_solutions t ~solutions =
    match t with
    | From_witness_locl (Type_variable (_, id))
    | From_witness_locl (Type_variable_generics (_, _, _, id))
    | From_witness_locl (Type_variable_error (_, id)) ->
      Option.value_map ~default:t ~f:(fun solution ->
          solved id ~solution ~in_:t)
      @@ Tvid.Map.find_opt id solutions
    | Flow { from; into; kind } ->
      Flow
        {
          from = push_solutions from ~solutions;
          into = push_solutions into ~solutions;
          kind;
        }
    | Def (pos, of_) -> Def (pos, push_solutions of_ ~solutions)
    | Instantiate (r1, s, r2) ->
      Instantiate (push_solutions r1 ~solutions, s, push_solutions r2 ~solutions)
    | Typeconst (r1, p, q, r2) ->
      Typeconst
        (push_solutions r1 ~solutions, p, q, push_solutions r2 ~solutions)
    | Expr_dep_type (r, s, t) ->
      Expr_dep_type (push_solutions r ~solutions, s, t)
    | Lost_info (x, t, y) -> Lost_info (x, push_solutions t ~solutions, y)
    | Type_access (t, x) -> Type_access (push_solutions t ~solutions, x)
    | Invariant_generic (t, x) ->
      Invariant_generic (push_solutions ~solutions t, x)
    | Contravariant_generic (t, x) ->
      Contravariant_generic (push_solutions ~solutions t, x)
    | Dynamic_coercion t -> Dynamic_coercion (push_solutions ~solutions t)
    | _ -> t

  let rec extract_last t =
    match t with
    | Prj_both { sub; _ } -> extract_last sub
    | Prj_one { whole; _ } -> extract_last whole
    | Axiom { prev; _ } -> extract_last prev
    | Flow { into; _ } -> extract_last into
    | Lower_bound { of_; _ } -> extract_last of_
    | Solved _
    | No_reason
    | From_witness_decl _
    | From_witness_locl _
    | Instantiate _
    | Def _
    | Invalid
    | Missing_field
    | Idx _
    | Arith_ret_float _
    | Arith_ret_num _
    | Lost_info _
    | Format _
    | Typeconst _
    | Type_access _
    | Expr_dep_type _
    | Contravariant_generic _
    | Invariant_generic _
    | Lambda_param _
    | Dynamic_coercion _
    | Dynamic_partial_enforcement _
    | Rigid_tvar_escape _
    | Opaque_type_from_module _ ->
      t

  let rec extract_first t =
    match t with
    | Prj_both { sub_prj; _ } -> extract_first sub_prj
    | Prj_one { part; _ } -> extract_first part
    | Axiom { next; _ } -> extract_first next
    | Flow { from; _ } -> extract_first from
    | Lower_bound { bound; _ } -> extract_first bound
    | Solved _
    | No_reason
    | From_witness_decl _
    | From_witness_locl _
    | Instantiate _
    | Def _
    | Invalid
    | Missing_field
    | Idx _
    | Arith_ret_float _
    | Arith_ret_num _
    | Lost_info _
    | Format _
    | Typeconst _
    | Type_access _
    | Expr_dep_type _
    | Contravariant_generic _
    | Invariant_generic _
    | Lambda_param _
    | Dynamic_coercion _
    | Dynamic_partial_enforcement _
    | Rigid_tvar_escape _
    | Opaque_type_from_module _ ->
      t

  (** Reasons are constructed by keeping track of preceeding subtype propositions
      during subtype constraint simplification. We reach a child subtype proposition
      either through a projection into a type constructor or using some user-declared
      axiom. We can convert this to a list of derivation steps for each subtype constraint.
      Since we also apply transitivity during constraint simplification, each reason may
      actually contain multiple subtype derivations. We reach a new derivation either
      because a upper- or lower-bound was added to a type variable and a new constraint
      generated for an exist lower- or upper-bound.

      This function is used to recover this representation from the reason representation. *)
  let of_reason ~sub ~super =
    let rec aux (sub, super) ~deriv ~solutions =
      match (sub, super) with
      (* -- Accumulate solutions -- *)
      | (Solved { solution; of_; in_ }, _) ->
        let solutions = Tvid.Map.add of_ (extract_first solution) solutions in
        aux (in_, super) ~deriv ~solutions
      | (_, Solved { solution; of_; in_ }) ->
        let solutions = Tvid.Map.add of_ (extract_first solution) solutions in
        aux (sub, in_) ~deriv ~solutions
      (* -- Transitive constraints -- *)
      | ( Lower_bound
            {
              bound = Lower_bound { bound = lb_sub; of_ = lb_super };
              of_ = ub_sub;
            },
          ub_super ) ->
        let lower = aux (lb_sub, lb_super) ~deriv:[] ~solutions
        and upper = aux (ub_sub, ub_super) ~deriv:[] ~solutions
        and in_ = aux (lb_sub, ub_super) ~deriv ~solutions
        and on_ = extract_last lb_super in
        transitive ~lower ~upper ~on_ ~in_
      | (Lower_bound { bound; of_ }, super) ->
        let bound = aux (bound, of_) ~deriv:[] ~solutions
        and in_ = aux (bound, super) ~deriv ~solutions in
        lower_bound ~bound ~in_
      | (sub, Lower_bound { bound; of_ }) ->
        let bound = aux (bound, of_) ~deriv:[] ~solutions
        and in_ = aux (sub, bound) ~deriv ~solutions in
        lower_bound ~bound ~in_
      (* -- One-sided projection on subtype -- *)
      | (Prj_one { part; whole; prj }, _) ->
        let step =
          let sub = push_solutions part ~solutions
          and super = push_solutions super ~solutions
          and rule = Subtype_rule.using_prj_sub prj in
          Subtype_step.(step rule Subtype ~sub ~super)
        in
        let deriv = step :: deriv in
        aux (whole, super) ~deriv ~solutions
      | (Axiom { next; prev; axiom }, _) ->
        let step =
          let sub = push_solutions next ~solutions
          and super = push_solutions super ~solutions
          and rule = Subtype_rule.using_axiom_sub axiom in
          Subtype_step.(step rule Subtype ~sub ~super)
        in
        let deriv = step :: deriv in
        aux (prev, super) ~deriv ~solutions
      (* -- One-sided projection on supertype -- *)
      | (_, Prj_one { part; whole; prj }) ->
        let step =
          let sub = push_solutions sub ~solutions
          and super = push_solutions part ~solutions
          and rule = Subtype_rule.using_prj_super prj in
          Subtype_step.(step rule Supertype ~sub ~super)
        in
        let deriv = step :: deriv in
        aux (sub, whole) ~deriv ~solutions
      | (_, Axiom { next; prev; axiom }) ->
        let step =
          let sub = push_solutions sub ~solutions
          and super = push_solutions next ~solutions
          and rule = Subtype_rule.using_axiom_super axiom in
          Subtype_step.(step rule Supertype ~sub ~super)
        in
        let deriv = step :: deriv in
        aux (sub, prev) ~deriv ~solutions
      (* -- Two-sided projections -- *)
      | (Prj_both { sub_prj; sub = parent_sub; super = parent_super; prj }, _)
        ->
        let step =
          let sub = push_solutions sub_prj ~solutions
          and super = push_solutions super ~solutions
          and rule = Subtype_rule.using_prj prj in
          Subtype_step.(step rule Both ~sub ~super)
        in
        let deriv = step :: deriv in
        aux (parent_sub, parent_super) ~deriv ~solutions
      | (_, Prj_both { sub_prj; sub = parent_sub; super = parent_super; prj })
        ->
        (* Does this happen? Should it?? *)
        let step =
          let sub = push_solutions sub ~solutions
          and super = push_solutions sub_prj ~solutions
          and rule = Subtype_rule.using_prj prj in
          Subtype_step.(step rule Both ~sub ~super)
        in
        let deriv = step :: deriv in
        aux (parent_sub, parent_super) ~deriv ~solutions
      (* -- Reasons corresponding to a single step -- *)
      | ( ( No_reason | From_witness_decl _ | From_witness_locl _
          | Instantiate _ | Flow _ | Def _ | Invalid | Missing_field | Idx _
          | Arith_ret_float _ | Arith_ret_num _ | Lost_info _ | Format _
          | Typeconst _ | Type_access _ | Expr_dep_type _
          | Contravariant_generic _ | Invariant_generic _ | Lambda_param _
          | Dynamic_coercion _ | Dynamic_partial_enforcement _
          | Rigid_tvar_escape _ | Opaque_type_from_module _ ),
          ( No_reason | From_witness_decl _ | From_witness_locl _
          | Instantiate _ | Flow _ | Def _ | Invalid | Missing_field | Idx _
          | Arith_ret_float _ | Arith_ret_num _ | Lost_info _ | Format _
          | Typeconst _ | Type_access _ | Expr_dep_type _
          | Contravariant_generic _ | Invariant_generic _ | Lambda_param _
          | Dynamic_coercion _ | Dynamic_partial_enforcement _
          | Rigid_tvar_escape _ | Opaque_type_from_module _ ) ) ->
        let step =
          let sub = push_solutions sub ~solutions
          and super = push_solutions super ~solutions in
          Subtype_step.begin_ ~sub ~super
        in
        derivation (step :: deriv)
    in

    aux (sub, super) ~deriv:[] ~solutions:Tvid.Map.empty

  (* ~~ Human-readable explanation of full derivation ~~~~~~~~~~~~~~~~~~~~~~~ *)

  module Explain = struct
    module Derivation_path = struct
      type elem =
        | Main
        | Upper
        | Lower
        | Solution
        | Step of int
      [@@deriving eq]

      let elem_to_string = function
        | Main -> "Main"
        | Upper -> "Upper"
        | Lower -> "Lower"
        | Solution -> "Solution"
        | Step n -> Format.sprintf "Step %d" (n + 1)

      type t = elem list [@@deriving eq]

      let explain elems =
        List.fold_left elems ~init:None ~f:(fun acc elem ->
            let part = elem_to_string elem in
            Some
              (Option.value_map
                 ~default:part
                 ~f:(Format.sprintf "%s / %s" part)
                 acc))
    end

    module State = struct
      type t = { defs_seen: Derivation_path.t Pos_or_decl.Map.t }

      let empty = { defs_seen = Pos_or_decl.Map.empty }

      let def_seen { defs_seen } pos path =
        { defs_seen = Pos_or_decl.Map.add pos path defs_seen }
    end

    module Config = struct
      type def_config =
        | Always  (** Always render a types definition whenever it appears *)
        | Once
            (** Render a types definition the first time it appears and provide a reference to it when it appear subsequently *)
        | Never  (** Don't render definitions *)

      type flow_config =
        | Full  (** Render all steps of a flow *)
        | Ends  (** Only render the start and endpoints of a flow *)

      type deriv_config =
        | Main  (** Render only the top-level derivation *)
        | All  (** Render all transitive derivations *)
        | Depth of int  (** Render derivations to a fixed depth  *)

      type t = {
        def_config: def_config;
        flow_config: flow_config;
        deriv_config: deriv_config;
        suppress_supportdyn: bool;
      }

      let verbose =
        {
          def_config = Always;
          flow_config = Full;
          deriv_config = All;
          suppress_supportdyn = false;
        }

      let chatty =
        {
          def_config = Once;
          flow_config = Full;
          deriv_config = Depth 2;
          suppress_supportdyn = true;
        }

      let terse =
        {
          def_config = Never;
          flow_config = Ends;
          deriv_config = Main;
          suppress_supportdyn = true;
        }

      let from_complexity complexity =
        if complexity = 0 then
          terse
        else if complexity = 1 then
          chatty
        else
          verbose

      let suppress_derivation { deriv_config; _ } cur_depth =
        match deriv_config with
        | All -> false
        | Main -> cur_depth > 0
        | Depth max_depth -> cur_depth > max_depth

      let get_def { def_config; suppress_supportdyn; _ } pos seen =
        match def_config with
        | Never -> None
        | _
          when suppress_supportdyn
               && String.equal
                    (Relative_path.suffix @@ Pos_or_decl.filename pos)
                    "supportdynamic.hhi" ->
          None
        | Always -> Some (Either.First pos)
        | Once ->
          (match Pos_or_decl.Map.find_opt pos seen with
          | Some path -> Some (Either.Second path)
          | None -> Some (Either.First pos))
    end

    module Context = struct
      type t = {
        derivation_depth: int;
        derivation_path: Derivation_path.t;
        in_flow: bool;
      }

      let empty =
        { derivation_depth = 0; derivation_path = []; in_flow = false }

      let deepen ({ derivation_depth = d; _ } as t) =
        { t with derivation_depth = d + 1 }

      let enter ({ derivation_path = d; _ } as t) elem =
        { t with derivation_path = elem :: d; in_flow = false }

      let enter_main t = enter t Derivation_path.Main

      let enter_upper t = enter t Derivation_path.Upper

      let enter_lower t = enter t Derivation_path.Lower

      let enter_solution t = enter t Derivation_path.Solution

      let enter_step t n = enter t @@ Derivation_path.Step n

      let set_in_flow t = { t with in_flow = true }

      let in_flow { in_flow; _ } = in_flow

      let explain_path { derivation_path; _ } =
        Derivation_path.explain derivation_path

      let is_toplevel { derivation_path; _ } =
        match derivation_path with
        | []
        | [Derivation_path.Main] ->
          true
        | _ -> false
    end

    let with_suffix ls ~suffix =
      match ls with
      | hd :: tl -> hd :: suffix :: tl
      | _ -> ls

    let explain_flow_kind = function
      | Flow_assign -> "via an assignment"
      | Flow_call -> "as the return type of the function call"
      | Flow_prop_access -> "as the type of property"
      | Flow_local -> "as the type of the local variable"
      | Flow_fun_return -> "as the return hint"
      | Flow_param_hint -> "as the parameter hint"
      | Flow_return_expr -> "because it is used in a return position"
      | Flow_instantiate nm ->
        Format.sprintf "as the instantiation of the generic `%s`" nm

    let rec explain t ~st ~cfg ~ctxt =
      match t with
      | _ when Config.suppress_derivation cfg ctxt.Context.derivation_depth ->
        ([], st)
      | Derivation steps -> explain_steps steps ~st ~cfg ~ctxt
      | Transitive { lower; upper; in_; on_ } ->
        let (expl_main, st) =
          explain in_ ~st ~cfg ~ctxt:(Context.enter_main ctxt)
        in
        let ctxt = Context.deepen ctxt in
        let (expl_lower, st) =
          explain lower ~st ~cfg ~ctxt:(Context.enter_lower ctxt)
        in
        let (expl_upper, st) =
          explain upper ~st ~cfg ~ctxt:(Context.enter_upper ctxt)
        in

        let main_path =
          Context.(Option.value_exn @@ explain_path @@ enter_main ctxt)
        in
        let prefix_main =
          let middle =
            match explain_reason on_ ~st ~cfg ~ctxt with
            | (Explanation.Witness (_, x) :: _, _) -> x
            | _ -> "an inferred type"
          in
          Explanation.Trans
            (Format.sprintf
               "I checked the subtype constraint in [%s] because it was implied by the other constraints on the %s."
               main_path
               middle)
        and prefix_lower =
          let lower_path =
            Context.(Option.value_exn @@ explain_path @@ enter_lower ctxt)
          in
          Explanation.Trans
            (Format.sprintf
               "I found the subtype for [%s] when I checked the subtype constraint in [%s]."
               main_path
               lower_path)
        and prefix_upper =
          let upper_path =
            Context.(Option.value_exn @@ explain_path @@ enter_upper ctxt)
          in
          Explanation.Trans
            (Format.sprintf
               "I found the supertype for [%s] when I checked the subtype constraint in [%s]."
               main_path
               upper_path)
        in
        let expl =
          (prefix_main :: expl_main)
          @ (prefix_lower :: expl_lower)
          @ (prefix_upper :: expl_upper)
        in
        (expl, st)
      | Lower { bound; in_ } ->
        let (expl_main, st) =
          explain in_ ~st ~cfg ~ctxt:(Context.enter_main ctxt)
        in
        let ctxt = Context.deepen ctxt in
        let (expl_lower, st) =
          explain bound ~st ~cfg ~ctxt:(Context.enter_lower ctxt)
        in
        let main_path =
          Context.(Option.value_exn @@ explain_path @@ enter_main ctxt)
        in
        let prefix_main =
          Explanation.Trans
            (Format.sprintf
               "I checked the subtype constraint in [%s] because it was implied by transitivity."
               main_path)
        and prefix_lower =
          let lower_path =
            Context.(Option.value_exn @@ explain_path @@ enter_lower ctxt)
          in
          Explanation.Trans
            (Format.sprintf
               "I found the subtype for [%s] is when I checked the subtype constraint in [%s]."
               main_path
               lower_path)
        in
        let expl = (prefix_main :: expl_main) @ (prefix_lower :: expl_lower) in
        (expl, st)

    and explain_steps steps ~st ~cfg ~ctxt =
      let steps =
        if cfg.Config.suppress_supportdyn then
          List.filter steps ~f:(fun step ->
              not @@ Subtype_step.uses_supportdyn step)
        else
          steps
      in
      let n_steps = List.length steps in
      let path = Context.explain_path ctxt in
      let pfx =
        Option.value_map path ~default:"Step" ~f:(Format.sprintf "[%s] Step")
      in
      let toplevel = Context.is_toplevel ctxt in
      let (_, st, acc) =
        List.fold_left
          ~f:(fun (idx, st, acc) step ->
            let (expl, st) =
              explain_step step (idx, n_steps, pfx, toplevel) ~st ~cfg ~ctxt
            in
            (idx + 1, st, expl :: acc))
          ~init:(0, st, [])
          steps
      in
      (List.fold_right acc ~init:[] ~f:(fun xs ys -> List.append ys xs), st)

    and explain_step step (idx, n_steps, pfx, toplevel) ~st ~cfg ~ctxt =
      let ctxt = Context.enter_step ctxt idx in
      let (sub, super, rule_opt, arg_opt) =
        match step with
        | Subtype_step.Begin { sub; super } -> (sub, super, None, None)
        | Subtype_step.Step { sub; super; rule; on_ } ->
          (sub, super, Some rule, Some on_)
      in

      let (expl_sub, st) =
        match (arg_opt, sub) with
        | (Some Subtype_step.Supertype, _) ->
          let pos = to_pos sub in
          ([Explanation.Witness (pos, "The subtype is the same as before.")], st)
        | (_, Missing_field) ->
          ( [
              Explanation.Witness_no_pos
                "The subtype didn't exist because the field was missing.";
            ],
            st )
        | _ ->
          let (expl_sub, st) = explain_reason sub ~st ~cfg ~ctxt in
          ( Explanation.Prefix
              { prefix = "The subtype comes from this"; sep = " " }
            :: expl_sub,
            st )
      in
      let (expl_super, st) =
        match (arg_opt, super) with
        | (Some Subtype_step.Subtype, _) ->
          let pos = to_pos super in
          ( [Explanation.Witness (pos, "The supertype is the same as before.")],
            st )
        | (_, Missing_field) ->
          ( [
              Explanation.Witness_no_pos
                "The supertype didn't exist because the field was missing.";
            ],
            st )
        | _ ->
          let (expl_super, st) = explain_reason super ~st ~cfg ~ctxt in
          let prefix =
            Explanation.Prefix
              { prefix = "The supertype comes from this"; sep = " " }
          in
          (prefix :: expl_super, st)
      in

      let rule =
        Explanation.Rule
          (Option.value_map
             rule_opt
             ~default:"I started by checking this subtype relationship."
             ~f:Subtype_rule.explain)
      in

      let step =
        Explanation.Step
          ( Format.sprintf "%s %d of %d" pfx (idx + 1) n_steps,
            toplevel && idx = n_steps - 1 )
      in

      ((step :: rule :: expl_sub) @ expl_super, st)

    and explain_reason reason ~st ~cfg ~ctxt =
      match reason with
      (* -- Expected 'atoms' in a derivation step -- *)
      | From_witness_locl witness -> ([explain_witness_locl witness], st)
      | From_witness_decl witness -> ([explain_witness_decl witness], st)
      | Def (pos, r) -> explain_def (pos, r) ~st ~cfg ~ctxt
      | Flow { from; into; kind } ->
        let into =
          match cfg.Config.flow_config with
          | Config.Full -> into
          | Config.Ends -> extract_last into
        in
        explain_flow (from, into, kind) ~st ~cfg ~ctxt
      | Solved { solution; in_; _ } ->
        explain_solved ~solution ~in_ ~st ~cfg ~ctxt
      | Missing_field ->
        (* This needs to be special cased in [explain_step] *)
        ([Explanation.Witness_no_pos "missing field"], st)
      (* Special handling of legacy structured reasons
         TODO(mjt) translate to flow?
      *)
      | Instantiate (r1, nm, r2) ->
        explain_instantiate (r1, nm, r2) ~st ~cfg ~ctxt
      | Idx (pos, r) -> explain_idx (pos, r) ~st ~cfg ~ctxt
      | Arith_ret_float (pos, r, arg_pos) ->
        explain_arith_ret_float (pos, r, arg_pos) ~st ~cfg ~ctxt
      | Arith_ret_num (pos, r, arg_pos) ->
        explain_arith_ret_num (pos, r, arg_pos) ~st ~cfg ~ctxt
      | Lost_info (nm, r, blame) ->
        explain_lost_info (nm, r, blame) ~st ~cfg ~ctxt
      | Format (pos, nm, r) -> explain_format (pos, nm, r) ~st ~cfg ~ctxt
      | Typeconst (r1, rs, str, r2) ->
        explain_typeconst (r1, rs, str, r2) ~st ~cfg ~ctxt
      | Type_access (r, rs) -> explain_type_access (r, rs) ~st ~cfg ~ctxt
      | Expr_dep_type (r, pos, expr_dep_type_reason) ->
        explain_expr_dep_type (r, pos, expr_dep_type_reason) ~st ~cfg ~ctxt
      | Contravariant_generic (r, nm) ->
        explain_contravariant_generic (r, nm) ~st ~cfg ~ctxt
      | Invariant_generic (r, nm) ->
        explain_invariant_generic (r, nm) ~st ~cfg ~ctxt
      | Lambda_param (pos, r) -> explain_lambda_param (pos, r) ~st ~cfg ~ctxt
      | Dynamic_coercion r -> explain_dynamic_coercion r ~st ~cfg ~ctxt
      | Dynamic_partial_enforcement (pos, nm, r) ->
        explain_dynamic_partial_enforcement (pos, nm, r) ~st ~cfg ~ctxt
      | Rigid_tvar_escape (pos, str1, str2, r) ->
        explain_rigid_tvar_escape (pos, str1, str2, r) ~st ~cfg ~ctxt
      | Opaque_type_from_module (pos, str, r) ->
        explain_opaque_type_from_module (pos, str, r) ~st ~cfg ~ctxt
      (* Its possible that one of the following remains in the derivation
          since we can have `Prj_one` in both subtype and supertype or
         `Prj_both` as subtype and `Prj_one` in supertype and we will always
         follow `Prj_one` before moving into `Prj_both` *)
      | Lower_bound { of_; _ } -> explain_reason of_ ~st ~cfg ~ctxt
      | Prj_both { sub_prj; _ } -> explain_reason sub_prj ~st ~cfg ~ctxt
      | Prj_one { part; _ } -> explain_reason part ~st ~cfg ~ctxt
      | Axiom { next; _ } -> explain_reason next ~st ~cfg ~ctxt
      (* We have no provenance information  *)
      | No_reason
      | Invalid ->
        ([Explanation.Witness_no_pos "this element"], st)

    and explain_witness_locl witness =
      match witness with
      | Is_refinement pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "`is` expression")
      | Witness pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "expression")
      | Type_variable (pos, _id) ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "type variable")
      | Type_variable_generics (pos, x, y, _id) ->
        Explanation.Witness
          ( Pos_or_decl.of_raw_pos pos,
            Format.sprintf "generic parameter `%s` of `%s`" x y )
      | Unpack_param (pos, _, _) ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "unpacked parameter")
      | Bitwise pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "expression")
      | Arith pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "arithmetic expression")
      | Idx_vector pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "index expression")
      | Splice pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "splice expression")
      | No_return pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "declaration")
      | Shape_literal pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "shape literal")
      | Destructure pos ->
        Explanation.Witness
          (Pos_or_decl.of_raw_pos pos, "destructure expression")
      | Join_point pos ->
        Explanation.Witness (Pos_or_decl.of_raw_pos pos, "join point")
      | _ ->
        Explanation.Witness
          ( witness_locl_to_raw_pos witness,
            Format.sprintf
              "element (`%s`)"
              (constructor_string_of_witness_locl witness) )

    and explain_witness_decl witness =
      match witness with
      | Hint pos -> Explanation.Witness (pos, "hint")
      | Witness_from_decl pos -> Explanation.Witness (pos, "declaration")
      | Support_dynamic_type pos ->
        Explanation.Witness (pos, "function or method declaration")
      | Pessimised_return pos -> Explanation.Witness (pos, "return hint")
      | Var_param_from_decl pos ->
        Explanation.Witness (pos, "variadic parameter declaration")
      | Tuple_from_splat pos ->
        Explanation.Witness (pos, "tuple from parameters")
      | Cstr_on_generics (pos, _) ->
        Explanation.Witness (pos, "constraint on the generic parameter")
      | Implicit_upper_bound (pos, nm) ->
        Explanation.Witness
          ( pos,
            Format.sprintf
              "implicit upper bound (`%s`) on the generic parameter"
              nm )
      | Class_class (pos, nm) ->
        Explanation.Witness
          ( pos,
            Format.sprintf
              "implicitly defined constant `::class` of class `%s`"
              nm )
      | _ ->
        Explanation.Witness
          ( witness_decl_to_raw_pos witness,
            Format.sprintf
              "element (`%s`)"
              (constructor_string_of_witness_decl witness) )

    and explain_def (pos, reason) ~st ~cfg ~ctxt =
      let (expl_reason, st) = explain_reason reason ~st ~cfg ~ctxt in
      match Config.get_def cfg pos st.State.defs_seen with
      | None -> (expl_reason, st)
      | Some (Either.Second seen_at) ->
        let expl_reason =
          if Derivation_path.equal seen_at ctxt.Context.derivation_path then
            let suffix =
              Explanation.Suffix
                { suffix = "(its definition was given above)"; sep = " " }
            in
            with_suffix expl_reason ~suffix
          else
            Option.value_map
              (Derivation_path.explain seen_at)
              ~default:expl_reason
              ~f:(fun path ->
                let suffix =
                  Explanation.Suffix
                    {
                      suffix =
                        Format.sprintf "(its definition was given in [%s])" path;
                      sep = " ";
                    }
                in
                with_suffix expl_reason ~suffix)
        in
        (expl_reason, st)
      | Some (Either.First pos) ->
        (* Suppress the 'definition' of supportdyn  *)
        if
          cfg.Config.suppress_supportdyn
          && Pos_or_decl.is_hhi pos
          && String.is_substring
               (Relative_path.suffix (Pos_or_decl.filename pos))
               ~substring:"supportdynamic"
        then
          (expl_reason, st)
        else
          let st = State.def_seen st pos ctxt.Context.derivation_path in
          ( expl_reason @ [Explanation.Witness (pos, "which is defined here")],
            st )

    and explain_flow (from, into, kind) ~st ~cfg ~ctxt =
      let (expl_from, st) = explain_reason from ~st ~cfg ~ctxt in
      let (expl_into, st) =
        explain_reason into ~st ~cfg ~ctxt:(Context.set_in_flow ctxt)
      in
      let suffix =
        Explanation.Suffix { suffix = explain_flow_kind kind; sep = " " }
      and prefix =
        Explanation.Prefix
          {
            prefix =
              (if Context.in_flow ctxt then
                "which itself flows into this"
              else
                "and flows into this");
            sep = " ";
          }
      in
      let expl_into = prefix :: with_suffix expl_into ~suffix in
      (expl_from @ expl_into, st)

    and explain_solved ~solution ~in_ ~st ~cfg ~ctxt =
      let (expl_in, st) = explain_reason in_ ~st ~cfg ~ctxt in
      let (expl_solution, st) =
        explain_reason solution ~st ~cfg ~ctxt:Context.(enter_solution ctxt)
      in
      let prefix =
        Explanation.Prefix { prefix = "which I solved to this"; sep = " " }
      in
      let expl_solution = prefix :: expl_solution in
      (expl_in @ expl_solution, st)

    and explain_instantiate (r1, nm, r2) ~st ~cfg ~ctxt =
      explain_flow (r1, r2, Flow_instantiate nm) ~st ~cfg ~ctxt

    and explain_idx (pos, _r) ~st ~cfg:_ ~ctxt:_ =
      ( [Explanation.Witness (Pos_or_decl.of_raw_pos pos, "index expression")],
        st )

    and explain_arith_ret_float (pos, _r, _arg_pos) ~st ~cfg:_ ~ctxt:_ =
      ( [
          Explanation.Witness
            (Pos_or_decl.of_raw_pos pos, "arithmetic expression");
        ],
        st )

    and explain_arith_ret_num (pos, _r, _arg_pos) ~st ~cfg:_ ~ctxt:_ =
      ( [
          Explanation.Witness
            (Pos_or_decl.of_raw_pos pos, "arithmetic expression");
        ],
        st )

    and explain_lost_info (_nm, r, _blame) ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt

    and explain_format (pos, _nm, _r) ~st ~cfg:_ ~ctxt:_ =
      ( [Explanation.Witness (Pos_or_decl.of_raw_pos pos, "format expression")],
        st )

    and explain_typeconst (_r1, (pos, _), _str, _t2) ~st ~cfg:_ ~ctxt:_ =
      ([Explanation.Witness (pos, "this type constant")], st)

    and explain_type_access (r, _rs) ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt

    and explain_expr_dep_type (r, _pos, _expr_dep_type_reason) ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt

    and explain_contravariant_generic (r, _nm) ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt

    and explain_invariant_generic (r, _nm) ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt

    and explain_lambda_param (pos, _r) ~st ~cfg:_ ~ctxt:_ =
      ( [Explanation.Witness (Pos_or_decl.of_raw_pos pos, "lambda parameter")],
        st )

    and explain_dynamic_coercion r ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt

    and explain_dynamic_partial_enforcement (_pos, _nm, r) ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt

    and explain_rigid_tvar_escape (_pos, _str1, _str2, r) ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt

    and explain_opaque_type_from_module (_pos, _str, r) ~st ~cfg ~ctxt =
      explain_reason r ~st ~cfg ~ctxt
  end

  let explain t ~complexity =
    let st = Explain.State.empty
    and cfg = Explain.Config.from_complexity complexity
    and ctxt = Explain.Context.empty in
    let (expl, _) = Explain.explain t ~st ~cfg ~ctxt in
    expl
end

let explain ~sub ~super ~complexity =
  Explanation.derivation
    Derivation.(explain (of_reason ~sub ~super) ~complexity)

let debug_reason ~sub ~super =
  let json =
    Hh_json.(
      JSON_Object
        [
          ( "Subtype",
            JSON_Object [("sub", to_json sub); ("super", to_json super)] );
        ])
  in
  Explanation.debug (Hh_json.json_to_string ~pretty:true json)

let debug_derivation ~sub ~super =
  let json = Derivation.(to_json @@ of_reason ~sub ~super) in
  Explanation.debug (Hh_json.json_to_string ~pretty:true json)
