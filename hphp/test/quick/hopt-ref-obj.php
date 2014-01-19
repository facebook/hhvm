<?php

class Foo { }

function run($a) {
  $b =& $a;

  return $b;
}

var_dump(run(new Foo()));
