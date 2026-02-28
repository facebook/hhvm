<?hh

interface I {
  function foo($x, $y=0):mixed;
}
interface J {
  function foo($x, $y):mixed;
}
interface K {
  function foo($x, $y=0, varray $z):mixed;
}
interface L {
  function foo($x, $y, ?varray $z=null):mixed;
}
interface M {
  function foo($x, $y=0, varray $z=vec[]):mixed;
}
class C implements I, J, K, L, M {
  public function foo($x, $y=0, ?varray $z=null, ?AnyArray $a=null) :mixed{
    $x = HH\is_any_array($x) ? 'Array' : $x;
    $y = HH\is_any_array($y) ? 'Array' : $y;
    $z = HH\is_any_array($z) ? 'Array' : $z;
    $x__str = (string)($x);
    $y__str = (string)($y);
    $z__str = (string)($z);
    echo "$x__str $y__str $z__str\n";
  }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->foo(1);
$obj->foo(1, 2);
$obj->foo(1, 2, null);
$obj->foo(1, 2, vec[]);
}
