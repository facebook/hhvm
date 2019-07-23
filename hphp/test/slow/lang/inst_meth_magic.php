<?hh

class M {
  public function __call($x, $y) {
    echo $x;
  }
}
<<__EntryPoint>> function main(): void {
$f = inst_meth(new M, "undef");

$f();
}
