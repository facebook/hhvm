<?hh

class IfPackageInternClass {}
class IfPackageElseClass {}
class IfPackageContinuationClass {}

function if_package_intern_continuation(): void {
  if (package intern) {
    $then = new IfPackageInternClass();
  } else {
    $else = new IfPackageElseClass();
  }
  $continuation = new IfPackageContinuationClass();
}
