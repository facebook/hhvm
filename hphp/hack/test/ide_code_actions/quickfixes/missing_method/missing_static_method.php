<?hh

interface IFoo {
  public static function bar(): void;
}

class Foo implements IFoo {
                   // ^ at-caret
  public function other(): void {}
}
