<?hh
// Test a case where only one of the overrides has a consistent type with the base.

class Base {
  public function foo(int $x): string {
    return (string)$x . " on base";
  }
}

class A extends Base {
  <<__Override>>
  public function foo(int $x): string {
    return (string)$x . " on a";
  }
}

class B extends Base {
  <<__Override>>
  public function foo(string $x): string {
    return $x . " on b";
  }
}

// We register these error handlers to ensure that the callee-side prologue
// checks are consistent with and without inlining.
function register_recoverable_error_handler() {
  set_error_handler(
    function(int $errno, string $errmsg, string $file, int $line) {
      throw new TypeError($errmsg);
    },
    E_RECOVERABLE_ERROR,
  );
}

function call_foo_on_base(Base $b) {
  return $b->foo(123);
}

function call_foo_on_a(A $a) {
  return $a->foo(123);
}

function call_foo_on_b(B $b) {
  return $b->foo('asdf');
}

<<__EntryPoint>>
function main(): void {
  register_recoverable_error_handler();
  $base = new Base();
  $a = new A();
  $b = new B();

  var_dump(call_foo_on_base($base));
  var_dump(call_foo_on_base($a));
  try {
    call_foo_on_base($b);
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
  var_dump(call_foo_on_a($a));
  var_dump(call_foo_on_b($b));
}
