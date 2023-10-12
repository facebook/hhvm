<?hh // strict

function test_bad(): void {
  $foo = null;
  $_ = (string)$foo;
}
