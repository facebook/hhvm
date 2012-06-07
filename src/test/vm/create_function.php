<?php

function lambda_1() {
  echo "lambda one\n";
}

function __lambda_func() {
  echo "lambda func\n";
}

function main() {
  $f = create_function('', 'echo "created " . __FUNCTION__ . "\n"; ');
  var_dump($f);
  $f();

  $f = 'lambda_1';
  var_dump($f);
  $f();

  $inject = create_function('', '} echo "derp!\n"; if (0) {');

  $bt = create_function('', 'var_dump(debug_backtrace()[0]["function"]);');
  $bt();

  for ($i = 0; $i < 5; $i++) {
    $foo = create_function('$item', 'return "foo - $item";');
    print $foo . "\n";
  }

  for ($i = 0; $i < 5; $i++) {
    $bar = create_function('$item', 'return "bar - $item";');
    print $bar . "\n";
  }
}
main();

