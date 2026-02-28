//// base-decls.php
<?hh


new module A {}
new module B {}

//// changed-decls.php
<?hh


new module A {}
new module B {}

//// base-foobar.php
<?hh

module A;

internal class Foobar {}

//// changed-foobar.php
<?hh

module B;

internal class Foobar {}

//// base-bing.php
<?hh

module A;

final internal class Bing extends Foobar {}

//// changed-bing.php
<?hh

module A;

final internal class Bing extends Foobar {}
