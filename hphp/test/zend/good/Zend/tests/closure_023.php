<?php
class foo {
    public static function bar() {
        $func = function() { echo "Done"; };
        $func();
    }
}
foo::bar();