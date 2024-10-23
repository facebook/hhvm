//// base-a-decl.php
<?hh


new module A {}

//// changed-a-decl.php
<?hh


new module A {}

//// base-foo-defn.php
<?hh

module A;

function foo(): void {}

//// changed-foo-defn.php
<?hh

module A;

internal function foo(): void {}
//// base-foo-use.php
<?hh

function call_foo(): void {
  foo();
}

//// changed-foo-use.php
<?hh

function call_foo(): void {
  foo();
}
//// base-foo-use-in-module.php
<?hh

module A;

function call_foo_in_module(): void {
  foo();
}

//// changed-foo-use-in-module.php
<?hh

module A;

function call_foo_in_module(): void {
  foo();
}
