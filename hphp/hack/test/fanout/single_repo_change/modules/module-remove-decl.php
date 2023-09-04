//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}

//// changed-a-decl.php
<?hh

//// base-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

internal class FooA {}

//// changed-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

internal class FooA {}
//// base-b-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

internal function foobar(): void {}
//// changed-b-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

internal function foobar(): void {}

//// base-c-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;
module newtype Bar = Foo;
internal newtype Foo = int;
//// changed-c-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;
module newtype Bar = Foo;
internal newtype Foo = int;
