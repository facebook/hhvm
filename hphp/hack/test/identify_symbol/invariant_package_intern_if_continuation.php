<?hh

class InvariantIfThenClass {}
class InvariantIfElseClass {}
class InvariantIfContinuationClass {}

function invariant_package_intern_if_continuation(bool $b): void {
  if ($b) {
    invariant(package intern, "intern not loaded");
    $then = new InvariantIfThenClass();
  } else {
    $else = new InvariantIfElseClass();
  }
  $continuation = new InvariantIfContinuationClass();
}
