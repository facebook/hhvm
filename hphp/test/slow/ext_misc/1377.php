<?php

function __autoload($c) {
  var_dump($c);
}
function test() {
  var_dump(is_subclass_of('C', 'D'));
  var_dump(get_class_methods('C'));
  var_dump(method_exists('C', 'foo'));
  class C {
}
  var_dump(is_subclass_of('C', 'D'));
  var_dump(is_subclass_of('C', 'C'));
}
test();
var_dump(class_exists('C'));
