(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type error = (Pos.t * string) list

exception Error of error

open Hh_json

let to_json (e : error) : json =
  let elts = List.map (fun (p, w) ->
                        let line, scol, ecol = Pos.info_pos p in
                        JAssoc [ "descr", JString w;
                                 "path",  JString p.Pos.pos_file;
                                 "line",  JInt line;
                                 "start", JInt scol;
                                 "end",   JInt ecol
                               ]
                      ) e
  in
  JAssoc [ "message", JList elts ]



let error_l message =
  raise (Error message)

let error p w =
  raise (Error [p, w])


let debug = ref false
let d s =
  if !debug
  then begin
    print_string s;
    flush stdout;
  end

let dn s =
  if !debug
  then begin
    print_string s;
    print_newline();
    flush stdout;
  end

let debug = ref false
let d s =
  if !debug
  then begin
    print_string s;
    flush stdout;
  end

let dn s =
  if !debug
  then begin
    print_string s;
    print_newline();
    flush stdout;
  end

let debug = ref false
let d s =
  if !debug
  then begin
    print_string s;
    flush stdout;
  end

let dn s = 
  if !debug
  then begin
    print_string s;
    print_newline();
    flush stdout;
  end

module MyMap: functor (Ord: Map.OrderedType) -> sig

  type +'a t
  type key = Ord.t

  val empty: 'a t
  val fold: (key -> 'a -> 'b -> 'b) -> 'a t -> 'b -> 'b
  val mem: key -> 'a t -> bool
  val add: key -> 'a -> 'a t -> 'a t
  val get: key -> 'a t -> 'a option
  val iter: (key -> 'a -> unit) -> 'a t -> unit
  val remove: key -> 'a t -> 'a t
  val map: ('a -> 'b) -> 'a t -> 'b t
  val find_unsafe: key -> 'a t -> 'a
  val is_empty: 'a t -> bool
  val union: 'a t -> 'a t -> 'a t
  val cardinal: 'a t -> int
  val compare: 'a t -> 'a t -> int
  val equal: 'a t -> 'a t -> bool
  val filter: (key -> 'a -> bool) -> 'a t -> 'a t

end = functor (Ord: Map.OrderedType) -> struct
  include Map.Make(Ord)
  let get x t =
    try Some (find x t) with Not_found -> None

  let find_unsafe = find

  let union x y =
    fold add x y

  let cardinal m = fold (fun _ _ acc -> 1 + acc) m 0
  let compare x y = compare Pervasives.compare x y
  let equal x y = compare x y = 0

  let filter f m =
    fold begin fun x y acc ->
      if f x y then add x y acc else acc
    end m empty
      
end


module SMap = MyMap(String)
module IMap = MyMap(Ident)
module ISet = Set.Make(Ident)
module SSet = Set.Make(String)
module Map = struct end

let pmsg p s =
  Printf.sprintf "%s\n%s\n" (Pos.string p) s

let pmsg_l l =
  let l = List.map (fun (p, e) -> pmsg p e) l in
  List.fold_right (^) l ""

let to_string (e : error) : string =
  let buf = Buffer.create 50 in
  List.iter (fun (p, w) -> Buffer.add_string buf (pmsg p w)) e;
  Buffer.contents buf

let internal_error s =
  Printf.fprintf stderr
    "You just found a bug!\nShoot me an email: julien.verlaguet@fb.com";
  exit 2

let opt f env = function
  | None -> env, None
  | Some x -> let env, x = f env x in env, Some x

let default x y f =
  match y with
  | None -> x
  | Some y -> f y

let rec lmap f env l =
  match l with
  | [] -> env, []
  | x :: rl ->
      let env, x = f env x in
      let env, rl = lmap f env rl in
      env, x :: rl

let smap_inter m1 m2 =
  SMap.fold (
  fun x y acc ->
    if SMap.mem x m2
    then SMap.add x y acc
    else acc
 ) m1 SMap.empty

let imap_inter m1 m2 =
  IMap.fold (
  fun x y acc ->
    if IMap.mem x m2
    then IMap.add x y acc
    else acc
 ) m1 IMap.empty

let smap_union m1 m2 = SMap.fold SMap.add m1 m2
let imap_union m1 m2 = IMap.fold IMap.add m1 m2

let smap_inter_list = function
  | [] -> SMap.empty
  | x :: rl ->
      List.fold_left smap_inter x rl

let imap_inter_list = function
  | [] -> IMap.empty
  | x :: rl ->
      List.fold_left imap_inter x rl


let partition_smap f m =
  SMap.fold (
  fun x ty (acc1, acc2) ->
    if f x
    then SMap.add x ty acc1, acc2
    else acc1, SMap.add x ty acc2
 ) m (SMap.empty, SMap.empty)

let smap_env f env m =
  SMap.fold (
  fun x y (env, acc) ->
    let env, y = f env y in
    env, SMap.add x y acc
 ) m (env, SMap.empty)

let rec lfold f env l =
  match l with
  | [] -> env, []
  | x :: rl ->
      let env, x = f env x in
      let env, rl = lfold f env rl in
      env, x :: rl

let rec lfold2 f env l1 l2 =
  match l1, l2 with
  | [], [] -> env, []
  | [], _ | _, [] -> raise (Invalid_argument "lfold2")
  | x1 :: rl1, x2 :: rl2 ->
      let env, x = f env x1 x2 in
      let env, rl = lfold2 f env rl1 rl2 in
      env, x :: rl

let rec wlfold2 f env l1 l2 =
  match l1, l2 with
  | [], [] -> env, []
  | [], l | l, [] -> env, l
  | x1 :: rl1, x2 :: rl2 ->
      let env, x = f env x1 x2 in
      let env, rl = lfold2 f env rl1 rl2 in
      env, x :: rl

let rec wfold_left2 f env l1 l2 =
  match l1, l2 with
  | [], _ | _, [] -> env
  | x1 :: rl1, x2 :: rl2 ->
      let env = f env x1 x2 in
      wfold_left2 f env rl1 rl2

let rec fold_left_env f env acc l =
  match l with
  | [] -> env, acc
  | x :: rl ->
      let env, acc = f env acc x in
      let env, acc = fold_left_env f env acc rl in
      env, acc

let rec make_list f n =
  if n = 0
  then []
  else f() :: make_list f (n-1)

let safe_ios p s =
  try int_of_string s
  with _ -> error p "Value is too large"

let sl l =
  List.fold_right (^) l ""

let soi = string_of_int

let maybe f env = function
  | None -> ()
  | Some x -> f env x

let unsafe_opt = function
  | None -> assert false
  | Some x -> x

let liter f env l = List.iter (f env) l

let inter_list = function
  | [] -> SSet.empty
  | x :: rl ->
      List.fold_left SSet.inter x rl

let rec list_last f1 f2 =
  function
    | [] -> ()
    | [x] -> f2 x
    | x :: rl -> f1 x; list_last f1 f2 rl

let rec uniq = function
  | [] -> []
  | [x] -> [x]
  | x :: (y :: _ as l) when x = y -> uniq l
  | x :: rl -> x :: uniq rl


let is_prefix_dir dir fn =
  let prefix = dir ^ Filename.dir_sep in
  String.length fn > String.length prefix && 
  String.sub fn 0 (String.length prefix) = prefix

let try_with_channel oc f1 f2 =
  try
    let result = f1 oc in
    close_out oc;
    result
  with e ->
    close_out oc;
    f2 e

let pipe (x : 'a)  (f : 'a -> 'b) : 'b = f x
let (|>) = pipe

let rec filter_some = function
  | [] -> []
  | None :: l -> filter_some l
  | Some e :: l -> e :: filter_some l

let map_filter f xs = xs |> List.map f |> filter_some

(*
let fork ?reason:(reason=None) () = 
  let result = Unix.fork() in
  (match result with
  | -1 | 0 -> ()
  | pid -> PidLog.log ~reason pid); 
  result
*)
let rec cut_after n = function
  | [] -> []
  | l when n <= 0 -> []
  | x :: rl -> x :: cut_after (n-1) rl

let iter_n_acc n f acc =
  let acc = ref acc in
  for i = 1 to n do
    acc := f !acc
  done;
  !acc
