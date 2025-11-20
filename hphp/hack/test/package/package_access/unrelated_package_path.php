///// pkg5/foo.php
<?hh

function foo(): void {
  bar(); // this should be an error as pkg6 does not include pkg5
}

///// pkg6/bar.php
<?hh

function bar(): void {}

function zot(): void {
  foo(); // this should be an error as pkg6 does not include pkg5
}
