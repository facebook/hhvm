(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)



(* The reason why something is expected to have a certain type *)
type t =
  | Rnone
  | Rwitness    of Pos.t
  | Ridx    of Pos.t (* Used as an index *)
  | Ridx_vector of Pos.t (* Used as an index, in the Vector case *)
  | Rappend    of Pos.t (* Used to append element to an array *)
  | Rfield    of Pos.t (* Array accessed with a static string index *)
  | Rforeach  of Pos.t (* Because it is iterated in a foreach loop *)
  | Raccess   of Pos.t
  | Rcall     of Pos.t
  | Rarith    of Pos.t
  | Rarith_ret of Pos.t
  | Rstring2   of Pos.t
  | Rcomp      of Pos.t
  | Rconcat    of Pos.t
  | Rconcat_ret of Pos.t
  | Rlogic      of Pos.t
  | Rlogic_ret  of Pos.t
  | Rbitwise    of Pos.t
  | Rbitwise_ret of Pos.t
  | Rstmt        of Pos.t
  | Rno_return   of Pos.t
  | Rno_return_async of Pos.t
  | Rhint          of Pos.t
  | Rnull_check    of Pos.t
  | Rnot_in_cstr   of Pos.t
  | Rthrow         of Pos.t
  | Rattr          of Pos.t
  | Rxhp           of Pos.t
  | Rret_div       of Pos.t
  | Rlost_info     of string * t * Pos.t
  | Rcoerced       of Pos.t * Pos.t * string
  | Rformat        of Pos.t * string * t
  | Rclass_class   of Pos.t * string
  | Runknown_class of Pos.t
  | Rdynamic_yield of Pos.t * Pos.t * string * string

let rec to_string = function
  | Rnone              -> ""
  | Rwitness         _ -> ""
  | Ridx             _ -> " because this is used as an index"
  | Ridx_vector      _ -> ". Only int can be used to index into a Vector."
  | Rappend          _ -> " because a value is appended to it"
  | Rfield           _ -> " because one of its field is accessed"
  | Rforeach         _ -> " because this is used in a foreach statement"
  | Raccess          _ -> " because one of its elements is accessed"
  | Rcall            _ -> " because this is used as a function"
  | Rarith           _ -> " because this is used in an arithmetic operation"
  | Rarith_ret       _ -> " because this is the result of an arithmetic operation"
  | Rstring2         _ -> " because this is used in a string"
  | Rcomp            _ -> " because this is the result of a comparison"
  | Rconcat          _ -> " because this is used in a string concatenation"
  | Rconcat_ret      _ -> " because this is the result of a concatenation"
  | Rlogic           _ -> " because this is used in a logical operation"
  | Rlogic_ret       _ -> " because this is the result of a logical operation"
  | Rbitwise         _ -> " because this is used in a bitwise operation"
  | Rbitwise_ret     _ -> " because this is the result of a bitwise operation"
  | Rstmt            _ -> " because this is a statement"
  | Rno_return       _ -> " because this function implicitly returns void"
  | Rno_return_async _ -> " because this async function implicitly returns Awaitable<void>"
  | Rhint            _ -> ""
  | Rnull_check      _ -> " because this was checked to see if the value was null"
  | Rnot_in_cstr     _ -> " because it is not always defined in __construct"
  | Rthrow           _ -> " because it is used as an exception"
  | Rattr            _ -> " because it is used in an attribute"
  | Rxhp             _ -> " because it is used as an XML element"
  | Rret_div         _ -> " because it is the result of a division (/)"
  | Rcoerced     (p1, p2, s)  ->
      "\n"^
      Pos.string p2^
      "\nIt was implicitely typed as "^s^" during this operation"
  | Rlost_info (s, r1, p2) ->
      to_string r1^
      "\n"^
      Pos.string p2^
      "\n\
All the local information about "^s^" has been invalidated during this call.\n\
This is a limitation of the type-checker, use a local if that's the problem.
"
  | Rformat       (_,s,t) ->
      let s = " because of the "^s^" format specifier" in
        (match to_string t with
           | "" -> s
           | s2 -> s ^ ", " ^ s2)
  | Rclass_class (p, s) ->
    "; implicitly defined constant ::class is a string that contains the fully qualified name of "^s
  | Runknown_class _ -> "; this class name is unknown to Hack"
  | Rdynamic_yield (p, yield_pos, implicit_name, yield_name) ->
      Printf.sprintf
        "\n%s\nDynamicYield implicitly defines %s() from the definition of %s()"
        (Pos.string yield_pos)
        implicit_name
        yield_name

