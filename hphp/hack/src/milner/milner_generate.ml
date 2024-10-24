(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let default_complexity = 5

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

module ReadOnlyEnvironment : sig
  type debug_info = {
    verbose: int;
    nesting: int;
        (** Used for keeping track of indentation of nested debug output. It
            increases each time we go under a debug frame where the log level is
            greater than that of the debug frame.. *)
    debug_pattern: string option;
        (** When there is a debug pattern, debug logs are only activated if that
            pattern appears as a substring in the log. This allows hiding unneeded
            output. *)
    in_debug_mode: bool;
        (** When set to true, debug outputs with appropriate log level will be
            logged to STDERR. *)
  }

  (** Read-only environment primarily contains contextual information that
      prevents generation of certain types due to various restrictions in the
      language. This in turn allows us to keep generating well-typed programs.
      *)
  type t = {
    for_option_ty: bool;
        (** There is a case type completeness bug (T201523298) that causes
            nullable case types to be rejected even if the program is valid. So
            we prevent case types to be generated under nullables. *)
    for_reified_ty: bool;
        (** Function types cannot used reified type arguments. *)
    for_alias_def: bool;
        (** Aliases (including vanilla aliases, newtypes, and case types) cannot
            refer to type constants due to an HHVM issues (T29968063). *)
    for_enum_def: bool;
        (** There is a check that bans case types to be used as a bound or the
            underlying type of an enum. *)
    pick_immediately_inhabited: bool;
        (** milner has the ability to create an inhabitant of any type it
            generates to replace a placeholder. To find such types, it computes
            subtypes that are trivially inhabited. For example, `arraykey` is
            _eventually_ inhabited whereas `int` and `string` are immediately
            inhabited.

            This option guides the search so that `subtype_of` produces an
            immediately inhabited type. *)
    debug_info: debug_info;
  }

  val default : verbose:int -> debug_pattern:string option -> t

  val debug :
    level:int ->
    t ->
    start:string Lazy.t ->
    end_:('a -> string) ->
    (t -> 'a) ->
    'a
end = struct
  type debug_info = {
    verbose: int;
    nesting: int;
    debug_pattern: string option;
    in_debug_mode: bool;
  }

  type t = {
    for_option_ty: bool;
    for_reified_ty: bool;
    for_alias_def: bool;
    for_enum_def: bool;
    pick_immediately_inhabited: bool;
    debug_info: debug_info;
  }

  let default ~verbose ~debug_pattern =
    {
      for_option_ty = false;
      for_reified_ty = false;
      for_alias_def = false;
      for_enum_def = false;
      pick_immediately_inhabited = false;
      debug_info =
        {
          verbose;
          nesting = 0;
          debug_pattern;
          in_debug_mode =
            (* If there is a debug pattern in effect, then by default we are not in
               debug mode. This changes once the debug pattern starts matching. *)
            Option.is_none debug_pattern;
        };
    }

  let show
      {
        for_option_ty;
        for_reified_ty;
        for_alias_def;
        for_enum_def;
        pick_immediately_inhabited;
        debug_info = _;
      } =
    Format.sprintf
      "{for_option_ty: %b, for_reified_ty: %b, for_alias_def: %b; for_enum_def: %b; pick_immediately_inhabited: %b}"
      for_option_ty
      for_reified_ty
      for_alias_def
      for_enum_def
      pick_immediately_inhabited

  let debug ~level ({ debug_info; _ } as renv) ~start ~end_ f =
    if debug_info.verbose >= level then begin
      let start = Lazy.force start in
      if
        debug_info.in_debug_mode
        || Option.value_map debug_info.debug_pattern ~default:false ~f:(fun p ->
               String.is_substring start ~substring:p)
      then begin
        let debug_info =
          {
            debug_info with
            nesting = debug_info.nesting + 1;
            in_debug_mode = true;
          }
        in
        let renv = { renv with debug_info } in
        let indentation = String.make (debug_info.nesting * 2) ' ' in
        Format.eprintf "%s[START][%d] %s\n" indentation debug_info.nesting start;
        Format.eprintf "%s  %s\n" indentation (show renv);
        Out_channel.flush Out_channel.stderr;
        let start_time = Sys.time () in
        let res = f renv in
        let duration = Sys.time () -. start_time in
        Format.eprintf
          "%s[ END ][%d][%.3fs] %s -> %s\n"
          indentation
          debug_info.nesting
          duration
          start
          (end_ res);
        Out_channel.flush Out_channel.stderr;
        res
      end else
        f renv
    end else
      f renv
end

module rec Environment : sig
  type t = {
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
    definitions: Definition.t list;
    subtypes: Type.t list TypeMap.t;
  }

  let default = { definitions = []; subtypes = TypeMap.empty }

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
  [@@deriving show { with_path = false }]

  (** Picks a kind that conforms to the constraints in the ReadOnlyEnvironment
      and the complexity budget.

      If the complexity budget is at 0 (or less), it will only pick kinds that
      lead to types with no other types in its structure. For example, we can
      pick a primitive or an alias because these don't have type arguments (in
      primitive case, this is inherent and in the alias case, it is a detail of
      the current implementation), but not a class because it can have a generic
      or a nullable type which always have a type under `?`. *)
  val pick : complexity:int -> ReadOnlyEnvironment.t -> t

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
  [@@deriving show { with_path = false }, enum, eq]

  (* suppress warning about to_enum not used *)
  let _ = to_enum

  let pick
      ~complexity
      ReadOnlyEnvironment.{ for_alias_def; for_reified_ty; for_option_ty; _ } =
    let kinds =
      List.range ~start:`inclusive ~stop:`inclusive min max
      |> List.map ~f:(fun i -> of_enum i |> Option.value_exn)
    in
    let kind_filter = function
      | Case -> not for_option_ty
      | Function -> not for_reified_ty
      | TypeConst -> not for_alias_def
      | _ -> true
    in
    (* Complexity filter ensures that we don't generate heavily nested types
       which make program generation expensive. *)
    let complexity_filter = function
      | Mixed
      | Primitive
      | Alias
      | Newtype
      | TypeConst
      | Case
      | Enum ->
        true
      | Option
      | Classish
      | Container
      | Tuple
      | Shape
      | Awaitable
      | Function
      | Like ->
        complexity > 0
    in
    let kinds =
      List.filter ~f:(fun ty -> kind_filter ty && complexity_filter ty) kinds
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

  val inhabitant_of : ReadOnlyEnvironment.t -> Environment.t -> t -> string

  val mk : ReadOnlyEnvironment.t -> Environment.t -> Environment.t * t
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
    | Alias of { name: string }
    | Newtype of { name: string }
    | TypeConst of { name: string }
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

  let show_tys tys = List.map ~f:show tys |> String.concat ~sep:", "

  (** Generate an expression if the type is immediately inhabited.

      This function needs to be deterministic in whether it turns `Some` or
      `None` as this information is used first in the `subtype_of` relation to
      determine if the type is inhabited and later to actually generate the
      expression.

      TODO: if this turns out to cause slowness make it return `string lazy
      option` so that it is cheaper when used within the `subtype_of` check. *)
  let rec expr_of = function
    | Primitive prim -> begin
      let open Primitive in
      match prim with
      | Null -> Some "null"
      | Int -> Some "42"
      | String -> Some "'apple'"
      | Float -> Some "42.0"
      | Bool -> Some "true"
      | Arraykey
      | Num ->
        None
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
    | Tuple { conjuncts; open_ } ->
      if open_ then
        None
      else
        List.map ~f:expr_of conjuncts
        |> Option.all
        |> Option.map ~f:(fun exprl ->
               String.concat ~sep:", " exprl |> Format.sprintf "tuple(%s)")
    | Shape { fields; open_ = _ } -> begin
      (* Check that all types are inhabited even if we won't end up using all of them. *)
      match List.map fields ~f:(fun { ty; _ } -> expr_of ty) |> Option.all with
      | None -> None
      | Some _ ->
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
    end
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
      Format.sprintf
        "(%s%s): %s ==> { return %s; }"
        parameters
        variadic
        (Type.show return_)
        return_expr
    | Mixed
    | Option _
    | Alias _
    | Newtype _
    | TypeConst _
    | Case _
    | Like _ ->
      None

  let ty_filter
      ReadOnlyEnvironment.
        {
          pick_immediately_inhabited;
          for_option_ty;
          for_reified_ty;
          for_alias_def;
          for_enum_def;
          _;
        }
      ty =
    ((not pick_immediately_inhabited) || expr_of ty |> Option.is_some)
    &&
    match ty with
    | Case _ -> not (for_option_ty || for_enum_def)
    | Function _ -> not for_reified_ty
    | TypeConst _ -> not for_alias_def
    | _ -> true

  exception Backtrack

  (** Goes on a backtracking stochastic walk to pick an inhabited subtype of the
      given type.

      Termination of this function crucially relies on the invariant that EVERY
      input type has a subtype that satisfies the constraints set in the read
      only environment.

      For example, when pick_immediately_inhabited is set, the input type must
      have some subtype that is inhabited, e.g., if one passes an abstract class
      without a concrete class extending it somewhere down the hierarchy.
      *)
  let subtype_of (renv : ReadOnlyEnvironment.t) (env : Environment.t) ty =
    ReadOnlyEnvironment.debug
      ~level:1
      renv
      ~start:(lazy (Format.sprintf "subtype_of: %s" (Type.show ty)))
      ~end_:show
    @@ fun renv ->
    (* select_step makes sure we don't get into infinite loops by randomly being
       an identity function and otherwise recursive via step. *)
    let rec subfield_of renv { key; ty; optional } =
      ReadOnlyEnvironment.debug
        ~level:3
        renv
        ~start:(lazy (Format.sprintf "subfield_of %s: %s" key (Type.show ty)))
        ~end_:show_field
      @@ fun renv ->
      let ty =
        driver ReadOnlyEnvironment.{ renv with for_alias_def = false } ty
      in
      let optional =
        if optional then
          select [true; false]
        else
          false
      in
      { key; ty; optional }
    and step renv ty =
      ReadOnlyEnvironment.debug
        ~level:3
        renv
        ~start:(lazy (Format.sprintf "step: %s" (Type.show ty)))
        ~end_:show_tys
      @@ fun renv ->
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
             let ty =
               driver ReadOnlyEnvironment.{ renv with for_alias_def = false } ty
             in
             [Awaitable ty]
           | TypeConst _
           | Newtype _
           | Alias _
           | Classish _
           | Case _
           | Enum _ ->
             (* These cases either have no proper subtypes (e.g., Enum) or their
                subtypes are covered by the subtyping relationship in the
                environment. *)
             []
           | Vec ty ->
             let renv =
               ReadOnlyEnvironment.
                 {
                   renv with
                   pick_immediately_inhabited = false;
                   for_alias_def = false;
                 }
             in
             let ty = driver renv ty in
             [Vec ty]
           | Dict { key; value } ->
             let renv =
               ReadOnlyEnvironment.
                 {
                   renv with
                   pick_immediately_inhabited = false;
                   for_alias_def = false;
                 }
             in
             let key = driver renv key in
             let value = driver renv value in
             [Dict { key; value }]
           | Keyset ty ->
             let renv =
               ReadOnlyEnvironment.
                 {
                   renv with
                   pick_immediately_inhabited = false;
                   for_alias_def = false;
                 }
             in
             let ty = driver renv ty in
             [Keyset ty]
           | Tuple { conjuncts; open_ } ->
             let conjuncts =
               List.map
                 ~f:
                   (driver
                      ReadOnlyEnvironment.{ renv with for_alias_def = false })
                 conjuncts
             in
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
             let fields = List.map ~f:(subfield_of renv) fields in
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
             let return_ =
               driver
                 ReadOnlyEnvironment.{ renv with for_alias_def = false }
                 return_
             in
             let variadic =
               match variadic with
               | None ->
                 Lazy.force
                 @@ select
                      [
                        lazy None;
                        lazy
                          (Some
                             (driver
                                ReadOnlyEnvironment.
                                  {
                                    renv with
                                    pick_immediately_inhabited = false;
                                    for_alias_def = false;
                                  }
                                Mixed));
                      ]
               | Some ty -> Some ty
             in
             [Function { parameters; variadic; return_ }]
           | Like ty ->
             (* TODO: dynamic here when it is supported *)
             [driver renv ty]
         end
    and driver renv candidate =
      try
        ReadOnlyEnvironment.debug
          ~level:2
          renv
          ~start:(lazy (Format.sprintf "driver: %s" (Type.show candidate)))
          ~end_:show
        @@ fun renv ->
        let candidates = step renv candidate in
        if Random.bool () then
          (* Go down on a stochastic walk and explore further subtypes. *)
          driver renv @@ select candidates
        else
          let viable_candidates = List.filter ~f:(ty_filter renv) candidates in
          if List.is_empty viable_candidates then
            (* We don't have any choices left after filtering! So try a different
               path either by going down or going up the tree. *)
            if Random.bool () then
              driver renv @@ select candidates
            else
              raise Backtrack
          else
            select viable_candidates
      with
      | Backtrack ->
        (* Either keep backtracking or explore alternative paths from this point
           downwards. *)
        if Random.bool () then
          driver renv candidate
        else
          raise Backtrack
    in
    let rec retry () =
      try driver renv ty with
      | Backtrack -> retry ()
    in
    retry ()

  let are_disjoint (renv : ReadOnlyEnvironment.t) (env : Environment.t) ty ty' =
    ReadOnlyEnvironment.debug
      ~level:1
      renv
      ~start:
        (lazy
          (Format.sprintf "are_disjoint: %s %s" (Type.show ty) (Type.show ty')))
      ~end_:string_of_bool
    @@ fun renv ->
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
      ReadOnlyEnvironment.debug
        ~level:2
        renv
        ~start:
          (lazy (Format.sprintf "weaken_for_disjointness: %s" (Type.show ty)))
        ~end_:show_tys
      @@ fun renv ->
      let step ty =
        ReadOnlyEnvironment.debug
          ~level:4
          renv
          ~start:
            (lazy
              (Format.sprintf "weaken_for_disjointness step: %s" (Type.show ty)))
          ~end_:show_tys
        @@ fun _renv ->
        match ty with
        | Classish info when Kind.equal_classish info.kind Kind.Interface ->
          (* This can be improved on if we introduce an internal Object type which
             is still disjoint to non classish types. *)
          [Mixed]
        | Classish _
        | Alias _
        | TypeConst _
        | Newtype _
        | Case _ ->
          Environment.get_subtypes env ty
        | Option ty -> [Primitive Primitive.Null; ty]
        | Awaitable _ -> [Awaitable Mixed]
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
        | Like _ -> [Mixed]
      in
      let rec driver acc =
        ReadOnlyEnvironment.debug
          ~level:3
          renv
          ~start:
            (lazy
              (Format.sprintf "driver: %s" (TypeSet.to_list acc |> show_tys)))
          ~end_:(Fn.compose show_tys TypeSet.to_list)
        @@ fun _renv ->
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

  let inhabitant_of
      (renv : ReadOnlyEnvironment.t) (env : Environment.t) (ty : t) =
    let renv =
      ReadOnlyEnvironment.{ renv with pick_immediately_inhabited = true }
    in
    let subtype = subtype_of renv env ty in
    let inhabitant = expr_of subtype in
    match inhabitant with
    | Some inhabitant -> inhabitant
    | None ->
      raise
      @@ Failure
           ("Tried to find an inhabitant for a type: "
           ^ show ty
           ^ " but it is uninhabitaed. This indicates bug in `milner`.")

  let mk_arraykey (renv : ReadOnlyEnvironment.t) (env : Environment.t) =
    let renv =
      ReadOnlyEnvironment.{ renv with pick_immediately_inhabited = false }
    in
    subtype_of renv env (Primitive Primitive.Arraykey)

  let rec mk_classish
      (renv : ReadOnlyEnvironment.t)
      (env : Environment.t)
      ~(parent : (Kind.classish * string * generic option) option)
      ~(complexity : int)
      ~(depth : int) =
    ReadOnlyEnvironment.debug
      ~level:2
      renv
      ~start:(lazy "mk_classish")
      ~end_:(fun (_, ty) -> show ty)
    @@ fun renv ->
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
      let (kind, name, generic) = parent in
      let super = Classish { kind; name; generic } in
      List.init n ~f:(fun _ -> ())
      |> List.fold ~init:env ~f:(fun env _ ->
             let parent = Some parent in
             let depth = depth + 1 in
             let (env, child) =
               mk_classish renv env ~parent ~complexity ~depth
             in
             Environment.record_subtype env ~super ~sub:child)
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
          let renv =
            ReadOnlyEnvironment.
              {
                renv with
                for_option_ty = false;
                for_reified_ty =
                  is_reified || renv.ReadOnlyEnvironment.for_reified_ty;
                for_alias_def = false;
                for_enum_def = false;
              }
          in
          mk renv env ~complexity ~depth:(Some depth)
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

  and mk
      (renv : ReadOnlyEnvironment.t)
      (env : Environment.t)
      ~(complexity : int)
      ~(depth : int option) : Environment.t * t =
    let depth = Option.value ~default:0 depth in
    let kind = Kind.pick ~complexity renv in
    let mk ?(for_alias_def = false) renv env =
      mk ReadOnlyEnvironment.{ renv with for_alias_def } env ~depth:(Some depth)
    in
    let subtype_of ?(for_alias_def = false) renv env =
      subtype_of ReadOnlyEnvironment.{ renv with for_alias_def } env
    in
    ReadOnlyEnvironment.debug
      ~level:1
      renv
      ~start:(lazy (Format.sprintf "mk %s" (Kind.show kind)))
      ~end_:(fun (_, ty) -> show ty)
    @@ fun renv ->
    match kind with
    | Kind.Mixed -> (env, Mixed)
    | Kind.Primitive -> (env, Primitive (Primitive.pick ()))
    | Kind.Option ->
      let rec candidate () =
        match
          mk
            ~complexity:(complexity - 1)
            ReadOnlyEnvironment.{ renv with for_option_ty = true }
            env
        with
        | (_, Mixed) -> candidate ()
        | (_, Option _) as res ->
          res
          (* Due to some misguided checks the parser and the typechecker has. We
             need to eliminate these cases. *)
        | (env, ty) -> (env, Option ty)
      in
      candidate ()
    | Kind.Awaitable ->
      let (env, ty) =
        mk
          ~complexity:(complexity - 1)
          ReadOnlyEnvironment.{ renv with for_option_ty = false }
          env
      in
      (env, Awaitable ty)
    | Kind.Classish -> mk_classish renv env ~parent:None ~complexity ~depth
    | Kind.Alias ->
      let name = fresh "A" in
      let ty = Alias { name } in
      let (env, aliased) =
        mk ~complexity:default_complexity renv env ~for_alias_def:true
      in
      let env = Environment.record_subtype env ~super:ty ~sub:aliased in
      let env =
        Environment.add_definition env @@ Definition.alias ~name aliased
      in
      (env, ty)
    | Kind.Newtype ->
      let name = fresh "N" in
      let ty = Newtype { name } in
      let (env, aliased, bound) =
        if Random.bool () then
          let (env, bound) = mk ~complexity:default_complexity renv env in
          let aliased = subtype_of ~for_alias_def:true renv env bound in
          (env, aliased, Some bound)
        else
          let (env, aliased) =
            mk ~complexity:default_complexity ~for_alias_def:true renv env
          in
          (env, aliased, None)
      in
      let env = Environment.record_subtype env ~super:ty ~sub:aliased in
      let env =
        Environment.add_definition env
        @@ Definition.newtype ~name ~bound aliased
      in
      (env, ty)
    | Kind.TypeConst ->
      let tc_name = fresh "TC" in
      let (env, aliased) = mk ~complexity:default_complexity renv env in
      let typeconst_def = Definition.typeconst ~name:tc_name aliased in
      let class_name = fresh "CTC" in
      let qualified_name = Format.sprintf "%s::%s" class_name tc_name in
      let ty = TypeConst { name = qualified_name } in
      let env = Environment.record_subtype env ~super:ty ~sub:aliased in
      let env =
        Environment.add_definition env
        @@ Definition.classish
             ~name:class_name
             ~parent:None
             ~generic:None
             ~members:[typeconst_def]
             Kind.Class
      in
      (env, TypeConst { name = qualified_name })
    | Kind.Case ->
      let name = fresh "CT" in
      let (env, bound) =
        if Random.bool () then
          let (env, bound) = mk ~complexity:default_complexity renv env in
          (env, Some bound)
        else
          (env, None)
      in
      let mk renv env =
        match bound with
        | Some bound ->
          let ty = subtype_of renv env ~for_alias_def:true bound in
          (env, ty)
        | None -> mk ~complexity:default_complexity renv env ~for_alias_def:true
      in
      let rec add_disjuncts (env, disjuncts) =
        if Random.bool () then
          let (env, disjunct) = mk renv env in
          if List.for_all disjuncts ~f:(are_disjoint renv env disjunct) then
            add_disjuncts @@ (env, disjunct :: disjuncts)
          else
            add_disjuncts (env, disjuncts)
        else
          (env, disjuncts)
      in
      let (env, ty) = mk renv env in
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
          let bound =
            mk_arraykey
              ReadOnlyEnvironment.{ renv with for_enum_def = true }
              env
          in
          let underlying_ty =
            subtype_of
              ReadOnlyEnvironment.{ renv with for_enum_def = true }
              env
              bound
          in
          let value = inhabitant_of renv env underlying_ty in
          let env = Environment.record_subtype env ~super:bound ~sub:ty in
          (env, Some bound, underlying_ty, value)
        else
          let underlying_ty =
            mk_arraykey
              ReadOnlyEnvironment.{ renv with for_enum_def = true }
              env
          in
          let value = inhabitant_of renv env underlying_ty in
          let env = Environment.record_subtype env ~super:Mixed ~sub:ty in
          (env, None, underlying_ty, value)
      in
      let env =
        Environment.add_definition env
        @@ Definition.enum ~name ~bound underlying_ty ~value
      in
      (env, ty)
    | Kind.Container -> begin
      let renv = ReadOnlyEnvironment.{ renv with for_option_ty = false } in
      match Container.pick () with
      | Container.Vec ->
        let (env, ty) = mk ~complexity:(complexity - 1) renv env in
        (env, Vec ty)
      | Container.Dict ->
        let key = mk_arraykey renv env in
        let (env, value) = mk ~complexity:(complexity - 1) renv env in
        (env, Dict { key; value })
      | Container.Keyset ->
        let ty = mk_arraykey renv env in
        (env, Keyset ty)
    end
    | Kind.Tuple ->
      (* Sadly, although nullary tuples can be generated with an expression,
         there is no corresponding denotable type. *)
      let n = geometric_between min_tuple_arity max_tuple_arity in
      let renv = ReadOnlyEnvironment.{ renv with for_option_ty = false } in
      let (env, conjuncts) =
        List.init n ~f:(fun _ -> ())
        |> List.fold_map ~init:env ~f:(fun env _ ->
               mk ~complexity:(complexity - 1) renv env)
      in
      (env, Tuple { conjuncts; open_ = false })
    | Kind.Shape ->
      let keys = choose_nondet shape_keys in
      let renv = ReadOnlyEnvironment.{ renv with for_option_ty = false } in
      let mk_field env key =
        let (env, ty) = mk ~complexity:(complexity - 1) renv env in
        let optional = Random.bool () in
        (env, { key; optional; ty })
      in
      let (env, fields) = List.fold_map ~init:env ~f:mk_field keys in
      let open_ = Random.bool () in
      (env, Shape { fields; open_ })
    | Kind.Function ->
      let renv = ReadOnlyEnvironment.{ renv with for_option_ty = false } in
      let (env, parameters) =
        List.init (geometric_between 0 3) ~f:(fun _ -> ())
        |> List.fold_map ~init:env ~f:(fun env _ ->
               mk ~complexity:(complexity - 1) renv env)
      in
      let (env, return_) = mk ~complexity:(complexity - 1) renv env in
      let (env, variadic) =
        if Random.bool () then
          (env, None)
        else
          let (env, ty) = mk ~complexity:(complexity - 1) renv env in
          (env, Some ty)
      in
      (env, Function { parameters; variadic; return_ })
    | Kind.Like ->
      let (env, ty) = mk ~complexity:(complexity - 1) renv env in
      (env, Like ty)

  let mk = mk ~depth:None ~complexity:default_complexity
end

and TypeMap : (WrappedMap.S with type key = Type.t) = WrappedMap.Make (Type)
and TypeSet : (Stdlib.Set.S with type elt = Type.t) = Stdlib.Set.Make (Type)
