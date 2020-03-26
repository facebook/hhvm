(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let strip_ns id = id |> Utils.strip_ns |> Hh_autoimport.reverse_type

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
  | Rarith_int of Pos.t
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
  | Rlost_info of string * t * Pos.t * bool  (** true if due to lambda *)
  | Rformat of Pos.t * string * t
  | Rclass_class of Pos.t * string
  | Runknown_class of Pos.t
  | Rdynamic_yield of Pos.t * Pos.t * string * string
  | Rmap_append of Pos.t
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
  | Rlambda_use of Pos.t
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

and arg_position =
  | Aonly
  | Afirst
  | Asecond

and expr_dep_type_reason =
  | ERexpr of int
  | ERstatic
  | ERclass of string
  | ERparent of string
  | ERself of string
[@@deriving eq]

let arg_pos_str ap =
  match ap with
  | Aonly -> "only"
  | Afirst -> "first"
  | Asecond -> "second"

(* Translate a reason to a (pos, string) list, suitable for error_l. This
 * previously returned a string, however the need to return multiple lines with
 * multiple locations meant that it needed to more than convert to a string *)
let rec to_string prefix r =
  let p = to_pos r in
  match r with
  | Rnone -> [(p, prefix)]
  | Rwitness _ -> [(p, prefix)]
  | Ridx (_, r2) ->
    [(p, prefix)]
    @ [
        ( ( if equal r2 Rnone then
            p
          else
            to_pos r2 ),
          "This can only be indexed with integers" );
      ]
  | Ridx_vector _ ->
    [
      ( p,
        prefix ^ " because only int can be used to index into a Vector or vec."
      );
    ]
  | Rforeach _ -> [(p, prefix ^ " because this is used in a foreach statement")]
  | Rasyncforeach _ ->
    [
      ( p,
        prefix
        ^ " because this is used in a foreach statement with \"await as\"" );
    ]
  | Rarith _ ->
    [(p, prefix ^ " because this is used in an arithmetic operation")]
  | Rarith_int _ ->
    [(p, prefix ^ " because this is used in integer arithmetic operation")]
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
        ^ " because this is the result of an arithmetic operation with a float as the "
        ^ arg_pos_str s
        ^ " argument." );
    ]
    @ to_string
        "Here is why I think the argument is a float: this is a float"
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
        ^ " because this is the result of an arithmetic operation with a num as the "
        ^ arg_pos_str s
        ^ " argument, and no floats." );
    ]
    @ to_string
        "Here is why I think the argument is a num: this is a num"
        r_last
  | Rarith_ret_int _ ->
    [
      ( p,
        prefix
        ^ " because this is the result of an integer arithmetic operation" );
    ]
  | Rarith_dynamic _ ->
    [
      ( p,
        prefix
        ^ " because this is the result of an arithmetic operation with two arguments typed dynamic"
      );
    ]
  | Rbitwise_dynamic _ ->
    [
      ( p,
        prefix
        ^ " because this is the result of a bitwise operation with all arguments typed dynamic"
      );
    ]
  | Rincdec_dynamic _ ->
    [
      ( p,
        prefix
        ^ " because this is the result of an increment/decrement of an argument typed dynamic"
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
          prefix ^ " (result of 'async function' containing a 'yield')"
        | Ast_defs.FGenerator ->
          prefix ^ " (result of function containing a 'yield')"
        | Ast_defs.FAsync -> prefix ^ " (result of 'async function')"
        | Ast_defs.FCoroutine
        | Ast_defs.FSync ->
          prefix );
    ]
  | Rhint _ -> [(p, prefix)]
  | Rthrow _ -> [(p, prefix ^ " because it is used as an exception")]
  | Rplaceholder _ ->
    [(p, prefix ^ " ($_ is a placeholder variable not meant to be used)")]
  | Rret_div _ -> [(p, prefix ^ " because it is the result of a division (/)")]
  | Ryield_gen _ ->
    [(p, prefix ^ " (result of function with 'yield' in the body)")]
  | Ryield_asyncgen _ ->
    [(p, prefix ^ " (result of 'async function' with 'yield' in the body)")]
  | Ryield_asyncnull _ ->
    [
      ( p,
        prefix
        ^ " because \"yield x\" is equivalent to \"yield null => x\" in an async function"
      );
    ]
  | Ryield_send _ ->
    [
      ( p,
        prefix
        ^ " ($generator->send() can always send a null back to a \"yield\")" );
    ]
  | Rvar_param _ -> [(p, prefix ^ " (variadic argument)")]
  | Runpack_param _ -> [(p, prefix ^ " (it is unpacked with '...')")]
  | Rinout_param _ -> [(p, prefix ^ " (inout parameter)")]
  | Rnullsafe_op _ -> [(p, prefix ^ " (use of ?-> operator)")]
  | Rlost_info (s, r1, p2, under_lambda) ->
    let s = strip_ns s in
    let cause =
      if under_lambda then
        "by this lambda function"
      else
        "during this call"
    in
    to_string prefix r1
    @ [
        ( p2,
          "All the local information about "
          ^ s
          ^ " has been invalidated "
          ^ cause
          ^ ".\nThis is a limitation of the type-checker, use a local if that's the problem."
        );
      ]
  | Rformat (_, s, t) ->
    let s = prefix ^ " because of the " ^ s ^ " format specifier" in
    (match to_string "" t with
    | [(_, "")] -> [(p, s)]
    | el -> [(p, s)] @ el)
  | Rclass_class (_, s) ->
    [
      ( p,
        prefix
        ^ "; implicitly defined constant ::class is a string that contains the fully qualified name of "
        ^ strip_ns s );
    ]
  | Runknown_class _ -> [(p, prefix ^ "; this class name is unknown to Hack")]
  | Rdynamic_yield (_, yield_pos, implicit_name, yield_name) ->
    [
      ( p,
        prefix
        ^ Printf.sprintf
            "\n%s\nDynamicYield implicitly defines %s() from the definition of %s()"
            (Pos.string (Pos.to_absolute yield_pos))
            implicit_name
            yield_name );
    ]
  | Rmap_append _ ->
    [
      ( p,
        prefix
        ^ " because you can only append a Pair<Tkey, Tvalue> to an Map<Tkey, Tvalue>"
      );
    ]
  | Rinstantiate (r_orig, generic_name, r_inst) ->
    to_string prefix r_orig
    @ to_string ("  via this generic " ^ generic_name) r_inst
  | Rtype_variable p ->
    [(p, prefix ^ " because a type could not be determined here")]
  | Rtype_variable_generics (p, tp_name, s) ->
    [
      ( p,
        prefix
        ^ " because type parameter "
        ^ tp_name
        ^ " of "
        ^ s
        ^ " could not be determined. Please add explicit type parameters to the invocation of "
        ^ s );
    ]
  | Rsolve_fail p ->
    [(p, prefix ^ " because a type could not be determined here")]
  | Rarray_filter (_, r) ->
    to_string prefix r
    @ [
        ( p,
          "array_filter converts KeyedContainer<Tk, Tv> to array<Tk, Tv>, and Container<Tv> to array<arraykey, Tv>. Single argument calls additionally remove nullability from Tv."
        );
      ]
  | Rtypeconst (Rnone, (pos, tconst), ty_str, r_root) ->
    let prefix =
      if String.equal prefix "" then
        ""
      else
        prefix ^ "\n  "
    in
    [(pos, sprintf "%sby accessing the type constant '%s'" prefix tconst)]
    @ to_string ("on " ^ ty_str) r_root
  | Rtypeconst (r_orig, (pos, tconst), ty_str, r_root) ->
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
  | Rtype_access (r, (r_hd, tconst) :: tail) ->
    to_string prefix r
    @ to_string ("  resulting from expanding the type constant " ^ tconst) r_hd
    @ List.concat_map tail ~f:(fun (r, s) ->
          to_string ("  then expanding the type constant " ^ s) r)
  | Rexpr_dep_type (r, p, e) ->
    to_string prefix r @ [(p, "  " ^ expr_dep_type_reason_string e)]
  | Rtconst_no_cstr (_, n) ->
    [(p, prefix ^ " because the type constant " ^ n ^ " has no constraints")]
  | Rpredicated (p, f) ->
    [(p, prefix ^ " from the argument to this " ^ f ^ " test")]
  | Ris p -> [(p, prefix ^ " from this `is` expression test")]
  | Ras p -> [(p, prefix ^ " from this \"as\" assertion")]
  | Rvarray_or_darray_key _ ->
    [
      ( p,
        "This is varray_or_darray, which requires arraykey-typed keys when used with an array (used like a hashtable)"
      );
    ]
  | Rusing p -> [(p, prefix ^ " because it was assigned in a 'using' clause")]
  | Rdynamic_prop p ->
    [(p, prefix ^ ", the result of accessing a property of a dynamic type")]
  | Rdynamic_call p ->
    [(p, prefix ^ ", the result of calling a dynamic type as a function")]
  | Ridx_dict _ ->
    [
      ( p,
        prefix
        ^ " because only array keys can be used to index into a Map, dict, darray, Set, or keyset"
      );
    ]
  | Rmissing_required_field (p, name) ->
    [
      ( p,
        prefix
        ^ " because the field '"
        ^ name
        ^ "' is not defined in this shape type, "
        ^ "and this shape type does not allow unknown fields" );
    ]
  | Rmissing_optional_field (p, name) ->
    [
      ( p,
        prefix
        ^ " because the field '"
        ^ name
        ^ "' may be set to any type in this shape" );
    ]
  | Runset_field (p, name) ->
    [(p, prefix ^ " because the field '" ^ name ^ "' was unset here")]
  | Rcontravariant_generic (r_orig, class_name) ->
    to_string prefix r_orig
    @ [
        ( p,
          "Considering that this type argument is contravariant with respect to "
          ^ strip_ns class_name );
      ]
  | Rinvariant_generic (r_orig, class_name) ->
    to_string prefix r_orig
    @ [
        ( p,
          "Considering that this type argument is invariant with respect to "
          ^ strip_ns class_name );
      ]
  | Rregex _ -> [(p, prefix ^ " resulting from this regex pattern")]
  | Rlambda_use p ->
    [(p, prefix ^ " because the lambda function was used here")]
  | Rimplicit_upper_bound (_, cstr) ->
    [
      ( p,
        prefix
        ^ " arising from an implicit 'as "
        ^ cstr
        ^ "' constraint on this type" );
    ]
  | Rcstr_on_generics _ -> [(p, prefix)]
  (* If type originated with an unannotated lambda parameter with type variable type,
   * suggested annotating the lambda parameter. Otherwise defer to original reason. *)
  | Rlambda_param
      ( p,
        ( Rsolve_fail _ | Rtype_variable_generics _ | Rtype_variable _
        | Rinstantiate _ ) ) ->
    [
      ( p,
        prefix
        ^ " because the type of the lambda parameter could not be determined. Please add a type hint to the parameter"
      );
    ]
  | Rlambda_param (_, r_orig) -> to_string prefix r_orig
  | Rshape (p, fun_name) ->
    [(p, prefix ^ " because " ^ fun_name ^ " expects a shape")]
  | Renforceable p -> [(p, prefix ^ " because it is an unenforceable type")]
  | Rdestructure p ->
    [(p, prefix ^ " resulting from a list destructuring assignment or a splat")]
  | Rkey_value_collection_key _ ->
    [(p, "This is a key-value collection, which requires arraykey-typed keys")]
  | Rglobal_class_prop p -> [(p, prefix)]
  | Rglobal_fun_param p -> [(p, prefix)]
  | Rglobal_fun_ret p -> [(p, prefix)]

