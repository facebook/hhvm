//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}

//// changed-a-decl.php
<?hh

//// base-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

internal class FooA {}

//// changed-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

internal class FooA {}
//// base-b-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

internal function foobar(): void {}
//// changed-b-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

internal function foobar(): void {}

//// base-c-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

internal newtype Foo = int;
//// changed-c-use.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

internal newtype Foo = int;
