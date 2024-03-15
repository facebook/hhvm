<?hh

abstract class MyGrandParent {
  abstract public function methodInGrandParent(): void;
}

abstract class MyParent extends MyGrandParent {
  abstract public function methodInParent(): void;
}

trait MyTrait {
  abstract public function methodInTrait(): void;
}

class MyClass extends MyParent {
  //                  ^ at-caret
  use MyTrait;
}
