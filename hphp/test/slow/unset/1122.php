<?php

class cls {
}
$obj = new cls;
$a = array(1,2);
unset($a[$obj]);
var_dump($a);
