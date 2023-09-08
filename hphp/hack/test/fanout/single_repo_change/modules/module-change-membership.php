//// base-decls.php
<?hh


new module A {}
new module B {}

//// changed-decls.php
<?hh


new module A {}
new module B {}

//// base-f.php
<?hh

module A;

internal function f(): void {}

//// changed-f.php
<?hh

module B;

internal function f(): void {}

//// base-g.php
<?hh

module A;

function g(): void {
  f();
}

//// changed-g.php
<?hh

module A;

function g(): void {
    f();
}
