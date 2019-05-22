<?hh

trait a {
  static function f(inout $a) { var_dump(__METHOD__); }
}

class c {
  use a {
    a::f as g;
  }
}

function test($a) {
  c::g(inout $a);
}
<<__EntryPoint>> function main(): void {
test(42);
}
