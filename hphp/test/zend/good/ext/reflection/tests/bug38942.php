<?php
class foo {
    public function __construct() {}
}

class bar extends foo {
}
ReflectionClass::export("bar");
