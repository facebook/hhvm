<?php
gc_disable();
var_dump(gc_enabled());
echo ini_get('zend.enable_gc') . "\n";
gc_enable();
var_dump(gc_enabled());
echo ini_get('zend.enable_gc') . "\n";
?>