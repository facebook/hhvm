<?hh

class MyClass {
  public function foo(): this {
    //                   ^ hover-at-caret
    throw new Exception();
  }
}
