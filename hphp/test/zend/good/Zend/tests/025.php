<?php

class foo {
    static public function a() {
        print "ok\n";
    }
}
<<__EntryPoint>> function main() {
$a = 'a';
$b = 'a';

$class = 'foo';

foo::a();
foo::$a();

$class::a();
$class::$a();
}
