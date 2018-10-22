<?hh

class C {
  static public function mth() {
    return 1;
  }
}

$m = class_meth(C::class, 'mth');
var_dump($m());
