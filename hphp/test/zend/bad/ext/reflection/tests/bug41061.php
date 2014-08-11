<?php

function foo() {
}
 
class bar {
    private function foo() {
    }
}

Reflection::export(new ReflectionFunction('foo'));
Reflection::export(new ReflectionMethod('bar', 'foo'));
?>
===DONE===
<?php exit(0); ?>
