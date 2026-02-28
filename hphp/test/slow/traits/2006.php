<?hh

trait T {
  function F(inout $a, $b="default") :mixed{
    $a .= " = " . $b;
  }
}
class C {
  use T;
}
<<__EntryPoint>> function main(): void {
$o = new C;
$x = "value";
$o->F(inout $x);
echo $x;
echo "\n";
$y = "zero";
$o->F(inout $y, "0");
echo $y;
}
