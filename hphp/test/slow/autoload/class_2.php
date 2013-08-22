<?php 
class _2 {
    public function __construct() {
        echo "__autoload_class_2";
       function __autoload($class_name) {
	include "./class".$class_name . '.php';
	}
	$a= new _3;
    }
}
?>
