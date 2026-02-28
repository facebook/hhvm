<?hh

abstract class B {
  private function priv() :mixed{ echo "B::priv\n"; }
  function func():mixed{
    $this->priv();
    var_dump(get_class_methods($this));
  }
}

class C extends B {
  private function priv() :mixed{ echo "C::priv\n"; }
}
<<__EntryPoint>> function main(): void {
$obj = new C();
$obj->func();
}
