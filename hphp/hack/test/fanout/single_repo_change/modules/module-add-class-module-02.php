//// base-decls.php
<?hh


new module A {}

//// changed-decls.php
<?hh


new module A {}

//// base-foobar.php
<?hh


class Foobar {
  internal function foo(): void {}
}

//// changed-foobar.php
<?hh

module A;

class Foobar {
  internal function foo(): void {}
}

//// base-bing.php
<?hh


function test(Foobar $x): void {
  $x->foo();
}

//// changed-bing.php
<?hh


function test(Foobar $x): void {
  $x->foo();
}
