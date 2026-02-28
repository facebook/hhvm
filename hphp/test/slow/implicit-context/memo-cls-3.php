<?hh

class Foo {
  <<__MemoizeLSB(#KeyedByIC)>>
  static function memo<reify T>($a, $b)[zoned] :mixed{
    $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
    $kind = HH\ReifiedGenerics\get_type_structure<T>()['kind'];
    $str_hash = HH\Lib\Str\join($hash, ', '); // can't do var_dump due to keyedByIC
    echo "args: $a, $b hash: $str_hash, kind: $kind\n";
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
  ClassContext2::start(new B(0), g<>);
  Foo::memo<int>(1, 2);
  Foo::memo<int>(1, 3);
  Foo::memo<string>(1, 2);
  Foo::memo<string>(1, 3);
}

<<__EntryPoint>>
function main() :mixed{
  include 'implicit.inc';
  ClassContext::start(new A(0), f<>);
}
