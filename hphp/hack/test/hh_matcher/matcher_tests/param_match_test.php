//// tosearch.php
<?hh //strict

function foo(float $arg1, float $arg2) : bool {
  return $arg1 === $arg2;
}

function bar(int $arg1, float $arg2) : bool {
  return $arg1 === $arg2;
}

function baz(int $arg1, float $arg2) : int {
  return $arg1 === $arg2;
}

//// matcherpattern.php
<?hh //strict

class __KSTAR {}

function __SOMENAME(int $arg1, float $arg2) : bool {
  return $arg1 === $arg2;
}

class __KSTAR {}
