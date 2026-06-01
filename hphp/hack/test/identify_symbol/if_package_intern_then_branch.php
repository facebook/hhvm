<?hh

class IfPackageInternClass {}
class IfPackageElseClass {}

function if_package_intern_then_branch(): void {
  if (package intern) {
    $then = new IfPackageInternClass();
  } else {
    $else = new IfPackageElseClass();
  }
}
