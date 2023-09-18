//// module_foo.php
<?hh
namespace A {
  new module foo {}
}

//// module_bar.php
<?hh
namespace {
  new module bar {}
}

//// foo.php
<?hh


module foo;

namespace Bing {
  // TODO(T108206307) You _need_ a top level symbol to attach a new module to in order
  // to get an unbound new module name error.
  function f(): void {}
}

//// bar.php
<?hh


module bar;

function g(): void {
  Bing\f(); // Not an error, demonstrates that modules have no effect on naming
}

//// a-foo.php
<?hh


module A\foo;
function h(): void {}
