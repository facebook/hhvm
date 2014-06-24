<?php function test2(){ var_dump(xdebug_call_line()); } test2();
test2();

function test() {
  var_dump(xdebug_call_line());
}

var_dump(xdebug_call_line());
test();
eval('var_dump(xdebug_call_line());');
eval('test();');
