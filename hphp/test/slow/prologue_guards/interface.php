<?hh

interface IInconsistent {
  public function foo(int $x): string;
}

class CInconsistent implements IInconsistent {
  public function foo(string $x): string {
    return $x . " on inconsistent implementation";
  }
}

interface IConsistent{
  public function foo(int $x): string;
}

class CConsistent implements IConsistent {
  public function foo(int $x): string {
    return (string)$x." on consistent implementation";
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

function call_foo_on_inconsistent(IInconsistent $i): string {
  return $i->foo(123);
}

function call_foo_on_consistent(IConsistent $i): string {
  return $i->foo(123);
}

<<__EntryPoint>>
function main(): void {
  register_recoverable_error_handler();
  $consistent = new CConsistent();
  $inconsistent = new CInconsistent();
  // Should enter the prologue on the callee side.
  var_dump(call_foo_on_consistent($consistent));
  try {
    call_foo_on_inconsistent($inconsistent);
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
}
