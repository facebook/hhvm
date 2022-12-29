<?hh


function test(bool $b, dynamic $untyped): void {
  if ($b) { // checked lvar

  }
  $untyped; // unchecked lvar
}
