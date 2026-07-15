<?hh

function error_boundary($fn) :mixed{
  try {
    return $fn();
  } catch (\Throwable $e) {
    print("Error: ".$e->getMessage()."\n");
    return null;
  }
}

// The compound-assignment result is dead, so repo-mode HHBBC's DCE must still
// keep the op: /= and %= throw DivisionByZeroException on a zero divisor.
function divEq(int $x, int $y) :mixed{
  return error_boundary(() ==> { $x /= $y; return 1; });
}

function modEq(int $x, int $y) :mixed{
  return error_boundary(() ==> { $x %= $y; return 1; });
}

<<__EntryPoint>> function main(): void {
  var_dump(divEq(10, 2));
  var_dump(divEq(10, 0));
  var_dump(modEq(10, 3));
  var_dump(modEq(10, 0));
}
