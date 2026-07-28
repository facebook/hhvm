//// pkg4/foo.php
<?hh
// package pkg4 by path; bar.php lives elsewhere so it is not in the same
// package by accident
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
