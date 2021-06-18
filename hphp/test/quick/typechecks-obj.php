<?hh

class C { }
class D extends C { }

function f(C $c = null) { var_dump($c);}
<<__EntryPoint>> function main(): void {

  $c = new C();
  $d = new D();
  f($c);
  f($d);
  f(null);
  f(new stdClass());
  echo "not reached\n";
}
