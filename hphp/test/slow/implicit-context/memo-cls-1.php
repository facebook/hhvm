<?hh

class Foo {
  static Foo $x;
  <<__PolicyShardedMemoize>>
  readonly function memo($a, $b)[zoned] {
    $hash = quoted_printable_encode(
      HH\ImplicitContext\_Private\get_implicit_context_memo_key()
    );
    echo "args: $a, $b hash: $hash\n";
  }
}

function g()[zoned] {
  (readonly Foo::$x)->memo(1, 2);
  (readonly Foo::$x)->memo(1, 3);
}

function f()[zoned] {
  (readonly Foo::$x)->memo(1, 2);
  (readonly Foo::$x)->memo(1, 3);
  ClassContext2::start(new B, g<>);
  (readonly Foo::$x)->memo(1, 2);
  (readonly Foo::$x)->memo(1, 3);
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  Foo::$x = new Foo();
  ClassContext::start(new A, f<>);
}
