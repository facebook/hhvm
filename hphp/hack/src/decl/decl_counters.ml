(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let mode = ref HackEventLogger.PerFileProfilingConfig.DeclingOff

type decl_kind =
  | Class
  | Fun
  | GConst
  | Typedef
  | Module_decl
[@@deriving show { with_path = false }]

type origin =
  | Body
  | TopLevel
  | Tast
  | Variance
  | NastCheck
  | TastCheck
[@@deriving show { with_path = false }]

type subdecl_kind =
  (* Shallow *)
  | Abstract
  | Final
  | Const
  | Kind
  | Is_xhp
  | Name
  | Module
  | Internal
  | IsModuleLevelTrait
  | Pos
  | Tparams
  | Where_constraints
  | Enum_type
  | Xhp_enum_values
  | Xhp_marked_empty
  | Sealed_whitelist
  | Docs_url
  | Decl_errors
  | Support_dynamic_type
  (* Lazy *)
  | Construct
  | Need_init
  | Get_ancestor of string [@printer (fun fmt _s -> fprintf fmt "Get_ancestor")]
  | Has_ancestor of string [@printer (fun fmt _s -> fprintf fmt "Has_ancestor")]
  | Requires_ancestor of string
      [@printer (fun fmt _s -> fprintf fmt "Requires_ancestor")]
  | Extends of string [@printer (fun fmt _s -> fprintf fmt "Extends")]
  | Is_disposable
  | Get_const of string [@printer (fun fmt _s -> fprintf fmt "Get_const")]
  | Has_const of string [@printer (fun fmt _s -> fprintf fmt "Has_const")]
  | Get_typeconst of string
      [@printer (fun fmt _s -> fprintf fmt "Get_typeconst")]
  | Has_typeconst of string
      [@printer (fun fmt _s -> fprintf fmt "Has_typeconst")]
  | Get_typeconst_enforceability of string
      [@printer (fun fmt _s -> fprintf fmt "Get_typeconst_enforceability")]
  | Get_prop of string [@printer (fun fmt _s -> fprintf fmt "Get_prop")]
  | Has_prop of string [@printer (fun fmt _s -> fprintf fmt "Has_prop")]
  | Get_sprop of string [@printer (fun fmt _s -> fprintf fmt "Get_sprop")]
  | Has_sprop of string [@printer (fun fmt _s -> fprintf fmt "Has_sprop")]
  | Get_method of string [@printer (fun fmt _s -> fprintf fmt "Get_method")]
  | Has_method of string [@printer (fun fmt _s -> fprintf fmt "Has_method")]
  | Get_smethod of string [@printer (fun fmt _s -> fprintf fmt "Get_smethod")]
  | Has_smethod of string [@printer (fun fmt _s -> fprintf fmt "Has_smethod")]
  (* Eager *)
  | Members_fully_known
  | All_ancestor_req_names
  | All_extends_ancestors
  | All_ancestors
  | All_ancestor_names
  | All_ancestor_reqs
  | Consts
  | Typeconsts
  | Props
  | SProps
  | Methods
  | SMethods
  | Overridden_method
  (* Misc *)
  | Deferred_init_members
[@@deriving show { with_path = false }]

type tracing_info = {
  origin: origin;
  file: Relative_path.t;
}

let subdecl_member_name (subdecl_kind : subdecl_kind) : string option =
  match subdecl_kind with
  | Get_ancestor s
  | Has_ancestor s
  | Requires_ancestor s
  | Extends s
  | Get_const s
  | Has_const s
  | Get_typeconst s
  | Has_typeconst s
  | Get_prop s
  | Has_prop s
  | Get_sprop s
  | Has_sprop s
  | Get_method s
  | Has_method s
  | Get_smethod s
  | Has_smethod s ->
    Some s
  | _ -> None

let subdecl_eagerness (subdecl_kind : subdecl_kind) : string =
  match subdecl_kind with
  | Abstract
  | Final
  | Const
  | Kind
  | Is_xhp
  | Name
  | Module
  | Internal
  | IsModuleLevelTrait
  | Pos
  | Tparams
  | Where_constraints
  | Enum_type
  | Xhp_enum_values
  | Xhp_marked_empty
  | Sealed_whitelist
  | Docs_url
  | Decl_errors
  | Support_dynamic_type ->
    "shallow"
  | Construct
  | Need_init
  | Get_ancestor _
  | Has_ancestor _
  | Requires_ancestor _
  | Extends _
  | Is_disposable
  | Get_const _
  | Has_const _
  | Get_typeconst _
  | Has_typeconst _
  | Get_typeconst_enforceability _
  (* Get_typeconst_enforceability is technically lazy, but much less so than Get_typeconst *)
  | Get_prop _
  | Has_prop _
  | Get_sprop _
  | Has_sprop _
  | Get_method _
  | Has_method _
  | Get_smethod _
  | Has_smethod _ ->
    "lazy"
  | Members_fully_known
  | All_ancestor_req_names
  | All_extends_ancestors
  | All_ancestors
  | All_ancestor_names
  | All_ancestor_reqs
  | Consts
  | Typeconsts
  | Props
  | SProps
  | Methods
  | SMethods
  | Overridden_method ->
    "eager"
  | Deferred_init_members -> "misc"

(** This type provides context that flows from the time someone accessed the top-level Class
decl, through to the time that they accessed a subsidiary decl like Get_method. It's used
solely in telemetry. The Decl_id would be enough alone to do a JOIN on the telemetry to
relate the decl to the subdecl. But for convenience we're also carrying additional information
to not even need the join. *)
type decl = {
  decl_id: string;
      (** a randomly generated ID unique to this particular top-level retrieval,
      i.e. invocation of get_class / get_fun / ... *)
  decl_name: string;
      (** name of the top-level class/fun/... that was retrieved *)
  decl_origin: origin option;
      (** a classification of what larger purpose the code was engaged in,
      at the moment of top-level decl retrieval. The caller of get_class / get_fun / ...
      will opt into self-describing itself with a suitable origin. As mentioned above,
      this is used solely for telemetry classification. *)
  decl_file: Relative_path.t option;
      (** the file we were typechecking at the moment of top-level decl retrieval.
      The caller of get_class / get_fun / ... will opt into providing a
      filename. *)
  decl_callstack: string option;
      (** callstack at the moment of top-level decl retrieval. This is
      optional in case the user opted out of (costly) callstack fetching. *)
  decl_start_time: float;
      (** wall-time at the moment of top-level decl retrieval *)
}

let set_mode (new_mode : HackEventLogger.PerFileProfilingConfig.profile_decling)
    : unit =
  mode := new_mode

let count_decl
    ?(tracing_info : tracing_info option)
    (decl_kind : decl_kind)
    (decl_name : string)
    (f : decl option -> 'a) : 'a =
  match !mode with
  | HackEventLogger.PerFileProfilingConfig.DeclingOff ->
    (* CARE! This path must be highly performant. *)
    f None
  | HackEventLogger.PerFileProfilingConfig.DeclingTopCounts ->
    Counters.count Counters.Category.Decl_provider_get (fun () -> f None)
  | HackEventLogger.PerFileProfilingConfig.DeclingAllTelemetry { callstacks } ->
    let start_time = Unix.gettimeofday () in
    let start_cpu_time = Sys.time () in
    let decl_id = Random_id.short_string () in
    let decl_callstack =
      if callstacks then
        Some (Exception.get_current_callstack_string 99 |> Exception.clean_stack)
      else
        None
    in
    let decl =
      {
        decl_id;
        decl_name;
        decl_callstack;
        decl_origin = Option.map tracing_info ~f:(fun ti -> ti.origin);
        decl_file = Option.map tracing_info ~f:(fun ti -> ti.file);
        decl_start_time = start_time;
      }
    in
    let result = f (Some decl) in
    HackEventLogger.ProfileDecl.count_decl
      ~kind:(show_decl_kind decl_kind)
      ~cpu_duration:(Sys.time () -. start_cpu_time)
      ~decl_id
      ~decl_name
      ~decl_origin:(Option.map decl.decl_origin ~f:show_origin)
      ~decl_file:decl.decl_file
      ~decl_callstack
      ~decl_start_time:start_time
      ~subdecl_member_name:None
      ~subdecl_eagerness:None
      ~subdecl_callstack:None
      ~subdecl_start_time:None;
    result

let count_subdecl
    (decl : decl option) (subdecl_kind : subdecl_kind) (f : unit -> 'a) : 'a =
  match decl with
  | None ->
    (* CARE! This path must be highly performant. It's called tens of thousands of times
       per file. The earlier function count_decl had used "decl=None" to signal that no logging
       is needed for subdecl accesses, and here we're (cheaply) picking up that fact. *)
    f ()
  | Some decl ->
    let start_time = Unix.gettimeofday () in
    let start_cpu_time = Sys.time () in
    let subdecl_get_callstack =
      Option.map decl.decl_callstack ~f:(fun _ ->
          Exception.get_current_callstack_string 99 |> Exception.clean_stack)
    in
    let result = f () in
    HackEventLogger.ProfileDecl.count_decl
      ~kind:("Class." ^ show_subdecl_kind subdecl_kind)
      ~cpu_duration:(Sys.time () -. start_cpu_time)
      ~decl_id:decl.decl_id
      ~decl_name:decl.decl_name
      ~decl_origin:(Option.map decl.decl_origin ~f:show_origin)
      ~decl_file:decl.decl_file
      ~decl_callstack:decl.decl_callstack
      ~decl_start_time:decl.decl_start_time
      ~subdecl_member_name:(subdecl_member_name subdecl_kind)
      ~subdecl_eagerness:(Some (subdecl_eagerness subdecl_kind))
      ~subdecl_callstack:subdecl_get_callstack
      ~subdecl_start_time:(Some start_time);
    result
