<?hh

trait T {
  private $x = 'init from T';
  function useT() :mixed{
 $this->x = 'set from trait';
 }
 }
class A {
  function useA() :mixed{
 $this->x = 'set from A';
 }
}
class B extends A {
  use T;
  function useB() :mixed{
 $this->x = 'set from B';
 }
}
class C extends B {
  function useC() :mixed{
 $this->x = 'set from C';
 }
}
class D extends C {
  function useD() :mixed{
 $this->x = 'set from D';
 }
}

<<__EntryPoint>>
function main_2061() :mixed{
$x = new D();
 echo serialize($x), "\n";
$x = new D();
 $x->useT();
 echo serialize($x), "\n";
$x = new D();
 $x->useA();
 echo serialize($x), "\n";
$x = new D();
 $x->useB();
 echo serialize($x), "\n";
$x = new D();
 $x->useC();
 echo serialize($x), "\n";
$x = new D();
 $x->useD();
 echo serialize($x), "\n";
}
