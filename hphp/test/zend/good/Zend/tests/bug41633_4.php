<?php
class Foo {
    const A = self::B;
    const B = "ok";
}
<<__EntryPoint>> function main() {
var_dump(defined("Foo::A"));
}
