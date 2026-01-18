//// foo.php
<?hh

// package pkg4 (disjoint from pkg1)
<<file: __PackageOverride('pkg4')>>

function foo(): void {}

//// bar.php
<?hh
// package pkg1

function bar(bool $x): void {
  if ($x) {
    invariant(package pkg4, "pkg4 not loaded");
    foo();  // allowed by invariant
  }
  foo();  // error here, out of the scope of invariant
}
