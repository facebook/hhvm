module Env = Typing_env
module Log = Typing_log
module TySet = Typing_set

let local_env_size env =
  match Env.next_cont_opt env with
  | None -> 0
  | Some Typing_per_cont_env.{ local_types; _ } ->
    Local_id.Map.fold (fun _ (ty, _) size -> size + (Typing_utils.ty_size env ty)) local_types 0

let ty_set_size env tyset =
  TySet.fold (fun ty size -> size + Typing_utils.ty_size env ty) tyset 0

let tvenv_size env =
  IMap.fold
    (fun _ { Env.lower_bounds; Env.upper_bounds; _ } size ->
      size + ty_set_size env lower_bounds + ty_set_size env upper_bounds) env.Env.tvenv 0

let env_size env =
  local_env_size env +
  tvenv_size env

let log_env_if_too_big pos env =
  if (Env.get_tcopt env).GlobalOptions.tco_timeout > 0
    && List.length !(env.Env.big_envs) < 1
    && env_size env >= 1000
  then
    env.Env.big_envs := (pos, env) :: !(env.Env.big_envs)
