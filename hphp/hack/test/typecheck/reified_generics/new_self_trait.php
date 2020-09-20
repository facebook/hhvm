<?hh

class BaseClass<reify T> {}

<<__ConsistentConstruct>>
trait MyTrait {
  require extends BaseClass<int>;

  public function test(): void {
    new self();
  }
}

class ChildClass extends BaseClass<int> {
  use MyTrait;
}

<<__EntryPoint>>
function test(): void {
  (new ChildClass())->test();
}
