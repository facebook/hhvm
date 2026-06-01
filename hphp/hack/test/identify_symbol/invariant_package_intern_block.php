<?hh

class InternBlockClass {}

function invariant_package_intern_block(): void {
  invariant(package intern, "intern not loaded");
  $c = new InternBlockClass();
}
