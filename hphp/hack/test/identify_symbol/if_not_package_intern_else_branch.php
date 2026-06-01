<?hh

class IfNotPackageThenClass {}
class IfNotPackageElseClass {}

function if_not_package_intern_else_branch(): void {
  if (!package intern) {
    $then = new IfNotPackageThenClass();
  } else {
    $else = new IfNotPackageElseClass();
  }
}
