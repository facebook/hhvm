<?hh

class Bar<Tv> {
  // Upper-bound is generic parameter, not enforced
  public function foo<reify T as Tv>(T $x) {
    var_dump($x);
  }
  public function foo1<reify T as Tv>() : T {
    return 10;
  }
  public function foo2<reify T as Tv>(inout T $x) {
    $x = $x + 1;
  }
}

<<__EntryPoint>> function main() {
  $o = new Bar<string>;
  $o->foo<int>(10);
  var_dump($o->foo1<int>());
  $x = 10;
  $o->foo2<int>(inout $x);
  var_dump($x);
}
