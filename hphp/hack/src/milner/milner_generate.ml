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

let choose_nondet = List.filter ~f:(fun _ -> Random.bool ())

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

  (* suppress warning about to_enum not used *)
  let _ = to_enum

  let all = List.init max ~f:(fun i -> of_enum i |> Option.value_exn)

  let pick () = Random.int_incl min max |> of_enum |> Option.value_exn
end

module Container = struct
  type t =
    | Vec
    | Dict
    | Keyset
  [@@deriving enum]

  (* suppress warning about to_enum not used *)
  let _ = to_enum

  let pick () = Random.int_incl min max |> of_enum |> Option.value_exn
end

module rec Environment : sig
  type t = {
    for_option: bool;
    for_reified: bool;
    for_alias: bool;
    definitions: Definition.t list;
    subtypes: Type.t list TypeMap.t;
  }

  val default : t

  val add_definition : t -> Definition.t -> t

  val definitions : t -> Definition.t list

  val record_subtype : t -> super:Type.t -> sub:Type.t -> t

  val get_subtypes : t -> Type.t -> Type.t list
end = struct
  type t = {
    for_option: bool;
    for_reified: bool;
    for_alias: bool;
    definitions: Definition.t list;
    subtypes: Type.t list TypeMap.t;
  }

  let default =
    {
      for_option = false;
      for_reified = false;
      for_alias = false;
      definitions = [];
      subtypes = TypeMap.empty;
    }

  let add_definition env def = { env with definitions = def :: env.definitions }

  let definitions env = env.definitions

  let record_subtype env ~super ~sub =
    let add = function
      | None -> Some [sub]
      | Some subs -> Some (sub :: subs)
    in
    { env with subtypes = TypeMap.update super add env.subtypes }

  let get_subtypes env super =
    Option.value ~default:[] @@ TypeMap.find_opt super env.subtypes
end

and Kind : sig
  type t =
    | Mixed
    | Primitive
    | Option
    | Classish
    | Alias
    | Newtype
    | TypeConst
    | Case
    | Enum
    | Container
    | Tuple
    | Shape
    | Awaitable
    | Function
    | Like

  val pick : Environment.t -> t

  type classish =
    | Class
    | Interface
    | AbstractClass
  [@@deriving eq, ord]

  val pick_classish : unit -> classish
end = struct
  type t =
    | Mixed
    | Primitive
    | Option
    | Classish
    | Alias
    | Newtype
    | TypeConst
    | Case
    | Enum
    | Container
    | Tuple
    | Shape
    | Awaitable
    | Function
    | Like
  [@@deriving enum, eq]

  (* suppress warning about to_enum not used *)
  let _ = to_enum

  let pick env =
    let kinds =
      List.range ~start:`inclusive ~stop:`inclusive min max
      |> List.map ~f:(fun i -> of_enum i |> Option.value_exn)
    in
    let kinds =
      if env.Environment.for_alias then
        List.filter ~f:(fun k -> not @@ equal k TypeConst) kinds
      else
        kinds
    in
    let kinds =
      if env.Environment.for_reified then
        List.filter ~f:(fun k -> not @@ equal k Function) kinds
      else
        kinds
    in
    let kinds =
      if env.Environment.for_option then
        List.filter ~f:(fun k -> not @@ equal k Case) kinds
      else
        kinds
    in
    select kinds

  type classish =
    | Class
    | Interface
    | AbstractClass
  [@@deriving enum, eq, ord]

  (* suppress warning about classish_to_enum not used *)
  let _ = classish_to_enum

  let pick_classish () =
    Random.int_incl min_classish max_classish
    |> classish_of_enum
    |> Option.value_exn
end

and Definition : sig
  type t

  val show : t -> string

  val typeconst : name:string -> Type.t -> t

  val classish :
    name:string ->
    parent:(Kind.classish * string * Type.generic option) option ->
    generic:Type.generic option ->
    members:t list ->
    Kind.classish ->
    t

  val alias : name:string -> Type.t -> t

  val newtype : name:string -> bound:Type.t option -> Type.t -> t

  val case_type : name:string -> bound:Type.t option -> Type.t list -> t

  val enum : name:string -> bound:Type.t option -> Type.t -> value:string -> t
