(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel

module SU = Hhbc_string_utils

(* We introduce a type for Hack/PHP values, mimicking what happens at runtime.
 * Currently this is used for constant folding. By defining a special type, we
 * ensure independence from usage: for example, it can be used for optimization
 * on ASTs, or on bytecode, or (in future) on a compiler intermediate language.
 * HHVM takes a similar approach: see runtime/base/typed-value.h
 *)
type t =
  (* Used for fields that are initialized in the 86pinit method *)
  | Uninit
  (* Hack/PHP integers are 64-bit *)
  | Int of Int64.t
  | Bool of bool
  (* Both Hack/PHP and Caml floats are IEEE754 64-bit *)
  | Float of float
  | String of string
  | Null
  (* Classic PHP arrays with explicit (key,value) entries *)
  | HhasAdata of string
  | Array of (t*t) list
  | VArray of t list
  | DArray of (t*t) list
  (* Hack arrays: vectors, keysets, and dictionaries *)
  | Vec of t list
  | Keyset of t list
  | Dict of (t*t) list

module TVMap : MyMap.S with type key = t = MyMap.Make (struct
  type key = t
  type t = key
  let compare = Pervasives.compare
end)

(* Some useful constants *)
let zero = Int Int64.zero
let null = Null

module StringOps = struct
  let make_with op s1 s2 =
    if String.length s1 = 0 || String.length s2 = 0 then ""
    else begin
      let len = min (String.length s1) (String.length s2) in
      let result = Bytes.create len in
      for i = 0 to len - 1 do
        let i1 = int_of_char (String.get s1 i) in
        let i2 = int_of_char (String.get s2 i) in
        Bytes.set result i (char_of_int (op i1 i2))
      done;
      Bytes.to_string result
    end

  let bitwise_and = make_with (land)
  let bitwise_or = make_with (lor)
  let bitwise_xor = make_with (lxor)

  let bitwise_not s =
    let result = Bytes.create (String.length s) in
    Caml.String.iteri (fun i c ->
      (* keep only last byte *)
      let b = lnot (int_of_char c) land 0xFF in
      Bytes.set result i (char_of_int b);
    ) s;
    Bytes.to_string result
end


(* Cast to a boolean: the (bool) operator in PHP *)
let to_bool v =
  match v with
  | Uninit -> false (* Should not happen *)
  | Bool b -> b
  | Null -> false
  | String "" -> false
  | String "0" -> false
  | String _ -> true
  | Int i -> i <> Int64.zero
  | Float f -> f <> 0.0
  (* Empty collections cast to false *)
  | Dict [] | Array [] | VArray [] | DArray [] | Keyset [] | Vec [] -> false
  (* Non-empty collections cast to true *)
  | HhasAdata _
  | Dict _ | Array _ | VArray _ | DArray _ | Keyset _ | Vec _-> true

(* try to convert numeric
 * or if allow_following passed then leading numeric string to a number *)
let string_to_int_opt ~allow_following ~allow_inf s =
  (* Interger conversion in scanf is slightly strange,
   * both 3 and 3ab are converted to 3, so we take a separate approach
   * when we do not want the following characters
   *)
  let int_opt = if allow_following then
    try Scanf.sscanf s "%Ld%s" (fun x _ -> Some x) with _ -> None
    else try Some (Int64.of_string s) with _ -> None
  in
  match int_opt with
  | None ->
    begin if allow_following then
      try Scanf.sscanf s "%f%s"
        (fun x _ -> Some (Int64.of_float x)) with _ -> None
      else
        try
          let s = float_of_string s in
          if not allow_inf && (s = Float.infinity || s = Float.neg_infinity)
          then None else Some (Int64.of_float s)
        with _ -> None
    end
  | x -> x

let php7_int_semantics () =
  Hhbc_options.php7_int_semantics !Hhbc_options.compiler_options

(* Cast to an integer: the (int) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_int v =
  match v with
  | Uninit -> None (* Should not happen *)
  | String s ->
    begin
    (*https://github.com/php/php-langspec/blob/master/spec/08-conversions.md
      If the source is a numeric string or leading-numeric string having
      integer format, if the precision can be preserved the result value
      is that string's integer value; otherwise, the result is undefined.
      If the source is a numeric string or leading-numeric string having
      floating-point format, the string's floating-point value is treated as
      described above for a conversion from float. The trailing non-numeric
      characters in leading-numeric strings are ignored. For any other string,
      the result value is 0.
    *)
    match string_to_int_opt ~allow_following:true ~allow_inf:true s with
    | None -> Some Int64.zero
    | x -> x
    end
  | Int i -> Some i
  | Float f ->
    let fpClass = Float.classify f in
    begin match fpClass with
      (* Here's a handy dandy chart of all possible values based on language
       * | float | PHP 5   | HHVM    | PHP 7
       * ----------------------------------------
       * |  NaN  | int_min | int_min | 0
       * |  INF  | int_min |  0      | 0
       * | -INF  | int_min | int_min | 0
       * For NaN, the value is min_int in HHVM
       * For positive infinity, the value is 0 in HHVM
       * For negative infinity the value is min_int in HHVM
       * For PHP7, the value is always 0
       * Thus if the float is infinity OR we're in PHP7, set it to 0
       *)
      | Float.Class.Nan
      | Float.Class.Infinite ->
        if f = Float.infinity || php7_int_semantics () then
        Some Int64.zero else Some Caml.Int64.min_int
      | _ ->
      (* mimic double-to-int64.h *)
      let cast v = try Some (Int64.of_float v) with Failure _ -> None in
      if f >= 0.0 then
      begin
        if f < Int64.to_float Caml.Int64.max_int
        then cast f
        else Some 0L
      end
      else cast f
    end
  | _ ->
    Some (if to_bool v then Int64.one else Int64.zero)

(* Cast to a float: the (float) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_float v =
  match v with
  | Uninit -> None (* Should not happen *)
  | String s ->
    (*If the source is a numeric string or leading-numeric string having
    integer format, the string's integer value is treated as described above
    for a conversion from int. If the source is a numeric string or
    leading-numeric string having floating-point format, the result value
    is the closest approximation to the string's floating-point value.
    The trailing non-numeric characters in leading-numeric strings are ignored.
    For any other string, the result value is 0.*)
    (try Scanf.sscanf s "%f%s" (fun x _ -> Some x)
      with _ -> Some 0.0)
  | Int i ->
    (try Some (Int64.to_float i) with Failure _ -> None)
  | Float f -> Some f
  | _ ->
    Some (if to_bool v then 1.0 else 0.0)

(* Cast to a string: the (string) operator in PHP. Return None if we can't
 * or won't produce the correct value *)
let to_string v =
  match v with
  | Uninit -> None (* Should not happen *)
  | Bool false -> Some ""
  | Bool true -> Some "1"
  | Null -> Some ""
  | Int i -> Some (Int64.to_string i)
  | String s -> Some s
  | Float f -> Some (SU.Float.to_string f)
  | _ -> None

(* Integer operations. For now, we don't attempt to implement the
 * overflow-to-float semantics *)
let add_int i1 i2 =
  Some (Int (Int64.(+) i1 i2))

let neg i =
  match i with
  | Int i -> Some (Int (Int64.neg i))
  | Float f -> Some (Float (0.0 -. f))
  | _ -> None

let sub_int i1 i2 =
  Some (Int (Int64.(-) i1 i2))

(* Arithmetic. For now, only on pure integer or float operands *)
let sub v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 -> sub_int i1 i2
  | Float f1, Float f2 -> Some (Float (f1 -. f2))
  | _ -> None

let mul_int i1 i2 =
  Some (Int (Int64.( * ) i1 i2))

(* Arithmetic. For now, only on pure integer or float operands *)
let mul v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 -> mul_int i1 i2
  | Float f1, Float f2 -> Some (Float (f1 *. f2))
  | Int i1, Float f2 -> Some (Float ((Int64.to_float i1) *. f2))
  | Float f1, Int i2 -> Some (Float (f1 *. (Int64.to_float i2)))
  | _ ->  None

(* Arithmetic. For now, only on pure integer or float operands *)
let div v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 when i2 <> 0L ->
    if Int64.rem i1 i2 = 0L then Some (Int (Int64.(/) i1 i2))
    else Some (Float (Int64.to_float i1 /. Int64.to_float i2))
  | Float f1, Float f2 when f2 <> 0.0 -> Some (Float (f1 /. f2))
  | Int i1, Float f2 when f2 <> 0.0 -> Some (Float ((Int64.to_float i1) /. f2))
  | Float f1, Int i2 when i2 <> 0L -> Some (Float (f1 /. (Int64.to_float i2)))
  | _ ->  None

(* Arithmetic. For now, only on pure integer or float operands *)
let add v1 v2 =
  match v1, v2 with
  | Float f1, Float f2 -> Some (Float (f1 +. f2))
  | Int i1, Int i2 -> add_int i1 i2
  | Int i1, Float f2 -> Some (Float ((Int64.to_float i1) +. f2))
  | Float f1, Int i2 -> Some (Float (f1 +. (Int64.to_float i2)))
  | _, _ -> None

let shift_left v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 when i2 >= 0L && (i2 < 64L || not @@ php7_int_semantics()) ->
    begin try
      let v = Int64.to_int_exn i2 in
      Some (Int (Int64.shift_left i1 v))
    with _ -> None
    end
  | _ -> None

(* Arithmetic. For now, only on pure integer operands *)
let bitwise_or v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 -> Some (Int (Int64.(lor) i1 i2))
  | _ -> None

(* String concatenation *)
let concat v1 v2 =
  match Option.both (to_string v1) (to_string v2) with
  | Some (l, r) -> Some (String (l ^ r))
  | None -> None

(* Bitwise operations. *)
let bitwise_not v =
  match v with
  | Int i -> Some (Int (Int64.lnot i))
  | String s -> Some (String (StringOps.bitwise_not s))
  | _ -> None

(* Logical operators *)
let not v =
  Some (Bool (not (to_bool v)))

(*
  returns (t * t) option option
  None - one of operands was not literal
  Some(None) - both operands were literal but with incompatible types
  Some (Some (l, r)) - both operands were converted successfully
*)
let convert_literals_for_relational_ops v1 v2 =
  let rec convert v1 v2 can_flip =
    match v1, v2 with
    | Null, Null
    | Bool _, Bool _
    | Int _ , Int _
    | Float _, Float _
    | String _, String _ -> Some (Some(v1, v2))
    (*  bool op null *)
    | Bool _, Null -> Some (Some (v1, Bool (to_bool v2)))
    (* int op null *)
    | Int _, Null -> Some (Option.map(to_int v2) (fun v -> v1, Int v))
    (* int op bool *)
    | Int _, Bool _ -> Some (Some (Bool (to_bool v1), v2))
    (* float op null *)
    | Float _, Null -> Some (Option.map(to_float v2) (fun v -> v1, Float v))
    (* float op bool *)
    | Float _, Bool _ -> Some (Some (Bool (to_bool v1), v2))
    (* float op int *)
    | Float _, Int _ -> Some (Option.map(to_float v2)(fun v -> v1, Float v))
    (* string op null *)
    | String _, Null ->
      Some (Option.map (to_string v2) (fun v -> v1, String v))
    (* string op bool*)
    | String _, Bool _ -> Some (Some (Bool (to_bool v1), v2))
    (* string op int *)
    | String _, Int _ -> Some (Option.map(to_int v1) (fun v -> Int v, v2))
    (* string op float*)
    | String _, Float _ -> Some (Option.map(to_float v1) (fun v -> Float v, v2))
    | _ ->
      if can_flip then
        match convert v2 v1 false with
        | Some (Some (r, l)) -> Some (Some (l, r))
        | x -> x
      else None
  in
  convert v1 v2 true

(*https://github.com/php/php-langspec/blob/master/spec/10-expressions.md#grammar-unary-op-expression*)
let eqeqeq v1 v2 =
  (*Operator === represents same type and value equality, or identity,
    comparison, and operator !== represents the opposite of ===.
    The values are considered identical if they have the same type and
    compare as equal*)
  let v =
    match v1, v2 with
    | Null, Null -> Some true
    | Bool v1, Bool v2 -> Some (v1 = v2)
    | Int v1, Int v2 -> Some (v1 = v2)
    | Float v1, Float v2 -> Some (v1 = v2)
    (*Strings are identical if they contain the same characters,
      unlike value comparison operators no conversions
      are performed for numeric strings*)
    | String v1, String v2 -> Some (v1 = v2)
    | _ -> Some false
  in
  Option.map v (fun v -> Bool v)

let compare_strings op_int op_float v1 v2 =
  (* handle numeric strings first
  If one of the operands has arithmetic type, is a resource,
  or a numeric string, which can be represented as int or float without
  loss of precision, the operands are converted to the corresponding
  arithmetic type, with float taking precedence over int*)
  match Option.both (to_int v1) (to_int v2) with
  | Some (l, r) -> Some (Bool (op_int l r))
  | _ ->
  match Option.both (to_float v1) (to_float v2) with
  | Some (l, r) -> Some (Bool (op_float l r))
  | _ -> None

let eqeq v1 v2 =
  let compare v1 v2 =
    match v1, v2 with
    | String _, String _ ->
      begin
        match compare_strings (=) (=) v1 v2 with
        | None -> eqeqeq v1 v2
        | x -> x
      end
    | _ -> eqeqeq v1 v2
  in
  (*For operators ==, !=, and <>, the operands of different types are converted
  and compared according to the same rules as in relational operators*)
  Option.bind (convert_literals_for_relational_ops v1 v2) (function
    | Some (l, r) ->
      compare l r
    (*Two objects of different types are always not equal.*)
    | None ->
      Some (Bool false)
  )

let less_than v1 v2 =
  let compare v1 v2 =
    match v1, v2 with
    | Null, Null -> Some (Bool false)
    | Float l, Float r -> Some (Bool (l < r))
    | Int l, Int r -> Some (Bool (l < r))
    | String l, String r ->
      begin
        match compare_strings (<) (<) v1 v2 with
        | None -> Some( Bool (l < r))
        | x ->x
      end
    | Bool l, Bool r -> Some (Bool (l < r))
    | _ -> None
  in
  Option.bind (convert_literals_for_relational_ops v1 v2) (function
    | Some (l, r) -> compare l r
    | None -> None
  )

let less_than_equals v1 v2 =
  let compare v1 v2 =
    match v1, v2 with
    | Null, Null -> Some (Bool true)
    | Float l, Float r -> Some (Bool (l <= r))
    | Int l, Int r -> Some (Bool (l <= r))
    | String l, String r ->
      begin
        match compare_strings (<=) (<=) v1 v2 with
        | None -> Some (Bool (l <= r))
        | x -> x
      end
    | Bool l, Bool r -> Some (Bool (l <= r))
    | _ -> None
  in
  Option.bind (convert_literals_for_relational_ops v1 v2) (function
    | Some (l, r) -> compare l r
    | None -> None
  )

let cast_to_string v = Option.map (to_string v) (fun x -> String x)

let cast_to_int v = Option.map (to_int v) (fun x -> Int x)

let cast_to_bool v = Some (Bool (to_bool v))

let cast_to_float v = Option.map (to_float v) (fun x -> Float x)

let cast_to_arraykey v =
  match v with
  | String s -> Some (String s)
  | Null -> Some (String "")
  | Uninit | Array _ | VArray _ | DArray _ | Vec _ | Keyset _ | Dict _ -> None
  | _ -> cast_to_int v
