<?php

function thrower() {
  for ($i = 0; $i < 10; ++$i) mt_rand();  // make it not-inlinable
  // Make hhbbc not prove it's going to throw:
  for ($i = 0; $i < 10 + mt_rand() ? 1 : 0; ++$i) {
    mt_rand();
  }
  if ($i >= 10) throw new Exception('heh');
  return false;
}

function better_handle_exceptions_if_you_inline() {
  try {
    thrower();
  } catch (Exception $x) {
    echo "got it\n";
    return "all is well";
  }
}

function main() {
  // Generate a prologue for thrower:
  try { thrower(); } catch (Exception $x) {}

  // Push down bytecode offsets
  for ($i = 0; $i < 10; ++$i) mt_rand();
  for ($i = 0; $i < 10; ++$i) mt_rand();
  for ($i = 0; $i < 10; ++$i) mt_rand();
  for ($i = 0; $i < 10; ++$i) mt_rand();
  for ($i = 0; $i < 10; ++$i) mt_rand();
  for ($i = 0; $i < 10; ++$i) mt_rand();
  for ($i = 0; $i < 10; ++$i) mt_rand();
  for ($i = 0; $i < 10; ++$i) mt_rand();
  $z = better_handle_exceptions_if_you_inline();
  var_dump($z);
}

main();
