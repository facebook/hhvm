<?hh

class NestedInvariantClass {}

function invariant_package_intern_nested_block(bool $outer, bool $inner): void {
  if ($outer) {
    invariant(package intern, "intern not loaded");
    if ($inner) {
      $nested = new NestedInvariantClass();
    }
  }
}
