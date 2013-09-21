<?php
class foo extends RecursiveIteratorIterator {

    public function __construct($str) {
    }

    public function bar() {
    }
}

$foo = new foo("This is bar");
echo $foo->bar();

?>
==DONE==
<?php exit(0); ?>