(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)
module A = Ast
module TV = Typed_value
module SN = Naming_special_names
module SU = Hhbc_string_utils
module TVL = Unique_list_typed_value
open Ast_class_expr
open Hh_core

exception NotLiteral
exception UserDefinedConstant

let hack_arr_compat_notices () =
  Hhbc_options.hack_arr_compat_notices !Hhbc_options.compiler_options

let hack_arr_dv_arrs () =
  Hhbc_options.hack_arr_dv_arrs !Hhbc_options.compiler_options

let radix (s : string) : [`Oct | `Hex | `Dec | `Bin ] =
  if String.length s > 1 && s.[0] = '0' then
    match s.[1] with
    (* Binary *)
    | 'b' | 'B' -> `Bin
    (* Hex *)
    | 'x' | 'X' -> `Hex
    (* Octal *)
    | _ -> `Oct
  else `Dec

let float_of_string_radix (s : string) (radix : int) : float =
  let float_radix = float_of_int radix in
  let float_of_char (c : char) : float = float_of_int @@ int_of_char c - 48 in
  let rec loop (idx : int) (acc : float) =
    if idx < String.length s then
      loop (idx + 1) ((acc *. float_radix) +. (float_of_char s.[idx]))
    else
      acc in
  loop 0 0.

let float_of_string_custom (s : string) : float =
  match radix s with
    | `Bin -> float_of_string_radix (String_utils.string_after s 2) 2
    | `Hex | `Dec -> float_of_string s
    | `Oct -> float_of_string_radix (String_utils.string_after s 1) 8

(* TODO: once we don't need to be PHP5 compliant, we may want to get rid of
this octal truncation at the first non-octal digit, and instead throw an error
either somewhere around here or in the lexer. *)
let int64_of_octal_opt (s : string) (truncate : bool) : Int64.t option =
  (* If we have a partial result that is strictly larger than this value,
   * then we are in danger of overflowing and should return None *)
  let limit = Int64.div Int64.max_int (Int64.of_int 8) in
  let to_int64 c = Int64.of_int @@ int_of_char c - 48 in
  let is_octal_digit ch = '0' <= ch && ch <= '7' in
  let rec loop (idx : int) (acc : Int64.t) =
    (* Given a new least significant digit [digit] and an orginal value [base]
     * append [digit] to the end of [base]. Example:
     * [(push 0o133 0o7) = 0o1337]
     *)
    let push base digit =
      Int64.add (Int64.mul base (Int64.of_int 8)) digit in
    if idx >= String.length s then
      Some acc
    else if Int64.compare acc limit > 0 then
      (** In this case we would overflow *)
      None
    else if not (is_octal_digit s.[idx]) then
      if truncate then Some acc else None
    else
      loop (idx + 1) (push acc (to_int64 s.[idx])) in
  loop 0 Int64.zero

(* Return None if this overflows *)
let try_type_intlike (s : string) : TV.t option =
  match radix s with
  | `Dec ->
    (* Ocaml source: ints.c: parse_sign_and_base treat
       dec form as signed so overflows are properly detected and reported *)
    Option.map (Int64.of_string_opt s) ~f:(fun v -> TV.Int v)
  | `Bin | `Hex -> begin
    (* Ocaml source: ints.c: parse_sign_and_base interprets hex/bin forms as
       unsigned so if the input exceeds Int64.max_int it is converted
       to the signed integer Int64.min_int + input - Int64.max_int - 1.*)
    match Int64.of_string s with
    | i ->
      let input_is_negative = String.get s 0 = '-' in
      (* treat as overflow if result has changed the sign comparing to input *)
      if input_is_negative = (i < 0L) then Some (TV.Int i) else None
    | exception _ -> None
  end
  | `Oct ->
    Option.map ~f:(fun v -> TV.Int v)
      (int64_of_octal_opt (String_utils.string_after s 1) true)

