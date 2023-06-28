<?hh

class C {
  public function f<reify T>() :mixed{
    var_dump("hi");
  }
}
<<__EntryPoint>> function main(): void {
$c = new C();
$c->f();
}
