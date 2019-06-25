<?hh
trait T1 {
  function m1() { echo "T:m1\n"; }
  function m2() { echo "T:m2\n"; }
}

class C1 {
  use T1 { m1 as a1; }
}
<<__EntryPoint>> function main(): void {
$o = new C1;
$o->m1();
$o->a1();
$o->m2();
$o->a2();
}
