<?php
class foo {
    public function foo() {}
}

class bar extends foo {
}
ReflectionClass::export("bar");
?>
