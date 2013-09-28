<?php

function foo($e='e') {
  return '<a name="'.$e.'" id="'.$e.'"></a>';
}
function test() {
  echo foo();
}
test();
