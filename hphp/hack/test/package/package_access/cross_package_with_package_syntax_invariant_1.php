//// foo.php
<?hh

// package pkg7 (includes pkg1)
<<file: __PackageOverride('pkg7')>>

function foo(): void {}

//// bar.php
<?hh
// package pkg1

function bar(): void {
  invariant(package pkg7, "pkg7 not loaded");
  foo();  // allowed by invariant
}