and to_pos = function
  | Rnone -> Pos.none
  | Rwitness p -> p
  | Ridx (p, _) -> p
  | Ridx_vector p -> p
  | Rforeach p -> p
  | Rasyncforeach p -> p
  | Rarith p -> p
  | Rarith_ret p -> p
  | Rarith_dynamic p -> p
  | Rcomp p -> p
  | Rconcat_ret p -> p
  | Rlogic_ret p -> p
  | Rbitwise p -> p
  | Rbitwise_ret p -> p
  | Rno_return p -> p
  | Rno_return_async p -> p
  | Rret_fun_kind (p, _) -> p
  | Rhint p -> p
  | Rthrow p -> p
  | Rplaceholder p -> p
  | Rret_div p -> p
  | Ryield_gen p -> p
  | Ryield_asyncgen p -> p
  | Ryield_asyncnull p -> p
  | Ryield_send p -> p
  | Rlost_info (_, r, _, _) -> to_pos r
  | Rformat (p, _, _) -> p
  | Rclass_class (p, _) -> p
  | Runknown_class p -> p
  | Rdynamic_yield (p, _, _, _) -> p
  | Rmap_append p -> p
  | Rvar_param p -> p
  | Runpack_param (p, _, _) -> p
  | Rinout_param p -> p
  | Rinstantiate (_, _, r) -> to_pos r
  | Rtypeconst (Rnone, (p, _), _, _)
  | Rarray_filter (p, _) ->
    p
  | Rtypeconst (r, _, _, _)
  | Rtype_access (r, _) ->
    to_pos r
  | Rexpr_dep_type (r, _, _) -> to_pos r
  | Rnullsafe_op p -> p
  | Rtconst_no_cstr (p, _) -> p
  | Rpredicated (p, _) -> p
  | Ris p -> p
  | Ras p -> p
  | Rvarray_or_darray_key p -> p
  | Rusing p -> p
  | Rdynamic_prop p -> p
  | Rdynamic_call p -> p
  | Ridx_dict p -> p
  | Rmissing_required_field (p, _) -> p
  | Rmissing_optional_field (p, _) -> p
  | Runset_field (p, _) -> p
  | Rcontravariant_generic (r, _) -> to_pos r
  | Rinvariant_generic (r, _) -> to_pos r
  | Rregex p -> p
  | Rlambda_use p -> p
  | Rimplicit_upper_bound (p, _) -> p
  | Rarith_int p -> p
  | Rarith_ret_float (p, _, _) -> p
  | Rarith_ret_num (p, _, _) -> p
  | Rarith_ret_int p -> p
  | Rbitwise_dynamic p -> p
  | Rincdec_dynamic p -> p
  | Rtype_variable p -> p
  | Rtype_variable_generics (p, _, _) -> p
  | Rsolve_fail p -> p
  | Rcstr_on_generics (p, _) -> p
  | Rlambda_param (p, _) -> p
  | Rshape (p, _) -> p
  | Renforceable p -> p
  | Rdestructure p -> p
  | Rkey_value_collection_key p -> p
  | Rglobal_class_prop p -> p
  | Rglobal_fun_param p -> p
  | Rglobal_fun_ret p -> p

