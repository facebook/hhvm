<?php

function SplAutoloadTest1($name) {}
function SplAutoloadTest2($name) {}

var_dump(spl_autoload_functions());

spl_autoload_register();

var_dump(spl_autoload_functions());

spl_autoload_register('SplAutoloadTest1');
spl_autoload_register('SplAutoloadTest2');
spl_autoload_register('SplAutoloadTest1');

var_dump(spl_autoload_functions());

spl_autoload_unregister('SplAutoloadTest1');

var_dump(spl_autoload_functions());

spl_autoload_unregister('spl_autoload_call');

var_dump(spl_autoload_functions());

spl_autoload_register();

var_dump(spl_autoload_functions());

spl_autoload_unregister('spl_autoload');

var_dump(spl_autoload_functions());

?>
===DONE===
<?php exit(0); ?>