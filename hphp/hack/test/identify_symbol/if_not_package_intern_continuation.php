<?hh

class IfNotPackageThenClass {}
class IfNotPackageElseClass {}
class IfNotPackageContinuationClass {}

function if_not_package_intern_continuation(): void {
  if (!package intern) {
    $then = new IfNotPackageThenClass();
  } else {
    $else = new IfNotPackageElseClass();
  }
  $continuation = new IfNotPackageContinuationClass();
}
