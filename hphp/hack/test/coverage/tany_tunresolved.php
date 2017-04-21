<?hh


function test(bool $b, $untyped): void {
  if ($b) { // checked lvar

  }
  $untyped; // unchecked lvar
}
