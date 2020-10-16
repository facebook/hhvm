(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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

(** The reason why something is expected to have a certain type *)
type t =
  | Rnone
  | Rwitness of Pos.t
  | Ridx of Pos.t * t
      (** Used as an index into a vector-like
          array or string. Position of indexing,
          reason for the indexed type *)
  | Ridx_vector of Pos.t  (** Used as an index, in the Vector case *)
  | Rforeach of Pos.t  (** Because it is iterated in a foreach loop *)
  | Rasyncforeach of Pos.t  (** Because it is iterated "await as" in foreach *)
  | Rarith of Pos.t
  | Rarith_ret of Pos.t
  | Rarith_ret_float of Pos.t * t * arg_position
      (** pos, arg float typing reason, arg position *)
  | Rarith_ret_num of Pos.t * t * arg_position
      (** pos, arg num typing reason, arg position *)
  | Rarith_ret_int of Pos.t
  | Rarith_dynamic of Pos.t
  | Rbitwise_dynamic of Pos.t
  | Rincdec_dynamic of Pos.t
  | Rcomp of Pos.t
  | Rconcat_ret of Pos.t
  | Rlogic_ret of Pos.t
  | Rbitwise of Pos.t
  | Rbitwise_ret of Pos.t
  | Rno_return of Pos.t
  | Rno_return_async of Pos.t
  | Rret_fun_kind of Pos.t * Ast_defs.fun_kind
  | Rhint of Pos.t
  | Rthrow of Pos.t
  | Rplaceholder of Pos.t
  | Rret_div of Pos.t
  | Ryield_gen of Pos.t
  | Ryield_asyncgen of Pos.t
  | Ryield_asyncnull of Pos.t
  | Ryield_send of Pos.t
  | Rlost_info of string * t * blame
  | Rformat of Pos.t * string * t
  | Rclass_class of Pos.t * string
  | Runknown_class of Pos.t
  | Rvar_param of Pos.t
  | Runpack_param of Pos.t * Pos.t * int
      (** splat pos, fun def pos, number of args before splat *)
  | Rinout_param of Pos.t
  | Rinstantiate of t * string * t
  | Rarray_filter of Pos.t * t
  | Rtypeconst of t * (Pos.t * string) * string * t
  | Rtype_access of t * (t * string) list
  | Rexpr_dep_type of t * Pos.t * expr_dep_type_reason
  | Rnullsafe_op of Pos.t  (** ?-> operator is used *)
  | Rtconst_no_cstr of Aast.sid
  | Rpredicated of Pos.t * string
  | Ris of Pos.t
  | Ras of Pos.t
  | Rvarray_or_darray_key of Pos.t
  | Rusing of Pos.t
  | Rdynamic_prop of Pos.t
  | Rdynamic_call of Pos.t
  | Ridx_dict of Pos.t
  | Rmissing_required_field of Pos.t * string
  | Rmissing_optional_field of Pos.t * string
  | Runset_field of Pos.t * string
  | Rcontravariant_generic of t * string
  | Rinvariant_generic of t * string
  | Rregex of Pos.t
  | Rimplicit_upper_bound of Pos.t * string
  | Rtype_variable of Pos.t
  | Rtype_variable_generics of Pos.t * string * string
  | Rsolve_fail of Pos.t
  | Rcstr_on_generics of Pos.t * Aast.sid
  | Rlambda_param of Pos.t * t
  | Rshape of Pos.t * string
  | Renforceable of Pos.t
  | Rdestructure of Pos.t
  | Rkey_value_collection_key of Pos.t
  | Rglobal_class_prop of Pos.t
  | Rglobal_fun_param of Pos.t
  | Rglobal_fun_ret of Pos.t
  | Rsplice of Pos.t
[@@deriving eq]

(** Translate a reason to a (pos, string) list, suitable for error_l. This
    previously returned a string, however the need to return multiple lines with
    multiple locations meant that it needed to more than convert to a string *)
val to_string : string -> t -> (Pos.t * string) list

val to_pos : t -> Pos.t

val expr_display_id_map : int IMap.t ref

val get_expr_display_id : int -> int

val to_constructor_string : t -> string

val pp : Format.formatter -> t -> unit

type ureason =
  | URnone
  | URassign
  | URassign_inout
  | URhint
  | URreturn
  | URforeach
  | URthrow
  | URvector
  | URkey
  | URvalue
  | URawait
  | URyield
  | URxhp of string * string  (** Name of XHP class, Name of XHP attribute *)
  | URxhp_spread
  | URindex of string
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
[@@deriving show]

val index_array : ureason

val index_tuple : ureason

val index_class : string -> ureason

val string_of_ureason : ureason -> string

val none : t

val compare : t -> t -> int

val explain_generic_constraint :
  Pos.t -> t -> string -> (Pos.t * string) list -> unit