(* Literal expressions can be converted into values *)
(* Restrict_keys flag forces keys to be only ints or strings *)
let rec expr_to_typed_value
  ?(allow_maps=false)
  ?(restrict_keys=false)
  ns (_, expr_) =
  match expr_ with
  | A.Int s -> begin
    match try_type_intlike s with
    | Some v -> v
    | None ->
      if Hhbc_options.ints_overflow_to_ints !Hhbc_options.compiler_options
      then TV.Int Int64.max_int
      else TV.Float (float_of_string_custom s)
  end
  | A.True -> TV.Bool true
  | A.False -> TV.Bool false
  | A.Null -> TV.null
  | A.String s -> TV.String s
  | A.Float s -> TV.Float (float_of_string s)
  | A.Id (_, id) when id = "NAN" -> TV.Float nan
  | A.Id (_, id) when id = "INF" -> TV.Float infinity
  | A.Call ((_, A.Id (_, "__hhas_adata")), _, [ (_, A.String data) ], [])
    ->
      TV.HhasAdata data
  | A.Array fields -> array_to_typed_value ns fields
  | A.Varray fields -> varray_to_typed_value ns fields
  | A.Darray fields -> darray_to_typed_value ns fields
  | A.Collection ((_, "vec"), fields) ->
    TV.Vec (List.map fields (value_afield_to_typed_value ns))
  | A.Collection ((_, "keyset"), fields) ->
    let l = List.fold_left fields
      ~f:(fun l x ->
          TVL.add l (keyset_value_afield_to_typed_value ns x))
      ~init:TVL.empty in
    TV.Keyset (TVL.items l)
  | A.Collection ((_, kind), fields)
    when kind = "dict" ||
         (allow_maps &&
           (SU.cmp ~case_sensitive:false ~ignore_ns:true kind "Map" ||
            SU.cmp ~case_sensitive:false ~ignore_ns:true kind "ImmMap")) ->
    let values =
      List.map fields ~f:(afield_to_typed_value_pair ~restrict_keys ns)
    in
    let d = update_duplicates_in_map values in
    TV.Dict d
  | A.Collection ((_, kind), fields)
    when allow_maps &&
         (SU.cmp ~case_sensitive:false ~ignore_ns:true kind "Set" ||
          SU.cmp ~case_sensitive:false ~ignore_ns:true kind "ImmSet") ->
    let values =
      List.map fields ~f:(set_afield_to_typed_value_pair ns)
    in
    let d = update_duplicates_in_map values in
    TV.Dict d
  | A.Shape fields ->
    shape_to_typed_value ns fields
  | A.Class_const (cid, id) ->
    class_const_to_typed_value ns cid id
  | A.BracedExpr e ->
    expr_to_typed_value ~allow_maps ~restrict_keys ns e
  | A.Id _ | A.Class_get _ -> raise UserDefinedConstant
  | _ ->
    raise NotLiteral

and update_duplicates_in_map kvs =
  (* map key -> (the latest value for the given key) *)
  let unique_values_map = List.fold_left kvs ~init:TV.TVMap.empty
    ~f:(fun m (k, v) -> TV.TVMap.add k v m)
  in
  let values, _ = List.fold_left kvs ~init:([], unique_values_map)
    ~f:(fun (result, uniq_map) (k, _) ->
      (* map stores the latest value for a key
         if map has an value for a given key, put value in the result list
         and remove if from the map to ignore similar keys later *)
      match TV.TVMap.get k uniq_map with
      | Some v -> (k, v)::result, TV.TVMap.remove k uniq_map
      | None -> result, uniq_map)
  in
  List.rev values

and class_const_to_typed_value ns cid id =
  if snd id = SN.Members.mClass
  then
    let cexpr = expr_to_class_expr ~resolve_self:true [] cid in
    begin match cexpr with
    | Class_id cid ->
      let fq_id, _ = Hhbc_id.Class.elaborate_id ns cid in
      TV.String (Hhbc_id.Class.to_raw_string fq_id)
    | _ -> raise UserDefinedConstant
    end
  else raise UserDefinedConstant

