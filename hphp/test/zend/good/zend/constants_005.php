<?php
var_dump(ZEND_THREAD_safe);
define("ZEND_THREAD_safe", 123);
var_dump(ZEND_THREAD_safe);
?>