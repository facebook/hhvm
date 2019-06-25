<?hh

trait T {
  function F(&$a, $b="default") {
    $a .= " = " . $b;
  }
}
class C {
  use T;
}
<<__EntryPoint>> function main(): void {
$o = new C;
$x = "value";
$o->F(&$x);
echo $x;
echo "\n";
$y = "zero";
$o->F(&$y, "0");
echo $y;
}
