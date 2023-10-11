<?hh

// It's illegal to assign a generic to a tuple
function test<T>(T $v): void {
  list($x, $y) = $v;
}
