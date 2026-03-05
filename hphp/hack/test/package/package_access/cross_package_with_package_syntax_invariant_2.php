//// foo.php
<?hh

// package pkg7 (includes pkg1)
<<file: __PackageOverride('pkg7')>>

function foo(): void {}

//// bar.php
<?hh
// package pkg1

function bar(bool $x): void {
  if ($x) {
    invariant(package pkg7, "pkg7 not loaded");
    foo();  // allowed by invariant
  }
  foo();  // error here, out of the scope of invariant
}
