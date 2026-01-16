//// foo.php
<?hh

// package pkg4 (disjoint from pkg1)
<<file: __PackageOverride('pkg4')>>

function foo(): void {}

//// bar.php
<?hh
// package pkg1

function bar(): void {
  invariant(package pkg4, "pkg4 not loaded");
  foo();  // FIXME: this should be allowed by invariant
}
