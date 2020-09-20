<?hh

class BaseClass<reify T> {}

<<__ConsistentConstruct>>
trait MyTrait {
  require extends BaseClass<int>;

  public static function test(): void {
    new static();
  }
}

class ChildClass<reify T> extends BaseClass<int> {
  use MyTrait;
}

<<__EntryPoint>>
function test(): void {
  ChildClass::test();
}
