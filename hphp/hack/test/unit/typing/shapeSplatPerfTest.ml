(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Scaling benchmarks for the shape-splat merge (Typing_shape_normalize.merge).
 *
 * Each scenario builds a splat of [n] elements and times the operation. Each
 * row reports the local exponent
 *   e = log (t_n / t_prev) / log (n / prev_n)
 * so e ~ 1 is linear and e ~ 2 is quadratic. Assertions use a log-log fit over
 * the largest sizes, with each time taken as the median of repeated samples.
 * The scenarios are chosen to isolate the two independent costs in a merge:
 *
 *   - the shape accumulator (merge_shapes_simple), exercised by the closed
 *     disjoint-field scenarios;
 *   - the disjointness accumulator (add_disjointness_operand), exercised by the
 *     generic / open / repeated-field scenarios, where nothing is merged into
 *     the shape accumulator at all.
 *
 * Run it directly to get the table:
 *   buck run @fbcode//mode/dev-nosan-lg \
 *     fbcode//hphp/hack/test/unit/typing:shapeSplatPerfTest -- \
 *     -runner sequential
 *)

open Hh_prelude
open OUnit2
open Typing_defs
module Env = Typing_env
module MakeType = Typing_make_type
module Reason = Typing_reason
module Norm = Typing_shape_normalize

let dummy_env =
  let () = Typing_subtype.set_fun_refs () in
  let ctx =
    Provider_context.empty_for_test
      ~popt:Parser_options.default
      ~tcopt:Typechecker_options.default
      ~deps_mode:(Typing_deps_mode.InMemoryMode None)
  in
  let env = Typing_env_types.empty ctx Relative_path.default ~droot:None in
  let dummy_file = Relative_path.from_root ~suffix:"test.php" in
  let dummy_pos = Pos.make dummy_file (Lexing.from_string "") in
  let (env, _restore) = Env.set_inference_env_pos env (Some dummy_pos) in
  env

let r = Reason.none

let tint = MakeType.int r

let tnothing = MakeType.nothing r

let tmixed = MakeType.mixed r

let field ty = { sft_optional = false; sft_ty = ty }

let opt_field ty = { sft_optional = true; sft_ty = ty }

let key name = TSFlit_str (Pos_or_decl.none, name)

let simple ?(unknown = tnothing) kvs =
  {
    s_origin = Missing_origin;
    s_unknown_value = unknown;
    s_fields =
      List.fold kvs ~init:TShapeMap.empty ~f:(fun acc (k, v) ->
          TShapeMap.add (key k) v acc);
  }

let shape_ty s = mk (r, Tshape (Shape_simple s))

let splat elems = mk (r, Tshape (Shape_splat { ss_elems = elems }))

let closed_field i = shape_ty (simple [(Printf.sprintf "f%d" i, field tint)])

let generic i = mk (r, Tgeneric (Printf.sprintf "T%d" i))

(* -- Inputs ---------------------------------------------------------------- *)

(* [shape(...shape('f0' => int), ..., ...shape('f{n-1}' => int))]: every element
   is a closed single-field shape and all field names are distinct, so the merge
   is a pure accumulate with no overlap to report. *)
let flat_disjoint n = List.init n ~f:closed_field

(* As above but every element carries the same field, so rightmost-wins
   overwrites at each step and every element overlaps every earlier one. *)
let flat_same_field n =
  List.init n ~f:(fun _ -> shape_ty (simple [("a", field tint)]))

(* [n] distinct type parameters. Nothing merges into the shape accumulator: the
   elements are all residual, so this isolates the disjointness bookkeeping. *)
let flat_generics n = List.init n ~f:generic

(* [n] open shapes with disjoint fields. Each contributes both a field and an
   unknown-fields upper bound. *)
let flat_open n =
  List.init n ~f:(fun i ->
      shape_ty (simple ~unknown:tmixed [(Printf.sprintf "f%d" i, field tint)]))

(* As [flat_open], but every field is OPTIONAL. An incoming open row widens the
   accumulator's optional fields, so unlike [flat_open] the walk does real work:
   a [Typing_union.union] per accumulated field per element. *)
