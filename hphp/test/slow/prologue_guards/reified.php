<?hh

class BaseWithReifiedGeneric {
  public function foo<reify T>(T $x): string {
    return "base";
  }
}

class OverrideWithReifiedGeneric extends BaseWithReifiedGeneric {
  <<__Override>>
  public function foo<reify T>(T $x): string {
    return "override";
  }
}

class InconsistentBaseWithReifiedGeneric {
  public function foo<reify T>(T $x): string {
    return "inconsistent base";
  }
}

class InconsistentOverrideNoReifiedGeneric extends InconsistentBaseWithReifiedGeneric {
  <<__Override>>
  public function foo(string $x): string {
    return $x;
  }
}

function call_foo_on_consistent_base(BaseWithReifiedGeneric $c, mixed $param): string {
  return $c->foo<string>($param);
}

function call_foo_on_inconsistent_base(InconsistentBaseWithReifiedGeneric $c, mixed $param): string {
  return $c->foo<string>($param);
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

<<__EntryPoint>>
function main(): void {
  register_recoverable_error_handler();

  var_dump(call_foo_on_consistent_base(new BaseWithReifiedGeneric(), "param"));
  var_dump(call_foo_on_consistent_base(new OverrideWithReifiedGeneric(), "param"));
  var_dump(call_foo_on_inconsistent_base(new InconsistentBaseWithReifiedGeneric(), "param"));
  var_dump(call_foo_on_inconsistent_base(new InconsistentOverrideNoReifiedGeneric(), "param"));

  // Type errors
  try {
    var_dump(call_foo_on_consistent_base(new BaseWithReifiedGeneric(), 1));
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(call_foo_on_consistent_base(new OverrideWithReifiedGeneric(), 1));
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(call_foo_on_inconsistent_base(new InconsistentBaseWithReifiedGeneric(), 1));
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump(call_foo_on_inconsistent_base(new InconsistentOverrideNoReifiedGeneric(), 1));
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
}
