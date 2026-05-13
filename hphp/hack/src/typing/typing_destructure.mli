(** Typing of shape and tuple destructuring patterns. *)

(** [shape ~assign_pos ~pat_pos env ds rhs_ty] types shape
    destructuring pattern [ds] against [rhs_ty]. [assign_pos]
    is the enclosing assignment/foreach position for variable
    binding. [pat_pos] is the position of the destructure
    pattern itself. *)
val shape :
  assign_pos:Pos.t ->
  pat_pos:Pos.t ->
  Typing_env_types.env ->
  Nast.destructure_shape ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Tast.expr * Typing_defs.locl_ty

(** [tuple ~assign_pos ~pat_pos env dt rhs_ty] types tuple
    destructuring pattern [dt] against [rhs_ty]. *)
val tuple :
  assign_pos:Pos.t ->
  pat_pos:Pos.t ->
  Typing_env_types.env ->
  Nast.destructure_tuple ->
  Typing_defs.locl_ty ->
  Typing_env_types.env * Tast.expr * Typing_defs.locl_ty
