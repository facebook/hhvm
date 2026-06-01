<?hh

class ThenBranchClass {}
class ElseBranchClass {}

function invariant_package_intern_if_then_branch(bool $b): void {
  if ($b) {
    invariant(package intern, "intern not loaded");
    $then = new ThenBranchClass();
  } else {
    $else = new ElseBranchClass();
  }
}
