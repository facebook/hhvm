(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open Utils

(* The reason why something is expected to have a certain type *)
type t =
  | Rnone
  | Rwitness         of Pos.t
  | Ridx             of Pos.t * t (* Used as an index into into a vector-like
                                     array or string. Position of indexing,
                                     reason for the indexed type *)
  | Ridx_vector      of Pos.t (* Used as an index, in the Vector case *)
  | Rappend          of Pos.t (* Used to append element to an array *)
  | Rfield           of Pos.t (* Array accessed with a static string index *)
  | Rforeach         of Pos.t (* Because it is iterated in a foreach loop *)
  | Rasyncforeach    of Pos.t (* Because it is iterated "await as" in foreach *)
  | Raccess          of Pos.t
  | Rarith           of Pos.t
  | Rarith_ret       of Pos.t
  | Rarray_plus_ret  of Pos.t
  | Rstring2         of Pos.t
  | Rcomp            of Pos.t
  | Rconcat          of Pos.t
  | Rconcat_ret      of Pos.t
  | Rlogic           of Pos.t
  | Rlogic_ret       of Pos.t
  | Rbitwise         of Pos.t
  | Rbitwise_ret     of Pos.t
  | Rstmt            of Pos.t
  | Rno_return       of Pos.t
  | Rno_return_async of Pos.t
  | Rret_fun_kind    of Pos.t * Ast.fun_kind
  | Rhint            of Pos.t
  | Rnull_check      of Pos.t
  | Rnot_in_cstr     of Pos.t
  | Rthrow           of Pos.t
  | Rplaceholder     of Pos.t
  | Rattr            of Pos.t
  | Rxhp             of Pos.t
  | Rret_div         of Pos.t
  | Ryield_gen       of Pos.t
  | Ryield_asyncgen  of Pos.t
  | Ryield_asyncnull of Pos.t
  | Ryield_send      of Pos.t
  | Rlost_info       of string * t * Pos.t
  | Rcoerced         of Pos.t * Pos.t * string
  | Rformat          of Pos.t * string * t
  | Rclass_class     of Pos.t * string
  | Runknown_class   of Pos.t
  | Rdynamic_yield   of Pos.t * Pos.t * string * string
  | Rmap_append      of Pos.t
  | Rvar_param       of Pos.t
  | Runpack_param    of Pos.t
  | Rinout_param     of Pos.t
  | Rinstantiate     of t * string * t
  | Rarray_filter    of Pos.t * t
  | Rtype_access     of t * string list * t
  | Rexpr_dep_type   of t * Pos.t * expr_dep_type_reason
  | Rnullsafe_op     of Pos.t (* ?-> operator is used *)
  | Rtconst_no_cstr  of Nast.sid
  | Rused_as_map     of Pos.t
  | Rused_as_shape   of Pos.t
  | Rpredicated      of Pos.t * string
  | Rinstanceof      of Pos.t * string
  | Ris              of Pos.t
  | Ras              of Pos.t
  | Rfinal_property  of Pos.t
  | Rvarray_or_darray_key of Pos.t
  | Rusing           of Pos.t
  | Rdynamic_prop    of Pos.t
  | Rdynamic_call    of Pos.t
  | Ridx_dict        of Pos.t
  | Rmissing_optional_field of Pos.t * string
  | Rcontravariant_generic of t * string
  | Rinvariant_generic of t * string
  | Rregex           of Pos.t
  | Rlambda_use      of Pos.t
  | Rimplicit_upper_bound of Pos.t

and expr_dep_type_reason =
  | ERexpr of int
  | ERstatic
  | ERclass of string
  | ERparent of string
  | ERself of string

(* Translate a reason to a (pos, string) list, suitable for error_l. This
 * previously returned a string, however the need to return multiple lines with
 * multiple locations meant that it needed to more than convert to a string *)
let rec to_string prefix r =
  let p = to_pos r in
  match r with
  | Rnone              -> [(p, prefix)]
  | Rwitness         _ -> [(p, prefix)]
  | Ridx (_, r2)       ->
      [(p, prefix)] @
      [(if r2 = Rnone then p else to_pos r2),
        "This can only be indexed with integers"]
  | Ridx_vector      _ -> [(p, prefix ^
    " because only int can be used to index into a Vector or vec.")]
  | Rappend          _ -> [(p, prefix ^ " because a value is appended to it")]
  | Rfield           _ -> [(p, prefix ^ " because one of its field is accessed")]
  | Rforeach         _ -> [(p, prefix ^ " because this is used in a foreach statement")]
  | Rasyncforeach    _ -> [(p, prefix ^ " because this is used in a foreach statement with \"await as\"")]
  | Raccess          _ -> [(p, prefix ^ " because one of its elements is accessed")]
  | Rarith           _ -> [(p, prefix ^ " because this is used in an arithmetic operation")]
  | Rarith_ret       _ -> [(p, prefix ^ " because this is the result of an arithmetic operation")]
  | Rarray_plus_ret  _ -> [(p, prefix ^ " because this is the result of adding arrays")]
  | Rstring2         _ -> [(p, prefix ^ " because this is used in a string")]
  | Rcomp            _ -> [(p, prefix ^ " because this is the result of a comparison")]
  | Rconcat          _ -> [(p, prefix ^ " because this is used in a string concatenation")]
  | Rconcat_ret      _ -> [(p, prefix ^ " because this is the result of a concatenation")]
  | Rlogic           _ -> [(p, prefix ^ " because this is used in a logical operation")]
  | Rlogic_ret       _ -> [(p, prefix ^ " because this is the result of a logical operation")]
  | Rbitwise         _ -> [(p, prefix ^ " because this is used in a bitwise operation")]
  | Rbitwise_ret     _ -> [(p, prefix ^ " because this is the result of a bitwise operation")]
  | Rstmt            _ -> [(p, prefix ^ " because this is a statement")]
  | Rno_return       _ -> [(p, prefix ^ " because this function implicitly returns void")]
  | Rno_return_async _ -> [(p, prefix ^ " because this async function implicitly returns Awaitable<void>")]
  | Rret_fun_kind    (_, kind) ->
    [(p, match kind with
      | Ast.FAsyncGenerator -> prefix ^ " (result of 'async function' containing a 'yield')"
      | Ast.FGenerator -> prefix ^ " (result of function containing a 'yield')"
      | Ast.FAsync -> prefix ^ " (result of 'async function')"
      | Ast.FCoroutine | Ast.FSync -> prefix)]
  | Rhint            _ -> [(p, prefix)]
  | Rnull_check      _ -> [(p, prefix ^ " because this was checked to see if the value was null")]
  | Rnot_in_cstr     _ -> [(p, prefix ^ " because it is not always defined in __construct")]
  | Rthrow           _ -> [(p, prefix ^ " because it is used as an exception")]
  | Rplaceholder     _ -> [(p, prefix ^ " ($_ is a placeholder variable not meant to be used)")]
  | Rattr            _ -> [(p, prefix ^ " because it is used in an attribute")]
  | Rxhp             _ -> [(p, prefix ^ " because it is used as an XML element")]
  | Rret_div         _ -> [(p, prefix ^ " because it is the result of a division (/)")]
  | Ryield_gen       _ -> [(p, prefix ^ " (result of function with 'yield' in the body)")]
  | Ryield_asyncgen  _ -> [(p, prefix ^ " (result of 'async function' with 'yield' in the body)")]
  | Ryield_asyncnull _ -> [(p, prefix ^ " because \"yield x\" is equivalent to \"yield null => x\" in an async function")]
  | Ryield_send      _ -> [(p, prefix ^ " ($generator->send() can always send a null back to a \"yield\")")]
  | Rvar_param       _ -> [(p, prefix ^ " (variadic argument)")]
  | Runpack_param    _ -> [(p, prefix ^ " (it is unpacked with '...')")]
  | Rinout_param     _ -> [(p, prefix ^ " (inout parameter)")]
  | Rnullsafe_op     _ -> [(p, prefix ^ " (use of ?-> operator)")]
  | Rcoerced     (_, p2, s)  ->
      [
        (p, prefix);
        (p2, "It was implicitly typed as "^s^" during this operation")
      ]
  | Rlost_info (s, r1, p2) ->
      let s = Utils.strip_ns s in
      (to_string prefix r1) @
      [
        (p2, "All the local information about "^s^" has been invalidated \
              during this call.\nThis is a limitation of the type-checker, \
              use a local if that's the problem.")
      ]
  | Rformat       (_,s,t) ->
      let s = prefix ^ " because of the "^s^" format specifier" in
        (match to_string "" t with
          | [(_, "")] -> [(p, s)]
          | el -> [(p, s)] @ el)
  | Rclass_class (_, s) ->
    [(p, prefix^"; implicitly defined constant ::class is a string that contains the \
          fully qualified name of "^(Utils.strip_ns s))]
  | Runknown_class _ -> [(p, prefix^"; this class name is unknown to Hack")]
  | Rdynamic_yield (_, yield_pos, implicit_name, yield_name) ->
      [(p, prefix ^ (Printf.sprintf
        "\n%s\nDynamicYield implicitly defines %s() from the definition of %s()"
        (Pos.string (Pos.to_absolute yield_pos))
        implicit_name
        yield_name))]
  | Rmap_append _ ->
      [(p, prefix^" because you can only append a Pair<Tkey, Tvalue> to an \
      Map<Tkey, Tvalue>")]
  | Rinstantiate (r_orig, generic_name, r_inst) ->
      (to_string prefix r_orig) @
        (to_string ("  via this generic " ^ generic_name) r_inst)
  | Rarray_filter (_, r) ->
      (to_string prefix r) @
      [(p, "array_filter converts KeyedContainer<Tk, Tv> to \
      array<Tk, Tv>, and Container<Tv> to array<arraykey, Tv>. \
      Single argument calls additionally remove nullability from Tv.")]
  | Rtype_access (r_orig, expansions, r_expanded) ->
      let expand_prefix =
        if List.length expansions = 1 then
          "  resulting from expanding the type constant "
        else
          "  resulting from expanding a type constant as follows:\n    " in
      (to_string prefix r_orig) @
      (to_string (expand_prefix^String.concat " -> " expansions) r_expanded)
  | Rexpr_dep_type (r, p, e) ->
      (to_string prefix r) @ [p, "  "^expr_dep_type_reason_string e]
  | Rtconst_no_cstr (_, n) ->
      [(p, prefix ^ " because the type constant "^n^" has no constraints")]
  | Rused_as_map _ -> [(p, prefix ^ " because it is used as map here")]
  | Rused_as_shape _ ->
      [(p, prefix ^ " because it is used as shape-like array here")]
  | Rpredicated (p, f) ->
      [(p, prefix ^ " from the argument to this "^ f ^" test")]
  | Rinstanceof (p,s) ->
      [(p, prefix ^ " from this instanceof test matching " ^ s)]
  | Ris p ->
    [(p, prefix ^ " from this is expression test")]
  | Ras p ->
    [(p, prefix ^ " from this \"as\" assertion")]
  | Rfinal_property _ ->
      [(p, prefix ^ " because properties cannot be declared final")]
  | Rvarray_or_darray_key _ ->
      [(
        p,
        "This is varray_or_darray, which requires arraykey-typed keys when \
        used with an array (used like a hashtable)"
      )]
  | Rusing p ->
    [(p, prefix ^ " because it was assigned in a 'using' clause")]
  | Rdynamic_prop p ->
    [(p, prefix ^ ", the result of accessing a property of a dynamic type")]
  | Rdynamic_call p ->
    [(p, prefix ^ ", the result of calling a dynamic type as a function")]
  | Ridx_dict _ -> [(p, prefix ^
    " because only array keys can be used to index into a Map, dict, darray, Set, or keyset")]
  | Rmissing_optional_field (p, name) ->
    [(p, prefix ^ " because the field '" ^ name ^ "' may be set to any type in this shape")]
  | Rcontravariant_generic (r_orig, class_name) ->
    (to_string prefix r_orig) @
    [(p, "Considering that this type argument is contravariant with respect to " ^ class_name)]
  | Rinvariant_generic (r_orig, class_name) ->
    (to_string prefix r_orig) @
    [(p, "Considering that this type argument is invariant with respect to " ^ class_name)]
  | Rregex _ ->
    [(p, prefix ^ " resulting from this regex pattern")]
  | Rlambda_use p ->
      [(p, prefix ^ " because the lambda function was used here")]
  | Rimplicit_upper_bound _ ->
    [(p, prefix ^ " arising from an implicit 'as ?nonnull' constraint on this type")]

and to_pos = function
  | Rnone     -> Pos.none
  | Rwitness   p -> p
  | Ridx (p, _) -> p
  | Ridx_vector p -> p
  | Rappend   p -> p
  | Rfield   p -> p
  | Rforeach     p -> p
  | Rasyncforeach p -> p
  | Raccess   p -> p
  | Rarith       p -> p
  | Rarith_ret   p -> p
  | Rarray_plus_ret p -> p
  | Rstring2     p -> p
  | Rcomp        p -> p
  | Rconcat      p -> p
  | Rconcat_ret  p -> p
  | Rlogic       p -> p
  | Rlogic_ret   p -> p
  | Rbitwise     p -> p
  | Rbitwise_ret p -> p
  | Rstmt        p -> p
  | Rno_return   p -> p
  | Rno_return_async p -> p
  | Rret_fun_kind (p, _) -> p
  | Rhint        p -> p
  | Rnull_check  p -> p
  | Rnot_in_cstr p -> p
  | Rthrow       p -> p
  | Rplaceholder p -> p
  | Rattr        p -> p
  | Rxhp         p -> p
  | Rret_div     p -> p
  | Ryield_gen   p -> p
  | Ryield_asyncgen p -> p
  | Ryield_asyncnull p -> p
  | Ryield_send  p -> p
  | Rcoerced    (p, _, _) -> p
  | Rlost_info (_, r, _) -> to_pos r
  | Rformat      (p, _, _) -> p
  | Rclass_class (p, _) -> p
  | Runknown_class p -> p
  | Rdynamic_yield (p, _, _, _) -> p
  | Rmap_append p -> p
  | Rvar_param p -> p
  | Runpack_param p -> p
  | Rinout_param p -> p
  | Rinstantiate (_, _, r) -> to_pos r
  | Rarray_filter (p, _) -> p
  | Rtype_access (r, _, _) -> to_pos r
  | Rexpr_dep_type (r, _, _) -> to_pos r
  | Rnullsafe_op p -> p
  | Rtconst_no_cstr (p, _) -> p
  | Rused_as_map p -> p
  | Rused_as_shape p -> p
  | Rpredicated (p, _) -> p
  | Rinstanceof (p, _) -> p
  | Ris p -> p
  | Ras p -> p
  | Rvarray_or_darray_key p
  | Rfinal_property p -> p
  | Rusing p -> p
  | Rdynamic_prop p -> p
  | Rdynamic_call p -> p
  | Ridx_dict p -> p
  | Rmissing_optional_field (p, _) -> p
  | Rcontravariant_generic (r, _) -> to_pos r
  | Rinvariant_generic (r, _) -> to_pos r
  | Rregex p -> p
  | Rlambda_use p -> p
  | Rimplicit_upper_bound p -> p

(* This is a mapping from internal expression ids to a standardized int.
 * Used for outputting cleaner error messages to users
 *)
and expr_display_id_map = ref IMap.empty
and get_expr_display_id id =
  let map = !expr_display_id_map in
  match IMap.get id map with
  | Some n -> n
  | None ->
      let n = (IMap.cardinal map) + 1 in
      expr_display_id_map := IMap.add id n map;
      n

and expr_dep_type_reason_string = function
  | ERexpr id ->
      let did = get_expr_display_id id in
      "where '<expr#"^string_of_int did^">' is a reference to this expression"
  | ERstatic ->
      "where '<static>' refers to the late bound type of the enclosing class"
  | ERclass c ->
      "where the class '"^(strip_ns c)^"' was referenced here"
  | ERparent p ->
      "where the class '"^(strip_ns p)^"' (the parent of the enclosing) \
       class was referenced here"
  | ERself c ->
      "where the class '"^(strip_ns c)^"' was referenced here via the keyword \
       'self'"

let to_constructor_string r =
match r with
  | Rnone -> "Rnone"
  | Rwitness _ -> "Rwitness"
  | Ridx _ -> "Ridx"
  | Ridx_vector _ -> "Ridx_vector"
  | Rappend _ -> "Rappend"
  | Rfield _ -> "Rfield"
  | Rforeach _ -> "Rforeach"
  | Rasyncforeach _ -> "Rasyncforeach"
  | Raccess _ -> "Raccess"
  | Rarith _ -> "Rarith"
  | Rarith_ret _ -> "Rarith_ret"
  | Rarray_plus_ret _ -> "Rarray_plus_ret"
  | Rstring2 _ -> "Rstring2"
  | Rcomp _ -> "Rcomp"
  | Rconcat _ -> "Rconcat"
  | Rconcat_ret _ -> "Rconcat_ret"
  | Rlogic _ -> "Rlogic"
  | Rlogic_ret _ -> "Rlogic_ret"
  | Rbitwise _ -> "Rbitwise"
  | Rbitwise_ret _ -> "Rbitwise_ret"
  | Rstmt _ -> "Rstmt"
  | Rno_return _ -> "Rno_return"
  | Rno_return_async _ -> "Rno_return_async"
  | Rret_fun_kind _ -> "Rret_fun_kind"
  | Rhint _ -> "Rhint"
  | Rnull_check _ -> "Rnull_check"
  | Rnot_in_cstr _ -> "Rnot_in_cstr"
  | Rthrow _ -> "Rthrow"
  | Rplaceholder _ -> "Rplaceholder"
  | Rattr _ -> "Rattr"
  | Rxhp _ -> "Rxhp"
  | Rret_div _ -> "Rret_div"
  | Ryield_gen _ -> "Ryield_gen"
  | Ryield_asyncgen _ -> "Ryield_asyncgen"
  | Ryield_asyncnull _ -> "Ryield_asyncnull"
  | Ryield_send _ -> "Ryield_send"
  | Rlost_info _ -> "Rlost_info"
  | Rcoerced _ -> "Rcoerced"
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
  | Rtype_access _ -> "Rtype_access"
  | Rexpr_dep_type _ -> "Rexpr_dep_type"
  | Rnullsafe_op _ -> "Rnullsafe_op"
  | Rtconst_no_cstr _ -> "Rtconst_no_cstr"
  | Rused_as_map _ -> "Rused_as_map"
  | Rused_as_shape _ -> "Rused_as_shape"
  | Rpredicated _ -> "Rpredicated"
  | Rinstanceof _ -> "Rinstanceof"
  | Ris _ -> "Ris"
  | Ras _ -> "Ras"
  | Rfinal_property _ -> "Rfinal_property"
  | Rvarray_or_darray_key _ -> "Rvarray_or_darray_key"
  | Rusing _ -> "Rusing"
  | Rdynamic_prop _ -> "Rdynamic_prop"
  | Rdynamic_call _ -> "Rdynamic_call"
  | Ridx_dict _ -> "Ridx_dict"
  | Rmissing_optional_field _ -> "Rmissing_optional_field"
  | Rcontravariant_generic _ -> "Rcontravariant_generic"
  | Rinvariant_generic _ -> "Rinvariant_generic"
  | Rregex _ -> "Rregex"
  | Rlambda_use _ -> "Rlambda_use"
  | Rimplicit_upper_bound _ -> "Rimplicit_upper_bound"

let pp fmt r =
  Format.pp_print_string fmt @@ to_constructor_string r

type ureason =
  | URnone
  | URassign
  | URassign_branch
  | URassign_inout
  | URhint
  | URreturn
  | URforeach
  | URthrow
  | URvector
  | URkey
  | URvalue
  | URif
  | URawait
  | URyield
  | URyield_from
  (* Name of XHP class, Name of XHP attribute *)
  | URxhp of string * string
  | URxhp_spread
  | URindex of string
  | URparam
  | URparam_inout
  | URarray_value
  | URarray_key
  | URtuple_access
  | URpair_access
  | URdynamic_yield
  | URnewtype_cstr
  | URclass_req
  | URenum
  | URenum_cstr
  | URtypeconst_cstr
  | URsubsume_tconst_cstr
  | URsubsume_tconst_assign
  | URfinal_property
  | URclone
  | URusing

let index_array = URindex "array"
let index_tuple = URindex "tuple"
let index_class s =  URindex (strip_ns s)

let string_of_ureason = function
  | URnone -> "Typing error"
  | URreturn -> "Invalid return type"
  | URhint -> "Wrong type hint"
  | URassign -> "Invalid assignment"
  | URassign_branch -> "Invalid assignment in this branch"
  | URassign_inout -> "Invalid assignment to an inout parameter"
  | URforeach -> "Invalid foreach"
  | URthrow -> "Invalid exception"
  | URvector -> "Some elements in this Vector are incompatible"
  | URkey -> "The keys of this Map are incompatible"
  | URvalue -> "The values of this Map are incompatible"
  | URif -> "The two branches of ? must have the same type"
  | URawait -> "await can only operate on an Awaitable"
  | URyield -> "Invalid yield"
  | URyield_from -> "Invalid yield from"
  | URxhp (cls, attr) ->
      "Invalid xhp value for attribute " ^ attr ^ " in " ^ (strip_ns cls)
  | URxhp_spread -> "The attribute spread operator cannot be called on non-XHP"
  | URindex s -> "Invalid index type for this " ^ s
  | URparam -> "Invalid argument"
  | URparam_inout -> "Invalid argument to an inout parameter"
  | URarray_value -> "Incompatible field values"
  | URarray_key -> "Incompatible array keys"
  | URtuple_access ->
          "Tuple elements can only be accessed with an integer literal"
  | URpair_access ->
          "Pair elements can only be accessed with an integer literal"
  | URdynamic_yield ->
      "When using DynamicYield, yield*() methods should have type Awaitable"
  | URnewtype_cstr ->
      "Invalid constraint on newtype"
  | URclass_req -> "Unable to satisfy trait/interface requirement"
  | URenum ->
      "Constant does not match the type of the enum it is in"
  | URenum_cstr ->
      "Invalid constraint on enum"
  | URtypeconst_cstr ->
     "Unable to satisfy constraint on this type constant"
  | URsubsume_tconst_cstr ->
     "The constraint on this type constant is inconsistent with its parent"
  | URsubsume_tconst_assign ->
     "The assigned type of this type constant is inconsistent with its parent"
  | URfinal_property ->
      "Property cannot be declared final"
  | URclone -> "Clone cannot be called on non-object"
  | URusing -> "Using cannot be used on non-disposable expression"

let compare r1 r2 =
  let get_pri = function
    | Rnone -> 0
    | Rforeach _ -> 1
    | Rwitness _ -> 3
    | Rused_as_shape _ | Rappend _ | Rused_as_map _ -> 4
    | Rlost_info _ -> 5
    | _ ->  2 in
  let d = (get_pri r2) - (get_pri r1) in
  if d <> 0 then d else compare (to_pos r1) (to_pos r2)
let none = Rnone

(*****************************************************************************)
(* When the subtyping fails because of a constraint. *)
(*****************************************************************************)

let explain_generic_constraint p_inst reason name error =
  let pos = to_pos reason in
  Errors.explain_constraint ~use_pos:p_inst ~definition_pos:pos ~param_name:name error
