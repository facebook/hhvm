<?hh

function mixed_context(
  (function()[_]: void) $f,
)[local, ctx $f]: void {
  $f();
}

function caller()[]: void {
  // pass pure closure
  mixed_context(()[] ==> {});
}
