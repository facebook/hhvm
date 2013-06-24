<?php

function zero() { return 0; }
function foo() { return "0x10"; }
function twelve() { return 12; }

function main() {
  var_dump(zero() + foo());
  var_dump(zero() - foo());
  var_dump(zero() / foo());
  var_dump(zero() * foo());

  var_dump(foo() + zero());
  var_dump(foo() - zero());
  var_dump(foo() / zero());
  var_dump(foo() * zero());

  var_dump(twelve() + foo());
  var_dump(twelve() - foo());
  var_dump(twelve() / foo());
  var_dump(twelve() * foo());

  var_dump(foo() + twelve());
  var_dump(foo() - twelve());
  var_dump(foo() / twelve());
  var_dump(foo() * twelve());
}

main();
