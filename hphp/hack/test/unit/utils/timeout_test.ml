(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

let test_basic_no_timeout () =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout:(fun _ -> false)
    ~do_:(fun _ -> true)

let test_basic_with_timeout () =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout:(fun _ -> true)
    ~do_:begin fun timeout ->
      let _ = Unix.select [] [] [] 2.0 in
      false
    end

let test_basic_nested_no_timeout () =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout:(fun _ -> false)
    ~do_:begin fun timeout ->
      Timeout.with_timeout
        ~timeout:1
        ~on_timeout:(fun _ -> false)
        ~do_:(fun _ -> true)
    end

let test_basic_nested_inner_timeout () =
  Timeout.with_timeout
    ~timeout:3
    ~on_timeout:(fun _ -> false)
    ~do_:begin fun timeout ->
      Timeout.with_timeout
        ~timeout:1
        ~on_timeout:(fun _ -> true)
        ~do_:begin fun timeout ->
          let _ = Unix.select [] [] [] 2.0 in
          false
        end
    end

let test_basic_nested_outer_timeout () =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout:(fun _ -> true)
    ~do_:begin fun timeout ->
      Timeout.with_timeout
        ~timeout:3
        ~on_timeout:(fun _ -> false)
        ~do_:begin fun timeout ->
          let _ = Unix.select [] [] [] 2.0 in
          false
        end
    end

let tests = [
  "test_basic_no_timeout", test_basic_no_timeout;
  "test_basic_with_timeout", test_basic_with_timeout;
  "test_basic_nested_no_timeout", test_basic_nested_no_timeout;
  "test_basic_nested_inner_timeout", test_basic_nested_inner_timeout;
  "test_basic_nested_outer_timeout", test_basic_nested_outer_timeout;
]

let () =
  Unit_test.run_all tests
