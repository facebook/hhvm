<?php
class foo {
    public function __construct() {}
}

class bar extends foo {
}
<<__EntryPoint>> function main() {
ReflectionClass::export("bar");
}
