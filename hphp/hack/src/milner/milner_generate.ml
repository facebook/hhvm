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

let string_literal () = Milner_syntax.Atom (select ["''"; "'apple'"; "'pear'"])

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

module FunctionContext = struct
  type t =
    | Pure
    | WriteProps
    | LeakSafe
    | Globals
    | WritePropsGlobals
    | LeakSafeGlobals
    | Defaults
  [@@deriving eq, ord]

  let all =
    [
      Pure;
      WriteProps;
      LeakSafe;
      Globals;
      WritePropsGlobals;
      LeakSafeGlobals;
      Defaults;
    ]

  let names = function
    | Pure -> []
    | WriteProps -> ["write_props"]
    | LeakSafe -> ["leak_safe"]
    | Globals -> ["globals"]
    | WritePropsGlobals -> ["write_props"; "globals"]
    | LeakSafeGlobals -> ["leak_safe"; "globals"]
    | Defaults -> ["defaults"]

  let show context = "[" ^ String.concat ~sep:", " (names context) ^ "]"

  let writes_properties = function
    | Pure
    | Globals ->
      false
    | _ -> true

  let uses_globals = function
    | Globals
    | WritePropsGlobals
    | LeakSafeGlobals
    | Defaults ->
      true
    | _ -> false

  let subcontexts = function
    | Pure -> [Pure]
    | WriteProps -> [Pure; WriteProps]
    | LeakSafe -> [Pure; WriteProps; LeakSafe]
    | Globals -> [Pure; Globals]
    | WritePropsGlobals -> [Pure; WriteProps; Globals; WritePropsGlobals]
    | LeakSafeGlobals ->
      [Pure; WriteProps; LeakSafe; Globals; WritePropsGlobals; LeakSafeGlobals]
    | Defaults -> all
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
    for_enum_class_value: bool;
        (** Avoid HHVM's abort when dynamic enum constants copy label values. *)
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

  val for_enum_initializer : t -> t

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
    for_enum_class_value: bool;
    for_enum_def: bool;
    pick_immediately_inhabited: bool;
    debug_info: debug_info;
  }

  let default ~verbose ~debug_pattern =
    {
      for_option_ty = false;
      for_reified_ty = false;
      for_alias_def = false;
      for_enum_class_value = false;
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

  (* T288868934: indirect label-valued enum initializers abort in HHVM. *)
  let for_enum_initializer renv = { renv with for_enum_class_value = true }

  let show
      {
        for_option_ty;
        for_reified_ty;
        for_alias_def;
        for_enum_class_value;
        for_enum_def;
        pick_immediately_inhabited;
        debug_info = _;
      } =
    Format.sprintf
      "{for_option_ty: %b, for_reified_ty: %b, for_alias_def: %b; for_enum_def: %b; for_enum_class_value: %b; pick_immediately_inhabited: %b}"
      for_option_ty
      for_reified_ty
      for_alias_def
      for_enum_def
      for_enum_class_value
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

  type dependent_info = {
    concrete: string;
    base: string;
    bound: Type.t option;
    value_type: Type.t;
    read_function: string;
  }

  type t = {
    definitions: Definition.t list;
    subtypes: Type.t list TypeMap.t;
    typedef_bodies: Type.t list TypeMap.t;
    case_bounds: Type.t TypeMap.t;
    nominals: nominal_info S_map.t;
    generic_families: string list;
    dependents: dependent_info S_map.t;
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

  val add_dependent : t -> name:string -> dependent_info -> t

  val get_dependent : t -> string -> dependent_info
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

  type dependent_info = {
    concrete: string;
    base: string;
    bound: Type.t option;
    value_type: Type.t;
    read_function: string;
  }

  type t = {
    definitions: Definition.t list;
    subtypes: Type.t list TypeMap.t;
    typedef_bodies: Type.t list TypeMap.t;
    case_bounds: Type.t TypeMap.t;
    nominals: nominal_info S_map.t;
    generic_families: string list;
    dependents: dependent_info S_map.t;
  }

  let default =
    {
      definitions = [];
      subtypes = TypeMap.empty;
      typedef_bodies = TypeMap.empty;
      case_bounds = TypeMap.empty;
      nominals = S_map.empty;
      generic_families = [];
      dependents = S_map.empty;
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

  let add_dependent env ~name info =
    { env with dependents = S_map.add name info env.dependents }

  let get_dependent env name = S_map.find name env.dependents
end

and Kind : sig
  type t =
    | Mixed
    | Nonnull
    | Primitive
    | Option
    | Classish
    | GenericClass
    | Dependent
    | Alias
    | Newtype
    | TypeConst
    | Case
    | Enum
    | EnumClass
    | ClassIdentity
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
    | Nonnull
    | Primitive
    | Option
    | Classish
    | GenericClass
    | Dependent
    | Alias
    | Newtype
    | TypeConst
    | Case
    | Enum
    | EnumClass
    | ClassIdentity
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
      | Function
      | ClassIdentity ->
        not for_reified_ty
      | TypeConst
      | Dependent ->
        not for_alias_def
      | _ -> true
    in
    (* Complexity filter ensures that we don't generate heavily nested types
       which make program generation expensive. *)
    let complexity_filter = function
      | Mixed
      | Nonnull
      | Primitive
      | Alias
      | Newtype
      | TypeConst
      | Case
      | Enum
      | ClassIdentity ->
        true
      | Option
      | Classish
      | EnumClass
      | GenericClass
      | Dependent
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

  val dependent_family :
    name:string ->
    base:string ->
    bound:Type.t option ->
    value_type:Type.t ->
    read_function:string ->
    t list

  val alias : name:string -> Type.t -> t

  val newtype : name:string -> bound:Type.t option -> Type.t -> t

  val case_type : name:string -> bound:Type.t option -> Type.t list -> t

  val enum : name:string -> bound:Type.t option -> Type.t -> value:string -> t

  val enum_class :
    name:string ->
    parent:string option ->
    members:(string * Type.t * Milner_syntax.expr) list ->
    t

  val enum_unwrap : name:string -> enum_name:string -> t

  val identity_class :
    name:string -> parent:string option -> payload:Type.t option -> t

  val effect_state : name:string -> t

  val raw : string -> t
end = struct
  type t = string

  let show def = def

  let raw source = source

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
        "final class %s<TKey as arraykey, TValue> implements %sReader<TValue>, %sWriter<TValue> { public ?Vector<TValue> $items; public function __construct(private TKey $key, private TValue $value)[write_props] { $this->items = new Vector(vec[$value]); } public function clear()[write_props]: void { $this->items = null; } public function get()[]: TValue { return $this->value; } public function set(TValue $value)[write_props]: void { $this->value = $value; } public function keyed()[]: dict<TKey, TValue> { return dict[$this->key => $this->value]; } public function project<TProjected as TValue>(TProjected $value)[]: TProjected { return $value; } public function widen<TWide super TValue>(TWide $_witness)[write_props]: %s<TKey, TWide> { return new %s<TKey, TWide>($this->key, $this->value); } public static function identity<TItem>(TItem $value)[]: TItem { return $value; } }"
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

  let dependent_family ~name ~base ~bound ~value_type ~read_function =
    let bound_type = Option.value_map bound ~default:"mixed" ~f:Type.show in
    let bound_clause =
      Option.value_map bound ~default:"" ~f:(fun bound_ty ->
          " as " ^ Type.show bound_ty)
    in
    let value_type = Type.show value_type in
    [
      Format.sprintf
        "abstract class %s { abstract const type Item%s; public function __construct(protected this::Item $value)[write_props] {} public function get()[]: this::Item { return $this->value; } public function set(this::Item $value)[write_props]: void { $this->value = $value; } }"
        base
        bound_clause;
      Format.sprintf
        "final class %s extends %s { const type Item = %s; }"
        name
        base
        value_type;
      Format.sprintf
        "function %s<TItem%s>(%s with { type Item = TItem } $source)[]: TItem { return $source->get(); }"
        read_function
        bound_clause
        base;
      Format.sprintf
        "function %s_bound(%s with { type Item as %s } $source)[]: %s { return $source->get(); }"
        read_function
        base
        bound_type
        bound_type;
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

  let enum_class ~name ~parent ~members =
    let parent =
      Option.value_map parent ~default:"" ~f:(Format.sprintf " extends %s")
    in
    let members =
      List.map members ~f:(fun (name, ty, value) ->
          Format.sprintf
            "  %s %s = %s;"
            (Type.show ty)
            name
            (Milner_syntax.render_expr value))
      |> String.concat ~sep:"\n"
    in
    Format.sprintf "enum class %s: mixed%s {\n%s\n}" name parent members

  let enum_unwrap ~name ~enum_name =
    Format.sprintf
      "function %s<TValue>(HH\\MemberOf<%s, TValue> $member)[]: TValue { return $member; }"
      name
      enum_name

  let identity_class ~name ~parent ~payload =
    let (constructor, getter) =
      match (payload, parent) with
      | (None, None) -> ("public function __construct()[] {}", "")
      | (None, Some _) ->
        ("public function __construct()[] { parent::__construct(); }", "")
      | (Some ty, None) ->
        ( Format.sprintf
            "public function __construct(protected %s $payload)[write_props] {}"
            (Type.show ty),
          Format.sprintf
            "public function get()[]: %s { return $this->payload; }"
            (Type.show ty) )
      | (Some ty, Some _) ->
        ( Format.sprintf
            "public function __construct(%s $payload)[write_props] { parent::__construct($payload); }"
            (Type.show ty),
          "" )
    in
    let parent =
      Option.value_map parent ~default:"" ~f:(Format.sprintf " extends %s")
    in
    Format.sprintf
      "<<__ConsistentConstruct>>\nclass %s%s {\n  %s\n  %s\n  public static function identity()[]: classname<this> { return static::class; }\n}"
      name
      parent
      constructor
      getter

  let effect_state ~name =
    Format.sprintf
      "final class %s { public int $value = 0; public static int $calls = 0; }"
      name
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

  val inhabitant_of :
    ReadOnlyEnvironment.t -> Environment.t -> t -> Environment.t * string

  val subtype_of : ReadOnlyEnvironment.t -> Environment.t -> t -> t

  val mk : ReadOnlyEnvironment.t -> Environment.t -> Environment.t * t
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

  and enum_class_member = {
    enum_name: string;
    member: string;
    payload: t;
  }

  and function_return =
    | ReturnsValue of t
    | ReturnsVoid
    | ReturnsNothing

  and t =
    | Mixed
    | Nonnull
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
    | Dependent of { name: string }
    | Alias of { name: string }
    | Newtype of { name: string }
    | TypeConst of { name: string }
    | Case of { name: string }
    | Enum of { name: string }
    | EnumClassMember of enum_class_member
    | EnumClassLabel of enum_class_member
    | ClassIdentity of {
        name: string;
        is_pointer: bool;
      }
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
        optional_conjuncts: t list;
        open_: bool;
      }
    | Shape of {
        fields: field list;
        open_: bool;
      }
    | Function of {
        parameters: t list;
        variadic: t option;
        return_: function_return;
        context: FunctionContext.t;
        effect_state: string;
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
    | Nonnull -> "nonnull"
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
    | Dependent info -> info.name
    | Alias info -> info.name
    | Newtype info -> info.name
    | TypeConst info -> info.name
    | Case info -> info.name
    | Enum info -> info.name
    | EnumClassMember { enum_name; payload; _ } ->
      Format.sprintf "HH\\MemberOf<%s, %s>" enum_name (show payload)
    | EnumClassLabel { enum_name; payload; _ } ->
      Format.sprintf "HH\\EnumClass\\Label<%s, %s>" enum_name (show payload)
    | ClassIdentity { name; is_pointer } ->
      Format.sprintf
        "%s<%s>"
        (if is_pointer then
          "class"
        else
          "classname")
        name
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
    | Tuple { conjuncts; optional_conjuncts; open_ } ->
      let has_optional = not (List.is_empty optional_conjuncts) in
      let fields =
        List.map ~f:show conjuncts
        @ List.map optional_conjuncts ~f:(fun ty -> "optional " ^ show ty)
      in
      let is_nullary = List.is_empty fields in
      let conjuncts = String.concat ~sep:", " fields in
      let open_ =
        if open_ && has_optional then
          ", mixed..."
        else if open_ && is_nullary then
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
    | Function { parameters; variadic; return_; context; _ } ->
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
      let return_ = show_return return_ in
      Format.sprintf
        "(function(%s%s)%s: %s)"
        parameters
        variadic
        (FunctionContext.show context)
        return_
    | Like ty -> "~" ^ show ty

  and show_return = function
    | ReturnsValue ty -> show ty
    | ReturnsVoid -> "void"
    | ReturnsNothing -> "nothing"

  let supports_return return_ context =
    match return_ with
    | ReturnsValue _ -> FunctionContext.writes_properties context
    | ReturnsVoid
    | ReturnsNothing ->
      true

  let function_context return_ =
    FunctionContext.all |> List.filter ~f:(supports_return return_) |> select

  let function_effects context name =
    let open Syntax in
    let candidates =
      (if FunctionContext.writes_properties context then
        [
          (fun () ->
            let state = fresh_local "state" in
            [
              Bind (state, New (name, []));
              Assign (Member (Local state, "value"), Atom "42");
            ]);
        ]
      else
        [])
      @
      if FunctionContext.uses_globals context then
        [(fun () -> [Assign (StaticProperty (name, "calls"), Atom "42")])]
      else
        []
    in
    List.permute candidates |> List.concat_map ~f:(fun effect -> effect ())

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
        | Tuple { conjuncts; optional_conjuncts; _ } ->
          List.exists
            (conjuncts @ optional_conjuncts)
            ~f:(has_like_head env seen)
        | Shape { fields; _ } ->
          List.exists fields ~f:(fun { ty; _ } -> has_like_head env seen ty)
        | Alias _
        | Newtype _
        | Dependent _
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
        | Dependent _
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
        | Tuple { conjuncts; optional_conjuncts; _ } ->
          List.exists
            (conjuncts @ optional_conjuncts)
            ~f:(has_nullable_form env seen)
        | Shape { fields; _ } ->
          List.exists fields ~f:(fun { ty; _ } -> has_nullable_form env seen ty)
        | Alias _
        | Newtype _
        | Dependent _
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
    let rec has_structural_tuple env seen ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Tuple _ -> true
        | Shape { fields; _ } ->
          List.exists fields ~f:(fun { ty; _ } ->
              has_structural_tuple env seen ty)
        | Alias _
        | Newtype _
        | TypeConst _
        | Dependent _
        | Case _ ->
          List.exists
            (Env.get_subtypes env ty)
            ~f:(has_structural_tuple env seen)
        | Like ty
        | Option ty
        | EnumClassMember { payload = ty; _ } ->
          has_structural_tuple env seen ty
        | _ -> false
    in
    let rec has_class_identity env seen ~is_pointer ~through_vec ty =
      if TypeSet.mem ty seen then
        false
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | ClassIdentity info -> Bool.equal is_pointer info.is_pointer
        | Vec ty when through_vec ->
          has_class_identity env seen ~is_pointer ~through_vec ty
        | Tuple { conjuncts; optional_conjuncts; _ } ->
          List.exists
            (conjuncts @ optional_conjuncts)
            ~f:(has_class_identity env seen ~is_pointer ~through_vec)
        | Shape { fields; _ } ->
          List.exists fields ~f:(fun { ty; _ } ->
              has_class_identity env seen ~is_pointer ~through_vec ty)
        | Alias _
        | Newtype _
        | TypeConst _
        | Dependent _
        | Case _ ->
          List.exists
            (Env.get_subtypes env ty)
            ~f:(has_class_identity env seen ~is_pointer ~through_vec)
        | Like ty
        | Option ty
        | EnumClassMember { payload = ty; _ } ->
          has_class_identity env seen ~is_pointer ~through_vec ty
        | _ -> false
    in
    (* T288868918: classname/class intersections can lose the pointer constraint. *)
    let identity_hazard env1 ty1 env2 ty2 =
      has_class_identity
        env1
        TypeSet.empty
        ~is_pointer:false
        ~through_vec:(has_structural_tuple env2 TypeSet.empty ty2)
        ty1
      && has_class_identity
           env2
           TypeSet.empty
           ~is_pointer:true
           ~through_vec:(has_structural_tuple env1 TypeSet.empty ty1)
           ty2
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
          | TypeConst _
          | Dependent _ ->
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
        | Dependent _
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
        | Dependent _
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
        | Nonnull
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
        | Dependent _
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
          | Dependent _
          | TypeConst _ ->
            Option.exists (Env.get_typedef_body env_case ty) ~f:(function
                | [inner] -> exposed_case seen inner
                | _ -> false)
          | _ -> false
      in
      null_head env_null TypeSet.empty ty_null
      && exposed_case TypeSet.empty ty_case
    in
    let rec expose_definition env seen ty =
      if TypeSet.mem ty seen then
        ty
      else
        match (ty, Env.get_typedef_body env ty) with
        | ((Alias _ | Newtype _ | TypeConst _ | Dependent _), Some [inner]) ->
          expose_definition env (TypeSet.add ty seen) inner
        | _ -> ty
    in
    let rec null_function_partition env seen ty =
      let ty = expose_definition env TypeSet.empty ty in
      if TypeSet.mem ty seen then
        None
      else
        let seen = TypeSet.add ty seen in
        match ty with
        | Primitive Primitive.Null -> Some true
        | Function _ -> Some false
        | Option inner ->
          Option.map (null_function_partition env seen inner) ~f:(Fn.const true)
        | Case _ ->
          Option.bind (Env.get_typedef_body env ty) ~f:(fun variants ->
              List.fold variants ~init:(Some false) ~f:(fun acc variant ->
                  match (acc, null_function_partition env seen variant) with
                  | (Some has_null, Some variant_null) ->
                    Some (has_null || variant_null)
                  | _ -> None))
        | _ -> None
    in
    (* T288865283: opaque enum/null partitions can fail reflexivity. *)
    let enum_null_partition_hazard env_enum ty_enum env_other ty_other =
      match null_function_partition env_other TypeSet.empty ty_other with
      | Some true ->
        let function_type =
          match expose_definition env_other TypeSet.empty ty_other with
          | Option inner -> begin
            match expose_definition env_other TypeSet.empty inner with
            | Function _ as ty -> Some ty
            | _ -> None
          end
          | _ -> None
        in
        let rec payload_hazard seen ~like ty =
          let ty = expose_definition env_enum TypeSet.empty ty in
          if TypeSet.mem ty seen then
            false
          else
            let seen = TypeSet.add ty seen in
            match ty with
            | Mixed
            | EnumClassLabel _ ->
              true
            | Case _ ->
              Option.value_map
                (Env.get_case_bound env_enum ty)
                ~default:true
                ~f:(payload_hazard seen ~like)
            | EnumClassMember { payload; _ } ->
              payload_hazard seen ~like payload
            | Like inner -> payload_hazard seen ~like:true inner
            | Primitive Primitive.Null -> like
            | Option inner ->
              let null_only =
                match expose_definition env_enum TypeSet.empty inner with
                | Primitive Primitive.Null -> true
                | _ -> false
              in
              let same_function =
                match
                  (expose_definition env_enum TypeSet.empty inner, function_type)
                with
                | ((Function _ as payload_function), Some other_function) ->
                  (* Witness state is absent from the emitted signature. *)
                  String.equal (show payload_function) (show other_function)
                | _ -> false
              in
              like || not (null_only || same_function)
            | _ -> false
        in
        let rec exposed_enum seen ty =
          let ty = expose_definition env_enum TypeSet.empty ty in
          if TypeSet.mem ty seen then
            false
          else
            let seen = TypeSet.add ty seen in
            match ty with
            | EnumClassLabel _ -> true
            | EnumClassMember { payload; _ } ->
              payload_hazard TypeSet.empty ~like:false payload
            | Like inner -> exposed_enum seen inner
            | Case _ -> begin
              match Env.get_typedef_body env_enum ty with
              | Some [inner] -> exposed_enum seen inner
              | _ -> false
            end
            | _ -> false
        in
        exposed_enum TypeSet.empty ty_enum
      | _ -> false
    in
    let rec case_variants env seen ty =
      if TypeSet.mem ty seen then
        None
      else
        let seen = TypeSet.add ty seen in
        match (ty, Env.get_typedef_body env ty) with
        | (Case _, Some (_ :: _ :: _ as variants)) -> Some variants
        | ( (Alias _ | Newtype _ | TypeConst _ | Dependent _ | Case _),
            Some [inner] ) ->
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
        | TypeConst _
        | Dependent _ ->
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
      || identity_hazard env1 ty1 env2 ty2
      || identity_hazard env2 ty2 env1 ty1
      || case_function_hazard env1 ty1 env2 ty2
      || case_function_hazard env2 ty2 env1 ty1
      || case_null_hazard env1 ty1 env2 ty2
      || case_null_hazard env2 ty2 env1 ty1
      || enum_null_partition_hazard env1 ty1 env2 ty2
      || enum_null_partition_hazard env2 ty2 env1 ty1
      || case_union_hazard env1 ty1 env2 ty2
      || case_union_hazard env2 ty2 env1 ty1)

  let rec is_immediately_inhabited = function
    | Primitive Primitive.(Null | Int | String | Float | Bool)
    | Classish { kind = Kind.Class; _ }
    | GenericClass _
    | Dependent _
    | Enum _
    | EnumClassMember _
    | EnumClassLabel _
    | ClassIdentity _
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
    | Tuple { conjuncts; optional_conjuncts; open_ } ->
      (not open_)
      && List.for_all
           (conjuncts @ optional_conjuncts)
           ~f:is_immediately_inhabited
    | Shape { fields; open_ = _ } ->
      List.for_all fields ~f:(fun { ty; _ } -> is_immediately_inhabited ty)
    | Awaitable ty
    | Function { return_ = ReturnsValue ty; _ } ->
      is_immediately_inhabited ty
    | Function { return_ = ReturnsVoid | ReturnsNothing; _ } -> true
    | Primitive Primitive.(Arraykey | Num)
    | Classish { kind = Kind.(Interface | AbstractClass); _ }
    | Mixed
    | Nonnull
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
          for_enum_class_value;
          _;
        }
      ty =
    ((not pick_immediately_inhabited) || is_immediately_inhabited ty)
    &&
    match ty with
    | Case _ -> not (for_option_ty || for_enum_def)
    | EnumClassLabel _ -> not for_enum_class_value
    | Function _
    | ClassIdentity _ ->
      not for_reified_ty
    | TypeConst _
    | Dependent _ ->
      not for_alias_def
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
             | Nonnull ->
               List.filter_map Primitive.all ~f:(fun prim ->
                   if Primitive.equal prim Primitive.Null then
                     None
                   else
                     Some (Primitive prim))
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
        | Nonnull
        | Primitive _
        | TypeConst _
        | Dependent _
        | GenericClass _
        | Newtype _
        | Alias _
        | Classish _
        | Case _
        | Enum _
        | EnumClassMember _
        | EnumClassLabel _
        | ClassIdentity _ ->
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
        | Tuple { conjuncts; optional_conjuncts; open_ } ->
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
          let optional_conjuncts =
            List.map
              ~f:(driver REnv.{ renv with for_alias_def = false })
              optional_conjuncts
          in
          let retained =
            if open_ then
              optional_conjuncts
            else
              List.take
                optional_conjuncts
                (geometric_between 0 (List.length optional_conjuncts))
          in
          let required = Random.int_incl 0 (List.length retained) in
          let conjuncts = conjuncts @ List.take retained required in
          let optional_conjuncts = List.drop retained required in
          [Tuple { conjuncts; optional_conjuncts; open_ }]
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
        | Function { parameters; variadic; return_; context; effect_state } ->
          let widen_parameter ty =
            if Random.bool () then
              Mixed
            else
              ty
          in
          let parameters = List.map parameters ~f:widen_parameter in
          let return_ =
            match return_ with
            | ReturnsValue ty ->
              ReturnsValue (driver REnv.{ renv with for_alias_def = false } ty)
            | ReturnsVoid -> ReturnsVoid
            | ReturnsNothing -> ReturnsNothing
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
            | Some ty -> Some (widen_parameter ty)
          in
          let context =
            FunctionContext.subcontexts context
            |> List.filter ~f:(supports_return return_)
            |> select
          in
          [Function { parameters; variadic; return_; context; effect_state }]
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
      let rec has_exposed_enum_member seen ty =
        if TypeSet.mem ty seen then
          false
        else
          let seen = TypeSet.add ty seen in
          match ty with
          | EnumClassMember _ -> true
          | Option ty -> has_exposed_enum_member seen ty
          | Alias _
          | Newtype _
          | TypeConst _
          | Dependent _
          | Case _ ->
            List.exists
              (Env.get_subtypes env ty)
              ~f:(has_exposed_enum_member seen)
          | Mixed
          | Nonnull
          | Primitive _
          | Awaitable _
          | Classish _
          | GenericClass _
          | ClassIdentity _
          | Enum _
          | EnumClassLabel _
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
          | Function _
          | Like _ ->
            false
      in
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
        | Dependent _
        | Newtype _
        | Case _ ->
          Env.get_subtypes env ty
        | Option ty -> [Primitive Primitive.Null; ty]
        | Awaitable _ -> [Awaitable Mixed]
        | Enum _ -> Primitive.[Primitive Int; Primitive String]
        | EnumClassMember { payload; _ } ->
          (* T288894213: repeated MemberOf upper bounds are misidentified as cycles. *)
          if has_exposed_enum_member TypeSet.empty payload then
            [Mixed]
          else
            [payload]
        | EnumClassLabel _
        | ClassIdentity _ ->
          [Mixed]
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
        | Vec _ ->
          [
            Vec Mixed;
            Tuple { conjuncts = []; optional_conjuncts = []; open_ = true };
          ]
        | Dict _ ->
          [
            Dict { key = Primitive Primitive.Arraykey; value = Mixed };
            Shape { fields = []; open_ = true };
          ]
        | Keyset _ -> [Keyset (Primitive Primitive.Arraykey)]
        | Tuple _ ->
          [
            Tuple { conjuncts = []; optional_conjuncts = []; open_ = true };
            Vec Mixed;
          ]
        | Shape _ ->
          [
            Shape { fields = []; open_ = true };
            Dict { key = Primitive Primitive.Arraykey; value = Mixed };
          ]
        | Function _ ->
          [Classish { kind = Kind.Class; name = "Closure"; generic = None }]
        | Mixed -> [ty]
        | Nonnull -> [Mixed]
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
      | String -> Some (string_literal ())
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
    | Dependent { name } ->
      let Env.{ concrete; value_type; _ } = Env.get_dependent env name in
      let value = inhabitant renv env value_type in
      Some
        (Syntax.Call (Syntax.Member (Syntax.New (concrete, [value]), "get"), []))
    | Enum info -> Some (StaticMember (info.name, "A"))
    | EnumClassMember { enum_name; member; _ } ->
      Some (Milner_syntax.StaticMember (enum_name, member))
    | EnumClassLabel { enum_name; member; _ } ->
      Some (Milner_syntax.EnumLabel (enum_name, member))
    | ClassIdentity { name; _ } ->
      Some (Milner_syntax.StaticMember (name, "class"))
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
    | Tuple { conjuncts; optional_conjuncts; open_ } ->
      if open_ then
        None
      else
        let present =
          conjuncts
          @ List.take
              optional_conjuncts
              (geometric_between 0 (List.length optional_conjuncts))
        in
        List.map present ~f:(expr_of renv env)
        |> Option.all
        |> Option.map ~f:(fun values -> Milner_syntax.Tuple values)
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
    | Function { parameters; variadic; return_; context; effect_state } ->
      let named_parameters =
        List.map parameters ~f:(fun ty -> (ty, Syntax.fresh_local "argument"))
      in
      let rest =
        Option.map variadic ~f:(fun ty -> (ty, Syntax.fresh_local "rest"))
      in
      let parameters =
        List.map named_parameters ~f:(fun (ty, local) ->
            Syntax.parameter (show ty) local)
        @ Option.to_list
            (Option.map rest ~f:(fun (ty, local) ->
                 Syntax.parameter ~variadic:true (show ty) local))
      in
      let locals =
        named_parameters
        @ Option.to_list
            (Option.map rest ~f:(fun (ty, local) -> (Vec ty, local)))
      in
      let open Option.Let_syntax in
      let+ result =
        match return_ with
        | ReturnsValue ty ->
          let matching_arguments =
            List.filter_map locals ~f:(fun (parameter, local) ->
                if equal ty parameter then
                  Some (Syntax.Local local)
                else
                  None)
          in
          let value =
            if (not (List.is_empty matching_arguments)) && Random.bool () then
              Some (select matching_arguments)
            else
              expr_of renv env ty
          in
          Option.map value ~f:(fun value -> [Syntax.Return (Some value)])
        | ReturnsVoid -> Some [Syntax.Return None]
        | ReturnsNothing ->
          Some
            [
              Syntax.Throw
                (Syntax.New
                   ("Exception", [Syntax.Atom "'milner expected nothing'"]));
            ]
      in
      Syntax.Lambda
        ( parameters,
          FunctionContext.names context,
          show_return return_,
          function_effects context effect_state @ result )
    | Mixed
    | Nonnull
    | Option _
    | Alias _
    | Newtype _
    | TypeConst _
    | Case _
    | Like _ ->
      None

  let constant renv env ty = inhabitant renv env ty |> Milner_syntax.render_expr

  let callable_application renv env ty ~value =
    match ty with
    | Function { parameters; variadic; _ } ->
      let tail =
        Option.value_map variadic ~default:[] ~f:(fun ty ->
            List.init (geometric_between 0 max_container_length) ~f:(fun _ ->
                inhabitant renv env ty))
      in
      let arguments =
        let positional = List.map parameters ~f:(inhabitant renv env) in
        match variadic with
        | Some _ ->
          let unpack = Random.bool () in
          (* T288960552: direct multi-tail calls can infer invalid dynamic bounds. *)
          if unpack || List.length tail > 1 then
            positional @ [Syntax.Unpack (Syntax.Array ("vec", tail))]
          else
            positional @ tail
        | None -> positional
      in
      Syntax.Call (value, arguments)
    | _ -> invalid_arg "callable_application expects a generated function"

  let mk_arraykey (renv : REnv.t) (env : Env.t) =
    let renv = REnv.{ renv with pick_immediately_inhabited = false } in
    subtype_of renv env (Primitive Primitive.Arraykey)

  let has_nullable_nominal_case_return env ty =
    (* Identical nullable enum/name case returns can fail override checking. *)
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
        | Enum _
        | ClassIdentity { is_pointer = false; _ } ->
          inside_case && nullable
        | _ -> false
    in
    visit TypeSet.empty ~inside_case:false ~nullable:false ty

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

  let declare_dependent env ~value_type ~bound =
    let concrete = fresh "Dependent" in
    let base = concrete ^ "Base" in
    let read_function = fresh "read_dependent" in
    let name = concrete ^ "::Item" in
    let ty = Dependent { name } in
    let env =
      Env.add_dependent
        env
        ~name
        Env.{ concrete; base; bound; value_type; read_function }
    in
    let env = Env.record_subtype env ~super:ty ~sub:value_type in
    let env = Env.record_typedef_body env ~ty ~body:[value_type] in
    let env =
      List.fold
        (Definition.dependent_family
           ~name:concrete
           ~base
           ~bound
           ~value_type
           ~read_function)
        ~init:env
        ~f:Env.add_definition
    in
    (env, ty)

  type enum_family = {
    enum_base: string;
    enum_child: string;
    enum_payload: t;
    enum_narrower: t;
  }

  let declare_enum_family value_renv env ~payload ~narrower =
    let base_name = fresh "EC" in
    let child_name = fresh "EC" in
    let base_value = inhabitant value_renv env payload in
    let child_value = inhabitant value_renv env narrower in
    let env =
      Env.add_definition env
      @@ Definition.enum_class
           ~name:base_name
           ~parent:None
           ~members:[("A", payload, base_value)]
    in
    let env =
      Env.add_definition env
      @@ Definition.enum_class
           ~name:child_name
           ~parent:(Some base_name)
           ~members:[("B", narrower, child_value)]
    in
    ( env,
      {
        enum_base = base_name;
        enum_child = child_name;
        enum_payload = payload;
        enum_narrower = narrower;
      } )

  let enum_view ~is_label enum_name member payload =
    let member = { enum_name; member; payload } in
    if is_label then
      EnumClassLabel member
    else
      EnumClassMember member

  let record_enum_views env family ~is_label =
    let base = enum_view ~is_label family.enum_base "A" family.enum_payload in
    let inherited =
      enum_view ~is_label family.enum_child "A" family.enum_payload
    in
    let child =
      enum_view ~is_label family.enum_child "B" family.enum_narrower
    in
    let env = Env.record_subtype env ~super:inherited ~sub:base in
    let env = Env.record_subtype env ~super:inherited ~sub:child in
    (env, [base; inherited; child])

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

  type identity_family = {
    identity_base: string;
    identity_child: string;
    identity_base_name: t;
    identity_child_name: t;
    identity_base_pointer: t;
    identity_child_pointer: t;
  }

  let declare_identity_family env ~payload =
    let base_name = fresh "CI" in
    let child_name = fresh "CI" in
    let env =
      Env.add_definition env
      @@ Definition.identity_class ~name:base_name ~parent:None ~payload
    in
    let env =
      Env.add_definition env
      @@ Definition.identity_class
           ~name:child_name
           ~parent:(Some base_name)
           ~payload
    in
    let name name = ClassIdentity { name; is_pointer = false } in
    let pointer name = ClassIdentity { name; is_pointer = true } in
    let base_name_ty = name base_name in
    let child_name_ty = name child_name in
    let base_pointer_ty = pointer base_name in
    let child_pointer_ty = pointer child_name in
    let env = Env.record_subtype env ~super:base_name_ty ~sub:child_name_ty in
    let env =
      Env.record_subtype env ~super:base_pointer_ty ~sub:child_pointer_ty
    in
    let env = Env.record_subtype env ~super:base_name_ty ~sub:base_pointer_ty in
    let env =
      Env.record_subtype env ~super:child_name_ty ~sub:child_pointer_ty
    in
    ( env,
      {
        identity_base = base_name;
        identity_child = child_name;
        identity_base_name = base_name_ty;
        identity_child_name = child_name_ty;
        identity_base_pointer = base_pointer_ty;
        identity_child_pointer = child_pointer_ty;
      } )

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
                 (has_nullable_nominal_case_return env contract.Env.value_type)
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

  and mk
      ?kind
      (renv : REnv.t)
      (env : Env.t)
      ~(complexity : int)
      ~(depth : int option) : Env.t * t =
    let depth = Option.value ~default:0 depth in
    let kind =
      Option.value_or_thunk kind ~default:(fun () -> Kind.pick ~complexity renv)
    in
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
    | Kind.Nonnull -> (env, Nonnull)
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
    | Kind.Dependent ->
      let (env, value_type, bound) =
        if Random.bool () then
          (env, mk_arraykey renv env, Some (Primitive Primitive.Arraykey))
        else
          let (env, value_type) = mk renv env ~complexity:(complexity - 1) in
          (env, value_type, None)
      in
      declare_dependent env ~value_type ~bound
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
          let value = constant renv env underlying_ty in
          let env = Env.record_subtype env ~super:bound ~sub:ty in
          (env, Some bound, underlying_ty, value)
        else
          let underlying_ty =
            mk_arraykey REnv.{ renv with for_enum_def = true } env
          in
          let value = constant renv env underlying_ty in
          let env = Env.record_subtype env ~super:Mixed ~sub:ty in
          (env, None, underlying_ty, value)
      in
      let env =
        Env.add_definition env
        @@ Definition.enum ~name ~bound underlying_ty ~value
      in
      (env, ty)
    | Kind.EnumClass ->
      let value_renv = REnv.for_enum_initializer renv in
      let (env, payload) = mk ~complexity:(complexity - 1) value_renv env in
      let narrower = subtype_of value_renv env payload in
      let (env, family) =
        declare_enum_family value_renv env ~payload ~narrower
      in
      let is_label = (not renv.REnv.for_enum_class_value) && Random.bool () in
      let (env, views) = record_enum_views env family ~is_label in
      (env, select views)
    | Kind.ClassIdentity ->
      let (env, family) = declare_identity_family env ~payload:None in
      ( env,
        select
          [
            family.identity_base_name;
            family.identity_child_name;
            family.identity_base_pointer;
            family.identity_child_pointer;
          ] )
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
      let required = Random.int_incl 0 n in
      let optional_conjuncts = List.drop conjuncts required in
      let conjuncts = List.take conjuncts required in
      (env, Tuple { conjuncts; optional_conjuncts; open_ = false })
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
      let (env, return_) =
        match Random.int_incl 0 4 with
        | 0 -> (env, ReturnsVoid)
        | 1 -> (env, ReturnsNothing)
        | _ ->
          if (not (List.is_empty parameters)) && Random.bool () then
            (env, ReturnsValue (select parameters))
          else
            let (env, ty) = mk ~complexity:(complexity - 1) renv env in
            (env, ReturnsValue ty)
      in
      let (env, variadic) =
        if Random.bool () then
          (env, None)
        else
          let (env, ty) = mk ~complexity:(complexity - 1) renv env in
          (env, Some ty)
      in
      let context = function_context return_ in
      let effect_state = fresh "EffectState" in
      let env =
        Env.add_definition env @@ Definition.effect_state ~name:effect_state
      in
      (env, Function { parameters; variadic; return_; context; effect_state })
    | Kind.Like ->
      let (env, ty) = mk ~complexity:(complexity - 1) renv env in
      (env, Like ty)

  let apply ty parameters body arguments =
    Syntax.Call
      (Syntax.Lambda (parameters, ["defaults"], show ty, body), arguments)

  let returning value = Syntax.Return (Some value)

  let forward source target value =
    let local = Syntax.fresh_local "forward" in
    Syntax.Call
      ( Syntax.Lambda
          ( [Syntax.parameter source local],
            [],
            target,
            [returning (Syntax.Local local)] ),
        [value] )

  let member receiver name arguments =
    Syntax.Call (Syntax.Member (receiver, name), arguments)

  let bind_value ty value body =
    let local = Syntax.fresh_local "input" in
    apply
      ty
      [Syntax.parameter (show ty) local]
      (body (Syntax.Local local))
      [value]

  let hierarchy_operations renv env ty =
    let (env, contract) = make_member_contract env ty in
    let (env, root) =
      mk_classish
        renv
        env
        ~parent:None
        ~contract:(Some contract)
        ~complexity:2
        ~depth:0
    in
    let rec ancestor = function
      | Classish { kind = Kind.Interface; _ } as ty ->
        ancestor (select (Env.get_subtypes env ty))
      | Classish _ as ty -> ty
      | _ -> failwith "Expected a nominal ancestor"
    in
    let ancestor = ancestor root in
    let concrete =
      subtype_of
        REnv.{ renv with pick_immediately_inhabited = true }
        env
        ancestor
    in
    let name =
      match concrete with
      | Classish { name; _ } -> name
      | _ -> failwith "Expected a concrete class"
    in
    let info = Env.get_nominal env name in
    let (static_owner, static_identity) =
      let owner =
        if Random.bool () then
          ancestor
        else
          concrete
      in
      match owner with
      | Classish { name = owner_name; _ } ->
        ( (if Random.bool () then
            owner_name
          else
            name),
          (Env.get_nominal env owner_name).Env.identity )
      | _ -> failwith "Expected a nominal owner"
    in
    let construct value = Syntax.New (show concrete, [value]) in
    let read value =
      let receiver = Syntax.fresh_local "receiver" in
      apply
        ty
        [Syntax.parameter (show ancestor) receiver]
        [returning (member (Syntax.Local receiver) contract.Env.getter [])]
        [construct value]
    in
    let write value =
      bind_value ty value (fun value ->
          let receiver = Syntax.fresh_local "receiver" in
          [
            Syntax.Bind (receiver, construct (inhabitant renv env ty));
            Syntax.Eval
              (Syntax.Call
                 ( Syntax.Atom contract.Env.write_function,
                   [Syntax.Local receiver; value] ));
            returning
              (Syntax.Call
                 ( Syntax.Atom contract.Env.read_function,
                   [Syntax.Local receiver] ));
          ])
    in
    let write_ancestor value =
      let receiver = Syntax.fresh_local "ancestor" in
      let input = Syntax.fresh_local "input" in
      apply
        ty
        [
          Syntax.parameter (show ancestor) receiver;
          Syntax.parameter (show ty) input;
        ]
        [
          Syntax.Eval
            (member
               (Syntax.Local receiver)
               contract.Env.setter
               [Syntax.Local input]);
          returning (member (Syntax.Local receiver) contract.Env.getter []);
        ]
        [construct (inhabitant renv env ty); value]
    in
    let operations =
      [
        read;
        write;
        write_ancestor;
        (fun value -> member (construct value) info.Env.dispatch []);
        (fun value ->
          Syntax.Call
            (Syntax.StaticMember (static_owner, static_identity), [value]));
      ]
    in
    let operations =
      (fun value ->
        let receiver = Syntax.fresh_local "receiver" in
        apply
          ty
          [Syntax.parameter (show ancestor) receiver]
          [
            returning
              (Syntax.Index
                 ( Syntax.Array
                     ( "vec",
                       [member (Syntax.Local receiver) contract.Env.getter []]
                     ),
                   Syntax.Binary
                     ( "-",
                       member (Syntax.Local receiver) contract.Env.probe [],
                       Syntax.Atom (string_of_int info.Env.probe_value) ) ));
          ]
          [construct value])
      :: operations
    in
    (env, operations)

  let generic_operations renv env ty =
    let key = mk_arraykey renv env in
    let (env, name, concrete, reader, _) = declare_generic env ~key ~value:ty in
    let construct value =
      Syntax.New (show concrete, [inhabitant renv env key; value])
    in
    let narrower = subtype_of renv env ty in
    let construct_narrow () =
      Syntax.New
        ( show (GenericClass { name; key; value = narrower }),
          [inhabitant renv env key; inhabitant renv env narrower] )
    in
    let read value =
      let receiver = Syntax.fresh_local "reader" in
      apply
        ty
        [Syntax.parameter (show reader) receiver]
        [returning (member (Syntax.Local receiver) "get" [])]
        [
          (if Random.bool () then
            construct value
          else
            construct_narrow ());
        ]
    in
    let write value =
      bind_value ty value (fun value ->
          let receiver = Syntax.fresh_local "generic" in
          [
            Syntax.Bind (receiver, construct value);
            Syntax.Eval
              (Syntax.Call
                 ( Syntax.Atom (name ^ "_write<" ^ show narrower ^ ">"),
                   [Syntax.Local receiver; inhabitant renv env narrower] ));
            returning (member (Syntax.Local receiver) "get" []);
          ])
    in
    let project value =
      member
        (construct value)
        ("project<" ^ show narrower ^ ">")
        [inhabitant renv env narrower]
    in
    let widen value =
      bind_value ty value (fun value ->
          [
            returning
              (member
                 (member
                    (construct_narrow ())
                    ("widen<" ^ show ty ^ ">")
                    [value])
                 "get"
                 []);
          ])
    in
    let keyed value =
      let index = Syntax.fresh_local "key" in
      let input = Syntax.fresh_local "input" in
      apply
        ty
        [Syntax.parameter (show key) index; Syntax.parameter (show ty) input]
        [
          returning
            (Syntax.Index
               ( member
                   (Syntax.New
                      (show concrete, [Syntax.Local index; Syntax.Local input]))
                   "keyed"
                   [],
                 Syntax.Local index ));
        ]
        [inhabitant renv env key; value]
    in
    let invalidate value =
      bind_value ty value (fun value ->
          let receiver = Syntax.fresh_local "cell" in
          let items = Syntax.Member (Syntax.Local receiver, "items") in
          [
            Syntax.Bind (receiver, construct value);
            Syntax.If
              ( Syntax.Is (items, "null"),
                [returning value],
                [
                  Syntax.Eval (member (Syntax.Local receiver) "clear" []);
                  returning
                    (Syntax.Index
                       ( Syntax.Array ("vec", [value]),
                         Syntax.Binary
                           ( "??",
                             Syntax.Call
                               (Syntax.NullsafeMember (items, "count"), []),
                             Syntax.Atom "0" ) ));
                ] );
          ])
    in
    let tagged_write value =
      let tagged value =
        Tuple
          {
            conjuncts = [Primitive Primitive.Int; value];
            optional_conjuncts = [];
            open_ = false;
          }
      in
      bind_value ty value (fun value ->
          let receiver = Syntax.fresh_local "tagged" in
          let wide = Syntax.fresh_local "wide" in
          let replacement = Syntax.fresh_local "replacement" in
          let put receiver =
            Syntax.Eval
              (Syntax.Call
                 ( Syntax.Atom (name ^ "_write<" ^ show (tagged narrower) ^ ">"),
                   [Syntax.Local receiver; Syntax.Local replacement] ))
          in
          [
            Syntax.Bind
              ( receiver,
                Syntax.New
                  ( show (GenericClass { name; key; value = tagged ty }),
                    [
                      inhabitant renv env key;
                      Syntax.Tuple [Syntax.Atom "0"; value];
                    ] ) );
            Syntax.Bind
              ( wide,
                member
                  (Syntax.Local receiver)
                  "widen<mixed>"
                  [Syntax.Atom "null"] );
            Syntax.Eval
              (Syntax.Call
                 (Syntax.Atom (name ^ "_read"), [Syntax.Local receiver]));
            Syntax.Bind
              ( replacement,
                Syntax.Tuple [Syntax.Atom "1"; inhabitant renv env narrower] );
            put receiver;
            put wide;
            Syntax.Eval
              (Syntax.Call
                 ( Syntax.Atom "invariant",
                   [
                     Syntax.Binary
                       ( "===",
                         Syntax.Array
                           ("vec", [member (Syntax.Local wide) "get" []]),
                         Syntax.Array ("vec", [Syntax.Local replacement]) );
                     Syntax.Atom
                       "'contravariant write preserves the new payload'";
                   ] ));
            returning
              (Syntax.Index
                 (member (Syntax.Local receiver) "get" [], Syntax.Atom "1"));
          ])
    in
    ( env,
      [
        read;
        write;
        project;
        widen;
        keyed;
        invalidate;
        tagged_write;
        (fun value ->
          Syntax.Call
            (Syntax.StaticMember (name, "identity<" ^ show ty ^ ">"), [value]));
      ] )

  let dependent_operations renv env ty =
    let bound =
      if Random.bool () then
        ty
      else
        Mixed
    in
    let (env, dependent) =
      declare_dependent env ~value_type:ty ~bound:(Some bound)
    in
    let name =
      match dependent with
      | Dependent { name } -> name
      | _ -> failwith "Expected a dependent type"
    in
    let info = Env.get_dependent env name in
    let construct value = Syntax.New (info.Env.concrete, [value]) in
    let read value =
      Syntax.Call
        ( Syntax.Atom (info.Env.read_function ^ "<" ^ show ty ^ ">"),
          [construct value] )
    in
    let read_bound value =
      bind_value ty value (fun value ->
          let result =
            Syntax.Call
              ( Syntax.Atom (info.Env.read_function ^ "_bound"),
                [construct value] )
          in
          if equal bound ty then
            [returning result]
          else
            [
              Syntax.Eval
                (Syntax.Call
                   ( Syntax.Atom "invariant",
                     [
                       Syntax.Binary
                         ( "===",
                           Syntax.Array ("vec", [result]),
                           Syntax.Array ("vec", [value]) );
                       Syntax.Atom "'upper-bound read preserves the payload'";
                     ] ));
              returning value;
            ])
    in
    let write value =
      let receiver = Syntax.fresh_local "dependent" in
      let input = Syntax.fresh_local "input" in
      apply
        dependent
        [
          Syntax.parameter
            (info.Env.base ^ " with { type Item = " ^ show ty ^ " }")
            receiver;
          Syntax.parameter (show ty) input;
        ]
        [
          Syntax.Eval (member (Syntax.Local receiver) "set" [Syntax.Local input]);
          returning (member (Syntax.Local receiver) "get" []);
        ]
        [construct (inhabitant renv env ty); value]
    in
    (env, [read; read_bound; write])

  let identity_operations _renv env ty =
    let (env, family) = declare_identity_family env ~payload:(Some ty) in
    let pointer () =
      if Random.bool () then
        forward
          (show family.identity_child_pointer)
          (show family.identity_base_pointer)
          (Syntax.StaticMember (family.identity_child, "class"))
      else
        Syntax.StaticMember (family.identity_base, "class")
    in
    let classname () =
      if Random.bool () then
        forward
          (show family.identity_child_name)
          (show family.identity_base_name)
          (Syntax.Nameof family.identity_child)
      else
        Syntax.Nameof family.identity_base
    in
    let construct value =
      let class_pointer = Syntax.fresh_local "class_pointer" in
      let input = Syntax.fresh_local "input" in
      apply
        ty
        [
          Syntax.parameter (show family.identity_base_pointer) class_pointer;
          Syntax.parameter (show ty) input;
        ]
        [
          returning
            (member
               (Syntax.NewDynamic (class_pointer, [Syntax.Local input]))
               "get"
               []);
        ]
        [pointer (); value]
    in
    let named_construct value =
      let name = Syntax.fresh_local "class_name" in
      let class_pointer = Syntax.fresh_local "class_pointer" in
      let input = Syntax.fresh_local "input" in
      let name_value =
        if Random.bool () then
          classname ()
        else
          forward
            (show family.identity_base_pointer)
            (show family.identity_base_name)
            (pointer ())
      in
      apply
        ty
        [
          Syntax.parameter (show family.identity_base_name) name;
          Syntax.parameter (show ty) input;
        ]
        [
          Syntax.Bind
            ( class_pointer,
              Syntax.Call
                (Syntax.Atom "HH\\classname_to_class", [Syntax.Local name]) );
          returning
            (member
               (Syntax.NewDynamic (class_pointer, [Syntax.Local input]))
               "get"
               []);
        ]
        [name_value; value]
    in
    let late_static ~object_receiver value =
      bind_value ty value (fun value ->
          let receiver = Syntax.fresh_local "identity_receiver" in
          let class_pointer = Syntax.fresh_local "identity_pointer" in
          let input = Syntax.fresh_local "input" in
          let receiver_type =
            if object_receiver then
              family.identity_base
            else
              show family.identity_base_pointer
          in
          let receiver_value =
            if object_receiver then
              Syntax.New
                ( (if Random.bool () then
                    family.identity_base
                  else
                    family.identity_child),
                  [value] )
            else
              pointer ()
          in
          [
            returning
              (apply
                 ty
                 [
                   Syntax.parameter receiver_type receiver;
                   Syntax.parameter (show ty) input;
                 ]
                 [
                   Syntax.Bind
                     ( class_pointer,
                       Syntax.Call
                         ( Syntax.Atom "HH\\classname_to_class",
                           [
                             Syntax.Call
                               ( Syntax.DynamicStaticMember
                                   (receiver, "identity"),
                                 [] );
                           ] ) );
                   returning
                     (member
                        (Syntax.NewDynamic (class_pointer, [Syntax.Local input]))
                        "get"
                        []);
                 ]
                 [receiver_value; value]);
          ])
    in
    let nominal_test value =
      let receiver = Syntax.fresh_local "identity_object" in
      apply
        ty
        [Syntax.parameter family.identity_base receiver]
        [
          Syntax.If
            ( Syntax.Is (Syntax.Local receiver, family.identity_child),
              [returning (member (Syntax.Local receiver) "get" [])],
              [
                returning
                  (member
                     (Syntax.New
                        ( family.identity_child,
                          [member (Syntax.Local receiver) "get" []] ))
                     "get"
                     []);
              ] );
        ]
        [
          Syntax.New
            ( (if Random.bool () then
                family.identity_base
              else
                family.identity_child),
              [value] );
        ]
    in
    ( env,
      [
        construct;
        named_construct;
        late_static ~object_receiver:true;
        late_static ~object_receiver:false;
        nominal_test;
      ] )

  let enum_operations renv env ty =
    let payload =
      if equal ty Mixed then
        Primitive (select [Primitive.Arraykey; Primitive.Num])
      else
        ty
    in
    let value_renv = REnv.for_enum_initializer renv in
    let narrower = subtype_of value_renv env payload in
    let (env, family) = declare_enum_family value_renv env ~payload ~narrower in
    let (env, _) = record_enum_views env family ~is_label:false in
    let (env, _) = record_enum_views env family ~is_label:true in
    let unwrap_name = fresh "unwrap_member" in
    let env =
      Env.add_definition
        env
        (Definition.enum_unwrap ~name:unwrap_name ~enum_name:family.enum_child)
    in
    let view ~is_label owner payload =
      show (enum_view ~is_label owner "A" payload)
    in
    let unwrap payload value =
      Syntax.Call (Syntax.Atom (unwrap_name ^ "<" ^ show payload ^ ">"), [value])
    in
    let lookup payload label =
      Syntax.Call
        ( Syntax.StaticMember
            ( family.enum_child,
              "valueOf<" ^ family.enum_child ^ ", " ^ show payload ^ ">" ),
          [label] )
    in
    let inherited ~is_label =
      let value =
        if is_label then
          Syntax.EnumLabel (family.enum_base, "A")
        else
          Syntax.StaticMember (family.enum_base, "A")
      in
      forward
        (view ~is_label family.enum_base payload)
        (view ~is_label family.enum_child payload)
        value
    in
    let widened ~is_label =
      let value =
        if is_label then
          Syntax.EnumLabel (family.enum_child, "B")
        else
          Syntax.StaticMember (family.enum_child, "B")
      in
      forward
        (view ~is_label family.enum_child narrower)
        (view ~is_label family.enum_child payload)
        value
    in
    ( env,
      [
        (fun _ -> unwrap payload (Syntax.StaticMember (family.enum_child, "A")));
        (fun _ ->
          unwrap
            payload
            (lookup payload (Syntax.EnumLabel (family.enum_child, "A"))));
        (fun _ ->
          unwrap narrower (Syntax.StaticMember (family.enum_child, "B")));
        (fun _ ->
          unwrap
            narrower
            (lookup narrower (Syntax.EnumLabel (family.enum_child, "B"))));
        (fun _ -> unwrap payload (inherited ~is_label:false));
        (fun _ -> unwrap payload (lookup payload (inherited ~is_label:true)));
        (fun _ -> unwrap payload (widened ~is_label:false));
        (fun _ -> unwrap payload (lookup payload (widened ~is_label:true)));
      ] )

  let callable_operations renv env ty =
    let (env, callable_ty) =
      mk ~kind:Kind.Function renv env ~depth:None ~complexity:2
    in
    let complete callable_ty call value =
      match callable_ty with
      | Function { return_ = ReturnsNothing; _ } ->
        let caught = Syntax.fresh_local "exception" in
        [
          Syntax.Try
            ([Syntax.Eval call], [("Exception", caught, [returning value])], []);
          returning value;
        ]
      | Function { return_ = ReturnsVoid; _ } ->
        [Syntax.Eval call; returning value]
      | Function { return_ = ReturnsValue _; _ } ->
        [Syntax.Bind (Syntax.fresh_local "result", call); returning value]
      | _ -> failwith "Expected a callable"
    in
    let invoke value =
      let callback = Syntax.fresh_local "callback" in
      let input = Syntax.fresh_local "input" in
      let subtype = subtype_of renv env callable_ty in
      apply
        ty
        [
          Syntax.parameter (show callable_ty) callback;
          Syntax.parameter (show ty) input;
        ]
        (complete
           callable_ty
           (callable_application
              renv
              env
              callable_ty
              ~value:(Syntax.Local callback))
           (Syntax.Local input))
        [
          forward (show subtype) (show callable_ty) (inhabitant renv env subtype);
          value;
        ]
    in
    let procedure value =
      let return_ =
        if Random.bool () then
          ReturnsVoid
        else
          ReturnsNothing
      in
      let effect_state =
        match callable_ty with
        | Function { effect_state; _ } -> effect_state
        | _ -> failwith "Expected a callable"
      in
      let procedure_ty =
        Function
          {
            parameters = [];
            variadic = None;
            return_;
            context = function_context return_;
            effect_state;
          }
      in
      bind_value ty value (fun value ->
          let callback = inhabitant renv env procedure_ty in
          let call =
            if Random.bool () then
              Syntax.Call (Syntax.Atom "milner_procedure", [callback])
            else
              Syntax.Call
                ( Syntax.Atom "HH\\Asio\\join",
                  [
                    Syntax.Call
                      (Syntax.Atom "milner_procedure_async", [callback]);
                  ] )
          in
          complete procedure_ty call value)
    in
    let effect value =
      let effect_state =
        match callable_ty with
        | Function { effect_state; _ } -> effect_state
        | _ -> failwith "Expected a callable"
      in
      bind_value ty value (fun value ->
          let state = Syntax.fresh_local "state" in
          let result = Syntax.fresh_local "effect_result" in
          let (context, initialize, slot) =
            if Random.bool () then
              let slot = Syntax.StaticProperty (effect_state, "calls") in
              (["globals"], Syntax.Assign (slot, Syntax.Atom "0"), slot)
            else
              ( ["write_props"],
                Syntax.Bind (state, Syntax.New (effect_state, [])),
                Syntax.Member (Syntax.Local state, "value") )
          in
          [
            initialize;
            Syntax.Bind
              ( result,
                Syntax.Call
                  ( Syntax.Atom ("milner_invoke<" ^ show ty ^ ">"),
                    [
                      Syntax.Lambda
                        ( [],
                          context,
                          show ty,
                          [
                            Syntax.Eval (Syntax.Unary ("++", slot));
                            returning value;
                          ] );
                    ] ) );
            returning
              (Syntax.Index
                 ( Syntax.Array ("vec", [Syntax.Local result]),
                   Syntax.Binary ("-", slot, Syntax.Atom "1") ));
          ])
    in
    (env, [invoke; procedure; effect])

  let add_fixture env source =
    if
      List.exists (Env.definitions env) ~f:(fun definition ->
          String.equal (Definition.show definition) source)
    then
      env
    else
      Env.add_definition env (Definition.raw source)

  let protocol_operations _renv env ty =
    if equal ty Mixed && Random.bool () then
      let env = add_fixture env Milner_protocol_fixture.expression_tree in
      (env, [Milner_protocol_bindings.expression_tree ~value_hint:(show ty)])
    else
      let env = add_fixture env Milner_protocol_fixture.xhp in
      let witness =
        Milner_protocol_bindings.xhp
          ~name:(fresh "milner-node")
          ~value_hint:(show ty)
          ~child:(string_literal ())
      in
      let env =
        List.fold
          witness.Milner_protocol_bindings.definitions
          ~init:env
          ~f:(fun env source -> Env.add_definition env (Definition.raw source))
      in
      (env, witness.Milner_protocol_bindings.operations)

  let inhabitant_of renv env ty =
    let env =
      add_fixture
        env
        "function milner_invoke<T>((function()[_]: T) $callback)[ctx $callback]: T { return $callback(); }"
    in
    let env =
      add_fixture
        env
        "function milner_procedure((function()[_]: void) $callback)[ctx $callback]: void { $callback(); }"
    in
    let env =
      add_fixture
        env
        "async function milner_procedure_async((function()[_]: void) $callback)[ctx $callback]: Awaitable<void> { milner_procedure($callback); }"
    in
    let builders =
      [
        hierarchy_operations;
        generic_operations;
        dependent_operations;
        identity_operations;
        callable_operations;
        protocol_operations;
      ]
    in
    let builders =
      match ty with
      | Mixed
      | Primitive _ ->
        enum_operations :: builders
      | _ -> builders
    in
    let (env, operations) =
      List.init (1 + Random.int 3) ~f:(fun _ -> select builders)
      |> List.fold ~init:(env, []) ~f:(fun (env, operations) build ->
             let (env, added) = build renv env ty in
             (env, operations @ added))
    in
    let value = inhabitant renv env ty in
    let expression =
      Milner_expression.compose ~ty:(show ty) ~value ~operations
    in
    (env, Syntax.render_expr expression)

  let mk renv env = mk renv env ~depth:None ~complexity:default_complexity
end

and TypeMap : (Wrapped_map.S with type key = Type.t) = Wrapped_map.Make (Type)
and TypeSet : (Stdlib.Set.S with type elt = Type.t) = Stdlib.Set.Make (Type)
