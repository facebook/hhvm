(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
module A = Aast
module TV = Typed_value
module SN = Naming_special_names
module SU = Hhbc_string_utils
module TVL = Unique_list_typed_value

exception NotLiteral

exception UserDefinedConstant

let radix (s : string) : [ `Oct | `Hex | `Dec | `Bin ] =
  if String.length s > 1 && s.[0] = '0' then
    match s.[1] with
    (* Binary *)
    | 'b'
    | 'B' ->
      `Bin
    (* Hex *)
    | 'x'
    | 'X' ->
      `Hex
    (* Octal *)
    | _ -> `Oct
  else
    `Dec

(* TODO: once we don't need to be PHP5 compliant, we may want to get rid of
this octal truncation at the first non-octal digit, and instead throw an error
either somewhere around here or in the lexer. *)
let int64_of_octal_opt (s : string) (truncate : bool) : Int64.t option =
  (* If we have a partial result that is strictly larger than this value,
   * then we are in danger of overflowing and should return None *)
  let limit = Int64.( / ) Caml.Int64.max_int (Int64.of_int 8) in
  let to_int64 c = Int64.of_int @@ (int_of_char c - 48) in
  let is_octal_digit ch = '0' <= ch && ch <= '7' in
  let rec loop (idx : int) (acc : Int64.t) =
    (* Given a new least significant digit [digit] and an orginal value [base]
     * append [digit] to the end of [base]. Example:
     * [(push 0o133 0o7) = 0o1337]
     *)
    let push base digit =
      Int64.( + ) (Int64.( * ) base (Int64.of_int 8)) digit
    in
    if idx >= String.length s then
      Some acc
    else if Int64.compare acc limit > 0 then
      (* In this case we would overflow *)
      None
    else if not (is_octal_digit s.[idx]) then
      if truncate then
        Some acc
      else
        None
    else
      loop (idx + 1) (push acc (to_int64 s.[idx]))
  in
  loop 0 Int64.zero

(* Return None if this overflows *)
let try_type_intlike (s : string) : TV.t option =
  match radix s with
  | `Dec ->
    (* Ocaml source: ints.c: parse_sign_and_base treat
       dec form as signed so overflows are properly detected and reported *)
    Option.map (Caml.Int64.of_string_opt s) ~f:(fun v -> TV.Int v)
  | `Bin
  | `Hex ->
    begin
      (* Ocaml source: ints.c: parse_sign_and_base interprets hex/bin forms as
       unsigned so if the input exceeds Int64.max_int it is converted
       to the signed integer Int64.min_int + input - Int64.max_int - 1.*)
      match Int64.of_string s with
      | i ->
        let input_is_negative = s.[0] = '-' in
        (* treat as overflow if result has changed the sign comparing to input *)
        if input_is_negative = (i < 0L) then
          Some (TV.Int i)
        else
          None
      | exception _ -> None
    end
  | `Oct ->
    Option.map
      ~f:(fun v -> TV.Int v)
      (int64_of_octal_opt (String_utils.string_after s 1) true)

(* Literal expressions can be converted into values *)
(* Restrict_keys flag forces keys to be only ints or strings *)
let rec expr_to_typed_value ?(allow_maps = false) ns ((_, expr_) as expr) =
  let pos = Tast_annotate.get_pos expr in
  match expr_ with
  | A.Int s ->
    begin
      match try_type_intlike s with
      | Some v -> v
      | None -> TV.Int Caml.Int64.max_int
    end
  | A.True -> TV.Bool true
  | A.False -> TV.Bool false
  | A.Null -> TV.null
  | A.String s -> TV.String s
  | A.Float s -> TV.Float (float_of_string s)
  | A.Call (_, (_, A.Id (_, id)), _, [(_, A.String data)], None)
    when id = SN.SpecialFunctions.hhas_adata ->
    TV.HhasAdata data
  | A.Array fields -> array_to_typed_value ns fields
  | A.Varray (_, fields) -> varray_to_typed_value ns fields pos
  | A.Darray (_, fields) -> darray_to_typed_value ns fields pos
  (* A.Id *)
  | A.Id (_, id) when id = "NAN" -> TV.Float Float.nan
  | A.Id (_, id) when id = "INF" -> TV.Float Float.infinity
  | A.Id _ -> raise UserDefinedConstant
  (* A.Coolection *)
  | A.Collection ((_, "vec"), _, fields) -> vec_to_typed_value ns pos fields
  | A.Collection ((_, "keyset"), _, fields) ->
    let l =
      List.fold_left
        fields
        ~f:(fun l x -> TVL.add l (keyset_value_afield_to_typed_value ns x))
        ~init:TVL.empty
    in
    TV.Keyset (TVL.items l)
  | A.Collection ((_, kind), _, fields)
    when kind = "dict"
         || allow_maps
            && ( SU.cmp ~case_sensitive:false ~ignore_ns:true kind "Map"
               || SU.cmp ~case_sensitive:false ~ignore_ns:true kind "ImmMap" )
    ->
    let values = List.map fields ~f:(afield_to_typed_value_pair ns) in
    let d = update_duplicates_in_map values in
    TV.Dict (d, Some pos)
  | A.Collection ((_, kind), _, fields)
    when allow_maps
         && ( SU.cmp ~case_sensitive:false ~ignore_ns:true kind "Set"
            || SU.cmp ~case_sensitive:false ~ignore_ns:true kind "ImmSet" ) ->
    let values = List.map fields ~f:(set_afield_to_typed_value_pair ns) in
    let d = update_duplicates_in_map values in
    TV.Dict (d, Some pos)
  (* A.ValCoolection *)
  | A.ValCollection (A.Vec, _, el)
  | A.ValCollection (A.Vector, _, el) ->
    TV.Vec (List.map el (expr_to_typed_value ns), Some pos)
  | A.ValCollection (A.Keyset, _, el) ->
    let fields = List.map el ~f:(fun e -> Aast.AFvalue e) in
    let l =
      List.fold_left
        fields
        ~f:(fun l x -> TVL.add l (keyset_value_afield_to_typed_value ns x))
        ~init:TVL.empty
    in
    TV.Keyset (TVL.items l)
  | A.ValCollection (A.Set, _, el)
  | A.ValCollection (A.ImmSet, _, el) ->
    let fields = List.map el ~f:(fun e -> Aast.AFvalue e) in
    let values = List.map fields ~f:(set_afield_to_typed_value_pair ns) in
    let d = update_duplicates_in_map values in
    TV.Dict (d, Some pos)
  (* A.KeyValCoolection *)
  | A.KeyValCollection (A.Dict, _, fields)
  | A.KeyValCollection (A.Map, _, fields)
  | A.KeyValCollection (A.ImmMap, _, fields) ->
    let values = List.map fields ~f:(kv_to_typed_value_pair ns) in
    let d = update_duplicates_in_map values in
    TV.Dict (d, Some pos)
  (* Others *)
  | A.Shape fields -> shape_to_typed_value ns fields pos
  | A.Class_const (cid, id) -> class_const_to_typed_value cid id
  | A.BracedExpr e -> expr_to_typed_value ~allow_maps ns e
  | A.Class_get _ -> raise UserDefinedConstant
  | A.As (e, (_, A.Hlike _), _nullable) -> expr_to_typed_value ~allow_maps ns e
  | _ -> raise NotLiteral

and vec_to_typed_value ns pos fields =
  TV.Vec (List.map fields (value_afield_to_typed_value ns), Some pos)

and update_duplicates_in_map kvs =
  (* map key -> (the latest value for the given key) *)
  let unique_values_map =
    List.fold_left kvs ~init:TV.TVMap.empty ~f:(fun m (k, v) ->
        TV.TVMap.add k v m)
  in
  let (values, _) =
    List.fold_left
      kvs
      ~init:([], unique_values_map)
      ~f:(fun (result, uniq_map) (k, _) ->
        (* map stores the latest value for a key
         if map has an value for a given key, put value in the result list
         and remove if from the map to ignore similar keys later *)
        match TV.TVMap.find_opt k uniq_map with
        | Some v -> ((k, v) :: result, TV.TVMap.remove k uniq_map)
        | None -> (result, uniq_map))
  in
  List.rev values

and class_const_to_typed_value cid id =
  if snd id = SN.Members.mClass then
    let open Ast_class_expr in
    let cexpr = class_id_to_class_expr ~resolve_self:true [] cid in
    match cexpr with
    | Class_id (_, cname) ->
      let cname = Hhbc_id.Class.(from_ast_name cname |> to_raw_string) in
      TV.String cname
    | _ -> raise UserDefinedConstant
  else
    raise UserDefinedConstant

and array_to_typed_value ns fields =
  let update_max_index newindex maxindex =
    if Int64.compare newindex maxindex >= 0 then
      Int64.( + ) newindex Int64.one
    else
      maxindex
  in
  let default key value pairs maxindex =
    let k_tv = key_expr_to_typed_value ns key in
    let maxindex =
      match k_tv with
      | TV.Int newindex -> update_max_index newindex maxindex
      | _ -> maxindex
    in
    ((k_tv, expr_to_typed_value ns value) :: pairs, maxindex)
  in
  let (pairs, _) =
    List.fold_left
      fields
      ~init:([], Int64.zero)
      ~f:(fun (pairs, maxindex) afield ->
        match afield with
        | A.AFkvalue (key, value) -> default key value pairs maxindex
        | A.AFvalue value ->
          ( (TV.Int maxindex, expr_to_typed_value ns value) :: pairs,
            Int64.( + ) maxindex Int64.one ))
  in
  let a = update_duplicates_in_map @@ List.rev pairs in
  TV.Array a

and varray_to_typed_value ns fields pos =
  let tv_fields = List.map fields ~f:(expr_to_typed_value ns) in
  TV.VArray (tv_fields, Some pos)

and darray_to_typed_value ns fields pos =
  let fields =
    List.map fields ~f:(fun (v1, v2) ->
        (key_expr_to_typed_value ns v1, expr_to_typed_value ns v2))
  in
  let a = update_duplicates_in_map fields in
  TV.DArray (a, Some pos)

and shape_to_typed_value ns fields pos =
  let aux (sf, expr) =
    let key =
      match sf with
      | Ast_defs.SFlit_int (pos, str) ->
        begin
          match
            expr_to_typed_value ns (Tast_annotate.with_pos pos (A.Int str))
          with
          | TV.Int _ as tv -> tv
          | _ -> failwith (str ^ " is not a valid integer index")
        end
      | Ast_defs.SFlit_str id -> TV.String (snd id)
      | Ast_defs.SFclass_const (class_id, id) ->
        class_const_to_typed_value (Tast_annotate.make (A.CI class_id)) id
    in
    (key, expr_to_typed_value ns expr)
  in
  let a = List.map fields ~f:aux in
  TV.DArray (a, Some pos)

and key_expr_to_typed_value ns expr =
  let tv = expr_to_typed_value ns expr in
  match tv with
  | TV.Int _
  | TV.String _ ->
    tv
  | _ -> raise NotLiteral

and afield_to_typed_value_pair ns afield =
  match afield with
  | A.AFvalue _value -> failwith "afield_to_typed_value_pair: unexpected value"
  | A.AFkvalue (k, v) -> kv_to_typed_value_pair ns (k, v)

and kv_to_typed_value_pair ns (key, value) =
  (key_expr_to_typed_value ns key, expr_to_typed_value ns value)

and value_afield_to_typed_value ns afield =
  match afield with
  | A.AFvalue e -> expr_to_typed_value ns e
  | A.AFkvalue (_key, _value) ->
    failwith "value_afield_to_typed_value: unexpected key=>value"

and keyset_value_afield_to_typed_value ns afield =
  let tv = value_afield_to_typed_value ns afield in
  begin
    match tv with
    | TV.Int _
    | TV.String _ ->
      ()
    | _ -> raise NotLiteral
  end;
  tv

and set_afield_to_typed_value_pair ns afield =
  match afield with
  | A.AFvalue value ->
    let tv = key_expr_to_typed_value ns value in
    (tv, tv)
  | A.AFkvalue (_, _) ->
    failwith "set_afield_to_typed_value_pair: unexpected key=>value"

let expr_to_opt_typed_value ?(allow_maps = false) ns e =
  match expr_to_typed_value ~allow_maps ns e with
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
  | TV.VArray (values, _) -> A.Varray (None, List.map values (value_to_expr p))
  | TV.DArray (pairs, _) ->
    A.Darray
      ( None,
        List.map pairs (fun (v1, v2) ->
            (value_to_expr p v1, value_to_expr p v2)) )
  | TV.Dict _ -> failwith "value_to_expr: dict NYI"

and value_to_expr p v = Tast_annotate.make (value_to_expr_ p v)

and value_pair_to_afield p (v1, v2) =
  A.AFkvalue (value_to_expr p v1, value_to_expr p v2)

(* Apply a unary operation on a typed value v.
 * Return None if we can't or won't determine the result *)
let unop_on_value unop v =
  match unop with
  | Ast_defs.Unot -> TV.not v
  | Ast_defs.Uplus -> TV.add TV.zero v
  | Ast_defs.Uminus -> TV.neg v
  | Ast_defs.Utild -> TV.bitwise_not v
  | Ast_defs.Usilence -> Some v
  | _ -> None

(* Likewise for binary operations *)
let binop_on_values binop v1 v2 =
  match binop with
  | Ast_defs.Dot -> TV.concat v1 v2
  | Ast_defs.Plus -> TV.add v1 v2
  | Ast_defs.Minus -> TV.sub v1 v2
  | Ast_defs.Star -> TV.mul v1 v2
  | Ast_defs.Ltlt -> TV.shift_left v1 v2
  | Ast_defs.Slash -> TV.div v1 v2
  | Ast_defs.Bar -> TV.bitwise_or v1 v2
  (* temporarily disabled *)
  (*
  | A.Gtgt -> TV.shift_right v1 v2
  | A.Star -> TV.mul v1 v2

  | A.Percent -> TV.rem v1 v2
  | A.Amp -> TV.bitwise_and v1 v2

  | A.LogXor -> TV.logical_xor v1 v2
  | A.Xor -> TV.bitwise_xor v1 v2
  | A.Ampamp -> TV.logical_and v1 v2
  | A.Barbar -> TV.logical_or v1 v2
  | A.Eqeq -> TV.eqeq v1 v2
  | A.Eqeqeq -> TV.eqeqeq v1 v2
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
  | A.Happly ((_, id), []) ->
    let id = SU.strip_hh_ns id in
    if id = SN.Typehints.int then
      (* temporarily disabled *)
      (* TV.cast_to_int v *)
      None
    else if id = SN.Typehints.bool then
      TV.cast_to_bool v
    else if id = SN.Typehints.string then
      TV.cast_to_string v
    else if id = SN.Typehints.float then
      TV.cast_to_float v
    else
      None
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
    inherit [_] A.endo as super

    method on_'ex _ ex = ex

    method on_'fb _ fb = fb

    method on_'en _ en = en

    method on_'hi _ hi = hi

    method! on_class_ _env cd = super#on_class_ cd.A.c_namespace cd

    method! on_fun_ _env fd = super#on_fun_ fd.A.f_namespace fd

    (* Type casts. cast_expr is A.Cast(hint, e) *)
    method! on_Cast env cast_expr hint e =
      let enew = self#on_expr env e in
      let default () =
        if phys_equal enew e then
          cast_expr
        else
          A.Cast (hint, enew)
      in
      match expr_to_opt_typed_value env enew with
      | None -> default ()
      | Some v ->
        (match cast_value (snd hint) v with
        | None -> default ()
        | Some v -> value_to_expr_ (fst e) v)

    (* Unary operations. unop_expr is A.Unop(unop, e) *)
    method! on_Unop env unop_expr unop e =
      let enew = self#on_expr env e in
      let default () =
        if phys_equal enew e then
          unop_expr
        else
          A.Unop (unop, enew)
      in
      match expr_to_opt_typed_value env enew with
      | None -> default ()
      | Some v ->
        (match unop_on_value unop v with
        | None -> default ()
        | Some result -> value_to_expr_ (fst e) result)

    (* Binary operations. binop_expr is A.Binop (binop, e1, e2) *)
    method! on_Binop env binop_expr binop e1 e2 =
      let e1new = self#on_expr env e1 in
      let e2new = self#on_expr env e2 in
      let default () =
        if phys_equal e1new e1 && phys_equal e2new e2 then
          binop_expr
        else
          A.Binop (binop, e1new, e2new)
      in
      match
        (expr_to_opt_typed_value env e1new, expr_to_opt_typed_value env e2new)
      with
      | (Some v1, Some v2) ->
        begin
          match binop_on_values binop v1 v2 with
          | None -> default ()
          | Some result -> value_to_expr_ (fst e1) result
        end
      | (_, _) -> default ()
  end

let fold_expr ns e = folder_visitor#on_expr ns e

let fold_program ~empty_namespace p =
  folder_visitor#on_program empty_namespace p

let literals_from_exprs ns exprs =
  try List.map exprs ~f:(fun e -> expr_to_typed_value ns (fold_expr ns e))
  with NotLiteral -> failwith "literals_from_exprs: not literal"