and array_to_typed_value ns fields =
  let update_max_index newindex maxindex =
    if Int64.compare newindex maxindex >= 0 then
      Int64.add newindex Int64.one else maxindex in
  let default key value pairs maxindex =
    let k_tv = key_expr_to_typed_value ns key in
    let maxindex = match k_tv with
      | TV.Int newindex ->
        update_max_index newindex maxindex
      | _ -> maxindex in
    (k_tv, expr_to_typed_value ns value) :: pairs, maxindex
  in
  let pairs, _ =
    List.fold_left fields ~init:([], Int64.zero)
      ~f:(fun (pairs, maxindex) afield ->
        match afield with
          (* Special treatment for explicit integer key, or string that
           * parses successfully as integer *)
        | A.AFkvalue (((_, (A.Int s | A.String s)) as key), value) ->
          begin match Int64.of_string s with
          | newindex when SU.Integer.is_decimal_int s ->
            begin match key with
            (* do not fold int-like strings when hack_arr_compat_notices is set.
               Arrays with such indices should not be placed in scalar table map
               since it will not trigger runtime notices on key type mismatch *)
            | _, A.String _ when hack_arr_compat_notices () -> raise NotLiteral
            | _ -> ()
            end;
            (TV.Int newindex, expr_to_typed_value ns value) :: pairs,
              update_max_index newindex maxindex
          | _ ->
            default key value pairs maxindex
          | exception Failure _ ->
            default key value pairs maxindex
          end
        | A.AFkvalue (key, value) ->
          default key value pairs maxindex
        | A.AFvalue value ->
          (TV.Int maxindex, expr_to_typed_value ns value) :: pairs,
            Int64.add maxindex Int64.one)
  in
  let a = update_duplicates_in_map @@ List.rev pairs in
  TV.Array a

and varray_to_typed_value ns fields =
  let tv_fields = (List.map fields ~f:(expr_to_typed_value ns)) in
  TV.VArray tv_fields

and darray_to_typed_value ns fields =
  let fields =
    List.map
      fields
      ~f:(fun (v1,v2) ->
        match snd v1 with
        | A.String s ->
           begin match Int64.of_string s with
           | index when (SU.Integer.is_decimal_int s) && not (hack_arr_dv_arrs ()) ->
              (TV.Int index, expr_to_typed_value ns v2)
           | _ ->
              (expr_to_typed_value ns v1, expr_to_typed_value ns v2)
           | exception Failure _ ->
              (key_expr_to_typed_value ns v1, expr_to_typed_value ns v2)
           end
        | _ -> (key_expr_to_typed_value ns v1, expr_to_typed_value ns v2))
  in
  let a = update_duplicates_in_map fields in
  TV.DArray a

and shape_to_typed_value ns fields =
  let a = List.map fields (fun (sf, expr) ->
    let key =
      match sf with
      | A.SFlit_int (pos, str) ->
        begin match expr_to_typed_value ns (pos, A.Int str) with
        | TV.Int _ as tv -> tv
        | _ -> failwith (str ^ " is not a valid integer index")
        end
      | A.SFlit_str id ->
        TV.String (snd id)
      | A.SFclass_const (class_id, id) ->
        class_const_to_typed_value ns (Pos.none, A.Id class_id) id in
    (key, expr_to_typed_value ns expr))
  in
  TV.DArray a

and key_expr_to_typed_value ?(restrict_keys=false) ns expr =
  let tv = expr_to_typed_value ns expr in
  begin match tv with
  | TV.Int _ | TV.String _ when restrict_keys || hack_arr_compat_notices () -> ()
  | _ when restrict_keys -> raise NotLiteral
  | TV.Bool _ when hack_arr_compat_notices () -> raise NotLiteral
  | _ when hack_arr_compat_notices () -> raise NotLiteral
  | _ -> () end;
  match TV.cast_to_arraykey tv with
  | Some tv -> tv
  | None -> raise NotLiteral

