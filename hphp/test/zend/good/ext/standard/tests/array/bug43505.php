<?php
class Test implements Countable {
    public function count() {
        return $some;
    }
}

$obj = new Test();

$a = array();
$b =& $a['test'];
var_dump($a);

$t = count($obj);

$a = array();
$b =& $a['test'];
var_dump($a);
?>
