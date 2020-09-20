//// file1.php
<?hh

function f(inout int $arg): void {}

//// file2.php
<?hh

function g(inout string $arg): void {
  f(inout $arg);
}
