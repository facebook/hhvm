<?php

function checker($x) {
  $msg = foo();
  $notice = $msg['title'].'. '.$msg['body'];
  foo();
  @list($a,$b) = $x;
  $x = @$x['a'];
  $x = @$x['b'];
  return $a - $b + $x;
}
