<?hh

// The trait is cloned into two classes - this should force a clone. Ensure
// that the AttrHasInheritedReturnTypes is set/cleared cleanly.
interface I {
  public function foo(): string;
}

trait T implements I {
  <<__Memoize>>
  public function foo(): int {
    return __hhvm_intrinsics\launder_value(42);
  }
}

class WithoutOverride {
  use T;
}

class WithOverride {
  use T;

  <<__Memoize>>
  public function foo(): float {
    return __hhvm_intrinsics\launder_value(1.5);
  }
}

<<__EntryPoint>>
function main(): void {
  var_dump(new WithoutOverride()->foo());
  var_dump(new WithOverride()->foo());
}
