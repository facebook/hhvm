<?php
Class C { function f(SomeClass $a) {} }

echo "Array hint, should be class.\n";
Class D extends C { function f(array $a) {} }
?>
==DONE==