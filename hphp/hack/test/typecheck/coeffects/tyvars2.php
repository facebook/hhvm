<?hh

function mixed_context(
  (function()[_]: void) $f,
)[non_det, ctx $f]: void {
  $f();
}

function caller()[]: void {
  // pass pure closure
  mixed_context(()[] ==> {});
}
