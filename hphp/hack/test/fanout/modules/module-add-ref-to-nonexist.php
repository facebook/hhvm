//// base-decls.php
<?hh
// If a reference to a module is added without the corresponding module
// declaration, we want to ensure that we properly invalidate any type that
// refers to the non-existent declaration s.t. we properly recheck any files
// with errors.

//// changed-decls.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>

new module A {}

//// base-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

class Foobar {}

//// changed-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

class Foobar {}
