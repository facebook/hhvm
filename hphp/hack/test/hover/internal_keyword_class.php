//// modules.php
<?hh

new module foo {}

//// test.php
<?hh
module foo;

internal class Foo {}
// ^ hover-at-caret
