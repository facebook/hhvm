<?hh

trait A {
  public function b() {
    return function() {
      return varray[
        __CLASS__,
        get_class($this)
      ];
    };
  }
}

class C {
  use A;
  public function d() {
    return function() {
      return varray[
        __CLASS__,
        get_class($this)
      ];
    };
  }
}
<<__EntryPoint>> function main(): void {
$c = new C;
$b = $c->b();
var_dump($b());
$d = $c->d();
var_dump($d());
}
