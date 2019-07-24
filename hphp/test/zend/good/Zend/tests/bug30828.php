<?hh
class A {
  function __construct() {
    debug_print_backtrace();
    $bt = debug_backtrace();
    foreach ($bt as $t) {
      print ($t['class'] ?? '').($t['type'] ?? '').$t['function']."\n";
    }
  }

  function foo(): void {
    debug_print_backtrace();
    $bt = debug_backtrace();
    foreach ($bt as $t) {
      print ($t['class'] ?? '').($t['type'] ?? '').$t['function']."\n";
    }
  }

  static function bar(): void {
    debug_print_backtrace();
    $bt = debug_backtrace();
    foreach ($bt as $t) {
      print ($t['class'] ?? '').($t['type'] ?? '').$t['function']."\n";
    }
  }
}

class B extends A {
  function __construct() {
    parent::__construct();
  }

  function foo(): void {
    parent::foo();
  }

  static function bar(): void {
    parent::bar();
  }
}
<<__EntryPoint>> function main(): void {
$b = new B();
$b->foo();
B::bar();
}
