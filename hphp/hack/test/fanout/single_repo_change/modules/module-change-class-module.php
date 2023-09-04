//// base-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

//// changed-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

//// base-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

public class Foobar {
  internal function foo(): void {}
}

//// changed-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module B;

public class Foobar {
  internal function foo(): void {}
}

//// base-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

function test(Foobar $x): void {
  $x->foo(); // will error here after change
}

//// changed-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

function test(Foobar $x): void {
  $x->foo(); // will error here after change
}
