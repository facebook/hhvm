<?hh

class M {
  static public function __callStatic($x, $y) {
    echo $x;
  }
}

$f = class_meth(M::class, "undef");

$f(1, 2);
