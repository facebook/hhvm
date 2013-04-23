<?php

echo "*** Test by calling function with permission error ***\n";

posix_setuid(0);
var_dump(posix_errno());

?>