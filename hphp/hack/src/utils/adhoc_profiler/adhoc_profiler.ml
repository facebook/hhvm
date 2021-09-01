(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type seconds = float

module Hashtbl = struct
  include Caml.Hashtbl

  let create () = create ~random:false 10

  let to_list t = fold (fun k v l -> (k, v) :: l) t []

  let find_or_add t k ~default =
    match find_opt t k with
    | Some v -> v
    | None ->
      let v = default () in
      add t k v;
      v
end

(** Counter for the number of times a function is called and the total duration it has run. *)
module Counter = struct
  type t = {
    count: int;  (** The number of times the function is called *)
    time: seconds;  (** The total time the function has run. *)
  }

  let init : t = { count = 0; time = 0. }

  let count ~start ~end_ { count; time } =
    { count = count + 1; time = time +. end_ -. start }

  let to_string ~total_time { count; time } =
    let percentage = 100. *. time /. total_time in
    Printf.sprintf "%-12d%.3f sec   %.2f%%" count time percentage
end

(** A node in the tree of counters which constitutes the profile. *)
module Node = struct
  type t = {
    mutable data: Counter.t;
    children:
      (* Use Hashtbl instead of Map for better perf. *)
      (string, t) Hashtbl.t;
  }

  let init : unit -> t =
   (fun () -> { data = Counter.init; children = Hashtbl.create () })

  let to_string : name:string -> t -> string =
   fun ~name node ->
    let total_time = node.data.Counter.time in
    let tab_len = 2 in
    let rec to_string ntabs name { data; children } =
      let tabs = String.make (ntabs * tab_len) ' ' in
      let node_s =
        Printf.sprintf
          "%s%-80s%s\n"
          tabs
          name
          (Counter.to_string ~total_time data)
      in
      let children_s =
        children
        |> Hashtbl.to_list
        |> List.sort ~compare:(fun (name1, _) (name2, _) ->
               String.compare name1 name2)
        |> List.map ~f:(fun (name, node) -> to_string (ntabs + 1) name node)
      in
      String.concat (node_s :: children_s)
    in
    to_string 0 name node
end

(** The profiler structure. A set of named roots. *)
type t = (string, Node.t) Hashtbl.t

let get_time_ref : (unit -> seconds) ref = ref Sys.time

let get_time () = !get_time_ref ()

module Test = struct
  let set_time_getter get_time = get_time_ref := get_time
end

let count_with_node : Node.t -> (t -> 'result) -> 'result =
 fun node f ->
  let { Node.data; children } = node in
  let start = get_time () in
  let result = f children in
  let end_ = get_time () in
  let data = Counter.count data ~start ~end_ in
  node.Node.data <- data;
  result

let count : t -> name:string -> (t -> 'result) -> 'result =
 fun nodes ~name f ->
  let node = Hashtbl.find_or_add nodes name ~default:Node.init in
  count_with_node node f

let count_leaf : t -> name:string -> (unit -> 'result) -> 'result =
 fun nodes ~name f ->
  let f _nodes = f () in
  count nodes ~name f

let init () : t = Hashtbl.create ()

let profilers : (string, Node.t) Hashtbl.t = Hashtbl.create ()

let create : name:string -> (t -> 'result) -> 'result =
 fun ~name f ->
  let node = Hashtbl.find_or_add profilers name ~default:Node.init in
  count_with_node node f

let without_profiling : (prof:t -> 'result) -> 'result =
 (fun f -> f ~prof:(init ()))

let get_and_reset : unit -> t =
 fun () ->
  let prof = Hashtbl.copy profilers in
  Hashtbl.clear profilers;
  prof

let to_string : t -> string =
 fun nodes ->
  nodes
  |> Hashtbl.to_list
  |> List.sort ~compare:(fun (name1, _) (name2, _) ->
         String.compare name1 name2)
  |> List.map ~f:(fun (name, node) -> Node.to_string ~name node)
  |> String.concat
