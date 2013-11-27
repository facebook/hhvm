<?php

function foo($a) {
  try {
    return 2;
  } finally {
    var_dump($a);
  }
  var_dump("lol");
}
var_dump(foo(4));
