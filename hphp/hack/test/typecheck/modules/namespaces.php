//// modules.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>

namespace A {
  new module foo {}
}

namespace {
  new module bar {}
}

//// foo.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('foo')>>

namespace Bing {
  // TODO(T108206307) You _need_ a top level symbol to attach a new module to in order
  // to get an unbound new module name error.
  function f(): void {}
}

//// bar.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('bar')>>

function g(): void {
  Bing\f(); // Not an error, demonstrates that modules have no effect on naming
}

//// a-foo.php
<?hh

<<file:__EnableUnstableFeatures('modules'), __Module('A\\foo')>>

function h(): void {}
