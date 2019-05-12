<?php
class foo {
        private function bar() {
                echo "private!\n";
        }
}

class fooson extends foo {
        function barson() {
                $this->bar();
        }
}

class foo2son extends fooson {

        function bar() {
                echo "public!\n";
        }
}
<<__EntryPoint>> function main() {
$b = new foo2son();
$b->barson();
}
