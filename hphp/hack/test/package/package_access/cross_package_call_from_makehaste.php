//// foo.php
<?hh
<<file:__PackageOverride('pkg4')>>
// package pkg4
// since pkg4 includes no paths we are sure that bar.php
// is not in the same package by accident
function foo(): void {}
function call_to_makehaste(): void {
  bar();  // ok since the flib/intern/makehaste directory is excluded from package checks
}

//// /flib/intern/makehaste/bar.php
<?hh
function bar(): void {}
function call_from_makehaste(): void {
  foo();  // ok since the flib/intern/makehaste directory is excluded from package checks
}
