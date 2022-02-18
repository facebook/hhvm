<?hh

class Foo {
  static Foo $x;
  <<__PolicyShardedMemoize>>
  readonly function memo<reify T>($a, $b)[zoned] {
    $hash = quoted_printable_encode(
      HH\ImplicitContext\_Private\get_implicit_context_memo_key()
    );
    $kind = HH\ReifiedGenerics\get_type_structure<T>()['kind'];
    echo "args: $a, $b hash: $hash, kind: $kind\n";
  }
}

function g()[zoned] {
  (readonly Foo::$x)->memo<int>(1, 2);
  (readonly Foo::$x)->memo<int>(1, 3);
  (readonly Foo::$x)->memo<string>(1, 2);
  (readonly Foo::$x)->memo<string>(1, 3);
}

function f()[zoned] {
  (readonly Foo::$x)->memo<int>(1, 2);
  (readonly Foo::$x)->memo<int>(1, 3);
  (readonly Foo::$x)->memo<string>(1, 2);
  (readonly Foo::$x)->memo<string>(1, 3);
  ClassContext2::start(new B, g<>);
  (readonly Foo::$x)->memo<int>(1, 2);
  (readonly Foo::$x)->memo<int>(1, 3);
  (readonly Foo::$x)->memo<string>(1, 2);
  (readonly Foo::$x)->memo<string>(1, 3);
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  Foo::$x = new Foo();
  ClassContext::start(new A, f<>);
}