and afield_to_typed_value_pair ?(restrict_keys=false) ns afield =
  match afield with
  | A.AFvalue (_value) ->
    failwith "afield_to_typed_value_pair: unexpected value"
  | A.AFkvalue (key, value) ->
    (key_expr_to_typed_value ~restrict_keys ns key,
     expr_to_typed_value ns value)

and value_afield_to_typed_value ns afield =
  match afield with
  | A.AFvalue e -> expr_to_typed_value ns e
  | A.AFkvalue (_key, _value) ->
    failwith "value_afield_to_typed_value: unexpected key=>value"

and keyset_value_afield_to_typed_value ns afield =
  let tv = value_afield_to_typed_value ns afield in
  begin match tv with
  | TV.Int _ | TV.String _ -> ()
  | _ -> raise NotLiteral end;
  tv

and set_afield_to_typed_value_pair ns afield =
  match afield with
  | A.AFvalue (value) ->
     let tv = key_expr_to_typed_value ~restrict_keys:true ns value
     in (tv, tv)
  | A.AFkvalue (_, _) ->
     failwith "set_afield_to_typed_value_pair: unexpected key=>value"

let expr_to_opt_typed_value ?(restrict_keys=false) ?(allow_maps=false) ns e =
  match expr_to_typed_value ~restrict_keys ~allow_maps ns e with
  | x -> Some x
  | exception (NotLiteral | UserDefinedConstant) -> None

(* Any value can be converted into a literal expression *)
let rec value_to_expr_ p v =
  match v with
  | TV.Uninit -> failwith "value_to_expr: uninit value"
  | TV.Null -> A.Null
  | TV.Int i -> A.Int (Int64.to_string i)
  | TV.Bool false -> A.False
  | TV.Bool true -> A.True
  | TV.String s -> A.String s
  | TV.Float f -> A.Float (SU.Float.to_string f)
  | TV.Vec _ -> failwith "value_to_expr: vec NYI"
  | TV.Keyset _ -> failwith "value_to_expr: keyset NYI"
  | TV.HhasAdata _ -> failwith "value_to_expr: HhasAdata NYI"
  | TV.Array pairs -> A.Array (List.map pairs (value_pair_to_afield p))
  | TV.VArray values -> A.Varray (List.map values (value_to_expr p))
  | TV.DArray pairs ->
     A.Darray (List.map pairs (fun (v1,v2) -> (value_to_expr p v1, value_to_expr p v2)))
  | TV.Dict _ -> failwith "value_to_expr: dict NYI"
and value_to_expr p v =
  (p, value_to_expr_ p v)
and value_pair_to_afield p (v1, v2) =
  A.AFkvalue (value_to_expr p v1, value_to_expr p v2)

(* Apply a unary operation on a typed value v.
 * Return None if we can't or won't determine the result *)
let unop_on_value unop v =
  match unop with
  | A.Unot -> TV.not v
  | A.Uplus -> TV.add TV.zero v
  | A.Uminus -> TV.neg v
  | A.Utild -> TV.bitwise_not v
  | A.Usilence -> Some v
  | _ -> None

(* Likewise for binary operations *)
let binop_on_values binop v1 v2 =
  match binop with
  | A.Dot -> TV.concat v1 v2
  | A.Plus -> TV.add v1 v2
  | A.Minus -> TV.sub v1 v2
  | A.Star -> TV.mul v1 v2
  | A.Ltlt -> TV.shift_left v1 v2
  | A.Slash -> TV.div v1 v2
  | A.Bar -> TV.bitwise_or v1 v2
  (* temporarily disabled *)
  (*
  | A.Gtgt -> TV.shift_right v1 v2
  | A.Star -> TV.mul v1 v2

  | A.Percent -> TV.rem v1 v2
  | A.Amp -> TV.bitwise_and v1 v2

  | A.LogXor -> TV.logical_xor v1 v2
  | A.Xor -> TV.bitwise_xor v1 v2
  | A.AMpamp -> TV.logical_and v1 v2
  | A.BArbar -> TV.logical_or v1 v2
  | A.Eqeq -> TV.eqeq v1 v2
  | A.EQeqeq -> TV.eqeqeq v1 v2
  | A.Diff -> TV.diff v1 v2
  | A.Diff2 -> TV.diff2 v1 v2
  | A.Gt -> TV.greater_than v1 v2
  | A.Gte -> TV.greater_than_equals v1 v2
  | A.Lt -> TV.less_than v1 v2
  | A.Lte -> TV.less_than_equals v1 v2 *)
  | _ -> None

