exception NoSuchEntry of string

type cell = {
  mutable count : float;
  mutable sum   : float;
}

let string_of_cell {count; sum} =
  Printf.sprintf "Cell(%f, %f)" count sum

(* the correctness of the implementation of find_or_add depends on
 * having a unique cell for every string *)
type t = (string, cell) Hashtbl.t

let to_string h =
  let buf = Buffer.create 80 in
  Buffer.add_string buf "Hashtbl{";
  Hashtbl.iter (fun s c ->
    Printf.bprintf buf "%s %s, " s (string_of_cell c)
  ) h;
  (* remove trailing space and comma *)
  Buffer.truncate buf (-2 + Buffer.length buf);
  Buffer.add_string buf "}";
  buf |> Buffer.contents


let show = to_string
let pp : Format.formatter -> t -> unit = fun _ x -> Printf.printf "%s\n" (show x)

let stats_instance_ref : t option ref = ref None

let set_instance statsopt = stats_instance_ref := statsopt

let get_instance () = !stats_instance_ref

let zero_cell () = {count = 0.0; sum = 0.0}

let container_initial_size = 8

let new_container () = Hashtbl.create ~random:true container_initial_size

(* get a cell from inside the container, creating a new zero cell if the
key has not been seen before *)
let find_or_add t k =
  (* NOTE: once we drop support for OCamls earlier than 4.05, replace Hashtbl.find_all
   * with find_opt.
   *
   * find_all is used in place of find because catching exceptions prevents inlining
   *
   * find_all is used in place of mem + find to prevent computing the hash twice
   * *)
  match Hashtbl.find_all t k with
  | [cell] -> cell
  | [] -> let cell = zero_cell () in (Hashtbl.add t k cell; cell)
  | _ -> failwith "impossible: bucket in Stats_container.t has multiple items"

let record_float t k v =
  let cell = find_or_add t k in
  (cell.count <- 1.0 +. cell.count; cell.sum <- v +. cell.sum)

let get_count_exn t k = match Hashtbl.find_all t k with
  | [{count; _}] -> count
  | [] -> raise (NoSuchEntry ("count: " ^ k))
  | _ -> failwith "get_count_exn: multiple items"

let get_sum_exn t k = match Hashtbl.find_all t k with
  | [{sum; _}] -> sum
  | [] -> raise (NoSuchEntry ("sum: " ^ k))
  | _ -> failwith "get_sum_exn: multiple items"

let get_mean_exn t k = match Hashtbl.find_all t k with
  | [{count; sum}] -> sum /. count
  | [] -> raise (NoSuchEntry ("mean: " ^ k))
  | _ -> failwith "get_mean_exn: multiple itmes"

(* non-polymorphic comparison function for strings *)
let strcmp : string -> string -> int =
  fun a b -> compare a b

(* create array populated with sorted keys of hash,
 * used to make sure that write_out prints the keys in a deterministic order *)
let sorted_keys t =
  let out = Array.make (Hashtbl.length t) "" in
  let i_ref = ref 0 in
  begin
    Hashtbl.iter (fun k _ -> (out.(!i_ref) <- k; incr i_ref)) t;
    Array.fast_sort strcmp out;
    out
  end

let write_out ~out t =
  let f k = begin
    let mean = get_mean_exn t k in
    let sum = get_sum_exn t k in
    let count = get_count_exn t k in
    Printf.fprintf out "STATS: %s mean:%f sum:%f count:%f\n" k mean sum count
  end in
  let keys = sorted_keys t in
  Array.iter f keys

(* time a unary funtion, preserve exceptions *)
let time_fn : 'a . stats:t -> key:string -> f:('a -> 'b) -> ('a -> 'b) =
  fun ~stats ~key ~f arg -> begin
    let start = Unix.gettimeofday () in
    let res = try `Ok (f arg)
      with exn -> `Error exn in
    let stop = Unix.gettimeofday () in
    let () = record_float stats key (stop -. start) in
    match res with
    | `Ok ok -> ok
    |  `Error exn -> raise exn
  end

let wrap_unary_fn_timing ?stats ~key ~f =
  match stats with
  | Some s -> time_fn ~stats:s ~key ~f
  | None -> begin
      match get_instance () with
      | Some s -> time_fn ~stats:s ~key ~f
      | None -> f
  end

let wrap_nullary_fn_timing ?stats ~key ~f =
  wrap_unary_fn_timing ?stats ~key ~f ()
