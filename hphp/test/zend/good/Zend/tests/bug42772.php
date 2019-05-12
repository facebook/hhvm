<?php
class Foo {
    static public $foo;
    function __toString() {
        self::$foo = $this;
        return 'foo';
    }
}
<<__EntryPoint>> function main() {
$foo = (string)new Foo();
var_dump(Foo::$foo);
}
