<?hh // partial

class A {}

<<__Rx>>
function f(): void {
  $a = \HH\Rx\mutable(new A());
  $b = varray[$a];
  $b1 = varray[$a];
  $c = Vector {$a};
  $d = Map {"x" => $a};
  $e = dict["x" => $a];
  $e1 = darray["x" => $a];
  $f = Pair {$a, $a};
  $g = shape('x' => $a);
}

<<__Rx>>
function g(<<__MaybeMutable>> A $a): void {
  $b = varray[$a];
  $b1 = varray[$a];
  $c = Vector {$a};
  $d = Map {"x" => $a};
  $e = dict["x" => $a];
  $e1 = darray["x" => $a];
  $f = Pair {$a, $a};
  $g = shape('x' => $a);
}

<<__Rx>>
function h(<<__OwnedMutable>> A $a): void {
  $b = varray[$a];
  $b1 = varray[$a];
  $c = Vector {$a};
  $d = Map {"x" => $a};
  $e = dict["x" => $a];
  $e1 = darray["x" => $a];
  $f = Pair {$a, $a};
  $g = shape('x' => $a);
}
