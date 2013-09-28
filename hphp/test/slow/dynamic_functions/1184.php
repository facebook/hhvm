<?php

$a = 'test';
 function &test() {
 global $a;
 return $a;
}
  $b = $a();
 $b = 'ok';
 var_dump($a);
  $b = &$a();
 $b = 'ok';
 var_dump($a);
