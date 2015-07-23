<?hh // strict

interface Foo { public function foo_(): void; }
interface Bar { public function bar_(): void; }
interface Baz extends Foo, Bar { }
interface Quux extends Baz { }

trait T {
  require implements Foo;
  public function thing(): void { echo "I'm a trait\n"; }
}
class Lol implements Foo {
  public function foo_(): void { echo "foo\n"; }
  use T;
}
class Lol2 extends Lol implements Baz {
  public function bar_(): void { echo "bar\n"; }
}

function test(): void {
  $lol = new Lol();
  $lol2 = new Lol2();
  $lol->thing();
  $lol->foo_();
  $lol2->thing();
  $lol2->foo_();
  $lol2->bar_();
}
