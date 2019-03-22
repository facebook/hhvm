<?php
try { posix_mkfifo(null); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(posix_mkfifo(null, 0644));
echo "===DONE===\n";
