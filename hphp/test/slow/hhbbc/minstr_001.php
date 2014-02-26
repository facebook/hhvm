<?php

function foo() {
  $lol = new stdclass;
  $x[$lol] = 2;
  var_dump($x);
}

foo();

