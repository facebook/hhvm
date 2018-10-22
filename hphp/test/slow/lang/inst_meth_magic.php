<?hh

class M {
  public function __call($x, $y) {
    echo $x;
  }
}

$f = inst_meth(new M, "undef");

$f();
