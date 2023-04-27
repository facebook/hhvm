<?hh

trait MyTrait {}

class MyClass {
  use MyTrait;
  //^ hover-at-caret
}
