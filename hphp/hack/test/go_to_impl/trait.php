<?hh
trait BaseTrait {

  public static function test(): string {
    return "a";
  }
}

trait ChildTrait {

  use BaseTrait;

  final public static function test(): string {
    return "b";
  }
}

class Foo {

  use ChildTrait;

  final public static function test2(): string {
    return test();
  }
}
