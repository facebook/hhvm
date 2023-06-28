<?hh

class Foo {
  <<__MemoizeLSB(#KeyedByIC)>>
  static function memo<reify T>($a, $b)[zoned] :mixed{
    $hash = quoted_printable_encode(
      HH\ImplicitContext\_Private\get_implicit_context_memo_key()
    );
    $kind = HH\ReifiedGenerics\get_type_structure<T>()['kind'];
    echo "args: $a, $b hash: $hash, kind: $kind\n";
  }
}

function g()[zoned] :mixed{
  Foo::memo<int>(1, 2);
  Foo::memo<int>(1, 3);
  Foo::memo<string>(1, 2);
  Foo::memo<string>(1, 3);
}

function f()[zoned] :mixed{
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
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A, f<>);
}
