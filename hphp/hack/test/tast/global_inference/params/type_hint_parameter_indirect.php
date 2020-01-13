<?hh //partial

function accessorentry($x) : int {
  return $x->foo;
}

function accessorarray($x, $i) : int {
  return $x[$i];
}

function mutentry($x, $w) : void {
  $x->foo = $w;
}

function mutarray(vec<int> $x, $i, $w) : void {
  $x[$i] = $w;
}
/*
function mutarray<T>(dict<string, T> $x, $i, $w) : void {
  $x[$i] = $w;
}

function mutarray2<T>(array<T> $x, $i, $w) : void {
  $x[$i] = $w;
}
*/
