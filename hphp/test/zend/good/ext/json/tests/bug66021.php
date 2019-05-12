<?php

class Foo {
    private $bar = 'baz';
}
<<__EntryPoint>> function main() {
echo json_encode([[], (object)[], new Foo], JSON_PRETTY_PRINT), "\n";
}
