<?hh

class BeforeInvariantClass {}
class AfterInvariantClass {}

function invariant_package_intern_mid_block(): void {
  $before = new BeforeInvariantClass();
  invariant(package intern, "intern not loaded");
  $after = new AfterInvariantClass();
}
