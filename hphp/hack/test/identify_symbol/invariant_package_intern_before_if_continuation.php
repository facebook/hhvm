<?hh

class AfterInvariantThenClass {}
class AfterInvariantElseClass {}
class AfterInvariantContinuationClass {}

function invariant_package_intern_before_if(bool $b): void {
  invariant(package intern, "intern not loaded");
  if ($b) {
    $then = new AfterInvariantThenClass();
  } else {
    $else = new AfterInvariantElseClass();
  }
  $continuation = new AfterInvariantContinuationClass();
}
