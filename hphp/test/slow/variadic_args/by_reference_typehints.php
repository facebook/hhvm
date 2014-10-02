<?php

function variadic_by_ref(int &...$args) {
  foreach ($args as $a) { $a++; }
}

function main() {
  $a = 10;
  $b = 20;
  variadic_by_ref($a, $b);
}
main();
