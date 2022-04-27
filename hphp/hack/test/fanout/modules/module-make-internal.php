//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}

//// changed-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}

//// base-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

class FooA {}

//// changed-foo-defn.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

internal class FooA {}
//// base-foo-use.php
<?hh

function make_fooa(): FooA {
  return new FooA();
}

function make_secret_fooa(): mixed {
  return new FooA();
}


//// changed-foo-use.php
<?hh

function make_fooa(): FooA {
  return new FooA();
}

function make_secret_fooa(): mixed {
  return new FooA();
}
