//// decls.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>

new module A {}
new module B {}

//// A.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module A;

internal type Foo = int;

internal class Crosby {}

//// also-A.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module A;

function foobar<T>(T $t): void where T = Foo {
  $t as Foo;
  foobar($t);
}

internal function quxx<T as Foo>(T $_): void {}

//// not-A.php
<?hh

<<file:__EnableUnstableFeatures('modules')>>
module B;

function baz<T>(T $t): void where T = Foo {
  $t as Foo;
  baz($t);
}

function bing<T as Foo>(T $_): void {}

function get_crosby<T>(): T where T = Crosby {
  invariant_violation('!!!');
}
