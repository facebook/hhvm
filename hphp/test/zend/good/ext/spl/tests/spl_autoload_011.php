<?php
class A {
    public $var = 1;
    public function autoload() {
        echo "var:".$this->var."\n";
    }
    public function __destruct() {
        echo "__destruct__\n";
    }
}

$a = new A;
$a->var = 2;

spl_autoload_register(array($a, 'autoload'));
unset($a);

var_dump(class_exists("C", true));
?>
===DONE===
<?php exit(0); ?>