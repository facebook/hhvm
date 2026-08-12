<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

// Companion to sdt_lambda_param.php, encoding the other half of the T247822323
// distinction. Here the enclosing `no_dynamic_pass` has no params, so it gets no
// dynamic pass of its own; the lambda's dynamic-assumptions TAST is discarded
// (typing.ml:11278-11294) and never re-created by an enclosing pass, so hover
// falls back to the bare normal-assumptions type `int` -- the
// "(dynamic when called dynamically)" qualifier is dropped even though the
// closure IS a supportdyn closure. This is the gap the task reports.
function my_map<Tv1, Tv2>(
  Traversable<Tv1> $t,
  (function(Tv1): Tv2) $f,
): vec<Tv2> {
  return vec[];
}

function no_dynamic_pass(): void { // no params -> no enclosing dynamic pass
  my_map(vec[1], $val ==> {
    return $val;
    //     ^ hover-at-caret
  });
}
