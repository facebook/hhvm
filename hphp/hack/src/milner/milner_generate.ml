(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let max_hierarchy_depth = 2

let max_branching_factor = 2

let min_tuple_arity = 1

let max_tuple_arity = 3

let shape_keys = ["'a'"; "'b'"; "'c'"]

let name_ctr = ref 0

let map_and_collect ~f xs =
  let f (ys, acc) x =
    let (y, l) = f x in
    (y :: ys, l @ acc)
  in
  let init = ([], []) in
  List.fold xs ~init ~f

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

let select_or_id ~pick xs =
  if pick then
    [select xs]
  else
    xs

let choose_nondet = List.filter ~f:(fun _ -> Random.bool ())

let permute_nondet xs =
  let len = List.length xs in
  List.map xs ~f:(fun x -> (Random.int len, x))
  |> List.sort ~compare:(fun a b -> compare (fst a) (fst b))
  |> List.map ~f:snd

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
    | Shape
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
    name:string ->
    parent:(Kind.classish * string * Type.generic option) option ->
    generic:Type.generic option ->
    Kind.classish ->
    t

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

  let classish
      ~name
      ~(parent : (Kind.classish * string * Type.generic option) option)
      ~(generic : Type.generic option)
      kind =
    let parent =
      match parent with
      | Some (parent_kind, name, generic) -> begin
        let generic =
          match generic with
          | Some generic ->
            Format.sprintf "<%s>" Type.(show generic.instantiation)
          | None -> ""
        in
        match parent_kind with
        | Kind.Interface when not (Kind.equal_classish kind Kind.Interface) ->
          Format.sprintf "implements %s%s " name generic
        | _ -> Format.sprintf "extends %s%s " name generic
      end
      | None -> ""
    in
    let generic =
      match generic with
      | Some Type.{ is_reified; _ } ->
        if is_reified then
          "<reify T>"
        else
          "<T>"
      | None -> ""
    in
    let kind =
      match kind with
      | Kind.Class -> "class"
      | Kind.AbstractClass -> "abstract class"
      | Kind.Interface -> "interface"
    in
    Format.sprintf "%s %s%s %s{}" kind name generic parent

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

  type generic = {
    instantiation: t;
    is_reified: bool;
  }

  val show : t -> string

  val inhabitant_of : t -> string

  val mk : depth:int option -> t * Definition.t list
end = struct
  type field = {
    key: string;
    ty: t;
    optional: bool;
  }

  and generic = {
    instantiation: t;
    is_reified: bool;
  }

  and t =
    | Mixed
    | Primitive of Primitive.t
    | Option of t
    | Classish of {
        name: string;
        kind: Kind.classish;
        children: t list;
        generic: generic option;
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
        conjuncts: t list;
        open_: bool;
      }
    | Shape of {
        fields: field list;
        open_: bool;
      }
  [@@deriving eq, ord]

  let rec is_immediately_inhabited = function
    | Primitive prim -> begin
      let open Primitive in
      match prim with
      | Null
      | Int
      | String
      | Float
      | Bool ->
        true
      | Arraykey
      | Num ->
        false
    end
    | Classish info -> begin
      match info.kind with
      | Kind.AbstractClass
      | Kind.Interface ->
        false
      | Kind.Class -> true
    end
    | Newtype _ -> true
    | Enum _ -> true
    | Tuple { conjuncts; open_ } ->
      List.for_all conjuncts ~f:is_immediately_inhabited && not open_
    | Shape { fields; open_ = _ } ->
      List.for_all fields ~f:(fun field -> is_immediately_inhabited field.ty)
    | Mixed
    | Option _
    | Alias _
    | Case _ ->
      false

  (** Computes all subfields of a given field. It combines all subtypes with
      optional status of the field.

      If `pick` is set it curtails field selection to one inhabited subfield for
      efficiency. *)
  let rec subfields_of ~pick { key; ty; optional } =
    let open List.Let_syntax in
    let* ty = subtypes_of ~pick ty in
    let+ optional =
      if optional then
        select_or_id ~pick [true; false]
      else
        [false]
    in
    { key; ty; optional }

  (** Reflexive transitive subtypes of the given type. It is based only on the
      knowledge about the structure of type and generalities in the system. For
      example, for mixed, we give int as a subtype but no classes because the
      function does not know which classes exist in the program.

      If `pick` is set to true, then we curtail picked types along the way to
      make it efficient to pick an inhabited type. *)
  and subtypes_of ~pick ty =
    let subtypes_of = subtypes_of ~pick in
    let subfields_of = subfields_of ~pick in
    let pick_inhabited xs =
      if pick then
        [select @@ List.filter ~f:is_immediately_inhabited xs]
      else
        xs
    in
    pick_inhabited
    @@ ty
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
       | Tuple { conjuncts; open_ } ->
         let open List.Let_syntax in
         let* conjuncts =
           List.map ~f:subtypes_of conjuncts |> List.Cartesian_product.all
         in
         let+ open_ =
           if open_ then
             (* Here we should be adding new conjuncts, but with the current setup
                that's too expensive. Need memoization to make it more affordable. *)
             select_or_id ~pick [true; false]
           else
             [false]
         in
         Tuple { conjuncts; open_ }
       | Shape { fields; open_ } ->
         let open List.Let_syntax in
         let* fields =
           List.map ~f:subfields_of fields |> List.Cartesian_product.all
         in
         let+ open_ =
           if open_ then
             (* Here we should be adding new fields, but with the current setup
                that's too expensive. Need memoization to make it more affordable. *)
             select_or_id ~pick [true; false]
           else
             [false]
         in
         Shape { fields; open_ })

  let rec show_field { key; ty; optional } =
    let optional =
      if optional then
        "?"
      else
        ""
    in
    Format.sprintf "%s%s => %s" optional key (show ty)

  and show = function
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
    | Classish { name; generic; children = _; kind = _ } ->
      let generic =
        match generic with
        | Some generic -> Format.sprintf "<%s>" (show generic.instantiation)
        | None -> ""
      in
      Format.sprintf "%s%s" name generic
    | Alias info -> info.name
    | Newtype info -> info.name
    | Case info -> info.name
    | Enum info -> info.name
    | Tuple { conjuncts; open_ } ->
      let is_nullary = List.length conjuncts = 0 in
      let conjuncts = List.map ~f:show conjuncts |> String.concat ~sep:", " in
      let open_ =
        if open_ && is_nullary then
          "..."
        else if open_ then
          ", ..."
        else
          ""
      in
      Format.sprintf "(%s%s)" conjuncts open_
    | Shape { fields; open_ } ->
      let is_nullary = List.length fields = 0 in
      let fields = permute_nondet fields in
      let fields = List.map ~f:show_field fields |> String.concat ~sep:", " in
      let open_ =
        if open_ && is_nullary then
          "..."
        else if open_ then
          ", ..."
        else
          ""
      in
      Format.sprintf "shape(%s%s)" fields open_

  let are_disjoint ty ty' =
    (* For the purposes of disjointness we can go higher up in the typing
       hierarchy so that it is easy to enumerate subtypes. This is fine because
       it can only make disjointness more conservative.

       The reason we output multiple types is that not every type has a unique
       weakening to establish disjointness. For example, 2-tuples are not
       disjoint from 3-tuples. So we weaken all tuples to (mixed),
       (mixed,mixed), and (mixed,mixed,mixed) (because we only generate 1/2/3 tuples).
    *)
    let rec weaken_for_disjointness ty : t list =
      match ty with
      | Classish info when Kind.equal_classish info.kind Kind.Interface ->
        (* This can be improved on if we introduce an internal Object type which
           is still disjoint to non classish types. *)
        [Mixed]
      | Option ty ->
        let open List.Let_syntax in
        let+ ty = weaken_for_disjointness ty in
        Option ty
      | Alias info -> weaken_for_disjointness info.aliased
      | Newtype _ -> [Mixed]
      | Case { name; disjuncts } ->
        let open List.Let_syntax in
        let+ disjuncts =
          List.map ~f:weaken_for_disjointness disjuncts
          |> List.Cartesian_product.all
        in
        Case { name; disjuncts }
      | Enum _ -> [Primitive Primitive.Arraykey]
      | Tuple _ -> [Tuple { conjuncts = []; open_ = true }]
      | Shape _ -> [Shape { fields = []; open_ = true }]
      | Mixed
      | Primitive _
      | Classish _ ->
        [ty]
    in
    let ordered_subtypes ty =
      weaken_for_disjointness ty
      |> List.concat_map ~f:(subtypes_of ~pick:false)
      |> List.sort ~compare
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
      | Kind.Class ->
        let generic =
          match info.generic with
          | Some generic when generic.is_reified || Random.bool () ->
            Format.sprintf "<%s>" (Type.show generic.instantiation)
          | _ -> ""
        in
        Some (Format.sprintf "new %s%s()" info.name generic)
    end
    | Newtype info -> Some (info.producer ^ "()")
    | Enum info -> Some (info.name ^ "::A")
    | Tuple { conjuncts; open_ = _ } ->
      List.map ~f:expr_of conjuncts
      |> Option.all
      |> Option.map ~f:(fun exprl ->
             String.concat ~sep:", " exprl |> Format.sprintf "tuple(%s)")
    | Shape { fields; open_ = _ } ->
      let fields =
        List.filter fields ~f:(fun f -> (not f.optional) || Random.bool ())
      in
      let fields = permute_nondet fields in
      let show_field { key; ty; _ } =
        expr_of ty |> Option.map ~f:(Format.sprintf "%s => %s" key)
      in
      List.map ~f:show_field fields
      |> Option.all
      |> Option.map ~f:(fun fields ->
             String.concat ~sep:", " fields |> Format.sprintf "shape(%s)")
    | Mixed
    | Option _
    | Alias _
    | Case _ ->
      None

  let inhabitant_of ty =
    let subtypes = subtypes_of ~pick:true ty in
    let inhabitants = List.filter_map subtypes ~f:expr_of in
    try select inhabitants with
    | Failure _ ->
      raise
      @@ Failure
           ("Tried to find an inhabitant for a type: "
           ^ show ty
           ^ " but it is uninhabitaed. This indicates bug in `milner`.")

  let rec mk_classish
      ~(parent : (Kind.classish * string * generic option) option)
      ~(depth : int) =
    let kind =
      if depth > max_hierarchy_depth then
        Kind.Class
      else
        match parent with
        | Some (Kind.(Class | AbstractClass), _, _) ->
          select [Kind.Class; Kind.AbstractClass]
        | Some (Kind.Interface, _, _)
        | None ->
          Kind.pick_classish ()
    in
    let gen_children ~parent n =
      List.init n ~f:(fun _ -> mk_classish ~parent ~depth:(depth + 1))
      |> map_and_collect ~f:Fn.id
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
    let (generic, generic_defs) =
      if depth <= max_hierarchy_depth && Random.bool () then
        let (instantiation, defs) = mk ~depth:(Some depth) in
        let is_reified =
          (not Kind.(equal_classish kind Interface)) && Random.bool ()
        in
        (Some { instantiation; is_reified }, defs)
      else
        (None, [])
    in
    let (children, defs) =
      gen_children ~parent:(Some (kind, name, generic)) num_of_children
    in
    let def = Definition.classish kind ~name ~parent ~generic in
    (Classish { name; kind; children; generic }, def :: (generic_defs @ defs))

  and mk ~(depth : int option) =
    let depth = Option.value ~default:0 depth in
    let mk () = mk ~depth:(Some depth) in
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
    | Kind.Classish -> mk_classish ~parent:None ~depth
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
      (* Sadly, although nullary tuples can be generated with an expression,
         there is no corresponding denotable type. *)
      let n = geometric_between min_tuple_arity max_tuple_arity in
      let (conjuncts, defs) =
        List.init n ~f:(fun _ -> mk ()) |> map_and_collect ~f:Fn.id
      in
      (Tuple { conjuncts; open_ = false }, defs)
    | Kind.Shape ->
      let keys = choose_nondet shape_keys in
      let mk_field key =
        let (ty, defs) = mk () in
        let optional = Random.bool () in
        ({ key; optional; ty }, defs)
      in
      let (fields, defs) = map_and_collect ~f:mk_field keys in
      let open_ = Random.bool () in
      (Shape { fields; open_ }, defs)
end
