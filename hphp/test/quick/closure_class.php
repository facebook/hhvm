<?hh

trait A {
  public function b() :mixed{
    return function() {
      return vec[
        __CLASS__,
        get_class($this)
      ];
    };
  }
}

class C {
  use A;
  public function d() :mixed{
    return function() {
      return vec[
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
