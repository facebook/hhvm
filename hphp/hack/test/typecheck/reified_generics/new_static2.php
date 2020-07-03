<?hh

<<__ConsistentConstruct>>
class BaseClass {
  public function f(): void {
    new static();
  }
}


class ChildClass<reify T> extends BaseClass {}
