<?hh

<<__ConsistentConstruct>>
abstract class Top {}
class Concrete extends Top {}

class C<<<__Newable>> reify T as Top> {
  public function f(): void {
    new T();
  }
}

class Bad extends C<Top> {}
class Good extends C<Concrete> {}
