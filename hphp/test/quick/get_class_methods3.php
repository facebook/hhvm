<?hh

abstract class B {
  private function priv() :mixed{ }
  function func():mixed{
    $this->priv();
    var_dump(get_class_methods($this));
  }
}

class C extends B {}
<<__EntryPoint>> function main(): void {
$obj = new C();
$obj->func();
}
