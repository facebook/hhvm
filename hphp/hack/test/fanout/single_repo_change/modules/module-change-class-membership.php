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

internal class Foobar {}

//// changed-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module B;

internal class Foobar {}

//// base-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

final internal class Bing extends Foobar {}

//// changed-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

final internal class Bing extends Foobar {}
