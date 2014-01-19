<?php

function foo() {
  global $a;
  $a = 1;
}
$a = 'a';
 $r = ++$a . $a;
 var_dump($r);
$a = 'a';
 $r = $a++ . $a;
 var_dump($r);
$a = 'a';
 $r = $a . ++$a;
 var_dump($r);
$a = 'a';
 $r = $a . $a++;
 var_dump($r);
$a = 'a';
 $r = ++$a . ++$a;
 var_dump($r);
$a = 'a';
 $r = ++$a . $a++;
 var_dump($r);
$a = 'a';
 $r = $a++ . ++$a;
 var_dump($r);
$a = 'a';
 $r = $a++ . $a++;
 var_dump($r);
$a = 'a';
 $b = 'b';
 $r = $a . foo() . $b;
 var_dump($r);
$a = 'a';
 $b = 'b';
 $r = $a . (foo() . $b);
 var_dump($r);