let rec to_pos = function
  | Rnone     -> Pos.none
  | Rwitness   p -> p
  | Ridx   p -> p
  | Ridx_vector p -> p
  | Rappend   p -> p
  | Rfield   p -> p
  | Rforeach     p -> p
  | Raccess   p -> p
  | Rcall        p -> p
  | Rarith       p -> p
  | Rarith_ret   p -> p
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
  | Rhint        p -> p
  | Rnull_check  p -> p
  | Rnot_in_cstr p -> p
  | Rthrow       p -> p
  | Rattr        p -> p
  | Rxhp         p -> p
  | Rret_div     p -> p
  | Rcoerced    (p, _, _) -> p
  | Rlost_info (_, r, _) -> to_pos r
  | Rformat      (p, _, _) -> p
  | Rclass_class (p, _) -> p
  | Runknown_class p -> p
  | Rdynamic_yield (p, _, _, _) -> p

type ureason =
  | URnone
  | URassign
  | URassign_branch
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
  | URxhp
  | URarray_get
  | URmap_get
  | URvector_get
  | URconst_vector_get
  | URimm_vector_get
  | URcontainer_get
  | URtuple_get
  | URpair_get
  | URparam
  | URarray_value
  | URarray_key
  | URtuple_access
  | URpair_access
  | URdynamic_yield
  | URnewtype_cstr

let string_of_ureason = function
  | URnone -> "Typing error"
  | URreturn -> "Invalid return type"
  | URhint -> "Wrong type hint"
  | URassign -> "Invalid assignment"
  | URassign_branch -> "Invalid assignment in this branch"
  | URforeach -> "Invalid foreach"
  | URthrow -> "Invalid exception"
  | URvector -> "Some elements in this Vector are incompatible"
  | URkey -> "The keys of this Map are incompatible"
  | URvalue -> "The values of this Map are incompatible"
  | URif -> "The two branches of ? must have the same type"
  | URawait -> "await can only operate on an Awaitable"
  | URyield -> "Some values passed to yield are incompatible"
  | URxhp -> "Invalid xhp value"
  | URarray_get -> "Invalid index type for this array"
  | URmap_get -> "Invalid index type for this Map"
  | URvector_get -> "Invalid index type for this Vector"
  | URconst_vector_get -> "Invalid index type for this ConstVector"
  | URimm_vector_get -> "Invalid index type for this ImmVector"
  | URcontainer_get -> "Invalid index type for this container"
  | URtuple_get -> "Invalid index for this tuple"
  | URpair_get -> "Invalid index for this pair"
  | URparam -> "Invalid argument"
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

let compare r1 r2 =
  match r1, r2 with
  | Rnone, Rnone             -> 0
  | Rnone, _                 -> 1
  | _, Rnone                 -> -1
  | Rlost_info _, Rlost_info _ -> 0
  | Rlost_info _, _          -> -1
  | _, Rlost_info _          -> 1
  | Rwitness p1, Rwitness p2 -> compare p1 p2
  | Rwitness _, _            -> -1
  | _, Rwitness _            -> 1
  | Rforeach _, Rforeach _   -> 0
  | Rforeach _, _            -> 1
  | _, Rforeach _            -> -1
  | _                        -> compare (to_pos r1) (to_pos r2)

let none = Rnone

(*****************************************************************************)
(* When the subtyping fails because of a constraint. *)
(*****************************************************************************)

let explain_generic_constraint reason name messagel =
  let pos = to_pos reason in
  Utils.error_l
    (messagel @ [pos, "Considering the constraint on the type '"^name^"'"])
