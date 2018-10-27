<?hh

class C {
  static public function foo() {
    return 1;
  }
}

$m = class_meth(C::class, 'foo');
var_dump($m());
