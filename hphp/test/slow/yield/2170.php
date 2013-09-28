<?php

function bar($x) {
 return $x ? $x + 1 : false;
 }
function foo($a) {
  $x = bar($a);
  switch ($x) {
    case 'hello': echo 1;
 break;
    case bar(3): echo 2;
 break;
  }
  yield $x;
}
foreach(foo(3) as $x) {
 var_dump($x);
 }
