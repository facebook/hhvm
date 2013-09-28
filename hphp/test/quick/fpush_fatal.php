<?php

function blah() {}
function foo() {
  global $x;
  blah(blah(), $x());
}

$x = 'asdasdasd';
foo();
