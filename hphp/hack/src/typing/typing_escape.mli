open Typing_defs
open Typing_env_types

type snapshot

type escaping_rigid_tvars

val snapshot_env : env -> snapshot

val escaping_from_snapshot : snapshot -> env -> escaping_rigid_tvars

val refresh_env_and_type :
  remove:escaping_rigid_tvars -> pos:Pos.t -> env -> locl_ty -> env * locl_ty
