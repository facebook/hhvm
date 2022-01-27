<?hh

class MyFoo<Tfoo> {
  public function bar<Tx>(): Tfoo {
    //                        ^ hover-at-caret
    throw new Exception('');
  }
}
