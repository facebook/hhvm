//// tosearch.php
<?hh //strict

function bar(int $arg): int {
  return $arg;
}

function baz(int $arg): int {
  return $arg;
}


//// matcherpattern.php
<?hh //strict

function bar(int $arg): int {
  return $arg;
}

function __KSTAR(int $arg): int { }
