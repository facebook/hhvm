<?hh

class NotInternBlockClass {}

function invariant_not_package_intern(): void {
  invariant(!package intern, "intern loaded");
  $c = new NotInternBlockClass();
}
