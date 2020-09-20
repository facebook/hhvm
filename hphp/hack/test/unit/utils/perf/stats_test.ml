let epsilon = 1.0e-10

let ( =. ) a b = abs_float (a -. b) < epsilon

let compare_quantiles ?(show = true) xs1 xs2 =
  let eq =
    let rec work xs1 xs2 =
      match (xs1, xs2) with
      | ([], []) -> true
      | (x1 :: xs1, x2 :: xs2) -> x1 =. x2 && work xs1 xs2
      | (_, _) -> false
    in
    work xs1 xs2
  in
  if (not eq) && show then (
    let pr xs = List.iter (fun x -> Printf.eprintf "%f " x) xs in
    pr xs1;
    Printf.eprintf "<> ";
    pr xs2;
    prerr_endline ""
  );
  eq

let test_quantiles_equal_step2 () =
  let qs = Perf_stats.quantiles [1.; 3.; 5.; 7.; 9.; 11.] 3 in
  compare_quantiles qs [3.; 7.; 11.]

let test_quantiles_equal_step4 () =
  let qs =
    Perf_stats.quantiles [0.; 0.5; 1.; 1.; 7.; 8.; 8.; 9.; 64.; 64.; 64.; 81.] 3
  in
  compare_quantiles qs [1.; 9.; 81.]

let test_quantiles_equal_uniform_4 () =
  let qs = Perf_stats.quantiles [1.; 2.; 3.; 4.] 4 in
  compare_quantiles qs [1.; 2.; 3.; 4.]

let test_quantiles_equal_uniform_3 () =
  let qs = Perf_stats.quantiles [1.; 2.; 3.; 4.] 3 in
  compare_quantiles qs [2.; 3.; 4.]

let () =
  Unit_test.run_all
    [
      ("test_quantiles_equal_step2", test_quantiles_equal_step2);
      ("test_quantiles_equal_step4", test_quantiles_equal_step4);
      ("test_quantiles_equal_uniform_4", test_quantiles_equal_uniform_4);
      ("test_quantiles_equal_uniform_3", test_quantiles_equal_uniform_3);
    ]
