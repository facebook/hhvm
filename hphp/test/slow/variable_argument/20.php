<?php

function test($a = 10) {
   var_dump($a);
  var_dump(func_get_args());
}

 <<__EntryPoint>>
function main_20() {
test();
 test(1);
 test(1, 2);
}
