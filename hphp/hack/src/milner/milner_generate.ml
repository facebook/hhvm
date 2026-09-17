(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Milner_syntax

let default_complexity = 5

let max_hierarchy_depth = 2

let max_branching_factor = 2

let min_tuple_arity = 0

let max_tuple_arity = 3

let max_container_length = 3

let shape_keys = ["'a'"; "'b'"; "'c'"]

let name_ctr = ref 0

let fresh_id () =
  let n = !name_ctr in
  name_ctr := !name_ctr + 1;
  n

let fresh prefix = prefix ^ "_" ^ string_of_int (fresh_id ())

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

  val for_alias : t -> t

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

  let for_alias renv = { renv with for_alias_def = true }

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
  type member_contract = {
    value_type: Type.t;
    property: string;
    getter: string;
    setter: string;
    probe: string;
    reader: string;
    writer: string;
    read_function: string;
    write_function: string;
  }

  type nominal_info = {
    constructor: Type.t list;
        (** Closed parameter types whose definitions precede this hierarchy. *)
    contract: member_contract;
    dispatch: string;
    identity: string;
    probe_value: int;
  }

  type t = {
    definitions: Definition.t list;
    subtypes: Type.t list TypeMap.t;
    typedef_bodies: Type.t list TypeMap.t;
    case_bounds: Type.t TypeMap.t;
    nominals: nominal_info S_map.t;
    generic_families: string list;
  }

  val default : t

  val add_definition : t -> Definition.t -> t

  val definitions : t -> Definition.t list

  val record_subtype : t -> super:Type.t -> sub:Type.t -> t

  val get_subtypes : t -> Type.t -> Type.t list

  val record_typedef_body : t -> ty:Type.t -> body:Type.t list -> t

  val get_typedef_body : t -> Type.t -> Type.t list option

  val record_case_bound : t -> ty:Type.t -> bound:Type.t -> t

  val get_case_bound : t -> Type.t -> Type.t option

  val add_nominal : t -> name:string -> nominal_info -> t

  val get_nominal : t -> string -> nominal_info

  val add_generic_family : t -> string -> t

  val generic_families : t -> string list
end = struct
  type member_contract = {
    value_type: Type.t;
    property: string;
    getter: string;
    setter: string;
    probe: string;
    reader: string;
    writer: string;
    read_function: string;
    write_function: string;
  }

  type nominal_info = {
    constructor: Type.t list;
    contract: member_contract;
    dispatch: string;
    identity: string;
    probe_value: int;
  }

  type t = {
    definitions: Definition.t list;
    subtypes: Type.t list TypeMap.t;
    typedef_bodies: Type.t list TypeMap.t;
    case_bounds: Type.t TypeMap.t;
    nominals: nominal_info S_map.t;
    generic_families: string list;
  }

  let default =
    {
      definitions = [];
      subtypes = TypeMap.empty;
      typedef_bodies = TypeMap.empty;
      case_bounds = TypeMap.empty;
      nominals = S_map.empty;
      generic_families = [];
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

  let record_typedef_body env ~ty ~body =
    { env with typedef_bodies = TypeMap.add ty body env.typedef_bodies }

  let get_typedef_body env ty = TypeMap.find_opt ty env.typedef_bodies

  let record_case_bound env ~ty ~bound =
    { env with case_bounds = TypeMap.add ty bound env.case_bounds }

  let get_case_bound env ty = TypeMap.find_opt ty env.case_bounds

  let add_nominal env ~name info =
    { env with nominals = S_map.add name info env.nominals }

  let get_nominal env name = S_map.find name env.nominals

  let add_generic_family env name =
    { env with generic_families = name :: env.generic_families }

  let generic_families env = env.generic_families
end

and Kind : sig
  type t =
    | Mixed
    | Primitive
    | Option
    | Classish
    | GenericClass
    | Alias
    | Newtype
    | TypeConst
    | Case
    | Enum
    | Container
    | BuiltinContainer
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
    | GenericClass
    | Alias
    | Newtype
    | TypeConst
    | Case
    | Enum
    | Container
    | BuiltinContainer
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
      | GenericClass
      | Container
      | BuiltinContainer
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
    interfaces:string list ->
    generic:Type.generic option ->
    members:t list ->
    Kind.classish ->
    t

  val member_interfaces : Environment.member_contract -> t list

  val stateful_members :
    Environment.member_contract ->
    inherited:bool ->
    omit_getter_override:bool ->
    identity:string ->
    probe_value:int ->
    t list

  val member_trait :
    Environment.member_contract ->
    name:string ->
    parent:(Kind.classish * string * Type.generic option) option ->
    dispatch:string ->
    t

  val use_trait : string -> t

  val generic_family : name:string -> t list

  val alias : name:string -> Type.t -> t

  val newtype : name:string -> bound:Type.t option -> Type.t -> t

  val case_type : name:string -> bound:Type.t option -> Type.t list -> t

  val enum : name:string -> bound:Type.t option -> Type.t -> value:string -> t
end = struct
  type t = string

  let show def = def

  let typeconst ~name aliased =
    Format.sprintf "const type %s = %s;" name (Type.show aliased)

  let show_application name generic =
    match generic with
    | Some generic ->
      Format.sprintf "%s<%s>" name Type.(show generic.instantiation)
    | None -> name

  let classish
      ~name
      ~(parent : (Kind.classish * string * Type.generic option) option)
      ~interfaces
      ~(generic : Type.generic option)
      ~(members : t list)
      kind =
    let (extends, implements) =
      match (kind, parent) with
      | (Kind.Interface, Some (Kind.Interface, name, generic)) ->
        (show_application name generic :: interfaces, [])
      | (Kind.Interface, None) -> (interfaces, [])
      | (_, Some (Kind.Interface, name, generic)) ->
        ([], show_application name generic :: interfaces)
      | (_, Some (_, name, generic)) ->
        ([show_application name generic], interfaces)
      | (_, None) -> ([], interfaces)
    in
    let clause keyword names =
      if List.is_empty names then
        ""
      else
        Format.sprintf "%s %s " keyword (String.concat ~sep:", " names)
    in
    let parent = clause "extends" extends ^ clause "implements" implements in
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

  let member_interfaces
      Environment.
        {
          value_type;
          getter;
          setter;
          reader;
          writer;
          read_function;
          write_function;
          property = _;
          probe = _;
        } =
    let value_type = Type.show value_type in
    [
      Format.sprintf
        "interface %s { public function %s()[]: %s; }"
        reader
        getter
        value_type;
      Format.sprintf
        "interface %s { public function %s(%s $value)[write_props]: void; }"
        writer
        setter
        value_type;
      Format.sprintf
        "function %s(%s $reader)[]: %s { return $reader->%s(); }"
        read_function
        reader
        value_type
        getter;
      Format.sprintf
        "function %s(%s $writer, %s $value)[write_props]: void { $writer->%s($value); }"
        write_function
        writer
        value_type
        setter;
    ]

  let stateful_members
      Environment.{ value_type; property; getter; setter; probe; _ }
      ~inherited
      ~omit_getter_override
      ~identity
      ~probe_value =
    let value_type = Type.show value_type in
    let stateful =
      if inherited && omit_getter_override then
        []
      else if inherited then
        [
          Format.sprintf
            "<<__Override>> public function %s()[]: %s { return parent::%s(); }"
            getter
            value_type
            getter;
        ]
      else
        [
          Format.sprintf
            "public function __construct(protected %s $%s)[write_props] {}"
            value_type
            property;
          Format.sprintf
            "public function %s()[]: %s { return $this->%s; }"
            getter
            value_type
            property;
          Format.sprintf
            "public function %s(%s $value)[write_props]: void { $this->%s = $value; }"
            setter
            value_type
            property;
        ]
    in
    stateful
    @ [
        Format.sprintf
          "%spublic function %s()[]: int { return %d; }"
          (if inherited then
            "<<__Override>> "
          else
            "")
          probe
          probe_value;
        Format.sprintf
          "public static function %s(%s $value)[]: %s { return $value; }"
          identity
          value_type
          value_type;
      ]

  let member_trait
      Environment.{ value_type; getter; reader; _ } ~name ~parent ~dispatch =
    let parent =
      match parent with
      | Some (Kind.(Class | AbstractClass), name, generic) ->
        Format.sprintf "require extends %s; " (show_application name generic)
      | Some (Kind.Interface, _, _)
      | None ->
        ""
    in
    Format.sprintf
      "trait %s { %srequire implements %s; public function %s()[]: %s { return $this->%s(); } }"
      name
      parent
      reader
      dispatch
      (Type.show value_type)
      getter

  let use_trait name = Format.sprintf "use %s;" name

  let generic_family ~name =
    [
      Format.sprintf
        "interface %sReader<+T> { public function get()[]: T; }"
        name;
      Format.sprintf
        "interface %sWriter<-T> { public function set(T $value)[write_props]: void; }"
        name;
      Format.sprintf
        "final class %s<TKey as arraykey, TValue> implements %sReader<TValue>, %sWriter<TValue> { public function __construct(private TKey $key, private TValue $value)[write_props] {} public function get()[]: TValue { return $this->value; } public function set(TValue $value)[write_props]: void { $this->value = $value; } public function keyed()[]: dict<TKey, TValue> { return dict[$this->key => $this->value]; } public function project<TProjected as TValue>(TProjected $value)[]: TProjected { return $value; } public function widen<TWide super TValue>(TWide $_witness)[write_props]: %s<TKey, TWide> { return new %s<TKey, TWide>($this->key, $this->value); } public static function identity<TItem>(TItem $value)[]: TItem { return $value; } }"
        name
        name
        name
        name
        name;
      Format.sprintf
        "function %s_read(%sReader<mixed> $reader)[]: mixed { return $reader->get(); }"
        name
        name;
      Format.sprintf
        "function %s_write<TWrite>(%sWriter<TWrite> $writer, TWrite $value)[write_props]: void { $writer->set($value); }"
        name
        name;
    ]

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

  val intersection_law_compatible :
    Environment.t -> t -> Environment.t -> t -> bool

  val inhabitant_of : ReadOnlyEnvironment.t -> Environment.t -> t -> string

  val subtype_of : ReadOnlyEnvironment.t -> Environment.t -> t -> t

  val mk : ReadOnlyEnvironment.t -> Environment.t -> Environment.t * t

  type generic_witness = {
    generic_family: string;
    generic_key: t;
    generic_payload: t;
    generic_narrow: t;
    generic_class: t;
    generic_wide: t;
    generic_reader: t;
    generic_writer: t;
    generic_tagged_class: t;
    generic_tagged_writer: t;
  }

  val mk_generic_witness :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    value:t ->
    Environment.t * generic_witness

  val hierarchy_bindings :
    ReadOnlyEnvironment.t ->
    Environment.t ->
    t ->
    Environment.t * (string * string) list
end = struct
  module Env = Environment
  module REnv = ReadOnlyEnvironment

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
    | GenericClass of {
        name: string;
        key: t;
        value: t;
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
    | Traversable of t
    | ContainerInterface of t
    | Iterator of t
    | KeyedTraversable of {
        key: t;
        value: t;
      }
    | KeyedContainer of {
        key: t;
        value: t;
      }
    | KeyedIterator of {
        key: t;
        value: t;
      }
    | VecOrDict of {
        key: t;
        value: t;
      }
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
    | GenericClass { name; key; value } ->
      Format.sprintf "%s<%s, %s>" name (show key) (show value)
    | Alias info -> info.name
    | Newtype info -> info.name
    | TypeConst info -> info.name
    | Case info -> info.name
    | Enum info -> info.name
    | Vec ty -> Format.sprintf "vec<%s>" (show ty)
    | Dict { key; value } ->
      Format.sprintf "dict<%s, %s>" (show key) (show value)
    | Keyset ty -> Format.sprintf "keyset<%s>" (show ty)
    | Traversable ty -> Format.sprintf "Traversable<%s>" (show ty)
    | ContainerInterface ty -> Format.sprintf "Container<%s>" (show ty)
    | Iterator ty -> Format.sprintf "Iterator<%s>" (show ty)
    | KeyedTraversable { key; value } ->
      Format.sprintf "KeyedTraversable<%s, %s>" (show key) (show value)
    | KeyedContainer { key; value } ->
      Format.sprintf "KeyedContainer<%s, %s>" (show key) (show value)
    | KeyedIterator { key; value } ->
      Format.sprintf "KeyedIterator<%s, %s>" (show key) (show value)
    | VecOrDict { key; value } ->
      if equal key (Primitive Primitive.Arraykey) then
        Format.sprintf "vec_or_dict<%s>" (show value)
      else
        Format.sprintf "vec_or_dict<%s, %s>" (show key) (show value)
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

  let intersection_law_compatible env1 ty1 env2 ty2 =
    (* T288868888: like/nullable intersection reordering can fail subtyping. *)
    let rec has_like_head env seen ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Like _ -> true
        | Tuple { conjuncts; _ } ->
          List.exists conjuncts ~f:(has_like_head env seen)
        | Shape { fields; _ } ->
          List.exists fields ~f:(fun { ty; _ } -> has_like_head env seen ty)
        | Alias _
        | Newtype _
        | TypeConst _ ->
          List.exists (Env.get_subtypes env ty) ~f:(has_like_head env seen)
        | _ -> false
    in
    let rec is_only_null env seen ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Primitive Primitive.Null -> true
        | Alias _
        | Newtype _
        | TypeConst _
        | Case _ ->
          let subtypes = Env.get_subtypes env ty in
          (not (List.is_empty subtypes))
          && List.for_all subtypes ~f:(is_only_null env seen)
        | _ -> false
    in
    let rec has_nullable_form env seen ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Option _ -> true
        | Tuple { conjuncts; _ } ->
          List.exists conjuncts ~f:(has_nullable_form env seen)
        | Shape { fields; _ } ->
          List.exists fields ~f:(fun { ty; _ } -> has_nullable_form env seen ty)
        | Alias _
        | Newtype _
        | TypeConst _ ->
          List.exists (Env.get_subtypes env ty) ~f:(has_nullable_form env seen)
        | Case _ ->
          let subtypes = Env.get_subtypes env ty in
          List.exists subtypes ~f:(has_nullable_form env seen)
          || List.exists subtypes ~f:(is_only_null env TypeSet.empty)
             && List.exists
                  subtypes
                  ~f:(Fn.non (is_only_null env TypeSet.empty))
        | _ -> false
    in
    let hazardous env_like ty_like env_nullable ty_nullable =
      has_like_head env_like TypeSet.empty ty_like
      && has_nullable_form env_nullable TypeSet.empty ty_nullable
    in
    let has_exposed_head env ty ~f =
      let rec visit seen ty =
        if TypeSet.mem ty seen then
          false
        else if f ty then
          true
        else
          let seen = TypeSet.add ty seen in
          match ty with
          | Alias _
          | Newtype _
          | TypeConst _ ->
            List.exists (Env.get_subtypes env ty) ~f:(visit seen)
          | _ -> false
      in
      visit TypeSet.empty ty
    in
    (* T288865283: case/nullable-function intersections can fail reflexivity. *)
    let case_function_hazard env_case ty_case env_function ty_function =
      has_exposed_head env_case ty_case ~f:(function
          | Case _ -> true
          | _ -> false)
      && has_exposed_head env_function ty_function ~f:(function
             | Option ty ->
               has_exposed_head env_function ty ~f:(function
                   | Function _ -> true
                   | _ -> false)
             | _ -> false)
    in
    let rec null_head env seen ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Primitive Primitive.Null -> true
        | Option inner -> null_head env seen inner
        | Alias _
        | Newtype _
        | TypeConst _ ->
          Option.exists (Env.get_typedef_body env ty) ~f:(function
              | [inner] -> null_head env seen inner
              | _ -> false)
        | _ -> false
    in
    let rec admits_null env seen ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Mixed
        | Option _
        | Like _
        | Primitive Primitive.Null ->
          true
        | Alias _
        | Newtype _
        | TypeConst _
        | Case _ ->
          Option.exists (Env.get_typedef_body env ty) ~f:(fun body ->
              List.exists body ~f:(admits_null env seen))
        | _ -> false
    in
    let rec known_nonnull env seen ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Primitive Primitive.Null -> false
        | Primitive _
        | Awaitable _
        | Classish _
        | Enum _
        | Vec _
        | Dict _
        | Keyset _
        | Traversable _
        | ContainerInterface _
        | Iterator _
        | KeyedTraversable _
        | KeyedContainer _
        | KeyedIterator _
        | VecOrDict _
        | Tuple _
        | Shape _
        | Function _ ->
          true
        | Like inner -> known_nonnull env seen inner
        | Alias _
        | Newtype _
        | TypeConst _ ->
          Option.exists (Env.get_typedef_body env ty) ~f:(function
              | [inner] -> known_nonnull env seen inner
              | _ -> false)
        | Case _ ->
          Option.exists (Env.get_case_bound env ty) ~f:(known_nonnull env seen)
        | _ -> false
    in
    (* T288865283: opaque case/null intersections can fail reflexivity. *)
    let case_null_hazard env_case ty_case env_null ty_null =
      let rec exposed_case seen ty =
        if TypeSet.mem ty seen then
          false
        else
          let seen = TypeSet.add ty seen in
          match ty with
          | Case _ ->
            (not (admits_null env_case TypeSet.empty ty))
            && not
                 (Option.exists
                    (Env.get_case_bound env_case ty)
                    ~f:(known_nonnull env_case TypeSet.empty))
          | Alias _
          | Newtype _
          | TypeConst _ ->
            Option.exists (Env.get_typedef_body env_case ty) ~f:(function
                | [inner] -> exposed_case seen inner
                | _ -> false)
          | _ -> false
      in
      null_head env_null TypeSet.empty ty_null
      && exposed_case TypeSet.empty ty_case
    in
    let rec case_variants env seen ty =
      if TypeSet.mem ty seen then
        None
      else
        let seen = TypeSet.add ty seen in
        match (ty, Env.get_typedef_body env ty) with
        | (Case _, Some (_ :: _ :: _ as variants)) -> Some variants
        | ((Alias _ | Newtype _ | TypeConst _ | Case _), Some [inner]) ->
          case_variants env seen inner
        | _ -> None
    in
    let rec has_nullable_head env seen ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Option _ -> true
        | Alias _
        | Newtype _
        | TypeConst _ ->
          Option.exists (Env.get_typedef_body env ty) ~f:(fun body ->
              List.exists body ~f:(has_nullable_head env seen))
        | _ -> false
    in
    (* T288865283 also affects nullable variants in multi-variant case types. *)
    let case_union_hazard env_case ty_case env_nullable ty_nullable =
      match
        ( case_variants env_case TypeSet.empty ty_case,
          case_variants env_nullable TypeSet.empty ty_nullable )
      with
      | (Some _, Some variants) ->
        List.exists variants ~f:(has_nullable_head env_nullable TypeSet.empty)
      | _ -> false
    in
    not
      (hazardous env1 ty1 env2 ty2
      || hazardous env2 ty2 env1 ty1
      || case_function_hazard env1 ty1 env2 ty2
      || case_function_hazard env2 ty2 env1 ty1
      || case_null_hazard env1 ty1 env2 ty2
      || case_null_hazard env2 ty2 env1 ty1
      || case_union_hazard env1 ty1 env2 ty2
      || case_union_hazard env2 ty2 env1 ty1)

  let rec is_immediately_inhabited = function
    | Primitive Primitive.(Null | Int | String | Float | Bool)
    | Classish { kind = Kind.Class; _ }
    | GenericClass _
    | Enum _
    | Vec _
    | Dict _
    | Keyset _
    | Traversable _
    | ContainerInterface _
    | Iterator _
    | KeyedTraversable _
    | KeyedContainer _
    | KeyedIterator _
    | VecOrDict _ ->
      true
    | Tuple { conjuncts; open_ } ->
      (not open_) && List.for_all conjuncts ~f:is_immediately_inhabited
    | Shape { fields; open_ = _ } ->
      List.for_all fields ~f:(fun { ty; _ } -> is_immediately_inhabited ty)
    | Awaitable ty
    | Function { return_ = ty; _ } ->
      is_immediately_inhabited ty
    | Primitive Primitive.(Arraykey | Num)
    | Classish { kind = Kind.(Interface | AbstractClass); _ }
    | Mixed
    | Option _
    | Alias _
    | Newtype _
    | TypeConst _
    | Case _
    | Like _ ->
      false

  let ty_filter
      REnv.
        {
          pick_immediately_inhabited;
          for_option_ty;
          for_reified_ty;
          for_alias_def;
          for_enum_def;
          _;
        }
      ty =
    ((not pick_immediately_inhabited) || is_immediately_inhabited ty)
    &&
    match ty with
    | Case _ -> not (for_option_ty || for_enum_def)
    | Function _ -> not for_reified_ty
    | TypeConst _ -> not for_alias_def
    | _ -> true

  exception Backtrack

  let admits_int_keys = function
    | Primitive Primitive.(Int | Arraykey) -> true
    | _ -> false

  let is_known_arraykey = function
    | Primitive Primitive.(Int | String | Arraykey)
    | Enum _ ->
      true
    | _ -> false

  let keyset_subtypes key value =
    if
      is_known_arraykey value
      && (equal key value || equal key (Primitive Primitive.Arraykey))
    then
      [Keyset value]
    else
      []

  (** Goes on a backtracking stochastic walk to pick an inhabited subtype of the
      given type.

      Termination of this function crucially relies on the invariant that EVERY
      input type has a subtype that satisfies the constraints set in the read
      only environment.

      For example, when pick_immediately_inhabited is set, the input type must
      have some subtype that is inhabited, e.g., if one passes an abstract class
      without a concrete class extending it somewhere down the hierarchy.
      *)
  let subtype_of (renv : REnv.t) (env : Env.t) ty =
    REnv.debug
      ~level:1
      renv
      ~start:(lazy (Format.sprintf "subtype_of: %s" (Type.show ty)))
      ~end_:show
    @@ fun renv ->
    (* select_step makes sure we don't get into infinite loops by randomly being
       an identity function and otherwise recursive via step. *)
    let rec subfield_of renv { key; ty; optional } =
      REnv.debug
        ~level:3
        renv
        ~start:(lazy (Format.sprintf "subfield_of %s: %s" key (Type.show ty)))
        ~end_:show_field
      @@ fun renv ->
      let ty = driver REnv.{ renv with for_alias_def = false } ty in
      let optional =
        if optional then
          select [true; false]
        else
          false
      in
      { key; ty; optional }
    and step renv ty =
      REnv.debug
        ~level:3
        renv
        ~start:(lazy (Format.sprintf "step: %s" (Type.show ty)))
        ~end_:show_tys
      @@ fun renv ->
      (* This choice point can early circuit a lot of recursive calls by
         selecting constant time subtypes half the time. This is either through
         reflexivity, subtyping environment, or types with fixed builtin subtypes. *)
      if Random.bool () then
        ty
        :: begin
             Env.get_subtypes env ty
             @
             match ty with
             | Mixed -> List.map ~f:(fun prim -> Primitive prim) Primitive.all
             | Primitive prim -> begin
               let open Primitive in
               match prim with
               | Arraykey -> [Primitive Int; Primitive String]
               | Num -> [Primitive Int; Primitive Float]
               | _ -> []
             end
             | _ -> []
           end
      else begin
        match ty with
        | Mixed
        | Primitive _
        | TypeConst _
        | GenericClass _
        | Newtype _
        | Alias _
        | Classish _
        | Case _
        | Enum _ ->
          []
        | Option ty ->
          (* The parser hates ??ty, so we don't return `Option (driver renv ty)`
             here *)
          [Primitive Primitive.Null; ty]
        | Like ty ->
          (* TODO: dynamic here when it is supported *)
          let ty = driver renv ty in
          [ty; Like ty]
        | Awaitable ty ->
          let ty = driver REnv.{ renv with for_alias_def = false } ty in
          [Awaitable ty]
        | Vec ty ->
          let renv =
            let open REnv in
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
            let open REnv in
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
            let open REnv in
            {
              renv with
              pick_immediately_inhabited = false;
              for_alias_def = false;
            }
          in
          let ty = driver renv ty in
          [Keyset ty]
        | Traversable value
        | ContainerInterface value
        | Iterator value ->
          let renv =
            REnv.
              {
                renv with
                pick_immediately_inhabited = false;
                for_alias_def = false;
              }
          in
          let value = driver renv value in
          let key = Primitive Primitive.Arraykey in
          begin
            match ty with
            | Traversable _ ->
              [
                Traversable value;
                ContainerInterface value;
                Iterator value;
                KeyedTraversable { key; value };
              ]
            | ContainerInterface _ ->
              [ContainerInterface value; KeyedContainer { key; value }]
              @ keyset_subtypes key value
            | _ -> [Iterator value; KeyedIterator { key; value }]
          end
        | KeyedTraversable { key; value }
        | KeyedContainer { key; value }
        | KeyedIterator { key; value }
        | VecOrDict { key; value } ->
          let renv =
            REnv.
              {
                renv with
                pick_immediately_inhabited = false;
                for_alias_def = false;
              }
          in
          let key = driver renv key in
          let value = driver renv value in
          let arrays =
            Dict { key; value }
            ::
            (if admits_int_keys key then
              [Vec value]
            else
              [])
          in
          begin
            match ty with
            | KeyedTraversable _ ->
              [
                KeyedTraversable { key; value };
                KeyedContainer { key; value };
                KeyedIterator { key; value };
              ]
            | KeyedContainer _ ->
              [KeyedContainer { key; value }; VecOrDict { key; value }]
              @ keyset_subtypes key value
              @ arrays
            | KeyedIterator _ -> [KeyedIterator { key; value }]
            | _ -> VecOrDict { key; value } :: arrays
          end
        | Tuple { conjuncts; open_ } ->
          let conjuncts =
            List.map
              ~f:(driver REnv.{ renv with for_alias_def = false })
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
            driver REnv.{ renv with for_alias_def = false } return_
          in
          let variadic =
            match variadic with
            | None ->
              let variadic_subtype =
                let renv =
                  let open REnv in
                  {
                    renv with
                    pick_immediately_inhabited = false;
                    for_alias_def = false;
                  }
                in
                lazy (Some (driver renv Mixed))
              in
              Lazy.force @@ select [lazy None; variadic_subtype]
            | Some ty -> Some ty
          in
          [Function { parameters; variadic; return_ }]
      end
    and driver renv candidate =
      try
        REnv.debug
          ~level:2
          renv
          ~start:(lazy (Format.sprintf "driver: %s" (Type.show candidate)))
          ~end_:show
        @@ fun renv ->
        let candidates = step renv candidate in
        if List.is_empty candidates then
          raise Backtrack
        else if Random.bool () then
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

  let are_disjoint (renv : REnv.t) (env : Env.t) ty ty' =
    REnv.debug
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

       Tuples and vecs share a runtime representation, so their weakenings
       must overlap even when the tuple arities differ.

       Although we don't have to keep the non-weakened types for disjointness
       checking, it makes termination of `weaken_for_disjointness` trivial, so
       we pay the price.
    *)
    let weaken_for_disjointness ty : t list =
      REnv.debug
        ~level:2
        renv
        ~start:
          (lazy (Format.sprintf "weaken_for_disjointness: %s" (Type.show ty)))
        ~end_:show_tys
      @@ fun renv ->
      let step ty =
        REnv.debug
          ~level:4
          renv
          ~start:
            (lazy
              (Format.sprintf "weaken_for_disjointness step: %s" (Type.show ty)))
          ~end_:show_tys
        @@ fun _renv ->
        match ty with
        | GenericClass { name; _ } ->
          (* Distinct applications of an erased generic share a runtime class. *)
          [Classish { kind = Kind.Class; name; generic = None }]
        | Classish info when Kind.equal_classish info.kind Kind.Interface ->
          (* This can be improved on if we introduce an internal Object type which
             is still disjoint to non classish types. *)
          [Mixed]
        | Classish _
        | Alias _
        | TypeConst _
        | Newtype _
        | Case _ ->
          Env.get_subtypes env ty
        | Option ty -> [Primitive Primitive.Null; ty]
        | Awaitable _ -> [Awaitable Mixed]
        | Enum _ -> Primitive.[Primitive Int; Primitive String]
        | Traversable _
        | ContainerInterface _
        | Iterator _
        | KeyedTraversable _
        | KeyedContainer _
        | KeyedIterator _ ->
          [Mixed]
        | VecOrDict _ ->
          [
            Vec Mixed; Dict { key = Primitive Primitive.Arraykey; value = Mixed };
          ]
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
        REnv.debug
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

  let rec inhabitant (renv : REnv.t) (env : Env.t) (ty : t) =
    let renv = REnv.{ renv with pick_immediately_inhabited = true } in
    let subtype = subtype_of renv env ty in
    let inhabitant = expr_of renv env subtype in
    match inhabitant with
    | Some inhabitant -> inhabitant
    | None ->
      raise
      @@ Failure
           ("Tried to find an inhabitant for a type: "
           ^ show ty
           ^ " but it is uninhabitaed. This indicates bug in `milner`.")

  and expr_of renv env ty =
    let open Milner_syntax in
    match ty with
    | Primitive prim -> begin
      let open Primitive in
      match prim with
      | Null -> Some (Atom "null")
      | Int -> Some (Atom (string_of_int (Random.int_incl (-2) 2)))
      | String -> Some (Atom (select ["''"; "'apple'"; "'pear'"]))
      | Float -> Some (Atom (select ["0.0"; "42.0"; "-1.0"]))
      | Bool -> Some (Atom (string_of_bool (Random.bool ())))
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
        let Env.{ constructor; _ } = Env.get_nominal env info.name in
        Some (New (show ty, List.map constructor ~f:(inhabitant renv env)))
    end
    | GenericClass { key; value; _ } as ty ->
      let key_expr = inhabitant renv env key in
      let value_expr = inhabitant renv env value in
      Some (Syntax.New (show ty, [key_expr; value_expr]))
    | Enum info -> Some (StaticMember (info.name, "A"))
    | Traversable value
    | ContainerInterface value ->
      Some (Array ("vec", [inhabitant renv env value]))
    | KeyedTraversable { key; value }
    | KeyedContainer { key; value }
    | VecOrDict { key; value } ->
      Some
        (Array
           ( "dict",
             [KeyValue (inhabitant renv env key, inhabitant renv env value)] ))
    | Iterator value ->
      Some
        (Call
           ( Member
               ( New
                   ( "Vector<" ^ show value ^ ">",
                     [Array ("vec", [inhabitant renv env value])] ),
                 "getIterator" ),
             [] ))
    | KeyedIterator { key; value } ->
      Some
        (Call
           ( Member
               ( New
                   ( "Map<" ^ show key ^ ", " ^ show value ^ ">",
                     [
                       Array
                         ( "dict",
                           [
                             KeyValue
                               ( inhabitant renv env key,
                                 inhabitant renv env value );
                           ] );
                     ] ),
                 "getIterator" ),
             [] ))
    | Vec ty ->
      let elements =
        List.init (geometric_between 0 max_container_length) ~f:(fun _ ->
            inhabitant renv env ty)
      in
      Some (Array ("vec", elements))
    | Dict { key; value } ->
      let fields =
        List.init (geometric_between 0 max_container_length) ~f:(fun _ ->
            KeyValue (inhabitant renv env key, inhabitant renv env value))
      in
      Some (Array ("dict", fields))
    | Keyset ty ->
      let elements =
        List.init (geometric_between 0 max_container_length) ~f:(fun _ ->
            inhabitant renv env ty)
      in
      Some (Array ("keyset", elements))
    | Tuple { conjuncts; open_ } ->
      if open_ then
        None
      else
        List.map ~f:(expr_of renv env) conjuncts
        |> Option.all
        |> Option.map ~f:(fun expressions -> Tuple expressions)
    | Shape { fields; open_ = _ } -> begin
      (* Check that all types are inhabited even if we won't end up using all of them. *)
      match
        List.map fields ~f:(fun { ty; _ } -> expr_of renv env ty) |> Option.all
      with
      | None -> None
      | Some _ ->
        let fields =
          List.filter fields ~f:(fun f -> (not f.optional) || Random.bool ())
        in
        let fields = List.permute fields in
        let show_field { key; ty; _ } =
          expr_of renv env ty |> Option.map ~f:(fun value -> (key, value))
        in
        List.map ~f:show_field fields
        |> Option.all
        |> Option.map ~f:(fun fields -> Shape fields)
    end
    | Awaitable ty ->
      let open Option.Let_syntax in
      let+ expr = expr_of renv env ty in
      Async [Return (Some expr)]
    | Function { parameters; variadic; return_ } ->
      let parameters =
        List.map parameters ~f:(fun ty -> (show ty, fresh_local "argument"))
        @ Option.to_list
            (Option.map variadic ~f:(fun ty ->
                 (show ty ^ " ...", fresh_local "rest")))
      in
      let open Option.Let_syntax in
      let+ return_expr = expr_of renv env return_ in
      Lambda
        (parameters, ["defaults"], show return_, [Return (Some return_expr)])
    | Mixed
    | Option _
    | Alias _
    | Newtype _
    | TypeConst _
    | Case _
    | Like _ ->
      None

  let inhabitant_of renv env ty =
    inhabitant renv env ty |> Milner_syntax.render_expr

  let mk_arraykey (renv : REnv.t) (env : Env.t) =
    let renv = REnv.{ renv with pick_immediately_inhabited = false } in
    subtype_of renv env (Primitive Primitive.Arraykey)

  let has_nullable_enum_case_return env ty =
    (* T288899890: identical nullable enum case returns can fail overriding. *)
    let rec visit seen ~inside_case ~nullable ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Alias _
        | Newtype _ ->
          (match Env.get_typedef_body env ty with
          | Some [body] -> visit seen ~inside_case ~nullable body
          | _ -> false)
        | Case _ ->
          (match Env.get_typedef_body env ty with
          | Some [body] -> visit seen ~inside_case:true ~nullable body
          | _ -> false)
        | Like ty -> visit seen ~inside_case ~nullable ty
        | Option ty when inside_case ->
          visit seen ~inside_case ~nullable:true ty
        | Enum _ -> inside_case && nullable
        | _ -> false
    in
    visit TypeSet.empty ~inside_case:false ~nullable:false ty

  type generic_witness = {
    generic_family: string;
    generic_key: t;
    generic_payload: t;
    generic_narrow: t;
    generic_class: t;
    generic_wide: t;
    generic_reader: t;
    generic_writer: t;
    generic_tagged_class: t;
    generic_tagged_writer: t;
  }

  let generic_protocol name suffix instantiation =
    Classish
      {
        name = name ^ suffix;
        kind = Kind.Interface;
        generic = Some { instantiation; is_reified = false };
      }

  let declare_generic env ~key ~value =
    let families = Env.generic_families env in
    let (env, name) =
      if (not (List.is_empty families)) && Random.bool () then
        (env, select families)
      else
        let name = fresh "Generic" in
        let env =
          List.fold
            (Definition.generic_family ~name)
            ~init:env
            ~f:Env.add_definition
        in
        (Env.add_generic_family env name, name)
    in
    let ty = GenericClass { name; key; value } in
    let reader = generic_protocol name "Reader" value in
    let writer = generic_protocol name "Writer" value in
    let env = Env.record_subtype env ~super:reader ~sub:ty in
    let env = Env.record_subtype env ~super:writer ~sub:ty in
    (env, name, ty, reader, writer)

  let make_member_contract env value_type =
    let contract =
      Env.
        {
          value_type;
          property = fresh "value";
          getter = fresh "get";
          setter = fresh "set";
          probe = fresh "probe";
          reader = fresh "Reader";
          writer = fresh "Writer";
          read_function = fresh "read";
          write_function = fresh "write";
        }
    in
    let env =
      List.fold
        (Definition.member_interfaces contract)
        ~init:env
        ~f:Env.add_definition
    in
    (env, contract)

  let rec mk_classish
      (renv : REnv.t)
      (env : Env.t)
      ~(parent : (Kind.classish * string * generic option) option)
      ~(contract : Env.member_contract option)
      ~(complexity : int)
      ~(depth : int) =
    REnv.debug
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
    let (env, contract) =
      match contract with
      | Some contract -> (env, contract)
      | None ->
        let (env, value_type) =
          if depth >= max_hierarchy_depth then
            (env, Primitive (Primitive.pick ()))
          else
            mk
              REnv.{ renv with for_option_ty = false; for_alias_def = false }
              env
              ~complexity:(complexity - 1)
              ~depth:(Some (depth + 1))
        in
        make_member_contract env value_type
    in
    let gen_children env ~parent n =
      let (kind, name, generic) = parent in
      let super = Classish { kind; name; generic } in
      List.init n ~f:(fun _ -> ())
      |> List.fold ~init:env ~f:(fun env _ ->
             let parent = Some parent in
             let depth = depth + 1 in
             let (env, child) =
               mk_classish
                 renv
                 env
                 ~parent
                 ~contract:(Some contract)
                 ~complexity
                 ~depth
             in
             Env.record_subtype env ~super ~sub:child)
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
            let open REnv in
            {
              renv with
              for_option_ty = false;
              for_reified_ty = is_reified || renv.for_reified_ty;
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
    let ty = Classish { kind; name; generic } in
    let interfaces = Env.[contract.reader; contract.writer] in
    let env =
      List.fold interfaces ~init:env ~f:(fun env name ->
          Env.record_subtype
            env
            ~super:(Classish { kind = Kind.Interface; name; generic = None })
            ~sub:ty)
    in
    let (env, members) =
      match kind with
      | Kind.Interface -> (env, [])
      | Kind.Class
      | Kind.AbstractClass ->
        let trait_name = fresh "Trait" in
        let dispatch = fresh "dispatch" in
        let identity = fresh "identity" in
        let probe_value = fresh_id () in
        let info =
          Env.
            {
              constructor = [contract.value_type];
              contract;
              identity;
              probe_value;
              dispatch;
            }
        in
        let env = Env.add_nominal env ~name info in
        let env =
          Env.add_definition env
          @@ Definition.member_trait contract ~name:trait_name ~parent ~dispatch
        in
        let inherited =
          match parent with
          | Some (Kind.(Class | AbstractClass), _, _) -> true
          | Some (Kind.Interface, _, _)
          | None ->
            false
        in
        let members =
          Definition.use_trait trait_name
          :: Definition.stateful_members
               contract
               ~inherited
               ~omit_getter_override:
                 (has_nullable_enum_case_return env contract.Env.value_type)
               ~identity
               ~probe_value
        in
        (env, members)
    in
    let env = gen_children env ~parent:(kind, name, generic) num_of_children in
    let env =
      Env.add_definition env
      @@ Definition.classish kind ~name ~parent ~interfaces ~generic ~members
    in
    (env, ty)

  and mk (renv : REnv.t) (env : Env.t) ~(complexity : int) ~(depth : int option)
      : Env.t * t =
    let depth = Option.value ~default:0 depth in
    let kind = Kind.pick ~complexity renv in
    let mk ?(for_alias_def = false) renv env =
      mk REnv.{ renv with for_alias_def } env ~depth:(Some depth)
    in
    let subtype_of ?(for_alias_def = false) renv env =
      subtype_of REnv.{ renv with for_alias_def } env
    in
    REnv.debug
      ~level:1
      renv
      ~start:(lazy (Format.sprintf "mk %s" (Kind.show kind)))
      ~end_:(fun (_, ty) -> show ty)
    @@ fun renv ->
    match kind with
    | Kind.Mixed -> (env, Mixed)
    | Kind.Primitive -> (env, Primitive (Primitive.pick ()))
    | Kind.Option -> begin
      match
        mk
          ~complexity:(complexity - 1)
          REnv.{ renv with for_option_ty = true }
          env
      with
      | (_, Option _) as res -> res
      | (env, ty) -> (env, Option ty)
    end
    | Kind.Awaitable ->
      let (env, ty) =
        mk
          ~complexity:(complexity - 1)
          REnv.{ renv with for_option_ty = false }
          env
      in
      (env, Awaitable ty)
    | Kind.Classish ->
      mk_classish renv env ~parent:None ~contract:None ~complexity ~depth
    | Kind.GenericClass ->
      let key = mk_arraykey renv env in
      let (env, value) =
        mk
          REnv.{ renv with for_option_ty = false }
          env
          ~complexity:(complexity - 1)
      in
      let (env, _, ty, reader, writer) = declare_generic env ~key ~value in
      (env, select [ty; reader; writer])
    | Kind.Alias ->
      let name = fresh "A" in
      let ty = Alias { name } in
      let (env, aliased) =
        mk ~complexity:default_complexity renv env ~for_alias_def:true
      in
      let env = Env.record_subtype env ~super:ty ~sub:aliased in
      let env = Env.record_typedef_body env ~ty ~body:[aliased] in
      let env = Env.add_definition env @@ Definition.alias ~name aliased in
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
      let env = Env.record_subtype env ~super:ty ~sub:aliased in
      let env = Env.record_typedef_body env ~ty ~body:[aliased] in
      let env =
        Env.add_definition env @@ Definition.newtype ~name ~bound aliased
      in
      (env, ty)
    | Kind.TypeConst ->
      let tc_name = fresh "TC" in
      let (env, aliased) = mk ~complexity:default_complexity renv env in
      let typeconst_def = Definition.typeconst ~name:tc_name aliased in
      let class_name = fresh "CTC" in
      let qualified_name = Format.sprintf "%s::%s" class_name tc_name in
      let ty = TypeConst { name = qualified_name } in
      let env = Env.record_subtype env ~super:ty ~sub:aliased in
      let env = Env.record_typedef_body env ~ty ~body:[aliased] in
      let env =
        Env.add_definition env
        @@ Definition.classish
             ~name:class_name
             ~parent:None
             ~interfaces:[]
             ~generic:None
             ~members:[typeconst_def]
             Kind.Class
      in
      (env, TypeConst { name = qualified_name })
    | Kind.Case ->
      let name = fresh "CT" in
      let ty = Case { name } in
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
        (* 2/3 odds for to add more disjuncts. Disjointness check is not
           guaranteed to succeed, so we tilt the odds of creating an interesting
           like type by trying hard. *)
        if Random.int_incl 1 4 = 1 then
          (env, disjuncts)
        else
          (* We are not guaranteed to use this disjunct if it fails the
             disjointness check, so we speculatively modify the environment and
             discard it if disjointness fails. *)
          let (env_with_disjunct, disjunct) = mk renv env in
          let env_with_disjunct =
            Env.record_subtype env_with_disjunct ~super:ty ~sub:disjunct
          in
          if
            List.for_all
              disjuncts
              ~f:(are_disjoint renv env_with_disjunct disjunct)
          then
            add_disjuncts @@ (env_with_disjunct, disjunct :: disjuncts)
          else
            add_disjuncts (env, disjuncts)
      in
      let (env, disjunct) = mk renv env in
      let env = Env.record_subtype env ~super:ty ~sub:disjunct in
      let (env, disjuncts) = add_disjuncts (env, [disjunct]) in
      let env = Env.record_typedef_body env ~ty ~body:disjuncts in
      let env =
        Env.add_definition env @@ Definition.case_type ~name ~bound disjuncts
      in
      let env =
        Option.fold bound ~init:env ~f:(fun env bound ->
            let env = Env.record_case_bound env ~ty ~bound in
            Env.record_subtype env ~super:bound ~sub:ty)
      in
      (env, ty)
    | Kind.Enum ->
      let name = fresh "E" in
      let ty = Enum { name } in
      let (env, bound, underlying_ty, value) =
        if Random.bool () then
          let bound = mk_arraykey REnv.{ renv with for_enum_def = true } env in
          let underlying_ty =
            subtype_of REnv.{ renv with for_enum_def = true } env bound
          in
          let value = inhabitant_of renv env underlying_ty in
          let env = Env.record_subtype env ~super:bound ~sub:ty in
          (env, Some bound, underlying_ty, value)
        else
          let underlying_ty =
            mk_arraykey REnv.{ renv with for_enum_def = true } env
          in
          let value = inhabitant_of renv env underlying_ty in
          let env = Env.record_subtype env ~super:Mixed ~sub:ty in
          (env, None, underlying_ty, value)
      in
      let env =
        Env.add_definition env
        @@ Definition.enum ~name ~bound underlying_ty ~value
      in
      (env, ty)
    | Kind.Container -> begin
      let renv = REnv.{ renv with for_option_ty = false } in
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
    | Kind.BuiltinContainer ->
      let renv = REnv.{ renv with for_option_ty = false } in
      let (env, value) = mk ~complexity:(complexity - 1) renv env in
      let key = mk_arraykey renv env in
      let ty =
        select
          [
            Traversable value;
            ContainerInterface value;
            Iterator value;
            KeyedTraversable { key; value };
            KeyedContainer { key; value };
            KeyedIterator { key; value };
            VecOrDict { key; value };
          ]
      in
      (env, ty)
    | Kind.Tuple ->
      let n = geometric_between min_tuple_arity max_tuple_arity in
      let renv = REnv.{ renv with for_option_ty = false } in
      let (env, conjuncts) =
        List.init n ~f:(fun _ -> ())
        |> List.fold_map ~init:env ~f:(fun env _ ->
               mk ~complexity:(complexity - 1) renv env)
      in
      (env, Tuple { conjuncts; open_ = false })
    | Kind.Shape ->
      let keys = choose_nondet shape_keys in
      let renv = REnv.{ renv with for_option_ty = false } in
      let mk_field env key =
        let (env, ty) = mk ~complexity:(complexity - 1) renv env in
        let optional = Random.bool () in
        (env, { key; optional; ty })
      in
      let (env, fields) = List.fold_map ~init:env ~f:mk_field keys in
      let open_ = Random.bool () in
      (env, Shape { fields; open_ })
    | Kind.Function ->
      let renv = REnv.{ renv with for_option_ty = false } in
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

  let mk_generic_witness renv env ~value =
    let key = mk_arraykey renv env in
    let (env, name, ty, _, _) = declare_generic env ~key ~value in
    let narrower =
      subtype_of
        REnv.
          {
            renv with
            pick_immediately_inhabited = false;
            for_alias_def = false;
          }
        env
        value
    in
    let wide = GenericClass { name; key; value = Mixed } in
    let reader = generic_protocol name "Reader" Mixed in
    let writer = generic_protocol name "Writer" narrower in
    let env = Env.record_subtype env ~super:reader ~sub:ty in
    let env = Env.record_subtype env ~super:reader ~sub:wide in
    let env = Env.record_subtype env ~super:writer ~sub:ty in
    let env = Env.record_subtype env ~super:writer ~sub:wide in
    let tagged ty =
      Tuple { conjuncts = [Primitive Primitive.Int; ty]; open_ = false }
    in
    let tagged_class = GenericClass { name; key; value = tagged value } in
    let tagged_writer = generic_protocol name "Writer" (tagged narrower) in
    let env = Env.record_subtype env ~super:tagged_writer ~sub:tagged_class in
    let env = Env.record_subtype env ~super:tagged_writer ~sub:wide in
    ( env,
      {
        generic_family = name;
        generic_key = key;
        generic_payload = value;
        generic_narrow = narrower;
        generic_class = ty;
        generic_wide = wide;
        generic_reader = reader;
        generic_writer = writer;
        generic_tagged_class = tagged_class;
        generic_tagged_writer = tagged_writer;
      } )

  let mk = mk ~depth:None ~complexity:default_complexity

  type operation = {
    parameters: t list;
    result: t;
    apply: Milner_syntax.expr list -> Milner_syntax.expr;
  }

  let compose operations ~locals ~fuel ty =
    let rec available fuel ty =
      List.exists locals ~f:(fun (local_ty, _) -> equal local_ty ty)
      || fuel > 0
         && List.exists operations ~f:(fun operation ->
                equal operation.result ty
                && List.for_all operation.parameters ~f:(available (fuel - 1)))
    in
    let rec generate fuel ty =
      let locals =
        List.filter_map locals ~f:(fun (local_ty, expression) ->
            if equal local_ty ty then
              Some (fun () -> expression)
            else
              None)
      in
      let calls =
        if fuel <= 0 then
          []
        else
          List.filter_map operations ~f:(fun operation ->
              if
                equal operation.result ty
                && List.for_all operation.parameters ~f:(available (fuel - 1))
              then
                Some
                  (fun () ->
                    operation.apply
                      (List.map operation.parameters ~f:(generate (fuel - 1))))
              else
                None)
      in
      (select (locals @ calls)) ()
    in
    generate fuel ty

  let hierarchy_bindings renv env value_type =
    let open Milner_syntax in
    let (env, contract) = make_member_contract env value_type in
    let (env, root) =
      mk_classish
        renv
        env
        ~parent:None
        ~contract:(Some contract)
        ~complexity:default_complexity
        ~depth:0
    in
    let rec class_ancestor = function
      | Classish { kind = Kind.Interface; _ } as ty ->
        class_ancestor (select (Env.get_subtypes env ty))
      | Classish _ as ty -> ty
      | _ -> failwith "Expected a generated nominal hierarchy"
    in
    let ancestor = class_ancestor root in
    let concrete =
      subtype_of
        REnv.{ renv with pick_immediately_inhabited = true }
        env
        ancestor
    in
    let owner =
      if Random.bool () then
        ancestor
      else
        concrete
    in
    let (owner_name, identity) =
      match owner with
      | Classish { name; _ } -> (name, (Env.get_nominal env name).Env.identity)
      | _ -> failwith "Expected a generated class"
    in
    let concrete_name =
      match concrete with
      | Classish { name; kind = Kind.Class; _ } -> name
      | _ -> failwith "Expected an instantiable class"
    in
    let value = fresh_local "value" in
    let object_ = fresh_local "object" in
    let expected_dispatch =
      (Env.get_nominal env concrete_name).Env.probe_value
    in
    let dispatch =
      Lambda
        ( [(show ancestor, object_)],
          [],
          "int",
          [
            Return (Some (Call (Member (Local object_, contract.Env.probe), [])));
          ] )
    in
    let construct =
      Lambda
        ( [(show value_type, value)],
          ["write_props"],
          show concrete,
          [Return (Some (New (show concrete, [Local value])))] )
    in
    let read =
      Lambda
        ( [(show ancestor, object_)],
          [],
          show value_type,
          [
            Return
              (Some (Call (Member (Local object_, contract.Env.getter), [])));
          ] )
    in
    let write =
      Lambda
        ( [(show ancestor, object_); (show value_type, value)],
          ["write_props"],
          "void",
          [
            Eval
              (Call (Member (Local object_, contract.Env.setter), [Local value]));
          ] )
    in
    let static_owner =
      if Random.bool () then
        owner_name
      else
        concrete_name
    in
    let identity =
      Lambda
        ( [(show value_type, value)],
          [],
          show value_type,
          [
            Return
              (Some
                 (Call (StaticMember (static_owner, identity), [Local value])));
          ] )
    in
    let operation parameters result callee =
      {
        parameters;
        result;
        apply = (fun arguments -> Call (callee, arguments));
      }
    in
    let interface_read =
      Lambda
        ( [(contract.Env.reader, object_)],
          [],
          show value_type,
          [
            Return
              (Some (Call (Atom contract.Env.read_function, [Local object_])));
          ] )
    in
    let interface_write =
      Lambda
        ( [(contract.Env.writer, object_); (show value_type, value)],
          ["write_props"],
          "void",
          [
            Eval
              (Call
                 (Atom contract.Env.write_function, [Local object_; Local value]));
          ] )
    in
    let trait_read =
      let method_name = (Env.get_nominal env concrete_name).Env.dispatch in
      Lambda
        ( [(show concrete, object_)],
          [],
          show value_type,
          [Return (Some (Call (Member (Local object_, method_name), [])))] )
    in
    let operations =
      [
        operation [value_type] concrete construct;
        operation [concrete] value_type read;
        operation [value_type] value_type identity;
        operation [concrete] value_type trait_read;
        operation [concrete] value_type interface_read;
      ]
    in
    let transform =
      Lambda
        ( [(show value_type, value)],
          ["write_props"],
          show value_type,
          [
            Return
              (Some
                 (compose
                    operations
                    ~locals:[(value_type, Local value)]
                    ~fuel:(geometric_between 2 6)
                    value_type));
          ] )
    in
    let bindings =
      List.map
        [
          ("construct", construct);
          ("read", read);
          ("write", write);
          ("identity", identity);
          ("hierarchy", transform);
          ("dispatch", dispatch);
          ("trait_read", trait_read);
          ("interface_read", interface_read);
          ("interface_write", interface_write);
        ]
        ~f:(fun (prefix, expression) -> (prefix, render_expr expression))
    in
    ( env,
      ("CLASS_TYPE", show concrete)
      :: ("ANCESTOR_TYPE", show ancestor)
      :: ("DISPATCH", string_of_int expected_dispatch)
      :: bindings )
end

and TypeMap : (Wrapped_map.S with type key = Type.t) = Wrapped_map.Make (Type)
and TypeSet : (Stdlib.Set.S with type elt = Type.t) = Stdlib.Set.Make (Type)
