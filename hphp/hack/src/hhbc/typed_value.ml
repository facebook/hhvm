(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

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
  | Array of (t*t) list
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
let one = Int Int64.one

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
    String.iteri (fun i c ->
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
  | Dict [] | Array [] | Keyset [] | Vec [] -> false
  (* Non-empty collections cast to true *)
  | Dict _ | Array _ | Keyset _ | Vec _-> true

(* try to convert numeric or leading numeric string to a number *)
let string_to_int_opt s =
  match (try Scanf.sscanf s "%Ld%s" (fun x _ -> Some x) with _ -> None) with
  | None ->
    begin
      try Scanf.sscanf s "%f%s" (fun x _ -> Some (Int64.of_float x))
      with _ -> None
    end
  | x -> x

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
    match string_to_int_opt s with
    | None -> Some Int64.zero
    | x -> x
    end
  | Int i -> Some i
  | Float f ->

    let fpClass = classify_float f in
    begin match fpClass with
      (* If the source type is float, for the value of NAN,
       * the result value is min int
       * TODO: This is only true in HHVM and PHP 4-5.
       * PHP 7 no longer follows this. *)
      | FP_nan -> Some Int64.min_int (* -9223372036854775808 *)
      (* If the source type is float, for the values INF and -INF,
      the result value is zero *)
      | FP_infinite -> Some Int64.zero
      | _ ->
      (* TODO: get this right. It's unlikely that Caml and
       * PHP semantics match up *)
      (try Some (Int64.of_float f) with Failure _ -> None)
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

let ints_overflow_to_ints () =
  Hhbc_options.ints_overflow_to_ints !Hhbc_options.compiler_options

(* Integer operations. For now, we don't attempt to implement the
 * overflow-to-float semantics *)
let add_int i1 i2 =
  if ints_overflow_to_ints ()
  then Some (Int (Int64.add i1 i2))
  else None

let sub_int i1 i2 =
  if ints_overflow_to_ints ()
  then Some (Int (Int64.sub i1 i2))
  else None

let mul_int i1 i2 =
  if ints_overflow_to_ints ()
  then Some (Int (Int64.mul i1 i2))
  else None

(* Arithmetic. For now, only on pure integer or float operands *)
let add v1 v2 =
  match v1, v2 with
  | Float f1, Float f2 -> Some (Float (f1 +. f2))
  | Int i1, Int i2 -> add_int i1 i2
  | _, _ -> None

let sub v1 v2 =
  match v1, v2 with
  | Float f1, Float f2 -> Some (Float (f1 -. f2))
  | Int i1, Int i2 -> sub_int i1 i2
  | _, _ -> None

let mul v1 v2 =
  match v1, v2 with
  | Float f1, Float f2 -> Some (Float (f1 *. f2))
  | Int i1, Int i2 -> mul_int i1 i2
  | _, _ -> None

let rem v1 v2 =
  match v1, v2 with
  | Int i1, Int i2 ->
    Some (Int (Int64.rem i1 i2))
  | _, _ ->
    None

let div v1 v2 =
  match v1, v2 with
  | Int left, Int right ->
    if Int64.rem left right = Int64.zero then
      Some (Int (Int64.div left right))
    else
      let left = Int64.to_float left in
      let right = Int64.to_float right in
      let quotient = left /. right in
      Some (Float quotient)
  | Float f1, Float f2 -> Some (Float (f1 /. f2))
  | _, _ -> None

let shift f v1 v2 =
  match Option.both (to_int v1) (to_int v2) with
  | Some (l, r) when r > 0L ->
    Some (Int (f l (Int64.to_int r)))
  | _ -> None

let shift_left v1 v2 = shift Int64.shift_left v1 v2

let shift_right v1 v2 = shift Int64.shift_right v1 v2

(* String concatenation *)
let concat v1 v2 =
  match Option.both (to_string v1) (to_string v2) with
  | Some (l, r) -> Some (String (l ^ r))
  | None -> None

(* Bitwise operations. *)
let bitwise_not v =
  match v with
  | Int i -> Some (Int (Int64.lognot i))
  | String s -> Some (String (StringOps.bitwise_not s))
  | _ -> None

let bitwise_and v1 v2 =
  match v1, v2 with
  | (Int _ | Bool _ | Null), (Int _ | Bool _ | Null) ->
    (match Option.both (to_int v1) (to_int v2) with
    | Some (i1, i2) -> Some (Int (Int64.logand i1 i2))
    | None -> None)
  | String s1, String s2 -> Some (String (StringOps.bitwise_and s1 s2))
  | _ -> None

let bitwise_or v1 v2 =
  match v1, v2 with
  | (Int _ | Bool _ | Null), (Int _ | Bool _ | Null) ->
    (match Option.both (to_int v1) (to_int v2) with
    | Some (i1, i2) -> Some (Int (Int64.logor i1 i2))
    | None -> None)
  | String s1, String s2 -> Some (String (StringOps.bitwise_or s1 s2))
  | _ -> None

let bitwise_xor v1 v2 =
  match v1, v2 with
  | (Int _ | Bool _ | Null), (Int _ | Bool _ | Null) ->
    (match Option.both (to_int v1) (to_int v2) with
    | Some (i1, i2) -> Some (Int (Int64.logxor i1 i2))
    | None -> None)
  | String s1, String s2 -> Some (String (StringOps.bitwise_xor s1 s2))
  | _ -> None

(* Logical operators *)
let not v =
  Some (Bool (not (to_bool v)))

let logical_or v1 v2 =
  Some (Bool (to_bool v1 || to_bool v2))

let logical_and v1 v2 =
  Some (Bool (to_bool v1 && to_bool v2))

let logical_xor v1 v2 =
  Some (Bool (to_bool v1 <> to_bool v2))

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

let diff v1 v2 = Option.bind (eqeq v1 v2) not

let diff2 v1 v2 = Option.bind (eqeqeq v1 v2) not

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

let greater_than v1 v2 = less_than v2 v1

let greater_than_equals v1 v2 = less_than_equals v2 v1

let cast_to_string v = Option.map (to_string v) (fun x -> String x)

let cast_to_int v = Option.map (to_int v) (fun x -> Int x)

let cast_to_bool v = Some (Bool (to_bool v))

let cast_to_float v = Option.map (to_float v) (fun x -> Float x)

let cast_to_arraykey v =
  match v with
  | String s -> Some (String s)
  | Null -> Some (String "")
  | Uninit | Array _ | Vec _ | Keyset _ | Dict _ -> None
  | _ -> cast_to_int v
