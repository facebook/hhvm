<?php

function foo() {
  global $a;
  $a = 123;
  $GLOBALS['a'] = "the"; // violate type inference via SetG
  var_dump($a);

  $a = 123;
  $GLOBALS['a'] .= "duke"; // via SetOpG
  var_dump($a);

  $a = 123;
  $b = $GLOBALS;
  $b['a'] = "of"; // via SetM
  var_dump($a);
}

foo();

function main1() {
  global $b;
  $GLOBALS['b'] = 123; // via SetG
  var_dump($b);

  $b = 123;
  $GLOBALS['b'] .= "moot"; // via SetOpG
  var_dump($b);

  $b = 123;
  $c = $GLOBALS;
  $c['b'] = "hoof"; // via SetM
  var_dump($b);
}
main1();
