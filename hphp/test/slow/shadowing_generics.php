<?hh
interface IBar {
  public function bar() : void;
}

class Foo<T as IBar> {
  public function __construct(T $t) {
    $this->t = $t;
  }
  <<__Memoize>>
  public function foo(T $t) : void {var_dump($t);}
  public function baz() : T {return $this->t; }
  private T $t;
}

class Bar implements IBar, IMemoizeParam {
  public int $i;
  public function __construct(int $j) {
    $this->i = $j;
  }
  public function bar() : void {}
  public function getInstanceKey(): string {
    return "bar".$this->i;
  }
}

<<__EntryPoint>>
function main() : void {
  $f = new Foo<Bar>(new Bar(2));
  $f->foo(new Bar(1));
  var_dump($f->baz());
  $g = new Foo<float>(2.2);
}
