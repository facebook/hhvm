//// file1.php
<?hh

function foo(): X {
  return new X();
}

//// file2.php
<?hh // strict

function test(): void {
  // The method 'bar' does not exist but that's ok
  foo()->bar();
}
