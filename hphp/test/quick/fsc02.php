<?hh
class B {
  public function f1() {
    var_dump(static::class);
    if ($this !== null) {
      var_dump(get_class($this));
    } else {
      var_dump(null);
    }
    echo "\n";
  }
}
class C extends B {
  public function g() {
    $obj = new B;
    $f = (array('B', 'f1'));
    $f();
  }
}
$obj = new C;
$obj->g();
