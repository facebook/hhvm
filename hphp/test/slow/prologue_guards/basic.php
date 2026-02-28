<?hh

class Base {
  public function foo(int $x): string {
    return (string)$x . " on base";
  }
}

class Child extends Base {
  <<__Override>>
  // Inconsistent with Base.
  public function foo(string $x): string {
    return $x . " on child";
  }
}

class Grandchild extends Child {
  <<__Override>>
  // Consistent with Child.
  public function foo(string $x): string {
    return $x . " on grandchild";
  }
}

function call_foo_with_int(Base $base): string {
  return $base->foo(123);
}

function call_foo_with_str_on_child(Child $child): string {
  return $child->foo("456");
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
  $base = new Base();
  $child = new Child();
  $gc = new Grandchild();

  // Should enter the prologue on the callee side.
  var_dump(call_foo_with_int($base));
  try {
    call_foo_with_int($child);
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
  try {
    call_foo_with_int($gc);
  } catch (TypeError $e) {
    var_dump($e->getMessage());
  }
  // We should not enter the prologue in either case.
  var_dump(call_foo_with_str_on_child($child));
  var_dump(call_foo_with_str_on_child($gc));
}
