(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let max_hierarchy_depth = 3

let max_branching_factor = 2

let name_ctr = ref 0

let fresh prefix =
  let n = !name_ctr in
  name_ctr := !name_ctr + 1;
  prefix ^ "_" ^ string_of_int n

(** Utility function for choosing numbers between `min` and `max` where
    approaching `max` gets harder and harder. *)
let rec geometric_between min max =
  if min > max then
    max
  else if Random.bool () then
    min
  else
    geometric_between (min + 1) max

let select l = List.length l - 1 |> Random.int_incl 0 |> List.nth_exn l

module Primitive = struct
  type t =
    | Null
    | Int
    | String
    | Float
    | Bool
    | Arraykey
    | Num
  [@@deriving enum, eq, ord]

  let all = List.init max ~f:(fun i -> of_enum i |> Option.value_exn)

  let pick () = Random.int_incl min max |> of_enum |> Option.value_exn
end

module Kind = struct
  type t =
    | Mixed
    | Primitive
    | Option
    | Classish
    | Alias
    | Newtype
    | Case
    | Enum
    | Tuple
  [@@deriving enum]

  let pick () = Random.int_incl min max |> of_enum |> Option.value_exn

  type classish =
    | Class
    | Interface
    | AbstractClass
  [@@deriving enum, eq, ord]

  let pick_classish () =
    Random.int_incl min_classish max_classish
    |> classish_of_enum
    |> Option.value_exn
end

module rec Definition : sig
  type t

  val show : t -> string

  val function_ : name:string -> ret:Type.t -> ret_expr:string -> t

  val classish :
    name:string -> parent:(Kind.classish * string) option -> Kind.classish -> t

  val alias : name:string -> Type.t -> t

  val newtype : name:string -> Type.t -> t

  val case_type : name:string -> Type.t list -> t

  val enum : name:string -> Type.t -> value:string -> t
end = struct
  type t = string

  let show def = def

  let function_ ~name ~ret ~ret_expr =
    Format.sprintf
      "function %s(): %s { return %s; }"
      name
      (Type.show ret)
      ret_expr

  let classish ~name ~parent kind =
    let parent =
      match parent with
      | None -> ""
      | Some (Kind.Interface, name)
        when not (Kind.equal_classish kind Kind.Interface) ->
        Format.sprintf "implements %s " name
      | Some (_, name) -> Format.sprintf "extends %s " name
    in
    match kind with
    | Kind.Class -> Format.sprintf "class %s %s{}" name parent
    | Kind.AbstractClass -> Format.sprintf "abstract class %s %s{}" name parent
    | Kind.Interface -> Format.sprintf "interface %s %s{}" name parent

  let alias ~name aliased =
    Format.sprintf "type %s = %s;" name (Type.show aliased)

  let newtype ~name aliased =
    Format.sprintf "newtype %s = %s;" name (Type.show aliased)

  let case_type ~name disjuncts =
    let rhs = String.concat ~sep:" | " (List.map ~f:Type.show disjuncts) in
    Format.sprintf "case type %s = %s;" name rhs

  let enum ~name ty ~value =
    Format.sprintf "enum %s: %s { A = %s; }" name (Type.show ty) value
end

and Type : sig
  type t

  val show : t -> string

  val inhabitant_of : t -> string

  val mk : unit -> t * Definition.t list
end = struct
  type t =
    | Mixed
    | Primitive of Primitive.t
    | Option of t
    | Classish of {
        name: string;
        kind: Kind.classish;
        children: t list;
      }
    | Alias of {
        name: string;
        aliased: t;
      }
    | Newtype of {
        name: string;
        producer: string;
      }
    | Case of {
        name: string;
        disjuncts: t list;
      }
    | Enum of { name: string }
    | Tuple of {
        fst: t;
        snd: t;
      }
  [@@deriving eq, ord]

  (** Reflexive transitive subtypes of the given type. It is based only on the
      knowledge about the structure of type and generalities in the system. For
      example, for mixed, we give int as a subtype but no classes because the
      function does not know which classes exist in the program. *)
  let rec subtypes_of ty =
    ty
    ::
    (match ty with
    | Mixed -> List.map ~f:(fun prim -> Primitive prim) Primitive.all
    | Primitive prim -> begin
      let open Primitive in
      match prim with
      | Null
      | Int
      | String
      | Float
      | Bool ->
        []
      | Arraykey -> [Primitive Int; Primitive String]
      | Num -> [Primitive Int; Primitive Float]
    end
    | Option ty -> Primitive Primitive.Null :: subtypes_of ty
    | Classish info -> List.concat_map ~f:subtypes_of info.children
    | Alias info -> subtypes_of info.aliased
    | Newtype _ -> []
    | Case info -> List.concat_map ~f:subtypes_of info.disjuncts
    | Enum _ -> []
    | Tuple { fst; snd } ->
      let fst_subtypes = subtypes_of fst in
      let snd_subtypes = subtypes_of snd in
      List.Cartesian_product.map2 fst_subtypes snd_subtypes ~f:(fun fst snd ->
          Tuple { fst; snd }))

  let rec show = function
    | Mixed -> "mixed"
    | Primitive prim -> begin
      let open Primitive in
      match prim with
      | Null -> "null"
      | Int -> "int"
      | String -> "string"
      | Float -> "float"
      | Bool -> "bool"
      | Arraykey -> "arraykey"
      | Num -> "num"
    end
    | Option ty -> "?" ^ show ty
    | Classish info -> info.name
    | Alias info -> info.name
    | Newtype info -> info.name
    | Case info -> info.name
    | Enum info -> info.name
    | Tuple { fst; snd } -> Format.sprintf "(%s,%s)" (show fst) (show snd)

  let are_disjoint ty ty' =
    (* For the purposes of disjointness we can go higher up in the typing
       hierarchy so that it is easy to enumerate subtypes. This is fine because
       it can only make disjointness more conservative. *)
    let rec weaken_for_disjointness ty =
      match ty with
      | Classish info when Kind.equal_classish info.kind Kind.Interface ->
        (* This can be improved on if we introduce an internal Object type which
           is still disjoint to non classish types. *)
        Mixed
      | Option ty -> Option (weaken_for_disjointness ty)
      | Alias info -> weaken_for_disjointness info.aliased
      | Newtype _ -> Mixed
      | Case { name; disjuncts } ->
        Case { name; disjuncts = List.map ~f:weaken_for_disjointness disjuncts }
      | Enum _ -> Primitive Primitive.Arraykey
      | Tuple _ -> Tuple { fst = Mixed; snd = Mixed }
      | Mixed
      | Primitive _
      | Classish _ ->
        ty
    in
    let ordered_subtypes ty =
      weaken_for_disjointness ty |> subtypes_of |> List.sort ~compare
    in
    let subtypes = ordered_subtypes ty in
    let subtypes' = ordered_subtypes ty' in
    let rec have_overlapping_types = function
      | (_, []) -> false
      | ([], _) -> false
      | (x :: xs, y :: ys) ->
        let result = compare x y in
        result = 0
        ||
        if result > 0 then
          have_overlapping_types (x :: xs, ys)
        else
          have_overlapping_types (xs, y :: ys)
    in
    not
    @@ (List.mem subtypes Mixed ~equal
       || List.mem subtypes' Mixed ~equal
       || have_overlapping_types (subtypes, subtypes'))

  let rec expr_of = function
    | Primitive prim -> begin
      let open Primitive in
      match prim with
      | Null -> Some "null"
      | Int -> Some "42"
      | String -> Some "'apple'"
      | Float -> Some "42.0"
      | Bool -> Some "true"
      | Arraykey -> None
      | Num -> None
    end
    | Classish info -> begin
      match info.kind with
      | Kind.AbstractClass
      | Kind.Interface ->
        None
      | Kind.Class -> Some ("new " ^ info.name ^ "()")
    end
    | Newtype info -> Some (info.producer ^ "()")
    | Enum info -> Some (info.name ^ "::A")
    | Tuple { fst; snd } ->
      let open Option.Let_syntax in
      let* fst_str = expr_of fst in
      let* snd_str = expr_of snd in
      let str = Format.sprintf "tuple(%s,%s)" fst_str snd_str in
      Some str
    | Mixed
    | Option _
    | Alias _
    | Case _ ->
      None

  let inhabitant_of ty =
    let subtypes = subtypes_of ty in
    let inhabitants = List.filter_map subtypes ~f:expr_of in
    try select inhabitants with
    | Failure _ ->
      raise
      @@ Failure
           ("Tried to find an inhabitant for a type: "
           ^ show ty
           ^ " but it is uninhabitaed. This indicates bug in `milner`.")

  let rec mk_classish ~parent ~depth =
    let kind =
      if depth > max_hierarchy_depth then
        Kind.Class
      else
        match parent with
        | Some (Kind.(Class | AbstractClass), _) ->
          select [Kind.Class; Kind.AbstractClass]
        | Some (Kind.Interface, _)
        | None ->
          Kind.pick_classish ()
    in
    let gen_children ~parent n =
      List.init n ~f:(fun _ -> mk_classish ~parent ~depth:(depth + 1))
      |> List.fold ~init:([], []) ~f:(fun (tys, defss) (ty, defs) ->
             (ty :: tys, defs @ defss))
    in
    let (name, num_of_children) =
      match kind with
      | Kind.AbstractClass ->
        let name = fresh "AC" in
        (* Since abstract classes are not instantiable, we add at least one
           child. This way we can always find an inhabitant for this type. *)
        let num_of_children = geometric_between 1 max_branching_factor in
        (name, num_of_children)
      | Kind.Interface ->
        let name = fresh "I" in
        (* Since interfaces are not instantiable, we add at least one child.
           This way we can always find an inhabitant for this type. *)
        let num_of_children = geometric_between 1 max_branching_factor in
        (name, num_of_children)
      | Kind.Class ->
        let name = fresh "C" in
        let num_of_children =
          if depth > max_hierarchy_depth then
            0
          else
            geometric_between 0 max_branching_factor
        in
        (name, num_of_children)
    in
    let (children, defs) =
      gen_children ~parent:(Some (kind, name)) num_of_children
    in
    let def = Definition.classish kind ~name ~parent in
    (Classish { name; kind; children }, def :: defs)

  let rec mk () =
    match Kind.pick () with
    | Kind.Mixed -> (Mixed, [])
    | Kind.Primitive -> (Primitive (Primitive.pick ()), [])
    | Kind.Option ->
      let rec candidate () =
        match mk () with
        | (Mixed, _)
        | (Option _, _) ->
          (* Due to some misguided checks the parser and the typechecker has. We
             need to eliminate these cases. *)
          candidate ()
        | res -> res
      in
      let (ty, defs) = candidate () in
      (Option ty, defs)
    | Kind.Classish -> mk_classish ~parent:None ~depth:0
    | Kind.Alias ->
      let name = fresh "A" in
      let (aliased, defs) = mk () in
      (Alias { name; aliased }, Definition.alias ~name aliased :: defs)
    | Kind.Newtype ->
      let name = fresh "N" in
      let (aliased, defs) = mk () in
      let producer = fresh ("mk" ^ name) in
      let newtype_def = Definition.newtype ~name aliased in
      let aliased_expr = inhabitant_of aliased in
      let ty = Newtype { name; producer } in
      let newtype_producer_def =
        Definition.function_ ~name:producer ~ret:ty ~ret_expr:aliased_expr
      in
      (ty, newtype_def :: newtype_producer_def :: defs)
    | Kind.Case ->
      let name = fresh "CT" in
      let rec add_disjuncts (disjuncts, defs) =
        if Random.bool () then
          let (disjunct, defs') = mk () in
          if List.for_all disjuncts ~f:(are_disjoint disjunct) then
            add_disjuncts @@ (disjunct :: disjuncts, defs' @ defs)
          else
            add_disjuncts (disjuncts, defs)
        else
          (disjuncts, defs)
      in
      let (ty, defs) = mk () in
      let (disjuncts, defs) = add_disjuncts ([ty], defs) in
      let case_type_def = Definition.case_type ~name disjuncts in
      (Case { name; disjuncts }, case_type_def :: defs)
    | Kind.Enum ->
      let name = fresh "E" in
      let underlying_ty =
        select Primitive.[Primitive Arraykey; Primitive String; Primitive Int]
      in
      let value = inhabitant_of underlying_ty in
      let enum_def = Definition.enum ~name underlying_ty ~value in
      (Enum { name }, [enum_def])
    | Kind.Tuple ->
      let (fst, fst_defs) = mk () in
      let (snd, snd_defs) = mk () in
      (Tuple { fst; snd }, fst_defs @ snd_defs)
end
