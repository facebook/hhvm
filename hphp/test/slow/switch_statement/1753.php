<?php

function test() {
  $a = 2;
  switch ($a) {
    case ++$a: var_dump('ok');
 break;
    case 2: var_dump('broken');
 break;
    case 3: var_dump('really broken');
 break;
    default: var_dump('fail');
 break;
  }
  $a = 'b';
  $b = 2;
  switch ($$a) {
    case ++$$a: var_dump('broken');
 break;
    case 2: var_dump('ok');
 break;
    case 3: var_dump('really broken');
 break;
    default: var_dump('fail');
 break;
  }
}
$a = 2;
switch ($a) {
  case ++$a: var_dump('ok');
 break;
  case 2: var_dump('broken');
 break;
  case 3: var_dump('really broken');
 break;
  default: var_dump('fail');
 break;
}
$a = 'b';
$b = 2;
switch ($$a) {
  case ++$$a: var_dump('broken');
 break;
  case 2: var_dump('ok');
 break;
  case 3: var_dump('really broken');
 break;
  default: var_dump('fail');
 break;
}
test();
