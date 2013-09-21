<?php

class Test {
    public $empty = array();
    public $three = array(1, "b"=>"c", 3=>array());
}

var_dump(get_class_vars('Test'));

?>
===DONE===