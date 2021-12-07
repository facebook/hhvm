<?hh

class Foo {
  <<__PolicyShardedMemoizeLSB>>
  static function memo<reify T>($a, $b)[zoned] {
    $hash = quoted_printable_encode(
      HH\ImplicitContext\_Private\get_implicit_context_memo_key()
    );
    $kind = HH\ReifiedGenerics\get_type_structure<T>()['kind'];
    echo "args: $a, $b hash: $hash, kind: $kind\n";
  }
}

function g()[zoned] {
  Foo::memo<int>(1, 2);
  Foo::memo<int>(1, 3);
  Foo::memo<string>(1, 2);
  Foo::memo<string>(1, 3);
}

function f()[zoned] {
  Foo::memo<int>(1, 2);
  Foo::memo<int>(1, 3);
  Foo::memo<string>(1, 2);
  Foo::memo<string>(1, 3);
  ClassContext2::start(new B, g<>);
  Foo::memo<int>(1, 2);
  Foo::memo<int>(1, 3);
  Foo::memo<string>(1, 2);
  Foo::memo<string>(1, 3);
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
