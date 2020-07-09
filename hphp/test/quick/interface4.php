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
  public function foo($x, $y=0, varray $z=null, arraylike $a=null) {
    echo "$x $y $z\n";
  }
}
<<__EntryPoint>> function main(): void {
// disable array -> "Array" conversion notice
error_reporting(error_reporting() & ~E_NOTICE);

$obj = new C;
$obj->foo(1);
$obj->foo(1, 2);
$obj->foo(1, 2, null);
$obj->foo(1, 2, varray[]);
}
