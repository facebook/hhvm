<?hh

class C<T as arraykey> {
  public function foo(T $x): void {}
//                    ^ enforcement-at-caret
}
