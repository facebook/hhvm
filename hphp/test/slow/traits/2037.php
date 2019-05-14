<?php

trait foo {
    public function abc() {
    }
}

interface baz {
    public function abc();
}

class bar implements baz {
    use foo;

}
<<__EntryPoint>> function main() {
new bar;
print "OK\n";
}
