<?hh

class Foo {
  public function bar(): tuple<int,string> {
    return tuple(1, 'baz');
  }
}
