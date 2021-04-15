<?hh

// erased generics

function f_tuple((int, int) $t) : void {}

function test_tuple(dynamic $d) : void {
  f_tuple($d);
}

function f_shape(shape ('x' => int) $s) : void {}

function test_shape(dynamic $d) : void {
  f_shape($d);
}

class C<+T> {}

function f_generic(C<int> $c) : void {}

function test_generic(dynamic $d) : void {
  f_generic($d);
}

function f_context(C<int> $c) : void {}

function test_context(C<dynamic> $d) : void {
  f_context($d);
}

function f_string(string $s) : void {}

function test_union(dynamic $d, bool $b) : void {
  if ($b) {
    $x = $d;
  } else {
    $x = 1;
  }
  f_string($x);
}
