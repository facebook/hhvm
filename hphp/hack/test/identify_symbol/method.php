<?hh

class Foo {
  public function bar() {
    // blah blah multiline method
  }
}

function test(Foo $foo) {
  $foo->bar();
}