(* This is a mapping from internal expression ids to a standardized int.
 * Used for outputting cleaner error messages to users
 *)
and expr_display_id_map = ref IMap.empty

and get_expr_display_id id =
  let map = !expr_display_id_map in
  match IMap.find_opt id map with
  | Some n -> n
  | None ->
    let n = IMap.cardinal map + 1 in
    expr_display_id_map := IMap.add id n map;
    n

and expr_dep_type_reason_string = function
  | ERexpr id ->
    let did = get_expr_display_id id in
    "where '<expr#" ^ string_of_int did ^ ">' is a reference to this expression"
  | ERstatic ->
    "where '<static>' refers to the late bound type of the enclosing class"
  | ERclass c -> "where the class '" ^ strip_ns c ^ "' was referenced here"
  | ERparent p ->
    "where the class '"
    ^ strip_ns p
    ^ "' (the parent of the enclosing) class was referenced here"
  | ERself c ->
    "where the class '"
    ^ strip_ns c
    ^ "' was referenced here via the keyword 'self'"

let to_constructor_string r =
  match r with
  | Rnone -> "Rnone"
  | Rwitness _ -> "Rwitness"
  | Ridx _ -> "Ridx"
  | Ridx_vector _ -> "Ridx_vector"
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
  | Rdynamic_yield _ -> "Rdynamic_yield"
  | Rmap_append _ -> "Rmap_append"
  | Rvar_param _ -> "Rvar_param"
  | Runpack_param _ -> "Runpack_param"
  | Rinout_param _ -> "Rinout_param"
  | Rinstantiate _ -> "Rinstantiate"
  | Rarray_filter _ -> "Rarray_filter"
  | Rtypeconst _ -> "Rtypeconst"
  | Rtype_access _ -> "Rtype_access"
  | Rexpr_dep_type _ -> "Rexpr_dep_type"
  | Rnullsafe_op _ -> "Rnullsafe_op"
  | Rtconst_no_cstr _ -> "Rtconst_no_cstr"
  | Rpredicated _ -> "Rpredicated"
  | Ris _ -> "Ris"
  | Ras _ -> "Ras"
  | Rvarray_or_darray_key _ -> "Rvarray_or_darray_key"
  | Rusing _ -> "Rusing"
  | Rdynamic_prop _ -> "Rdynamic_prop"
  | Rdynamic_call _ -> "Rdynamic_call"
  | Ridx_dict _ -> "Ridx_dict"
  | Rmissing_required_field _ -> "Rmissing_required_field"
  | Rmissing_optional_field _ -> "Rmissing_optional_field"
  | Runset_field _ -> "Runset_field"
  | Rcontravariant_generic _ -> "Rcontravariant_generic"
  | Rinvariant_generic _ -> "Rinvariant_generic"
  | Rregex _ -> "Rregex"
  | Rlambda_use _ -> "Rlambda_use"
  | Rimplicit_upper_bound _ -> "Rimplicit_upper_bound"
  | Rarith_int _ -> "Rarith_int"
  | Rarith_ret_num _ -> "Rarith_ret_num"
  | Rarith_ret_float _ -> "Rarith_ret_float"
  | Rarith_ret_int _ -> "Rarith_ret_int"
  | Rarith_dynamic _ -> "Rarith_dynamic"
  | Rbitwise_dynamic _ -> "Rbitwise_dynamic"
  | Rincdec_dynamic _ -> "Rincdec_dynamic"
  | Rtype_variable _ -> "Rtype_variable"
  | Rtype_variable_generics _ -> "Rtype_variable_generics"
  | Rsolve_fail _ -> "Rsolve_fail"
  | Rcstr_on_generics _ -> "Rcstr_on_generics"
  | Rlambda_param _ -> "Rlambda_param"
  | Rshape _ -> "Rshape"
  | Renforceable _ -> "Renforceable"
  | Rdestructure _ -> "Rdestructure"
  | Rkey_value_collection_key _ -> "Rkey_value_collection_key"
  | Rglobal_class_prop _ -> "Rglobal_class_prop"
  | Rglobal_fun_param _ -> "Rglobal_fun_param"
  | Rglobal_fun_ret _ -> "Rglobal_fun_ret"

