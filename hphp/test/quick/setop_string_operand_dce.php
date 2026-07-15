<?hh

function error_boundary($fn) :mixed{
  try {
    return $fn();
  } catch (\Throwable $e) {
    print("Error: ".$e->getMessage()."\n");
    return null;
  }
}

// A dead compound-assignment on an `arraykey` (int|str) operand still throws a
// TypeError when the value is a string, so repo-mode DCE must keep the op.
function plusEq(int $x, arraykey $y) :mixed{
  return error_boundary(() ==> { $x += $y; return 1; });
}

function mulEq(int $x, arraykey $y) :mixed{
  return error_boundary(() ==> { $x *= $y; return 1; });
}

function divEq(int $x, arraykey $y) :mixed{
  return error_boundary(() ==> { $x /= $y; return 1; });
}

function modEq(int $x, arraykey $y) :mixed{
  return error_boundary(() ==> { $x %= $y; return 1; });
}

<<__EntryPoint>> function main(): void {
  var_dump(plusEq(10, 5));
  var_dump(plusEq(10, "abc"));
  var_dump(mulEq(10, 5));
  var_dump(mulEq(10, "abc"));
  var_dump(divEq(10, 5));
  var_dump(divEq(10, "abc"));
  var_dump(modEq(10, 5));
  var_dump(modEq(10, "abc"));
}
