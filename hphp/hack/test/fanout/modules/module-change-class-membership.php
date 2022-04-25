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
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

internal class Foobar {}

//// changed-foobar.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('B')>>

internal class Foobar {}

//// base-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

final internal class Bing extends Foobar {}

//// changed-bing.php
<?hh
<<file: __EnableUnstableFeatures('modules'), __Module('A')>>

final internal class Bing extends Foobar {}
