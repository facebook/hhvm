//// base-a-decl.php
<?hh


new module A {}

//// changed-a-decl.php
<?hh


new module A {}
//// base-a-use.php
<?hh


class FooA {}

//// changed-a-use.php
<?hh

module A;

internal class FooA {}
