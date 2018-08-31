<?php


<<__EntryPoint>>
function main_offset_set() {
$it = new ArrayIterator();
$it[] = "foo";
$it[] = "bar";
$it[] = "baz";

foreach($it as $value) {
  var_dump($value);
}

var_dump(sizeof($it));
}
