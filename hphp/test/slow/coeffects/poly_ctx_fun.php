<?hh

function poly(
  (function (int)[_]: void) $f
)[ctx $f]: void {}

function poly2(
  (function ()[_]: void) $f,
  (function ()[defaults]: void) $g,
)[ctx $f, output]: void {
  $f();
  echo "between f and g\n";
  $g();
}

class Obj {
  public function poly(
    (function()[_]: int) $f
  )[ctx $f]: int { return $f(); }
}

<<__EntryPoint>>
function main(): void {
  poly($_ ==> {});
  $f = () ==> {};
  poly2($f, $f);
  echo "method poly: ";
  echo new Obj()->poly(() ==> 42);
  echo "\nDone";
}
