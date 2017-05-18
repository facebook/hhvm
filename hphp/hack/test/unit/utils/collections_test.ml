module IntMap = MyMap.Make(struct
  type t = int
  let compare x y = x - y
end)

let map_of_elements = List.fold_left
  (fun map (k, v) -> IntMap.add k v map)
  IntMap.empty

let test_myMap_union () =
  let map1 = map_of_elements [(1, 2); (3, 4)] in
  let map2 = map_of_elements [(1, 10); (5, 6)] in
  let () =
    let union = IntMap.union map1 map2 in
    let expected = map_of_elements [(1, 2); (3, 4); (5, 6)] in
    if not (IntMap.equal union expected) then failwith "Maps not equal"
  in
  let () =
    let union = IntMap.union ~combine:(fun _ _ snd -> Some snd) map1 map2 in
    let expected = map_of_elements [(1, 10); (3, 4); (5, 6)] in
    if not (IntMap.equal union expected) then failwith "Maps not equal"
  in
  let () =
    let union = IntMap.union ~combine:(fun _ _ _ -> None) map1 map2 in
    let expected = map_of_elements [(3, 4); (5, 6)] in
    if not (IntMap.equal union expected) then failwith "Maps not equal"
  in
  true

let tests = [
  "test_myMap_union", test_myMap_union;
]

let () =
  Unit_test.run_all tests
