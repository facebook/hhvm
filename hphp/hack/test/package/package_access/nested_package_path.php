//// foo.php
<?hh

function foo(): void {
  bar(); // this should be an error as pkg1 does not include pkg6
}

//// pkg6/bar.php
<?hh

function bar(): void {}

function zot(): void {
  foo(); // this should not be an error as pkg6 includes pkg1
}