end = struct
  type t = string

  let show def = def

  let typeconst ~name aliased =
    Format.sprintf "const type %s = %s;" name (Type.show aliased)

  let classish
      ~name
      ~(parent : (Kind.classish * string * Type.generic option) option)
      ~(generic : Type.generic option)
      ~(members : t list)
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
    let body =
      if List.is_empty members then
        "{}"
      else
        Format.sprintf
          "{\n  %s\n}"
          (List.map ~f:show members |> String.concat ~sep:"\n  ")
    in
    Format.sprintf "%s %s%s %s%s" kind name generic parent body

  let alias ~name aliased =
    Format.sprintf "type %s = %s;" name (Type.show aliased)

  let newtype ~name ~bound aliased =
    match bound with
    | Some bound ->
      Format.sprintf
        "newtype %s as %s = %s;"
        name
        (Type.show bound)
        (Type.show aliased)
    | None -> Format.sprintf "newtype %s = %s;" name (Type.show aliased)

  let case_type ~name ~bound disjuncts =
    let rhs = String.concat ~sep:" | " (List.map ~f:Type.show disjuncts) in
    match bound with
    | Some bound ->
      Format.sprintf "case type %s as %s = %s;" name (Type.show bound) rhs
    | None -> Format.sprintf "case type %s = %s;" name rhs

  let enum ~name ~bound ty ~value =
    match bound with
    | Some bound ->
      Format.sprintf
        "enum %s: %s as %s { A = %s; }"
        name
        (Type.show ty)
        (Type.show bound)
        value
    | None -> Format.sprintf "enum %s: %s { A = %s; }" name (Type.show ty) value
end

