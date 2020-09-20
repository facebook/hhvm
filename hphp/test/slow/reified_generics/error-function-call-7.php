<?hh

class C {
  public function f<reify T>() {
    var_dump("hi");
  }
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->f();
}
