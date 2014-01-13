<?hh

class Foo {
  public function foo(?Foo $x) {
    var_dump($x);
  }
}

new Foo();
