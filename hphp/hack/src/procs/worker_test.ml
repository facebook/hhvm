(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)



let test1 workers =
  let size = Array.length workers in
  try
    for i = 0 to 10000 do
      for i = 0 to size - 1 do
        let x = 
          Worker.call workers.(i) begin fun x ->
            -x
          end i in
        if x <> -i then raise Exit
      done
    done;
    Printf.printf "TestWorker: OK\n"; flush stdout;
  with Exit ->
    Printf.printf "TestWorker: FAILED\n"; flush stdout

let rec make_bucket n acc l =
  match l with
  | _ when n <= 0 -> acc, l
  | [] -> acc, l
  | x :: rl ->
      make_bucket (n-1) (x :: acc) rl

let rec make_list n acc =
  if n <= 0 then acc else
  make_list (n-1) (n :: acc)

let test_multi workers sum =
  let workers = Array.to_list workers in
  let args = make_list sum [] in
  let merge x y = x + y in
  let job x acc = x + acc in
  let make_bucket = make_bucket 10 [] in
  let neutral = 0 in
  let result =
    MultiWorker.call workers ~args ~merge ~job ~make_bucket ~neutral in
  if sum * (sum + 1) / 2 = result
  then Printf.printf "TestMulti %d: OK\n" sum
  else Printf.printf "TestMulti %d: FAILED\n" sum;
  flush stdout

let main() = 
  let size = 12 in
  let workers = Array.init size (fun _ -> Worker.make()) in
  test1 workers;
  for i = 0 to 100 do
    test_multi workers i
  done
      
