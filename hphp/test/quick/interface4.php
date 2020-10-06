<?hh

interface I {
  function foo($x, $y=0);
}
interface J {
  function foo($x, $y);
}
interface K {
  function foo($x, $y=0, varray $z);
}
interface L {
  function foo($x, $y, varray $z=null);
}
interface M {
  function foo($x, $y=0, varray $z=varray[]);
}
class C implements I, J, K, L, M {
  public function foo($x, $y=0, varray $z=null, AnyArray $a=null) {
    $x = HH\is_any_array($x) ? 'Array' : $x;
    $y = HH\is_any_array($y) ? 'Array' : $y;
    $z = HH\is_any_array($z) ? 'Array' : $z;
    echo "$x $y $z\n";
  }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->foo(1);
$obj->foo(1, 2);
$obj->foo(1, 2, null);
$obj->foo(1, 2, varray[]);
}
