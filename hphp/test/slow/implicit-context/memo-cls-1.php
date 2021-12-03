<?hh

class Foo {
  static Foo $x;
  <<__PolicyShardedMemoize>>
  function memo($a, $b)[zoned] {
    $hash = quoted_printable_encode(
      HH\ImplicitContext\_Private\get_implicit_context_memo_key()
    );
    echo "args: $a, $b hash: $hash\n";
  }
}

function g() {
  Foo::$x->memo(1, 2);
  Foo::$x->memo(1, 3);
}

function f() {
  Foo::$x->memo(1, 2);
  Foo::$x->memo(1, 3);
  ClassContext2::start(new B, g<>);
  Foo::$x->memo(1, 2);
  Foo::$x->memo(1, 3);
}

<<__EntryPoint>>
function main() {
  include 'implicit.inc';
  Foo::$x = new Foo();
  ClassContext::start(new A, f<>);
}
