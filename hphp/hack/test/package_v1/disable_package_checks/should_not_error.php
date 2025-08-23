///// module_a.php
<?hh

new module a {}

///// module_b.php
<?hh

new module b {}

///// foo.php
<?hh

module a;

function foo(): void {
  bar();
}

///// bar.php
<?hh

module b;

function bar(): void {}
