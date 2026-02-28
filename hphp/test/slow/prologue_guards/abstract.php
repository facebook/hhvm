<?hh

abstract class AbsBase {
  abstract public function foo(int $x): string;
}

class InconsistentChild extends AbsBase {
  <<__Override>>
  public function foo(string $x): string {
    return $x . " on inconsistent child";
  }
}

class ConsistentChild extends AbsBase {
  <<__Override>>
  public function foo(int $x): string {
    return (string)$x." on consistent child";
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

function call_foo_on_base(AbsBase $base): string {
  return $base->foo(123);
}

<<__EntryPoint>>
function main(): void {
  register_recoverable_error_handler();
  $consistent = new ConsistentChild();
  $inconsistent = new InconsistentChild();
  // Should enter the prologue on the callee side.
  var_dump(call_foo_on_base($consistent));
  try {
    call_foo_on_base($inconsistent);
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
}
