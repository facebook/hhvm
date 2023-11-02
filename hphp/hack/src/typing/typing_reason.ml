(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let strip_ns id =
  id |> Utils.strip_ns |> Hh_autoimport.strip_HH_namespace_if_autoimport

type pos_id = (Pos_or_decl.t[@hash.ignore]) * Ast_defs.id_
[@@deriving eq, hash, ord, show]

type arg_position =
  | Aonly
  | Afirst
  | Asecond
[@@deriving eq, hash, show]

type expr_dep_type_reason =
  | ERexpr of Ident_provider.Ident.t
  | ERstatic
  | ERclass of string
  | ERparent of string
  | ERself of string
  | ERpu of string
[@@deriving eq, hash, show]

type blame_source =
  | BScall
  | BSlambda
  | BSassignment
  | BSout_of_scope
[@@deriving eq, hash, show]

type blame = Blame of Pos.t * blame_source [@@deriving eq, hash, show]

(* create private types to represent the different type phases *)
type decl_phase = private DeclPhase [@@deriving eq, hash, show]

type locl_phase = private LoclPhase [@@deriving eq, hash, show]

(* This is to avoid a compile error with ppx_hash "Unbound value _hash_fold_phase". *)
let _hash_fold_phase hsv _ = hsv

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
  | Ris : Pos.t -> locl_phase t_
  | Ras : Pos.t -> locl_phase t_
  | Requal : Pos.t -> locl_phase t_
  | Rvarray_or_darray_key : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rvec_or_dict_key : (Pos_or_decl.t[@hash.ignore]) -> 'phase t_
  | Rusing : Pos.t -> locl_phase t_
  | Rdynamic_prop : Pos.t -> locl_phase t_
  | Rdynamic_call : Pos.t -> locl_phase t_
  | Rdynamic_construct : Pos.t -> locl_phase t_
  | Ridx_dict : Pos.t -> locl_phase t_
  | Rset_element : Pos.t -> locl_phase t_
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
  | Rpessimised_inout : Pos_or_decl.t -> 'phase t_
  | Rpessimised_return : Pos_or_decl.t -> 'phase t_
  | Rpessimised_prop : Pos_or_decl.t -> 'phase t_
[@@deriving hash]

let rec to_raw_pos : type ph. ph t_ -> Pos_or_decl.t =
 fun r ->
  match r with
  | Rnone
  | Rinvalid ->
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
  | Ris p
  | Ras p
  | Requal p
  | Rusing p
  | Rdynamic_prop p
  | Rdynamic_call p
  | Rdynamic_construct p
  | Ridx_dict p
  | Rset_element p
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
  | Rrigid_tvar_escape (p, _, _, _) ->
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
  | Ris _ -> "Ris"
  | Ras _ -> "Ras"
  | Requal _ -> "Requal"
  | Rvarray_or_darray_key _ -> "Rvarray_or_darray_key"
  | Rvec_or_dict_key _ -> "Rvec_or_dict_key"
  | Rusing _ -> "Rusing"
  | Rdynamic_prop _ -> "Rdynamic_prop"
  | Rdynamic_call _ -> "Rdynamic_call"
  | Rdynamic_construct _ -> "Rdynamic_construct"
  | Ridx_dict _ -> "Ridx_dict"
  | Rset_element _ -> "Rset_element"
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
  | Rinvalid ->
    ()
  | _ ->
    Format.fprintf fmt "@ (@[";
    (match r with
    | Rnone
    | Rinvalid ->
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
    | Ris p
    | Ras p
    | Requal p
    | Rusing p
    | Rdynamic_prop p
    | Rdynamic_call p
    | Rdynamic_construct p
    | Ridx_dict p
    | Rset_element p
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
    | Rcaptured_like p ->
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
    | Rdynamic_coercion r -> pp_t_ fmt r);
    Format.fprintf fmt "@])"

and show_t_ : type ph. ph t_ -> string = (fun r -> Format.asprintf "%a" pp_t_ r)

type t = locl_phase t_

let pp : type ph. _ -> ph t_ -> unit = (fun fmt r -> pp_t_ fmt r)

let show r = Format.asprintf "%a" pp r

type decl_t = decl_phase t_

let is_none = function
  | Rnone -> true
  | _ -> false

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

let arg_pos_str ap =
  match ap with
  | Aonly -> "only"
  | Afirst -> "first"
  | Asecond -> "second"

(* This is a mapping from internal expression ids to a standardized int.
 * Used for outputting cleaner error messages to users
 *)
let expr_display_id_map = ref Ident_provider.Ident.Map.empty

let reset_expr_display_id_map () =
  expr_display_id_map := Ident_provider.Ident.Map.empty

let get_expr_display_id id =
  let map = !expr_display_id_map in
  match Ident_provider.Ident.Map.find_opt id map with
  | Some n -> n
  | None ->
    let n = Ident_provider.Ident.Map.cardinal map + 1 in
    expr_display_id_map := Ident_provider.Ident.Map.add id n map;
    n

let get_expr_display_id_map () = !expr_display_id_map

(* Translate a reason to a (pos, string) list, suitable for error_l. This
 * previously returned a string, however the need to return multiple lines with
 * multiple locations meant that it needed to more than convert to a string *)
let rec to_string : type ph. string -> ph t_ -> (Pos_or_decl.t * string) list =
 fun prefix r ->
  let p = to_pos r in
  match r with
  | Rnone -> [(p, prefix)]
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
  | Ris _ -> [(p, prefix ^ " from this `is` expression test")]
  | Ras _ -> [(p, prefix ^ " from this \"as\" assertion")]
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
  | Rset_element _ ->
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

and to_pos : type ph. ph t_ -> Pos_or_decl.t =
 fun r ->
  if !Errors.report_pos_from_reason then
    Pos_or_decl.set_from_reason (to_raw_pos r)
  else
    to_raw_pos r

and expr_dep_type_reason_string = function
  | ERexpr id ->
    let did = get_expr_display_id id in
    "where "
    ^ Markdown_lite.md_codify ("<expr#" ^ string_of_int did ^ ">")
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

let none = Rnone

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
        | Ris x -> Ris x
        | Ras x -> Ras x
        | Requal x -> Requal x
        | Rvarray_or_darray_key x -> Rvarray_or_darray_key x
        | Rvec_or_dict_key x -> Rvec_or_dict_key x
        | Rusing x -> Rusing x
        | Rdynamic_prop x -> Rdynamic_prop x
        | Rdynamic_call x -> Rdynamic_call x
        | Rdynamic_construct x -> Rdynamic_construct x
        | Ridx_dict x -> Ridx_dict x
        | Rset_element x -> Rset_element x
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
