<?hh // strict

<<__ConsistentConstruct>>
interface Foo { public function foo_(): void; }
interface Bar { public function bar_(): void; }
interface Baz extends Foo, Bar { }
interface Quux extends Baz { }


trait Wug {
  require implements Foo;
  public function thing(): void { echo "I'm a trait\n"; }
  public static function trait_sfactory(): Foo { return new static(); }
}

class Lol implements Foo {
  public function foo_(): void { echo "foo\n"; }
  public static function factory(): Lol { return new self(); }
  public static function sfactory(): Lol { return new static(); }
  use Wug;
}
class Lol2 extends Lol implements Baz {
  public function bar_(): void { echo "bar\n"; }
}

function make_foo(classname<Foo> $c): Foo {
  return new $c();
}

function test(): void {
  $lol = new Lol();
  $lol2 = new Lol2();
  $lol->thing();
  $lol->foo_();
  $lol2->thing();
  $lol2->foo_();
  $lol2->bar_();

  var_dump(Lol::factory());
  var_dump(Lol2::factory());
  var_dump(Lol::sfactory());
  var_dump(Lol2::sfactory());
  var_dump(Lol::trait_sfactory());
  var_dump(Lol2::trait_sfactory());
  var_dump(make_foo(Lol::class));
  var_dump(make_foo(Lol2::class));
}
