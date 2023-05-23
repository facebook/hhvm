(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** This opaque type carries decl_counter-specific context that will be used
in telemetry to relate the subdecl accessors like "get member foo of class",
back to the original decl accessor "get the class named Bar" which got this
specific class in the first place. *)
type decl

type decl_kind =
  | Class
  | Fun
  | GConst
  | Typedef
  | Module_decl

(** The idea of [origin] is so that callsites have a way of characterizing
what they are, and have this characterization end up in telemetry.
That's all. *)
type origin =
  | Body  (** block, expr, statement *)
  | TopLevel  (** typing_toplevel stuff, well-formedness *)
  | Tast  (** tast_env *)
  | Variance  (** typing_variance *)
  | NastCheck  (** typing/nast_check *)
  | TastCheck  (** typing/tast_check *)

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
  | Get_ancestor of string
  | Has_ancestor of string
  | Requires_ancestor of string
  | Extends of string
  | Is_disposable
  | Get_const of string
  | Has_const of string
  | Get_typeconst of string
  | Has_typeconst of string
  | Get_typeconst_enforceability of string
  | Get_prop of string
  | Has_prop of string
  | Get_sprop of string
  | Has_sprop of string
  | Get_method of string
  | Has_method of string
  | Get_smethod of string
  | Has_smethod of string
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

type tracing_info = {
  origin: origin;
  file: Relative_path.t;
}

(** E.g. 'count_decl Class "Foo" (fun d -> <implementation>)'. The idea is that
<implementation> is the body of work which fetches the decl of class "foo",
and we want to record telemetry about its performance. This function will
execute <implementation>, and will internally keep track of things like how long
it took and what decl was fetched, and will log the telemetry appropriately.

The callback takes a parameter [d]. This is for use of subsidiary decl accessors,
e.g. 'count_subdecl d (Get_method "bar") (fun () -> <implementation2>)'. It's
so that, when we record telemetry about <implementation2>, we can relate it
back to the original count_decl which fetched the class Foo in the first place.
[d] is optional; it will be None if e.g. the original call to count_decl established
that no telemetry should be collected during this run, and we want to avoid
re-establishing this fact during the call to count_subdecl. *)
val count_decl :
  ?tracing_info:tracing_info -> decl_kind -> string -> (decl option -> 'a) -> 'a

(** E.g. 'count_subdecl d (Get_method "bar") (fun () -> <implementation2>)'. The idea
is that <implementation2> is the body of work which fetches the decl of method 'bar'
of some class, and we want to record telemetry about its performance. This function
will execution <implementation2> , and will internally keep track of things like
how long it took, and will log the telemetry appropriately. *)
val count_subdecl : decl option -> subdecl_kind -> (unit -> 'a) -> 'a

val set_mode : HackEventLogger.PerFileProfilingConfig.profile_decling -> unit
