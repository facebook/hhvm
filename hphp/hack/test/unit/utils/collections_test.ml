module IntMap = Wrapped_map.Make (struct
  type t = int

  let compare = Int.compare
end)

let map_of_elements =
  List.fold_left (fun map (k, v) -> IntMap.add k v map) IntMap.empty

let test_Wrapped_map_union () =
  let map1 = map_of_elements [(1, 2); (3, 4)] in
  let map2 = map_of_elements [(1, 10); (5, 6)] in
  let () =
    let union = IntMap.union map1 map2 in
    let expected = map_of_elements [(1, 2); (3, 4); (5, 6)] in
    if not (IntMap.equal ( = ) union expected) then failwith "Maps not equal"
  in
  let () =
    let union = IntMap.union ~combine:(fun _ _ snd -> Some snd) map1 map2 in
    let expected = map_of_elements [(1, 10); (3, 4); (5, 6)] in
    if not (IntMap.equal ( = ) union expected) then failwith "Maps not equal"
  in
  let () =
    let union = IntMap.union ~combine:(fun _ _ _ -> None) map1 map2 in
    let expected = map_of_elements [(3, 4); (5, 6)] in
    if not (IntMap.equal ( = ) union expected) then failwith "Maps not equal"
  in
  true

let test_ImmQueue () =
  let queue = Imm_queue.empty in
  if not (Imm_queue.is_empty queue) then failwith "not empty";
  let (x, queue) = Imm_queue.peek queue in
  if not (x = None) then failwith "peeking an empty queue should return None";
  let queue = Imm_queue.push queue 4 in
  if Imm_queue.is_empty queue then failwith "empty";
  let queue = Imm_queue.push queue 5 in
  if Imm_queue.length queue <> 2 then failwith "wrong length";
  let queue = Imm_queue.push queue 6 in
  let (x, queue) = Imm_queue.peek queue in
  (match x with
  | Some 4 -> ()
  | _ -> failwith "wrong value");
  let (x, queue) = Imm_queue.pop queue in
  (match x with
  | Some 4 -> ()
  | _ -> failwith "wrong value");
  let (x, queue) = Imm_queue.pop_unsafe queue in
  if x <> 5 then failwith "wrong value";
  let (x, queue) = Imm_queue.pop_unsafe queue in
  if x <> 6 then failwith "wrong value";
  let did_throw =
    try
      ignore (Imm_queue.pop_unsafe queue);
      false
    with
    | Imm_queue.Empty -> true
  in
  if not did_throw then failwith "expected an exception";
  let (x, _) = Imm_queue.pop queue in
  match x with
  | Some _ -> failwith "expected none"
  | None ->
    let queue =
      Imm_queue.push (Imm_queue.push (Imm_queue.push Imm_queue.empty 1) 2) 3
    in
    let (_, queue) = Imm_queue.pop queue in
    let queue = Imm_queue.push (Imm_queue.push queue 4) 5 in
    let acc = ref [] in
    Imm_queue.iter queue ~f:(fun i -> acc := !acc @ [i]);
    if !acc <> [2; 3; 4; 5] then failwith "expected 2345 iter order";
    if Imm_queue.to_list queue <> [2; 3; 4; 5] then
      failwith "expected 2345 list";

    let queue2 = Imm_queue.from_list [6; 7; 8] in
    let (_, queue2) = Imm_queue.pop queue2 in
    let queue2 = Imm_queue.push (Imm_queue.push queue2 9) 0 in
    let queue3 = Imm_queue.concat [queue; queue2] in
    if Imm_queue.to_list queue3 <> [2; 3; 4; 5; 7; 8; 9; 0] then
      failwith "expected 23457890 cat";

    true

let tests =
  [
    ("test_Wrapped_map_union", test_Wrapped_map_union);
    ("test_ImmQueue", test_ImmQueue);
  ]

let () = Unit_test.run_all tests
