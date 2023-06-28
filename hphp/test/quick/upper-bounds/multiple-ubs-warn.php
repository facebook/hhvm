<?hh

interface Fooable {
  public function foo(): void;
}

interface Barable {
  public function bar(): void;
}

class Meh {
  public function foo(): void {
    var_dump("foo");
  }
  public function bar(): void {
    var_dump("bar");
  }
}

class FooBar implements Fooable, Barable {
  public function foo(): void {
    var_dump("foo");
  }
  public function bar(): void {
    var_dump("bar");
  }
}

class JustFoo implements Fooable {
  public function foo(): void {
    var_dump("just foo");
  }
  public function bar(): void {
    var_dump("just bar");
  }
}

function example<T as Fooable as Barable>(T $x): T {
  $x->foo();
  $x->bar();
  return $x;
}

function foo<T as Fooable as Barable>(): T {
  return new Meh;
}

function baz<T as Fooable as Barable>(inout T $x, int $c): void {
  var_dump("baz");
  if ($c == 1) $x = new FooBar;
  else if ($c == 2) $x = new Meh;
}

function foobaz<T as Fooable as Barable>(T $x, ?T $y) :mixed{
  if($x) $x->foo();
  if($y) $y->bar();
}

<<__EntryPoint>> function main() :mixed{
  $o = foo();
  example($o);
  $p = new FooBar;
  $q = example($p);
  baz(inout $o, 1); // error on in, pass on out
  baz(inout $p, 2); // pass on in, error in out
  $o = new Meh;
  baz(inout $o, 3); // error on both
  $r = new JustFoo;
  example($r);      // fail one UB, pass other
  $p = new FooBar;
  foobaz($p, null); // no error
  foobaz(null, $p); // warn on arg 1
}