let flat_open_optional n =
  List.init n ~f:(fun i ->
      shape_ty
        (simple ~unknown:tmixed [(Printf.sprintf "f%d" i, opt_field tint)]))

(* [n] closed elements followed by a single open one. Isolates the cost of one
   open row meeting a large accumulator: linear if the walk is the only extra
   cost, quadratic only if every element pays it. *)
let closed_then_one_open n =
  List.init n ~f:closed_field
  @ [shape_ty (simple ~unknown:tmixed [("last", field tint)])]

(* A left-nested chain [shape(...shape(...shape(), 'f0' => int), 'f1' => int)...]
   of depth [n], the shape an alias chain expands to. *)
let left_nested n =
  let rec build i acc =
    if i >= n then
      [acc]
    else
      build (i + 1) (splat [acc; closed_field i])
  in
  build 0 (shape_ty (simple []))

(* The mirror image: [shape('f0' => int, ...shape('f1' => int, ...))]. *)
let right_nested n =
  let rec build i acc =
    if i >= n then
      [acc]
    else
      build (i + 1) (splat [closed_field i; acc])
  in
  build 0 (shape_ty (simple []))

(* Two shapes of [n] fields each, merged once. A single call to
   [merge_shapes_simple] over [n] keys: the linear-in-n baseline that the
   scenarios above should not be able to beat asymptotically. *)
let wide_pair n =
  let fields prefix =
    List.init n ~f:(fun i -> (Printf.sprintf "%s%d" prefix i, field tint))
  in
  [shape_ty (simple (fields "l")); shape_ty (simple (fields "r"))]

(* -- Harness --------------------------------------------------------------- *)

let sizes = [64; 128; 256; 512; 1024]

let run_merge elems =
  let (_env, _err, result) = Norm.merge ~on_error:None elems dummy_env in
  ignore (Sys.opaque_identity result)

let min_sample_seconds = 0.02

let timing_samples = 3

let fitted_size_count = 4

let median samples =
  let samples = List.sort samples ~compare:Float.compare in
  List.nth_exn samples (List.length samples / 2)

(* CPU seconds per run. Calibrate fast operations to a sufficiently long batch,
   then reuse that iteration count for independent samples. The discarded
   calibration batches together cost less than the final calibration batch. *)
let time ~samples f =
  let measure iterations =
    let t0 = Sys.time () in
    for _ = 1 to iterations do
      f ()
    done;
    let elapsed = Sys.time () -. t0 in
    (elapsed, elapsed /. float_of_int iterations)
  in
  Gc.compact ();
  let rec calibrate iterations =
    let (elapsed, per_run) = measure iterations in
    if Float.(elapsed >= min_sample_seconds) then
      (iterations, per_run)
    else
      calibrate (iterations * 2)
  in
  let (iterations, first_sample) = calibrate 1 in
  let remaining_samples =
    List.init (samples - 1) ~f:(fun _ ->
        Gc.compact ();
        measure iterations |> snd)
  in
  median (first_sample :: remaining_samples)

type measurement = {
  size: int;
  seconds: float;
}

let fit_exponent measurements =
  let reversed = List.rev measurements in
  let measurements = List.rev (List.take reversed fitted_size_count) in
  let points =
    List.map measurements ~f:(fun { size; seconds } ->
        (Float.log (float_of_int size), Float.log seconds))
  in
  let count = float_of_int (List.length points) in
  let (sum_x, sum_y) =
    List.fold points ~init:(0., 0.) ~f:(fun (sum_x, sum_y) (x, y) ->
        (sum_x +. x, sum_y +. y))
  in
  let mean_x = sum_x /. count in
  let mean_y = sum_y /. count in
  let (covariance, variance) =
    List.fold points ~init:(0., 0.) ~f:(fun (covariance, variance) (x, y) ->
        let dx = x -. mean_x in
        (covariance +. (dx *. (y -. mean_y)), variance +. (dx *. dx)))
  in
  covariance /. variance

(* Time [build n |> run] across [sizes] and return the exponent fitted over the
   largest [fitted_size_count] measurements. *)
