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

//// base-f.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

internal function f(): void {}

//// changed-f.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module B;

internal function f(): void {}

//// base-g.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

function g(): void {
  f();
}

//// changed-g.php
<?hh
<<file: __EnableUnstableFeatures('modules')>>
module A;

function g(): void {
    f();
}
