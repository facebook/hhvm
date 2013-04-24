<?php
function autoload1() {}

function autoload2() {}

spl_autoload_register('autoload2');
spl_autoload_register('autoload1', true, true);
var_dump(spl_autoload_functions());

spl_autoload_unregister('autoload2');
var_dump(spl_autoload_functions());
?>