let bench_with ~samples run name build =
  Printf.printf "\n%s\n" name;
  Printf.printf
    "%8s %12s %14s %14s %10s\n"
    "n"
    "t (ms)"
    "t/n (us)"
    "t/n^2 (ns)"
    "local exp.";
  let prev = ref None in
  let measurements =
    List.map sizes ~f:(fun n ->
        let elems = build n in
        let t = time ~samples (fun () -> run elems) in
        let n_f = float_of_int n in
        let local_exponent =
          match !prev with
          | Some (prev_n, prev_t) ->
            Float.log (t /. prev_t) /. Float.log (n_f /. float_of_int prev_n)
          | None -> Float.nan
        in
        Printf.printf
          "%8d %12.2f %14.3f %14.3f %10s\n%!"
          n
          (t *. 1000.)
          (t /. n_f *. 1e6)
          (t /. (n_f *. n_f) *. 1e9)
          (if Float.is_nan local_exponent then
            "-"
          else
            Printf.sprintf "%.2f" local_exponent);
        prev := Some (n, t);
        { size = n; seconds = t })
  in
  let exponent = fit_exponent measurements in
  Printf.printf
    "fitted exponent over largest %d sizes: %.2f\n%!"
    fitted_size_count
    exponent;
  exponent

let bench name build = bench_with ~samples:timing_samples run_merge name build

let report_bench name build = bench_with ~samples:1 run_merge name build

(* Perf assertions are inherently noisy, so the threshold is deliberately slack:
   it is here to catch a return to quadratic scaling, not to police constant
   factors. A scenario that is genuinely linear fits near 1.0 and stays well
   below 1.5 once the fixed costs wash out. *)
let assert_subquadratic name exponent =
  assert_bool
    (Printf.sprintf
       "%s should scale sub-quadratically, fitted exponent %.2f"
       name
       exponent)
    (Float.is_finite exponent && Float.(exponent < 1.5))

let perf_same_field _ =
  assert_subquadratic
    "flat_same_field"
    (bench "flat_same_field" flat_same_field)

let perf_generics _ =
  assert_subquadratic "flat_generics" (bench "flat_generics" flat_generics)

let perf_wide_pair _ =
  assert_subquadratic "wide_pair" (bench "wide_pair" wide_pair)

(* This benchmark remains report-only until its scaling assertion uses the
   larger input sizes introduced later in the optimization stack. *)
let report_disjoint_fields _ =
  let (_ : float) = report_bench "flat_disjoint" flat_disjoint in
  ()

(* Measured but not asserted on: open rows still walk the accumulated fields;
   optional fields additionally need to be widened. *)
let report_flat_open _ =
  let (_ : float) = report_bench "flat_open" flat_open in
  ()

let perf_closed_then_one_open _ =
  assert_subquadratic
    "closed_then_one_open"
    (bench "closed_then_one_open" closed_then_one_open)

let perf_left_nested _ =
  assert_subquadratic "left_nested" (bench "left_nested" left_nested)

let perf_right_nested _ =
  assert_subquadratic "right_nested" (bench "right_nested" right_nested)

let report_flat_open_optional _ =
  let (_ : float) = report_bench "flat_open_optional" flat_open_optional in
  ()

let () =
  let asserted_tests =
    [
      "wide_pair" >:: perf_wide_pair;
      "flat_same_field" >:: perf_same_field;
      "flat_generics" >:: perf_generics;
      "closed_then_one_open" >:: perf_closed_then_one_open;
      "left_nested" >:: perf_left_nested;
      "right_nested" >:: perf_right_nested;
    ]
  in
  (* Report-only quadratic cases are useful when running the benchmark directly,
     but should not add several seconds to the unit-test target. *)
  let tests =
    match Sys.getenv_opt "UNITTEST" with
    | Some "1" -> asserted_tests
    | _ ->
      asserted_tests
      @ [
          "flat_disjoint" >:: report_disjoint_fields;
          "flat_open" >:: report_flat_open;
          "flat_open_optional" >:: report_flat_open_optional;
        ]
  in
  "shapeSplatPerfTest" >::: tests |> run_test_tt_main