and Type : sig
  type t [@@deriving ord]

  type generic = {
    instantiation: t;
    is_reified: bool;
  }

  val show : t -> string

  val inhabitant_of : Environment.t -> t -> string

  val mk : Environment.t -> depth:int option -> Environment.t * t
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
    | Awaitable of t
    | Classish of {
        name: string;
        kind: Kind.classish;
        generic: generic option;
      }
    | Alias of {
        name: string;
        aliased: t;
      }
    | Newtype of {
        name: string;
        aliased: t;
      }
    | TypeConst of {
        name: string;
        aliased: t;
      }
    | Case of { name: string }
    | Enum of { name: string }
    | Vec of t
    | Dict of {
        key: t;
        value: t;
      }
    | Keyset of t
    | Tuple of {
        conjuncts: t list;
        open_: bool;
      }
    | Shape of {
        fields: field list;
        open_: bool;
      }
    | Function of {
        parameters: t list;
        variadic: t option;
        return_: t;
      }
    | Like of t
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
    | Enum _ -> true
    | Vec _ -> true
    | Dict _ -> true
    | Keyset _ -> true
    | Tuple { conjuncts; open_ } ->
      List.for_all conjuncts ~f:is_immediately_inhabited && not open_
    | Shape { fields; open_ = _ } ->
      List.for_all fields ~f:(fun field -> is_immediately_inhabited field.ty)
    | Awaitable ty -> is_immediately_inhabited ty
    | Function { parameters = _; variadic = _; return_ } ->
      is_immediately_inhabited return_
    | Mixed
    | Option _
    | Alias _
    | Newtype _
    | TypeConst _
    | Case _
    | Like _ ->
      false

  (** Picks an inhabited subfield of a given field *)
  let rec subfield_of env { key; ty; optional } =
    let ty = subtype_of env ty in
    let optional =
      if optional then
        select [true; false]
      else
        false
    in
    { key; ty; optional }

  (** Goes on a stochastic walk to pick an inhabited subtype of the given type.

      Termination of this function crucially relies on the invariant that EVERY
      type has an inhabitant subtype. There are quite few denotable types where
      this invariant does not hold, most trivially the `nothing` type. *)
  and subtype_of env ty =
    let rec step ty =
      ty
      :: begin
           Environment.get_subtypes env ty
           @
           match ty with
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
           | Option ty -> [Primitive Primitive.Null; ty]
           | Awaitable ty ->
             let ty = select @@ step ty in
             [Awaitable ty]
           | Alias info -> [info.aliased]
           | TypeConst info -> [info.aliased]
           | Newtype info -> [info.aliased]
           | Classish _
           | Case _
           | Enum _ ->
             []
           | Vec ty ->
             let ty = select @@ step ty in
             [Vec ty]
           | Dict { key; value } ->
             let key = select @@ step key in
             let value = select @@ step value in
             [Dict { key; value }]
           | Keyset ty ->
             let ty = select @@ step ty in
             [Keyset ty]
           | Tuple { conjuncts; open_ } ->
             let conjuncts = List.map ~f:(Fn.compose select step) conjuncts in
             let open_ =
               if open_ then
                 (* Here we should be adding new conjuncts, but with the current setup
                    that's too expensive. Need memoization to make it more affordable. *)
                 select [true; false]
               else
                 false
             in
             [Tuple { conjuncts; open_ }]
           | Shape { fields; open_ } ->
             let fields = List.map ~f:(subfield_of env) fields in
             let open_ =
               if open_ then
                 (* Here we should be adding new fields, but with the current setup
                    that's too expensive. Need memoization to make it more affordable. *)
                 select [true; false]
               else
                 false
             in
             [Shape { fields; open_ }]
           | Function { parameters; variadic; return_ } ->
             let return_ = select @@ step return_ in
             let variadic =
               match variadic with
               | None -> select [None; Some (select @@ step Mixed)]
               | Some ty -> Some ty
             in
             [Function { parameters; variadic; return_ }]
           | Like ty ->
             (* TODO: dynamic here when it is supported *)
             step ty
         end
    in
    let rec driver ty =
      let tys = step ty in
      if Random.bool () then
        (* We decide to go on a stochastic walk and explore further subtypes. *)
        driver @@ select tys
      else
        (* We would like to terminate with an immediately inhabited type. *)
        let (immediately_inhabited, latent_inhabited) =
          List.partition_tf ~f:is_immediately_inhabited tys
        in
        if List.is_empty immediately_inhabited then
          (* There are no immediately inhabited type, so we must keep exploring. *)
          driver @@ select latent_inhabited
        else
          select immediately_inhabited
    in
    driver ty

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
    | Awaitable ty -> Format.sprintf "Awaitable<%s>" (show ty)
    | Classish { name; generic; kind = _ } ->
      let generic =
        match generic with
        | Some generic -> Format.sprintf "<%s>" (show generic.instantiation)
        | None -> ""
      in
      Format.sprintf "%s%s" name generic
    | Alias info -> info.name
    | Newtype info -> info.name
    | TypeConst info -> info.name
    | Case info -> info.name
    | Enum info -> info.name
    | Vec ty -> Format.sprintf "vec<%s>" (show ty)
    | Dict { key; value } ->
      Format.sprintf "dict<%s, %s>" (show key) (show value)
    | Keyset ty -> Format.sprintf "keyset<%s>" (show ty)
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
    | Function { parameters; variadic; return_ } ->
      let variadic =
        match variadic with
        | Some ty ->
          (if List.is_empty parameters then
            ""
          else
            ", ")
          ^ show ty
          ^ "..."
        | None -> ""
      in
      let parameters = List.map ~f:show parameters |> String.concat ~sep:", " in
      let return_ = show return_ in
      Format.sprintf "(function(%s%s): %s)" parameters variadic return_
    | Like ty -> "~" ^ show ty

  let are_disjoint env ty ty' =
    (* For the purposes of disjointness we can go higher up in the typing
       hierarchy so that it is easy to enumerate subtypes. This is fine because
       it can only make disjointness more conservative.

       The reason we output multiple types is that not every type has a unique
       weakening to establish disjointness. For example, 2-tuples are not
       disjoint from 3-tuples. So we weaken all tuples to (mixed),
       (mixed,mixed), and (mixed,mixed,mixed) (because we only generate 1/2/3 tuples).

       Although we don't have to keep the non-weakened types for disjointness
       checking, it makes termination of `weaken_for_disjointness` trivial, so
       we pay the price.
    *)
    let weaken_for_disjointness ty : t list =
      let rec step ty =
        match ty with
        | Classish info when Kind.equal_classish info.kind Kind.Interface ->
          (* This can be improved on if we introduce an internal Object type which
             is still disjoint to non classish types. *)
          [Mixed]
        | Option ty -> [Primitive Primitive.Null; ty]
        | Awaitable _ -> [Awaitable Mixed]
        | Alias info -> [info.aliased]
        | TypeConst info -> [info.aliased]
        | Newtype info -> [info.aliased]
        | Case _ -> Environment.get_subtypes env ty
        | Enum _ -> Primitive.[Primitive Int; Primitive String]
        | Vec _ -> [Vec Mixed; Tuple { conjuncts = []; open_ = true }]
        | Dict _ ->
          [
            Dict { key = Primitive Primitive.Arraykey; value = Mixed };
            Shape { fields = []; open_ = true };
          ]
        | Keyset _ -> [Keyset (Primitive Primitive.Arraykey)]
        | Tuple _ -> [Tuple { conjuncts = []; open_ = true }; Vec Mixed]
        | Shape _ ->
          [
            Shape { fields = []; open_ = true };
            Dict { key = Primitive Primitive.Arraykey; value = Mixed };
          ]
        | Function _ ->
          [Classish { kind = Kind.Class; name = "Closure"; generic = None }]
        | Mixed -> [ty]
        | Primitive Primitive.Arraykey ->
          Primitive.[Primitive Int; Primitive String]
        | Primitive Primitive.Num -> Primitive.[Primitive Int; Primitive Float]
        | Primitive _ -> [ty]
        | Classish _ -> Environment.get_subtypes env ty
        | Like _ -> [Mixed]
      and driver acc =
        let acc' =
          TypeSet.to_list acc
          |> List.concat_map ~f:step
          |> TypeSet.of_list
          |> TypeSet.union acc
        in
        if TypeSet.cardinal acc = TypeSet.cardinal acc' then
          acc
        else
          driver acc'
      in
      driver (TypeSet.singleton ty) |> TypeSet.to_list
    in
    let ordered_subtypes ty =
      weaken_for_disjointness ty |> List.sort ~compare
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
    | Enum info -> Some (info.name ^ "::A")
    | Vec _ -> Some "vec[]"
    | Dict _ -> Some "dict[]"
    | Keyset _ -> Some "keyset[]"
    | Tuple { conjuncts; open_ = _ } ->
      List.map ~f:expr_of conjuncts
      |> Option.all
      |> Option.map ~f:(fun exprl ->
             String.concat ~sep:", " exprl |> Format.sprintf "tuple(%s)")
    | Shape { fields; open_ = _ } ->
      let fields =
        List.filter fields ~f:(fun f -> (not f.optional) || Random.bool ())
      in
      let fields = List.permute fields in
      let show_field { key; ty; _ } =
        expr_of ty |> Option.map ~f:(Format.sprintf "%s => %s" key)
      in
      List.map ~f:show_field fields
      |> Option.all
      |> Option.map ~f:(fun fields ->
             String.concat ~sep:", " fields |> Format.sprintf "shape(%s)")
    | Awaitable ty ->
      let open Option.Let_syntax in
      let+ expr = expr_of ty in
      Format.sprintf "async { return %s; }" expr
    | Function { parameters; variadic; return_ } ->
      let variadic =
        match variadic with
        | Some ty ->
          (if List.is_empty parameters then
            ""
          else
            ", ")
          ^ show ty
          ^ " ...$_"
        | None -> ""
      in
      let parameters =
        List.map ~f:(fun param -> show param ^ " $_") parameters
        |> String.concat ~sep:", "
      in
      let open Option.Let_syntax in
      let+ return_expr = expr_of return_ in
      let return_ = Type.show return_ in
      Format.sprintf
        "(%s%s): %s ==> { return %s; }"
        parameters
        variadic
        return_
        return_expr
    | Mixed
    | Option _
    | Alias _
    | Newtype _
    | TypeConst _
    | Case _
    | Like _ ->
      None

  let inhabitant_of env ty =
    let subtype = subtype_of env ty in
    let inhabitant = expr_of subtype in
    match inhabitant with
    | Some inhabitant -> inhabitant
    | None ->
      raise
      @@ Failure
           ("Tried to find an inhabitant for a type: "
           ^ show ty
           ^ " but it is uninhabitaed. This indicates bug in `milner`.")

  let mk_arraykey env = subtype_of env (Primitive Primitive.Arraykey)

  let rec mk_classish
      env
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
    let gen_children env ~parent n =
      List.init n ~f:(fun _ -> ())
      |> List.fold ~init:env ~f:(fun env _ ->
             let (env, child) =
               mk_classish env ~parent:(Some parent) ~depth:(depth + 1)
             in
             let (kind, name, generic) = parent in
             Environment.record_subtype
               env
               ~super:(Classish { kind; name; generic })
               ~sub:child)
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
    let (env, generic) =
      if depth <= max_hierarchy_depth && Random.bool () then
        let is_reified =
          (not Kind.(equal_classish kind Interface)) && Random.bool ()
        in
        let (env, instantiation) =
          let env =
            Environment.
              {
                env with
                for_option = false;
                for_reified = is_reified || env.Environment.for_reified;
                for_alias = false;
              }
          in
          mk env ~depth:(Some depth)
        in
        (env, Some { instantiation; is_reified })
      else
        (env, None)
    in
    let env = gen_children env ~parent:(kind, name, generic) num_of_children in
    let env =
      Environment.add_definition env
      @@ Definition.classish kind ~name ~parent ~generic ~members:[]
    in
    (env, Classish { kind; name; generic })

  and mk (env : Environment.t) ~(depth : int option) : Environment.t * t =
    let depth = Option.value ~default:0 depth in
    let mk ?(for_alias = false) env =
      mk Environment.{ env with for_alias } ~depth:(Some depth)
    in
    match Kind.pick env with
    | Kind.Mixed -> (env, Mixed)
    | Kind.Primitive -> (env, Primitive (Primitive.pick ()))
    | Kind.Option ->
      let rec candidate () =
        match mk Environment.{ env with for_option = true } with
        | (_, Mixed) -> candidate ()
        | (_, Option _) as res ->
          res
          (* Due to some misguided checks the parser and the typechecker has. We
             need to eliminate these cases. *)
        | (env, ty) -> (env, Option ty)
      in
      candidate ()
    | Kind.Awaitable ->
      let (env, ty) = mk Environment.{ env with for_option = false } in
      (env, Awaitable ty)
    | Kind.Classish -> mk_classish env ~parent:None ~depth
    | Kind.Alias ->
      let name = fresh "A" in
      let (env, aliased) = mk env ~for_alias:true in
      let env =
        Environment.add_definition env @@ Definition.alias ~name aliased
      in
      (env, Alias { name; aliased })
    | Kind.Newtype ->
      let name = fresh "N" in
      let mk = mk ~for_alias:true in
      let (env, aliased, bound) =
        if Random.bool () then
          let (env, bound) = mk env in
          let aliased = subtype_of env bound in
          (env, aliased, Some bound)
        else
          let (env, aliased) = mk env in
          (env, aliased, None)
      in
      let env =
        Environment.add_definition env
        @@ Definition.newtype ~name ~bound aliased
      in
      let ty = Newtype { name; aliased } in
      (env, ty)
    | Kind.TypeConst ->
      let tc_name = fresh "TC" in
      let (env, aliased) = mk env in
      let typeconst_def = Definition.typeconst ~name:tc_name aliased in
      let class_name = fresh "CTC" in
      let env =
        Environment.add_definition env
        @@ Definition.classish
             ~name:class_name
             ~parent:None
             ~generic:None
             ~members:[typeconst_def]
             Kind.Class
      in
      let qualified_name = Format.sprintf "%s::%s" class_name tc_name in
      (env, TypeConst { name = qualified_name; aliased })
    | Kind.Case ->
      let name = fresh "CT" in
      let (env, bound) =
        if Random.bool () then
          let (env, bound) = mk env in
          (env, Some bound)
        else
          (env, None)
      in
      let mk env =
        match bound with
        | Some bound ->
          let ty = subtype_of env bound in
          (env, ty)
        | None -> mk env ~for_alias:true
      in
      let rec add_disjuncts (env, disjuncts) =
        if Random.bool () then
          let (env, disjunct) = mk env in
          if List.for_all disjuncts ~f:(are_disjoint env disjunct) then
            add_disjuncts @@ (env, disjunct :: disjuncts)
          else
            add_disjuncts (env, disjuncts)
        else
          (env, disjuncts)
      in
      let (env, ty) = mk env in
      let (env, disjuncts) = add_disjuncts (env, [ty]) in
      let env =
        Environment.add_definition env
        @@ Definition.case_type ~name ~bound disjuncts
      in
      let ty = Case { name } in
      let env =
        Option.fold bound ~init:env ~f:(fun env bound ->
            Environment.record_subtype env ~super:bound ~sub:ty)
      in
      let env =
        List.fold disjuncts ~init:env ~f:(fun env disjunct ->
            Environment.record_subtype env ~super:ty ~sub:disjunct)
      in
      (env, ty)
    | Kind.Enum ->
      let name = fresh "E" in
      let ty = Enum { name } in
      let (env, bound, underlying_ty, value) =
        if Random.bool () then
          let bound = mk_arraykey env in
          let underlying_ty = subtype_of env bound in
          let value = inhabitant_of env underlying_ty in
          let env = Environment.record_subtype env ~super:bound ~sub:ty in
          (env, Some bound, underlying_ty, value)
        else
          let underlying_ty = mk_arraykey env in
          let value = inhabitant_of env underlying_ty in
          let env = Environment.record_subtype env ~super:Mixed ~sub:ty in
          (env, None, underlying_ty, value)
      in
      let env =
        Environment.add_definition env
        @@ Definition.enum ~name ~bound underlying_ty ~value
      in
      (env, ty)
    | Kind.Container -> begin
      let env = Environment.{ env with for_option = false } in
      match Container.pick () with
      | Container.Vec ->
        let (env, ty) = mk env in
        (env, Vec ty)
      | Container.Dict ->
        let key = mk_arraykey env in
        let (env, value) = mk env in
        (env, Dict { key; value })
      | Container.Keyset ->
        let ty = mk_arraykey env in
        (env, Keyset ty)
    end
    | Kind.Tuple ->
      (* Sadly, although nullary tuples can be generated with an expression,
         there is no corresponding denotable type. *)
      let n = geometric_between min_tuple_arity max_tuple_arity in
      let env = Environment.{ env with for_option = false } in
      let (env, conjuncts) =
        List.init n ~f:(fun _ -> ())
        |> List.fold_map ~init:env ~f:(fun env _ -> mk env)
      in
      (env, Tuple { conjuncts; open_ = false })
    | Kind.Shape ->
      let keys = choose_nondet shape_keys in
      let mk_field env key =
        let (env, ty) = mk Environment.{ env with for_option = false } in
        let optional = Random.bool () in
        (env, { key; optional; ty })
      in
      let (env, fields) = List.fold_map ~init:env ~f:mk_field keys in
      let open_ = Random.bool () in
      (env, Shape { fields; open_ })
    | Kind.Function ->
      let env = Environment.{ env with for_option = false } in
      let (env, parameters) =
        List.init (geometric_between 0 3) ~f:(fun _ -> ())
        |> List.fold_map ~init:env ~f:(fun env _ -> mk env)
      in
      let (env, return_) = mk env in
      let (env, variadic) =
        if Random.bool () then
          (env, None)
        else
          let (env, ty) = mk env in
          (env, Some ty)
      in
      (env, Function { parameters; variadic; return_ })
    | Kind.Like ->
      let (env, ty) = mk env in
      (env, Like ty)
end

and TypeMap : (WrappedMap.S with type key = Type.t) = WrappedMap.Make (Type)
and TypeSet : (Stdlib.Set.S with type elt = Type.t) = Stdlib.Set.Make (Type)
