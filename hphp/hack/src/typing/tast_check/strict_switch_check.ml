(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Aast
open Typing_defs
module EnumErr = Typing_error.Primary.Enum
module EnumConstSet = Set.Make (String)

module EnumInfo = struct
  type t = {
    name: string;
    decl_pos: (Pos_or_decl.t[@sexp.opaque]);
        [@compare (fun _ _ -> 0)] [@hash.ignore]
    consts: EnumConstSet.t; [@compare (fun _ _ -> 0)] [@hash.ignore]
  }
  [@@deriving ord, sexp, hash]

  let to_json { name; consts; decl_pos } =
    let rec take n = function
      | [] -> []
      | x :: xs ->
        if n <= 0 then
          ["..."]
        else
          x :: take (n - 1) xs
    in
    Hh_json.JSON_Object
      [
        ("enum", Hh_json.string_ @@ Utils.strip_ns name);
        ( "values",
          consts
          |> EnumConstSet.to_list
          |> take 10
          |> Hh_json.array_ Hh_json.string_ );
        ("decl_pos", Pos_or_decl.json decl_pos);
      ]

  let of_decl decl ~name =
    decl
    |> Decl_provider.Class.consts
    |> List.filter_map ~f:(fun (name, _) ->
           if String.(name <> "class") then
             Some (Utils.strip_ns name)
           else
             None)
    |> EnumConstSet.of_list
    |> fun consts -> { name; decl_pos = Decl_provider.Class.pos decl; consts }

  let of_env env name =
    Tast_env.get_enum env name |> Option.map ~f:(of_decl ~name)
end

type literal = EnumErr.Const.t

type ast_case = (Tast.ty, Tast.saved_env) Aast.expr

type case = Pos.t * literal

module Value = struct
  type t =
    | Unsupported  (** requires default case *)
    | Null
    | Bool of bool
    | Int
    | String
    | AllEnums
    | Enum of EnumInfo.t
  [@@deriving ord, sexp, hash]

  let bools = [Bool true; Bool false]

  let universe = Unsupported :: Null :: Int :: String :: AllEnums :: bools

  let finite_or_dynamic = function
    | Unsupported
    | Int
    | String
    | AllEnums ->
      false
    | Null
    | Bool _
    | Enum _ ->
      true

  let to_json = function
    | Unsupported -> Hh_json.string_ "unsupported"
    | Null -> Hh_json.string_ "null"
    | Bool bool -> bool |> Bool.to_string |> Hh_json.string_
    | Int -> Hh_json.string_ "int"
    | String -> Hh_json.string_ "string"
    | AllEnums -> Hh_json.string_ "enum"
    | Enum info -> EnumInfo.to_json info

  let if_missing : t -> _ option = function
    | Null -> Some `Null
    | Bool b -> Some (`Bool b)
    | Int -> Some `Int
    | String -> Some `String
    | Enum EnumInfo.{ name; consts = _; decl_pos = _ } -> Some (`Enum name)
    | AllEnums -> Some `AnyEnum
    | Unsupported -> None

  type value = t

  (* design choice: case expressions should only allow literals or class
     constants, and so disregard non-literal expressions *)
  let and_literal_of_ast_case env ((ty, _, expr) : ast_case) :
      t list * literal option =
    let open EnumErr.Const in
    match expr with
    | Null -> ([Null], Some Null)
    | True -> ([Bool true], Some (Bool true))
    | False -> ([Bool false], Some (Bool false))
    | Int literal ->
      (* we rely on the fact that there are no underscores in the string literal,
         so that "1_0_0", "100" and "10_0" are considered the same *)
      assert (not @@ String.contains literal '_');
      ([Int], Some (Int (Some literal)))
    | String literal -> ([String], Some (String (Some literal)))
    | Class_const ((_, _, CI (_, class_)), (_, const)) ->
      let rnone = Reason.Rnone in
      let (int, string, arraykey) =
        Typing_make_type.(int rnone, string rnone, arraykey rnone)
      in
      let is_sub ty1 ty2 =
        Typing_subtype.is_sub_type (Tast_env.tast_env_as_typing_env env) ty1 ty2
      in

      let list_if b (x : value list) =
        if b then
          x
        else
          []
      in

      let values =
        (* only class constants which are subtypes of arraykeys are currently supported;
           ideally this would be restricted to only enums but this depends on how often
           class constants are used as case expressions *)
        if not @@ is_sub ty arraykey then
          [Unsupported]
        else if is_sub arraykey ty then
          (* ty <: arraykey && arraykey <: ty *)
          [Int; String; AllEnums]
        else
          (* ty <: arraykey && ! (arraykey <: ty) *)
          list_if (is_sub ty int) [Int]
          @ list_if (is_sub ty string) [String]
          @ Option.value_map
              (EnumInfo.of_env env class_)
              ~default:[]
              ~f:(fun info -> [Enum info; AllEnums])
      in
      (values, Some (Label { class_; const }))
    | _ -> ([Unsupported], None)

  let is_countable_enum = function
    | Enum _ -> true
    | Unsupported
    | Null
    | Bool _
    | Int
    | String
    | AllEnums ->
      false

  (* NOTE: this relies on intersections between transparent enum and int/string
     being handled by type simplification prior to the value computation. *)
  (* NOTE: this under-approximates intersection between two enums by selecting the left *)
  let inter v1 v2 =
    match (v1, v2) with
    | ((Enum _ as e), AllEnums)
    | (AllEnums, (Enum _ as e)) ->
      Some e
    | (Enum info, Enum _) -> Some (Enum info)
    | _ -> None
end

module ValueSet = struct
  include Set.Make (Value)

  let universe = of_list Value.universe

  let bools = of_list Value.bools

  let symbolic_union_list set_list =
    (* so big_union [ { AllEnums } , { Enum i1 } , { Enum i2 } ] = { AllEnums } *)
    let res = union_list set_list in
    if mem res Value.AllEnums then
      filter res ~f:(fun v -> not @@ Value.is_countable_enum v)
    else
      res

  (* Though this is O(n^2 log (n)) where n is the mean size of the argument sets,
     the type simplification should mean that types are disjunctive-normal form,
     and so both sets should usually just be singletons. *)
  let symbolic_inter value_set1 value_set2 =
    fold_right value_set1 ~init:empty ~f:(fun v1 acc ->
        fold_right value_set2 ~init:acc ~f:(fun v2 acc ->
            Option.value_map (Value.inter v1 v2) ~default:acc ~f:(add acc)))

  let symbolic_inter_list value_sets =
    List.fold_right ~init:universe ~f:symbolic_inter value_sets

  let non_symbolic_diff = diff

  let symbolic_diff value_set1 value_set2 =
    let add_if_mem1 res ~elem =
      if mem value_set1 elem then
        add res elem
      else
        res
    in
    non_symbolic_diff value_set1 value_set2
    (* so { Unsupported } - { Unsupported }  is safely over-approximated to { Other } *)
    |> add_if_mem1 ~elem:Value.Unsupported
    (* so { AllEnums } - { AllEnums }  is safely over-approximated to { AllEnums } *)
    |> add_if_mem1 ~elem:Value.AllEnums
end

let prim_to_values = function
  | Tnoreturn -> ValueSet.empty
  | Tnull
  | Tvoid ->
    ValueSet.singleton Value.Null
  | Tbool -> ValueSet.bools
  | Tint -> ValueSet.singleton Value.Int
  | Tnum -> ValueSet.of_list Value.[Int; Unsupported]
  | Tstring ->
    ValueSet.singleton Value.String
    (* arraykey is the supertype of strings, ints, and all enums *)
  | Tarraykey -> ValueSet.of_list Value.[Int; String; AllEnums]
  | Tfloat
  | Tresource ->
    ValueSet.singleton Value.Unsupported

(* Symbolically evaluate the values corresponding to a given type; the result is
   essentially in disjunctive normal form, so that cases can be partitioned
   according to each element (transparent enums need special care, see later). *)
let rec symbolic_dnf_values env ty : ValueSet.t =
  match Typing_defs.get_node ty with
  | Tunion tyl ->
    tyl |> List.map ~f:(symbolic_dnf_values env) |> ValueSet.symbolic_union_list
  | Tintersection tyl ->
    tyl |> List.map ~f:(symbolic_dnf_values env) |> ValueSet.symbolic_inter_list
  | Tprim prim -> prim_to_values prim
  | Tnonnull -> ValueSet.(symbolic_diff universe (singleton Value.Null))
  | Toption ty -> begin
    match Typing_defs.get_node ty with
    (* This won't work when dealing with types that are semantically the same as
       nonnull, but are syntactically different. For instance if a generic or type
       constant ends up with a equality bound of nonnull (can happen via use of
       where constraints). This can be generalised by checking subtyping both ways
       (ty <: nonnull && nonnull <: ty) but for now it's not clear if it affects
       expressivity in a tangible way. *)
    | Tnonnull -> ValueSet.universe
    | _ -> ValueSet.(add (symbolic_dnf_values env ty) Value.Null)
  end
  | Tneg (Neg_prim prim) ->
    ValueSet.(symbolic_diff universe (prim_to_values prim))
  | Tneg (Neg_class _) (* a safe over-approximation *)
  | Tany _ ->
    ValueSet.universe
  | Tnewtype (newtype_name, args, _) ->
    let open If_enum_or_enum_class in
    apply
      env
      newtype_name
      args
      ~default:(ValueSet.singleton Value.Unsupported)
      ~f:(function
        | Enum { name; class_decl } ->
          ValueSet.singleton (Value.Enum (EnumInfo.of_decl ~name class_decl))
        | EnumClass _
        | EnumClassLabel _ ->
          ValueSet.singleton Value.Unsupported)
  | Tdynamic
  | Ttuple _
  | Tshape _
  | Tvec_or_dict _
  | Tclass _
  | Tvar _
  | Tfun _
  | Tgeneric _
  | Tdependent _
  | Taccess _ ->
    ValueSet.singleton Value.Unsupported
  | Tunapplied_alias _ ->
    Typing_defs.error_Tunapplied_alias_in_illegal_context ()

(* Partition cases based on the kind of literal expression (and later, type) *)
let partition_cases env (cases : (ast_case * _) list) values =
  let partitions : (Value.t, _ * case list) Hashtbl.t =
    let tbl = Hashtbl.create (module Value) in

    ValueSet.iter values ~f:(fun key ->
        Option.iter (Value.if_missing key) ~f:(fun if_missing ->
            Hashtbl.add_exn tbl ~key ~data:(if_missing, [])));
    tbl
  in

  let key_present_data_consed table ~key ~data =
    Hashtbl.find_and_call
      table
      key
      ~if_found:(fun (if_missing, rest) ->
        Hashtbl.set table ~key ~data:(if_missing, data :: rest);
        true)
      ~if_not_found:(fun _ -> false)
  in

  let unused_cases =
    List.fold_left cases ~init:[] ~f:(fun unused (((ty, pos, _) as case), _) ->
        let (keys, lit) = Value.and_literal_of_ast_case env case in
        match lit with
        | None -> (ty, pos, None) :: unused
        | Some lit ->
          let added =
            List.map keys ~f:(fun key ->
                key_present_data_consed partitions ~key ~data:(pos, lit))
          in
          if List.exists added ~f:(fun x -> x) then
            unused
          else
            (ty, pos, Some lit) :: unused)
  in
  (partitions, unused_cases)

let add_err env err =
  Typing_error_utils.add_typing_error ~env @@ Typing_error.enum err

let get_missing_cases env partitions =
  let check_enum_coverage EnumInfo.{ name; consts; decl_pos } cases =
    let given_cases =
      EnumConstSet.of_list
      @@ List.map cases ~f:(fun (_, const) ->
             match const with
             | EnumErr.Const.Label { class_; const } ->
               (* check_enum_coverage is called on a (Enum info, cases)
                  key-value pair from the table created by partition_cases. Both
                  the key (Enum info) and the literal for an ast_case
                  are determined by Value.and_literal_of_ast_case; this is what
                  ensures that all `Enum info` only point to (a) Labels and (b)
                  Labels where name = class_. Enforcing this invariant in the
                  type system would require heterogenous maps *)
               assert (String.(name = class_));
               const
             | _ -> assert false)
    in
    match EnumConstSet.(diff consts given_cases |> to_list) with
    | [] -> None
    | _ :: _ as missing ->
      Some
        (`Labels (decl_pos, List.map missing ~f:(fun const -> (name, const))))
  in

  let check_redundant env cases =
    Hashtbl.group
      (module EnumErr.Const)
      cases
      ~get_key:snd
      ~get_data:(fun (pos, _) -> [pos])
      ~combine:( @ )
    (* ...to perform a redundant case check. This relies on the
       `Strict_switch_int_literal_check` to ensure that all integer literals
       are in the same format (hex, bin, oct, dec). Otherwise int literals
       could represent the same value (e.g. "0b10" and "2") but not be caught
       as redundant. *)
    |> Hashtbl.iteri ~f:(fun ~key:const_name ~data:positions ->
           match positions with
           | [] ->
             (* ~get_data never produces an empty list;
                ~combine preserves non-emptiness *)
             assert false
           | [_] -> ()
           | redundant_pos :: (_ :: _ as tl) ->
             let first_pos = List.last_exn tl in
             add_err env
             @@ EnumErr.Enum_switch_redundant
                  { const_name; first_pos; pos = redundant_pos })
  in

  let get_missing env key missing cases =
    let missing_enum =
      match key with
      | Value.Enum enum_info -> Some (check_enum_coverage enum_info cases)
      | _ -> None
    in
    match cases with
    (* `Some None` is a valid result for missing_enum - even when there are no
       cases, e.g. an enum with no members *)
    | [] -> Option.value missing_enum ~default:(Some missing)
    | _ :: _ ->
      check_redundant env cases;
      Option.value missing_enum ~default:None
  in

  Hashtbl.fold partitions ~init:[] ~f:(fun ~key ~data:(if_missing, cases) acc ->
      (get_missing env key if_missing cases |> Option.to_list) @ acc)

let error_unused_cases env expected_ty unused_cases =
  List.iter unused_cases ~f:(fun (ty, pos, lit) ->
      add_err env
      @@
      match lit with
      | None -> EnumErr.Enum_switch_not_const pos
      | Some _ ->
        EnumErr.Enum_switch_wrong_class
          {
            pos;
            kind = "";
            expected = lazy (Typing_print.full_strip_ns env expected_ty);
            actual = lazy (Typing_print.full_strip_ns env ty);
            expected_pos = expected_ty |> Typing_defs.get_pos |> Option.some;
          })

type ast_default = (Tast.ty, Tast.saved_env) Aast.default_case

let check_default
    env
    pos
    (opt_default_case : ast_default option)
    needs_default
    types_and_labels =
  match (opt_default_case, needs_default) with
  | (None, false)
  | (Some _, true) ->
    ()
  | (Some (default_pos, _), false) ->
    add_err env
    @@ EnumErr.Enum_switch_redundant_default
         {
           pos;
           kind = "default";
           decl_pos = Pos_or_decl.of_raw_pos default_pos;
         }
  | (None, true) ->
    let type_or_label = function
      | None -> Either.First None
      | Some `Null -> Either.First (Some EnumErr.Const.Null)
      | Some (`Bool b) -> Either.First (Some (EnumErr.Const.Bool b))
      | Some `Int -> Either.First (Some (EnumErr.Const.Int None))
      | Some `String -> Either.First (Some (EnumErr.Const.String None))
      | Some (`Enum class_) ->
        Either.First (Some EnumErr.Const.(Label { class_; const = class_ }))
      | Some `AnyEnum ->
        Either.First
          (Some EnumErr.Const.(Label { class_ = "enum"; const = "enum" }))
      | Some (`Labels (decl_pos, labels)) ->
        Either.Second
          ( decl_pos,
            List.map labels ~f:(fun (class_, const) ->
                Some (EnumErr.Const.Label { class_; const })) )
    in
    (* `labels` is for missing enum labels, it has `decl_pos` for the error;
       `types` is for missing types *)
    let (types, labels) =
      List.partition_map ~f:type_or_label types_and_labels
    in
    (if not @@ List.is_empty types then
      let decl_pos = Pos_or_decl.of_raw_pos pos in
      add_err env
      @@ EnumErr.Enum_switch_nonexhaustive
           { pos; kind = None; decl_pos; missing = types });
    List.iter labels ~f:(fun (decl_pos, missing) ->
        add_err env
        @@ EnumErr.Enum_switch_nonexhaustive
             { pos; kind = Some "enum"; decl_pos; missing })

let add_default_if_needed values missing_cases =
  (* write `exists ~f:(fun x -> not @@ finite_or_dynamic x)` instead of
     `forall ~f:finite` to ensure set is non-empty *)
  if ValueSet.exists ~f:(fun x -> not @@ Value.finite_or_dynamic x) values then
    None :: List.map ~f:Option.some missing_cases
  else
    List.map ~f:Option.some missing_cases

(* The algorithm below works as follows:
   1. Partition the case list into buckets for each (non-dynamic) element of the computed set.
     a. Any leftover values are considered to be redundant/ill-typed.
   2. For each finite value (bool, null, and enums) check that all cases have been covered.
     a. Note: the code does not check if each infinite (non-dynamic) values
        has at least one case.
   3. If any of the below are true, then we need a default case.
     a. The value set is the singleton { Dynamic }.
     b. The value set contains an neither-finite-nor-dynamic value.
     c. There are missing cases for finite values.
   4. Otherwise a default is redundant. *)
let check_cases_against_values env pos expected values cases opt_default_case =
  let typing_env = Tast_env.tast_env_as_typing_env env in
  let (partitions, unused_cases) = partition_cases env cases values in
  error_unused_cases typing_env expected unused_cases;
  let missing_cases =
    get_missing_cases typing_env partitions |> add_default_if_needed values
  in
  let needs_default = not (List.is_empty missing_cases) in
  check_default typing_env pos opt_default_case needs_default missing_cases

let add_fields json ~fields =
  match json with
  | Hh_json.JSON_Object old -> Hh_json.JSON_Object (old @ fields)
  | _ -> Hh_json.JSON_Object (("warning_expected_object", json) :: fields)

let check_exhaustiveness env pos ty cases opt_default =
  let values = symbolic_dnf_values env ty in
  check_cases_against_values env pos ty values cases opt_default;
  let tcopt = env |> Tast_env.get_decl_env |> Decl_env.tcopt in
  if TypecheckerOptions.tco_log_exhaustivity_check tcopt then
    let fields =
      [
        ("values", values |> ValueSet.to_list |> Hh_json.array_ Value.to_json);
        ("switch_pos", Pos.(pos |> to_absolute |> json));
      ]
    in
    ty
    |> Tast_env.ty_to_json env ~show_like_ty:true
    |> add_fields ~fields
    |> Hh_json.json_to_string
    |> Hh_logger.log "[hh_tco_enable_strict_switch] %s"

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_stmt env x =
      match snd x with
      | Switch ((scrutinee_ty, scrutinee_pos, _), cases, opt_default_case) ->
        let (_, scrutinee_ty) =
          Typing_union.simplify_unions
            (Tast_env.tast_env_as_typing_env env)
            scrutinee_ty
        in
        check_exhaustiveness
          env
          scrutinee_pos
          scrutinee_ty
          cases
          opt_default_case
      | _ -> ()
  end
