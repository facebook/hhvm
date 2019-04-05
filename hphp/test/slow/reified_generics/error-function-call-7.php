<?hh

class C {
  public function f<reify T>() {
    var_dump("hi");
  }
}

$c = new C();
$c->f();
