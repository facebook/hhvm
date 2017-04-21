<?php

function alpha() {
  throw new Error('Foo');
}

function beta() {
  alpha();
}

function gamma() {
  beta();
}

function main() {
  try {
    gamma();
  } catch (Throwable $t) {
    var_dump($t->getMessage());
  }
}

main();
