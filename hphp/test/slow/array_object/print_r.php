<?php

function test($flag = 0) {
  $ao = new ArrayObject([], $flag);
  $ao['foo'] = 'bar';
  $ao->bar = 'baz';

  print_r($ao);
}


<<__EntryPoint>>
function main_print_r() {
test();
test(ArrayObject::STD_PROP_LIST);
test(ArrayObject::ARRAY_AS_PROPS);
}
