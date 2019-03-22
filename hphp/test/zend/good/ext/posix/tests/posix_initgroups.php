<?php

try { var_dump(posix_initgroups('foo', 'bar')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(posix_initgroups(NULL, NULL));

