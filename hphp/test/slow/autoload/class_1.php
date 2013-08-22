<?php 
class _1 {
    public function __construct() {
       echo "__autoload_class_1";
       function __autoload($class_name) {
	include "./class".$class_name . '.php';
	}
	$a= new _2;
    }
}
?>
