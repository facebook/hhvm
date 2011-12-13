<?php

function lambda_1() {
  echo "lambda one\n";
}

function __lambda_func() {
  echo "lambda func\n";
}

$f = create_function('', 'echo "created " . __FUNCTION__ . "\n"; ');
var_dump($f);
$f();

$f = 'lambda_1';
var_dump($f);
$f();

$inject = create_function('', '} echo "derp!\n"; if (0) {');

$bt = create_function('', 'var_dump(debug_backtrace()[0]["function"]);');
$bt();
