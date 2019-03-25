<?php

DECLARE(independence=true);

FUNCTION test_echo_print() {
  ECHo "WHY ARE WE YELLING\n";
  PrinT "I HAVE NO IDEA!\n";
}

FUNction test_empty_isset_unset() {
  $foo = ARRay('a' => 'b');
  var_dump(EMpty($foo));
  var_dump(isSet($foo['a']));
  unSET($foo['a']);
  var_dump($foo);
}

fUNCTION test_global() {
  var_dump($GLOBALS['counter']);
  $GLOBALS['counter']++;
}

test_echo_print();
test_empty_isset_unset();
$counter = 3;
test_global();
test_global();
