<?php

function test() {
  $x =& $y;
  $x = false;
  $y .= 'hello';
  echo $x;
}
test();
