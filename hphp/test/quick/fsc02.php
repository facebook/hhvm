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
    $f = (varray['B', 'f1']);
    $f();
  }
}
<<__EntryPoint>> function main(): void {
$obj = new C;
$obj->g();
}
