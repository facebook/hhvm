<?hh

function f()[]: (function()[output, local]: void) {
  return ()[output] ==> print "Hi!\n";
}

function test()[output]: void {
  caller(f(), f());
}

function caller((function()[_]: void) $f1, (function()[_]: void) $f2)[ctx $f1, ctx $f2]: void {
  $f1();
  $f2();
}
