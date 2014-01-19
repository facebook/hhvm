<?php

function blocker() {
 print 'block';
 }
function f($x) {
  $x = (string) $x;
  blocker();
  var_dump($x[0]);
  var_dump($x[0]);
}
var_dump('foo');
