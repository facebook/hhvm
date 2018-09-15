<?php

function test() {
   var_dump(func_get_arg(0));
  var_dump(func_get_arg(1));
}

 <<__EntryPoint>>
function main_12() {
test(2, 'ok');
}
