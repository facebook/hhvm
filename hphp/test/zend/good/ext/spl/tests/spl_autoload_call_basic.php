<?php
function customAutolader($class) {
    require_once __DIR__ . '/testclass.class.inc';
}
spl_autoload_register('customAutolader');

spl_autoload_call('TestClass');
var_dump(class_exists('TestClass', false));
?>