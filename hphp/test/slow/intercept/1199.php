<?php

function test() {
  define('X', 2);
  define('Y', 3);
  fb_intercept('foo', 'bar');
  var_dump(X);
  define('Y', 4);
  var_dump(Y);
}

<<__EntryPoint>>
function main_1199() {
test();
}
