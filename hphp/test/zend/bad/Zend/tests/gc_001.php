<?php
gc_disable();
var_dump(gc_enabled());
gc_enable();
var_dump(gc_enabled());
?>