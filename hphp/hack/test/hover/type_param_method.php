<?hh

class MyFoo {
  public function bar<Tfoo>(): Tfoo {
    //                         ^ hover-at-caret
    throw new Exception('');
  }
}
