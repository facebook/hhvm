<?php

class Foo {
    public function __call($a, $b) {
        print "nonstatic\n";
        var_dump($a);
    }
}
<<__EntryPoint>> function main() {
$a = new Foo;
call_user_func(array($a, 'aAa'));
}
