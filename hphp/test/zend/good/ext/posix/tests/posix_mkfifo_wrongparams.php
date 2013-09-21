<?php
posix_mkfifo(null);
var_dump(posix_mkfifo(null, 0644));
?>
===DONE===