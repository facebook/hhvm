<?php

$fp = fopen(__FILE__, "r");
fseek($fp, __COMPILER_HALT_OFFSET__+1);
print fread($fp, 1000);

__HALT_COMPILER();
Overlay information...