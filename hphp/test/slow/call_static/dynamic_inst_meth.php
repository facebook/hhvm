<?hh

class C {
  public function foo() {
    return 1;
  }
}

$m = inst_meth(new C, 'foo');
var_dump($m());
