<?hh

class BaseClass<reify T> {}

<<__ConsistentConstruct>>
trait MyTrait {
  require extends BaseClass<int>;

  public static function test(): void {
    new static();
  }
}

class ChildClass extends BaseClass<int> {
  use MyTrait;
}

<<__EntryPoint>>
function test(): void {
  ChildClass::test();
}
