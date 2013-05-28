<?php

function pid($x) {
  var_dump($x);
  return $x;
}
function f($x) {
  return $x;
}
function ttest() {
  return f(pid('arg1'),pid('arg2'));
}
ttest();
