<?php

function test($flag = 0) {
  $ao = new ArrayObject([], $flag);
  $ao['foo'] = 'bar';
  $ao->bar = 'baz';

  var_dump($ao);
}


<<__EntryPoint>>
function main_var_dump() {
test();
test(ArrayObject::STD_PROP_LIST);
test(ArrayObject::ARRAY_AS_PROPS);
}
