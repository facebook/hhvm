(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type pos_id = Pos_or_decl.t * Ast_defs.id_ [@@deriving eq, hash, ord, show]

type arg_position =
  | Aonly
  | Afirst
  | Asecond
[@@deriving eq]

type expr_dep_type_reason =
  | ERexpr of int
  | ERstatic
  | ERclass of string
  | ERparent of string
  | ERself of string
  | ERpu of string
[@@deriving eq]

type blame_source =
  | BScall
  | BSlambda
  | BSassignment
  | BSout_of_scope
[@@deriving eq]

type blame = Blame of Pos.t * blame_source [@@deriving eq]

(* All the possible types, reason is a trace of why a type
   was inferred in a certain way.

   Types exists in two phases. Phase one is 'decl_phase', meaning it is a type that
   was declared in user code. Phase two is 'locl_phase', meaning it is a type that is
   inferred via local inference.
*)
(* create private types to represent the different type phases *)
type decl_phase = private DeclPhase [@@deriving eq, hash, show]

type locl_phase = private LoclPhase [@@deriving eq, hash, show]

(* The phase below helps enforce that only Pos_or_decl.t positions end up in the heap.
 * To enforce that, any reason taking a Pos.t should be a locl_phase t_
 * to prevent a decl ty from using it.
 * Reasons used for decl types should be 'phase t_ so that they can be localized
 * to be used in the localized version of the type. *)

(** The reason why something is expected to have a certain type *)
type _ t_ =
  | Rnone : 'phase t_
  | Rwitness : Pos.t -> locl_phase t_
  | Rwitness_from_decl : Pos_or_decl.t -> 'phase t_
  | Ridx : Pos.t * locl_phase t_ -> locl_phase t_
      (** Used as an index into a vector-like
          array or string. Position of indexing,
          reason for the indexed type *)
  | Ridx_vector : Pos.t -> locl_phase t_
  | Ridx_vector_from_decl : Pos_or_decl.t -> 'phase t_
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
  | Rret_fun_kind_from_decl : Pos_or_decl.t * Ast_defs.fun_kind -> 'phase t_
  | Rhint : Pos_or_decl.t -> 'phase t_
  | Rthrow : Pos.t -> locl_phase t_
  | Rplaceholder : Pos.t -> locl_phase t_
  | Rret_div : Pos.t -> locl_phase t_
  | Ryield_gen : Pos.t -> locl_phase t_
  | Ryield_asyncgen : Pos.t -> locl_phase t_
  | Ryield_asyncnull : Pos.t -> locl_phase t_
  | Ryield_send : Pos.t -> locl_phase t_
  | Rlost_info : string * locl_phase t_ * blame -> locl_phase t_
  | Rformat : Pos.t * string * locl_phase t_ -> locl_phase t_
  | Rclass_class : Pos_or_decl.t * string -> 'phase t_
  | Runknown_class : Pos.t -> locl_phase t_
  | Rvar_param : Pos.t -> locl_phase t_
  | Rvar_param_from_decl : Pos_or_decl.t -> 'phase t_
  | Runpack_param : Pos.t * Pos_or_decl.t * int -> locl_phase t_
      (** splat pos, fun def pos, number of args before splat *)
  | Rinout_param : Pos_or_decl.t -> 'phase t_
  | Rinstantiate : 'phase t_ * string * 'phase t_ -> 'phase t_
  | Rtypeconst :
      'phase t_ * (Pos_or_decl.t * string) * string Lazy.t * 'phase t_
      -> 'phase t_
  | Rtype_access :
      locl_phase t_ * (locl_phase t_ * string Lazy.t) list
      -> locl_phase t_
  | Rexpr_dep_type :
      'phase t_ * Pos_or_decl.t * expr_dep_type_reason
      -> 'phase t_
  | Rnullsafe_op : Pos.t -> locl_phase t_  (** ?-> operator is used *)
  | Rtconst_no_cstr : pos_id -> 'phase t_
  | Rpredicated : Pos.t * string -> locl_phase t_
  | Ris : Pos.t -> locl_phase t_
  | Ras : Pos.t -> locl_phase t_
  | Requal : Pos.t -> locl_phase t_
  | Rvarray_or_darray_key : Pos_or_decl.t -> 'phase t_
  | Rvec_or_dict_key : Pos_or_decl.t -> 'phase t_
  | Rusing : Pos.t -> locl_phase t_
  | Rdynamic_prop : Pos.t -> locl_phase t_
  | Rdynamic_call : Pos.t -> locl_phase t_
  | Rdynamic_construct : Pos.t -> locl_phase t_
  | Ridx_dict : Pos.t -> locl_phase t_
  | Rset_element : Pos.t -> locl_phase t_
  | Rmissing_optional_field : Pos_or_decl.t * string -> 'phase t_
  | Runset_field : Pos.t * string -> locl_phase t_
  | Rcontravariant_generic : locl_phase t_ * string -> locl_phase t_
  | Rinvariant_generic : locl_phase t_ * string -> locl_phase t_
  | Rregex : Pos.t -> locl_phase t_
  | Rimplicit_upper_bound : Pos_or_decl.t * string -> 'phase t_
  | Rtype_variable : Pos.t -> locl_phase t_
  | Rtype_variable_generics : Pos.t * string * string -> locl_phase t_
  | Rtype_variable_error : Pos.t -> locl_phase t_
  | Rglobal_type_variable_generics :
      Pos_or_decl.t * string * string
      -> 'phase t_
  | Rsolve_fail : Pos_or_decl.t -> 'phase t_
  | Rcstr_on_generics : Pos_or_decl.t * pos_id -> 'phase t_
  | Rlambda_param : Pos.t * locl_phase t_ -> locl_phase t_
  | Rshape : Pos.t * string -> locl_phase t_
  | Rshape_literal : Pos.t -> locl_phase t_
  | Renforceable : Pos_or_decl.t -> 'phase t_
  | Rdestructure : Pos.t -> locl_phase t_
  | Rkey_value_collection_key : Pos.t -> locl_phase t_
  | Rglobal_class_prop : Pos_or_decl.t -> 'phase t_
  | Rglobal_fun_param : Pos_or_decl.t -> 'phase t_
  | Rglobal_fun_ret : Pos_or_decl.t -> 'phase t_
  | Rsplice : Pos.t -> locl_phase t_
  | Ret_boolean : Pos.t -> locl_phase t_
  | Rdefault_capability : Pos_or_decl.t -> 'phase t_
  | Rconcat_operand : Pos.t -> locl_phase t_
  | Rinterp_operand : Pos.t -> locl_phase t_
  | Rdynamic_coercion of locl_phase t_
  | Rsupport_dynamic_type : Pos_or_decl.t -> 'phase t_
  | Rdynamic_partial_enforcement :
      Pos_or_decl.t * string * locl_phase t_
      -> locl_phase t_
  | Rrigid_tvar_escape :
      Pos.t * string * string * locl_phase t_
      -> locl_phase t_
  | Ropaque_type_from_module :
      Pos_or_decl.t * string * locl_phase t_
      -> locl_phase t_
  | Rmissing_class : Pos.t -> locl_phase t_
  | Rinvalid : 'phase t_
[@@deriving hash]

type t = locl_phase t_

type decl_t = decl_phase t_

val is_none : t -> bool

val localize : decl_phase t_ -> locl_phase t_

(** Translate a reason to a (pos, string) list, suitable for error_l. This
    previously returned a string, however the need to return multiple lines with
    multiple locations meant that it needed to more than convert to a string *)
val to_string : string -> 'phase t_ -> (Pos_or_decl.t * string) list

val to_pos : 'phase t_ -> Pos_or_decl.t

val expr_display_id_map : int IMap.t ref

val get_expr_display_id : int -> int

val get_expr_display_id_map : unit -> int IMap.t

val to_constructor_string : 'phase t_ -> string

val pp : Format.formatter -> 'phase t_ -> unit

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

val index_array : ureason

val index_tuple : ureason

val index_class : string -> ureason

val set_element : string -> ureason

val string_of_ureason : ureason -> string

val none : 'phase t_

val compare : 'phase t_ -> 'phase t_ -> int

val force_lazy_values : t -> t
