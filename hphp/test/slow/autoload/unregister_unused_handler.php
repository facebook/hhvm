<?php

function a($class) {
  print "a\n";
}
function b($class) {
  print "b\n";
}

spl_autoload_register('a');
spl_autoload_register('b');
var_dump(spl_autoload_functions()); // a and b
spl_autoload_unregister('b');
var_dump(spl_autoload_functions()); // a only
spl_autoload_unregister('b');
var_dump(spl_autoload_functions()); // still a
spl_autoload_unregister('a');
var_dump(spl_autoload_functions() === array());
