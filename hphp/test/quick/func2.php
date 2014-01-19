<?php

function foo(&$a) {
  var_dump($a);
}

foo(array()[]);
