<?php

function asd($x) {
  var_dump($x);
}

function foo($b) {
  asd($b[]);
}

foo(array());
