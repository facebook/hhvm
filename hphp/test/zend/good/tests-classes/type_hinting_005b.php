<?php
Class C { function f(array $a) {} }

echo "No hint, should be array.\n";
Class D extends C { function f($a) {} }
?>
==DONE==