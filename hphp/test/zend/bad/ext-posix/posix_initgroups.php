<?php

var_dump(posix_initgroups('foo', 'bar'));
var_dump(posix_initgroups(NULL, NULL));

?>