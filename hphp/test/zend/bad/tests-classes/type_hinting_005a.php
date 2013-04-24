<?php
Class C { function f(array $a) {} }

echo "Compatible hint.\n";
Class D1 extends C { function f(array $a) {} }

echo "Class hint, should be array.\n";
Class D2 extends C { function f(SomeClass $a) {} }
?>
==DONE==