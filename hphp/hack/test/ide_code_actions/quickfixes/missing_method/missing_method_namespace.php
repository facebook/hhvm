<?hh

namespace Foo {

interface IFoo {
  public function bar(): void;
}

class Foo implements IFoo {}
  //                  ^ at-caret

}