let pp fmt r =
  let pos = to_pos r in
  Format.pp_print_string fmt
  @@ Printf.sprintf
       "%s (%s)"
       (to_constructor_string r)
       (Printf.sprintf
          "%s %s"
          (Relative_path.S.to_string (Pos.filename pos))
          (Pos.string_no_file pos))

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
  | URyield_from
  | URxhp of string * string  (** Name of XHP class, Name of XHP attribute *)
  | URxhp_spread
  | URindex of string
  | URparam
  | URparam_inout
  | URarray_value
  | URtuple_access
  | URpair_access
  | URnewtype_cstr
  | URclass_req
  | URenum
  | URenum_cstr
  | URenum_underlying
  | URenum_incompatible_cstr
  | URtypeconst_cstr
  | URsubsume_tconst_cstr
  | URsubsume_tconst_assign
  | URclone
  | URusing
[@@deriving show]

let index_array = URindex "array"

let index_tuple = URindex "tuple"

let index_class s = URindex (strip_ns s)

let string_of_ureason = function
  | URnone -> "Typing error"
  | URreturn -> "Invalid return type"
  | URhint -> "Wrong type hint"
  | URassign -> "Invalid assignment"
  | URassign_inout -> "Invalid assignment to an inout parameter"
  | URforeach -> "Invalid foreach"
  | URthrow -> "Invalid exception"
  | URvector -> "Some elements in this collection are incompatible"
  | URkey -> "The keys of this Map are incompatible"
  | URvalue -> "The values of this Map are incompatible"
  | URawait -> "await can only operate on an Awaitable"
  | URyield -> "Invalid yield"
  | URyield_from -> "Invalid yield from"
  | URxhp (cls, attr) ->
    "Invalid xhp value for attribute " ^ attr ^ " in " ^ strip_ns cls
  | URxhp_spread -> "The attribute spread operator cannot be called on non-XHP"
  | URindex s -> "Invalid index type for this " ^ strip_ns s
  | URparam -> "Invalid argument"
  | URparam_inout -> "Invalid argument to an inout parameter"
  | URarray_value -> "Incompatible field values"
  | URtuple_access ->
    "Tuple elements can only be accessed with an integer literal"
  | URpair_access ->
    "Pair elements can only be accessed with an integer literal"
  | URnewtype_cstr -> "Invalid constraint on newtype"
  | URclass_req -> "Unable to satisfy trait/interface requirement"
  | URenum -> "Constant does not match the type of the enum it is in"
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

let compare r1 r2 =
  let get_pri = function
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
    Pos.compare (to_pos r1) (to_pos r2)

let none = Rnone

(*****************************************************************************)
(* When the subtyping fails because of a constraint. *)
(*****************************************************************************)

let explain_generic_constraint p_inst reason name error =
  let pos = to_pos reason in
  Errors.explain_constraint
    ~use_pos:p_inst
    ~definition_pos:pos
    ~param_name:name
    error
