//// file1.php
<?hh // strict

newtype N<+T> = int;

//// file2.php
<?hh // strict

function nullthrows<T>(?T $x): T {
  throw new Exception();
}

function f(N<int> $_): void {}

function test(?N<num> $x): void {
  f(nullthrows($x));
}
