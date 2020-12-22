<?hh

function poly(
  (function ()[_]: void) $f
)[ctx $f]: void {}

function pure()[]: void {}
function impure()[defaults]: void {}

function pure_caller()[]: void {
  poly(pure<>);
  poly(impure<>);
}
