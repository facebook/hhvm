<?hh

class Foo {
  public static Foo $x;
  <<__Memoize(#KeyedByIC)>>
  readonly function memo<reify T>($a, $b)[zoned] :mixed{
    $hash = quoted_printable_encode(
      HH\ImplicitContext\_Private\get_implicit_context_memo_key()
    );
    $kind = HH\ReifiedGenerics\get_type_structure<T>()['kind'];
    echo "args: $a, $b hash: $hash, kind: $kind\n";
  }
}

function g()[zoned] :mixed{
  (readonly Foo::$x)->memo<int>(1, 2);
  (readonly Foo::$x)->memo<int>(1, 3);
  (readonly Foo::$x)->memo<string>(1, 2);
  (readonly Foo::$x)->memo<string>(1, 3);
}

function f()[zoned] :mixed{
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
function main() :mixed{
  include 'implicit.inc';
  Foo::$x = new Foo();
  ClassContext::start(new A, f<>);
}
