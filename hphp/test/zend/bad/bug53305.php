<?php
error_reporting(E_ALL);

define('__COMPILER_HALT_OFFSET__1', 1);
define('__COMPILER_HALT_OFFSET__2', 2);
define('__COMPILER_HALT_OFFSET__', 3);
define('__COMPILER_HALT_OFFSET__1'.chr(0), 4);

var_dump(__COMPILER_HALT_OFFSET__1);
var_dump(constant('__COMPILER_HALT_OFFSET__1'.chr(0)));

?>