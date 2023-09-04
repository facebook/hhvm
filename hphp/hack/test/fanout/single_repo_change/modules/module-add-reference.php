//// base-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}

//// changed-a-decl.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}
//// base-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

class FooA {}

//// changed-a-use.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

internal class FooA {}
