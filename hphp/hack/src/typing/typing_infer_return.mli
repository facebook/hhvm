type t =
  Typing_env.env *
  Pos.t *
  Typing_suggest.hint_kind *
  Typing_defs.locl Typing_defs.ty

val print_type_locl: Typing_env.env -> 'a Typing_defs.ty -> string

val print_type: t -> string

val print_list: f:('a -> string) -> 'a list -> string

val format_types: Nast.sid -> t list -> unit

val typing_env_from_file: GlobalOptions.t -> Relative_path.t -> Typing_env.env

val type_from_hint: GlobalOptions.t -> Relative_path.t -> Nast.hint ->
  Typing_env.env * Typing_defs.locl Typing_defs.ty

val get_inferred_types: GlobalOptions.t -> Relative_path.S.t list ->
  process:(Nast.sid -> t list -> unit) -> unit

val get_matching_types: TypecheckerOptions.t -> Nast.def -> Nast.sid -> t list
