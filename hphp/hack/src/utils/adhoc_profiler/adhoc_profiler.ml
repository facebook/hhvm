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

  let merge :
      (string, 'a) t ->
      (string, 'b) t ->
      f:
        (key:string ->
        [< `Both of 'a * 'b | `Left of 'a | `Right of 'b ] ->
        'c option) ->
      (string, 'c) t =
   fun t1 t2 ~f ->
    let module SMap = Caml.Map.Make (String) in
    let t1 = to_seq t1 |> SMap.of_seq in
    let t2 = to_seq t2 |> SMap.of_seq in
    let t = create () in
    SMap.merge
      (fun key left right ->
        match (left, right) with
        | (None, None) -> None
        | (Some left, None) -> f ~key (`Left left)
        | (None, Some right) -> f ~key (`Right right)
        | (Some left, Some right) -> f ~key (`Both (left, right)))
      t1
      t2
    |> SMap.iter (fun key data -> add t key data);
    t
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

  let add { count = count1; time = time1 } { count = count2; time = time2 } =
    { count = count1 + count2; time = time1 +. time2 }
end

(** A node in the tree of counters which constitutes the profile. *)
module Node = struct
  type t = {
    mutable data: Counter.t;
    children:
      (* Use Hashtbl instead of Map for better perf. *)
      (string, t) Hashtbl.t;
  }

  let make : unit -> t =
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

module CallTree = struct
  (** The profiling data. A set of named roots. *)
  type t = (string, Node.t) Hashtbl.t

  let make () = Hashtbl.create ()

  let rec merge : t -> t -> t =
   fun p1 p2 ->
    Hashtbl.merge p1 p2 ~f:(fun ~key:_ -> function
      | `Left n
      | `Right n ->
        Some n
      | `Both
          ( { Node.data = data1; children = children1 },
            { Node.data = data2; children = children2 } ) ->
        Some
          {
            Node.data = Counter.add data1 data2;
            children = merge children1 children2;
          })

  let to_string : t -> string =
   fun nodes ->
    nodes
    |> Hashtbl.to_list
    |> List.sort ~compare:(fun (name1, _) (name2, _) ->
           String.compare name1 name2)
    |> List.map ~f:(fun (name, node) -> Node.to_string ~name node)
    |> String.concat
end

(** The profiler structure. *)
type prof = {
  nodes: CallTree.t;
  depth: int;
      (** Used to limit depth of stacks. Useful especially with recursive functions. *)
}

type t = prof option

let get_time_ref : (unit -> seconds) ref = ref Sys.time

let get_time () = !get_time_ref ()

module Test = struct
  let set_time_getter get_time = get_time_ref := get_time
end

let max_depth = 10

let count_with_node : Node.t * int -> (t -> 'result) -> 'result =
 fun (node, depth) f ->
  let { Node.data; children } = node in
  let start = get_time () in
  let result = f (Some { nodes = children; depth = depth + 1 }) in
  let end_ = get_time () in
  let data = Counter.count data ~start ~end_ in
  node.Node.data <- data;
  result

let count : t -> name:string -> (t -> 'result) -> 'result =
 fun prof ~name f ->
  match prof with
  | None -> f None
  | Some { nodes; depth } ->
    if depth > max_depth then
      f None
    else
      let node = Hashtbl.find_or_add nodes name ~default:Node.make in
      count_with_node (node, depth) f

let count_leaf : t -> name:string -> (unit -> 'result) -> 'result =
 fun nodes ~name f ->
  let f _nodes = f () in
  count nodes ~name f

let profilers : CallTree.t = CallTree.make ()

let create : name:string -> (t -> 'result) -> 'result =
 fun ~name f ->
  let node = Hashtbl.find_or_add profilers name ~default:Node.make in
  count_with_node (node, 0) f

let without_profiling : (prof:t -> 'result) -> 'result = (fun f -> f ~prof:None)

let get_and_reset : unit -> CallTree.t =
 fun () ->
  let prof = Hashtbl.copy profilers in
  Hashtbl.clear profilers;
  prof
