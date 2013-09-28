<?php
Class C { function f($a) {} }

echo "Array hint, should be nothing.\n";
Class D extends C { function f(array $a) {} }
?>
==DONE==