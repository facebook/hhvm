<?hh

<<__NoFlatten>>
trait T {
  public function f1() :mixed{
    return () ==> $this->x;
  }
}

class A {
  use T;
  private int $x = 123;
}

class B {
  use T;
  private int $x = 456;
}

class C {
  use T;
  private int $x = 789;
}

<<__EntryPoint>>
function main() :mixed{
  $a = new A();
  $b = new B();
  $c = new C();
  var_dump($a->f1()());
  var_dump($b->f1()());
  var_dump($c->f1()());
  var_dump($a->f1()());
  var_dump($b->f1()());
  var_dump($c->f1()());
}
