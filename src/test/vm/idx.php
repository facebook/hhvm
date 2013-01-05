<?php

function idx($a, $b) {
  var_dump("override");
}

function foo($a,$b) {
  idx($a, $b);
}

foo("a", "b");
