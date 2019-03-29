<?php

function __autoload($a) {
  include 'class_alias_007.inc';
}

class_alias('foo', 'bar', 1);

var_dump(new foo, new bar);

