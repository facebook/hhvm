<?php

function test($a) {
   var_dump(func_get_arg(0));
  var_dump(func_get_arg(1));
  var_dump(func_get_arg(2));
}

 <<__EntryPoint>>
function main_14() {
test(2, 'ok', array(1));
}
