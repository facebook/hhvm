(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type pos = Relative_path.t * int * int

val pos_to_json : Relative_path.t -> int -> int -> Hh_json.json

type 'a walker = {
  plus: 'a -> 'a -> 'a;
  on_method: Tast_env.env -> Tast.method_ -> 'a;
  on_fun_def: Tast_env.env -> Tast.fun_def -> 'a;
}

type ('a, 'r, 's) handlers = {
  result_to_string: ('r option, string) result -> pos -> string;
  walker: 'a walker;
  get_state: Provider_context.t -> Relative_path.t -> 's;
  map_result: Provider_context.t -> 's -> 'a -> 'r;
}

val go :
  MultiWorker.worker list option ->
  (string * int * int) list ->
  ServerEnv.env ->
  ('a, 'b, 'c) handlers ->
  string list

(** For test: *)
val helper :
  ('a, 'b, 'c) handlers ->
  Provider_context.t ->
  string list ->
  pos list ->
  string list
