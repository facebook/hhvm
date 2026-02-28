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

public class Foobar {
  internal function foo(): void {}
}

//// changed-foobar.php
<?hh

module B;

public class Foobar {
  internal function foo(): void {}
}

//// base-bing.php
<?hh

module A;

function test(Foobar $x): void {
  $x->foo(); // will error here after change
}

//// changed-bing.php
<?hh

module A;

function test(Foobar $x): void {
  $x->foo(); // will error here after change
}
