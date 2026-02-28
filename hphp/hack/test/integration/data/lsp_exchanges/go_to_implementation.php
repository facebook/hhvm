<?hh
abstract class Foo {
  public function test(): string {
    return "a";
  }
}

class Bar extends Foo {
  public function test(): string {
    return "b";
  }
}

interface IFoo {
  public static function test(Foo $f): string;
}

class Baz implements IFoo {
  final public static function test(Foo $f): string {
    return $f->test();
  }
}

trait BaseTrait {

  public static function test(): string {
    return "c";
  }
}

trait ChildTrait {

  use BaseTrait;

  final public static function test(): string {
    return "d";
  }
}
