<?hh // strict
trait BaseTrait {

  final public static function test(): string {
    return "a";
  }
}

trait ChildTrait {

  use BaseTrait;

  final public static function test(): string {
    return "a";
  }
}
