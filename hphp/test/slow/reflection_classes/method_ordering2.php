<?hh
class B {
  public function A(): void {
    echo "A\n";
  }
}
class C extends B {
  public function a(): void {
    echo "a\n";
  }
}

<<__EntryPoint>> function ordertest() :mixed{
  $c = new C();
  $c->A();
  $c->a();
  $r = new ReflectionClass("C");
  var_dump($r->getMethods());
}
