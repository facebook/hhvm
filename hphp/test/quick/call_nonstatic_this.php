<?hh

class B {
  function test() :mixed{
    D::meth();
  }
  function test2() :mixed{
    E::meth();
  }
}
class D extends B {
  function meth() :mixed{
    var_dump(isset($this));
    if (isset($this)) var_dump($this);
  }
}
class E extends B {
  function meth() :mixed{
    var_dump(isset($this));
    if (isset($this)) var_dump($this);
  }
}
<<__EntryPoint>> function main(): void {
  $d = new D;
  $d->test();
  $e = new E;
  $e->test2();
}
