<?hh

abstract class B {
  private function priv() { echo "B::priv\n"; }
  function func(){
    $this->priv();
    var_dump(get_class_methods($this));
  }
}

class C extends B {
  private function priv() { echo "C::priv\n"; }
}
<<__EntryPoint>> function main(): void {
$obj = new C();
$obj->func();
}
