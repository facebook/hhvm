<?hh

interface IFoo {
  public static function bar(): void;
}

class Foo implements IFoo {
  public function other(): void {}
}