(* try to apply type cast to a value *)
let cast_value hint v =
  match hint with
  | A.Happly((_, id), []) ->
    if id = SN.Typehints.int || id = SN.Typehints.integer
    then
      (* temporarily disabled *)
      (* TV.cast_to_int v *)
      None
    else
    if id = SN.Typehints.bool || id = SN.Typehints.boolean
    then TV.cast_to_bool v
    else
    if id = SN.Typehints.string
    then TV.cast_to_string v
    else if id = SN.Typehints.real ||
            id = SN.Typehints.double ||
            id = SN.Typehints.float
    then TV.cast_to_float v
    else None
  | _ -> None

(* We build a visitor over the syntax tree that recursively transforms unary and
 * binary operations on literal expressions.
 * NOTE: although it will exhaustively transform something like 2+(3*4), it does
 * so by converting 3*4 into a Typed_value.Int 12, then back to a literal 12, before
 * transforming this back into a Typed_value.t in order to compute the addition.
 * In future we might try and maintain typed values and avoid going back to
 * expressions. *)
let folder_visitor =
object (self)
  inherit [_] Ast.endo as super

  method! on_class_ _env cd =
    super#on_class_ cd.Ast.c_namespace cd

  method! on_fun_ _env fd =
    super#on_fun_ fd.Ast.f_namespace fd

  (* Type casts. cast_expr is A.Cast(hint, e) *)
  method! on_Cast env cast_expr hint e =
    let enew = self#on_expr env e in
    let default () =
      if enew == e
      then cast_expr
      else A.Cast(hint, enew) in
    match expr_to_opt_typed_value env enew with
    | None -> default ()
    | Some v ->
      match cast_value (snd hint) v with
      | None -> default ()
      | Some v -> value_to_expr_ (fst e) v

  (* Unary operations. unop_expr is A.Unop(unop, e) *)
  method! on_Unop env unop_expr unop e =
    let enew = self#on_expr env e in
    let default () =
      if enew == e
      then unop_expr
      else A.Unop(unop, enew) in
    match expr_to_opt_typed_value env enew with
    | None -> default ()
    | Some v ->
      match unop_on_value unop v with
      | None -> default ()
      | Some result -> value_to_expr_ (fst e) result

  (* Binary operations. binop_expr is A.Binop(binop, e1, e2) *)
  method! on_Binop env binop_expr binop e1 e2 =
    let e1new = self#on_expr env e1 in
    let e2new = self#on_expr env e2 in
    let default () =
      if e1new == e1 && e2new == e2
      then binop_expr
      else (A.Binop(binop, e1new, e2new)) in
    match expr_to_opt_typed_value env e1new,
          expr_to_opt_typed_value env e2new with
    | Some v1, Some v2 ->
      begin match binop_on_values binop v1 v2 with
      | None -> default ()
      | Some result -> value_to_expr_ (fst e1) result
      end
    | _, _ -> default ()

end

let fold_expr ns e =
  folder_visitor#on_expr ns e
let fold_program p =
  folder_visitor#on_program Namespace_env.empty_with_default_popt p

let literals_from_exprs_with_index ns exprs =
  try
    List.concat_mapi exprs (fun index e ->
      [TV.Int (Int64.of_int index); expr_to_typed_value ns (fold_expr ns e)])
  with
  | NotLiteral -> failwith "literals_from_exprs_with_index: not literal"
